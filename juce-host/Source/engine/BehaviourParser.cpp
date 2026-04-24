#include "BehaviourParser.h"

namespace
{
juce::StringArray nonEmptyLines(const juce::String& text)
{
    juce::StringArray lines;
    lines.addLines(text);
    lines.trim();
    lines.removeEmptyStrings();
    return lines;
}
}

ParsedStepPattern BehaviourParser::parseBinarySteps(const juce::String& codeText)
{
    ParsedStepPattern result;
    auto trimmed = codeText.trim();

    if (! trimmed.startsWithIgnoreCase("steps"))
    {
        result.diagnostic = "expected steps: [0, 1, ...]";
        return result;
    }

    auto open = trimmed.indexOfChar('[');
    auto close = trimmed.indexOfChar(']');
    if (open < 0 || close <= open)
    {
        result.diagnostic = "missing step brackets";
        return result;
    }

    auto items = juce::StringArray::fromTokens(trimmed.substring(open + 1, close), ",", {});
    items.trim();
    items.removeEmptyStrings();

    if (items.isEmpty())
    {
        result.diagnostic = "step pattern cannot be empty";
        return result;
    }

    for (const auto& item : items)
    {
        if (item != "0" && item != "1")
        {
            result.steps.clear();
            result.diagnostic = "steps must contain only 0 or 1";
            return result;
        }

        result.steps.add(item.getIntValue());
    }

    result.success = true;
    result.diagnostic = "pattern parsed";
    return result;
}

ParsedScalarBlock BehaviourParser::parseScalarBlock(const juce::String& codeText)
{
    ParsedScalarBlock result;
    auto lines = nonEmptyLines(codeText);

    if (lines.isEmpty())
    {
        result.diagnostic = "empty surface";
        return result;
    }

    for (const auto& line : lines)
    {
        auto separator = line.indexOfChar(':');
        if (separator <= 0)
        {
            result.values.clear();
            result.diagnostic = "expected key: value lines";
            return result;
        }

        auto key = line.substring(0, separator).trim();
        auto valueText = line.substring(separator + 1).trim();

        if (key.isEmpty() || valueText.isEmpty())
        {
            result.values.clear();
            result.diagnostic = "empty key or value";
            return result;
        }

        auto value = valueText.getDoubleValue();
        if (! std::isfinite(value))
        {
            result.values.clear();
            result.diagnostic = "invalid number for " + key;
            return result;
        }

        result.values.set(juce::Identifier(key), value);
    }

    result.success = true;
    result.diagnostic = "surface parsed";
    return result;
}

double BehaviourParser::readScalar(const ParsedScalarBlock& block, const juce::String& key, double fallback)
{
    if (auto* value = block.values.getVarPointer(juce::Identifier(key)))
        return static_cast<double>(*value);

    return fallback;
}
