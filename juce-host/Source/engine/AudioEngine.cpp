#include "AudioEngine.h"

namespace
{
constexpr double twoPi = juce::MathConstants<double>::twoPi;
}

AudioEngine::AudioEngine()
{
    auto error = deviceManager.initialise(0, 2, nullptr, true);
    if (error.isEmpty())
    {
        deviceManager.addAudioCallback(this);
        status.ready = true;
        status.detail = "audio ready";
        if (auto* device = deviceManager.getCurrentAudioDevice())
        {
            status.sampleRate = device->getCurrentSampleRate();
            status.outputChannels = device->getActiveOutputChannels().countNumberOfSetBits();
        }
    }
    else
    {
        status.ready = false;
        status.detail = error;
    }
}

AudioEngine::~AudioEngine()
{
    deviceManager.removeAudioCallback(this);
}

void AudioEngine::setTransportState(const TransportState& state)
{
    const juce::ScopedLock scopedLock(lock);
    transportState = state;
    if (transportState.isPlaying)
        hasEverPlayed = true;

    if (! transportState.isPlaying)
    {
        toneEnvelope *= 0.6f;
        lastTriggeredBeatIndex = -1;
    }
}

void AudioEngine::setModuleState(const ModuleState& state)
{
    const juce::ScopedLock scopedLock(lock);
    moduleState = state;
}

void AudioEngine::setMixerState(const MixerState& state)
{
    const juce::ScopedLock scopedLock(lock);
    mixerState = state;
}

void AudioEngine::setRouteState(const RouteState& state)
{
    const juce::ScopedLock scopedLock(lock);
    routeState = state;
}

AudioEngine::Status AudioEngine::getStatus() const
{
    const juce::ScopedLock scopedLock(lock);
    return status;
}

void AudioEngine::audioDeviceIOCallbackWithContext(const float* const*,
                                                   int,
                                                   float* const* outputChannelData,
                                                   int numOutputChannels,
                                                   int numSamples,
                                                   const juce::AudioIODeviceCallbackContext&)
{
    for (int channel = 0; channel < numOutputChannels; ++channel)
        if (auto* channelData = outputChannelData[channel])
            juce::FloatVectorOperations::clear(channelData, numSamples);

    TransportState transportCopy;
    ModuleState moduleCopy;
    MixerState mixerCopy;
    RouteState routeCopy;

    {
        const juce::ScopedLock scopedLock(lock);
        transportCopy = transportState;
        moduleCopy = moduleState;
        mixerCopy = mixerState;
        routeCopy = routeState;
    }

    if (! status.ready || numOutputChannels <= 0)
        return;

    const auto* toneModule = moduleCopy.findById("module.tone");
    const auto* droneModule = moduleCopy.findById("module.drone");
    auto hasAudioRoute = [&routeCopy](const juce::String& moduleId)
    {
        const auto sourceId = moduleId + ".audio.out";
        const auto prefixedId = "module." + moduleId.fromLastOccurrenceOf(".", false, false) + ".audio.out";
        for (const auto& route : routeCopy.routes)
        {
            if (! route.enabled || route.family != "audio")
                continue;
            if (route.source == sourceId || route.source == prefixedId || route.source.startsWith(moduleId + ".audio.out"))
                return true;
        }
        return false;
    };

    auto mixerGainForModule = [&mixerCopy](const juce::String& moduleId)
    {
        const MixerStripEntry* moduleStrip = nullptr;
        const MixerStripEntry* masterStrip = nullptr;
        const MixerStripEntry* groupStrip = nullptr;

        for (const auto& strip : mixerCopy.strips)
        {
            if (strip.kind == "master")
                masterStrip = &strip;
            if (strip.targetModuleId == moduleId)
                moduleStrip = &strip;
        }

        if (moduleStrip == nullptr || masterStrip == nullptr || moduleStrip->muted || masterStrip->muted)
            return 0.0f;

        if (moduleStrip->assignedGroupId.isNotEmpty())
            groupStrip = mixerCopy.findById(moduleStrip->assignedGroupId);

        if (groupStrip != nullptr && groupStrip->muted)
            return 0.0f;

        auto gain = static_cast<float>(moduleStrip->level * masterStrip->level);
        if (groupStrip != nullptr)
            gain *= static_cast<float>(groupStrip->level);
        return gain;
    };

    const auto* patternModule = moduleCopy.findById("module.pattern");
    const bool toneEnabled = toneModule != nullptr && isPlayableLifecycle(toneModule->lifecycleState) && hasAudioRoute("module.tone");
    const bool patternEnabled = patternModule != nullptr
        && isPlayableLifecycle(patternModule->lifecycleState)
        && hasAudioRoute("module.pattern");
    const bool droneEnabled = droneModule != nullptr && isPlayableLifecycle(droneModule->lifecycleState) && hasEverPlayed && hasAudioRoute("module.drone");
    const auto toneGain = mixerGainForModule("module.tone");
    const auto patternGain = mixerGainForModule("module.pattern");
    const auto droneGain = mixerGainForModule("module.drone");

    auto beatPosition = transportCopy.beatPosition;
    const auto beatIncrement = transportCopy.isPlaying ? ((transportCopy.tempo / 60.0) / sampleRate) : 0.0;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float mixedSample = 0.0f;

        if (toneEnabled && transportCopy.isPlaying)
        {
            const auto beatIndex = static_cast<int>(std::floor(beatPosition));
            const auto& steps = toneModule->stepPattern;
            const bool toneGate = steps.isEmpty() || steps[beatIndex % steps.size()] > 0;
            if (beatIndex != lastTriggeredBeatIndex)
            {
                lastTriggeredBeatIndex = beatIndex;
                toneEnvelope = toneGate ? static_cast<float>(toneModule->accent) : 0.0f;
            }

            const auto toneValue = static_cast<float>(std::sin(tonePhase)) * toneEnvelope * 0.18f * toneGain;
            mixedSample += toneValue;
            tonePhase = advanceWrappedPhase(tonePhase,
                                            juce::jmax(40.0, toneModule->baseFrequency + (beatIndex % 2 == 0 ? 0.0 : 55.0)),
                                            sampleRate);
            toneEnvelope *= 0.9972f;
            beatPosition += beatIncrement;
        }
        else
        {
            toneEnvelope *= 0.985f;
        }

        if (patternEnabled && transportCopy.isPlaying)
        {
            const auto patternStepIndex = static_cast<int>(std::floor(beatPosition * 2.0));
            const auto& steps = patternModule->stepPattern;
            const bool patternGate = steps.isEmpty() || steps[patternStepIndex % steps.size()] > 0;
            if (patternStepIndex != lastPatternStepIndex)
            {
                lastPatternStepIndex = patternStepIndex;
                if (patternGate)
                    patternEnvelope = (patternStepIndex % 2 == 0)
                        ? static_cast<float>(patternModule->accent)
                        : static_cast<float>(patternModule->accent * 0.5);
            }

            const auto patternValue = static_cast<float>(std::sin(patternPhase)) * patternEnvelope * 0.12f * patternGain;
            mixedSample += patternValue;
            patternPhase = advanceWrappedPhase(patternPhase,
                                               juce::jmax(60.0, patternModule->baseFrequency),
                                               sampleRate);
            patternEnvelope *= 0.994f;
        }

        if (droneEnabled)
        {
            const auto droneBlend = static_cast<float>(juce::jlimit(0.0, 1.0, droneModule->density));
            const auto droneValue = static_cast<float>((std::sin(dronePhase) + (0.35 * std::sin(dronePhaseTwo))) / 1.35)
                * (0.03f + (0.09f * droneBlend))
                * droneGain;
            mixedSample += droneValue;
            const auto baseFrequency = juce::jmax(30.0, droneModule->baseFrequency);
            dronePhase = advanceWrappedPhase(dronePhase, baseFrequency, sampleRate);
            dronePhaseTwo = advanceWrappedPhase(dronePhaseTwo,
                                                baseFrequency * juce::jmax(1.0, droneModule->spread),
                                                sampleRate);
        }

        for (int channel = 0; channel < numOutputChannels; ++channel)
            if (auto* channelData = outputChannelData[channel])
                channelData[sample] = mixedSample;
    }
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    const juce::ScopedLock scopedLock(lock);
    sampleRate = device != nullptr ? juce::jmax(1.0, device->getCurrentSampleRate()) : 44100.0;
    tonePhase = 0.0;
    patternPhase = 0.0;
    dronePhase = 0.0;
    dronePhaseTwo = 0.0;
    toneEnvelope = 0.0f;
    patternEnvelope = 0.0f;
    lastTriggeredBeatIndex = -1;
    lastPatternStepIndex = -1;
    status.ready = device != nullptr;
    status.detail = device != nullptr ? "audio ready" : "audio stopped";
    status.sampleRate = sampleRate;
    status.outputChannels = device != nullptr ? device->getActiveOutputChannels().countNumberOfSetBits() : 0;
}

void AudioEngine::audioDeviceStopped()
{
    const juce::ScopedLock scopedLock(lock);
    status.ready = false;
    status.detail = "audio stopped";
    status.sampleRate = 0.0;
    status.outputChannels = 0;
}

bool AudioEngine::isPlayableLifecycle(const juce::String& lifecycle)
{
    return lifecycle == "ready" || lifecycle == "active";
}

double AudioEngine::advanceWrappedPhase(double phase, double frequency, double currentSampleRate)
{
    phase += (twoPi * frequency) / juce::jmax(1.0, currentSampleRate);
    if (phase >= twoPi)
        phase -= twoPi;
    return phase;
}
