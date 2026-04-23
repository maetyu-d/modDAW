#include "EngineProcessManager.h"
#include "../util/Ids.h"
#include "../util/TimeUtils.h"

namespace
{
juce::String stateToText(EngineProcessManager::ConnectionState state)
{
    switch (state)
    {
        case EngineProcessManager::ConnectionState::offline: return "offline";
        case EngineProcessManager::ConnectionState::booting: return "booting";
        case EngineProcessManager::ConnectionState::ready: return "ready";
    }

    return "offline";
}

MessageEnvelope makeEnvelope(MessageKind kind,
                             const juce::String& type,
                             const juce::var& payload,
                             const juce::String& correlationId = {})
{
    MessageEnvelope envelope;
    envelope.protocolVersion = moddaw::ids::protocolVersion;
    envelope.messageId = moddaw::time::makeMessageId("host");
    envelope.correlationId = correlationId;
    envelope.timestamp = moddaw::time::makeTimestampUtc();
    envelope.sender = moddaw::ids::senderHost;
    envelope.recipient = moddaw::ids::recipientEngine;
    envelope.kind = kind;
    envelope.type = type;
    envelope.payload = payload;
    return envelope;
}

juce::var makeObject(std::initializer_list<std::pair<const char*, juce::var>> values)
{
    auto* object = new juce::DynamicObject();
    for (const auto& [key, value] : values)
        object->setProperty(key, value);
    return juce::var(object);
}
}

EngineProcessManager::EngineProcessManager()
    : juce::Thread("EngineProcessManager")
{
    configureCallbacks();
}

EngineProcessManager::~EngineProcessManager()
{
    stop();
}

void EngineProcessManager::start()
{
    stop();

    if (! connection.openSocket(moddaw::ids::hostReceivePort, moddaw::ids::engineReceivePort))
    {
        setConnectionState(ConnectionState::offline);
        return;
    }

    auto script = resolveEngineLaunchScript();
    if (! script.existsAsFile())
    {
        appendLog("Engine launch script not found: " + script.getFullPathName());
        connection.closeSocket();
        setConnectionState(ConnectionState::offline);
        return;
    }

    childProcess = std::make_unique<SubprocessHandle>();
    auto command = makeLaunchCommand(script);

    if (! childProcess->start(command))
    {
        appendLog("Failed to start sclang child process");
        childProcess.reset();
        connection.closeSocket();
        setConnectionState(ConnectionState::offline);
        return;
    }

    appendLog("Started sclang child process");
    setConnectionState(ConnectionState::booting);
    sendHandshake();
    lastHandshakeSentAtMs = juce::Time::getMillisecondCounter();
    startThread();
}

void EngineProcessManager::stop()
{
    signalThreadShouldExit();
    stopThread(1000);

    if (childProcess != nullptr)
    {
        childProcess->kill();
        childProcess.reset();
    }

    connection.closeSocket();

    {
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
        childOutputBuffer.clear();
    }

    setConnectionState(ConnectionState::offline);
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

TransportState EngineProcessManager::getTransportState() const
{
    const juce::ScopedLock scopedLock(lock);
    return transportState;
}

ClockDomainState EngineProcessManager::getClockDomainState() const
{
    const juce::ScopedLock scopedLock(lock);
    return clockDomainState;
}

ModuleState EngineProcessManager::getModuleState() const
{
    const juce::ScopedLock scopedLock(lock);
    return moduleState;
}

MixerState EngineProcessManager::getMixerState() const
{
    const juce::ScopedLock scopedLock(lock);
    return mixerState;
}

RouteState EngineProcessManager::getRouteState() const
{
    const juce::ScopedLock scopedLock(lock);
    return routeState;
}

RegionState EngineProcessManager::getRegionState() const
{
    const juce::ScopedLock scopedLock(lock);
    return regionState;
}

RecoveryState EngineProcessManager::getRecoveryState() const
{
    const juce::ScopedLock scopedLock(lock);
    return recoveryState;
}

RenderState EngineProcessManager::getRenderState() const
{
    const juce::ScopedLock scopedLock(lock);
    return renderState;
}

AutomationState EngineProcessManager::getAutomationState() const
{
    const juce::ScopedLock scopedLock(lock);
    return automationState;
}

AnalysisState EngineProcessManager::getAnalysisState() const
{
    const juce::ScopedLock scopedLock(lock);
    return analysisState;
}

StructuralState EngineProcessManager::getStructuralState() const
{
    const juce::ScopedLock scopedLock(lock);
    return structuralState;
}

ValidationState EngineProcessManager::getValidationState() const
{
    const juce::ScopedLock scopedLock(lock);
    return validationState;
}

void EngineProcessManager::requestTransportState()
{
    sendTransportRequestState();
}

void EngineProcessManager::requestTransportPlay()
{
    sendTransportPlay();
}

void EngineProcessManager::requestTransportStop()
{
    sendTransportStop();
}

void EngineProcessManager::requestClockDomainState()
{
    sendClockDomainsRequestState();
}

void EngineProcessManager::requestClockDomainRelation(const juce::String& domainId,
                                                      const juce::String& relationType,
                                                      double phaseOffsetBeats)
{
    if (domainId.isNotEmpty() && relationType.isNotEmpty())
        sendClockDomainsSetRelation(domainId, relationType, phaseOffsetBeats);
}

void EngineProcessManager::requestModuleState()
{
    sendModulesRequestState();
}

void EngineProcessManager::requestMixerState()
{
    sendMixerRequestState();
}

void EngineProcessManager::requestRouteState()
{
    sendRoutesRequestState();
}

void EngineProcessManager::requestRegionState()
{
    sendRegionsRequestState();
}

void EngineProcessManager::requestAutomationState()
{
    sendAutomationRequestState();
}

void EngineProcessManager::requestAnalysisState()
{
    sendAnalysisRequestState();
}

void EngineProcessManager::requestStructuralState()
{
    sendStructuralRequestState();
}

void EngineProcessManager::requestValidationState()
{
    sendValidationRequestState();
}

void EngineProcessManager::requestRecoveryState()
{
    sendRecoveryRequestState();
}

void EngineProcessManager::requestRenderState()
{
    sendRenderRequestState();
}

void EngineProcessManager::requestRenderFullMix()
{
    sendRenderFullMix();
}

void EngineProcessManager::requestRenderStems()
{
    sendRenderStems();
}

void EngineProcessManager::requestModuleActivateNextBar(const juce::String& moduleId)
{
    if (moduleId.isNotEmpty())
        sendModulesActivateNextBar(moduleId);
}

void EngineProcessManager::requestModuleCodeSurfaceUpdateNextBar(const juce::String& moduleId,
                                                                 const juce::String& surfaceId,
                                                                 const juce::String& codeSurface)
{
    if (moduleId.isNotEmpty() && surfaceId.isNotEmpty() && codeSurface.isNotEmpty())
        sendModulesUpdateCodeSurfaceNextBar(moduleId, surfaceId, codeSurface);
}

void EngineProcessManager::requestMixerStripLevel(const juce::String& stripId, double level)
{
    if (stripId.isNotEmpty())
        sendMixerSetLevel(stripId, level);
}

void EngineProcessManager::requestMixerStripMuted(const juce::String& stripId, bool muted)
{
    if (stripId.isNotEmpty())
        sendMixerSetMuted(stripId, muted);
}

void EngineProcessManager::requestMixerStripGroup(const juce::String& stripId, const juce::String& groupId)
{
    if (stripId.isNotEmpty())
        sendMixerAssignGroup(stripId, groupId);
}

void EngineProcessManager::requestMixerSendLevel(const juce::String& sendId, double level)
{
    if (sendId.isNotEmpty())
        sendMixerSetSendLevel(sendId, level);
}

void EngineProcessManager::requestMixerSendMode(const juce::String& sendId, const juce::String& mode)
{
    if (sendId.isNotEmpty() && mode.isNotEmpty())
        sendMixerSetSendMode(sendId, mode);
}

void EngineProcessManager::requestRouteCreate(const juce::String& family,
                                              const juce::String& source,
                                              const juce::String& destination,
                                              bool enabled)
{
    if (family.isNotEmpty() && source.isNotEmpty() && destination.isNotEmpty())
        sendRoutesCreate(family, source, destination, enabled);
}

void EngineProcessManager::requestRouteDelete(const juce::String& routeId)
{
    if (routeId.isNotEmpty())
        sendRoutesDelete(routeId);
}

void EngineProcessManager::requestProjectSave()
{
    sendProjectSave();
}

void EngineProcessManager::requestProjectLoad()
{
    sendProjectLoad();
}

void EngineProcessManager::requestModuleFreezeToRegion(const juce::String& moduleId)
{
    if (moduleId.isNotEmpty())
        sendModuleFreezeToRegion(moduleId);
}

void EngineProcessManager::requestModuleLiveLinkedRegion(const juce::String& moduleId)
{
    if (moduleId.isNotEmpty())
        sendModuleLiveLinkedRegion(moduleId);
}

void EngineProcessManager::requestRegionMove(const juce::String& regionId, double deltaBeats)
{
    if (regionId.isNotEmpty())
        sendRegionMove(regionId, deltaBeats);
}

void EngineProcessManager::requestRegionTrim(const juce::String& regionId, double deltaBeats)
{
    if (regionId.isNotEmpty())
        sendRegionTrim(regionId, deltaBeats);
}

void EngineProcessManager::requestRegionSplit(const juce::String& regionId)
{
    if (regionId.isNotEmpty())
        sendRegionSplit(regionId);
}

void EngineProcessManager::requestRegionDelete(const juce::String& regionId)
{
    if (regionId.isNotEmpty())
        sendRegionDelete(regionId);
}

void EngineProcessManager::requestAutomationAddPoint(const juce::String& laneId, double value)
{
    if (laneId.isNotEmpty())
        sendAutomationAddPoint(laneId, value);
}

void EngineProcessManager::requestAutomationResetDemo(const juce::String& laneId)
{
    if (laneId.isNotEmpty())
        sendAutomationResetDemo(laneId);
}

void EngineProcessManager::requestSceneTransition(const juce::String& quantizationTarget,
                                                  int afterCycles,
                                                  const juce::String& domainId,
                                                  const juce::String& sceneName)
{
    if (quantizationTarget.isNotEmpty() && domainId.isNotEmpty() && sceneName.isNotEmpty())
        sendStructuralScheduleSceneTransition(quantizationTarget, afterCycles, domainId, sceneName);
}

void EngineProcessManager::requestExternalCue(const juce::String& cueName)
{
    sendStructuralExternalCue(cueName.isNotEmpty() ? cueName : juce::String("manual"));
}

void EngineProcessManager::requestPerformanceMacro(const juce::String& macroId, double value)
{
    if (macroId.isNotEmpty())
        sendPerformanceTriggerMacro(macroId, value);
}

void EngineProcessManager::run()
{
    lastPingSentAtMs = juce::Time::getMillisecondCounter();

    while (! threadShouldExit())
    {
        if (childProcess == nullptr)
            break;

        char buffer[2048] {};
        auto bytesRead = childProcess->readProcessOutput(buffer, static_cast<int>(sizeof(buffer) - 1));

        if (bytesRead > 0)
        {
            auto chunk = juce::String::fromUTF8(buffer, bytesRead);
            childOutputBuffer += chunk;

            while (true)
            {
                auto newlineIndex = childOutputBuffer.indexOfChar('\n');

                if (newlineIndex < 0)
                    break;

                auto line = childOutputBuffer.substring(0, newlineIndex).trim();
                childOutputBuffer = childOutputBuffer.substring(newlineIndex + 1);

                if (line.isNotEmpty())
                    appendLog("ENGINE " + line);
            }
        }

        connection.poll();

        auto now = juce::Time::getMillisecondCounter();

        if (getConnectionState() != ConnectionState::ready)
        {
            if (now - lastHandshakeSentAtMs >= 1000)
            {
                sendHandshake();
                lastHandshakeSentAtMs = now;
            }
        }
        else
        {
            if (now - lastPingSentAtMs >= 2000)
            {
                sendPing();
                lastPingSentAtMs = now;
            }
        }

        wait(20);

        if (! childProcess->isRunning())
        {
            appendLog("sclang child process exited");
            break;
        }
    }

    {
        const juce::ScopedLock scopedLock(lock);
        transportState = {};
        clockDomainState = {};
        moduleState = {};
        mixerState = {};
        routeState = {};
        regionState = {};
        automationState = {};
        analysisState = {};
        structuralState = {};
        validationState = {};
    }

    setConnectionState(ConnectionState::offline);
}

void EngineProcessManager::configureCallbacks()
{
    connection.onLog = [this](const juce::String& line)
    {
        appendLog(line);
    };

    connection.onError = [this](const juce::String& line)
    {
        appendLog(line);
    };

    connection.onEnvelope = [this](const MessageEnvelope& envelope)
    {
        handleEnvelope(envelope);
    };
}

void EngineProcessManager::setConnectionState(ConnectionState newState)
{
    const juce::ScopedLock scopedLock(lock);

    if (connectionState == newState)
        return;

    connectionState = newState;
    pendingLogLines.add("STATE " + stateToText(newState));
}

void EngineProcessManager::appendLog(const juce::String& line)
{
    const juce::ScopedLock scopedLock(lock);
    pendingLogLines.add(line);
}

void EngineProcessManager::handleEnvelope(const MessageEnvelope& envelope)
{
    if (envelope.type == moddaw::ids::typeReady)
    {
        setConnectionState(ConnectionState::ready);
        sendTransportRequestState();
        sendClockDomainsRequestState();
        sendModulesRequestState();
        sendMixerRequestState();
        sendRoutesRequestState();
        sendRegionsRequestState();
        sendAutomationRequestState();
        sendAnalysisRequestState();
        sendStructuralRequestState();
        sendPerformanceRequestState();
        sendValidationRequestState();
        sendRecoveryRequestState();
        sendRenderRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typePong)
        return;

    if (envelope.type == moddaw::ids::typeTransportState)
    {
        auto newTransportState = TransportState::fromPayload(envelope.payload);

        {
            const juce::ScopedLock scopedLock(lock);
            transportState = newTransportState;
        }

        appendLog("TRANSPORT " + newTransportState.toSummaryString());

        if (onTransportStateChanged)
            onTransportStateChanged(newTransportState);

        return;
    }

    if (envelope.type == moddaw::ids::typeClockDomainsState)
    {
        auto newClockDomainState = ClockDomainState::fromPayload(envelope.payload);

        {
            const juce::ScopedLock scopedLock(lock);
            clockDomainState = newClockDomainState;
        }

        appendLog("CLOCK-DOMAINS " + newClockDomainState.toSummaryString());
        return;
    }

    if (envelope.type == moddaw::ids::typeClockDomainsRelationChanged)
    {
        appendLog("CLOCK-DOMAINS relation changed " + juce::JSON::toString(envelope.payload, false));
        sendClockDomainsRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeClockDomainsRelationRejected)
    {
        appendLog("CLOCK-DOMAINS relation rejected " + juce::JSON::toString(envelope.payload, false));
        sendClockDomainsRequestState();
        sendValidationRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeModulesState)
    {
        auto newModuleState = ModuleState::fromPayload(envelope.payload);

        {
            const juce::ScopedLock scopedLock(lock);
            moduleState = newModuleState;
        }

        appendLog("MODULES " + newModuleState.toSummaryString());
        return;
    }

    if (envelope.type == moddaw::ids::typeMixerState)
    {
        auto newMixerState = MixerState::fromPayload(envelope.payload);

        {
            const juce::ScopedLock scopedLock(lock);
            mixerState = newMixerState;
        }

        appendLog("MIXER " + newMixerState.toSummaryString());
        return;
    }

    if (envelope.type == moddaw::ids::typeRoutesState)
    {
        auto newRouteState = RouteState::fromPayload(envelope.payload);

        {
            const juce::ScopedLock scopedLock(lock);
            routeState = newRouteState;
        }

        appendLog("ROUTES " + newRouteState.toSummaryString());
        sendMixerRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeRegionsState)
    {
        auto newRegionState = RegionState::fromPayload(envelope.payload);

        {
            const juce::ScopedLock scopedLock(lock);
            regionState = newRegionState;
        }

        appendLog("REGIONS " + newRegionState.toSummaryString());
        return;
    }

    if (envelope.type == moddaw::ids::typeAutomationState)
    {
        auto newAutomationState = AutomationState::fromPayload(envelope.payload);

        {
            const juce::ScopedLock scopedLock(lock);
            automationState = newAutomationState;
        }

        appendLog("AUTOMATION " + newAutomationState.toSummaryString());
        return;
    }

    if (envelope.type == moddaw::ids::typeAnalysisState)
    {
        auto newAnalysisState = AnalysisState::fromPayload(envelope.payload);

        {
            const juce::ScopedLock scopedLock(lock);
            analysisState = newAnalysisState;
        }

        appendLog("ANALYSIS " + newAnalysisState.toSummaryString());
        return;
    }

    if (envelope.type == moddaw::ids::typeStructuralState)
    {
        auto newStructuralState = StructuralState::fromPayload(envelope.payload);

        {
            const juce::ScopedLock scopedLock(lock);
            structuralState = newStructuralState;
        }

        appendLog("STRUCTURAL " + newStructuralState.toSummaryString());
        sendModulesRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeStructuralSceneTransitionScheduled)
    {
        appendLog("STRUCTURAL scene transition scheduled " + juce::JSON::toString(envelope.payload, false));
        sendStructuralRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeStructuralSceneTransitionApplied)
    {
        appendLog("STRUCTURAL scene transition applied " + juce::JSON::toString(envelope.payload, false));
        sendStructuralRequestState();
        sendModulesRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeStructuralSceneTransitionRejected)
    {
        appendLog("STRUCTURAL scene transition rejected " + juce::JSON::toString(envelope.payload, false));
        sendStructuralRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeStructuralExternalCueReceived)
    {
        appendLog("STRUCTURAL external cue " + juce::JSON::toString(envelope.payload, false));
        sendStructuralRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typePerformanceState)
    {
        appendLog("PERFORMANCE " + juce::JSON::toString(envelope.payload, false));
        return;
    }

    if (envelope.type == moddaw::ids::typePerformanceMacroTriggered)
    {
        appendLog("PERFORMANCE macro " + juce::JSON::toString(envelope.payload, false));
        sendModulesRequestState();
        sendStructuralRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typePerformanceError)
    {
        appendLog("PERFORMANCE error " + juce::JSON::toString(envelope.payload, false));
        return;
    }

    if (envelope.type == moddaw::ids::typeValidationState)
    {
        auto newValidationState = ValidationState::fromPayload(envelope.payload);

        {
            const juce::ScopedLock scopedLock(lock);
            validationState = newValidationState;
        }

        appendLog("VALIDATION " + newValidationState.toSummaryString());
        return;
    }

    if (envelope.type == moddaw::ids::typeRecoveryState)
    {
        auto newRecoveryState = RecoveryState::fromPayload(envelope.payload);

        {
            const juce::ScopedLock scopedLock(lock);
            recoveryState = newRecoveryState;
        }

        appendLog("RECOVERY " + newRecoveryState.toSummaryString());
        return;
    }

    if (envelope.type == moddaw::ids::typeRenderState)
    {
        auto newRenderState = RenderState::fromPayload(envelope.payload);

        {
            const juce::ScopedLock scopedLock(lock);
            renderState = newRenderState;
        }

        appendLog("RENDER " + newRenderState.toSummaryString());
        return;
    }

    if (envelope.type == moddaw::ids::typeRenderCompleted)
    {
        auto newRenderState = RenderState::fromPayload(envelope.payload);

        {
            const juce::ScopedLock scopedLock(lock);
            renderState = newRenderState;
        }

        appendLog("RENDER complete " + newRenderState.toSummaryString());
        return;
    }

    if (envelope.type == moddaw::ids::typeRecovered)
    {
        appendLog("RECOVERY restored autosave " + juce::JSON::toString(envelope.payload, false));
        sendClockDomainsRequestState();
        sendModulesRequestState();
        sendRoutesRequestState();
        sendRegionsRequestState();
        sendAutomationRequestState();
        sendMixerRequestState();
        sendValidationRequestState();
        sendRecoveryRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeModulesActivationScheduled)
    {
        appendLog("MODULES activation scheduled " + juce::JSON::toString(envelope.payload, false));
        return;
    }

    if (envelope.type == moddaw::ids::typeModulesCodeSwapScheduled)
    {
        appendLog("MODULES code swap scheduled " + juce::JSON::toString(envelope.payload, false));
        return;
    }

    if (envelope.type == moddaw::ids::typeModulesCodeSwapApplied)
    {
        appendLog("MODULES code swap applied " + juce::JSON::toString(envelope.payload, false));
        sendModulesRequestState();
        sendRecoveryRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeRegionsCreated)
    {
        appendLog("REGIONS created " + juce::JSON::toString(envelope.payload, false));
        sendRegionsRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeRegionsEdited)
    {
        appendLog("REGIONS edited " + juce::JSON::toString(envelope.payload, false));
        sendRegionsRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeRegionsDeleted)
    {
        appendLog("REGIONS deleted " + juce::JSON::toString(envelope.payload, false));
        sendRegionsRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeAutomationPointAdded)
    {
        appendLog("AUTOMATION point added " + juce::JSON::toString(envelope.payload, false));
        sendAutomationRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeAutomationReset)
    {
        appendLog("AUTOMATION reset " + juce::JSON::toString(envelope.payload, false));
        sendAutomationRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeProjectSaved)
    {
        appendLog("PROJECT saved " + juce::JSON::toString(envelope.payload, false));
        sendRecoveryRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeProjectLoaded)
    {
        appendLog("PROJECT loaded " + juce::JSON::toString(envelope.payload, false));
        sendClockDomainsRequestState();
        sendModulesRequestState();
        sendRoutesRequestState();
        sendRegionsRequestState();
        sendAutomationRequestState();
        sendAnalysisRequestState();
        sendStructuralRequestState();
        sendPerformanceRequestState();
        sendMixerRequestState();
        sendValidationRequestState();
        sendRecoveryRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeProjectError)
    {
        appendLog("PROJECT error " + juce::JSON::toString(envelope.payload, false));
        return;
    }

    if (envelope.kind == MessageKind::error)
    {
        appendLog("ENGINE error " + envelope.type + " " + juce::JSON::toString(envelope.payload, false));
        sendRecoveryRequestState();
        return;
    }
}

void EngineProcessManager::sendHandshake()
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeHandshake,
                                 makeObject({
                                     { "hostName", "modular-sc-daw" },
                                     { "milestone", "M28" }
                                 })));
}

void EngineProcessManager::sendPing()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typePing,
                                 makeObject({
                                     { "request", "heartbeat" }
                                 })));
}

void EngineProcessManager::sendTransportRequestState()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typeTransportRequestState,
                                 makeObject({
                                     { "request", "transport-state" }
                                 })));
}

void EngineProcessManager::sendTransportPlay()
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeTransportPlay,
                                 makeObject({
                                     { "origin", "juce-host" }
                                 })));
}

void EngineProcessManager::sendTransportStop()
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeTransportStop,
                                 makeObject({
                                     { "origin", "juce-host" }
                                 })));
}

void EngineProcessManager::sendClockDomainsRequestState()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typeClockDomainsRequestState,
                                 makeObject({
                                     { "request", "clock-domain-state" }
                                 })));
}

void EngineProcessManager::sendClockDomainsSetRelation(const juce::String& domainId,
                                                       const juce::String& relationType,
                                                       double phaseOffsetBeats)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeClockDomainsSetRelation,
                                 makeObject({
                                     { "domainId", domainId },
                                     { "relationType", relationType },
                                     { "phaseOffsetBeats", phaseOffsetBeats }
                                 })));
}

void EngineProcessManager::sendModulesRequestState()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typeModulesRequestState,
                                 makeObject({
                                     { "request", "module-state" }
                                 })));
}

void EngineProcessManager::sendMixerRequestState()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typeMixerRequestState,
                                 makeObject({
                                     { "request", "mixer-state" }
                                 })));
}

void EngineProcessManager::sendRoutesRequestState()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typeRoutesRequestState,
                                 makeObject({
                                     { "request", "route-state" }
                                 })));
}

void EngineProcessManager::sendRegionsRequestState()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typeRegionsRequestState,
                                 makeObject({
                                     { "request", "region-state" }
                                 })));
}

void EngineProcessManager::sendAutomationRequestState()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typeAutomationRequestState,
                                 makeObject({
                                     { "request", "automation-state" }
                                 })));
}

void EngineProcessManager::sendAnalysisRequestState()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typeAnalysisRequestState,
                                 makeObject({
                                     { "request", "analysis-state" }
                                 })));
}

void EngineProcessManager::sendStructuralRequestState()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typeStructuralRequestState,
                                 makeObject({
                                     { "request", "structural-state" }
                                 })));
}

void EngineProcessManager::sendValidationRequestState()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typeValidationRequestState,
                                 makeObject({
                                     { "request", "validation-state" }
                                 })));
}

void EngineProcessManager::sendRecoveryRequestState()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typeRecoveryRequestState,
                                 makeObject({
                                     { "request", "recovery-state" }
                                 })));
}

void EngineProcessManager::sendRenderRequestState()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typeRenderRequestState,
                                 makeObject({
                                     { "request", "render-state" }
                                 })));
}

void EngineProcessManager::sendRenderFullMix()
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeRenderFullMix,
                                 makeObject({
                                     { "startBeat", 0.0 },
                                     { "lengthBeats", 16.0 }
                                 })));
}

void EngineProcessManager::sendRenderStems()
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeRenderStems,
                                 makeObject({
                                     { "startBeat", 0.0 },
                                     { "lengthBeats", 16.0 }
                                 })));
}

void EngineProcessManager::sendMixerSetLevel(const juce::String& stripId, double level)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeMixerSetLevel,
                                 makeObject({
                                     { "stripId", stripId },
                                     { "level", level }
                                 })));
}

void EngineProcessManager::sendMixerSetMuted(const juce::String& stripId, bool muted)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeMixerSetMuted,
                                 makeObject({
                                     { "stripId", stripId },
                                     { "muted", muted }
                                 })));
}

void EngineProcessManager::sendMixerAssignGroup(const juce::String& stripId, const juce::String& groupId)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeMixerAssignGroup,
                                 makeObject({
                                     { "stripId", stripId },
                                     { "groupId", groupId }
                                 })));
}

void EngineProcessManager::sendMixerSetSendLevel(const juce::String& sendId, double level)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeMixerSetSendLevel,
                                 makeObject({
                                     { "sendId", sendId },
                                     { "level", level }
                                 })));
}

void EngineProcessManager::sendMixerSetSendMode(const juce::String& sendId, const juce::String& mode)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeMixerSetSendMode,
                                 makeObject({
                                     { "sendId", sendId },
                                     { "mode", mode }
                                 })));
}

void EngineProcessManager::sendRoutesCreate(const juce::String& family,
                                            const juce::String& source,
                                            const juce::String& destination,
                                            bool enabled)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeRoutesCreate,
                                 makeObject({
                                     { "family", family },
                                     { "source", source },
                                     { "destination", destination },
                                     { "enabled", enabled }
                                 })));
}

void EngineProcessManager::sendRoutesDelete(const juce::String& routeId)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeRoutesDelete,
                                 makeObject({
                                     { "routeId", routeId }
                                 })));
}

void EngineProcessManager::sendProjectSave()
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeProjectSave,
                                 makeObject({
                                     { "target", "default-demo-project" }
                                 })));
}

void EngineProcessManager::sendProjectLoad()
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeProjectLoad,
                                 makeObject({
                                     { "target", "default-demo-project" }
                                 })));
}

void EngineProcessManager::sendModuleFreezeToRegion(const juce::String& moduleId)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeRegionsFreezeModule,
                                 makeObject({
                                     { "moduleId", moduleId },
                                     { "mode", "freeze" }
                                 })));
}

void EngineProcessManager::sendModuleLiveLinkedRegion(const juce::String& moduleId)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeRegionsCreateLiveLinked,
                                 makeObject({
                                     { "moduleId", moduleId },
                                     { "mode", "live-linked" }
                                 })));
}

void EngineProcessManager::sendRegionMove(const juce::String& regionId, double deltaBeats)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeRegionsMove,
                                 makeObject({
                                     { "regionId", regionId },
                                     { "deltaBeats", deltaBeats },
                                     { "snap", "global-beat" }
                                 })));
}

void EngineProcessManager::sendRegionTrim(const juce::String& regionId, double deltaBeats)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeRegionsTrim,
                                 makeObject({
                                     { "regionId", regionId },
                                     { "deltaBeats", deltaBeats },
                                     { "snap", "global-beat" }
                                 })));
}

void EngineProcessManager::sendRegionSplit(const juce::String& regionId)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeRegionsSplit,
                                 makeObject({
                                     { "regionId", regionId },
                                     { "snap", "global-beat-midpoint" }
                                 })));
}

void EngineProcessManager::sendRegionDelete(const juce::String& regionId)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeRegionsDelete,
                                 makeObject({
                                     { "regionId", regionId }
                                 })));
}

void EngineProcessManager::sendAutomationAddPoint(const juce::String& laneId, double value)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeAutomationAddPoint,
                                 makeObject({
                                     { "laneId", laneId },
                                     { "value", value },
                                     { "position", "engine-current-beat" }
                                 })));
}

void EngineProcessManager::sendAutomationResetDemo(const juce::String& laneId)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeAutomationResetDemo,
                                 makeObject({
                                     { "laneId", laneId }
                                 })));
}

void EngineProcessManager::sendStructuralScheduleSceneTransition(const juce::String& quantizationTarget,
                                                                 int afterCycles,
                                                                 const juce::String& domainId,
                                                                 const juce::String& sceneName)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeStructuralScheduleSceneTransition,
                                 makeObject({
                                     { "quantizationTarget", quantizationTarget },
                                     { "afterCycles", afterCycles },
                                     { "domainId", domainId },
                                     { "sceneName", sceneName }
                                 })));
}

void EngineProcessManager::sendStructuralExternalCue(const juce::String& cueName)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeStructuralExternalCue,
                                 makeObject({
                                     { "cueName", cueName }
                                 })));
}

void EngineProcessManager::sendPerformanceRequestState()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typePerformanceRequestState,
                                 makeObject({
                                     { "request", "performance-state" }
                                 })));
}

void EngineProcessManager::sendPerformanceTriggerMacro(const juce::String& macroId, double value)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typePerformanceTriggerMacro,
                                 makeObject({
                                     { "macroId", macroId },
                                     { "value", value },
                                     { "inputSource", "juce-host" }
                                 })));
}

void EngineProcessManager::sendModulesActivateNextBar(const juce::String& moduleId)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeModulesActivateNextBar,
                                 makeObject({
                                     { "moduleId", moduleId },
                                     { "quantisation", "next-bar" }
                                 })));
}

void EngineProcessManager::sendModulesUpdateCodeSurfaceNextBar(const juce::String& moduleId,
                                                               const juce::String& surfaceId,
                                                               const juce::String& codeSurface)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeModulesUpdateCodeSurfaceNextBar,
                                 makeObject({
                                     { "moduleId", moduleId },
                                     { "surfaceId", surfaceId },
                                     { "codeSurface", codeSurface },
                                     { "quantisation", "next-bar" }
                                 })));
}

juce::File EngineProcessManager::resolveEngineLaunchScript() const
{
   #if JUCE_WINDOWS
    return juce::File(MODULAR_SC_DAW_ROOT).getChildFile("sc-engine").getChildFile("scripts").getChildFile("run_engine.bat");
   #else
    return juce::File(MODULAR_SC_DAW_ROOT).getChildFile("sc-engine").getChildFile("scripts").getChildFile("run_engine.sh");
   #endif
}

juce::StringArray EngineProcessManager::makeLaunchCommand(const juce::File& script) const
{
   #if JUCE_WINDOWS
    return { "cmd.exe", "/c", script.getFullPathName(),
             "--host-port", juce::String(moddaw::ids::hostReceivePort),
             "--engine-port", juce::String(moddaw::ids::engineReceivePort) };
   #else
    return { "/bin/bash", script.getFullPathName(),
             "--host-port", juce::String(moddaw::ids::hostReceivePort),
             "--engine-port", juce::String(moddaw::ids::engineReceivePort) };
   #endif
}
