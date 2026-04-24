#include "MixerEngine.h"

void MixerEngine::setStateCallback(StateCallback callback)
{
    const juce::ScopedLock scopedLock(lock);
    stateCallback = std::move(callback);
}

void MixerEngine::initialiseState(const MixerState& initialState)
{
    {
        const juce::ScopedLock scopedLock(lock);
        state = initialState;
    }

    emitStateChanged();
}

MixerState MixerEngine::getState() const
{
    const juce::ScopedLock scopedLock(lock);
    return state;
}

bool MixerEngine::setStripLevel(const juce::String& stripId, double level)
{
    bool changed = false;
    {
        const juce::ScopedLock scopedLock(lock);
        for (auto& strip : state.strips)
        {
            if (strip.id != stripId)
                continue;

            strip.level = juce::jlimit(0.0, 1.5, level);
            changed = true;
            break;
        }
    }

    if (changed)
        emitStateChanged();

    return changed;
}

bool MixerEngine::setStripMuted(const juce::String& stripId, bool muted)
{
    bool changed = false;
    {
        const juce::ScopedLock scopedLock(lock);
        for (auto& strip : state.strips)
        {
            if (strip.id != stripId)
                continue;

            strip.muted = muted;
            changed = true;
            break;
        }
    }

    if (changed)
        emitStateChanged();

    return changed;
}

bool MixerEngine::setStripGroup(const juce::String& stripId, const juce::String& groupId)
{
    bool changed = false;
    {
        const juce::ScopedLock scopedLock(lock);
        for (auto& strip : state.strips)
        {
            if (strip.id != stripId || strip.kind != "module")
                continue;

            strip.assignedGroupId = groupId;
            changed = true;
            break;
        }
    }

    if (changed)
        emitStateChanged();

    return changed;
}

bool MixerEngine::setSendLevel(const juce::String& sendId, double level)
{
    bool changed = false;
    {
        const juce::ScopedLock scopedLock(lock);
        for (auto& send : state.sends)
        {
            if (send.sendId != sendId)
                continue;

            send.level = juce::jlimit(0.0, 1.0, level);
            changed = true;
            break;
        }
    }

    if (changed)
        emitStateChanged();

    return changed;
}

bool MixerEngine::setSendMode(const juce::String& sendId, const juce::String& mode)
{
    bool changed = false;
    {
        const juce::ScopedLock scopedLock(lock);
        for (auto& send : state.sends)
        {
            if (send.sendId != sendId)
                continue;

            send.mode = mode == "pre" ? "pre" : "post";
            changed = true;
            break;
        }
    }

    if (changed)
        emitStateChanged();

    return changed;
}

double MixerEngine::computeModuleGain(const juce::String& moduleId) const
{
    const juce::ScopedLock scopedLock(lock);

    const MixerStripEntry* moduleStrip = nullptr;
    const MixerStripEntry* masterStrip = nullptr;
    const MixerStripEntry* groupStrip = nullptr;

    for (const auto& strip : state.strips)
    {
        if (strip.kind == "master")
            masterStrip = &strip;

        if (strip.targetModuleId == moduleId)
            moduleStrip = &strip;
    }

    if (moduleStrip == nullptr || masterStrip == nullptr || moduleStrip->muted || masterStrip->muted)
        return 0.0;

    if (moduleStrip->assignedGroupId.isNotEmpty())
        groupStrip = state.findById(moduleStrip->assignedGroupId);

    if (groupStrip != nullptr && groupStrip->muted)
        return 0.0;

    auto gain = moduleStrip->level * masterStrip->level;
    if (groupStrip != nullptr)
        gain *= groupStrip->level;

    return gain;
}

bool MixerEngine::hasAudioPath(const juce::String& moduleId) const
{
    const juce::ScopedLock scopedLock(lock);
    for (const auto& strip : state.strips)
        if (strip.targetModuleId == moduleId)
            return strip.hasAudioPath;

    return false;
}

void MixerEngine::emitStateChanged()
{
    StateCallback callbackCopy;
    MixerState snapshot;

    {
        const juce::ScopedLock scopedLock(lock);
        snapshot = state;
        callbackCopy = stateCallback;
    }

    if (callbackCopy)
        callbackCopy(snapshot);
}
