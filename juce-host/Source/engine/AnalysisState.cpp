#include "AnalysisState.h"

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

juce::String AnalysisModuleEntry::toSummaryString() const
{
    return sourceModuleId + " env=" + juce::String(envelope, 2)
         + " density=" + juce::String(density, 2)
         + " bright=" + juce::String(brightness, 2);
}

AnalysisState AnalysisState::fromPayload(const juce::var& payload)
{
    AnalysisState state;

    if (auto* dynamicObject = payload.getDynamicObject())
    {
        if (auto* modulesArray = dynamicObject->getProperty("modules").getArray())
        {
            for (const auto& item : *modulesArray)
            {
                AnalysisModuleEntry module;
                module.moduleId = readString(item, "moduleId");
                module.sourceModuleId = readString(item, "sourceModuleId");
                module.routing = readString(item, "routing");
                module.envelope = readDouble(item, "envelope", 0.0);
                module.onset = readBool(item, "onset", false);
                module.density = readDouble(item, "density", 0.0);
                module.brightness = readDouble(item, "brightness", 0.0);
                module.lastBeat = readDouble(item, "lastBeat", 0.0);

                if (auto* object = item.getDynamicObject())
                {
                    if (auto* targets = object->getProperty("targetModuleIds").getArray())
                    {
                        for (const auto& target : *targets)
                            module.targetModuleIds.add(target.toString());
                    }
                }

                state.modules.add(module);
            }
        }
    }

    return state;
}

juce::String AnalysisState::toSummaryString() const
{
    return juce::String(modules.size()) + " analysis modules";
}

const AnalysisModuleEntry* AnalysisState::firstModule() const
{
    return modules.isEmpty() ? nullptr : &modules.getReference(0);
}
