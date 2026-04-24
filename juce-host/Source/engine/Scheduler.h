#pragma once

#include <JuceHeader.h>
#include "ClockDomainState.h"
#include "TransportState.h"

class Scheduler
{
public:
    enum class Policy
    {
        immediate,
        nextBeat,
        nextBar,
        nextPhrase
    };

    struct Action
    {
        enum class Kind
        {
            activateModule,
            applySurface
        };

        juce::String actionId;
        Kind kind = Kind::activateModule;
        juce::String moduleId;
        juce::String clockDomainId;
        juce::String surfaceId;
        juce::String payload;
        Policy policy = Policy::immediate;
        double targetBeat = 0.0;
        int targetBarIndex = 0;
        juce::String boundaryLabel;
    };

    using ResolveCallback = std::function<void(const Action&)>;

    void setResolveCallback(ResolveCallback callback);
    void reset();
    void cancel(const juce::String& actionId);
    void setClockDomainState(const ClockDomainState& state);
    void processTransportState(const TransportState& state);
    Action schedule(Action action, const TransportState& transportState);

private:
    void resolveDueActions(const TransportState& transportState, juce::Array<Action>& readyActions);
    Action withResolvedBoundary(Action action, const TransportState& transportState) const;
    double nextPhraseBeatForDomain(const juce::String& domainId, double fallbackBeat) const;

    mutable juce::CriticalSection lock;
    juce::Array<Action> pendingActions;
    ClockDomainState clockDomainState;
    TransportState lastTransportState;
    int actionCounter = 0;
    ResolveCallback resolveCallback;
};
