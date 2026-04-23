#include "RegionState.h"

namespace
{
juce::String readString(const juce::var& object, const juce::Identifier& propertyName)
{
    if (auto* dynamicObject = object.getDynamicObject())
        return dynamicObject->getProperty(propertyName).toString();

    return {};
}

double readDouble(const juce::var& object, const juce::Identifier& propertyName, double fallback)
{
    if (auto* dynamicObject = object.getDynamicObject())
    {
        auto value = dynamicObject->getProperty(propertyName);
        return value.isVoid() ? fallback : static_cast<double>(value);
    }

    return fallback;
}
}

juce::String RegionEntry::toSummaryString() const
{
    return displayName + " [" + regionId + "] beat=" + juce::String(startBeat, 2)
         + " len=" + juce::String(lengthBeats, 2);
}

RegionState RegionState::fromPayload(const juce::var& payload)
{
    RegionState state;

    if (auto* dynamicObject = payload.getDynamicObject())
    {
        if (auto* regionsArray = dynamicObject->getProperty("regions").getArray())
        {
            for (const auto& item : *regionsArray)
            {
                RegionEntry region;
                region.regionId = readString(item, "regionId");
                region.moduleId = readString(item, "moduleId");
                region.displayName = readString(item, "displayName");
                region.kind = readString(item, "kind");
                region.source = readString(item, "source");
                region.startBeat = readDouble(item, "startBeat", 0.0);
                region.lengthBeats = readDouble(item, "lengthBeats", 0.0);
                state.regions.add(region);
            }
        }
    }

    return state;
}

juce::String RegionState::toSummaryString() const
{
    return juce::String(regions.size()) + " regions";
}

juce::Array<RegionEntry> RegionState::regionsForModule(const juce::String& moduleId) const
{
    juce::Array<RegionEntry> result;

    for (const auto& region : regions)
        if (region.moduleId == moduleId)
            result.add(region);

    return result;
}
