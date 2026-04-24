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

    relationSelector.addItem("tempoShared", 1);
    relationSelector.addItem("meterShared", 2);
    relationSelector.addItem("phaseShared", 3);
    relationSelector.addItem("phaseOffset", 4);
    relationSelector.addItem("hardSync", 5);
    relationSelector.setSelectedId(1, juce::dontSendNotification);

    phaseOffsetEditor.setText("0.000", juce::dontSendNotification);
    phaseOffsetEditor.setInputRestrictions(12, "0123456789.-");
    phaseOffsetEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff101419));
    phaseOffsetEditor.setColour(juce::TextEditor::textColourId, juce::Colour(0xffd7e2ef));
    phaseOffsetEditor.setFont(juce::FontOptions(12.0f));

    applyRelationButton.addListener(this);

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(summaryLabel);
    addAndMakeVisible(relationSelector);
    addAndMakeVisible(phaseOffsetEditor);
    addAndMakeVisible(applyRelationButton);
    addAndMakeVisible(detailsEditor);

    refreshText();
}

void TimingInspectorComponent::setInspectorState(const TransportState& transportState,
                                                 const ModuleEntry* selectedModule,
                                                 const ClockDomainEntry* selectedClockDomain,
                                                 const StructuralState& structuralState,
                                                 const AnalysisState& analysisState)
{
    transport = transportState;
    structural = structuralState;
    analysis = analysisState;
    hasSelectedModule = (selectedModule != nullptr);
    hasSelectedClockDomain = (selectedClockDomain != nullptr);

    if (selectedModule != nullptr)
        module = *selectedModule;

    if (selectedClockDomain != nullptr)
        domain = *selectedClockDomain;

    refreshRelationEditor();
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
    auto editRow = area.removeFromTop(26);
    relationSelector.setBounds(editRow.removeFromLeft(130));
    editRow.removeFromLeft(8);
    phaseOffsetEditor.setBounds(editRow.removeFromLeft(74));
    editRow.removeFromLeft(8);
    applyRelationButton.setBounds(editRow.removeFromLeft(72));
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

void TimingInspectorComponent::buttonClicked(juce::Button* button)
{
    if (button == &applyRelationButton && hasSelectedClockDomain && onRelationChangeRequested)
        onRelationChangeRequested(domain.id, relationSelector.getText(), phaseOffsetEditor.getText().getDoubleValue());
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
    const auto selectedDomainDeltaToNextBar = domain.kind == "free"
        ? 0.0
        : ((globalBeatAtNextBar - transport.beatPosition) * juce::jmax(0.001, domain.ratioToParent)
           * (static_cast<double>(juce::jmax(1, domain.meterDenominator)) / 4.0));

    juce::StringArray lines;
    lines.add("module: " + module.displayName + " | " + module.lifecycleState + " | " + module.behaviourType);
    lines.add("transport: " + juce::String(transport.isPlaying ? "playing" : "stopped")
              + " | " + formatDouble(transport.tempo, 2) + " bpm"
              + " | " + juce::String(transport.meterNumerator) + "/" + juce::String(transport.meterDenominator));
    lines.add("global: bar " + juce::String(transport.barIndex)
              + " | beat " + formatDouble(transport.beatInBar, 3)
              + " | absolute " + formatDouble(transport.beatPosition, 3));
    lines.add("clock: " + domain.displayName
              + " | relation " + (domain.relationType.isNotEmpty() ? domain.relationType : "tempoShared")
              + " | meter " + juce::String(domain.meterNumerator) + "/" + juce::String(domain.meterDenominator)
              + " | ratio " + formatDouble(domain.ratioToParent, 3)
              + " | phase " + formatDouble(domain.phaseOffsetBeats, 3));
    if (domain.relationDescription.isNotEmpty())
        lines.add("relation: " + domain.relationDescription);
    lines.add("local: bar " + juce::String(domain.localBarIndex)
              + " | beat " + formatDouble(domain.localBeatInBar, 3)
              + " | absolute " + formatDouble(domain.localBeatPosition, 3));
    lines.add("global beat anchor: " + formatDouble(domain.absoluteBeatPosition, 3)
              + " | bar length " + formatDouble(domain.barLengthBeats, 3));
    lines.add("phrase: " + juce::String(domain.phraseLengthBars) + " bars"
              + " | index " + juce::String(domain.phraseIndex)
              + " | phase " + formatDouble(domain.phrasePhase, 3)
              + " | next beat " + formatDouble(domain.nextPhraseBeat, 3));
    if (domain.kind == "free")
        lines.add("next global bar: global in " + formatDouble(beatsUntilNextBar, 3) + " beats | local delta independent");
    else
        lines.add("next global bar: in " + formatDouble(beatsUntilNextBar, 3)
                  + " beats | local delta " + formatDouble(selectedDomainDeltaToNextBar, 3));

    if (const auto* pending = structural.nextPendingTransition())
    {
        lines.add("");
        lines.add("pending scene: " + pending->toSummaryString());
    }

    if (const auto* analysisModule = analysis.firstModule())
    {
        lines.add("");
        lines.add("analysis: " + analysisModule->toSummaryString());
        lines.add("analysis route: " + analysisModule->routing);
        lines.add("analysis targets: " + analysisModule->targetModuleIds.joinIntoString(", "));
    }

    if (module.analysisSummary.isNotEmpty())
        lines.add("module analysis summary: " + module.analysisSummary);

    if (module.lastAnalysisInfluence.isNotEmpty())
        lines.add("analysis influence: " + module.lastAnalysisInfluence);

    if (const auto* latest = structural.latestDirective())
    {
        lines.add("");
        lines.add("structural directive: " + latest->toSummaryString());
        lines.add("id: " + latest->directiveId
                  + " | emitter " + latest->emitterModuleId
                  + " | phrase " + juce::String(latest->phraseIndex)
                  + " | beat " + formatDouble(latest->beat, 3));
        lines.add("targets: " + latest->targetModuleIds.joinIntoString(", "));
        lines.add("selected module influence: " + (module.lastStructuralDirective.isNotEmpty()
            ? module.lastStructuralDirective
            : juce::String("none")));
    }

    detailsEditor.setText(lines.joinIntoString("\n"), juce::dontSendNotification);
}

void TimingInspectorComponent::refreshRelationEditor()
{
    if (! hasSelectedClockDomain)
    {
        relationSelector.setEnabled(false);
        phaseOffsetEditor.setEnabled(false);
        applyRelationButton.setEnabled(false);
        return;
    }

    const bool editable = domain.kind != "global" && domain.kind != "free";
    relationSelector.setEnabled(editable);
    phaseOffsetEditor.setEnabled(editable);
    applyRelationButton.setEnabled(editable);

    const auto relationType = domain.relationType.isNotEmpty() ? domain.relationType : juce::String("tempoShared");
    if (relationSelector.getText() != relationType)
        relationSelector.setText(relationType, juce::dontSendNotification);

    if (! phaseOffsetEditor.hasKeyboardFocus(true))
        phaseOffsetEditor.setText(formatDouble(domain.phaseOffsetBeats, 3), juce::dontSendNotification);
}

juce::String TimingInspectorComponent::formatDouble(double value, int decimals)
{
    return juce::String(value, decimals);
}
