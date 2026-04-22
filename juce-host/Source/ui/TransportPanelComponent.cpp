#include "TransportPanelComponent.h"

TransportPanelComponent::TransportPanelComponent()
{
    titleLabel.setText("Canonical Transport (sclang)", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    summaryLabel.setJustificationType(juce::Justification::centredLeft);
    hintLabel.setText("Engine-owned transport and quantised trigger requests.", juce::dontSendNotification);
    hintLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(summaryLabel);
    addAndMakeVisible(hintLabel);
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(refreshButton);
    addAndMakeVisible(activateNextBarButton);

    playButton.addListener(this);
    stopButton.addListener(this);
    refreshButton.addListener(this);
    activateNextBarButton.addListener(this);

    setTransportState(state);
}

TransportPanelComponent::~TransportPanelComponent()
{
    playButton.removeListener(this);
    stopButton.removeListener(this);
    refreshButton.removeListener(this);
    activateNextBarButton.removeListener(this);
}

void TransportPanelComponent::setTransportState(const TransportState& newState)
{
    state = newState;
    summaryLabel.setText(state.toSummaryString(), juce::dontSendNotification);
    repaint();
}

void TransportPanelComponent::resized()
{
    auto area = getLocalBounds().reduced(12);
    titleLabel.setBounds(area.removeFromTop(24));
    area.removeFromTop(4);
    summaryLabel.setBounds(area.removeFromTop(24));
    area.removeFromTop(2);
    hintLabel.setBounds(area.removeFromTop(18));
    area.removeFromTop(10);

    auto topButtons = area.removeFromTop(30);
    auto buttonWidth = (topButtons.getWidth() - 16) / 3;
    playButton.setBounds(topButtons.removeFromLeft(buttonWidth));
    topButtons.removeFromLeft(8);
    stopButton.setBounds(topButtons.removeFromLeft(buttonWidth));
    topButtons.removeFromLeft(8);
    refreshButton.setBounds(topButtons);

    area.removeFromTop(8);
    activateNextBarButton.setBounds(area.removeFromTop(30));
}

void TransportPanelComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff171b20));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

    g.setColour(juce::Colour(0xff2b333d));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 8.0f, 1.0f);
}

void TransportPanelComponent::buttonClicked(juce::Button* button)
{
    if (button == &playButton)
    {
        if (onPlayPressed) onPlayPressed();
        return;
    }

    if (button == &stopButton)
    {
        if (onStopPressed) onStopPressed();
        return;
    }

    if (button == &refreshButton)
    {
        if (onRefreshPressed) onRefreshPressed();
        return;
    }

    if (button == &activateNextBarButton)
    {
        if (onActivateNextBarPressed) onActivateNextBarPressed();
    }
}
