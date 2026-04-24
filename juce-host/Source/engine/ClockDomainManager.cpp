#include "ClockDomainManager.h"

ClockDomainManager::ClockDomainManager() = default;
ClockDomainManager::~ClockDomainManager() = default;

void ClockDomainManager::setStateCallback(StateCallback callback)
{
    const juce::ScopedLock scopedLock(lock);
    stateCallback = std::move(callback);
}

void ClockDomainManager::initialiseDemoDomains()
{
    const auto nowSeconds = currentSeconds();

    {
        const juce::ScopedLock scopedLock(lock);
        domains.clear();

        ClockDomainEntry globalDomain;
        globalDomain.id = "global.main";
        globalDomain.kind = "global";
        globalDomain.displayName = "Global Main";
        globalDomain.relationType = "global";
        globalDomain.relationDescription = "JUCE native global transport domain";
        globalDomain.meterNumerator = 4;
        globalDomain.meterDenominator = 4;
        globalDomain.ratioToParent = 1.0;
        globalDomain.phraseLengthBars = 2;
        domains.add(ClockDomain(globalDomain, nowSeconds));

        ClockDomainEntry localSeven;
        localSeven.id = "domain.seven";
        localSeven.parentId = "global.main";
        localSeven.kind = "local";
        localSeven.displayName = "Seven Eight";
        localSeven.relationType = "meterShared";
        localSeven.relationDescription = "Local 7/8 lane over the global transport";
        localSeven.meterNumerator = 7;
        localSeven.meterDenominator = 8;
        localSeven.ratioToParent = 1.0;
        localSeven.phraseLengthBars = 4;
        domains.add(ClockDomain(localSeven, nowSeconds));

        ClockDomainEntry triplet;
        triplet.id = "domain.triplet";
        triplet.parentId = "global.main";
        triplet.kind = "derived";
        triplet.displayName = "Triplet Drift";
        triplet.relationType = "phaseOffset";
        triplet.relationDescription = "Derived ratio domain";
        triplet.meterNumerator = 3;
        triplet.meterDenominator = 4;
        triplet.ratioToParent = 1.5;
        triplet.phaseOffsetBeats = 0.25;
        triplet.phraseLengthBars = 3;
        domains.add(ClockDomain(triplet, nowSeconds));

        ClockDomainEntry freeRunning;
        freeRunning.id = "domain.free";
        freeRunning.kind = "free";
        freeRunning.displayName = "Free Drift";
        freeRunning.relationType = "freeRunning";
        freeRunning.relationDescription = "Independent free-running domain";
        freeRunning.meterNumerator = 5;
        freeRunning.meterDenominator = 4;
        freeRunning.ratioToParent = 1.0;
        freeRunning.phraseLengthBars = 2;
        domains.add(ClockDomain(freeRunning, nowSeconds));
    }

    recomputeState();
    startTimerHz(8);
}

void ClockDomainManager::reset()
{
    stopTimer();
    const juce::ScopedLock scopedLock(lock);
    domains.clear();
    currentState = {};
}

void ClockDomainManager::setTransportState(const TransportState& state)
{
    {
        const juce::ScopedLock scopedLock(lock);
        transportState = state;
    }

    recomputeState();
}

bool ClockDomainManager::updateRelation(const juce::String& domainId,
                                        const juce::String& relationType,
                                        double phaseOffsetBeats)
{
    const juce::StringArray allowed { "tempoShared", "meterShared", "phaseShared", "phaseOffset", "hardSync" };
    bool updated = false;

    {
        const juce::ScopedLock scopedLock(lock);
        for (auto& domain : domains)
        {
            auto definition = domain.getDefinition();
            if (definition.id != domainId)
                continue;

            if (definition.kind == "global" || definition.kind == "free")
                return false;

            if (! allowed.contains(relationType))
                return false;

            domain.setRelation(relationType, juce::jlimit(-64.0, 64.0, phaseOffsetBeats));
            updated = true;
            break;
        }
    }

    if (updated)
        recomputeState();

    return updated;
}

ClockDomainState ClockDomainManager::getState() const
{
    const juce::ScopedLock scopedLock(lock);
    return currentState;
}

void ClockDomainManager::timerCallback()
{
    recomputeState();
}

void ClockDomainManager::recomputeState()
{
    ClockDomainState newState;
    StateCallback callbackCopy;

    {
        const juce::ScopedLock scopedLock(lock);
        const auto nowSeconds = currentSeconds();

        for (const auto& domain : domains)
        {
            const auto& definition = domain.getDefinition();
            const auto* parentState = definition.parentId.isNotEmpty() ? newState.findById(definition.parentId) : nullptr;
            newState.domains.add(domain.computeState(transportState, parentState, nowSeconds));
        }

        currentState = newState;
        callbackCopy = stateCallback;
    }

    if (callbackCopy)
        callbackCopy(newState);
}

double ClockDomainManager::currentSeconds()
{
    return juce::Time::getMillisecondCounterHiRes() * 0.001;
}
