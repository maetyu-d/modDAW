#include "MixerPanelComponent.h"

MixerPanelComponent::MixerPanelComponent()
{
    titleLabel.setText("Mixer / Sends (sclang)", juce::dontSendNotification);
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
        strip->groupSelector.removeListener(this);
        strip->sendSlider.removeListener(this);
        strip->sendModeSelector.removeListener(this);
    }
}

void MixerPanelComponent::setMixerState(const MixerState& newState)
{
    if (state.strips.size() == newState.strips.size())
    {
        bool sameStructure = true;

        for (int i = 0; i < state.strips.size(); ++i)
            sameStructure = sameStructure && (state.strips[i].id == newState.strips[i].id);

        if (sameStructure)
        {
            suppressCallbacks = true;
            state = newState;
            summaryLabel.setText(state.toSummaryString(), juce::dontSendNotification);

            for (int i = 0; i < strips.size(); ++i)
            {
                const auto& stripState = state.strips[i];
                auto* strip = strips[i];
                strip->levelSlider.setValue(stripState.level, juce::dontSendNotification);
                strip->muteToggle.setToggleState(stripState.muted, juce::dontSendNotification);
                strip->groupSelector.setText(stripState.assignedGroupId, juce::dontSendNotification);
                if (const auto* send = state.findSendForStrip(stripState.id))
                {
                    strip->sendId = send->sendId;
                    strip->sendSlider.setValue(send->level, juce::dontSendNotification);
                    strip->sendModeSelector.setText(send->mode, juce::dontSendNotification);
                }
            }

            suppressCallbacks = false;
            repaint();
            return;
        }
    }

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
        auto row = area.removeFromTop(54);
        auto topRow = row.removeFromTop(24);
        auto bottomRow = row.removeFromTop(24);
        strip->nameLabel.setBounds(topRow.removeFromLeft(110));
        strip->levelSlider.setBounds(topRow.removeFromLeft(150));
        topRow.removeFromLeft(8);
        strip->muteToggle.setBounds(topRow.removeFromLeft(74));
        topRow.removeFromLeft(8);
        strip->statusLabel.setBounds(topRow);

        bottomRow.removeFromLeft(110);
        strip->groupSelector.setBounds(bottomRow.removeFromLeft(116));
        bottomRow.removeFromLeft(8);
        strip->sendSlider.setBounds(bottomRow.removeFromLeft(150));
        bottomRow.removeFromLeft(8);
        strip->sendModeSelector.setBounds(bottomRow.removeFromLeft(74));
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

    if (auto* strip = findControlsForSendSlider(slider))
        if (onSendLevelChanged && strip->sendId.isNotEmpty())
            onSendLevelChanged(strip->sendId, slider->getValue());
}

void MixerPanelComponent::buttonClicked(juce::Button* button)
{
    if (suppressCallbacks)
        return;

    if (auto* strip = findControlsForButton(button))
        if (onStripMuteChanged)
            onStripMuteChanged(strip->stripId, strip->muteToggle.getToggleState());
}

void MixerPanelComponent::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (suppressCallbacks)
        return;

    if (auto* strip = findControlsForComboBox(comboBox))
    {
        if (comboBox == &strip->groupSelector && onStripGroupChanged)
            onStripGroupChanged(strip->stripId, comboBox->getText() == "(none)" ? juce::String() : comboBox->getText());
        else if (comboBox == &strip->sendModeSelector && onSendModeChanged && strip->sendId.isNotEmpty())
            onSendModeChanged(strip->sendId, comboBox->getText());
    }
}

void MixerPanelComponent::rebuildControls()
{
    suppressCallbacks = true;

    for (auto* strip : strips)
    {
        strip->levelSlider.removeListener(this);
        strip->muteToggle.removeListener(this);
        strip->groupSelector.removeListener(this);
        strip->sendSlider.removeListener(this);
        strip->sendModeSelector.removeListener(this);
        removeChildComponent(&strip->nameLabel);
        removeChildComponent(&strip->levelSlider);
        removeChildComponent(&strip->muteToggle);
        removeChildComponent(&strip->groupSelector);
        removeChildComponent(&strip->sendSlider);
        removeChildComponent(&strip->sendModeSelector);
        removeChildComponent(&strip->statusLabel);
    }

    strips.clear(true);
    summaryLabel.setText(state.toSummaryString(), juce::dontSendNotification);

    for (const auto& stripState : state.strips)
    {
        auto* strip = strips.add(new StripControls());
        strip->stripId = stripState.id;
        if (const auto* send = state.findSendForStrip(stripState.id))
            strip->sendId = send->sendId;

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

        strip->groupSelector.addItem("(none)", 1);
        int groupItemId = 2;
        for (const auto& group : state.groups)
            strip->groupSelector.addItem(group.id, groupItemId++);
        strip->groupSelector.setEnabled(stripState.kind == "module");
        strip->groupSelector.setText(stripState.assignedGroupId.isNotEmpty() ? stripState.assignedGroupId : juce::String("(none)"),
                                     juce::dontSendNotification);
        strip->groupSelector.addListener(this);

        strip->sendSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        strip->sendSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 46, 18);
        strip->sendSlider.setRange(0.0, 1.0, 0.01);
        strip->sendSlider.setEnabled(strip->sendId.isNotEmpty());
        strip->sendSlider.setValue(strip->sendId.isNotEmpty() ? state.findSendForStrip(stripState.id)->level : 0.0,
                                   juce::dontSendNotification);
        strip->sendSlider.addListener(this);

        strip->sendModeSelector.addItem("post", 1);
        strip->sendModeSelector.addItem("pre", 2);
        strip->sendModeSelector.setEnabled(strip->sendId.isNotEmpty());
        strip->sendModeSelector.setText(strip->sendId.isNotEmpty() ? state.findSendForStrip(stripState.id)->mode : juce::String("post"),
                                        juce::dontSendNotification);
        strip->sendModeSelector.addListener(this);

        auto status = juce::String(stripState.kind == "master" ? "master bus" : (stripState.kind == "group" ? "group bus" : "module strip"));
        status += stripState.hasAudioPath ? " | live" : " | placeholder";
        if (stripState.kind == "group")
            status += " | " + juce::String(stripState.childCount) + " children";
        else if (stripState.kind == "return")
            status += " | shared FX";
        else if (stripState.assignedGroupId.isNotEmpty())
            status += " | -> " + stripState.assignedGroupId;
        strip->statusLabel.setText(status, juce::dontSendNotification);
        strip->statusLabel.setJustificationType(juce::Justification::centredLeft);
        strip->statusLabel.setFont(juce::FontOptions(12.0f));

        addAndMakeVisible(strip->nameLabel);
        addAndMakeVisible(strip->levelSlider);
        addAndMakeVisible(strip->muteToggle);
        addAndMakeVisible(strip->groupSelector);
        addAndMakeVisible(strip->sendSlider);
        addAndMakeVisible(strip->sendModeSelector);
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

MixerPanelComponent::StripControls* MixerPanelComponent::findControlsForComboBox(juce::ComboBox* comboBox)
{
    for (auto* strip : strips)
        if (&strip->groupSelector == comboBox)
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

MixerPanelComponent::StripControls* MixerPanelComponent::findControlsForSendSlider(juce::Slider* slider)
{
    for (auto* strip : strips)
        if (&strip->sendSlider == slider)
            return strip;

    return nullptr;
}
