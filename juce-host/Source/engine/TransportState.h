#pragma once

#include <JuceHeader.h>

struct TransportState
{
    bool isPlaying = false;
    double tempo = 120.0;
    int meterNumerator = 4;
    int meterDenominator = 4;
    double beatPosition = 0.0;
    int barIndex = 1;
    double beatInBar = 1.0;
    double phaseWithinBar = 0.0;

    static TransportState fromPayload(const juce::var& payload);
    juce::String toSummaryString() const;
};
