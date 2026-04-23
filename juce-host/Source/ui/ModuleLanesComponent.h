#pragma once

#include <JuceHeader.h>
#include "../engine/ModuleState.h"
#include "../engine/RegionState.h"

class ModuleLanesComponent final : public juce::Component
{
public:
    using SelectionCallback = std::function<void(const juce::String&)>;
    using FreezeCallback = std::function<void(const juce::String&)>;
    using RegionEditCallback = std::function<void(const juce::String& regionId, const juce::String& action)>;

    ModuleLanesComponent();

    void setModuleState(const ModuleState& newState);
    void setRegionState(const RegionState& newState);
    void setSelectedModuleId(const juce::String& moduleId);
    void paint(juce::Graphics& g) override;
    void mouseUp(const juce::MouseEvent& event) override;

    SelectionCallback onModuleSelected;
    FreezeCallback onFreezeModule;
    RegionEditCallback onRegionEdit;

private:
    int laneIndexAtPosition(juce::Point<float> position) const;
    juce::Rectangle<float> laneBoundsForIndex(int index) const;
    juce::Rectangle<float> freezeButtonBoundsForLane(juce::Rectangle<float> lane) const;
    juce::Rectangle<float> regionRailBoundsForLane(juce::Rectangle<float> lane) const;
    juce::Rectangle<float> regionBoundsInRail(const RegionEntry& region, juce::Rectangle<float> rail) const;
    juce::Rectangle<float> editButtonBoundsForLane(juce::Rectangle<float> lane, int index) const;
    juce::String editActionForIndex(int index) const;
    const RegionEntry* findSelectedRegion() const;

    ModuleState state;
    RegionState regionState;
    juce::String selectedModuleId;
    juce::String selectedRegionId;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleLanesComponent)
};
