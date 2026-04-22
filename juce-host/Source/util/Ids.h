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
static constexpr const char* typeValidationRequestState = "validation.requestState";
static constexpr const char* typeValidationState = "validation.state";
static constexpr const char* typeModulesActivateNextBar = "modules.activateNextBar";
static constexpr const char* typeModulesActivationScheduled = "modules.activationScheduled";
static constexpr const char* typeModulesUpdateCodeSurfaceNextBar = "modules.updateCodeSurfaceNextBar";
static constexpr const char* typeModulesCodeSwapScheduled = "modules.codeSwapScheduled";
static constexpr const char* typeModulesCodeSwapApplied = "modules.codeSwapApplied";
} // namespace moddaw::ids
