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
        automationState = {};
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

AutomationState EngineProcessManager::getAutomationState() const
{
    const juce::ScopedLock scopedLock(lock);
    return automationState;
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

void EngineProcessManager::requestValidationState()
{
    sendValidationRequestState();
}

void EngineProcessManager::requestModuleActivateNextBar(const juce::String& moduleId)
{
    if (moduleId.isNotEmpty())
        sendModulesActivateNextBar(moduleId);
}

void EngineProcessManager::requestModuleCodeSurfaceUpdateNextBar(const juce::String& moduleId, const juce::String& codeSurface)
{
    if (moduleId.isNotEmpty() && codeSurface.isNotEmpty())
        sendModulesUpdateCodeSurfaceNextBar(moduleId, codeSurface);
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
        sendValidationRequestState();
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
        sendMixerRequestState();
        sendValidationRequestState();
        return;
    }

    if (envelope.type == moddaw::ids::typeProjectError)
    {
        appendLog("PROJECT error " + juce::JSON::toString(envelope.payload, false));
        return;
    }
}

void EngineProcessManager::sendHandshake()
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeHandshake,
                                 makeObject({
                                     { "hostName", "modular-sc-daw" },
                                     { "milestone", "M17" }
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

void EngineProcessManager::sendValidationRequestState()
{
    connection.send(makeEnvelope(MessageKind::query,
                                 moddaw::ids::typeValidationRequestState,
                                 makeObject({
                                     { "request", "validation-state" }
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

void EngineProcessManager::sendModulesActivateNextBar(const juce::String& moduleId)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeModulesActivateNextBar,
                                 makeObject({
                                     { "moduleId", moduleId },
                                     { "quantisation", "next-bar" }
                                 })));
}

void EngineProcessManager::sendModulesUpdateCodeSurfaceNextBar(const juce::String& moduleId, const juce::String& codeSurface)
{
    connection.send(makeEnvelope(MessageKind::command,
                                 moddaw::ids::typeModulesUpdateCodeSurfaceNextBar,
                                 makeObject({
                                     { "moduleId", moduleId },
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
