#pragma once

#include <JuceHeader.h>

struct AutomationPointEntry
{
    double beat = 0.0;
    double value = 0.0;
};

struct AutomationLaneEntry
{
    juce::String laneId;
    juce::String displayName;
    juce::String targetType;
    juce::String targetId;
    juce::String parameterId;
    juce::String interpolation;
    double currentValue = 0.0;
    juce::Array<AutomationPointEntry> points;

    juce::String toSummaryString() const;
};

struct AutomationState
{
    juce::Array<AutomationLaneEntry> lanes;

    static AutomationState fromPayload(const juce::var& payload);
    juce::String toSummaryString() const;
    const AutomationLaneEntry* firstLane() const;
};
