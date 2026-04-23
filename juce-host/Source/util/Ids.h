#pragma once

#include <JuceHeader.h>

namespace moddaw::ids
{
static constexpr const char* protocolVersion = "1.0";
static constexpr const char* senderHost = "juce-host";
static constexpr const char* recipientEngine = "sc-engine";
static constexpr int hostReceivePort = 57130;
static constexpr int engineReceivePort = 57131;

static constexpr const char* kindCommand = "command";
static constexpr const char* kindAck = "ack";
static constexpr const char* kindEvent = "event";
static constexpr const char* kindError = "error";
static constexpr const char* kindQuery = "query";
static constexpr const char* kindResponse = "response";

static constexpr const char* typeHandshake = "engine.handshake";
static constexpr const char* typeReady = "engine.ready";
static constexpr const char* typePing = "engine.ping";
static constexpr const char* typePong = "engine.pong";
static constexpr const char* typeTransportRequestState = "transport.requestState";
static constexpr const char* typeTransportState = "transport.state";
static constexpr const char* typeTransportPlay = "transport.play";
static constexpr const char* typeTransportStop = "transport.stop";
static constexpr const char* typeClockDomainsRequestState = "clockdomains.requestState";
static constexpr const char* typeClockDomainsState = "clockdomains.state";
static constexpr const char* typeModulesRequestState = "modules.requestState";
static constexpr const char* typeModulesState = "modules.state";
static constexpr const char* typeMixerRequestState = "mixer.requestState";
static constexpr const char* typeMixerState = "mixer.state";
static constexpr const char* typeMixerSetLevel = "mixer.setLevel";
static constexpr const char* typeMixerSetMuted = "mixer.setMuted";
static constexpr const char* typeRoutesRequestState = "routes.requestState";
static constexpr const char* typeRoutesState = "routes.state";
static constexpr const char* typeRoutesCreate = "routes.create";
static constexpr const char* typeRoutesDelete = "routes.delete";
static constexpr const char* typeRegionsRequestState = "regions.requestState";
static constexpr const char* typeRegionsState = "regions.state";
static constexpr const char* typeRegionsFreezeModule = "regions.freezeModule";
static constexpr const char* typeRegionsCreated = "regions.created";
static constexpr const char* typeRegionsMove = "regions.move";
static constexpr const char* typeRegionsTrim = "regions.trim";
static constexpr const char* typeRegionsSplit = "regions.split";
static constexpr const char* typeRegionsDelete = "regions.delete";
static constexpr const char* typeRegionsEdited = "regions.edited";
static constexpr const char* typeRegionsDeleted = "regions.deleted";
static constexpr const char* typeAutomationRequestState = "automation.requestState";
static constexpr const char* typeAutomationState = "automation.state";
static constexpr const char* typeAutomationAddPoint = "automation.addPoint";
static constexpr const char* typeAutomationPointAdded = "automation.pointAdded";
static constexpr const char* typeAutomationResetDemo = "automation.resetDemo";
static constexpr const char* typeAutomationReset = "automation.reset";
static constexpr const char* typeProjectSave = "project.save";
static constexpr const char* typeProjectLoad = "project.load";
static constexpr const char* typeProjectSaved = "project.saved";
static constexpr const char* typeProjectLoaded = "project.loaded";
static constexpr const char* typeProjectError = "project.error";
static constexpr const char* typeValidationRequestState = "validation.requestState";
static constexpr const char* typeValidationState = "validation.state";
static constexpr const char* typeModulesActivateNextBar = "modules.activateNextBar";
static constexpr const char* typeModulesActivationScheduled = "modules.activationScheduled";
static constexpr const char* typeModulesUpdateCodeSurfaceNextBar = "modules.updateCodeSurfaceNextBar";
static constexpr const char* typeModulesCodeSwapScheduled = "modules.codeSwapScheduled";
static constexpr const char* typeModulesCodeSwapApplied = "modules.codeSwapApplied";
} // namespace moddaw::ids
