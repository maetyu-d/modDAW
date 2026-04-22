#include "App.h"

const juce::String ModularScDawApplication::getApplicationName()
{
    return "Modular SC DAW";
}

const juce::String ModularScDawApplication::getApplicationVersion()
{
    return "0.1.0-m1";
}

bool ModularScDawApplication::moreThanOneInstanceAllowed()
{
    return true;
}

void ModularScDawApplication::initialise(const juce::String&)
{
    mainWindow = std::make_unique<MainWindow>(getApplicationName());
}

void ModularScDawApplication::shutdown()
{
    mainWindow.reset();
}

void ModularScDawApplication::systemRequestedQuit()
{
    quit();
}

void ModularScDawApplication::anotherInstanceStarted(const juce::String&)
{
}
