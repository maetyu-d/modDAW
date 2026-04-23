#pragma once

#include <JuceHeader.h>
#include "AnalysisState.h"
#include "AutomationState.h"
#include "ClockDomainState.h"
#include "EngineConnection.h"
#include "MixerState.h"
#include "ModuleState.h"
#include "RegionState.h"
#include "RecoveryState.h"
#include "RouteState.h"
#include "StructuralState.h"
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
    RouteState getRouteState() const;
    RegionState getRegionState() const;
    RecoveryState getRecoveryState() const;
    AutomationState getAutomationState() const;
    AnalysisState getAnalysisState() const;
    StructuralState getStructuralState() const;
    ValidationState getValidationState() const;

    void requestTransportState();
    void requestTransportPlay();
    void requestTransportStop();
    void requestClockDomainState();
    void requestClockDomainRelation(const juce::String& domainId,
                                    const juce::String& relationType,
                                    double phaseOffsetBeats);
    void requestModuleState();
    void requestMixerState();
    void requestRouteState();
    void requestRegionState();
    void requestAutomationState();
    void requestAnalysisState();
    void requestStructuralState();
    void requestValidationState();
    void requestRecoveryState();
    void requestModuleActivateNextBar(const juce::String& moduleId);
    void requestModuleCodeSurfaceUpdateNextBar(const juce::String& moduleId,
                                               const juce::String& surfaceId,
                                               const juce::String& codeSurface);
    void requestMixerStripLevel(const juce::String& stripId, double level);
    void requestMixerStripMuted(const juce::String& stripId, bool muted);
    void requestMixerStripGroup(const juce::String& stripId, const juce::String& groupId);
    void requestMixerSendLevel(const juce::String& sendId, double level);
    void requestMixerSendMode(const juce::String& sendId, const juce::String& mode);
    void requestRouteCreate(const juce::String& family,
                            const juce::String& source,
                            const juce::String& destination,
                            bool enabled);
    void requestRouteDelete(const juce::String& routeId);
    void requestProjectSave();
    void requestProjectLoad();
    void requestModuleFreezeToRegion(const juce::String& moduleId);
    void requestModuleLiveLinkedRegion(const juce::String& moduleId);
    void requestRegionMove(const juce::String& regionId, double deltaBeats);
    void requestRegionTrim(const juce::String& regionId, double deltaBeats);
    void requestRegionSplit(const juce::String& regionId);
    void requestRegionDelete(const juce::String& regionId);
    void requestAutomationAddPoint(const juce::String& laneId, double value);
    void requestAutomationResetDemo(const juce::String& laneId);
    void requestSceneTransition(const juce::String& quantizationTarget,
                                int afterCycles,
                                const juce::String& domainId,
                                const juce::String& sceneName);
    void requestExternalCue(const juce::String& cueName);
    void requestPerformanceMacro(const juce::String& macroId, double value = 1.0);

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
    void sendClockDomainsSetRelation(const juce::String& domainId,
                                     const juce::String& relationType,
                                     double phaseOffsetBeats);
    void sendModulesRequestState();
    void sendMixerRequestState();
    void sendRoutesRequestState();
    void sendRegionsRequestState();
    void sendAutomationRequestState();
    void sendAnalysisRequestState();
    void sendStructuralRequestState();
    void sendValidationRequestState();
    void sendRecoveryRequestState();
    void sendModulesActivateNextBar(const juce::String& moduleId);
    void sendModulesUpdateCodeSurfaceNextBar(const juce::String& moduleId,
                                             const juce::String& surfaceId,
                                             const juce::String& codeSurface);
    void sendMixerSetLevel(const juce::String& stripId, double level);
    void sendMixerSetMuted(const juce::String& stripId, bool muted);
    void sendMixerAssignGroup(const juce::String& stripId, const juce::String& groupId);
    void sendMixerSetSendLevel(const juce::String& sendId, double level);
    void sendMixerSetSendMode(const juce::String& sendId, const juce::String& mode);
    void sendRoutesCreate(const juce::String& family,
                          const juce::String& source,
                          const juce::String& destination,
                          bool enabled);
    void sendRoutesDelete(const juce::String& routeId);
    void sendProjectSave();
    void sendProjectLoad();
    void sendModuleFreezeToRegion(const juce::String& moduleId);
    void sendModuleLiveLinkedRegion(const juce::String& moduleId);
    void sendRegionMove(const juce::String& regionId, double deltaBeats);
    void sendRegionTrim(const juce::String& regionId, double deltaBeats);
    void sendRegionSplit(const juce::String& regionId);
    void sendRegionDelete(const juce::String& regionId);
    void sendAutomationAddPoint(const juce::String& laneId, double value);
    void sendAutomationResetDemo(const juce::String& laneId);
    void sendStructuralScheduleSceneTransition(const juce::String& quantizationTarget,
                                               int afterCycles,
                                               const juce::String& domainId,
                                               const juce::String& sceneName);
    void sendStructuralExternalCue(const juce::String& cueName);
    void sendPerformanceRequestState();
    void sendPerformanceTriggerMacro(const juce::String& macroId, double value);
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
    RouteState routeState;
    RegionState regionState;
    RecoveryState recoveryState;
    AutomationState automationState;
    AnalysisState analysisState;
    StructuralState structuralState;
    ValidationState validationState;
    juce::StringArray pendingLogLines;
    juce::uint32 lastPingSentAtMs = 0;
    juce::uint32 lastHandshakeSentAtMs = 0;
    juce::String childOutputBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EngineProcessManager)
};
