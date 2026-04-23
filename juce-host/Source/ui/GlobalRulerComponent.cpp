#include "GlobalRulerComponent.h"

namespace
{
constexpr double visibleBars = 4.0;
constexpr float headerHeight = 22.0f;
constexpr float tickLabelInset = 6.0f;
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
    repaint();
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

    repaint();
}

void GlobalRulerComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto inner = bounds.reduced(12.0f);
    auto header = inner.removeFromTop(headerHeight);
    auto rulerArea = inner;

    g.setColour(juce::Colour(0xff171b20));
    g.fillRoundedRectangle(bounds, 8.0f);

    g.setColour(juce::Colour(0xff2b333d));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);

    g.setColour(juce::Colour(0xfff3f5f7));
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.drawText("Global Ruler", header.toNearestInt(), juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff8f99a5));
    g.setFont(juce::FontOptions(12.0f));
    g.drawText("Rendered from sclang transport only", header.removeFromRight(240).toNearestInt(), juce::Justification::centredRight);

    g.setColour(juce::Colour(0xff101419));
    g.fillRoundedRectangle(rulerArea, 6.0f);

    const double beatsPerBar = static_cast<double>(state.meterNumerator);
    const double visibleBeats = juce::jmax(4.0, beatsPerBar * visibleBars);
    const double currentBeat = state.beatPosition;
    const double startBeat = juce::jmax(0.0, currentBeat - (visibleBeats * 0.35));
    const double endBeat = startBeat + visibleBeats;
    const int firstBar = static_cast<int>(std::floor(startBeat / beatsPerBar)) + 1;
    const int lastBar = static_cast<int>(std::ceil(endBeat / beatsPerBar)) + 1;

    auto overlayArea = rulerArea.removeFromBottom(34.0f);

    if (hasSelectedOverlay && overlayClockDomain.ratioToParent > 0.0)
    {
        g.setColour(juce::Colour(0x2222c55e));
        g.fillRoundedRectangle(overlayArea.reduced(2.0f), 4.0f);

        const double localBeatsPerBar = static_cast<double>(state.meterNumerator);
        const double localStartBeat = (startBeat * overlayClockDomain.ratioToParent) + overlayClockDomain.phaseOffsetBeats;
        const double localEndBeat = (endBeat * overlayClockDomain.ratioToParent) + overlayClockDomain.phaseOffsetBeats;
        const int firstLocalBar = static_cast<int>(std::floor(localStartBeat / localBeatsPerBar));
        const int lastLocalBar = static_cast<int>(std::ceil(localEndBeat / localBeatsPerBar));

        for (int localBar = firstLocalBar; localBar <= lastLocalBar; ++localBar)
        {
            const double localBarStart = static_cast<double>(localBar) * localBeatsPerBar;
            const double globalBarBeat = (localBarStart - overlayClockDomain.phaseOffsetBeats) / overlayClockDomain.ratioToParent;

            if (globalBarBeat < startBeat || globalBarBeat > endBeat)
                continue;

            const float localBarX = beatToX(globalBarBeat - startBeat, rulerArea, visibleBeats);
            g.setColour(juce::Colour(0xff34d399));
            g.drawVerticalLine(static_cast<int>(localBarX), overlayArea.getY(), overlayArea.getBottom());

            for (int beatIndex = 1; beatIndex < state.meterNumerator; ++beatIndex)
            {
                const double localBeatBoundary = localBarStart + static_cast<double>(beatIndex);
                const double globalBeat = (localBeatBoundary - overlayClockDomain.phaseOffsetBeats) / overlayClockDomain.ratioToParent;

                if (globalBeat < startBeat || globalBeat > endBeat)
                    continue;

                const float localBeatX = beatToX(globalBeat - startBeat, rulerArea, visibleBeats);
                g.setColour(juce::Colour(0x8834d399));
                g.drawVerticalLine(static_cast<int>(localBeatX), overlayArea.getY() + 12.0f, overlayArea.getBottom());
            }
        }

        g.setColour(juce::Colour(0xff8f99a5));
        g.setFont(juce::FontOptions(11.5f));
        g.drawText("Overlay: " + overlayModule.displayName + " @ " + overlayClockDomain.displayName,
                   juce::Rectangle<float>(overlayArea.getX() + 8.0f, overlayArea.getY() + 2.0f, overlayArea.getWidth() - 16.0f, 14.0f).toNearestInt(),
                   juce::Justification::centredLeft);
    }
    else
    {
        g.setColour(juce::Colour(0xff8f99a5));
        g.setFont(juce::FontOptions(11.5f));
        g.drawText("Overlay: none", overlayArea.toNearestInt(), juce::Justification::centredLeft);
    }

    g.setFont(juce::FontOptions(12.0f));

    for (int bar = firstBar; bar <= lastBar; ++bar)
    {
        const double barStartBeat = (static_cast<double>(bar) - 1.0) * beatsPerBar;
        const float barX = beatToX(barStartBeat - startBeat, rulerArea, visibleBeats);

        g.setColour(juce::Colour(0xff42505f));
        g.drawVerticalLine(static_cast<int>(barX), rulerArea.getY(), rulerArea.getBottom());

        g.setColour(juce::Colour(0xffd9e3ee));
        g.drawText("Bar " + juce::String(bar),
                   juce::Rectangle<float>(barX + tickLabelInset, rulerArea.getY() + 4.0f, 64.0f, 16.0f).toNearestInt(),
                   juce::Justification::centredLeft);

        for (int beatIndex = 1; beatIndex < state.meterNumerator; ++beatIndex)
        {
            const double beatStart = barStartBeat + static_cast<double>(beatIndex);

            if (beatStart < startBeat || beatStart > endBeat)
                continue;

            const float beatX = beatToX(beatStart - startBeat, rulerArea, visibleBeats);
            g.setColour(juce::Colour(0xff2b333d));
            g.drawVerticalLine(static_cast<int>(beatX), rulerArea.getY() + 22.0f, rulerArea.getBottom());
        }
    }

    for (const auto& transition : structuralState.pendingTransitions)
    {
        if (transition.targetBeat < startBeat || transition.targetBeat > endBeat)
            continue;

        const float boundaryX = beatToX(transition.targetBeat - startBeat, rulerArea, visibleBeats);
        g.setColour(juce::Colour(0xffffd166));
        g.drawVerticalLine(static_cast<int>(boundaryX), rulerArea.getY(), rulerArea.getBottom());
        g.fillRoundedRectangle({ boundaryX - 4.0f, rulerArea.getY() + 24.0f, 8.0f, rulerArea.getHeight() - 30.0f }, 3.0f);
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText("pending " + transition.quantizationTarget,
                   juce::Rectangle<float>(boundaryX + 6.0f, rulerArea.getY() + 24.0f, 140.0f, 16.0f).toNearestInt(),
                   juce::Justification::centredLeft);
    }

    if (const auto* pending = structuralState.nextPendingTransition())
    {
        if (pending->targetBeat < 0.0)
        {
            g.setColour(juce::Colour(0xffffd166));
            g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
            g.drawText("Pending scene waits for external cue",
                       juce::Rectangle<float>(rulerArea.getX() + 8.0f, rulerArea.getY() + 42.0f, 260.0f, 18.0f).toNearestInt(),
                       juce::Justification::centredLeft);
        }
    }

    const float playheadX = beatToX(currentBeat - startBeat, rulerArea, visibleBeats);
    g.setColour(state.isPlaying ? juce::Colour(0xfff59e0b) : juce::Colour(0xff7dd3fc));
    g.drawVerticalLine(static_cast<int>(playheadX), rulerArea.getY(), rulerArea.getBottom());
    g.fillEllipse(playheadX - 5.0f, rulerArea.getY() + 6.0f, 10.0f, 10.0f);

    g.setColour(juce::Colour(0xff9ea7b3));
    g.setFont(juce::FontOptions(13.0f));
    g.drawText("Beat " + juce::String(state.beatPosition, 2)
                   + " | Phase " + juce::String(state.phaseWithinBar, 3),
               juce::Rectangle<float>(rulerArea.getX() + 8.0f, rulerArea.getBottom() - 24.0f, rulerArea.getWidth() - 16.0f, 18.0f).toNearestInt(),
               juce::Justification::bottomLeft);
}

float GlobalRulerComponent::beatToX(double beatOffset, juce::Rectangle<float> bounds, double visibleBeats) const
{
    return bounds.getX() + static_cast<float>((beatOffset / visibleBeats) * bounds.getWidth());
}
