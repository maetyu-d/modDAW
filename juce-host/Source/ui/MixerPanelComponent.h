#pragma once

#include <JuceHeader.h>
#include "../engine/MixerState.h"

class MixerPanelComponent final : public juce::Component,
                                  private juce::Slider::Listener,
                                  private juce::Button::Listener
{
public:
    MixerPanelComponent();
    ~MixerPanelComponent() override;

    void setMixerState(const MixerState& newState);

    std::function<void(const juce::String&, double)> onStripLevelChanged;
    std::function<void(const juce::String&, bool)> onStripMuteChanged;

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    struct StripControls
    {
        juce::String stripId;
        juce::Label nameLabel;
        juce::Slider levelSlider;
        juce::ToggleButton muteToggle;
        juce::Label statusLabel;
    };

    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;
    void rebuildControls();
    StripControls* findControlsForSlider(juce::Slider* slider);
    StripControls* findControlsForButton(juce::Button* button);

    juce::Label titleLabel;
    juce::Label summaryLabel;
    MixerState state;
    juce::OwnedArray<StripControls> strips;
    bool suppressCallbacks = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerPanelComponent)
};
