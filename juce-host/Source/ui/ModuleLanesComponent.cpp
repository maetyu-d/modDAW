#include "ModuleLanesComponent.h"

ModuleLanesComponent::ModuleLanesComponent()
{
}

void ModuleLanesComponent::setModuleState(const ModuleState& newState)
{
    state = newState;
    repaint();
}

void ModuleLanesComponent::setRegionState(const RegionState& newState)
{
    regionState = newState;
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
        auto lane = juce::Rectangle<float>(inner.getX(),
                                           inner.getY() + ((laneHeight + laneGap) * static_cast<float>(i)),
                                           inner.getWidth(),
                                           laneHeight);
        const bool isSelected = (module.id == selectedModuleId);
        auto freezeButton = freezeButtonBoundsForLane(lane);

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
                   juce::Rectangle<float>(lane.getX() + 12.0f, lane.getY() + 28.0f, lane.getWidth() - freezeButton.getWidth() - 34.0f, 16.0f).toNearestInt(),
                   juce::Justification::centredLeft);

        g.setColour(juce::Colour(0xff22313a));
        g.fillRoundedRectangle(freezeButton, 5.0f);
        g.setColour(juce::Colour(0xff7dd3fc));
        g.drawRoundedRectangle(freezeButton.reduced(0.5f), 5.0f, 1.0f);
        g.setColour(juce::Colour(0xfff3f5f7));
        g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        g.drawText("Freeze", freezeButton.toNearestInt(), juce::Justification::centred);

        auto regions = regionState.regionsForModule(module.id);
        if (! regions.isEmpty())
        {
            auto regionRail = regionRailBoundsForLane(lane);

            g.setColour(juce::Colour(0xff0b0d10));
            g.fillRoundedRectangle(regionRail, 4.0f);

            for (const auto& region : regions)
            {
                auto block = regionBoundsInRail(region, regionRail);

                if (! block.isEmpty())
                {
                    const bool isRegionSelected = region.regionId == selectedRegionId;
                    g.setColour(isRegionSelected ? juce::Colour(0xfff1b852) : juce::Colour(0xffd59f42));
                    g.fillRoundedRectangle(block, 4.0f);
                    g.setColour(isRegionSelected ? juce::Colour(0xfffff0b8) : juce::Colour(0xffffdf8a));
                    g.drawRoundedRectangle(block.reduced(0.5f), 4.0f, isRegionSelected ? 2.0f : 1.0f);
                }
            }
        }

        if (isSelected && findSelectedRegion() != nullptr)
        {
            static const char* labels[] = { "< Beat", "> Beat", "Trim -", "Trim +", "Split", "Delete" };

            for (int buttonIndex = 0; buttonIndex < 6; ++buttonIndex)
            {
                auto button = editButtonBoundsForLane(lane, buttonIndex);
                g.setColour(buttonIndex == 5 ? juce::Colour(0xff402428) : juce::Colour(0xff1f2b34));
                g.fillRoundedRectangle(button, 4.0f);
                g.setColour(buttonIndex == 5 ? juce::Colour(0xffff8a8a) : juce::Colour(0xff8fc7da));
                g.drawRoundedRectangle(button.reduced(0.5f), 4.0f, 1.0f);
                g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
                g.drawText(labels[buttonIndex], button.toNearestInt(), juce::Justification::centred);
            }
        }

        g.setColour(isSelected ? juce::Colour(0xff7dd3fc) : juce::Colour(0xff8f99a5));
        g.setFont(juce::FontOptions(11.5f));
        g.drawText(isSelected ? "selected lane overlay active | frozen regions are engine-owned" : "click to select lane",
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
    auto lane = laneBoundsForIndex(laneIndex);

    if (freezeButtonBoundsForLane(lane).contains(event.position))
    {
        if (onFreezeModule)
            onFreezeModule(module.id);
        return;
    }

    for (const auto& region : regionState.regionsForModule(module.id))
    {
        auto block = regionBoundsInRail(region, regionRailBoundsForLane(lane));
        if (block.contains(event.position))
        {
            selectedModuleId = module.id;
            selectedRegionId = region.regionId;
            repaint();

            if (onModuleSelected)
                onModuleSelected(module.id);

            return;
        }
    }

    if (module.id == selectedModuleId && findSelectedRegion() != nullptr)
    {
        for (int buttonIndex = 0; buttonIndex < 6; ++buttonIndex)
        {
            if (editButtonBoundsForLane(lane, buttonIndex).contains(event.position))
            {
                if (onRegionEdit)
                    onRegionEdit(selectedRegionId, editActionForIndex(buttonIndex));
                return;
            }
        }
    }

    selectedModuleId = module.id;
    repaint();

    if (onModuleSelected)
        onModuleSelected(module.id);
}

int ModuleLanesComponent::laneIndexAtPosition(juce::Point<float> position) const
{
    const int laneCount = juce::jmin(3, state.modules.size());
    if (laneCount <= 0)
        return -1;

    for (int i = 0; i < laneCount; ++i)
    {
        if (laneBoundsForIndex(i).contains(position))
            return i;
    }

    return -1;
}

juce::Rectangle<float> ModuleLanesComponent::laneBoundsForIndex(int index) const
{
    auto inner = getLocalBounds().toFloat().reduced(12.0f);
    inner.removeFromTop(28.0f);

    const int laneCount = juce::jmin(3, state.modules.size());
    if (laneCount <= 0)
        return {};

    const float laneGap = 8.0f;
    const float laneHeight = (inner.getHeight() - (laneGap * static_cast<float>(laneCount - 1))) / static_cast<float>(laneCount);

    return { inner.getX(), inner.getY() + ((laneHeight + laneGap) * static_cast<float>(index)), inner.getWidth(), laneHeight };
}

juce::Rectangle<float> ModuleLanesComponent::freezeButtonBoundsForLane(juce::Rectangle<float> lane) const
{
    return { lane.getRight() - 88.0f, lane.getY() + 8.0f, 72.0f, 24.0f };
}

juce::Rectangle<float> ModuleLanesComponent::regionRailBoundsForLane(juce::Rectangle<float> lane) const
{
    return lane.reduced(12.0f, 0.0f).withY(lane.getY() + 45.0f).withHeight(12.0f);
}

juce::Rectangle<float> ModuleLanesComponent::regionBoundsInRail(const RegionEntry& region, juce::Rectangle<float> rail) const
{
    const auto visibleBeats = 16.0;
    auto x = rail.getX() + static_cast<float>((region.startBeat / visibleBeats) * rail.getWidth());
    auto w = juce::jmax(18.0f, static_cast<float>((region.lengthBeats / visibleBeats) * rail.getWidth()));
    return juce::Rectangle<float>(x, rail.getY(), w, rail.getHeight()).getIntersection(rail);
}

juce::Rectangle<float> ModuleLanesComponent::editButtonBoundsForLane(juce::Rectangle<float> lane, int index) const
{
    auto row = lane.reduced(12.0f, 0.0f).withY(lane.getBottom() - 24.0f).withHeight(18.0f);
    const auto buttonWidth = juce::jmin(58.0f, (row.getWidth() - 25.0f) / 6.0f);
    return { row.getX() + static_cast<float>(index) * (buttonWidth + 5.0f), row.getY(), buttonWidth, row.getHeight() };
}

juce::String ModuleLanesComponent::editActionForIndex(int index) const
{
    switch (index)
    {
        case 0: return "move-left";
        case 1: return "move-right";
        case 2: return "trim-shorter";
        case 3: return "trim-longer";
        case 4: return "split";
        case 5: return "delete";
        default: break;
    }

    return {};
}

const RegionEntry* ModuleLanesComponent::findSelectedRegion() const
{
    if (selectedRegionId.isEmpty())
        return nullptr;

    for (const auto& region : regionState.regions)
        if (region.regionId == selectedRegionId)
            return &region;

    return nullptr;
}
