#pragma once

#include <JuceHeader.h>
#include "AutomationPanelComponent.h"
#include "ClockDomainsPanelComponent.h"
#include "CodeSurfaceComponent.h"
#include "GlobalRulerComponent.h"
#include "StatusBarComponent.h"
#include "LogPanelComponent.h"
#include "MixerPanelComponent.h"
#include "ModuleLanesComponent.h"
#include "RouteGraphPanelComponent.h"
#include "RouteListPanelComponent.h"
#include "TimingInspectorComponent.h"
#include "TransportPanelComponent.h"
#include "ValidationPanelComponent.h"
#include "../engine/EngineProcessManager.h"

class MainComponent final : public juce::Component,
                            private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

private:
    void timerCallback() override;
    void ensureSelectedModule(const ModuleState& moduleState);
    void setWorkflowMode(const juce::String& mode);
    void updateWorkflowVisibility();

    StatusBarComponent statusBar;
    GlobalRulerComponent globalRuler;
    ModuleLanesComponent moduleLanes;
    CodeSurfaceComponent codeSurface;
    MixerPanelComponent mixerPanel;
    AutomationPanelComponent automationPanel;
    RouteGraphPanelComponent routeGraphPanel;
    RouteListPanelComponent routeListPanel;
    TransportPanelComponent transportPanel;
    TimingInspectorComponent timingInspector;
    ValidationPanelComponent validationPanel;
    ClockDomainsPanelComponent clockDomainsPanel;
    LogPanelComponent logPanel;
    EngineProcessManager processManager;
    TransportState transportStateCache;
    ClockDomainState clockDomainStateCache;
    ModuleState moduleStateCache;
    MixerState mixerStateCache;
    RouteState routeStateCache;
    RegionState regionStateCache;
    RecoveryState recoveryStateCache;
    RenderState renderStateCache;
    AutomationState automationStateCache;
    AnalysisState analysisStateCache;
    StructuralState structuralStateCache;
    ValidationState validationStateCache;
    juce::String selectedModuleId;
    std::uint64_t lastTransportRevision = 0;
    std::uint64_t lastClockDomainRevision = 0;
    std::uint64_t lastModuleRevision = 0;
    std::uint64_t lastMixerRevision = 0;
    std::uint64_t lastRouteRevision = 0;
    std::uint64_t lastRegionRevision = 0;
    std::uint64_t lastRecoveryRevision = 0;
    std::uint64_t lastRenderRevision = 0;
    std::uint64_t lastAutomationRevision = 0;
    std::uint64_t lastAnalysisRevision = 0;
    std::uint64_t lastStructuralRevision = 0;
    std::uint64_t lastValidationRevision = 0;
    bool selectionDirty = true;
    juce::String workflowMode { "arrangement" };
    juce::TextButton arrangementModeButton { "Arrangement" };
    juce::TextButton mixerModeButton { "Mixer" };
    juce::TextButton graphModeButton { "Graph" };
    juce::TextButton codeModeButton { "Code" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
