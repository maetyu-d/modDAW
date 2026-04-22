#include "SubprocessHandle.h"

#if ! JUCE_WINDOWS
 #include <errno.h>
 #include <fcntl.h>
 #include <signal.h>
 #include <spawn.h>
 #include <sys/types.h>
 #include <sys/wait.h>
 #include <unistd.h>
 extern char** environ;
#endif

SubprocessHandle::~SubprocessHandle()
{
    kill();
}

bool SubprocessHandle::start(const juce::StringArray& arguments)
{
   #if JUCE_WINDOWS
    fallbackProcess = std::make_unique<juce::ChildProcess>();
    return fallbackProcess->start(arguments, juce::ChildProcess::wantStdOut | juce::ChildProcess::wantStdErr);
   #else
    kill();

    if (arguments.isEmpty())
        return false;

    int stdinPipe[2] { -1, -1 };
    int stdoutPipe[2] { -1, -1 };

    if (pipe(stdinPipe) != 0)
        return false;

    if (pipe(stdoutPipe) != 0)
    {
        ::close(stdinPipe[0]);
        ::close(stdinPipe[1]);
        return false;
    }

    posix_spawn_file_actions_t fileActions;
    posix_spawn_file_actions_init(&fileActions);
    posix_spawn_file_actions_adddup2(&fileActions, stdinPipe[0], STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&fileActions, stdoutPipe[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&fileActions, stdoutPipe[1], STDERR_FILENO);
    posix_spawn_file_actions_addclose(&fileActions, stdinPipe[1]);
    posix_spawn_file_actions_addclose(&fileActions, stdoutPipe[0]);

    std::vector<std::string> utf8Arguments;
    std::vector<char*> argv;
    utf8Arguments.reserve(static_cast<size_t>(arguments.size()));
    argv.reserve(static_cast<size_t>(arguments.size() + 1));

    for (const auto& argument : arguments)
        utf8Arguments.emplace_back(argument.toStdString());

    for (auto& argument : utf8Arguments)
        argv.push_back(argument.data());

    argv.push_back(nullptr);

    pid_t pid = -1;
    const auto result = posix_spawnp(&pid, argv.front(), &fileActions, nullptr, argv.data(), environ);
    posix_spawn_file_actions_destroy(&fileActions);

    ::close(stdinPipe[0]);
    ::close(stdoutPipe[1]);

    if (result != 0)
    {
        ::close(stdinPipe[1]);
        ::close(stdoutPipe[0]);
        return false;
    }

    stdinFd = stdinPipe[1];
    stdoutFd = stdoutPipe[0];
    childPid = pid;

    fcntl(stdoutFd, F_SETFL, fcntl(stdoutFd, F_GETFL, 0) | O_NONBLOCK);

    return true;
   #endif
}

bool SubprocessHandle::isRunning() const
{
   #if JUCE_WINDOWS
    return fallbackProcess != nullptr && fallbackProcess->isRunning();
   #else
    if (childPid <= 0)
        return false;

    int status = 0;
    const auto waitResult = waitpid(childPid, &status, WNOHANG);
    return waitResult == 0;
   #endif
}

int SubprocessHandle::readProcessOutput(void* destBuffer, int numBytesToRead)
{
   #if JUCE_WINDOWS
    return fallbackProcess != nullptr ? fallbackProcess->readProcessOutput(destBuffer, numBytesToRead) : 0;
   #else
    if (stdoutFd < 0)
        return 0;

    const auto bytesRead = ::read(stdoutFd, destBuffer, static_cast<size_t>(numBytesToRead));

    if (bytesRead > 0)
        return static_cast<int>(bytesRead);

    if (bytesRead < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return 0;

    return 0;
   #endif
}

bool SubprocessHandle::writeToProcess(const void* data, int numBytes)
{
   #if JUCE_WINDOWS
    juce::ignoreUnused(data, numBytes);
    return false;
   #else
    if (stdinFd < 0 || data == nullptr || numBytes <= 0)
        return false;

    const auto bytesWritten = ::write(stdinFd, data, static_cast<size_t>(numBytes));
    return bytesWritten == numBytes;
   #endif
}

bool SubprocessHandle::kill()
{
   #if JUCE_WINDOWS
    if (fallbackProcess == nullptr)
        return true;

    const auto ok = fallbackProcess->kill();
    fallbackProcess.reset();
    return ok;
   #else
    if (stdinFd >= 0)
    {
        ::close(stdinFd);
        stdinFd = -1;
    }

    if (stdoutFd >= 0)
    {
        ::close(stdoutFd);
        stdoutFd = -1;
    }

    if (childPid <= 0)
        return true;

    ::kill(childPid, SIGTERM);
    waitpid(childPid, nullptr, 0);
    childPid = -1;
    return true;
   #endif
}
