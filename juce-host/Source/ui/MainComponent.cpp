#include "MainComponent.h"

MainComponent::MainComponent()
{
    addAndMakeVisible(statusBar);
    addAndMakeVisible(globalRuler);
    addAndMakeVisible(moduleLanes);
    addAndMakeVisible(codeSurface);
    addAndMakeVisible(mixerPanel);
    addAndMakeVisible(automationPanel);
    addAndMakeVisible(routeListPanel);
    addAndMakeVisible(transportPanel);
    addAndMakeVisible(timingInspector);
    addAndMakeVisible(validationPanel);
    addAndMakeVisible(logPanel);

    transportPanel.onPlayPressed = [this]
    {
        processManager.requestTransportPlay();
    };

    transportPanel.onStopPressed = [this]
    {
        processManager.requestTransportStop();
    };

    transportPanel.onRefreshPressed = [this]
    {
        processManager.requestTransportState();
    };

    transportPanel.onActivateNextBarPressed = [this]
    {
        if (selectedModuleId.isNotEmpty())
            processManager.requestModuleActivateNextBar(selectedModuleId);
    };

    transportPanel.onSaveProjectPressed = [this]
    {
        processManager.requestProjectSave();
    };

    transportPanel.onLoadProjectPressed = [this]
    {
        processManager.requestProjectLoad();
    };

    moduleLanes.onModuleSelected = [this](const juce::String& moduleId)
    {
        selectedModuleId = moduleId;
    };

    moduleLanes.onFreezeModule = [this](const juce::String& moduleId)
    {
        processManager.requestModuleFreezeToRegion(moduleId);
    };

    moduleLanes.onRegionEdit = [this](const juce::String& regionId, const juce::String& action)
    {
        if (action == "move-left")
            processManager.requestRegionMove(regionId, -1.0);
        else if (action == "move-right")
            processManager.requestRegionMove(regionId, 1.0);
        else if (action == "trim-shorter")
            processManager.requestRegionTrim(regionId, -1.0);
        else if (action == "trim-longer")
            processManager.requestRegionTrim(regionId, 1.0);
        else if (action == "split")
            processManager.requestRegionSplit(regionId);
        else if (action == "delete")
            processManager.requestRegionDelete(regionId);
    };

    codeSurface.onSubmitPressed = [this](const juce::String& moduleId, const juce::String& codeText)
    {
        processManager.requestModuleCodeSurfaceUpdateNextBar(moduleId, codeText);
    };

    mixerPanel.onStripLevelChanged = [this](const juce::String& stripId, double level)
    {
        processManager.requestMixerStripLevel(stripId, level);
    };

    mixerPanel.onStripMuteChanged = [this](const juce::String& stripId, bool muted)
    {
        processManager.requestMixerStripMuted(stripId, muted);
    };

    automationPanel.onAddPoint = [this](const juce::String& laneId, double value)
    {
        processManager.requestAutomationAddPoint(laneId, value);
    };

    automationPanel.onResetDemo = [this](const juce::String& laneId)
    {
        processManager.requestAutomationResetDemo(laneId);
    };

    routeListPanel.onCreateRoute = [this](const juce::String& family,
                                          const juce::String& source,
                                          const juce::String& destination,
                                          bool enabled)
    {
        processManager.requestRouteCreate(family, source, destination, enabled);
    };

    routeListPanel.onDeleteRoute = [this](const juce::String& routeId)
    {
        processManager.requestRouteDelete(routeId);
    };

    processManager.start();
    startTimerHz(10);
    setSize(1480, 1120);
}

MainComponent::~MainComponent()
{
    stopTimer();
    processManager.stop();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0b0d10));

    g.setColour(juce::Colour(0xfff3f5f7));
    g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    g.drawText("Modular SC DAW", 16, 12, 240, 24, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff8d97a6));
    g.setFont(juce::FontOptions(12.5f));
    g.drawText("M17 automation lanes", 236, 13, 220, 22, juce::Justification::centredLeft);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(16);
    auto headerArea = area.removeFromTop(36);
    statusBar.setBounds(headerArea.removeFromRight(300));
    area.removeFromTop(8);
    globalRuler.setBounds(area.removeFromTop(148));
    area.removeFromTop(8);
    moduleLanes.setBounds(area.removeFromTop(170));
    area.removeFromTop(10);

    auto lowerArea = area;
    auto dashboardLeft = lowerArea.removeFromLeft((lowerArea.getWidth() - 12) / 2);
    lowerArea.removeFromLeft(12);
    auto sideColumn = lowerArea;

    auto codeArea = dashboardLeft.removeFromTop(juce::roundToInt(static_cast<float>(dashboardLeft.getHeight()) * 0.55f));
    codeSurface.setBounds(codeArea);
    dashboardLeft.removeFromTop(10);
    mixerPanel.setBounds(dashboardLeft);

    transportPanel.setBounds(sideColumn.removeFromTop(120));
    sideColumn.removeFromTop(10);
    automationPanel.setBounds(sideColumn.removeFromTop(150));
    sideColumn.removeFromTop(10);
    routeListPanel.setBounds(sideColumn.removeFromTop(170));
    sideColumn.removeFromTop(10);
    timingInspector.setBounds(sideColumn.removeFromTop(120));
    sideColumn.removeFromTop(10);

    auto diagnosticsRow = sideColumn;
    auto validationWidth = juce::roundToInt(static_cast<float>(diagnosticsRow.getWidth()) * 0.40f);
    validationPanel.setBounds(diagnosticsRow.removeFromLeft(validationWidth));
    diagnosticsRow.removeFromLeft(10);
    logPanel.setBounds(diagnosticsRow);
}

void MainComponent::timerCallback()
{
    const auto connectionState = processManager.getConnectionState();
    const auto transportState = processManager.getTransportState();
    const auto clockDomainState = processManager.getClockDomainState();
    const auto moduleState = processManager.getModuleState();
    const auto mixerState = processManager.getMixerState();
    const auto routeState = processManager.getRouteState();
    const auto regionState = processManager.getRegionState();
    const auto automationState = processManager.getAutomationState();
    const auto validationState = processManager.getValidationState();

    ensureSelectedModule(moduleState);

    const auto* selectedModule = moduleState.findById(selectedModuleId);
    const auto* selectedClockDomain = selectedModule != nullptr
        ? clockDomainState.findById(selectedModule->clockDomainId)
        : nullptr;

    statusBar.setConnectionState(connectionState);
    globalRuler.setTransportState(transportState);
    globalRuler.setSelectedLaneOverlay(selectedModule, selectedClockDomain);
    moduleLanes.setModuleState(moduleState);
    moduleLanes.setRegionState(regionState);
    moduleLanes.setSelectedModuleId(selectedModuleId);
    codeSurface.setSelectedModule(selectedModule);
    mixerPanel.setMixerState(mixerState);
    automationPanel.setAutomationState(automationState);
    routeListPanel.setRouteState(routeState);
    transportPanel.setTransportState(transportState);
    timingInspector.setInspectorState(transportState, selectedModule, selectedClockDomain);
    validationPanel.setValidationState(validationState);

    for (const auto& line : processManager.takePendingLogLines())
        logPanel.appendLine(line);
}

void MainComponent::ensureSelectedModule(const ModuleState& moduleState)
{
    if (moduleState.modules.isEmpty())
    {
        selectedModuleId.clear();
        return;
    }

    if (selectedModuleId.isEmpty() || moduleState.findById(selectedModuleId) == nullptr)
        selectedModuleId = moduleState.modules.getFirst().id;
}
