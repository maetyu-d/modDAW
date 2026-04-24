#include "Scheduler.h"

void Scheduler::setResolveCallback(ResolveCallback callback)
{
    const juce::ScopedLock scopedLock(lock);
    resolveCallback = std::move(callback);
}

void Scheduler::reset()
{
    const juce::ScopedLock scopedLock(lock);
    pendingActions.clear();
    clockDomainState = {};
    lastTransportState = {};
    actionCounter = 0;
}

void Scheduler::cancel(const juce::String& actionId)
{
    const juce::ScopedLock scopedLock(lock);
    pendingActions.removeIf([&](const Action& action) { return action.actionId == actionId; });
}

void Scheduler::setClockDomainState(const ClockDomainState& state)
{
    const juce::ScopedLock scopedLock(lock);
    clockDomainState = state;
}

void Scheduler::processTransportState(const TransportState& state)
{
    juce::Array<Action> readyActions;
    ResolveCallback callbackCopy;

    {
        const juce::ScopedLock scopedLock(lock);
        resolveDueActions(state, readyActions);
        lastTransportState = state;
        callbackCopy = resolveCallback;
    }

    if (callbackCopy)
        for (const auto& action : readyActions)
            callbackCopy(action);
}

Scheduler::Action Scheduler::schedule(Action action, const TransportState& transportState)
{
    {
        const juce::ScopedLock scopedLock(lock);
        action = withResolvedBoundary(std::move(action), transportState);
        action.actionId = "sched." + juce::String(++actionCounter);

        if (action.policy != Policy::immediate)
            pendingActions.add(action);
    }

    return action;
}

void Scheduler::resolveDueActions(const TransportState& transportState, juce::Array<Action>& readyActions)
{
    for (int i = pendingActions.size(); --i >= 0;)
    {
        const auto& action = pendingActions.getReference(i);
        bool due = false;

        switch (action.policy)
        {
            case Policy::immediate:
                due = true;
                break;
            case Policy::nextBeat:
            case Policy::nextPhrase:
                due = transportState.beatPosition >= action.targetBeat;
                break;
            case Policy::nextBar:
                due = transportState.barIndex >= action.targetBarIndex;
                break;
        }

        if (due)
        {
            readyActions.add(action);
            pendingActions.remove(i);
        }
    }
}

Scheduler::Action Scheduler::withResolvedBoundary(Action action, const TransportState& transportState) const
{
    switch (action.policy)
    {
        case Policy::immediate:
            action.boundaryLabel = "immediate";
            break;
        case Policy::nextBeat:
            action.targetBeat = std::floor(transportState.beatPosition) + 1.0;
            action.targetBarIndex = static_cast<int>(std::floor(action.targetBeat / juce::jmax(1, transportState.meterNumerator))) + 1;
            action.boundaryLabel = "next beat";
            break;
        case Policy::nextBar:
            action.targetBarIndex = juce::jmax(1, transportState.barIndex + 1);
            action.targetBeat = static_cast<double>((action.targetBarIndex - 1) * juce::jmax(1, transportState.meterNumerator));
            action.boundaryLabel = "bar " + juce::String(action.targetBarIndex);
            break;
        case Policy::nextPhrase:
            action.targetBeat = nextPhraseBeatForDomain(action.clockDomainId,
                                                        std::floor(transportState.beatPosition / 8.0) * 8.0 + 8.0);
            action.targetBarIndex = static_cast<int>(std::floor(action.targetBeat / juce::jmax(1, transportState.meterNumerator))) + 1;
            action.boundaryLabel = "next phrase";
            break;
    }

    return action;
}

double Scheduler::nextPhraseBeatForDomain(const juce::String& domainId, double fallbackBeat) const
{
    if (const auto* domain = clockDomainState.findById(domainId))
        return juce::jmax(fallbackBeat, domain->nextPhraseBeat);

    return fallbackBeat;
}
