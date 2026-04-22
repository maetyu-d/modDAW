#pragma once

#include <JuceHeader.h>
#include "../engine/ValidationState.h"

class ValidationPanelComponent final : public juce::Component
{
public:
    ValidationPanelComponent();

    void setValidationState(const ValidationState& newState);

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    void refreshText();

    juce::Label titleLabel;
    juce::Label summaryLabel;
    juce::TextEditor detailsEditor;
    ValidationState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValidationPanelComponent)
};
