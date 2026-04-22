#pragma once

#include <JuceHeader.h>
#include "../engine/ClockDomainState.h"

class ClockDomainsPanelComponent final : public juce::Component
{
public:
    ClockDomainsPanelComponent();

    void setClockDomainState(const ClockDomainState& newState);
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    void refreshText();

    ClockDomainState state;
    juce::Label titleLabel;
    juce::Label summaryLabel;
    juce::TextEditor detailsEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClockDomainsPanelComponent)
};
