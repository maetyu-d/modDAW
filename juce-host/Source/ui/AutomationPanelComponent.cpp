#include "AutomationPanelComponent.h"

AutomationPanelComponent::AutomationPanelComponent()
{
    titleLabel.setText("Automation (sclang)", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    summaryLabel.setJustificationType(juce::Justification::centredRight);

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(summaryLabel);
    addAndMakeVisible(addLowButton);
    addAndMakeVisible(addHighButton);
    addAndMakeVisible(resetButton);

    addLowButton.addListener(this);
    addHighButton.addListener(this);
    resetButton.addListener(this);

    refreshLabels();
}

AutomationPanelComponent::~AutomationPanelComponent()
{
    addLowButton.removeListener(this);
    addHighButton.removeListener(this);
    resetButton.removeListener(this);
}

void AutomationPanelComponent::setAutomationState(const AutomationState& newState)
{
    state = newState;
    refreshLabels();
    repaint();
}

void AutomationPanelComponent::resized()
{
    auto area = getLocalBounds().reduced(12);
    auto top = area.removeFromTop(24);
    titleLabel.setBounds(top.removeFromLeft(210));
    summaryLabel.setBounds(top);
    area.removeFromTop(58);

    auto buttons = area.removeFromBottom(28);
    auto buttonWidth = (buttons.getWidth() - 16) / 3;
    addLowButton.setBounds(buttons.removeFromLeft(buttonWidth));
    buttons.removeFromLeft(8);
    addHighButton.setBounds(buttons.removeFromLeft(buttonWidth));
    buttons.removeFromLeft(8);
    resetButton.setBounds(buttons);
}

void AutomationPanelComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff171b20));
    g.fillRoundedRectangle(bounds, 8.0f);

    g.setColour(juce::Colour(0xff2b333d));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);

    auto lane = laneBounds();
    g.setColour(juce::Colour(0xff0b0d10));
    g.fillRoundedRectangle(lane, 5.0f);

    if (const auto* automationLane = state.firstLane())
    {
        g.setColour(juce::Colour(0xff8f99a5));
        g.setFont(juce::FontOptions(11.5f));
        g.drawText(automationLane->targetId + "." + automationLane->parameterId + " | " + automationLane->interpolation,
                   lane.withY(lane.getY() - 18.0f).withHeight(15.0f).toNearestInt(),
                   juce::Justification::centredLeft);

        if (automationLane->points.size() > 1)
        {
            juce::Path path;
            bool started = false;

            for (const auto& point : automationLane->points)
            {
                const auto x = lane.getX() + static_cast<float>((point.beat / 16.0) * lane.getWidth());
                const auto y = lane.getBottom() - static_cast<float>((point.value / 1.5) * lane.getHeight());

                if (! started)
                {
                    path.startNewSubPath(x, y);
                    started = true;
                }
                else
                {
                    path.lineTo(x, y);
                }

                g.setColour(juce::Colour(0xffffdf8a));
                g.fillEllipse(x - 3.5f, y - 3.5f, 7.0f, 7.0f);
            }

            g.setColour(juce::Colour(0xffd59f42));
            g.strokePath(path, juce::PathStrokeType(2.0f));
        }
    }
    else
    {
        g.setColour(juce::Colour(0xff8f99a5));
        g.setFont(juce::FontOptions(12.0f));
        g.drawText("No automation lanes from engine.", lane.toNearestInt(), juce::Justification::centred);
    }
}

void AutomationPanelComponent::buttonClicked(juce::Button* button)
{
    const auto* lane = state.firstLane();
    if (lane == nullptr)
        return;

    if (button == &addLowButton)
    {
        if (onAddPoint)
            onAddPoint(lane->laneId, 0.25);
        return;
    }

    if (button == &addHighButton)
    {
        if (onAddPoint)
            onAddPoint(lane->laneId, 1.0);
        return;
    }

    if (button == &resetButton)
    {
        if (onResetDemo)
            onResetDemo(lane->laneId);
    }
}

void AutomationPanelComponent::refreshLabels()
{
    if (const auto* lane = state.firstLane())
        summaryLabel.setText(lane->toSummaryString(), juce::dontSendNotification);
    else
        summaryLabel.setText(state.toSummaryString(), juce::dontSendNotification);
}

juce::Rectangle<float> AutomationPanelComponent::laneBounds() const
{
    return getLocalBounds().toFloat().reduced(12.0f).withTrimmedTop(42.0f).withTrimmedBottom(42.0f);
}
