#pragma once

#include <JuceHeader.h>

class SubprocessHandle
{
public:
    SubprocessHandle() = default;
    ~SubprocessHandle();

    bool start(const juce::StringArray& arguments);
    bool isRunning() const;
    int readProcessOutput(void* destBuffer, int numBytesToRead);
    bool writeToProcess(const void* data, int numBytes);
    bool kill();

private:
   #if JUCE_WINDOWS
    std::unique_ptr<juce::ChildProcess> fallbackProcess;
   #else
    pid_t childPid = -1;
    int stdinFd = -1;
    int stdoutFd = -1;
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SubprocessHandle)
};
