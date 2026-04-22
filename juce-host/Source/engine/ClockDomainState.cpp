#include "ClockDomainState.h"

namespace
{
double readDouble(const juce::NamedValueSet& values, const juce::Identifier& id, double fallback)
{
    if (auto* value = values.getVarPointer(id))
        return static_cast<double>(*value);

    return fallback;
}

int readInt(const juce::NamedValueSet& values, const juce::Identifier& id, int fallback)
{
    if (auto* value = values.getVarPointer(id))
        return static_cast<int>(*value);

    return fallback;
}

juce::String readString(const juce::NamedValueSet& values, const juce::Identifier& id)
{
    if (auto* value = values.getVarPointer(id))
        return value->toString();

    return {};
}
}

juce::String ClockDomainEntry::toSummaryString() const
{
    auto parentText = parentId.isNotEmpty() ? parentId : "none";
    return displayName + " | " + kind
        + " | parent " + parentText
        + " | ratio " + juce::String(ratioToParent, 3)
        + " | beat " + juce::String(localBeatPosition, 2);
}

ClockDomainState ClockDomainState::fromPayload(const juce::var& payload)
{
    ClockDomainState state;
    auto* object = payload.getDynamicObject();

    if (object == nullptr)
        return state;

    auto domainsVar = object->getProperty("domains");
    if (! domainsVar.isArray())
        return state;

    for (const auto& item : *domainsVar.getArray())
    {
        auto* domainObject = item.getDynamicObject();

        if (domainObject == nullptr)
            continue;

        const auto& values = domainObject->getProperties();
        ClockDomainEntry entry;
        entry.id = readString(values, "id");
        entry.parentId = readString(values, "parentId");
        entry.kind = readString(values, "kind");
        entry.displayName = readString(values, "displayName");
        entry.ratioToParent = readDouble(values, "ratioToParent", entry.ratioToParent);
        entry.phaseOffsetBeats = readDouble(values, "phaseOffsetBeats", entry.phaseOffsetBeats);
        entry.localBeatPosition = readDouble(values, "localBeatPosition", entry.localBeatPosition);
        entry.localBarIndex = readInt(values, "localBarIndex", entry.localBarIndex);
        entry.localBeatInBar = readDouble(values, "localBeatInBar", entry.localBeatInBar);
        state.domains.add(entry);
    }

    return state;
}

juce::String ClockDomainState::toSummaryString() const
{
    return juce::String(domains.size()) + " clock domains";
}

const ClockDomainEntry* ClockDomainState::findById(const juce::String& id) const
{
    for (const auto& domain : domains)
        if (domain.id == id)
            return &domain;

    return nullptr;
}
