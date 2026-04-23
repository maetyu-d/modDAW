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
#include "RenderState.h"
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
    RenderState getRenderState() const;
    AutomationState getAutomationState() const;
    AnalysisState getAnalysisState() const;
    StructuralState getStructuralState() const;
    ValidationState getValidationState() const;
    std::uint64_t getTransportRevision() const;
    std::uint64_t getClockDomainRevision() const;
    std::uint64_t getModuleRevision() const;
    std::uint64_t getMixerRevision() const;
    std::uint64_t getRouteRevision() const;
    std::uint64_t getRegionRevision() const;
    std::uint64_t getRecoveryRevision() const;
    std::uint64_t getRenderRevision() const;
    std::uint64_t getAutomationRevision() const;
    std::uint64_t getAnalysisRevision() const;
    std::uint64_t getStructuralRevision() const;
    std::uint64_t getValidationRevision() const;

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
    void requestRenderState();
    void requestRenderFullMix();
    void requestRenderStems();
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
    void sendRenderRequestState();
    void sendRenderFullMix();
    void sendRenderStems();
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
    void cleanupStaleEngineProcesses() const;
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
    RenderState renderState;
    AutomationState automationState;
    AnalysisState analysisState;
    StructuralState structuralState;
    ValidationState validationState;
    std::uint64_t transportRevision = 0;
    std::uint64_t clockDomainRevision = 0;
    std::uint64_t moduleRevision = 0;
    std::uint64_t mixerRevision = 0;
    std::uint64_t routeRevision = 0;
    std::uint64_t regionRevision = 0;
    std::uint64_t recoveryRevision = 0;
    std::uint64_t renderRevision = 0;
    std::uint64_t automationRevision = 0;
    std::uint64_t analysisRevision = 0;
    std::uint64_t structuralRevision = 0;
    std::uint64_t validationRevision = 0;
    juce::StringArray pendingLogLines;
    juce::HashMap<juce::String, juce::uint32> lastRateLimitedLogAtMs;
    juce::uint32 lastPingSentAtMs = 0;
    juce::uint32 lastHandshakeSentAtMs = 0;
    juce::String childOutputBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EngineProcessManager)
};
