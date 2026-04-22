#pragma once

#include <JuceHeader.h>
#include "../engine/ModuleState.h"

class ModulesPanelComponent final : public juce::Component
{
public:
    ModulesPanelComponent();

    void setModuleState(const ModuleState& newState);
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    void refreshText();

    ModuleState state;
    juce::Label titleLabel;
    juce::Label summaryLabel;
    juce::TextEditor detailsEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulesPanelComponent)
};
