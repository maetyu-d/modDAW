#pragma once

#include "../engine/Module.h"

class DroneModule final : public Module
{
public:
    DroneModule();

    ModuleEntry buildState() const override;
    juce::String moduleId() const override;
    juce::String clockDomainId() const override;
    void markActivationQueued(const juce::String& boundaryLabel, int targetBarIndex) override;
    void activateNow(const juce::String& boundaryLabel) override;
    bool loadState(const ModuleEntry& entry, juce::String& errorText) override;
    QueueResult queueSurfaceUpdate(const juce::String& surfaceId,
                                   const juce::String& codeText,
                                   const juce::String& boundaryLabel,
                                   int targetBarIndex) override;
    void applyQueuedSurfaceNow(const juce::String& surfaceId,
                               const juce::String& boundaryLabel) override;

private:
    struct Behaviour
    {
        double density = 0.25;
        double frequency = 82.41;
        double spread = 1.5;
    };

    QueueResult parseQueuedSurface(const juce::String& surfaceId,
                                   const juce::String& codeText,
                                   Behaviour& nextBehaviour,
                                   juce::String& diagnostic) const;
    void publishBehaviour();

    ModuleEntry state;
    Behaviour activeBehaviour;
    Behaviour pendingBehaviour;
};
