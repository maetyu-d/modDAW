#pragma once

#include <JuceHeader.h>
#include "MixerState.h"

class MixerEngine
{
public:
    using StateCallback = std::function<void(const MixerState&)>;

    MixerEngine() = default;

    void setStateCallback(StateCallback callback);
    void initialiseState(const MixerState& initialState);
    MixerState getState() const;

    bool setStripLevel(const juce::String& stripId, double level);
    bool setStripMuted(const juce::String& stripId, bool muted);
    bool setStripGroup(const juce::String& stripId, const juce::String& groupId);
    bool setSendLevel(const juce::String& sendId, double level);
    bool setSendMode(const juce::String& sendId, const juce::String& mode);

    double computeModuleGain(const juce::String& moduleId) const;
    bool hasAudioPath(const juce::String& moduleId) const;

private:
    void emitStateChanged();

    mutable juce::CriticalSection lock;
    MixerState state;
    StateCallback stateCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerEngine)
};
