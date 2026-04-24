#pragma once

#include "../engine/Module.h"

class PatternModule final : public Module
{
public:
    PatternModule();

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
        juce::Array<int> steps { 1, 0, 1, 0, 0, 1, 0 };
        double frequency = 440.0;
        double accent = 0.6;
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
