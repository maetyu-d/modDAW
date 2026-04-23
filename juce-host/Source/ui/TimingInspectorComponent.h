#pragma once

#include <JuceHeader.h>
#include "../engine/AnalysisState.h"
#include "../engine/ClockDomainState.h"
#include "../engine/ModuleState.h"
#include "../engine/StructuralState.h"
#include "../engine/TransportState.h"

class TimingInspectorComponent final : public juce::Component,
                                       private juce::Button::Listener
{
public:
    TimingInspectorComponent();

    std::function<void(const juce::String& domainId,
                       const juce::String& relationType,
                       double phaseOffsetBeats)> onRelationChangeRequested;

    void setInspectorState(const TransportState& transportState,
                           const ModuleEntry* selectedModule,
                           const ClockDomainEntry* selectedClockDomain,
                           const StructuralState& structuralState,
                           const AnalysisState& analysisState);

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    void buttonClicked(juce::Button* button) override;
    void refreshText();
    void refreshRelationEditor();
    static juce::String formatDouble(double value, int decimals);

    juce::Label titleLabel;
    juce::Label summaryLabel;
    juce::ComboBox relationSelector;
    juce::TextEditor phaseOffsetEditor;
    juce::TextButton applyRelationButton { "Apply" };
    juce::TextEditor detailsEditor;

    TransportState transport;
    bool hasSelectedModule = false;
    bool hasSelectedClockDomain = false;
    ModuleEntry module;
    ClockDomainEntry domain;
    StructuralState structural;
    AnalysisState analysis;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimingInspectorComponent)
};
