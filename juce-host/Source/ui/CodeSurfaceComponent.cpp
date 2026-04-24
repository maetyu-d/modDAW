#include "CodeSurfaceComponent.h"

CodeSurfaceComponent::CodeSurfaceComponent()
{
    titleLabel.setText("Code", juce::dontSendNotification);
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
    surfaceSelector.addListener(this);

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(moduleLabel);
    addAndMakeVisible(statusLabel);
    addAndMakeVisible(surfaceSelector);
    addAndMakeVisible(submitButton);
    addAndMakeVisible(codeEditor);

    refreshFromModuleState(true);
}

CodeSurfaceComponent::~CodeSurfaceComponent()
{
    submitButton.removeListener(this);
    surfaceSelector.removeListener(this);
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
    titleLabel.setBounds(top.removeFromLeft(120));
    submitButton.setBounds(top.removeFromRight(132));
    top.removeFromRight(8);
    surfaceSelector.setBounds(top.removeFromRight(132));
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
        onSubmitPressed(selectedModule.id, selectedSurfaceId, codeEditor.getText());
}

void CodeSurfaceComponent::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox != &surfaceSelector)
        return;

    const auto text = surfaceSelector.getText();
    auto nextSurfaceId = text.fromLastOccurrenceOf("[", false, false)
                            .upToFirstOccurrenceOf("]", false, false)
                            .trim();

    if (nextSurfaceId.isEmpty())
        return;

    selectedSurfaceId = nextSurfaceId;
    refreshFromModuleState(true);
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
        selectedSurfaceId = "pattern";
        loadedEngineCodeSurface.clear();
        editorDirty = false;
        moduleLabel.setText("No module selected", juce::dontSendNotification);
        codeEditor.setText({}, juce::dontSendNotification);
        codeEditor.setReadOnly(true);
        submitButton.setEnabled(false);
        surfaceSelector.clear(juce::dontSendNotification);
        surfaceSelector.setEnabled(false);
        updateStatusText();
        return;
    }

    const auto moduleChanged = (selectedModule.id != loadedModuleId);
    loadedModuleId = selectedModule.id;
    if (moduleChanged && selectedModule.findSurfaceById(selectedSurfaceId) == nullptr)
        selectedSurfaceId = selectedModule.defaultSurface() != nullptr ? selectedModule.defaultSurface()->surfaceId : "pattern";

    refreshSurfaceSelector();
    moduleLabel.setText(selectedModule.displayName + " [" + selectedModule.id + "]", juce::dontSendNotification);

    const auto* surface = selectedSurface();
    const auto authoritativeCode = surface != nullptr ? surface->currentEditableCode() : selectedModule.currentEditableCodeSurface();
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
    surfaceSelector.setEnabled(true);
    updateStatusText();
}

void CodeSurfaceComponent::refreshSurfaceSelector()
{
    const auto previousSurfaceId = selectedSurfaceId;
    surfaceSelector.clear(juce::dontSendNotification);

    int itemId = 1;
    for (const auto& surface : selectedModule.codeSurfaces)
    {
        surfaceSelector.addItem(surface.displayName + " [" + surface.surfaceId + "]", itemId++);
    }

    if (selectedModule.codeSurfaces.isEmpty())
        surfaceSelector.addItem("Pattern [pattern]", 1);

    for (int i = 0; i < surfaceSelector.getNumItems(); ++i)
    {
        const auto text = surfaceSelector.getItemText(i);
        const auto itemSurfaceId = text.fromLastOccurrenceOf("[", false, false)
                                       .upToFirstOccurrenceOf("]", false, false)
                                       .trim();
        if (itemSurfaceId == previousSurfaceId)
        {
            surfaceSelector.setSelectedItemIndex(i, juce::dontSendNotification);
            return;
        }
    }

    surfaceSelector.setSelectedItemIndex(0, juce::dontSendNotification);
    selectedSurfaceId = surfaceSelector.getText().fromLastOccurrenceOf("[", false, false)
                                        .upToFirstOccurrenceOf("]", false, false)
                                        .trim();
}

void CodeSurfaceComponent::updateStatusText()
{
    if (! hasSelectedModule)
    {
        statusLabel.setText("Select a lane to edit.", juce::dontSendNotification);
        return;
    }

    const auto* surface = selectedSurface();
    auto status = selectedSurfaceId + " | "
                + (surface != nullptr ? surface->state : selectedModule.codeSurfaceState);

    const auto pendingBar = surface != nullptr ? surface->pendingCodeSwapBarIndex : selectedModule.pendingCodeSwapBarIndex;
    if (pendingBar > 0)
        status += " | next bar " + juce::String(pendingBar);

    if (surface != nullptr)
        status += " | rev " + juce::String(surface->revision);

    if (editorDirty)
        status += " | edited";

    const auto diagnostic = surface != nullptr ? surface->diagnostic : selectedModule.lastCodeEvalMessage;
    if (diagnostic.isNotEmpty())
        status += " | " + diagnostic;

    statusLabel.setText(status, juce::dontSendNotification);
}

const CodeSurfaceEntry* CodeSurfaceComponent::selectedSurface() const
{
    return selectedModule.findSurfaceById(selectedSurfaceId);
}
