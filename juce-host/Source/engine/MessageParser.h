#pragma once

#include <JuceHeader.h>
#include "MessageEnvelope.h"

class MessageParser
{
public:
    juce::Array<MessageEnvelope> pushChunk(const juce::String& chunk, juce::StringArray& errors);

private:
    juce::String pendingBuffer;
};
