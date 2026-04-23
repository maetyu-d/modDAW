#pragma once

#include <JuceHeader.h>

struct StructuralDirectiveEntry
{
    juce::String directiveId;
    juce::String emitterModuleId;
    juce::String section;
    juce::String syncCue;
    juce::String quantizationTarget;
    juce::String orchestration;
    juce::StringArray targetModuleIds;
    double beat = 0.0;
    double scheduledBeat = 0.0;
    int phraseIndex = 0;
    double densityTarget = 0.0;
    bool phraseReset = false;

    juce::String toSummaryString() const;
};

struct StructuralPendingTransitionEntry
{
    juce::String transitionId;
    juce::String domainId;
    juce::String sceneName;
    juce::String quantizationTarget;
    juce::String state;
    int afterCycles = 0;
    double requestedBeat = 0.0;
    double targetBeat = -1.0;

    juce::String toSummaryString() const;
};

struct StructuralState
{
    juce::Array<StructuralDirectiveEntry> directives;
    juce::Array<StructuralPendingTransitionEntry> pendingTransitions;
    juce::String currentScene;

    static StructuralState fromPayload(const juce::var& payload);
    juce::String toSummaryString() const;
    const StructuralDirectiveEntry* latestDirective() const;
    const StructuralPendingTransitionEntry* nextPendingTransition() const;
};
