#include "MessageParser.h"

juce::Array<MessageEnvelope> MessageParser::pushChunk(const juce::String& chunk, juce::StringArray& errors)
{
    juce::Array<MessageEnvelope> messages;
    pendingBuffer += chunk;

    while (true)
    {
        auto newlineIndex = pendingBuffer.indexOfChar('\n');

        if (newlineIndex < 0)
            break;

        auto line = pendingBuffer.substring(0, newlineIndex).trim();
        pendingBuffer = pendingBuffer.substring(newlineIndex + 1);

        if (line.isEmpty())
            continue;

        juce::String errorText;
        auto envelope = MessageEnvelope::fromJsonLine(line, errorText);

        if (envelope.has_value())
            messages.add(*envelope);
        else
            errors.add(errorText + " | line=" + line);
    }

    return messages;
}
