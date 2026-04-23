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
                strip.assignedGroupId = readString(item, "assignedGroupId");
                strip.childCount = static_cast<int>(readDouble(item, "childCount", 0.0));
                strip.hasAudioPath = readBool(item, "hasAudioPath", false);
                strip.level = readDouble(item, "level", 1.0);
                strip.muted = readBool(item, "muted", false);
                state.strips.add(strip);
            }
        }

        if (auto* groupsArray = dynamicObject->getProperty("groups").getArray())
        {
            for (const auto& item : *groupsArray)
            {
                MixerGroupEntry group;
                group.id = readString(item, "id");
                group.displayName = readString(item, "displayName");
                state.groups.add(group);
            }
        }

        if (auto* sendsArray = dynamicObject->getProperty("sends").getArray())
        {
            for (const auto& item : *sendsArray)
            {
                MixerSendEntry send;
                send.sendId = readString(item, "sendId");
                send.displayName = readString(item, "displayName");
                send.sourceStripId = readString(item, "sourceStripId");
                send.targetModuleId = readString(item, "targetModuleId");
                send.returnStripId = readString(item, "returnStripId");
                send.mode = readString(item, "mode");
                send.level = readDouble(item, "level", 0.0);
                state.sends.add(send);
            }
        }
    }

    return state;
}

juce::String MixerState::toSummaryString() const
{
    return juce::String(strips.size()) + " mixer strips, "
         + juce::String(groups.size()) + " groups, "
         + juce::String(sends.size()) + " sends";
}

const MixerStripEntry* MixerState::findById(const juce::String& id) const
{
    for (const auto& strip : strips)
        if (strip.id == id)
            return &strip;

    return nullptr;
}

const MixerGroupEntry* MixerState::findGroupById(const juce::String& id) const
{
    for (const auto& group : groups)
        if (group.id == id)
            return &group;

    return nullptr;
}

const MixerSendEntry* MixerState::findSendForStrip(const juce::String& stripId) const
{
    for (const auto& send : sends)
        if (send.sourceStripId == stripId)
            return &send;

    return nullptr;
}
