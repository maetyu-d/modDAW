#pragma once

#include <JuceHeader.h>

struct RegionEntry
{
    juce::String regionId;
    juce::String moduleId;
    juce::String displayName;
    juce::String kind;
    juce::String source;
    double startBeat = 0.0;
    double lengthBeats = 0.0;

    juce::String toSummaryString() const;
};

struct RegionState
{
    juce::Array<RegionEntry> regions;

    static RegionState fromPayload(const juce::var& payload);
    juce::String toSummaryString() const;
    juce::Array<RegionEntry> regionsForModule(const juce::String& moduleId) const;
};
