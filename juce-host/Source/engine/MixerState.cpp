#include "MixerState.h"

namespace
{
juce::String readString(const juce::var& object, const juce::Identifier& propertyName)
{
    if (auto* dynamicObject = object.getDynamicObject())
        return dynamicObject->getProperty(propertyName).toString();

    return {};
}

bool readBool(const juce::var& object, const juce::Identifier& propertyName, bool fallbackValue)
{
    if (auto* dynamicObject = object.getDynamicObject())
        return static_cast<bool>(dynamicObject->getProperty(propertyName));

    return fallbackValue;
}

double readDouble(const juce::var& object, const juce::Identifier& propertyName, double fallbackValue)
{
    if (auto* dynamicObject = object.getDynamicObject())
        return static_cast<double>(dynamicObject->getProperty(propertyName));

    return fallbackValue;
}
}

juce::String MixerStripEntry::toSummaryString() const
{
    return displayName + " [" + id + "] level=" + juce::String(level, 2)
         + " muted=" + juce::String(muted ? "true" : "false");
}

MixerState MixerState::fromPayload(const juce::var& payload)
{
    MixerState state;

    if (auto* dynamicObject = payload.getDynamicObject())
    {
        if (auto* stripsArray = dynamicObject->getProperty("strips").getArray())
        {
            for (const auto& item : *stripsArray)
            {
                MixerStripEntry strip;
                strip.id = readString(item, "id");
                strip.displayName = readString(item, "displayName");
                strip.kind = readString(item, "kind");
                strip.targetModuleId = readString(item, "targetModuleId");
                strip.hasAudioPath = readBool(item, "hasAudioPath", false);
                strip.level = readDouble(item, "level", 1.0);
                strip.muted = readBool(item, "muted", false);
                state.strips.add(strip);
            }
        }
    }

    return state;
}

juce::String MixerState::toSummaryString() const
{
    return juce::String(strips.size()) + " mixer strips";
}

const MixerStripEntry* MixerState::findById(const juce::String& id) const
{
    for (const auto& strip : strips)
        if (strip.id == id)
            return &strip;

    return nullptr;
}
