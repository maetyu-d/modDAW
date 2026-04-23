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
    static constexpr int maxLogLines = 300;
    juce::Label titleLabel;
    juce::TextEditor logEditor;
    juce::StringArray logLines;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LogPanelComponent)
};
