#pragma once

#include <JuceHeader.h>
#include "ui/MainComponent.h"

class MainWindow final : public juce::DocumentWindow
{
public:
    explicit MainWindow(const juce::String& name);
    void closeButtonPressed() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};
