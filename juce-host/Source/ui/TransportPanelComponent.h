#pragma once

#include <JuceHeader.h>
#include "../engine/RecoveryState.h"
#include "../engine/RenderState.h"
#include "../engine/TransportState.h"

class TransportPanelComponent final : public juce::Component,
                                      private juce::Button::Listener
{
public:
    using SimpleCallback = std::function<void()>;

    TransportPanelComponent();
    ~TransportPanelComponent() override;

    void setTransportState(const TransportState& newState);
    void setRecoveryState(const RecoveryState& newState);
    void setRenderState(const RenderState& newState);
    void resized() override;
    void paint(juce::Graphics& g) override;

    SimpleCallback onPlayPressed;
    SimpleCallback onStopPressed;
    SimpleCallback onRefreshPressed;
    SimpleCallback onActivateNextBarPressed;
    SimpleCallback onSaveProjectPressed;
    SimpleCallback onLoadProjectPressed;
    SimpleCallback onSceneNextPhrasePressed;
    SimpleCallback onSceneAfterTwoCyclesPressed;
    SimpleCallback onSceneExternalCuePressed;
    SimpleCallback onExternalCuePressed;
    SimpleCallback onPerformanceAccentPressed;
    SimpleCallback onPerformanceCuePressed;
    SimpleCallback onPerformanceLiftPressed;
    SimpleCallback onRenderMixPressed;
    SimpleCallback onRenderStemsPressed;

private:
    void buttonClicked(juce::Button* button) override;
    void updateSummaryText();

    TransportState state;
    RecoveryState recoveryState;
    RenderState renderState;
    juce::Label titleLabel;
    juce::Label summaryLabel;
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::TextButton refreshButton { "Refresh" };
    juce::TextButton activateNextBarButton { "Activate On Next Bar" };
    juce::TextButton saveProjectButton { "Save Project" };
    juce::TextButton loadProjectButton { "Load Project" };
    juce::TextButton sceneNextPhraseButton { "Scene Next Phrase" };
    juce::TextButton sceneAfterCyclesButton { "Scene +2 Cycles" };
    juce::TextButton sceneExternalCueButton { "Wait External Cue" };
    juce::TextButton externalCueButton { "Send Cue" };
    juce::TextButton performanceAccentButton { "Accent [1]" };
    juce::TextButton performanceCueButton { "Cue [2]" };
    juce::TextButton performanceLiftButton { "Lift [3]" };
    juce::TextButton renderMixButton { "Render Mix" };
    juce::TextButton renderStemsButton { "Export Stems" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportPanelComponent)
};
