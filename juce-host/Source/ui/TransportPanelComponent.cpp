#include "TransportPanelComponent.h"

TransportPanelComponent::TransportPanelComponent()
{
    titleLabel.setText("Canonical Transport (sclang)", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    summaryLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(summaryLabel);
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(refreshButton);
    addAndMakeVisible(activateNextBarButton);
    addAndMakeVisible(saveProjectButton);
    addAndMakeVisible(loadProjectButton);
    addAndMakeVisible(sceneNextPhraseButton);
    addAndMakeVisible(sceneAfterCyclesButton);
    addAndMakeVisible(sceneExternalCueButton);
    addAndMakeVisible(externalCueButton);
    addAndMakeVisible(performanceAccentButton);
    addAndMakeVisible(performanceCueButton);
    addAndMakeVisible(performanceLiftButton);

    playButton.addListener(this);
    stopButton.addListener(this);
    refreshButton.addListener(this);
    activateNextBarButton.addListener(this);
    saveProjectButton.addListener(this);
    loadProjectButton.addListener(this);
    sceneNextPhraseButton.addListener(this);
    sceneAfterCyclesButton.addListener(this);
    sceneExternalCueButton.addListener(this);
    externalCueButton.addListener(this);
    performanceAccentButton.addListener(this);
    performanceCueButton.addListener(this);
    performanceLiftButton.addListener(this);

    setTransportState(state);
}

TransportPanelComponent::~TransportPanelComponent()
{
    playButton.removeListener(this);
    stopButton.removeListener(this);
    refreshButton.removeListener(this);
    activateNextBarButton.removeListener(this);
    saveProjectButton.removeListener(this);
    loadProjectButton.removeListener(this);
    sceneNextPhraseButton.removeListener(this);
    sceneAfterCyclesButton.removeListener(this);
    sceneExternalCueButton.removeListener(this);
    externalCueButton.removeListener(this);
    performanceAccentButton.removeListener(this);
    performanceCueButton.removeListener(this);
    performanceLiftButton.removeListener(this);
}

void TransportPanelComponent::setTransportState(const TransportState& newState)
{
    state = newState;
    updateSummaryText();
    repaint();
}

void TransportPanelComponent::setRecoveryState(const RecoveryState& newState)
{
    recoveryState = newState;
    updateSummaryText();
    repaint();
}

void TransportPanelComponent::resized()
{
    auto area = getLocalBounds().reduced(12);
    titleLabel.setBounds(area.removeFromTop(24));
    area.removeFromTop(4);
    summaryLabel.setBounds(area.removeFromTop(24));
    area.removeFromTop(12);

    auto topButtons = area.removeFromTop(30);
    auto buttonWidth = (topButtons.getWidth() - 16) / 3;
    playButton.setBounds(topButtons.removeFromLeft(buttonWidth));
    topButtons.removeFromLeft(8);
    stopButton.setBounds(topButtons.removeFromLeft(buttonWidth));
    topButtons.removeFromLeft(8);
    refreshButton.setBounds(topButtons);

    area.removeFromTop(8);
    auto lowerButtons = area.removeFromTop(30);
    activateNextBarButton.setBounds(lowerButtons.removeFromLeft(juce::roundToInt(static_cast<float>(lowerButtons.getWidth()) * 0.50f)));
    lowerButtons.removeFromLeft(8);
    saveProjectButton.setBounds(lowerButtons.removeFromLeft((lowerButtons.getWidth() - 8) / 2));
    lowerButtons.removeFromLeft(8);
    loadProjectButton.setBounds(lowerButtons);

    area.removeFromTop(8);
    auto sceneRow = area.removeFromTop(28);
    auto sceneButtonWidth = (sceneRow.getWidth() - 24) / 4;
    sceneNextPhraseButton.setBounds(sceneRow.removeFromLeft(sceneButtonWidth));
    sceneRow.removeFromLeft(8);
    sceneAfterCyclesButton.setBounds(sceneRow.removeFromLeft(sceneButtonWidth));
    sceneRow.removeFromLeft(8);
    sceneExternalCueButton.setBounds(sceneRow.removeFromLeft(sceneButtonWidth));
    sceneRow.removeFromLeft(8);
    externalCueButton.setBounds(sceneRow);

    area.removeFromTop(8);
    auto performanceRow = area.removeFromTop(28);
    auto performanceButtonWidth = (performanceRow.getWidth() - 16) / 3;
    performanceAccentButton.setBounds(performanceRow.removeFromLeft(performanceButtonWidth));
    performanceRow.removeFromLeft(8);
    performanceCueButton.setBounds(performanceRow.removeFromLeft(performanceButtonWidth));
    performanceRow.removeFromLeft(8);
    performanceLiftButton.setBounds(performanceRow);
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
        return;
    }

    if (button == &saveProjectButton)
    {
        if (onSaveProjectPressed) onSaveProjectPressed();
        return;
    }

    if (button == &loadProjectButton)
    {
        if (onLoadProjectPressed) onLoadProjectPressed();
        return;
    }

    if (button == &sceneNextPhraseButton)
    {
        if (onSceneNextPhrasePressed) onSceneNextPhrasePressed();
        return;
    }

    if (button == &sceneAfterCyclesButton)
    {
        if (onSceneAfterTwoCyclesPressed) onSceneAfterTwoCyclesPressed();
        return;
    }

    if (button == &sceneExternalCueButton)
    {
        if (onSceneExternalCuePressed) onSceneExternalCuePressed();
        return;
    }

    if (button == &externalCueButton)
    {
        if (onExternalCuePressed) onExternalCuePressed();
        return;
    }

    if (button == &performanceAccentButton)
    {
        if (onPerformanceAccentPressed) onPerformanceAccentPressed();
        return;
    }

    if (button == &performanceCueButton)
    {
        if (onPerformanceCuePressed) onPerformanceCuePressed();
        return;
    }

    if (button == &performanceLiftButton)
    {
        if (onPerformanceLiftPressed) onPerformanceLiftPressed();
    }
}

void TransportPanelComponent::updateSummaryText()
{
    auto summary = state.toSummaryString();

    if (recoveryState.engineOnline)
    {
        summary += recoveryState.projectDirty ? " | unsaved autosave active" : " | saved";

        if (! recoveryState.moduleErrors.isEmpty())
            summary += " | " + juce::String(recoveryState.moduleErrors.size()) + " isolated error";
    }

    summaryLabel.setText(summary, juce::dontSendNotification);
}
