#pragma once

#include <JuceHeader.h>

struct CodeSurfaceEntry
{
    juce::String surfaceId;
    juce::String displayName;
    juce::String role;
    juce::String code;
    juce::String pendingCode;
    juce::String state;
    juce::String diagnostic;
    int revision = 1;
    int pendingCodeSwapBarIndex = 0;

    juce::String currentEditableCode() const;
};

struct ModuleEntry
{
    juce::String id;
    juce::String displayName;
    juce::String clockDomainId;
    juce::String timingMode;
    juce::String lifecycleState;
    juce::String behaviourType;
    juce::String laneType;
    juce::StringArray capabilities;
    juce::String lastStructuralDirective;
    juce::String analysisSummary;
    juce::String lastAnalysisInfluence;
    juce::String codeSurface;
    juce::String pendingCodeSurface;
    juce::String codeSurfaceState;
    juce::String lastCodeEvalMessage;
    int pendingActivationBarIndex = 0;
    int pendingCodeSwapBarIndex = 0;
    juce::Array<int> stepPattern;
    double baseFrequency = 0.0;
    double accent = 1.0;
    double density = 1.0;
    double spread = 1.0;
    juce::Array<CodeSurfaceEntry> codeSurfaces;

    juce::String toSummaryString() const;
    juce::String currentEditableCodeSurface() const;
    const CodeSurfaceEntry* findSurfaceById(const juce::String& surfaceId) const;
    const CodeSurfaceEntry* defaultSurface() const;
};

struct ModuleState
{
    juce::Array<ModuleEntry> modules;

    static ModuleState fromPayload(const juce::var& payload);
    juce::String toSummaryString() const;
    const ModuleEntry* findById(const juce::String& id) const;
};
