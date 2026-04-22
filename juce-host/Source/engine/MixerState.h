#pragma once

#include <JuceHeader.h>

struct MixerStripEntry
{
    juce::String id;
    juce::String displayName;
    juce::String kind;
    juce::String targetModuleId;
    bool hasAudioPath = false;
    double level = 1.0;
    bool muted = false;

    juce::String toSummaryString() const;
};

struct MixerState
{
    juce::Array<MixerStripEntry> strips;

    static MixerState fromPayload(const juce::var& payload);
    juce::String toSummaryString() const;
    const MixerStripEntry* findById(const juce::String& id) const;
};
