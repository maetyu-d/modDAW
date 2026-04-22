#include "ModuleLanesComponent.h"

ModuleLanesComponent::ModuleLanesComponent()
{
}

void ModuleLanesComponent::setModuleState(const ModuleState& newState)
{
    state = newState;
    repaint();
}

void ModuleLanesComponent::setSelectedModuleId(const juce::String& moduleId)
{
    selectedModuleId = moduleId;
    repaint();
}

void ModuleLanesComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto inner = bounds.reduced(12.0f);
    auto header = inner.removeFromTop(28.0f);

    g.setColour(juce::Colour(0xff171b20));
    g.fillRoundedRectangle(bounds, 8.0f);

    g.setColour(juce::Colour(0xff2b333d));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);

    g.setColour(juce::Colour(0xfff3f5f7));
    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.drawText("Module Lanes", header.toNearestInt(), juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff8f99a5));
    g.setFont(juce::FontOptions(12.5f));
    g.drawText("Select one lane to show a local timing overlay", header.removeFromRight(280).toNearestInt(), juce::Justification::centredRight);

    if (state.modules.isEmpty())
        return;

    const int laneCount = juce::jmin(3, state.modules.size());
    const float laneGap = 8.0f;
    const float laneHeight = (inner.getHeight() - (laneGap * static_cast<float>(laneCount - 1))) / static_cast<float>(juce::jmax(1, laneCount));

    for (int i = 0; i < laneCount; ++i)
    {
        const auto& module = state.modules.getReference(i);
        auto laneY = inner.getY() + ((laneHeight + laneGap) * static_cast<float>(i));
        auto lane = juce::Rectangle<float>(inner.getX(), laneY, inner.getWidth(), laneHeight);
        const bool isSelected = (module.id == selectedModuleId);

        g.setColour(isSelected ? juce::Colour(0xff243443) : juce::Colour(0xff101419));
        g.fillRoundedRectangle(lane, 6.0f);

        g.setColour(isSelected ? juce::Colour(0xff7dd3fc) : juce::Colour(0xff2b333d));
        g.drawRoundedRectangle(lane.reduced(0.5f), 6.0f, 1.0f);

        g.setColour(juce::Colour(0xfff3f5f7));
        g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        g.drawText(module.displayName,
                   juce::Rectangle<float>(lane.getX() + 12.0f, lane.getY() + 8.0f, lane.getWidth() - 24.0f, 18.0f).toNearestInt(),
                   juce::Justification::centredLeft);

        g.setColour(juce::Colour(0xff9ea7b3));
        g.setFont(juce::FontOptions(12.0f));
        g.drawText("clock " + module.clockDomainId + " | state " + module.lifecycleState + " | " + module.behaviourType,
                   juce::Rectangle<float>(lane.getX() + 12.0f, lane.getY() + 28.0f, lane.getWidth() - 24.0f, 16.0f).toNearestInt(),
                   juce::Justification::centredLeft);

        g.setColour(isSelected ? juce::Colour(0xff7dd3fc) : juce::Colour(0xff8f99a5));
        g.drawText(isSelected ? "selected lane overlay active" : "click to select lane",
                   juce::Rectangle<float>(lane.getX() + 12.0f, lane.getBottom() - 20.0f, lane.getWidth() - 24.0f, 14.0f).toNearestInt(),
                   juce::Justification::centredLeft);
    }
}

void ModuleLanesComponent::mouseUp(const juce::MouseEvent& event)
{
    const auto laneIndex = laneIndexAtPosition(event.position);

    if (laneIndex < 0 || laneIndex >= juce::jmin(3, state.modules.size()))
        return;

    const auto& module = state.modules.getReference(laneIndex);
    selectedModuleId = module.id;
    repaint();

    if (onModuleSelected)
        onModuleSelected(module.id);
}

int ModuleLanesComponent::laneIndexAtPosition(juce::Point<float> position) const
{
    auto inner = getLocalBounds().toFloat().reduced(12.0f);
    inner.removeFromTop(24.0f);

    const int laneCount = juce::jmin(3, state.modules.size());
    if (laneCount <= 0)
        return -1;

    const float laneGap = 8.0f;
    const float laneHeight = (inner.getHeight() - (laneGap * static_cast<float>(laneCount - 1))) / static_cast<float>(laneCount);

    for (int i = 0; i < laneCount; ++i)
    {
        auto lane = juce::Rectangle<float>(inner.getX(), inner.getY() + ((laneHeight + laneGap) * static_cast<float>(i)), inner.getWidth(), laneHeight);
        if (lane.contains(position))
            return i;
    }

    return -1;
}
