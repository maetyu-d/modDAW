#pragma once

#include <JuceHeader.h>
#include "../engine/TransportState.h"

class TransportPanelComponent final : public juce::Component,
                                      private juce::Button::Listener
{
public:
    using SimpleCallback = std::function<void()>;

    TransportPanelComponent();
    ~TransportPanelComponent() override;

    void setTransportState(const TransportState& newState);
    void resized() override;
    void paint(juce::Graphics& g) override;

    SimpleCallback onPlayPressed;
    SimpleCallback onStopPressed;
    SimpleCallback onRefreshPressed;
    SimpleCallback onActivateNextBarPressed;
    SimpleCallback onSaveProjectPressed;
    SimpleCallback onLoadProjectPressed;

private:
    void buttonClicked(juce::Button* button) override;

    TransportState state;
    juce::Label titleLabel;
    juce::Label summaryLabel;
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::TextButton refreshButton { "Refresh" };
    juce::TextButton activateNextBarButton { "Activate On Next Bar" };
    juce::TextButton saveProjectButton { "Save Project" };
    juce::TextButton loadProjectButton { "Load Project" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportPanelComponent)
};
