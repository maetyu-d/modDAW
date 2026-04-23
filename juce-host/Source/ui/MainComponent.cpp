#include "MainComponent.h"

MainComponent::MainComponent()
{
    addAndMakeVisible(statusBar);
    addAndMakeVisible(globalRuler);
    addAndMakeVisible(moduleLanes);
    addAndMakeVisible(codeSurface);
    addAndMakeVisible(mixerPanel);
    addAndMakeVisible(automationPanel);
    addAndMakeVisible(routeGraphPanel);
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

    transportPanel.onSceneNextPhrasePressed = [this]
    {
        processManager.requestSceneTransition("nextPhrase", 0, "global.main", "Next Phrase Scene");
    };

    transportPanel.onSceneAfterTwoCyclesPressed = [this]
    {
        processManager.requestSceneTransition("afterNCycles", 2, "global.main", "After Two Cycles Scene");
    };

    transportPanel.onSceneExternalCuePressed = [this]
    {
        processManager.requestSceneTransition("externalCue", 0, "global.main", "External Cue Scene");
    };

    transportPanel.onExternalCuePressed = [this]
    {
        processManager.requestExternalCue("manual");
    };

    transportPanel.onPerformanceAccentPressed = [this]
    {
        processManager.requestPerformanceMacro("kick.accent", 1.0);
    };

    transportPanel.onPerformanceCuePressed = [this]
    {
        processManager.requestPerformanceMacro("scene.cue", 1.0);
    };

    transportPanel.onPerformanceLiftPressed = [this]
    {
        processManager.requestPerformanceMacro("density.lift", 1.0);
    };

    moduleLanes.onModuleSelected = [this](const juce::String& moduleId)
    {
        selectedModuleId = moduleId;
    };

    moduleLanes.onFreezeModule = [this](const juce::String& moduleId)
    {
        processManager.requestModuleFreezeToRegion(moduleId);
    };

    moduleLanes.onLiveLinkModule = [this](const juce::String& moduleId)
    {
        processManager.requestModuleLiveLinkedRegion(moduleId);
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

    codeSurface.onSubmitPressed = [this](const juce::String& moduleId, const juce::String& surfaceId, const juce::String& codeText)
    {
        processManager.requestModuleCodeSurfaceUpdateNextBar(moduleId, surfaceId, codeText);
    };

    mixerPanel.onStripLevelChanged = [this](const juce::String& stripId, double level)
    {
        processManager.requestMixerStripLevel(stripId, level);
    };

    mixerPanel.onStripMuteChanged = [this](const juce::String& stripId, bool muted)
    {
        processManager.requestMixerStripMuted(stripId, muted);
    };

    mixerPanel.onStripGroupChanged = [this](const juce::String& stripId, const juce::String& groupId)
    {
        processManager.requestMixerStripGroup(stripId, groupId);
    };

    mixerPanel.onSendLevelChanged = [this](const juce::String& sendId, double level)
    {
        processManager.requestMixerSendLevel(sendId, level);
    };

    mixerPanel.onSendModeChanged = [this](const juce::String& sendId, const juce::String& mode)
    {
        processManager.requestMixerSendMode(sendId, mode);
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

    routeGraphPanel.onCreateRoute = [this](const juce::String& family,
                                           const juce::String& source,
                                           const juce::String& destination,
                                           bool enabled)
    {
        processManager.requestRouteCreate(family, source, destination, enabled);
    };

    routeGraphPanel.onDeleteRoute = [this](const juce::String& routeId)
    {
        processManager.requestRouteDelete(routeId);
    };

    timingInspector.onRelationChangeRequested = [this](const juce::String& domainId,
                                                       const juce::String& relationType,
                                                       double phaseOffsetBeats)
    {
        processManager.requestClockDomainRelation(domainId, relationType, phaseOffsetBeats);
    };

    processManager.start();
    startTimerHz(10);
    setWantsKeyboardFocus(true);
    grabKeyboardFocus();
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
    g.drawText("M28 recovery containment", 236, 13, 260, 22, juce::Justification::centredLeft);
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

    transportPanel.setBounds(sideColumn.removeFromTop(190));
    sideColumn.removeFromTop(10);
    routeGraphPanel.setBounds(sideColumn.removeFromTop(230));
    sideColumn.removeFromTop(10);
    routeListPanel.setBounds(sideColumn.removeFromTop(150));
    sideColumn.removeFromTop(10);
    automationPanel.setBounds(sideColumn.removeFromTop(130));
    sideColumn.removeFromTop(10);
    timingInspector.setBounds(sideColumn.removeFromTop(120));
    sideColumn.removeFromTop(10);

    auto diagnosticsRow = sideColumn;
    auto validationWidth = juce::roundToInt(static_cast<float>(diagnosticsRow.getWidth()) * 0.40f);
    validationPanel.setBounds(diagnosticsRow.removeFromLeft(validationWidth));
    diagnosticsRow.removeFromLeft(10);
    logPanel.setBounds(diagnosticsRow);
}

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    if (key.getTextCharacter() == '1')
    {
        processManager.requestPerformanceMacro("kick.accent", 1.0);
        return true;
    }

    if (key.getTextCharacter() == '2')
    {
        processManager.requestPerformanceMacro("scene.cue", 1.0);
        return true;
    }

    if (key.getTextCharacter() == '3')
    {
        processManager.requestPerformanceMacro("density.lift", 1.0);
        return true;
    }

    return false;
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
    const auto recoveryState = processManager.getRecoveryState();
    const auto automationState = processManager.getAutomationState();
    const auto analysisState = processManager.getAnalysisState();
    const auto structuralState = processManager.getStructuralState();
    const auto validationState = processManager.getValidationState();

    ensureSelectedModule(moduleState);

    const auto* selectedModule = moduleState.findById(selectedModuleId);
    const auto* selectedClockDomain = selectedModule != nullptr
        ? clockDomainState.findById(selectedModule->clockDomainId)
        : nullptr;

    statusBar.setConnectionState(connectionState);
    globalRuler.setTransportState(transportState);
    globalRuler.setStructuralState(structuralState);
    globalRuler.setSelectedLaneOverlay(selectedModule, selectedClockDomain);
    moduleLanes.setModuleState(moduleState);
    moduleLanes.setRegionState(regionState);
    moduleLanes.setStructuralState(structuralState);
    moduleLanes.setSelectedModuleId(selectedModuleId);
    codeSurface.setSelectedModule(selectedModule);
    mixerPanel.setMixerState(mixerState);
    automationPanel.setAutomationState(automationState);
    routeGraphPanel.setRouteState(routeState);
    routeListPanel.setRouteState(routeState);
    transportPanel.setTransportState(transportState);
    transportPanel.setRecoveryState(recoveryState);
    timingInspector.setInspectorState(transportState, selectedModule, selectedClockDomain, structuralState, analysisState);
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
