#include "ClockDomain.h"

namespace
{
double beatUnitScale(const ClockDomainEntry& definition)
{
    return static_cast<double>(juce::jmax(1, definition.meterDenominator)) / 4.0;
}

double barLengthInQuarterBeats(const ClockDomainEntry& definition)
{
    return static_cast<double>(juce::jmax(1, definition.meterNumerator))
         * (4.0 / static_cast<double>(juce::jmax(1, definition.meterDenominator)));
}

double positiveModulo(double value, double modulus)
{
    if (modulus <= 0.0)
        return 0.0;

    auto result = std::fmod(value, modulus);
    if (result < 0.0)
        result += modulus;
    return result;
}
}

ClockDomain::ClockDomain(const ClockDomainEntry& definitionToUse, double startSeconds)
    : definition(definitionToUse), freeRunningStartSeconds(startSeconds)
{
}

const ClockDomainEntry& ClockDomain::getDefinition() const noexcept
{
    return definition;
}

void ClockDomain::setDefinition(const ClockDomainEntry& newDefinition)
{
    definition = newDefinition;
}

void ClockDomain::setRelation(const juce::String& relationType, double phaseOffsetBeats)
{
    definition.relationType = relationType;
    definition.phaseOffsetBeats = phaseOffsetBeats;
}

void ClockDomain::resetFreeRunningAnchor(double startSeconds)
{
    freeRunningStartSeconds = startSeconds;
}

ClockDomainEntry ClockDomain::computeState(const TransportState& transportState,
                                           const ClockDomainEntry* parentState,
                                           double nowSeconds) const
{
    auto result = definition;
    const auto meterNum = juce::jmax(1, result.meterNumerator);
    const auto beatScale = beatUnitScale(result);
    result.barLengthBeats = barLengthInQuarterBeats(result);
    result.phraseLengthBeats = result.barLengthBeats * static_cast<double>(juce::jmax(1, result.phraseLengthBars));

    if (result.kind == "global")
    {
        result.absoluteBeatPosition = transportState.beatPosition;
    }
    else if (result.kind == "free")
    {
        const auto elapsedSeconds = juce::jmax(0.0, nowSeconds - freeRunningStartSeconds);
        const auto quarterBeats = elapsedSeconds * (transportState.tempo / 60.0);
        result.absoluteBeatPosition = quarterBeats;
    }
    else
    {
        const auto parentAbsoluteBeat = parentState != nullptr ? parentState->absoluteBeatPosition : transportState.beatPosition;
        const auto relation = result.relationType.isNotEmpty() ? result.relationType : juce::String("tempoShared");

        if (relation == "meterShared")
        {
            result.ratioToParent = 1.0;
            result.absoluteBeatPosition = parentAbsoluteBeat;
        }
        else if (relation == "phaseShared")
        {
            result.phaseOffsetBeats = 0.0;
            result.absoluteBeatPosition = parentAbsoluteBeat;
        }
        else if (relation == "hardSync")
        {
            result.ratioToParent = 1.0;
            result.phaseOffsetBeats = 0.0;
            const auto parentBarLength = parentState != nullptr ? parentState->barLengthBeats
                                                                : static_cast<double>(juce::jmax(1, transportState.meterNumerator));
            result.absoluteBeatPosition = std::floor(parentAbsoluteBeat / juce::jmax(1.0, parentBarLength)) * juce::jmax(1.0, parentBarLength);
        }
        else
        {
            const auto rate = juce::jmax(0.001, result.ratioToParent) * beatScale;
            result.absoluteBeatPosition = (parentAbsoluteBeat * rate + result.phaseOffsetBeats) / rate;
        }
    }

    const auto rate = (result.kind == "free" ? beatScale
                       : juce::jmax(0.001, result.ratioToParent) * beatScale);
    const auto localBeats = result.absoluteBeatPosition * rate + result.phaseOffsetBeats;

    result.localBeatPosition = localBeats;
    result.localBarIndex = static_cast<int>(std::floor(localBeats / static_cast<double>(meterNum))) + 1;
    result.localBeatInBar = positiveModulo(localBeats, static_cast<double>(meterNum)) + 1.0;
    result.phraseIndex = static_cast<int>(std::floor(result.absoluteBeatPosition / juce::jmax(1.0, result.phraseLengthBeats)));
    result.phrasePhase = positiveModulo(result.absoluteBeatPosition, juce::jmax(1.0, result.phraseLengthBeats));
    result.nextPhraseBeat = (std::floor(result.absoluteBeatPosition / juce::jmax(1.0, result.phraseLengthBeats)) + 1.0)
        * juce::jmax(1.0, result.phraseLengthBeats);

    return result;
}
