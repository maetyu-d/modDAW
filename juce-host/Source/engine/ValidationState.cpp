#include "ValidationState.h"

namespace
{
juce::String readString(const juce::var& object, const juce::Identifier& propertyName)
{
    if (auto* dynamicObject = object.getDynamicObject())
        return dynamicObject->getProperty(propertyName).toString();

    return {};
}

bool readBool(const juce::var& object, const juce::Identifier& propertyName, bool fallbackValue)
{
    if (auto* dynamicObject = object.getDynamicObject())
        return static_cast<bool>(dynamicObject->getProperty(propertyName));

    return fallbackValue;
}

int readInt(const juce::var& object, const juce::Identifier& propertyName, int fallbackValue)
{
    if (auto* dynamicObject = object.getDynamicObject())
        return static_cast<int>(dynamicObject->getProperty(propertyName));

    return fallbackValue;
}
}

juce::String ValidationFinding::toSummaryString() const
{
    return "[" + severity + "] " + ruleId + ": " + message;
}

ValidationState ValidationState::fromPayload(const juce::var& payload)
{
    ValidationState state;
    state.isValid = readBool(payload, "isValid", true);
    state.findingCount = readInt(payload, "findingCount", 0);

    if (auto* dynamicObject = payload.getDynamicObject())
    {
        if (auto* findingsArray = dynamicObject->getProperty("findings").getArray())
        {
            for (const auto& item : *findingsArray)
            {
                ValidationFinding finding;
                finding.severity = readString(item, "severity");
                finding.ruleId = readString(item, "ruleId");
                finding.message = readString(item, "message");
                state.findings.add(finding);
            }
        }
    }

    if (state.findingCount <= 0)
        state.findingCount = state.findings.size();

    return state;
}

juce::String ValidationState::toSummaryString() const
{
    if (isValid)
        return "valid | 0 findings";

    return "invalid | " + juce::String(findingCount) + " findings";
}
