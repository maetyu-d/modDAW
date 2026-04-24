#pragma once

#include <JuceHeader.h>
#include "ClockDomainState.h"
#include "TransportState.h"

class ClockDomain
{
public:
    ClockDomain() = default;
    explicit ClockDomain(const ClockDomainEntry& definitionToUse, double startSeconds = 0.0);

    const ClockDomainEntry& getDefinition() const noexcept;
    void setDefinition(const ClockDomainEntry& newDefinition);
    void setRelation(const juce::String& relationType, double phaseOffsetBeats);
    void resetFreeRunningAnchor(double startSeconds);

    ClockDomainEntry computeState(const TransportState& transportState,
                                  const ClockDomainEntry* parentState,
                                  double nowSeconds) const;

private:
    ClockDomainEntry definition;
    double freeRunningStartSeconds = 0.0;

    JUCE_LEAK_DETECTOR(ClockDomain)
};
