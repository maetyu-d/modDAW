#include "RecoveryState.h"

namespace
{
juce::String readString(const juce::var& object, const juce::Identifier& propertyName)
{
    if (auto* dynamicObject = object.getDynamicObject())
        return dynamicObject->getProperty(propertyName).toString();

    return {};
}

bool readBool(const juce::var& object, const juce::Identifier& propertyName, bool fallback)
{
    if (auto* dynamicObject = object.getDynamicObject())
    {
        auto value = dynamicObject->getProperty(propertyName);
        return value.isVoid() ? fallback : static_cast<bool>(value);
    }

    return fallback;
}
}

RecoveryState RecoveryState::fromPayload(const juce::var& payload)
{
    RecoveryState state;

    if (auto* dynamicObject = payload.getDynamicObject())
    {
        state.engineOnline = readBool(payload, "engineOnline", false);
        state.audioServerReady = readBool(payload, "audioServerReady", false);
        state.projectDirty = readBool(payload, "projectDirty", false);
        state.recoveryPath = readString(payload, "recoveryPath");
        state.lastRecoverySnapshotAt = readString(payload, "lastRecoverySnapshotAt");
        state.lastReason = readString(payload, "lastReason");

        if (auto* errors = dynamicObject->getProperty("moduleErrors").getArray())
        {
            for (const auto& item : *errors)
            {
                RecoveryModuleError error;
                error.moduleId = readString(item, "moduleId");
                error.surfaceId = readString(item, "surfaceId");
                error.diagnostic = readString(item, "diagnostic");
                state.moduleErrors.add(error);
            }
        }
    }

    return state;
}

juce::String RecoveryState::toSummaryString() const
{
    auto text = juce::String(projectDirty ? "unsaved changes" : "clean")
              + " | audio " + juce::String(audioServerReady ? "ready" : "not ready");

    if (! moduleErrors.isEmpty())
        text += " | " + juce::String(moduleErrors.size()) + " isolated module errors";

    if (lastRecoverySnapshotAt.isNotEmpty())
        text += " | autosave " + lastRecoverySnapshotAt;

    return text;
}
