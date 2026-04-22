#pragma once

#include <JuceHeader.h>

class LogPanelComponent final : public juce::Component
{
public:
    LogPanelComponent();

    void appendLine(const juce::String& line);
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::Label titleLabel;
    juce::TextEditor logEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LogPanelComponent)
};
