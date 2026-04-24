#pragma once

#include <JuceHeader.h>
#include "ClockDomain.h"

class ClockDomainManager final : private juce::Timer
{
public:
    using StateCallback = std::function<void(const ClockDomainState&)>;

    ClockDomainManager();
    ~ClockDomainManager() override;

    void setStateCallback(StateCallback callback);
    void initialiseDemoDomains();
    void initialiseFromState(const ClockDomainState& state);
    void reset();
    void setTransportState(const TransportState& state);
    bool updateRelation(const juce::String& domainId, const juce::String& relationType, double phaseOffsetBeats);
    ClockDomainState getState() const;

private:
    void timerCallback() override;
    void recomputeState();
    static double currentSeconds();

    mutable juce::CriticalSection lock;
    juce::Array<ClockDomain> domains;
    TransportState transportState;
    ClockDomainState currentState;
    StateCallback stateCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClockDomainManager)
};
