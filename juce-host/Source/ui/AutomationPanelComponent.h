#pragma once

#include <JuceHeader.h>
#include "../engine/AutomationState.h"

class AutomationPanelComponent final : public juce::Component,
                                       private juce::Button::Listener
{
public:
    AutomationPanelComponent();
    ~AutomationPanelComponent() override;

    void setAutomationState(const AutomationState& newState);

    std::function<void(const juce::String& laneId, double value)> onAddPoint;
    std::function<void(const juce::String& laneId)> onResetDemo;

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    void buttonClicked(juce::Button* button) override;
    void refreshLabels();
    juce::Rectangle<float> laneBounds() const;

    juce::Label titleLabel;
    juce::Label summaryLabel;
    juce::TextButton addLowButton { "Add Low @ Now" };
    juce::TextButton addHighButton { "Add High @ Now" };
    juce::TextButton resetButton { "Reset Demo" };
    AutomationState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationPanelComponent)
};
