#pragma once

#include <JuceHeader.h>
#include "ModuleState.h"
#include "TransportState.h"

class Module
{
public:
    struct QueueResult
    {
        bool success = false;
        juce::String diagnostic;
    };

    Module() = default;
    virtual ~Module() = default;

    virtual ModuleEntry buildState() const = 0;
    virtual juce::String moduleId() const = 0;
    virtual juce::String clockDomainId() const = 0;
    virtual void markActivationQueued(const juce::String& boundaryLabel, int targetBarIndex) = 0;
    virtual void activateNow(const juce::String& boundaryLabel) = 0;
    virtual bool loadState(const ModuleEntry& entry, juce::String& errorText) = 0;
    virtual QueueResult queueSurfaceUpdate(const juce::String& surfaceId,
                                           const juce::String& codeText,
                                           const juce::String& boundaryLabel,
                                           int targetBarIndex) = 0;
    virtual void applyQueuedSurfaceNow(const juce::String& surfaceId,
                                       const juce::String& boundaryLabel) = 0;
};
