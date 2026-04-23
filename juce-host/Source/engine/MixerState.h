#pragma once

#include <JuceHeader.h>

struct MixerStripEntry
{
    juce::String id;
    juce::String displayName;
    juce::String kind;
    juce::String targetModuleId;
    juce::String assignedGroupId;
    int childCount = 0;
    bool hasAudioPath = false;
    double level = 1.0;
    bool muted = false;

    juce::String toSummaryString() const;
};

struct MixerGroupEntry
{
    juce::String id;
    juce::String displayName;
};

struct MixerSendEntry
{
    juce::String sendId;
    juce::String displayName;
    juce::String sourceStripId;
    juce::String targetModuleId;
    juce::String returnStripId;
    juce::String mode;
    double level = 0.0;
};

struct MixerState
{
    juce::Array<MixerStripEntry> strips;
    juce::Array<MixerGroupEntry> groups;
    juce::Array<MixerSendEntry> sends;

    static MixerState fromPayload(const juce::var& payload);
    juce::String toSummaryString() const;
    const MixerStripEntry* findById(const juce::String& id) const;
    const MixerGroupEntry* findGroupById(const juce::String& id) const;
    const MixerSendEntry* findSendForStrip(const juce::String& stripId) const;
};
