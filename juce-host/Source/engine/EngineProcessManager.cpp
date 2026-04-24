#include "EngineProcessManager.h"

EngineProcessManager::EngineProcessManager()
{
    transportEngine.setStateCallback([this](const TransportState& state)
    {
        applyTransportState(state);
    });

    clockDomainManager.setStateCallback([this](const ClockDomainState& state)
    {
        applyClockDomainState(state);
    });

    moduleRegistry.setStateCallback([this](const ModuleState& state)
    {
        applyModuleState(state);
    });

    mixerEngine.setStateCallback([this](const MixerState& state)
    {
        applyMixerState(state);
    });

    routingGraph.setStateCallback([this](const RouteState& state)
    {
        applyRouteState(state);
    });
}

EngineProcessManager::~EngineProcessManager()
{
    routingGraph.setStateCallback({});
    mixerEngine.setStateCallback({});
    moduleRegistry.setStateCallback({});
    moduleRegistry.reset();
    clockDomainManager.setStateCallback({});
    clockDomainManager.reset();
    transportEngine.setStateCallback({});
    transportEngine.stopPlayback();
}

void EngineProcessManager::start()
{
    stop();

    setConnectionState(ConnectionState::booting);
    appendLog("STATE booting");
    appendLog("ENGINE internal JUCE runtime boot");

    {
        const juce::ScopedLock scopedLock(lock);
        initializeInternalState();
    }

    transportEngine.configure(transportState.tempo, transportState.meterNumerator, transportState.meterDenominator);
    clockDomainManager.setTransportState(transportState);
    clockDomainManager.initialiseDemoDomains();
    moduleRegistry.setClockDomainState(clockDomainManager.getState());
    moduleRegistry.setTransportState(transportState);
    moduleRegistry.initialiseDemoModules();
    mixerEngine.initialiseState(mixerState);
    routingGraph.initialiseState(routeState);
    audioEngine.setTransportState(transportState);
    audioEngine.setModuleState(moduleRegistry.getState());
    audioEngine.setMixerState(mixerState);
    audioEngine.setRouteState(routeState);
    applyTransportState(transportEngine.getState());

    {
        const auto audioStatus = audioEngine.getStatus();
        const juce::ScopedLock scopedLock(lock);
        recoveryState.audioServerReady = audioStatus.ready;
        recoveryState.lastReason = audioStatus.detail;
    }

    setConnectionState(ConnectionState::ready);
    appendLog("STATE ready");
    appendLog("ENGINE J8 native transport + clocks + modules + scheduler + audio ready");
}

void EngineProcessManager::stop()
{
    moduleRegistry.reset();
    clockDomainManager.reset();
    transportEngine.stopPlayback();
    transportEngine.reset();

    const juce::ScopedLock scopedLock(lock);
    transportState = {};
    clockDomainState = {};
    moduleState = {};
    mixerState = {};
    routeState = {};
    regionState = {};
    recoveryState = {};
    renderState = {};
    automationState = {};
    analysisState = {};
    structuralState = {};
    validationState = {};
    transportRevision = 0;
    clockDomainRevision = 0;
    moduleRevision = 0;
    mixerRevision = 0;
    routeRevision = 0;
    regionRevision = 0;
    recoveryRevision = 0;
    renderRevision = 0;
    automationRevision = 0;
    analysisRevision = 0;
    structuralRevision = 0;
    validationRevision = 0;
    pendingLogLines.clear();
    connectionState = ConnectionState::offline;
}

EngineProcessManager::ConnectionState EngineProcessManager::getConnectionState() const
{
    const juce::ScopedLock scopedLock(lock);
    return connectionState;
}

juce::StringArray EngineProcessManager::takePendingLogLines()
{
    const juce::ScopedLock scopedLock(lock);
    auto lines = pendingLogLines;
    pendingLogLines.clear();
    return lines;
}

TransportState EngineProcessManager::getTransportState() const { const juce::ScopedLock scopedLock(lock); return transportState; }
ClockDomainState EngineProcessManager::getClockDomainState() const { const juce::ScopedLock scopedLock(lock); return clockDomainState; }
ModuleState EngineProcessManager::getModuleState() const { const juce::ScopedLock scopedLock(lock); return moduleState; }
MixerState EngineProcessManager::getMixerState() const { const juce::ScopedLock scopedLock(lock); return mixerState; }
RouteState EngineProcessManager::getRouteState() const { const juce::ScopedLock scopedLock(lock); return routeState; }
RegionState EngineProcessManager::getRegionState() const { const juce::ScopedLock scopedLock(lock); return regionState; }
RecoveryState EngineProcessManager::getRecoveryState() const { const juce::ScopedLock scopedLock(lock); return recoveryState; }
RenderState EngineProcessManager::getRenderState() const { const juce::ScopedLock scopedLock(lock); return renderState; }
AutomationState EngineProcessManager::getAutomationState() const { const juce::ScopedLock scopedLock(lock); return automationState; }
AnalysisState EngineProcessManager::getAnalysisState() const { const juce::ScopedLock scopedLock(lock); return analysisState; }
StructuralState EngineProcessManager::getStructuralState() const { const juce::ScopedLock scopedLock(lock); return structuralState; }
ValidationState EngineProcessManager::getValidationState() const { const juce::ScopedLock scopedLock(lock); return validationState; }

std::uint64_t EngineProcessManager::getTransportRevision() const { const juce::ScopedLock scopedLock(lock); return transportRevision; }
std::uint64_t EngineProcessManager::getClockDomainRevision() const { const juce::ScopedLock scopedLock(lock); return clockDomainRevision; }
std::uint64_t EngineProcessManager::getModuleRevision() const { const juce::ScopedLock scopedLock(lock); return moduleRevision; }
std::uint64_t EngineProcessManager::getMixerRevision() const { const juce::ScopedLock scopedLock(lock); return mixerRevision; }
std::uint64_t EngineProcessManager::getRouteRevision() const { const juce::ScopedLock scopedLock(lock); return routeRevision; }
std::uint64_t EngineProcessManager::getRegionRevision() const { const juce::ScopedLock scopedLock(lock); return regionRevision; }
std::uint64_t EngineProcessManager::getRecoveryRevision() const { const juce::ScopedLock scopedLock(lock); return recoveryRevision; }
std::uint64_t EngineProcessManager::getRenderRevision() const { const juce::ScopedLock scopedLock(lock); return renderRevision; }
std::uint64_t EngineProcessManager::getAutomationRevision() const { const juce::ScopedLock scopedLock(lock); return automationRevision; }
std::uint64_t EngineProcessManager::getAnalysisRevision() const { const juce::ScopedLock scopedLock(lock); return analysisRevision; }
std::uint64_t EngineProcessManager::getStructuralRevision() const { const juce::ScopedLock scopedLock(lock); return structuralRevision; }
std::uint64_t EngineProcessManager::getValidationRevision() const { const juce::ScopedLock scopedLock(lock); return validationRevision; }

void EngineProcessManager::requestTransportState()
{
    applyTransportState(transportEngine.getState());
}
void EngineProcessManager::requestClockDomainState()
{
    applyClockDomainState(clockDomainManager.getState());
}
void EngineProcessManager::requestModuleState()
{
    applyModuleState(moduleRegistry.getState());
}
void EngineProcessManager::requestMixerState()
{
    applyMixerState(mixerEngine.getState());
}
void EngineProcessManager::requestRouteState()
{
    applyRouteState(routingGraph.getState());
}
void EngineProcessManager::requestRegionState() { bumpRegionRevision(); }
void EngineProcessManager::requestAutomationState() { bumpAutomationRevision(); }
void EngineProcessManager::requestAnalysisState() { bumpAnalysisRevision(); }
void EngineProcessManager::requestStructuralState() { bumpStructuralRevision(); }
void EngineProcessManager::requestValidationState() { bumpValidationRevision(); }
void EngineProcessManager::requestRecoveryState() { bumpRecoveryRevision(); }
void EngineProcessManager::requestRenderState() { bumpRenderRevision(); }

void EngineProcessManager::requestTransportPlay()
{
    appendLog("ENGINE transport play (J2 native)");
    transportEngine.startPlayback();
}

void EngineProcessManager::requestTransportStop()
{
    appendLog("ENGINE transport stop (J2 native)");
    transportEngine.stopPlayback();
}

void EngineProcessManager::requestClockDomainRelation(const juce::String& domainId,
                                                      const juce::String& relationType,
                                                      double phaseOffsetBeats)
{
    if (clockDomainManager.updateRelation(domainId, relationType, phaseOffsetBeats))
    {
        appendLog("ENGINE clock relation updated internally");
        return;
    }

    appendLog("ENGINE clock relation rejected internally");
}

void EngineProcessManager::requestRenderFullMix()
{
    const juce::ScopedLock scopedLock(lock);
    renderState.status = "complete";
    renderState.lastRenderId = "juce-j1-render";
    renderState.outputDirectory = "internal://render";
    renderState.artifacts.clear();
    {
        RenderArtifact artifact;
        artifact.stemId = "master";
        artifact.path = "internal://render/full-mix";
        artifact.artifactKind = "placeholder";
        artifact.eventCount = 0;
        renderState.artifacts.add(artifact);
    }
    appendLog("ENGINE render full mix requested (J1 stub)");
    bumpRenderRevision();
}

void EngineProcessManager::requestRenderStems()
{
    const juce::ScopedLock scopedLock(lock);
    renderState.status = "complete";
    renderState.lastRenderId = "juce-j1-stems";
    renderState.outputDirectory = "internal://render";
    renderState.artifacts.clear();
    {
        RenderArtifact artifact;
        artifact.stemId = "module.tone";
        artifact.path = "internal://render/tone";
        artifact.artifactKind = "placeholder";
        artifact.eventCount = 0;
        renderState.artifacts.add(artifact);
    }
    {
        RenderArtifact artifact;
        artifact.stemId = "module.pattern";
        artifact.path = "internal://render/pattern";
        artifact.artifactKind = "placeholder";
        artifact.eventCount = 0;
        renderState.artifacts.add(artifact);
    }
    {
        RenderArtifact artifact;
        artifact.stemId = "module.drone";
        artifact.path = "internal://render/drone";
        artifact.artifactKind = "placeholder";
        artifact.eventCount = 0;
        renderState.artifacts.add(artifact);
    }
    appendLog("ENGINE render stems requested (J1 stub)");
    bumpRenderRevision();
}

void EngineProcessManager::requestModuleActivateNextBar(const juce::String& moduleId)
{
    if (moduleRegistry.scheduleActivation(moduleId, Scheduler::Policy::nextBar))
        appendLog("ENGINE activation scheduled for next bar");
}

void EngineProcessManager::requestModuleCodeSurfaceUpdateNextBar(const juce::String& moduleId,
                                                                 const juce::String& surfaceId,
                                                                 const juce::String& codeSurface)
{
    if (moduleRegistry.scheduleSurfaceUpdate(moduleId, surfaceId, codeSurface, Scheduler::Policy::nextBar))
        appendLog("ENGINE behaviour surface queued for next bar");
    else
        appendLog("ENGINE behaviour surface rejected");
}

void EngineProcessManager::requestMixerStripLevel(const juce::String& stripId, double level)
{
    if (mixerEngine.setStripLevel(stripId, level))
        appendLog("ENGINE mixer level updated internally");
}

void EngineProcessManager::requestMixerStripMuted(const juce::String& stripId, bool muted)
{
    if (mixerEngine.setStripMuted(stripId, muted))
        appendLog("ENGINE mixer mute updated internally");
}

void EngineProcessManager::requestMixerStripGroup(const juce::String& stripId, const juce::String& groupId)
{
    if (mixerEngine.setStripGroup(stripId, groupId))
        appendLog("ENGINE mixer group assignment updated internally");
}

void EngineProcessManager::requestMixerSendLevel(const juce::String& sendId, double level)
{
    if (mixerEngine.setSendLevel(sendId, level))
        appendLog("ENGINE send level updated internally");
}

void EngineProcessManager::requestMixerSendMode(const juce::String& sendId, const juce::String& mode)
{
    if (mixerEngine.setSendMode(sendId, mode))
        appendLog("ENGINE send mode updated internally");
}

void EngineProcessManager::requestRouteCreate(const juce::String& family,
                                              const juce::String& source,
                                              const juce::String& destination,
                                              bool enabled)
{
    juce::String errorText;
    if (routingGraph.createRoute(family, source, destination, enabled, errorText))
        appendLog("ENGINE route created internally");
    else
        appendLog("ENGINE route rejected internally: " + errorText);
}

void EngineProcessManager::requestRouteDelete(const juce::String& routeId)
{
    if (routingGraph.deleteRoute(routeId))
        appendLog("ENGINE route deleted internally");
}

void EngineProcessManager::requestProjectSave()
{
    const juce::ScopedLock scopedLock(lock);
    recoveryState.projectDirty = false;
    recoveryState.lastReason = "JUCE J1 placeholder save";
    appendLog("ENGINE project save requested (J1 stub)");
    bumpRecoveryRevision();
}

void EngineProcessManager::requestProjectLoad()
{
    {
        const juce::ScopedLock scopedLock(lock);
        initializeInternalState();
    }

    transportEngine.configure(transportState.tempo, transportState.meterNumerator, transportState.meterDenominator);
    clockDomainManager.setTransportState(transportState);
    clockDomainManager.initialiseDemoDomains();
    moduleRegistry.setClockDomainState(clockDomainManager.getState());
    moduleRegistry.setTransportState(transportState);
    moduleRegistry.initialiseDemoModules();
    mixerEngine.initialiseState(mixerState);
    routingGraph.initialiseState(routeState);
    applyTransportState(transportEngine.getState());
    appendLog("ENGINE project load requested (J1 stub)");
}

void EngineProcessManager::requestModuleFreezeToRegion(const juce::String& moduleId)
{
    const juce::ScopedLock scopedLock(lock);
    RegionEntry region;
    region.regionId = "region.juce." + juce::String(regionState.regions.size() + 1);
    region.moduleId = moduleId;
    region.displayName = "Freeze";
    region.kind = "frozen-placeholder";
    region.regionIdentity = "frozen";
    region.editPolicy = "detached-audio";
    region.source = "juce-j1";
    region.startBeat = 0.0;
    region.lengthBeats = 4.0;
    regionState.regions.add(region);
    appendLog("ENGINE freeze region created internally");
    bumpRegionRevision();
}

void EngineProcessManager::requestModuleLiveLinkedRegion(const juce::String& moduleId)
{
    const juce::ScopedLock scopedLock(lock);
    RegionEntry region;
    region.regionId = "region.juce." + juce::String(regionState.regions.size() + 1);
    region.moduleId = moduleId;
    region.linkedModuleId = moduleId;
    region.displayName = "Live Link";
    region.kind = "live-linked-placeholder";
    region.regionIdentity = "live-linked";
    region.editPolicy = "projection-only";
    region.source = "juce-j1";
    region.startBeat = 0.0;
    region.lengthBeats = 8.0;
    regionState.regions.add(region);
    appendLog("ENGINE live-linked region created internally");
    bumpRegionRevision();
}

void EngineProcessManager::requestRegionMove(const juce::String& regionId, double deltaBeats)
{
    const juce::ScopedLock scopedLock(lock);
    for (auto& region : regionState.regions)
        if (region.regionId == regionId)
            region.startBeat = juce::jmax(0.0, region.startBeat + deltaBeats);
    bumpRegionRevision();
}

void EngineProcessManager::requestRegionTrim(const juce::String& regionId, double deltaBeats)
{
    const juce::ScopedLock scopedLock(lock);
    for (auto& region : regionState.regions)
        if (region.regionId == regionId)
            region.lengthBeats = juce::jmax(1.0, region.lengthBeats + deltaBeats);
    bumpRegionRevision();
}

void EngineProcessManager::requestRegionSplit(const juce::String& regionId)
{
    const juce::ScopedLock scopedLock(lock);
    appendLog("ENGINE region split requested (J1 stub): " + regionId);
    bumpRegionRevision();
}

void EngineProcessManager::requestRegionDelete(const juce::String& regionId)
{
    const juce::ScopedLock scopedLock(lock);
    regionState.regions.removeIf([&](const RegionEntry& region) { return region.regionId == regionId; });
    bumpRegionRevision();
}

void EngineProcessManager::requestAutomationAddPoint(const juce::String& laneId, double value)
{
    const juce::ScopedLock scopedLock(lock);
    if (auto* lane = findMutableLane(laneId))
    {
        AutomationPointEntry point;
        point.beat = transportState.beatPosition;
        point.value = value;
        lane->points.add(point);
        lane->currentValue = value;
        appendLog("ENGINE automation point added internally");
        bumpAutomationRevision();
    }
}

void EngineProcessManager::requestAutomationResetDemo(const juce::String& laneId)
{
    const juce::ScopedLock scopedLock(lock);
    if (auto* lane = findMutableLane(laneId))
    {
        lane->points.clear();
        AutomationPointEntry pointA;
        pointA.beat = 0.0;
        pointA.value = 1.0;
        lane->points.add(pointA);

        AutomationPointEntry pointB;
        pointB.beat = 4.0;
        pointB.value = 0.35;
        lane->points.add(pointB);

        AutomationPointEntry pointC;
        pointC.beat = 8.0;
        pointC.value = 1.0;
        lane->points.add(pointC);
        lane->currentValue = 1.0;
        appendLog("ENGINE automation demo reset internally");
        bumpAutomationRevision();
    }
}

void EngineProcessManager::requestSceneTransition(const juce::String& quantizationTarget,
                                                  int afterCycles,
                                                  const juce::String& domainId,
                                                  const juce::String& sceneName)
{
    const juce::ScopedLock scopedLock(lock);
    StructuralPendingTransitionEntry transition;
    transition.transitionId = "transition.juce." + juce::String(structuralState.pendingTransitions.size() + 1);
    transition.domainId = domainId;
    transition.sceneName = sceneName;
    transition.quantizationTarget = quantizationTarget;
    transition.afterCycles = afterCycles;
    transition.state = "pending";
    structuralState.pendingTransitions.add(transition);
    appendLog("ENGINE scene transition queued internally");
    bumpStructuralRevision();
}

void EngineProcessManager::requestExternalCue(const juce::String& cueName)
{
    const juce::ScopedLock scopedLock(lock);
    structuralState.currentScene = cueName.isNotEmpty() ? cueName : "manual";
    appendLog("ENGINE external cue received internally");
    bumpStructuralRevision();
}

void EngineProcessManager::requestPerformanceMacro(const juce::String& macroId, double value)
{
    const juce::ScopedLock scopedLock(lock);
    juce::ignoreUnused(value);
    appendLog("ENGINE performance macro requested internally: " + macroId);
}

void EngineProcessManager::initializeInternalState()
{
    transportState = {};
    transportState.tempo = 120.0;
    transportState.meterNumerator = 4;
    transportState.meterDenominator = 4;
    transportState.beatPosition = 0.0;
    transportState.barIndex = 1;
    transportState.beatInBar = 1.0;
    transportState.phaseWithinBar = 0.0;

    clockDomainState = {};
    {
        ClockDomainEntry domain;
        domain.id = "global.main";
        domain.kind = "global";
        domain.displayName = "Global Main";
        domain.relationType = "global";
        domain.relationDescription = "JUCE native global transport domain";
        domain.meterNumerator = 4;
        domain.meterDenominator = 4;
        domain.ratioToParent = 1.0;
        domain.phaseOffsetBeats = 0.0;
        domain.phraseLengthBars = 2;
        domain.phraseLengthBeats = 8.0;
        domain.phraseIndex = 0;
        domain.phrasePhase = 0.0;
        domain.nextPhraseBeat = 8.0;
        domain.absoluteBeatPosition = 0.0;
        domain.barLengthBeats = 4.0;
        domain.localBeatPosition = 0.0;
        domain.localBarIndex = 1;
        domain.localBeatInBar = 1.0;
        clockDomainState.domains.add(domain);
    }
    {
        ClockDomainEntry domain;
        domain.id = "domain.seven";
        domain.parentId = "global.main";
        domain.kind = "local";
        domain.displayName = "Seven Eight";
        domain.relationType = "meterShared";
        domain.relationDescription = "Local 7/8 lane over the global transport";
        domain.meterNumerator = 7;
        domain.meterDenominator = 8;
        domain.ratioToParent = 1.0;
        domain.phaseOffsetBeats = 0.0;
        domain.phraseLengthBars = 4;
        domain.phraseLengthBeats = 14.0;
        domain.phraseIndex = 0;
        domain.phrasePhase = 0.0;
        domain.nextPhraseBeat = 14.0;
        domain.absoluteBeatPosition = 0.0;
        domain.barLengthBeats = 3.5;
        domain.localBeatPosition = 0.0;
        domain.localBarIndex = 1;
        domain.localBeatInBar = 1.0;
        clockDomainState.domains.add(domain);
    }
    {
        ClockDomainEntry domain;
        domain.id = "domain.triplet";
        domain.parentId = "global.main";
        domain.kind = "derived";
        domain.displayName = "Triplet Drift";
        domain.relationType = "phaseOffset";
        domain.relationDescription = "Derived ratio domain";
        domain.meterNumerator = 3;
        domain.meterDenominator = 4;
        domain.ratioToParent = 1.5;
        domain.phaseOffsetBeats = 0.25;
        domain.phraseLengthBars = 3;
        domain.phraseLengthBeats = 12.0;
        domain.phraseIndex = 0;
        domain.phrasePhase = 0.0;
        domain.nextPhraseBeat = 12.0;
        domain.absoluteBeatPosition = 0.0;
        domain.barLengthBeats = 3.0;
        domain.localBeatPosition = 0.0;
        domain.localBarIndex = 1;
        domain.localBeatInBar = 1.0;
        clockDomainState.domains.add(domain);
    }
    {
        ClockDomainEntry domain;
        domain.id = "domain.free";
        domain.kind = "free";
        domain.displayName = "Free Drift";
        domain.relationType = "freeRunning";
        domain.relationDescription = "Independent free-running domain";
        domain.meterNumerator = 5;
        domain.meterDenominator = 4;
        domain.ratioToParent = 1.0;
        domain.phaseOffsetBeats = 0.0;
        domain.phraseLengthBars = 2;
        domain.phraseLengthBeats = 10.0;
        domain.phraseIndex = 0;
        domain.phrasePhase = 0.0;
        domain.nextPhraseBeat = 10.0;
        domain.absoluteBeatPosition = 0.0;
        domain.barLengthBeats = 5.0;
        domain.localBeatPosition = 0.0;
        domain.localBarIndex = 1;
        domain.localBeatInBar = 1.0;
        clockDomainState.domains.add(domain);
    }

    moduleState = {};

    mixerState = {};
    {
        MixerStripEntry strip;
        strip.id = "master.main";
        strip.displayName = "Master";
        strip.kind = "master";
        strip.childCount = 0;
        strip.hasAudioPath = true;
        strip.level = 1.0;
        strip.muted = false;
        mixerState.strips.add(strip);
    }
    {
        MixerStripEntry strip;
        strip.id = "group.drums";
        strip.displayName = "Drum Group";
        strip.kind = "group";
        strip.childCount = 3;
        strip.hasAudioPath = true;
        strip.level = 1.0;
        strip.muted = false;
        mixerState.strips.add(strip);
    }
    {
        MixerStripEntry strip;
        strip.id = "strip.module.tone";
        strip.displayName = "Tone Pulse";
        strip.kind = "module";
        strip.targetModuleId = "module.tone";
        strip.assignedGroupId = "group.drums";
        strip.childCount = 0;
        strip.hasAudioPath = true;
        strip.level = 1.0;
        strip.muted = false;
        mixerState.strips.add(strip);
    }
    {
        MixerStripEntry strip;
        strip.id = "strip.module.pattern";
        strip.displayName = "Seven Lane";
        strip.kind = "module";
        strip.targetModuleId = "module.pattern";
        strip.assignedGroupId = "group.drums";
        strip.childCount = 0;
        strip.hasAudioPath = false;
        strip.level = 1.0;
        strip.muted = false;
        mixerState.strips.add(strip);
    }
    {
        MixerStripEntry strip;
        strip.id = "strip.module.drone";
        strip.displayName = "Free Drone";
        strip.kind = "module";
        strip.targetModuleId = "module.drone";
        strip.assignedGroupId = "group.drums";
        strip.childCount = 0;
        strip.hasAudioPath = true;
        strip.level = 1.0;
        strip.muted = false;
        mixerState.strips.add(strip);
    }
    {
        MixerGroupEntry group;
        group.id = "group.drums";
        group.displayName = "Drum Group";
        mixerState.groups.add(group);
    }
    {
        MixerSendEntry send;
        send.sendId = "send.tone.space";
        send.displayName = "Tone Send";
        send.sourceStripId = "strip.module.tone";
        send.targetModuleId = "module.tone";
        send.returnStripId = "return.space";
        send.mode = "post";
        send.level = 0.0;
        mixerState.sends.add(send);
    }

    routeState = {};
    {
        RouteEndpointEntry endpoint;
        endpoint.id = "module.tone.audio.out";
        endpoint.displayName = "Tone Out";
        endpoint.family = "audio";
        endpoint.direction = "output";
        endpoint.ownerId = "module.tone";
        routeState.endpoints.add(endpoint);
    }
    {
        RouteEndpointEntry endpoint;
        endpoint.id = "strip.module.tone.audio.in";
        endpoint.displayName = "Tone Strip In";
        endpoint.family = "audio";
        endpoint.direction = "input";
        endpoint.ownerId = "strip.module.tone";
        routeState.endpoints.add(endpoint);
    }
    {
        RouteEndpointEntry endpoint;
        endpoint.id = "module.pattern.audio.out";
        endpoint.displayName = "Pattern Out";
        endpoint.family = "audio";
        endpoint.direction = "output";
        endpoint.ownerId = "module.pattern";
        routeState.endpoints.add(endpoint);
    }
    {
        RouteEndpointEntry endpoint;
        endpoint.id = "strip.module.pattern.audio.in";
        endpoint.displayName = "Pattern Strip In";
        endpoint.family = "audio";
        endpoint.direction = "input";
        endpoint.ownerId = "strip.module.pattern";
        routeState.endpoints.add(endpoint);
    }
    {
        RouteEndpointEntry endpoint;
        endpoint.id = "module.drone.audio.out";
        endpoint.displayName = "Drone Out";
        endpoint.family = "audio";
        endpoint.direction = "output";
        endpoint.ownerId = "module.drone";
        routeState.endpoints.add(endpoint);
    }
    {
        RouteEndpointEntry endpoint;
        endpoint.id = "strip.module.drone.audio.in";
        endpoint.displayName = "Drone Strip In";
        endpoint.family = "audio";
        endpoint.direction = "input";
        endpoint.ownerId = "strip.module.drone";
        routeState.endpoints.add(endpoint);
    }
    {
        RouteEntry route;
        route.routeId = "route.audio.tone";
        route.family = "audio";
        route.source = "module.tone.audio.out";
        route.destination = "strip.module.tone.audio.in";
        route.enabled = true;
        routeState.routes.add(route);
    }
    {
        RouteEntry route;
        route.routeId = "route.audio.pattern";
        route.family = "audio";
        route.source = "module.pattern.audio.out";
        route.destination = "strip.module.pattern.audio.in";
        route.enabled = true;
        routeState.routes.add(route);
    }
    {
        RouteEntry route;
        route.routeId = "route.audio.drone";
        route.family = "audio";
        route.source = "module.drone.audio.out";
        route.destination = "strip.module.drone.audio.in";
        route.enabled = true;
        routeState.routes.add(route);
    }

    regionState = {};

    automationState = {};
    {
        AutomationLaneEntry lane;
        lane.laneId = "automation.tone.level";
        lane.displayName = "Tone Level";
        lane.targetType = "mixer";
        lane.targetId = "strip.module.tone";
        lane.parameterId = "level";
        lane.interpolation = "linear";
        lane.currentValue = 1.0;

        AutomationPointEntry pointA;
        pointA.beat = 0.0;
        pointA.value = 1.0;
        lane.points.add(pointA);

        AutomationPointEntry pointB;
        pointB.beat = 4.0;
        pointB.value = 0.35;
        lane.points.add(pointB);

        AutomationPointEntry pointC;
        pointC.beat = 8.0;
        pointC.value = 1.0;
        lane.points.add(pointC);

        automationState.lanes.add(lane);
    }

    analysisState = {};
    {
        AnalysisModuleEntry module;
        module.moduleId = "module.analysis.tone";
        module.sourceModuleId = "module.tone";
        module.routing = "internal JUCE placeholder";
        module.targetModuleIds.add("module.pattern");
        module.targetModuleIds.add("module.drone");
        module.envelope = 0.0;
        module.onset = false;
        module.density = 0.0;
        module.brightness = 0.0;
        module.lastBeat = 0.0;
        analysisState.modules.add(module);
    }

    structuralState = {};
    structuralState.currentScene = "Ready";

    recoveryState = {};
    recoveryState.engineOnline = true;
    recoveryState.audioServerReady = true;
    recoveryState.projectDirty = false;
    recoveryState.lastReason = "J1 JUCE internal engine";

    renderState = {};
    renderState.status = "idle";

    validationState = {};
    validationState.isValid = true;
    validationState.findingCount = 0;

    transportRevision++;
    clockDomainRevision++;
    moduleRevision++;
    mixerRevision++;
    routeRevision++;
    regionRevision++;
    recoveryRevision++;
    renderRevision++;
    automationRevision++;
    analysisRevision++;
    structuralRevision++;
    validationRevision++;
}

void EngineProcessManager::appendLog(const juce::String& line)
{
    pendingLogLines.add(line);
    while (pendingLogLines.size() > 200)
        pendingLogLines.remove(0);
}

void EngineProcessManager::applyTransportState(const TransportState& state)
{
    TransportStateCallback callbackCopy;

    {
        const juce::ScopedLock scopedLock(lock);
        transportState = state;
        ++transportRevision;
        callbackCopy = onTransportStateChanged;
    }

    clockDomainManager.setTransportState(state);
    moduleRegistry.setTransportState(state);
    audioEngine.setTransportState(state);

    if (callbackCopy)
        callbackCopy(state);
}

void EngineProcessManager::applyClockDomainState(const ClockDomainState& state)
{
    {
        const juce::ScopedLock scopedLock(lock);
        clockDomainState = state;
        ++clockDomainRevision;
    }

    moduleRegistry.setClockDomainState(state);
}

void EngineProcessManager::applyModuleState(const ModuleState& state)
{
    const juce::ScopedLock scopedLock(lock);
    moduleState = state;
    ++moduleRevision;
    audioEngine.setModuleState(state);
}

void EngineProcessManager::applyMixerState(const MixerState& state)
{
    const juce::ScopedLock scopedLock(lock);
    mixerState = state;
    ++mixerRevision;
    audioEngine.setMixerState(state);
}

void EngineProcessManager::applyRouteState(const RouteState& state)
{
    const juce::ScopedLock scopedLock(lock);
    routeState = state;
    ++routeRevision;
    audioEngine.setRouteState(state);
}

void EngineProcessManager::setConnectionState(ConnectionState newState)
{
    const juce::ScopedLock scopedLock(lock);
    connectionState = newState;
}

void EngineProcessManager::bumpTransportRevision() { ++transportRevision; }
void EngineProcessManager::bumpClockDomainRevision() { ++clockDomainRevision; }
void EngineProcessManager::bumpModuleRevision() { ++moduleRevision; }
void EngineProcessManager::bumpMixerRevision() { ++mixerRevision; }
void EngineProcessManager::bumpRouteRevision() { ++routeRevision; }
void EngineProcessManager::bumpRegionRevision() { ++regionRevision; }
void EngineProcessManager::bumpRecoveryRevision() { ++recoveryRevision; }
void EngineProcessManager::bumpRenderRevision() { ++renderRevision; }
void EngineProcessManager::bumpAutomationRevision() { ++automationRevision; }
void EngineProcessManager::bumpAnalysisRevision() { ++analysisRevision; }
void EngineProcessManager::bumpStructuralRevision() { ++structuralRevision; }
void EngineProcessManager::bumpValidationRevision() { ++validationRevision; }

ModuleEntry* EngineProcessManager::findMutableModule(const juce::String& moduleId)
{
    for (auto& module : moduleState.modules)
        if (module.id == moduleId)
            return &module;
    return nullptr;
}

MixerStripEntry* EngineProcessManager::findMutableStrip(const juce::String& stripId)
{
    for (auto& strip : mixerState.strips)
        if (strip.id == stripId)
            return &strip;
    return nullptr;
}

MixerSendEntry* EngineProcessManager::findMutableSend(const juce::String& sendId)
{
    for (auto& send : mixerState.sends)
        if (send.sendId == sendId)
            return &send;
    return nullptr;
}

AutomationLaneEntry* EngineProcessManager::findMutableLane(const juce::String& laneId)
{
    for (auto& lane : automationState.lanes)
        if (lane.laneId == laneId)
            return &lane;
    return nullptr;
}

RouteEntry* EngineProcessManager::findMutableRoute(const juce::String& routeId)
{
    for (auto& route : routeState.routes)
        if (route.routeId == routeId)
            return &route;
    return nullptr;
}
