#include "CodeSurfaceComponent.h"

CodeSurfaceComponent::CodeSurfaceComponent()
{
    titleLabel.setText("Code Surface (sclang authority)", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));

    moduleLabel.setJustificationType(juce::Justification::centredLeft);
    moduleLabel.setFont(juce::FontOptions(13.5f, juce::Font::bold));
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setFont(juce::FontOptions(12.5f));

    codeEditor.setMultiLine(true);
    codeEditor.setReturnKeyStartsNewLine(true);
    codeEditor.setScrollbarsShown(true);
    codeEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff101419));
    codeEditor.setColour(juce::TextEditor::textColourId, juce::Colour(0xffd7e2ef));
    codeEditor.setColour(juce::TextEditor::highlightColourId, juce::Colour(0xff264a64));
    codeEditor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff38424e));
    codeEditor.setFont(juce::FontOptions(12.5f));
    codeEditor.addListener(this);

    submitButton.addListener(this);

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(moduleLabel);
    addAndMakeVisible(statusLabel);
    addAndMakeVisible(submitButton);
    addAndMakeVisible(codeEditor);

    refreshFromModuleState(true);
}

CodeSurfaceComponent::~CodeSurfaceComponent()
{
    submitButton.removeListener(this);
    codeEditor.removeListener(this);
}

void CodeSurfaceComponent::setSelectedModule(const ModuleEntry* module)
{
    const auto previousModuleId = loadedModuleId;
    hasSelectedModule = (module != nullptr);

    if (module != nullptr)
        selectedModule = *module;

    refreshFromModuleState(module == nullptr || module->id != previousModuleId);
    repaint();
}

void CodeSurfaceComponent::resized()
{
    auto area = getLocalBounds().reduced(12);
    auto top = area.removeFromTop(24);
    titleLabel.setBounds(top.removeFromLeft(250));
    submitButton.setBounds(top.removeFromRight(160));
    area.removeFromTop(6);
    moduleLabel.setBounds(area.removeFromTop(20));
    statusLabel.setBounds(area.removeFromTop(18));
    area.removeFromTop(6);
    codeEditor.setBounds(area);
}

void CodeSurfaceComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff171b20));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

    g.setColour(juce::Colour(0xff2b333d));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 8.0f, 1.0f);
}

void CodeSurfaceComponent::buttonClicked(juce::Button* button)
{
    if (button != &submitButton || ! hasSelectedModule)
        return;

    if (onSubmitPressed)
        onSubmitPressed(selectedModule.id, codeEditor.getText());
}

void CodeSurfaceComponent::textEditorTextChanged(juce::TextEditor&)
{
    editorDirty = (codeEditor.getText() != loadedEngineCodeSurface);
    updateStatusText();
}

void CodeSurfaceComponent::refreshFromModuleState(bool forceReloadEditor)
{
    if (! hasSelectedModule)
    {
        loadedModuleId.clear();
        loadedEngineCodeSurface.clear();
        editorDirty = false;
        moduleLabel.setText("No module selected", juce::dontSendNotification);
        codeEditor.setText({}, juce::dontSendNotification);
        codeEditor.setReadOnly(true);
        submitButton.setEnabled(false);
        updateStatusText();
        return;
    }

    const auto moduleChanged = (selectedModule.id != loadedModuleId);
    loadedModuleId = selectedModule.id;
    moduleLabel.setText(selectedModule.displayName + " [" + selectedModule.id + "]", juce::dontSendNotification);

    const auto authoritativeCode = selectedModule.currentEditableCodeSurface();
    const bool shouldReload = forceReloadEditor
        || (! codeEditor.hasKeyboardFocus(false) && (! editorDirty || codeEditor.getText().isEmpty()));

    loadedEngineCodeSurface = authoritativeCode;

    if (shouldReload || moduleChanged)
    {
        codeEditor.setReadOnly(false);
        codeEditor.setText(authoritativeCode, juce::dontSendNotification);
        editorDirty = false;
    }

    submitButton.setEnabled(true);
    updateStatusText();
}

void CodeSurfaceComponent::updateStatusText()
{
    if (! hasSelectedModule)
    {
        statusLabel.setText("Select a lane to edit one engine-owned code surface.", juce::dontSendNotification);
        return;
    }

    auto status = "state: " + selectedModule.codeSurfaceState;

    if (selectedModule.pendingCodeSwapBarIndex > 0)
        status += " | pending bar " + juce::String(selectedModule.pendingCodeSwapBarIndex);

    if (editorDirty)
        status += " | local edits not yet submitted";

    if (selectedModule.lastCodeEvalMessage.isNotEmpty())
        status += " | " + selectedModule.lastCodeEvalMessage;

    statusLabel.setText(status, juce::dontSendNotification);
}
