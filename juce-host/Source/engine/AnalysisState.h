#pragma once

#include <JuceHeader.h>

struct AnalysisModuleEntry
{
    juce::String moduleId;
    juce::String sourceModuleId;
    juce::String routing;
    juce::StringArray targetModuleIds;
    double envelope = 0.0;
    bool onset = false;
    double density = 0.0;
    double brightness = 0.0;
    double lastBeat = 0.0;

    juce::String toSummaryString() const;
};

struct AnalysisState
{
    juce::Array<AnalysisModuleEntry> modules;

    static AnalysisState fromPayload(const juce::var& payload);
    juce::String toSummaryString() const;
    const AnalysisModuleEntry* firstModule() const;
};
