#include "ProjectState.h"

namespace
{
juce::var makeStringArray(const juce::StringArray& values)
{
    juce::var array;
    for (const auto& value : values)
        array.append(value);
    return array;
}

juce::var makeIntArray(const juce::Array<int>& values)
{
    juce::var array;
    for (const auto& value : values)
        array.append(value);
    return array;
}

juce::var serialiseCodeSurface(const CodeSurfaceEntry& surface)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("surfaceId", surface.surfaceId);
    object->setProperty("displayName", surface.displayName);
    object->setProperty("role", surface.role);
    object->setProperty("code", surface.code);
    object->setProperty("pendingCode", surface.pendingCode);
    object->setProperty("state", surface.state);
    object->setProperty("diagnostic", surface.diagnostic);
    object->setProperty("revision", surface.revision);
    object->setProperty("pendingCodeSwapBarIndex", surface.pendingCodeSwapBarIndex);
    return juce::var(object);
}

juce::var serialiseModule(const ModuleEntry& module)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("id", module.id);
    object->setProperty("displayName", module.displayName);
    object->setProperty("clockDomainId", module.clockDomainId);
    object->setProperty("timingMode", module.timingMode);
    object->setProperty("lifecycleState", module.lifecycleState);
    object->setProperty("behaviourType", module.behaviourType);
    object->setProperty("laneType", module.laneType);
    object->setProperty("capabilities", makeStringArray(module.capabilities));
    object->setProperty("lastStructuralDirective", module.lastStructuralDirective);
    object->setProperty("analysisSummary", module.analysisSummary);
    object->setProperty("lastAnalysisInfluence", module.lastAnalysisInfluence);
    object->setProperty("codeSurface", module.codeSurface);
    object->setProperty("pendingCodeSurface", module.pendingCodeSurface);
    object->setProperty("codeSurfaceState", module.codeSurfaceState);
    object->setProperty("lastCodeEvalMessage", module.lastCodeEvalMessage);
    object->setProperty("pendingActivationBarIndex", module.pendingActivationBarIndex);
    object->setProperty("pendingCodeSwapBarIndex", module.pendingCodeSwapBarIndex);
    object->setProperty("stepPattern", makeIntArray(module.stepPattern));
    object->setProperty("baseFrequency", module.baseFrequency);
    object->setProperty("accent", module.accent);
    object->setProperty("density", module.density);
    object->setProperty("spread", module.spread);

    juce::var surfaces;
    for (const auto& surface : module.codeSurfaces)
        surfaces.append(serialiseCodeSurface(surface));
    object->setProperty("codeSurfaces", surfaces);

    return juce::var(object);
}

juce::var serialiseTransport(const TransportState& transport)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("isPlaying", transport.isPlaying);
    object->setProperty("tempo", transport.tempo);
    object->setProperty("meterNumerator", transport.meterNumerator);
    object->setProperty("meterDenominator", transport.meterDenominator);
    object->setProperty("beatPosition", transport.beatPosition);
    object->setProperty("barIndex", transport.barIndex);
    object->setProperty("beatInBar", transport.beatInBar);
    object->setProperty("phaseWithinBar", transport.phaseWithinBar);
    return juce::var(object);
}

juce::var serialiseClockDomain(const ClockDomainEntry& domain)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("id", domain.id);
    object->setProperty("parentId", domain.parentId);
    object->setProperty("kind", domain.kind);
    object->setProperty("displayName", domain.displayName);
    object->setProperty("relationType", domain.relationType);
    object->setProperty("relationDescription", domain.relationDescription);
    object->setProperty("meterNumerator", domain.meterNumerator);
    object->setProperty("meterDenominator", domain.meterDenominator);
    object->setProperty("ratioToParent", domain.ratioToParent);
    object->setProperty("phaseOffsetBeats", domain.phaseOffsetBeats);
    object->setProperty("phraseLengthBars", domain.phraseLengthBars);
    object->setProperty("phraseLengthBeats", domain.phraseLengthBeats);
    object->setProperty("phraseIndex", domain.phraseIndex);
    object->setProperty("phrasePhase", domain.phrasePhase);
    object->setProperty("nextPhraseBeat", domain.nextPhraseBeat);
    object->setProperty("absoluteBeatPosition", domain.absoluteBeatPosition);
    object->setProperty("barLengthBeats", domain.barLengthBeats);
    object->setProperty("localBeatPosition", domain.localBeatPosition);
    object->setProperty("localBarIndex", domain.localBarIndex);
    object->setProperty("localBeatInBar", domain.localBeatInBar);
    return juce::var(object);
}

juce::var serialiseMixerStrip(const MixerStripEntry& strip)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("id", strip.id);
    object->setProperty("displayName", strip.displayName);
    object->setProperty("kind", strip.kind);
    object->setProperty("targetModuleId", strip.targetModuleId);
    object->setProperty("assignedGroupId", strip.assignedGroupId);
    object->setProperty("level", strip.level);
    object->setProperty("muted", strip.muted);
    object->setProperty("hasAudioPath", strip.hasAudioPath);
    object->setProperty("childCount", strip.childCount);
    return juce::var(object);
}

juce::var serialiseMixerGroup(const MixerGroupEntry& group)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("id", group.id);
    object->setProperty("displayName", group.displayName);
    return juce::var(object);
}

juce::var serialiseMixerSend(const MixerSendEntry& send)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("sendId", send.sendId);
    object->setProperty("displayName", send.displayName);
    object->setProperty("sourceStripId", send.sourceStripId);
    object->setProperty("targetModuleId", send.targetModuleId);
    object->setProperty("returnStripId", send.returnStripId);
    object->setProperty("mode", send.mode);
    object->setProperty("level", send.level);
    return juce::var(object);
}

juce::var serialiseRouteEndpoint(const RouteEndpointEntry& endpoint)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("id", endpoint.id);
    object->setProperty("displayName", endpoint.displayName);
    object->setProperty("family", endpoint.family);
    object->setProperty("direction", endpoint.direction);
    object->setProperty("ownerId", endpoint.ownerId);
    return juce::var(object);
}

juce::var serialiseRoute(const RouteEntry& route)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("routeId", route.routeId);
    object->setProperty("family", route.family);
    object->setProperty("source", route.source);
    object->setProperty("destination", route.destination);
    object->setProperty("enabled", route.enabled);
    return juce::var(object);
}

juce::var serialiseRegion(const RegionEntry& region)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("regionId", region.regionId);
    object->setProperty("moduleId", region.moduleId);
    object->setProperty("linkedModuleId", region.linkedModuleId);
    object->setProperty("displayName", region.displayName);
    object->setProperty("kind", region.kind);
    object->setProperty("regionIdentity", region.regionIdentity);
    object->setProperty("editPolicy", region.editPolicy);
    object->setProperty("source", region.source);
    object->setProperty("startBeat", region.startBeat);
    object->setProperty("lengthBeats", region.lengthBeats);
    return juce::var(object);
}

juce::var serialiseAutomationPoint(const AutomationPointEntry& point)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("beat", point.beat);
    object->setProperty("value", point.value);
    return juce::var(object);
}

juce::var serialiseAutomationLane(const AutomationLaneEntry& lane)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("laneId", lane.laneId);
    object->setProperty("displayName", lane.displayName);
    object->setProperty("targetType", lane.targetType);
    object->setProperty("targetId", lane.targetId);
    object->setProperty("parameterId", lane.parameterId);
    object->setProperty("interpolation", lane.interpolation);
    object->setProperty("currentValue", lane.currentValue);
    juce::var points;
    for (const auto& point : lane.points)
        points.append(serialiseAutomationPoint(point));
    object->setProperty("points", points);
    return juce::var(object);
}

juce::var serialiseDirective(const StructuralDirectiveEntry& directive)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("directiveId", directive.directiveId);
    object->setProperty("emitterModuleId", directive.emitterModuleId);
    object->setProperty("section", directive.section);
    object->setProperty("syncCue", directive.syncCue);
    object->setProperty("quantizationTarget", directive.quantizationTarget);
    object->setProperty("orchestration", directive.orchestration);
    object->setProperty("targetModuleIds", makeStringArray(directive.targetModuleIds));
    object->setProperty("beat", directive.beat);
    object->setProperty("scheduledBeat", directive.scheduledBeat);
    object->setProperty("phraseIndex", directive.phraseIndex);
    object->setProperty("densityTarget", directive.densityTarget);
    object->setProperty("phraseReset", directive.phraseReset);
    return juce::var(object);
}

juce::var serialisePendingTransition(const StructuralPendingTransitionEntry& transition)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("transitionId", transition.transitionId);
    object->setProperty("domainId", transition.domainId);
    object->setProperty("sceneName", transition.sceneName);
    object->setProperty("quantizationTarget", transition.quantizationTarget);
    object->setProperty("afterCycles", transition.afterCycles);
    object->setProperty("state", transition.state);
    object->setProperty("requestedBeat", transition.requestedBeat);
    object->setProperty("targetBeat", transition.targetBeat);
    return juce::var(object);
}

juce::var serialiseSnapshot(const ProjectSnapshot& snapshot)
{
    auto* root = new juce::DynamicObject();
    root->setProperty("schemaVersion", snapshot.schemaVersion);
    root->setProperty("savedAt", snapshot.savedAt);
    root->setProperty("transport", serialiseTransport(snapshot.transport));

    juce::var domains;
    for (const auto& domain : snapshot.clockDomains.domains)
        domains.append(serialiseClockDomain(domain));
    root->setProperty("clockDomains", domains);

    juce::var modules;
    for (const auto& module : snapshot.modules.modules)
        modules.append(serialiseModule(module));
    root->setProperty("modules", modules);

    juce::var strips;
    for (const auto& strip : snapshot.mixer.strips)
        strips.append(serialiseMixerStrip(strip));
    root->setProperty("mixerStrips", strips);

    juce::var groups;
    for (const auto& group : snapshot.mixer.groups)
        groups.append(serialiseMixerGroup(group));
    root->setProperty("mixerGroups", groups);

    juce::var sends;
    for (const auto& send : snapshot.mixer.sends)
        sends.append(serialiseMixerSend(send));
    root->setProperty("mixerSends", sends);

    juce::var endpoints;
    for (const auto& endpoint : snapshot.routes.endpoints)
        endpoints.append(serialiseRouteEndpoint(endpoint));
    root->setProperty("routeEndpoints", endpoints);

    juce::var routes;
    for (const auto& route : snapshot.routes.routes)
        routes.append(serialiseRoute(route));
    root->setProperty("routes", routes);

    juce::var regions;
    for (const auto& region : snapshot.regions.regions)
        regions.append(serialiseRegion(region));
    root->setProperty("regions", regions);

    juce::var automation;
    for (const auto& lane : snapshot.automation.lanes)
        automation.append(serialiseAutomationLane(lane));
    root->setProperty("automationLanes", automation);

    juce::var directives;
    for (const auto& directive : snapshot.structural.directives)
        directives.append(serialiseDirective(directive));
    root->setProperty("structuralDirectives", directives);

    juce::var pending;
    for (const auto& transition : snapshot.structural.pendingTransitions)
        pending.append(serialisePendingTransition(transition));
    root->setProperty("pendingTransitions", pending);
    root->setProperty("currentScene", snapshot.structural.currentScene);

    return juce::var(root);
}

juce::String readString(const juce::var& source, const juce::String& key, const juce::String& fallback = {})
{
    if (auto* object = source.getDynamicObject())
        return object->getProperty(key).toString();
    return fallback;
}

double readDouble(const juce::var& source, const juce::String& key, double fallback = 0.0)
{
    if (auto* object = source.getDynamicObject())
    {
        auto value = object->getProperty(key);
        return value.isVoid() ? fallback : static_cast<double>(value);
    }
    return fallback;
}

int readInt(const juce::var& source, const juce::String& key, int fallback = 0)
{
    if (auto* object = source.getDynamicObject())
    {
        auto value = object->getProperty(key);
        return value.isVoid() ? fallback : static_cast<int>(value);
    }
    return fallback;
}

bool readBool(const juce::var& source, const juce::String& key, bool fallback = false)
{
    if (auto* object = source.getDynamicObject())
    {
        auto value = object->getProperty(key);
        return value.isVoid() ? fallback : static_cast<bool>(value);
    }
    return fallback;
}

CodeSurfaceEntry loadCodeSurface(const juce::var& value)
{
    CodeSurfaceEntry surface;
    surface.surfaceId = readString(value, "surfaceId");
    surface.displayName = readString(value, "displayName");
    surface.role = readString(value, "role");
    surface.code = readString(value, "code");
    surface.pendingCode = readString(value, "pendingCode");
    surface.state = readString(value, "state");
    surface.diagnostic = readString(value, "diagnostic");
    surface.revision = readInt(value, "revision", 1);
    surface.pendingCodeSwapBarIndex = readInt(value, "pendingCodeSwapBarIndex", 0);
    return surface;
}

ModuleEntry loadModule(const juce::var& value)
{
    ModuleEntry module;
    module.id = readString(value, "id");
    module.displayName = readString(value, "displayName");
    module.clockDomainId = readString(value, "clockDomainId");
    module.timingMode = readString(value, "timingMode");
    module.lifecycleState = readString(value, "lifecycleState");
    module.behaviourType = readString(value, "behaviourType");
    module.laneType = readString(value, "laneType");
    module.lastStructuralDirective = readString(value, "lastStructuralDirective");
    module.analysisSummary = readString(value, "analysisSummary");
    module.lastAnalysisInfluence = readString(value, "lastAnalysisInfluence");
    module.codeSurface = readString(value, "codeSurface");
    module.pendingCodeSurface = readString(value, "pendingCodeSurface");
    module.codeSurfaceState = readString(value, "codeSurfaceState");
    module.lastCodeEvalMessage = readString(value, "lastCodeEvalMessage");
    module.pendingActivationBarIndex = readInt(value, "pendingActivationBarIndex", 0);
    module.pendingCodeSwapBarIndex = readInt(value, "pendingCodeSwapBarIndex", 0);
    module.baseFrequency = readDouble(value, "baseFrequency", 0.0);
    module.accent = readDouble(value, "accent", 1.0);
    module.density = readDouble(value, "density", 1.0);
    module.spread = readDouble(value, "spread", 1.0);

    if (auto* object = value.getDynamicObject())
    {
        if (auto* capabilities = object->getProperty("capabilities").getArray())
            for (const auto& capability : *capabilities)
                module.capabilities.add(capability.toString());

        if (auto* steps = object->getProperty("stepPattern").getArray())
            for (const auto& step : *steps)
                module.stepPattern.add(static_cast<int>(step));

        if (auto* surfaces = object->getProperty("codeSurfaces").getArray())
            for (const auto& surface : *surfaces)
                module.codeSurfaces.add(loadCodeSurface(surface));
    }

    return module;
}

TransportState loadTransport(const juce::var& value)
{
    TransportState transport;
    transport.isPlaying = readBool(value, "isPlaying");
    transport.tempo = readDouble(value, "tempo", 120.0);
    transport.meterNumerator = readInt(value, "meterNumerator", 4);
    transport.meterDenominator = readInt(value, "meterDenominator", 4);
    transport.beatPosition = readDouble(value, "beatPosition", 0.0);
    transport.barIndex = readInt(value, "barIndex", 1);
    transport.beatInBar = readDouble(value, "beatInBar", 1.0);
    transport.phaseWithinBar = readDouble(value, "phaseWithinBar", 0.0);
    return transport;
}

ClockDomainEntry loadClockDomain(const juce::var& value)
{
    ClockDomainEntry domain;
    domain.id = readString(value, "id");
    domain.parentId = readString(value, "parentId");
    domain.kind = readString(value, "kind");
    domain.displayName = readString(value, "displayName");
    domain.relationType = readString(value, "relationType");
    domain.relationDescription = readString(value, "relationDescription");
    domain.meterNumerator = readInt(value, "meterNumerator", 4);
    domain.meterDenominator = readInt(value, "meterDenominator", 4);
    domain.ratioToParent = readDouble(value, "ratioToParent", 1.0);
    domain.phaseOffsetBeats = readDouble(value, "phaseOffsetBeats", 0.0);
    domain.phraseLengthBars = readInt(value, "phraseLengthBars", 1);
    domain.phraseLengthBeats = readDouble(value, "phraseLengthBeats", 4.0);
    domain.phraseIndex = readInt(value, "phraseIndex", 0);
    domain.phrasePhase = readDouble(value, "phrasePhase", 0.0);
    domain.nextPhraseBeat = readDouble(value, "nextPhraseBeat", domain.phraseLengthBeats);
    domain.absoluteBeatPosition = readDouble(value, "absoluteBeatPosition", 0.0);
    domain.barLengthBeats = readDouble(value, "barLengthBeats", 4.0);
    domain.localBeatPosition = readDouble(value, "localBeatPosition", 0.0);
    domain.localBarIndex = readInt(value, "localBarIndex", 1);
    domain.localBeatInBar = readDouble(value, "localBeatInBar", 1.0);
    return domain;
}

MixerStripEntry loadMixerStrip(const juce::var& value)
{
    MixerStripEntry strip;
    strip.id = readString(value, "id");
    strip.displayName = readString(value, "displayName");
    strip.kind = readString(value, "kind");
    strip.targetModuleId = readString(value, "targetModuleId");
    strip.assignedGroupId = readString(value, "assignedGroupId");
    strip.level = readDouble(value, "level", 1.0);
    strip.muted = readBool(value, "muted", false);
    strip.hasAudioPath = readBool(value, "hasAudioPath", true);
    strip.childCount = readInt(value, "childCount", 0);
    return strip;
}

MixerGroupEntry loadMixerGroup(const juce::var& value)
{
    MixerGroupEntry group;
    group.id = readString(value, "id");
    group.displayName = readString(value, "displayName");
    return group;
}

MixerSendEntry loadMixerSend(const juce::var& value)
{
    MixerSendEntry send;
    send.sendId = readString(value, "sendId");
    send.displayName = readString(value, "displayName");
    send.sourceStripId = readString(value, "sourceStripId");
    send.targetModuleId = readString(value, "targetModuleId");
    send.returnStripId = readString(value, "returnStripId");
    send.mode = readString(value, "mode");
    send.level = readDouble(value, "level", 0.0);
    return send;
}

RouteEndpointEntry loadRouteEndpoint(const juce::var& value)
{
    RouteEndpointEntry endpoint;
    endpoint.id = readString(value, "id");
    endpoint.displayName = readString(value, "displayName");
    endpoint.family = readString(value, "family");
    endpoint.direction = readString(value, "direction");
    endpoint.ownerId = readString(value, "ownerId");
    return endpoint;
}

RouteEntry loadRoute(const juce::var& value)
{
    RouteEntry route;
    route.routeId = readString(value, "routeId");
    route.family = readString(value, "family");
    route.source = readString(value, "source");
    route.destination = readString(value, "destination");
    route.enabled = readBool(value, "enabled", true);
    return route;
}

RegionEntry loadRegion(const juce::var& value)
{
    RegionEntry region;
    region.regionId = readString(value, "regionId");
    region.moduleId = readString(value, "moduleId");
    region.linkedModuleId = readString(value, "linkedModuleId");
    region.displayName = readString(value, "displayName");
    region.kind = readString(value, "kind");
    region.regionIdentity = readString(value, "regionIdentity");
    region.editPolicy = readString(value, "editPolicy");
    region.source = readString(value, "source");
    region.startBeat = readDouble(value, "startBeat", 0.0);
    region.lengthBeats = readDouble(value, "lengthBeats", 4.0);
    return region;
}

AutomationPointEntry loadAutomationPoint(const juce::var& value)
{
    AutomationPointEntry point;
    point.beat = readDouble(value, "beat", 0.0);
    point.value = readDouble(value, "value", 1.0);
    return point;
}

AutomationLaneEntry loadAutomationLane(const juce::var& value)
{
    AutomationLaneEntry lane;
    lane.laneId = readString(value, "laneId");
    lane.displayName = readString(value, "displayName");
    lane.targetType = readString(value, "targetType");
    lane.targetId = readString(value, "targetId");
    lane.parameterId = readString(value, "parameterId");
    lane.interpolation = readString(value, "interpolation");
    lane.currentValue = readDouble(value, "currentValue", 1.0);
    if (auto* object = value.getDynamicObject())
        if (auto* points = object->getProperty("points").getArray())
            for (const auto& point : *points)
                lane.points.add(loadAutomationPoint(point));
    return lane;
}

StructuralDirectiveEntry loadDirective(const juce::var& value)
{
    StructuralDirectiveEntry directive;
    directive.directiveId = readString(value, "directiveId");
    directive.emitterModuleId = readString(value, "emitterModuleId");
    directive.section = readString(value, "section");
    directive.syncCue = readString(value, "syncCue");
    directive.quantizationTarget = readString(value, "quantizationTarget");
    directive.orchestration = readString(value, "orchestration");
    directive.beat = readDouble(value, "beat", 0.0);
    directive.scheduledBeat = readDouble(value, "scheduledBeat", 0.0);
    directive.phraseIndex = readInt(value, "phraseIndex", 0);
    directive.densityTarget = readDouble(value, "densityTarget", 0.0);
    directive.phraseReset = readBool(value, "phraseReset", false);
    if (auto* object = value.getDynamicObject())
        if (auto* targets = object->getProperty("targetModuleIds").getArray())
            for (const auto& target : *targets)
                directive.targetModuleIds.add(target.toString());
    return directive;
}

StructuralPendingTransitionEntry loadPendingTransition(const juce::var& value)
{
    StructuralPendingTransitionEntry transition;
    transition.transitionId = readString(value, "transitionId");
    transition.domainId = readString(value, "domainId");
    transition.sceneName = readString(value, "sceneName");
    transition.quantizationTarget = readString(value, "quantizationTarget");
    transition.afterCycles = readInt(value, "afterCycles", 0);
    transition.state = readString(value, "state");
    transition.requestedBeat = readDouble(value, "requestedBeat", 0.0);
    transition.targetBeat = readDouble(value, "targetBeat", -1.0);
    return transition;
}
}

juce::File ProjectState::defaultProjectFile()
{
    auto root = juce::File(MODULAR_SC_DAW_ROOT);
    return root.getChildFile("projects").getChildFile("demo-project.json");
}

ProjectState::Result ProjectState::save(const ProjectSnapshot& snapshot, const juce::File& file)
{
    Result result;
    result.path = file.getFullPathName();

    if (! file.getParentDirectory().createDirectory())
    {
        result.message = "could not create project directory";
        return result;
    }

    if (file.replaceWithText(juce::JSON::toString(serialiseSnapshot(snapshot), true)))
    {
        result.success = true;
        result.message = "saved project";
    }
    else
    {
        result.message = "could not write project file";
    }

    return result;
}

ProjectState::Result ProjectState::load(const juce::File& file, ProjectSnapshot& snapshot)
{
    Result result;
    result.path = file.getFullPathName();

    if (! file.existsAsFile())
    {
        result.message = "project file missing";
        return result;
    }

    auto parsed = juce::JSON::parse(file);
    if (parsed.isVoid())
    {
        result.message = "invalid project json";
        return result;
    }

    snapshot = {};
    snapshot.savedAt = readString(parsed, "savedAt");
    snapshot.transport = loadTransport(parsed.getDynamicObject()->getProperty("transport"));

    if (auto* object = parsed.getDynamicObject())
    {
        if (auto* domains = object->getProperty("clockDomains").getArray())
            for (const auto& domain : *domains)
                snapshot.clockDomains.domains.add(loadClockDomain(domain));

        if (auto* modules = object->getProperty("modules").getArray())
            for (const auto& module : *modules)
                snapshot.modules.modules.add(loadModule(module));

        if (auto* strips = object->getProperty("mixerStrips").getArray())
            for (const auto& strip : *strips)
                snapshot.mixer.strips.add(loadMixerStrip(strip));

        if (auto* groups = object->getProperty("mixerGroups").getArray())
            for (const auto& group : *groups)
                snapshot.mixer.groups.add(loadMixerGroup(group));

        if (auto* sends = object->getProperty("mixerSends").getArray())
            for (const auto& send : *sends)
                snapshot.mixer.sends.add(loadMixerSend(send));

        if (auto* endpoints = object->getProperty("routeEndpoints").getArray())
            for (const auto& endpoint : *endpoints)
                snapshot.routes.endpoints.add(loadRouteEndpoint(endpoint));

        if (auto* routes = object->getProperty("routes").getArray())
            for (const auto& route : *routes)
                snapshot.routes.routes.add(loadRoute(route));

        if (auto* regions = object->getProperty("regions").getArray())
            for (const auto& region : *regions)
                snapshot.regions.regions.add(loadRegion(region));

        if (auto* lanes = object->getProperty("automationLanes").getArray())
            for (const auto& lane : *lanes)
                snapshot.automation.lanes.add(loadAutomationLane(lane));

        if (auto* directives = object->getProperty("structuralDirectives").getArray())
            for (const auto& directive : *directives)
                snapshot.structural.directives.add(loadDirective(directive));

        if (auto* pending = object->getProperty("pendingTransitions").getArray())
            for (const auto& transition : *pending)
                snapshot.structural.pendingTransitions.add(loadPendingTransition(transition));

        snapshot.structural.currentScene = object->getProperty("currentScene").toString();
    }

    result.success = true;
    result.message = "loaded project";
    return result;
}
