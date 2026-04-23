#include "StructuralState.h"

namespace
{
juce::String readString(const juce::var& object, const juce::Identifier& propertyName)
{
    if (auto* dynamicObject = object.getDynamicObject())
        return dynamicObject->getProperty(propertyName).toString();

    return {};
}

double readDouble(const juce::var& object, const juce::Identifier& propertyName, double fallback)
{
    if (auto* dynamicObject = object.getDynamicObject())
    {
        auto value = dynamicObject->getProperty(propertyName);
        return value.isVoid() ? fallback : static_cast<double>(value);
    }

    return fallback;
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

juce::String StructuralDirectiveEntry::toSummaryString() const
{
    return section + " | density " + juce::String(densityTarget, 2)
         + " | " + syncCue + " | " + quantizationTarget + " | " + orchestration;
}

juce::String StructuralPendingTransitionEntry::toSummaryString() const
{
    auto target = targetBeat >= 0.0 ? juce::String(targetBeat, 2) : juce::String("external cue");
    return sceneName + " | " + quantizationTarget + " | target " + target + " | " + state;
}

StructuralState StructuralState::fromPayload(const juce::var& payload)
{
    StructuralState state;

    if (auto* dynamicObject = payload.getDynamicObject())
    {
        if (auto* directivesArray = dynamicObject->getProperty("directives").getArray())
        {
            for (const auto& item : *directivesArray)
            {
                StructuralDirectiveEntry directive;
                directive.directiveId = readString(item, "directiveId");
                directive.emitterModuleId = readString(item, "emitterModuleId");
                directive.section = readString(item, "section");
                directive.syncCue = readString(item, "syncCue");
                directive.orchestration = readString(item, "orchestration");
                directive.quantizationTarget = readString(item, "quantizationTarget");
                directive.beat = readDouble(item, "beat", 0.0);
                directive.scheduledBeat = readDouble(item, "scheduledBeat", directive.beat);
                directive.phraseIndex = static_cast<int>(readDouble(item, "phraseIndex", 0.0));
                directive.densityTarget = readDouble(item, "densityTarget", 0.0);
                directive.phraseReset = readBool(item, "phraseReset", false);

                if (auto* itemObject = item.getDynamicObject())
                {
                    if (auto* targets = itemObject->getProperty("targetModuleIds").getArray())
                    {
                        for (const auto& target : *targets)
                            directive.targetModuleIds.add(target.toString());
                    }
                }

                state.directives.add(directive);
            }
        }

        state.currentScene = readString(payload, "currentScene");

        if (auto* pendingArray = dynamicObject->getProperty("pendingTransitions").getArray())
        {
            for (const auto& item : *pendingArray)
            {
                StructuralPendingTransitionEntry transition;
                transition.transitionId = readString(item, "transitionId");
                transition.domainId = readString(item, "domainId");
                transition.sceneName = readString(item, "sceneName");
                transition.quantizationTarget = readString(item, "quantizationTarget");
                transition.state = readString(item, "state");
                transition.afterCycles = static_cast<int>(readDouble(item, "afterCycles", 0.0));
                transition.requestedBeat = readDouble(item, "requestedBeat", 0.0);
                transition.targetBeat = readDouble(item, "targetBeat", -1.0);
                state.pendingTransitions.add(transition);
            }
        }
    }

    return state;
}

juce::String StructuralState::toSummaryString() const
{
    return juce::String(directives.size()) + " structural directives, "
         + juce::String(pendingTransitions.size()) + " pending transitions";
}

const StructuralDirectiveEntry* StructuralState::latestDirective() const
{
    return directives.isEmpty() ? nullptr : &directives.getReference(directives.size() - 1);
}

const StructuralPendingTransitionEntry* StructuralState::nextPendingTransition() const
{
    return pendingTransitions.isEmpty() ? nullptr : &pendingTransitions.getReference(0);
}
