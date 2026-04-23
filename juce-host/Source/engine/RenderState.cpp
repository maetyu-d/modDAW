#include "RenderState.h"

namespace
{
juce::String readString(const juce::var& object, const juce::Identifier& propertyName)
{
    if (auto* dynamicObject = object.getDynamicObject())
        return dynamicObject->getProperty(propertyName).toString();

    return {};
}

int readInt(const juce::var& object, const juce::Identifier& propertyName, int fallback)
{
    if (auto* dynamicObject = object.getDynamicObject())
    {
        auto value = dynamicObject->getProperty(propertyName);
        return value.isVoid() ? fallback : static_cast<int>(value);
    }

    return fallback;
}
}

RenderState RenderState::fromPayload(const juce::var& payload)
{
    RenderState state;

    if (auto* dynamicObject = payload.getDynamicObject())
    {
        state.status = readString(payload, "status");
        state.lastRenderId = readString(payload, "lastRenderId");
        state.outputDirectory = readString(payload, "outputDirectory");

        if (auto* artifacts = dynamicObject->getProperty("artifacts").getArray())
        {
            for (const auto& item : *artifacts)
            {
                RenderArtifact artifact;
                artifact.stemId = readString(item, "stemId");
                artifact.path = readString(item, "path");
                artifact.artifactKind = readString(item, "artifactKind");
                artifact.eventCount = readInt(item, "eventCount", 0);
                state.artifacts.add(artifact);
            }
        }
    }

    return state;
}

juce::String RenderState::toSummaryString() const
{
    if (artifacts.isEmpty())
        return status;

    return status + " | " + juce::String(artifacts.size()) + " artifacts | " + outputDirectory;
}
