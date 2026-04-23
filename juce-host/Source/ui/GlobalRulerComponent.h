#pragma once

#include <JuceHeader.h>
#include "../engine/ClockDomainState.h"
#include "../engine/ModuleState.h"
#include "../engine/StructuralState.h"
#include "../engine/TransportState.h"

class GlobalRulerComponent final : public juce::Component
{
public:
    GlobalRulerComponent();

    void setTransportState(const TransportState& newState);
    void setStructuralState(const StructuralState& newState);
    void setSelectedLaneOverlay(const ModuleEntry* selectedModule, const ClockDomainEntry* selectedClockDomain);
    void paint(juce::Graphics& g) override;

private:
    float beatToX(double beatOffset, juce::Rectangle<float> bounds, double visibleBeats) const;

    TransportState state;
    StructuralState structuralState;
    bool hasSelectedOverlay = false;
    ModuleEntry overlayModule;
    ClockDomainEntry overlayClockDomain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlobalRulerComponent)
};
