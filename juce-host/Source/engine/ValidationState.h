#pragma once

#include <JuceHeader.h>

struct ValidationFinding
{
    juce::String severity;
    juce::String ruleId;
    juce::String message;

    juce::String toSummaryString() const;
};

struct ValidationState
{
    bool isValid = true;
    int findingCount = 0;
    juce::Array<ValidationFinding> findings;

    static ValidationState fromPayload(const juce::var& payload);
    juce::String toSummaryString() const;
};
