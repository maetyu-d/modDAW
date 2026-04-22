#pragma once

#include <JuceHeader.h>
#include "../engine/ModuleState.h"

class CodeSurfaceComponent final : public juce::Component,
                                   private juce::Button::Listener,
                                   private juce::TextEditor::Listener
{
public:
    using SubmitCallback = std::function<void(const juce::String&, const juce::String&)>;

    CodeSurfaceComponent();
    ~CodeSurfaceComponent() override;

    void setSelectedModule(const ModuleEntry* module);
    void resized() override;
    void paint(juce::Graphics& g) override;

    SubmitCallback onSubmitPressed;

private:
    void buttonClicked(juce::Button* button) override;
    void textEditorTextChanged(juce::TextEditor&) override;
    void refreshFromModuleState(bool forceReloadEditor);
    void updateStatusText();

    bool hasSelectedModule = false;
    ModuleEntry selectedModule;
    juce::String loadedModuleId;
    juce::String loadedEngineCodeSurface;
    bool editorDirty = false;

    juce::Label titleLabel;
    juce::Label moduleLabel;
    juce::Label statusLabel;
    juce::TextButton submitButton { "Hot-Swap Next Bar" };
    juce::TextEditor codeEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CodeSurfaceComponent)
};
