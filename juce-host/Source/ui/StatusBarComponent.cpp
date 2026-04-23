#include "StatusBarComponent.h"

namespace
{
juce::String stateText(EngineProcessManager::ConnectionState state)
{
    switch (state)
    {
        case EngineProcessManager::ConnectionState::offline: return "offline";
        case EngineProcessManager::ConnectionState::booting: return "booting";
        case EngineProcessManager::ConnectionState::ready: return "ready";
    }

    return "offline";
}

juce::Colour stateColour(EngineProcessManager::ConnectionState state)
{
    switch (state)
    {
        case EngineProcessManager::ConnectionState::offline: return juce::Colours::indianred;
        case EngineProcessManager::ConnectionState::booting: return juce::Colours::goldenrod;
        case EngineProcessManager::ConnectionState::ready: return juce::Colours::seagreen;
    }

    return juce::Colours::grey;
}
}

StatusBarComponent::StatusBarComponent()
{
    titleLabel.setText("Engine Status", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::FontOptions(13.0f));
    addAndMakeVisible(titleLabel);

    valueLabel.setJustificationType(juce::Justification::centredLeft);
    valueLabel.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    addAndMakeVisible(valueLabel);

    setConnectionState(EngineProcessManager::ConnectionState::offline);
}

void StatusBarComponent::setConnectionState(EngineProcessManager::ConnectionState newState)
{
    valueLabel.setText(stateText(newState), juce::dontSendNotification);
    valueLabel.setColour(juce::Label::textColourId, stateColour(newState));
}

void StatusBarComponent::resized()
{
    auto area = getLocalBounds().reduced(8, 6);
    titleLabel.setBounds(area.removeFromLeft(120));
    valueLabel.setBounds(area.removeFromLeft(120));
}
