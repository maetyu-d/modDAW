#include "TransportState.h"

namespace
{
double readDouble(const juce::NamedValueSet& values, const juce::Identifier& id, double fallback)
{
    if (auto* value = values.getVarPointer(id))
        return static_cast<double>(*value);

    return fallback;
}

int readInt(const juce::NamedValueSet& values, const juce::Identifier& id, int fallback)
{
    if (auto* value = values.getVarPointer(id))
        return static_cast<int>(*value);

    return fallback;
}

bool readBool(const juce::NamedValueSet& values, const juce::Identifier& id, bool fallback)
{
    if (auto* value = values.getVarPointer(id))
        return static_cast<bool>(*value);

    return fallback;
}
}

TransportState TransportState::fromPayload(const juce::var& payload)
{
    TransportState state;
    if (auto* object = payload.getDynamicObject())
    {
        const auto& values = object->getProperties();
        state.isPlaying = readBool(values, "isPlaying", state.isPlaying);
        state.tempo = readDouble(values, "tempo", state.tempo);
        state.meterNumerator = readInt(values, "meterNumerator", state.meterNumerator);
        state.meterDenominator = readInt(values, "meterDenominator", state.meterDenominator);
        state.beatPosition = readDouble(values, "beatPosition", state.beatPosition);
        state.barIndex = readInt(values, "barIndex", state.barIndex);
        state.beatInBar = readDouble(values, "beatInBar", state.beatInBar);
        state.phaseWithinBar = readDouble(values, "phaseWithinBar", state.phaseWithinBar);
    }

    return state;
}

juce::String TransportState::toSummaryString() const
{
    return juce::String(isPlaying ? "playing" : "stopped")
        + " | "
        + juce::String(tempo, 2) + " BPM"
        + " | "
        + juce::String(meterNumerator) + "/" + juce::String(meterDenominator)
        + " | bar "
        + juce::String(barIndex)
        + " beat "
        + juce::String(beatInBar, 2);
}
