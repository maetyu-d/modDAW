#include "TimingInspectorComponent.h"

TimingInspectorComponent::TimingInspectorComponent()
{
    titleLabel.setText("Timing Inspector", juce::dontSendNotification);
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

void TimingInspectorComponent::setInspectorState(const TransportState& transportState,
                                                 const ModuleEntry* selectedModule,
                                                 const ClockDomainEntry* selectedClockDomain)
{
    transport = transportState;
    hasSelectedModule = (selectedModule != nullptr);
    hasSelectedClockDomain = (selectedClockDomain != nullptr);

    if (selectedModule != nullptr)
        module = *selectedModule;

    if (selectedClockDomain != nullptr)
        domain = *selectedClockDomain;

    refreshText();
    repaint();
}

void TimingInspectorComponent::resized()
{
    auto area = getLocalBounds().reduced(12);
    auto top = area.removeFromTop(24);
    titleLabel.setBounds(top.removeFromLeft(200));
    summaryLabel.setBounds(top);
    area.removeFromTop(8);
    detailsEditor.setBounds(area);
}

void TimingInspectorComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff171b20));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

    g.setColour(juce::Colour(0xff2b333d));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 8.0f, 1.0f);
}

void TimingInspectorComponent::refreshText()
{
    if (! hasSelectedModule || ! hasSelectedClockDomain)
    {
        summaryLabel.setText("Select a lane to inspect engine timing.", juce::dontSendNotification);
        detailsEditor.setText("No module selected.\nPick a lane to inspect transport, clock ratio, and next-bar relation.", juce::dontSendNotification);
        return;
    }

    summaryLabel.setText(module.displayName + " on " + domain.displayName, juce::dontSendNotification);

    const auto beatsPerBar = static_cast<double>(juce::jmax(1, transport.meterNumerator));
    const auto globalBeatAtNextBar = static_cast<double>(transport.barIndex) * beatsPerBar;
    const auto beatsUntilNextBar = juce::jmax(0.0, globalBeatAtNextBar - transport.beatPosition);
    const auto selectedDomainBeatAtCurrentGlobalBeat = domain.localBeatPosition;
    const auto selectedDomainBeatAtNextBar = domain.phaseOffsetBeats + (globalBeatAtNextBar * domain.ratioToParent);
    const auto selectedDomainDeltaToNextBar = selectedDomainBeatAtNextBar - selectedDomainBeatAtCurrentGlobalBeat;

    juce::StringArray lines;
    lines.add("module: " + module.displayName + " | " + module.lifecycleState + " | " + module.behaviourType);
    lines.add("transport: " + juce::String(transport.isPlaying ? "playing" : "stopped")
              + " | " + formatDouble(transport.tempo, 2) + " bpm"
              + " | " + juce::String(transport.meterNumerator) + "/" + juce::String(transport.meterDenominator));
    lines.add("global: bar " + juce::String(transport.barIndex)
              + " | beat " + formatDouble(transport.beatInBar, 3)
              + " | absolute " + formatDouble(transport.beatPosition, 3));
    lines.add("clock: " + domain.displayName
              + " | ratio " + formatDouble(domain.ratioToParent, 3)
              + " | phase " + formatDouble(domain.phaseOffsetBeats, 3));
    lines.add("local: bar " + juce::String(domain.localBarIndex)
              + " | beat " + formatDouble(domain.localBeatInBar, 3)
              + " | absolute " + formatDouble(domain.localBeatPosition, 3));
    lines.add("next global bar: in " + formatDouble(beatsUntilNextBar, 3)
              + " beats | local delta " + formatDouble(selectedDomainDeltaToNextBar, 3));

    detailsEditor.setText(lines.joinIntoString("\n"), juce::dontSendNotification);
}

juce::String TimingInspectorComponent::formatDouble(double value, int decimals)
{
    return juce::String(value, decimals);
}
