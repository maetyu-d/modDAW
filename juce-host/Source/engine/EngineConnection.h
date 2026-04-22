#pragma once

#include <JuceHeader.h>
#include <functional>
#include "MessageEnvelope.h"
#include "MessageParser.h"

class EngineConnection
{
public:
    using EnvelopeCallback = std::function<void(const MessageEnvelope&)>;
    using ErrorCallback = std::function<void(const juce::String&)>;
    using LogCallback = std::function<void(const juce::String&)>;

    bool openSocket(int hostPort, int enginePort);
    void closeSocket();
    bool send(const MessageEnvelope& envelope);
    void poll();

    EnvelopeCallback onEnvelope;
    ErrorCallback onError;
    LogCallback onLog;

private:
    std::unique_ptr<juce::DatagramSocket> socket;
    int engineReceivePort = 0;
    MessageParser parser;

    void consumeInputChunk(const juce::String& inputChunk);
};
