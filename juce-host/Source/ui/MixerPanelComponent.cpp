#include "MixerPanelComponent.h"

MixerPanelComponent::MixerPanelComponent()
{
    titleLabel.setText("Minimal Mixer (sclang)", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    summaryLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(summaryLabel);
    rebuildControls();
}

MixerPanelComponent::~MixerPanelComponent()
{
    for (auto* strip : strips)
    {
        strip->levelSlider.removeListener(this);
        strip->muteToggle.removeListener(this);
    }
}

void MixerPanelComponent::setMixerState(const MixerState& newState)
{
    state = newState;
    rebuildControls();
    repaint();
}

void MixerPanelComponent::resized()
{
    auto area = getLocalBounds().reduced(12);
    auto top = area.removeFromTop(24);
    titleLabel.setBounds(top.removeFromLeft(220));
    summaryLabel.setBounds(top);
    area.removeFromTop(8);

    for (auto* strip : strips)
    {
        auto row = area.removeFromTop(36);
        strip->nameLabel.setBounds(row.removeFromLeft(110));
        strip->levelSlider.setBounds(row.removeFromLeft(200));
        row.removeFromLeft(8);
        strip->muteToggle.setBounds(row.removeFromLeft(74));
        row.removeFromLeft(8);
        strip->statusLabel.setBounds(row);
        area.removeFromTop(6);
    }
}

void MixerPanelComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff171b20));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

    g.setColour(juce::Colour(0xff2b333d));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 8.0f, 1.0f);
}

void MixerPanelComponent::sliderValueChanged(juce::Slider* slider)
{
    if (suppressCallbacks)
        return;

    if (auto* strip = findControlsForSlider(slider))
        if (onStripLevelChanged)
            onStripLevelChanged(strip->stripId, slider->getValue());
}

void MixerPanelComponent::buttonClicked(juce::Button* button)
{
    if (suppressCallbacks)
        return;

    if (auto* strip = findControlsForButton(button))
        if (onStripMuteChanged)
            onStripMuteChanged(strip->stripId, strip->muteToggle.getToggleState());
}

void MixerPanelComponent::rebuildControls()
{
    suppressCallbacks = true;

    for (auto* strip : strips)
    {
        strip->levelSlider.removeListener(this);
        strip->muteToggle.removeListener(this);
        removeChildComponent(&strip->nameLabel);
        removeChildComponent(&strip->levelSlider);
        removeChildComponent(&strip->muteToggle);
        removeChildComponent(&strip->statusLabel);
    }

    strips.clear(true);
    summaryLabel.setText(state.toSummaryString(), juce::dontSendNotification);

    for (const auto& stripState : state.strips)
    {
        auto* strip = strips.add(new StripControls());
        strip->stripId = stripState.id;
        strip->nameLabel.setText(stripState.displayName, juce::dontSendNotification);
        strip->nameLabel.setJustificationType(juce::Justification::centredLeft);
        strip->nameLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));

        strip->levelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        strip->levelSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 20);
        strip->levelSlider.setRange(0.0, 1.5, 0.01);
        strip->levelSlider.setValue(stripState.level, juce::dontSendNotification);
        strip->levelSlider.addListener(this);

        strip->muteToggle.setButtonText("Mute");
        strip->muteToggle.setToggleState(stripState.muted, juce::dontSendNotification);
        strip->muteToggle.addListener(this);

        auto status = juce::String(stripState.kind == "master" ? "master bus" : "module strip");
        status += stripState.hasAudioPath ? " | live" : " | placeholder";
        strip->statusLabel.setText(status, juce::dontSendNotification);
        strip->statusLabel.setJustificationType(juce::Justification::centredLeft);
        strip->statusLabel.setFont(juce::FontOptions(12.0f));

        addAndMakeVisible(strip->nameLabel);
        addAndMakeVisible(strip->levelSlider);
        addAndMakeVisible(strip->muteToggle);
        addAndMakeVisible(strip->statusLabel);
    }

    suppressCallbacks = false;
    resized();
}

MixerPanelComponent::StripControls* MixerPanelComponent::findControlsForSlider(juce::Slider* slider)
{
    for (auto* strip : strips)
        if (&strip->levelSlider == slider)
            return strip;

    return nullptr;
}

MixerPanelComponent::StripControls* MixerPanelComponent::findControlsForButton(juce::Button* button)
{
    for (auto* strip : strips)
        if (&strip->muteToggle == button)
            return strip;

    return nullptr;
}
