#include "EngineConnection.h"
#if JUCE_MAC || JUCE_LINUX || JUCE_BSD
 #include <fcntl.h>
#endif

bool EngineConnection::openSocket(int hostPort, int enginePort)
{
    closeSocket();

    auto newSocket = std::make_unique<juce::DatagramSocket>();

    if (! newSocket->bindToPort(hostPort, "127.0.0.1"))
    {
        if (onError) onError("Failed to bind host UDP socket to port " + juce::String(hostPort));
        return false;
    }

#if JUCE_MAC || JUCE_LINUX || JUCE_BSD
    // Prevent the host socket from leaking into sclang/scsynth across exec.
    if (auto rawSocket = newSocket->getRawSocketHandle(); rawSocket >= 0)
        fcntl(rawSocket, F_SETFD, fcntl(rawSocket, F_GETFD) | FD_CLOEXEC);
#endif

    engineReceivePort = enginePort;
    socket = std::move(newSocket);
    return true;
}

void EngineConnection::closeSocket()
{
    if (socket != nullptr)
        socket->shutdown();

    socket.reset();
    engineReceivePort = 0;
}

bool EngineConnection::send(const MessageEnvelope& envelope)
{
    if (socket == nullptr || engineReceivePort <= 0)
    {
        if (onError) onError("Cannot send message: engine UDP socket is not ready");
        return false;
    }

    auto jsonLine = envelope.toJsonLine();
    auto utf8 = jsonLine.toRawUTF8();
    auto bytesToWrite = static_cast<int>(std::strlen(utf8));

    if (onLog)
        onLog("TX " + envelope.type + " " + jsonLine.trimEnd());

    if (socket->write("127.0.0.1", engineReceivePort, utf8, bytesToWrite) != bytesToWrite)
    {
        if (onError) onError("Failed to write message to engine UDP socket");
        return false;
    }

    return true;
}

void EngineConnection::poll()
{
    if (socket == nullptr)
        return;

    while (socket->waitUntilReady(true, 0) > 0)
    {
        char buffer[8192] {};
        juce::String senderIp;
        int senderPort = 0;
        auto bytesRead = socket->read(buffer, static_cast<int>(sizeof(buffer) - 1), false, senderIp, senderPort);

        if (bytesRead <= 0)
            break;

        consumeInputChunk(juce::String::fromUTF8(buffer, bytesRead));
    }
}

void EngineConnection::consumeInputChunk(const juce::String& inputChunk)
{
    juce::StringArray errors;
    auto messages = parser.pushChunk(inputChunk, errors);

    for (const auto& error : errors)
        if (onError) onError("Protocol parse error: " + error);

    for (const auto& message : messages)
    {
        if (onLog)
            onLog("RX " + message.type + " " + message.toJsonLine().trimEnd());

        if (onEnvelope)
            onEnvelope(message);
    }
}
