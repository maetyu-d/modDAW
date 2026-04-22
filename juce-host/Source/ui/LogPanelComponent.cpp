#include "LogPanelComponent.h"

LogPanelComponent::LogPanelComponent()
{
    titleLabel.setText("Engine Log", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);

    logEditor.setMultiLine(true);
    logEditor.setReadOnly(true);
    logEditor.setScrollbarsShown(true);
    logEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff13161a));
    logEditor.setColour(juce::TextEditor::textColourId, juce::Colour(0xffd7e2ef));
    logEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    logEditor.setFont(juce::FontOptions(12.5f));

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(logEditor);
}

void LogPanelComponent::appendLine(const juce::String& line)
{
    logEditor.moveCaretToEnd();
    logEditor.insertTextAtCaret(line + "\n");
    logEditor.moveCaretToEnd();
}

void LogPanelComponent::resized()
{
    auto area = getLocalBounds().reduced(12);
    titleLabel.setBounds(area.removeFromTop(24));
    area.removeFromTop(8);
    logEditor.setBounds(area);
}

void LogPanelComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff171b20));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

    g.setColour(juce::Colour(0xff2b333d));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 8.0f, 1.0f);
}
