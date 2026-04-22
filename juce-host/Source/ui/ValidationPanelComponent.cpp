#include "ValidationPanelComponent.h"

ValidationPanelComponent::ValidationPanelComponent()
{
    titleLabel.setText("Validation (sclang)", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    summaryLabel.setJustificationType(juce::Justification::centredLeft);

    detailsEditor.setMultiLine(true);
    detailsEditor.setReadOnly(true);
    detailsEditor.setScrollbarsShown(true);
    detailsEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff101419));
    detailsEditor.setColour(juce::TextEditor::textColourId, juce::Colour(0xffd7e2ef));
    detailsEditor.setFont(juce::FontOptions(13.0f));

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(summaryLabel);
    addAndMakeVisible(detailsEditor);

    refreshText();
}

void ValidationPanelComponent::setValidationState(const ValidationState& newState)
{
    state = newState;
    refreshText();
    repaint();
}

void ValidationPanelComponent::resized()
{
    auto area = getLocalBounds().reduced(12);
    auto top = area.removeFromTop(24);
    titleLabel.setBounds(top.removeFromLeft(200));
    summaryLabel.setBounds(top);
    area.removeFromTop(8);
    detailsEditor.setBounds(area);
}

void ValidationPanelComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff171b20));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

    g.setColour(juce::Colour(0xff2b333d));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 8.0f, 1.0f);
}

void ValidationPanelComponent::refreshText()
{
    summaryLabel.setText(state.toSummaryString(), juce::dontSendNotification);

    if (state.findings.isEmpty())
    {
        detailsEditor.setText("No validation findings.\nTransport, clock domains, and module references currently pass the engine checks.", juce::dontSendNotification);
        return;
    }

    juce::StringArray lines;
    for (const auto& finding : state.findings)
    {
        lines.add("[" + finding.severity + "] " + finding.message);
    }

    detailsEditor.setText(lines.joinIntoString("\n"), juce::dontSendNotification);
}
