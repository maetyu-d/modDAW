#pragma once

#include <JuceHeader.h>

struct RecoveryModuleError
{
    juce::String moduleId;
    juce::String surfaceId;
    juce::String diagnostic;
};

struct RecoveryState
{
    bool engineOnline = false;
    bool audioServerReady = false;
    bool projectDirty = false;
    juce::String recoveryPath;
    juce::String lastRecoverySnapshotAt;
    juce::String lastReason;
    juce::Array<RecoveryModuleError> moduleErrors;

    static RecoveryState fromPayload(const juce::var& payload);
    juce::String toSummaryString() const;
};
