#pragma once

#include <JuceHeader.h>

struct RenderArtifact
{
    juce::String stemId;
    juce::String path;
    juce::String artifactKind;
    int eventCount = 0;
};

struct RenderState
{
    juce::String status = "idle";
    juce::String lastRenderId;
    juce::String outputDirectory;
    juce::Array<RenderArtifact> artifacts;

    static RenderState fromPayload(const juce::var& payload);
    juce::String toSummaryString() const;
};
