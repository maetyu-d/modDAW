#pragma once

#include <JuceHeader.h>

struct ModuleEntry
{
    juce::String id;
    juce::String displayName;
    juce::String clockDomainId;
    juce::String lifecycleState;
    juce::String behaviourType;
    juce::String codeSurface;
    juce::String pendingCodeSurface;
    juce::String codeSurfaceState;
    juce::String lastCodeEvalMessage;
    int pendingCodeSwapBarIndex = 0;

    juce::String toSummaryString() const;
    juce::String currentEditableCodeSurface() const;
};

struct ModuleState
{
    juce::Array<ModuleEntry> modules;

    static ModuleState fromPayload(const juce::var& payload);
    juce::String toSummaryString() const;
    const ModuleEntry* findById(const juce::String& id) const;
};
