#pragma once

#include <JuceHeader.h>
#include <optional>

enum class MessageKind
{
    command,
    ack,
    event,
    error,
    query,
    response
};

struct MessageEnvelope
{
    juce::String protocolVersion;
    juce::String messageId;
    juce::String correlationId;
    juce::String timestamp;
    juce::String sender;
    juce::String recipient;
    MessageKind kind {};
    juce::String type;
    juce::var payload;

    juce::String toJsonLine() const;

    static juce::String kindToString(MessageKind kind);
    static std::optional<MessageKind> kindFromString(const juce::String& text);
    static std::optional<MessageEnvelope> fromJsonLine(const juce::String& line, juce::String& errorText);
};
