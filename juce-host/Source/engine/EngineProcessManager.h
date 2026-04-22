#pragma once

#include <JuceHeader.h>
#include "ClockDomainState.h"
#include "EngineConnection.h"
#include "MixerState.h"
#include "ModuleState.h"
#include "SubprocessHandle.h"
#include "TransportState.h"
#include "ValidationState.h"

class EngineProcessManager final : private juce::Thread
{
public:
    using TransportStateCallback = std::function<void(const TransportState&)>;

    enum class ConnectionState
    {
        offline,
        booting,
        ready
    };

    EngineProcessManager();
    ~EngineProcessManager() override;

    void start();
    void stop();

    ConnectionState getConnectionState() const;
    juce::StringArray takePendingLogLines();
    TransportState getTransportState() const;
    ClockDomainState getClockDomainState() const;
    ModuleState getModuleState() const;
    MixerState getMixerState() const;
    ValidationState getValidationState() const;

    void requestTransportState();
    void requestTransportPlay();
    void requestTransportStop();
    void requestClockDomainState();
    void requestModuleState();
    void requestMixerState();
    void requestValidationState();
    void requestModuleActivateNextBar(const juce::String& moduleId);
    void requestModuleCodeSurfaceUpdateNextBar(const juce::String& moduleId, const juce::String& codeSurface);
    void requestMixerStripLevel(const juce::String& stripId, double level);
    void requestMixerStripMuted(const juce::String& stripId, bool muted);

    TransportStateCallback onTransportStateChanged;

private:
    void run() override;

    void configureCallbacks();
    void setConnectionState(ConnectionState newState);
    void appendLog(const juce::String& line);
    void handleEnvelope(const MessageEnvelope& envelope);
    void sendHandshake();
    void sendPing();
    void sendTransportRequestState();
    void sendTransportPlay();
    void sendTransportStop();
    void sendClockDomainsRequestState();
    void sendModulesRequestState();
    void sendMixerRequestState();
    void sendValidationRequestState();
    void sendModulesActivateNextBar(const juce::String& moduleId);
    void sendModulesUpdateCodeSurfaceNextBar(const juce::String& moduleId, const juce::String& codeSurface);
    void sendMixerSetLevel(const juce::String& stripId, double level);
    void sendMixerSetMuted(const juce::String& stripId, bool muted);
    juce::File resolveEngineLaunchScript() const;
    juce::StringArray makeLaunchCommand(const juce::File& script) const;

    mutable juce::CriticalSection lock;
    std::unique_ptr<SubprocessHandle> childProcess;
    EngineConnection connection;
    ConnectionState connectionState = ConnectionState::offline;
    TransportState transportState;
    ClockDomainState clockDomainState;
    ModuleState moduleState;
    MixerState mixerState;
    ValidationState validationState;
    juce::StringArray pendingLogLines;
    juce::uint32 lastPingSentAtMs = 0;
    juce::uint32 lastHandshakeSentAtMs = 0;
    juce::String childOutputBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EngineProcessManager)
};
