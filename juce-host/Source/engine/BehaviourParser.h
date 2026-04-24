#pragma once

#include <JuceHeader.h>

struct ParsedStepPattern
{
    bool success = false;
    juce::String diagnostic;
    juce::Array<int> steps;
};

struct ParsedScalarBlock
{
    bool success = false;
    juce::String diagnostic;
    juce::NamedValueSet values;
};

class BehaviourParser
{
public:
    static ParsedStepPattern parseBinarySteps(const juce::String& codeText);
    static ParsedScalarBlock parseScalarBlock(const juce::String& codeText);
    static double readScalar(const ParsedScalarBlock& block, const juce::String& key, double fallback);
};
