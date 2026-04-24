#pragma once

#include <JuceHeader.h>
#include "AnalysisState.h"
#include "AudioEngine.h"
#include "AutomationState.h"
#include "ClockDomainState.h"
#include "ClockDomainManager.h"
#include "MixerEngine.h"
#include "MixerState.h"
#include "ModuleRegistry.h"
#include "ModuleState.h"
#include "ProjectState.h"
#include "RecoveryState.h"
#include "RegionState.h"
#include "RenderState.h"
#include "RoutingGraph.h"
#include "Scheduler.h"
#include "RouteState.h"
#include "StructuralState.h"
#include "TransportEngine.h"
#include "TransportState.h"
#include "ValidationState.h"

class EngineProcessManager final
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
    ~EngineProcessManager();

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
    void initializeInternalState();
    bool restoreProjectState(const ProjectSnapshot& snapshot, juce::StringArray& errors);
    void bootRuntimeFromCurrentState();
    void applyTransportState(const TransportState& state);
    void applyClockDomainState(const ClockDomainState& state);
    void applyModuleState(const ModuleState& state);
    void applyMixerState(const MixerState& state);
    void applyRouteState(const RouteState& state);
    void appendLog(const juce::String& line);
    void markProjectDirty(const juce::String& reason);
    juce::File defaultProjectFile() const;
    void setConnectionState(ConnectionState newState);
    void bumpTransportRevision();
    void bumpClockDomainRevision();
    void bumpModuleRevision();
    void bumpMixerRevision();
    void bumpRouteRevision();
    void bumpRegionRevision();
    void bumpRecoveryRevision();
    void bumpRenderRevision();
    void bumpAutomationRevision();
    void bumpAnalysisRevision();
    void bumpStructuralRevision();
    void bumpValidationRevision();
    ModuleEntry* findMutableModule(const juce::String& moduleId);
    MixerStripEntry* findMutableStrip(const juce::String& stripId);
    MixerSendEntry* findMutableSend(const juce::String& sendId);
    AutomationLaneEntry* findMutableLane(const juce::String& laneId);
    RouteEntry* findMutableRoute(const juce::String& routeId);

    mutable juce::CriticalSection lock;
    AudioEngine audioEngine;
    TransportEngine transportEngine;
    ClockDomainManager clockDomainManager;
    MixerEngine mixerEngine;
    ModuleRegistry moduleRegistry;
    RoutingGraph routingGraph;
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
    juce::File currentProjectFile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EngineProcessManager)
};
