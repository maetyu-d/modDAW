#pragma once

#include <JuceHeader.h>
#include "../engine/ModuleState.h"

class ModuleLanesComponent final : public juce::Component
{
public:
    using SelectionCallback = std::function<void(const juce::String&)>;

    ModuleLanesComponent();

    void setModuleState(const ModuleState& newState);
    void setSelectedModuleId(const juce::String& moduleId);
    void paint(juce::Graphics& g) override;
    void mouseUp(const juce::MouseEvent& event) override;

    SelectionCallback onModuleSelected;

private:
    int laneIndexAtPosition(juce::Point<float> position) const;

    ModuleState state;
    juce::String selectedModuleId;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleLanesComponent)
};
