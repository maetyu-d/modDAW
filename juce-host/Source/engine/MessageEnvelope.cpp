#include "MessageEnvelope.h"

namespace
{
juce::var objectFromEnvelope(const MessageEnvelope& envelope)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("protocolVersion", envelope.protocolVersion);
    object->setProperty("messageId", envelope.messageId);

    if (envelope.correlationId.isNotEmpty())
        object->setProperty("correlationId", envelope.correlationId);

    object->setProperty("timestamp", envelope.timestamp);
    object->setProperty("sender", envelope.sender);
    object->setProperty("recipient", envelope.recipient);
    object->setProperty("kind", MessageEnvelope::kindToString(envelope.kind));
    object->setProperty("type", envelope.type);
    object->setProperty("payload", envelope.payload);
    return juce::var(object);
}
}

juce::String MessageEnvelope::toJsonLine() const
{
    return juce::JSON::toString(objectFromEnvelope(*this), false) + "\n";
}

juce::String MessageEnvelope::kindToString(MessageKind kind)
{
    switch (kind)
    {
        case MessageKind::command: return "command";
        case MessageKind::ack: return "ack";
        case MessageKind::event: return "event";
        case MessageKind::error: return "error";
        case MessageKind::query: return "query";
        case MessageKind::response: return "response";
    }

    jassertfalse;
    return "error";
}

std::optional<MessageKind> MessageEnvelope::kindFromString(const juce::String& text)
{
    if (text == "command") return MessageKind::command;
    if (text == "ack") return MessageKind::ack;
    if (text == "event") return MessageKind::event;
    if (text == "error") return MessageKind::error;
    if (text == "query") return MessageKind::query;
    if (text == "response") return MessageKind::response;
    return std::nullopt;
}

std::optional<MessageEnvelope> MessageEnvelope::fromJsonLine(const juce::String& line, juce::String& errorText)
{
    auto parsed = juce::JSON::parse(line);
    auto* object = parsed.getDynamicObject();

    if (object == nullptr)
    {
        errorText = "Parsed JSON is not an object";
        return std::nullopt;
    }

    auto kindString = object->getProperty("kind").toString();
    auto kindValue = kindFromString(kindString);

    if (! kindValue.has_value())
    {
        errorText = "Unknown message kind: " + kindString;
        return std::nullopt;
    }

    MessageEnvelope envelope;
    envelope.protocolVersion = object->getProperty("protocolVersion").toString();
    envelope.messageId = object->getProperty("messageId").toString();
    envelope.correlationId = object->getProperty("correlationId").toString();
    envelope.timestamp = object->getProperty("timestamp").toString();
    envelope.sender = object->getProperty("sender").toString();
    envelope.recipient = object->getProperty("recipient").toString();
    envelope.kind = *kindValue;
    envelope.type = object->getProperty("type").toString();
    envelope.payload = object->getProperty("payload");

    if (envelope.protocolVersion.isEmpty() || envelope.messageId.isEmpty() || envelope.type.isEmpty())
    {
        errorText = "Missing required envelope fields";
        return std::nullopt;
    }

    return envelope;
}
