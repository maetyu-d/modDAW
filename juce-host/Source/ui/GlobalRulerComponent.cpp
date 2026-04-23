#include "GlobalRulerComponent.h"

namespace
{
constexpr float headerHeight = 22.0f;
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
    g.drawText("Minimal ruler isolation mode", header.removeFromRight(220).toNearestInt(), juce::Justification::centredRight);

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
}

float GlobalRulerComponent::beatToX(double beatOffset, juce::Rectangle<float> bounds, double visibleBeats) const
{
    return bounds.getX() + static_cast<float>((beatOffset / visibleBeats) * bounds.getWidth());
}
