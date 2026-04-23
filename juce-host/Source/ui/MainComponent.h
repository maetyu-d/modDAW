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

private:
    void timerCallback() override;
    void ensureSelectedModule(const ModuleState& moduleState);

    StatusBarComponent statusBar;
    GlobalRulerComponent globalRuler;
    ModuleLanesComponent moduleLanes;
    CodeSurfaceComponent codeSurface;
    MixerPanelComponent mixerPanel;
    AutomationPanelComponent automationPanel;
    RouteListPanelComponent routeListPanel;
    TransportPanelComponent transportPanel;
    TimingInspectorComponent timingInspector;
    ValidationPanelComponent validationPanel;
    ClockDomainsPanelComponent clockDomainsPanel;
    LogPanelComponent logPanel;
    EngineProcessManager processManager;
    juce::String selectedModuleId;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
