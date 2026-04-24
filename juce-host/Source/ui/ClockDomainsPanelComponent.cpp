#include "ClockDomainsPanelComponent.h"

ClockDomainsPanelComponent::ClockDomainsPanelComponent()
{
    titleLabel.setText("Clock Domains", juce::dontSendNotification);
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

void ClockDomainsPanelComponent::setClockDomainState(const ClockDomainState& newState)
{
    state = newState;
    refreshText();
    repaint();
}

void ClockDomainsPanelComponent::resized()
{
    auto area = getLocalBounds().reduced(12);
    auto top = area.removeFromTop(24);
    titleLabel.setBounds(top.removeFromLeft(220));
    summaryLabel.setBounds(top);
    area.removeFromTop(8);
    detailsEditor.setBounds(area);
}

void ClockDomainsPanelComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff171b20));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

    g.setColour(juce::Colour(0xff2b333d));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 8.0f, 1.0f);
}

void ClockDomainsPanelComponent::refreshText()
{
    summaryLabel.setText(state.toSummaryString(), juce::dontSendNotification);

    juce::StringArray lines;
    for (const auto& domain : state.domains)
    {
        lines.add(domain.displayName + " [" + domain.id + "]");
        lines.add("  kind=" + domain.kind
                  + " parent=" + (domain.parentId.isNotEmpty() ? domain.parentId : "none")
                  + " relation=" + (domain.relationType.isNotEmpty() ? domain.relationType : "tempoShared")
                  + " meter=" + juce::String(domain.meterNumerator) + "/" + juce::String(domain.meterDenominator)
                  + " ratio=" + juce::String(domain.ratioToParent, 3)
                  + " phaseOffset=" + juce::String(domain.phaseOffsetBeats, 3));
        if (domain.relationDescription.isNotEmpty())
            lines.add("  " + domain.relationDescription);
        lines.add("  localBar=" + juce::String(domain.localBarIndex)
                  + " localBeat=" + juce::String(domain.localBeatInBar, 2)
                  + " localAbsolute=" + juce::String(domain.localBeatPosition, 2)
                  + " globalAnchor=" + juce::String(domain.absoluteBeatPosition, 2));
        lines.add("  phraseBars=" + juce::String(domain.phraseLengthBars)
                  + " phraseIndex=" + juce::String(domain.phraseIndex)
                  + " phrasePhase=" + juce::String(domain.phrasePhase, 3)
                  + " nextPhraseBeat=" + juce::String(domain.nextPhraseBeat, 2));
        lines.add({});
    }

    detailsEditor.setText(lines.joinIntoString("\n"), juce::dontSendNotification);
}
