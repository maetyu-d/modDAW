#include "GlobalRulerComponent.h"

namespace
{
constexpr float headerHeight = 22.0f;

double beatUnitScale(const ClockDomainEntry& domain)
{
    return static_cast<double>(juce::jmax(1, domain.meterDenominator)) / 4.0;
}
}

GlobalRulerComponent::GlobalRulerComponent()
{
}

void GlobalRulerComponent::setTransportState(const TransportState& newState)
{
    state = newState;
    repaint();
}

void GlobalRulerComponent::setStructuralState(const StructuralState& newState)
{
    structuralState = newState;
}

void GlobalRulerComponent::setSelectedLaneOverlay(const ModuleEntry* selectedModule, const ClockDomainEntry* selectedClockDomain)
{
    hasSelectedOverlay = (selectedModule != nullptr && selectedClockDomain != nullptr);

    if (hasSelectedOverlay)
    {
        overlayModule = *selectedModule;
        overlayClockDomain = *selectedClockDomain;
    }
    else
    {
        overlayModule = {};
        overlayClockDomain = {};
    }
}

void GlobalRulerComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto inner = bounds.reduced(12.0f);
    auto header = inner.removeFromTop(headerHeight);
    auto rulerArea = inner.reduced(0.0f, 10.0f);

    g.setColour(juce::Colour(0xff171b20));
    g.fillRoundedRectangle(bounds, 8.0f);

    g.setColour(juce::Colour(0xff2b333d));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);

    g.setColour(juce::Colour(0xfff3f5f7));
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.drawText("Global Ruler", header.toNearestInt(), juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff8f99a5));
    g.setFont(juce::FontOptions(12.0f));
    g.drawText("TransportEngine timing", header.removeFromRight(220).toNearestInt(), juce::Justification::centredRight);

    g.setColour(juce::Colour(0xff101419));
    g.fillRoundedRectangle(rulerArea, 6.0f);

    const double beatsPerBar = static_cast<double>(state.meterNumerator);
    const double visibleBeats = juce::jmax(4.0, beatsPerBar * 4.0);
    const double currentBeat = state.beatPosition;
    const double startBeat = juce::jmax(0.0, currentBeat - (visibleBeats * 0.35));
    const int firstBar = static_cast<int>(std::floor(startBeat / beatsPerBar)) + 1;
    const int lastBar = firstBar + 4;

    for (int bar = firstBar; bar <= lastBar; ++bar)
    {
        const double barStartBeat = (static_cast<double>(bar) - 1.0) * beatsPerBar;
        const float barX = beatToX(barStartBeat - startBeat, rulerArea, visibleBeats);
        g.setColour(juce::Colour(0xff42505f));
        g.drawVerticalLine(static_cast<int>(barX), rulerArea.getY(), rulerArea.getBottom());
    }

    const float playheadX = beatToX(currentBeat - startBeat, rulerArea, visibleBeats);
    g.setColour(state.isPlaying ? juce::Colour(0xfff59e0b) : juce::Colour(0xff7dd3fc));
    g.drawVerticalLine(static_cast<int>(playheadX), rulerArea.getY(), rulerArea.getBottom());
    g.fillEllipse(playheadX - 5.0f, rulerArea.getY() + 6.0f, 10.0f, 10.0f);

    g.setColour(juce::Colour(0xff9ea7b3));
    g.setFont(juce::FontOptions(13.0f));
    g.drawText("Beat " + juce::String(state.beatPosition, 2)
                   + " | Bar " + juce::String(state.barIndex)
                   + " | " + juce::String(state.isPlaying ? "playing" : "stopped"),
               juce::Rectangle<float>(rulerArea.getX() + 8.0f, rulerArea.getBottom() - 24.0f, rulerArea.getWidth() - 16.0f, 18.0f).toNearestInt(),
               juce::Justification::bottomLeft);

    if (! hasSelectedOverlay)
        return;

    auto overlayArea = rulerArea.reduced(8.0f, 0.0f).withY(rulerArea.getBottom() - 42.0f).withHeight(28.0f);
    const auto overlayColour = overlayClockDomain.kind == "free" ? juce::Colour(0xff22413a)
                              : overlayClockDomain.kind == "local" ? juce::Colour(0xff213746)
                              : juce::Colour(0xff173b32);

    g.setColour(overlayColour.withAlpha(0.82f));
    g.fillRoundedRectangle(overlayArea, 5.0f);

    g.setColour(juce::Colour(0xff9ea7b3));
    g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    g.drawText("Overlay: " + overlayModule.displayName + " @ " + overlayClockDomain.displayName,
               overlayArea.removeFromTop(12.0f).reduced(6.0f, -1.0f).toNearestInt(),
               juce::Justification::centredLeft);

    auto markerArea = overlayArea.reduced(4.0f, 2.0f);
    const auto endBeat = startBeat + visibleBeats;

    if (overlayClockDomain.kind == "free")
    {
        const auto barLength = juce::jmax(1.0, overlayClockDomain.barLengthBeats);
        const auto phase = std::fmod(overlayClockDomain.absoluteBeatPosition, barLength);
        for (int step = -2; step < 6; ++step)
        {
            const auto beat = currentBeat + (static_cast<double>(step) * barLength) - phase;
            const auto x = beatToX(beat - startBeat, markerArea, visibleBeats);
            g.setColour(juce::Colour(0xff6ee7b7).withAlpha(step == 0 ? 0.95f : 0.55f));
            g.drawVerticalLine(static_cast<int>(x), markerArea.getY(), markerArea.getBottom());
        }
        return;
    }

    const auto rate = juce::jmax(0.001, overlayClockDomain.ratioToParent) * beatUnitScale(overlayClockDomain);
    const auto localStartBeat = startBeat * rate + overlayClockDomain.phaseOffsetBeats;
    const auto localEndBeat = endBeat * rate + overlayClockDomain.phaseOffsetBeats;
    const auto firstMarker = static_cast<int>(std::floor(localStartBeat)) - 1;
    const auto lastMarker = static_cast<int>(std::ceil(localEndBeat)) + 1;

    for (int localBeatIndex = firstMarker; localBeatIndex <= lastMarker; ++localBeatIndex)
    {
        const auto globalBeat = (static_cast<double>(localBeatIndex) - overlayClockDomain.phaseOffsetBeats) / rate;
        if (globalBeat < startBeat || globalBeat > endBeat)
            continue;

        const auto x = beatToX(globalBeat - startBeat, markerArea, visibleBeats);
        const auto beatInBarIndex = ((localBeatIndex % juce::jmax(1, overlayClockDomain.meterNumerator))
                                     + juce::jmax(1, overlayClockDomain.meterNumerator))
                                    % juce::jmax(1, overlayClockDomain.meterNumerator);
        const bool isBarBoundary = beatInBarIndex == 0;
        g.setColour((isBarBoundary ? juce::Colour(0xff34d399) : juce::Colour(0xff4fd1c5)).withAlpha(isBarBoundary ? 0.95f : 0.55f));
        g.drawVerticalLine(static_cast<int>(x), markerArea.getY(), markerArea.getBottom());
    }
}

float GlobalRulerComponent::beatToX(double beatOffset, juce::Rectangle<float> bounds, double visibleBeats) const
{
    return bounds.getX() + static_cast<float>((beatOffset / visibleBeats) * bounds.getWidth());
}
