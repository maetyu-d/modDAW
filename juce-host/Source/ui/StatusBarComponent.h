#pragma once

#include <JuceHeader.h>
#include "../engine/EngineProcessManager.h"

class StatusBarComponent final : public juce::Component
{
public:
    StatusBarComponent();

    void setConnectionState(EngineProcessManager::ConnectionState newState);
    void resized() override;

private:
    juce::Label titleLabel;
    juce::Label valueLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatusBarComponent)
};
