#include "AutomationState.h"

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

juce::String AutomationLaneEntry::toSummaryString() const
{
    return displayName + " -> " + targetId + "." + parameterId
         + " | " + juce::String(points.size()) + " points";
}

AutomationState AutomationState::fromPayload(const juce::var& payload)
{
    AutomationState state;

    if (auto* dynamicObject = payload.getDynamicObject())
    {
        if (auto* lanesArray = dynamicObject->getProperty("lanes").getArray())
        {
            for (const auto& item : *lanesArray)
            {
                AutomationLaneEntry lane;
                lane.laneId = readString(item, "laneId");
                lane.displayName = readString(item, "displayName");
                lane.targetType = readString(item, "targetType");
                lane.targetId = readString(item, "targetId");
                lane.parameterId = readString(item, "parameterId");
                lane.interpolation = readString(item, "interpolation");
                lane.currentValue = readDouble(item, "currentValue", 0.0);

                if (auto* laneObject = item.getDynamicObject())
                {
                    if (auto* pointsArray = laneObject->getProperty("points").getArray())
                    {
                        for (const auto& pointItem : *pointsArray)
                        {
                            AutomationPointEntry point;
                            point.beat = readDouble(pointItem, "beat", 0.0);
                            point.value = readDouble(pointItem, "value", 0.0);
                            lane.points.add(point);
                        }
                    }
                }

                state.lanes.add(lane);
            }
        }
    }

    return state;
}

juce::String AutomationState::toSummaryString() const
{
    return juce::String(lanes.size()) + " automation lanes";
}

const AutomationLaneEntry* AutomationState::firstLane() const
{
    return lanes.isEmpty() ? nullptr : &lanes.getReference(0);
}
