#pragma once

#include <JuceHeader.h>

namespace moddaw::time
{
inline juce::String makeTimestampUtc()
{
    return juce::Time::getCurrentTime().toISO8601(true);
}

inline juce::String makeMessageId(const juce::String& prefix)
{
    return prefix + "-" + juce::Uuid().toString();
}
} // namespace moddaw::time
