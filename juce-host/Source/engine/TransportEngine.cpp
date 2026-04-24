#include "TransportEngine.h"

namespace
{
double currentSeconds()
{
    return juce::Time::getMillisecondCounterHiRes() * 0.001;
}
}

TransportEngine::TransportEngine() = default;
TransportEngine::~TransportEngine() = default;

void TransportEngine::configure(double tempo, int meterNumerator, int meterDenominator)
{
    {
        const juce::ScopedLock scopedLock(lock);
        updateStateFromNow();
        state.tempo = juce::jmax(1.0, tempo);
        state.meterNumerator = juce::jmax(1, meterNumerator);
        state.meterDenominator = juce::jmax(1, meterDenominator);
    }

    emitStateChanged();
}

void TransportEngine::setStateCallback(StateCallback callback)
{
    const juce::ScopedLock scopedLock(lock);
    stateCallback = std::move(callback);
}

void TransportEngine::setState(const TransportState& newState)
{
    {
        const juce::ScopedLock scopedLock(lock);
        const auto wasPlaying = state.isPlaying;
        if (wasPlaying)
            stopTimer();

        state = newState;
        playStartBeatPosition = state.beatPosition;
        playStartSeconds = currentSeconds();

        if (state.isPlaying)
            startTimerHz(20);
    }

    emitStateChanged();
}

void TransportEngine::startPlayback()
{
    {
        const juce::ScopedLock scopedLock(lock);
        if (state.isPlaying)
            return;

        playStartBeatPosition = state.beatPosition;
        playStartSeconds = currentSeconds();
        state.isPlaying = true;
        startTimerHz(20);
    }

    emitStateChanged();
}

void TransportEngine::stopPlayback()
{
    {
        const juce::ScopedLock scopedLock(lock);
        updateStateFromNow();
        stopTimer();
        state.isPlaying = false;
        state.beatPosition = 0.0;
        state.barIndex = 1;
        state.beatInBar = 1.0;
        state.phaseWithinBar = 0.0;
        playStartBeatPosition = 0.0;
        playStartSeconds = 0.0;
    }

    emitStateChanged();
}

void TransportEngine::reset()
{
    {
        const juce::ScopedLock scopedLock(lock);
        const auto wasPlaying = state.isPlaying;
        if (wasPlaying)
            stopTimer();

        state = {};
        playStartBeatPosition = 0.0;
        playStartSeconds = 0.0;

        if (wasPlaying)
            startTimerHz(20);
    }

    emitStateChanged();
}

TransportState TransportEngine::getState() const
{
    const juce::ScopedLock scopedLock(lock);
    const_cast<TransportEngine*>(this)->updateStateFromNow();
    return state;
}

void TransportEngine::timerCallback()
{
    {
        const juce::ScopedLock scopedLock(lock);
        updateStateFromNow();
    }

    emitStateChanged();
}

void TransportEngine::updateStateFromNow()
{
    if (! state.isPlaying)
        return;

    const auto elapsedSeconds = juce::jmax(0.0, currentSeconds() - playStartSeconds);
    const auto beatsPerSecond = state.tempo / 60.0;
    state.beatPosition = playStartBeatPosition + (elapsedSeconds * beatsPerSecond);

    const auto beatsPerBar = static_cast<double>(juce::jmax(1, state.meterNumerator));
    const auto completedBars = static_cast<int>(std::floor(state.beatPosition / beatsPerBar));
    const auto beatRemainder = std::fmod(state.beatPosition, beatsPerBar);
    const auto wrappedBeat = beatRemainder < 0.0 ? beatRemainder + beatsPerBar : beatRemainder;

    state.barIndex = completedBars + 1;
    state.beatInBar = wrappedBeat + 1.0;
    state.phaseWithinBar = beatsPerBar > 0.0 ? (wrappedBeat / beatsPerBar) : 0.0;
}

void TransportEngine::emitStateChanged()
{
    StateCallback callbackCopy;
    TransportState stateCopy;

    {
        const juce::ScopedLock scopedLock(lock);
        stateCopy = state;
        callbackCopy = stateCallback;
    }

    if (callbackCopy)
        callbackCopy(stateCopy);
}
