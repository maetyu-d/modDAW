#pragma once

#include <JuceHeader.h>
#include "../engine/ClockDomainState.h"
#include "../engine/ModuleState.h"
#include "../engine/TransportState.h"

class TimingInspectorComponent final : public juce::Component
{
public:
    TimingInspectorComponent();

    void setInspectorState(const TransportState& transportState,
                           const ModuleEntry* selectedModule,
                           const ClockDomainEntry* selectedClockDomain);

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    void refreshText();
    static juce::String formatDouble(double value, int decimals);

    juce::Label titleLabel;
    juce::Label summaryLabel;
    juce::TextEditor detailsEditor;

    TransportState transport;
    bool hasSelectedModule = false;
    bool hasSelectedClockDomain = false;
    ModuleEntry module;
    ClockDomainEntry domain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimingInspectorComponent)
};
