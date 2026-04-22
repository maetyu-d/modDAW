#pragma once

#include <JuceHeader.h>

struct ClockDomainEntry
{
    juce::String id;
    juce::String parentId;
    juce::String kind;
    juce::String displayName;
    double ratioToParent = 1.0;
    double phaseOffsetBeats = 0.0;
    double localBeatPosition = 0.0;
    int localBarIndex = 1;
    double localBeatInBar = 1.0;

    juce::String toSummaryString() const;
};

struct ClockDomainState
{
    juce::Array<ClockDomainEntry> domains;

    static ClockDomainState fromPayload(const juce::var& payload);
    juce::String toSummaryString() const;
    const ClockDomainEntry* findById(const juce::String& id) const;
};
