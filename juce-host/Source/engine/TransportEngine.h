#pragma once

#include <JuceHeader.h>
#include "TransportState.h"

class TransportEngine final : private juce::Timer
{
public:
    using StateCallback = std::function<void(const TransportState&)>;

    TransportEngine();
    ~TransportEngine() override;

    void configure(double tempo, int meterNumerator, int meterDenominator);
    void setStateCallback(StateCallback callback);
    void setState(const TransportState& newState);

    void startPlayback();
    void stopPlayback();
    void reset();

    TransportState getState() const;

private:
    void timerCallback() override;
    void updateStateFromNow();
    void emitStateChanged();

    mutable juce::CriticalSection lock;
    TransportState state;
    double playStartSeconds = 0.0;
    double playStartBeatPosition = 0.0;
    StateCallback stateCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportEngine)
};
