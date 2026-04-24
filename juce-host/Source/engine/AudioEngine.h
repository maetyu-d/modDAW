#pragma once

#include <JuceHeader.h>
#include "MixerState.h"
#include "ModuleState.h"
#include "RouteState.h"
#include "TransportState.h"

class AudioEngine final : private juce::AudioIODeviceCallback
{
public:
    struct Status
    {
        bool ready = false;
        juce::String detail = "offline";
        double sampleRate = 0.0;
        int outputChannels = 0;
    };

    AudioEngine();
    ~AudioEngine() override;

    void setTransportState(const TransportState& state);
    void setModuleState(const ModuleState& state);
    void setMixerState(const MixerState& state);
    void setRouteState(const RouteState& state);
    Status getStatus() const;

private:
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

    static bool isPlayableLifecycle(const juce::String& lifecycle);
    static double advanceWrappedPhase(double phase, double frequency, double sampleRate);

    mutable juce::CriticalSection lock;
    juce::AudioDeviceManager deviceManager;
    TransportState transportState;
    ModuleState moduleState;
    MixerState mixerState;
    RouteState routeState;
    Status status;
    double sampleRate = 44100.0;
    double tonePhase = 0.0;
    double patternPhase = 0.0;
    double dronePhase = 0.0;
    double dronePhaseTwo = 0.0;
    float toneEnvelope = 0.0f;
    float patternEnvelope = 0.0f;
    int lastTriggeredBeatIndex = -1;
    int lastPatternStepIndex = -1;
    bool hasEverPlayed = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
