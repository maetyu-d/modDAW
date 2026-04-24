#include "ModulesPanelComponent.h"

ModulesPanelComponent::ModulesPanelComponent()
{
    titleLabel.setText("Modules", juce::dontSendNotification);
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

void ModulesPanelComponent::setModuleState(const ModuleState& newState)
{
    state = newState;
    refreshText();
    repaint();
}

void ModulesPanelComponent::resized()
{
    auto area = getLocalBounds().reduced(12);
    auto top = area.removeFromTop(24);
    titleLabel.setBounds(top.removeFromLeft(180));
    summaryLabel.setBounds(top);
    area.removeFromTop(8);
    detailsEditor.setBounds(area);
}

void ModulesPanelComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff171b20));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

    g.setColour(juce::Colour(0xff2b333d));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 8.0f, 1.0f);
}

void ModulesPanelComponent::refreshText()
{
    summaryLabel.setText(state.toSummaryString(), juce::dontSendNotification);

    juce::StringArray lines;
    for (const auto& module : state.modules)
    {
        lines.add(module.displayName + " [" + module.id + "]");
        lines.add("  lifecycle=" + module.lifecycleState
                  + " clockDomain=" + module.clockDomainId
                  + " timingMode=" + module.timingMode
                  + " behaviour=" + module.behaviourType);
        if (! module.capabilities.isEmpty())
            lines.add("  capabilities=" + module.capabilities.joinIntoString(", "));
        lines.add("  codeSurface=" + module.codeSurface);
        lines.add({});
    }

    detailsEditor.setText(lines.joinIntoString("\n"), juce::dontSendNotification);
}
