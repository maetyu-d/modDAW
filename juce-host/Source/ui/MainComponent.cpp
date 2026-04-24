#include "MainComponent.h"

namespace
{
bool shouldKeepPlaybackLogLine(const juce::String& line)
{
    auto lower = line.toLowerCase();
    return line.startsWith("STATE ")
        || lower.contains("error")
        || lower.contains("failed")
        || lower.contains("exited");
}
}

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
    addAndMakeVisible(arrangementModeButton);
    addAndMakeVisible(mixerModeButton);
    addAndMakeVisible(graphModeButton);
    addAndMakeVisible(codeModeButton);

    arrangementModeButton.setButtonText("Arrange");
    mixerModeButton.setButtonText("Mix");
    graphModeButton.setButtonText("Route");
    codeModeButton.setButtonText("Code");

    arrangementModeButton.onClick = [this] { setWorkflowMode("arrangement"); };
    mixerModeButton.onClick = [this] { setWorkflowMode("mixer"); };
    graphModeButton.onClick = [this] { setWorkflowMode("graph"); };
    codeModeButton.onClick = [this] { setWorkflowMode("code"); };

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

    transportPanel.onRenderMixPressed = [this]
    {
        processManager.requestRenderFullMix();
    };

    transportPanel.onRenderStemsPressed = [this]
    {
        processManager.requestRenderStems();
    };

    moduleLanes.onModuleSelected = [this](const juce::String& moduleId)
    {
        selectedModuleId = moduleId;
        selectionDirty = true;
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
    startTimerHz(4);
    updateWorkflowVisibility();
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
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(16);
    auto headerArea = area.removeFromTop(28);
    statusBar.setBounds(headerArea.removeFromRight(300));
    auto modeArea = headerArea.removeFromRight(320);
    auto modeWidth = (modeArea.getWidth() - 18) / 4;
    arrangementModeButton.setBounds(modeArea.removeFromLeft(modeWidth));
    modeArea.removeFromLeft(6);
    mixerModeButton.setBounds(modeArea.removeFromLeft(modeWidth));
    modeArea.removeFromLeft(6);
    graphModeButton.setBounds(modeArea.removeFromLeft(modeWidth));
    modeArea.removeFromLeft(6);
    codeModeButton.setBounds(modeArea);
    area.removeFromTop(10);
    globalRuler.setBounds(area.removeFromTop(126));
    area.removeFromTop(8);
    moduleLanes.setBounds(area.removeFromTop(154));
    area.removeFromTop(8);

    auto lowerArea = area;
    auto dashboardLeft = lowerArea.removeFromLeft((lowerArea.getWidth() - 10) / 2);
    lowerArea.removeFromLeft(10);
    auto sideColumn = lowerArea;

    if (workflowMode == "mixer")
    {
        mixerPanel.setBounds(dashboardLeft);
        transportPanel.setBounds(sideColumn.removeFromTop(150));
        sideColumn.removeFromTop(8);
        automationPanel.setBounds(sideColumn.removeFromTop(150));
        sideColumn.removeFromTop(8);
        timingInspector.setBounds(sideColumn.removeFromTop(160));
    }
    else if (workflowMode == "graph")
    {
        routeGraphPanel.setBounds(dashboardLeft);
        routeListPanel.setBounds(sideColumn.removeFromTop(190));
        sideColumn.removeFromTop(8);
        transportPanel.setBounds(sideColumn.removeFromTop(150));
        sideColumn.removeFromTop(8);
        timingInspector.setBounds(sideColumn);
    }
    else if (workflowMode == "code")
    {
        codeSurface.setBounds(dashboardLeft);
        transportPanel.setBounds(sideColumn.removeFromTop(150));
        sideColumn.removeFromTop(8);
        timingInspector.setBounds(sideColumn.removeFromTop(170));
        sideColumn.removeFromTop(8);
        logPanel.setBounds(sideColumn);
    }
    else
    {
        codeSurface.setBounds(dashboardLeft);
        transportPanel.setBounds(sideColumn.removeFromTop(150));
        sideColumn.removeFromTop(8);
        timingInspector.setBounds(sideColumn.removeFromTop(170));
        sideColumn.removeFromTop(8);
        logPanel.setBounds(sideColumn);
    }

    updateWorkflowVisibility();
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

    if (key.getTextCharacter() == 'a')
    {
        setWorkflowMode("arrangement");
        return true;
    }

    if (key.getTextCharacter() == 'm')
    {
        setWorkflowMode("mixer");
        return true;
    }

    if (key.getTextCharacter() == 'g')
    {
        setWorkflowMode("graph");
        return true;
    }

    if (key.getTextCharacter() == 'c')
    {
        setWorkflowMode("code");
        return true;
    }

    if (key.getTextCharacter() == 'r')
    {
        processManager.requestRenderFullMix();
        return true;
    }

    if (key.getTextCharacter() == 's' && key.getModifiers().isCommandDown())
    {
        processManager.requestProjectSave();
        return true;
    }

    return false;
}

void MainComponent::timerCallback()
{
    const auto connectionState = processManager.getConnectionState();
    statusBar.setConnectionState(connectionState);

    const auto transportRevision = processManager.getTransportRevision();
    const auto clockDomainRevision = processManager.getClockDomainRevision();
    const auto moduleRevision = processManager.getModuleRevision();
    const auto mixerRevision = processManager.getMixerRevision();
    const auto routeRevision = processManager.getRouteRevision();
    const auto regionRevision = processManager.getRegionRevision();
    const auto recoveryRevision = processManager.getRecoveryRevision();
    const auto renderRevision = processManager.getRenderRevision();
    const auto automationRevision = processManager.getAutomationRevision();
    const auto analysisRevision = processManager.getAnalysisRevision();
    const auto structuralRevision = processManager.getStructuralRevision();
    const auto validationRevision = processManager.getValidationRevision();

    const auto transportChanged = transportRevision != lastTransportRevision;
    const auto clockChanged = clockDomainRevision != lastClockDomainRevision;
    const auto moduleChanged = moduleRevision != lastModuleRevision;
    const auto mixerChanged = mixerRevision != lastMixerRevision;
    const auto routeChanged = routeRevision != lastRouteRevision;
    const auto regionChanged = regionRevision != lastRegionRevision;
    const auto recoveryChanged = recoveryRevision != lastRecoveryRevision;
    const auto renderChanged = renderRevision != lastRenderRevision;
    const auto automationChanged = automationRevision != lastAutomationRevision;
    const auto analysisChanged = analysisRevision != lastAnalysisRevision;
    const auto structuralChanged = structuralRevision != lastStructuralRevision;
    const auto validationChanged = validationRevision != lastValidationRevision;

    if (transportChanged)
        transportStateCache = processManager.getTransportState();

    if (clockChanged)
        clockDomainStateCache = processManager.getClockDomainState();

    if (moduleChanged)
    {
        moduleStateCache = processManager.getModuleState();
        ensureSelectedModule(moduleStateCache);
    }

    if (mixerChanged)
        mixerStateCache = processManager.getMixerState();

    if (routeChanged)
        routeStateCache = processManager.getRouteState();

    if (regionChanged)
        regionStateCache = processManager.getRegionState();

    if (recoveryChanged)
        recoveryStateCache = processManager.getRecoveryState();

    if (renderChanged)
        renderStateCache = processManager.getRenderState();

    if (automationChanged)
        automationStateCache = processManager.getAutomationState();

    if (analysisChanged)
        analysisStateCache = processManager.getAnalysisState();

    if (structuralChanged)
        structuralStateCache = processManager.getStructuralState();

    if (validationChanged)
        validationStateCache = processManager.getValidationState();

    const auto selectionChanged = selectionDirty;

    const auto* selectedModule = moduleStateCache.findById(selectedModuleId);
    const auto* selectedClockDomain = (selectedModule != nullptr)
        ? clockDomainStateCache.findById(selectedModule->clockDomainId)
        : nullptr;
    const bool playbackActive = transportStateCache.isPlaying;

    if (transportChanged)
        globalRuler.setTransportState(transportStateCache);

    if (structuralChanged && ! playbackActive)
        globalRuler.setStructuralState(structuralStateCache);

    if (clockChanged || moduleChanged || selectionChanged)
        globalRuler.setSelectedLaneOverlay(selectedModule, selectedClockDomain);

    if (moduleChanged)
        moduleLanes.setModuleState(moduleStateCache);

    if (regionChanged)
        moduleLanes.setRegionState(regionStateCache);

    if (structuralChanged)
        moduleLanes.setStructuralState(structuralStateCache);

    if (moduleChanged || selectionChanged)
        moduleLanes.setSelectedModuleId(selectedModuleId);

    if (moduleChanged || selectionChanged)
        codeSurface.setSelectedModule(selectedModule);

    if (mixerChanged)
        mixerPanel.setMixerState(mixerStateCache);

    if (automationChanged)
        automationPanel.setAutomationState(automationStateCache);

    if (routeChanged)
    {
        routeGraphPanel.setRouteState(routeStateCache);
        routeListPanel.setRouteState(routeStateCache);
    }

    if (transportChanged)
        transportPanel.setTransportState(transportStateCache);

    if (recoveryChanged)
        transportPanel.setRecoveryState(recoveryStateCache);

    if (renderChanged)
        transportPanel.setRenderState(renderStateCache);

    if ((moduleChanged || clockChanged || structuralChanged || analysisChanged || selectionChanged)
        || (transportChanged && ! playbackActive))
        timingInspector.setInspectorState(transportStateCache, selectedModule, selectedClockDomain, structuralStateCache, analysisStateCache);

    if (validationChanged)
        validationPanel.setValidationState(validationStateCache);

    lastTransportRevision = transportRevision;
    lastClockDomainRevision = clockDomainRevision;
    lastModuleRevision = moduleRevision;
    lastMixerRevision = mixerRevision;
    lastRouteRevision = routeRevision;
    lastRegionRevision = regionRevision;
    lastRecoveryRevision = recoveryRevision;
    lastRenderRevision = renderRevision;
    lastAutomationRevision = automationRevision;
    lastAnalysisRevision = analysisRevision;
    lastStructuralRevision = structuralRevision;
    lastValidationRevision = validationRevision;
    selectionDirty = false;

    auto pendingLogLines = processManager.takePendingLogLines();

    if (playbackActive)
    {
        for (const auto& line : pendingLogLines)
            if (shouldKeepPlaybackLogLine(line))
                logPanel.appendLine(line);
    }
    else
    {
        const auto startIndex = juce::jmax(0, pendingLogLines.size() - 24);

        for (int i = startIndex; i < pendingLogLines.size(); ++i)
            logPanel.appendLine(pendingLogLines[i]);
    }
}

void MainComponent::ensureSelectedModule(const ModuleState& moduleState)
{
    if (moduleState.modules.isEmpty())
    {
        selectedModuleId.clear();
        selectionDirty = true;
        return;
    }

    if (selectedModuleId.isEmpty() || moduleState.findById(selectedModuleId) == nullptr)
    {
        selectedModuleId = moduleState.modules.getFirst().id;
        selectionDirty = true;
    }
}

void MainComponent::setWorkflowMode(const juce::String& mode)
{
    workflowMode = mode;
    updateWorkflowVisibility();
    resized();
    repaint();
}

void MainComponent::updateWorkflowVisibility()
{
    const auto arrangement = workflowMode == "arrangement";
    const auto mixer = workflowMode == "mixer";
    const auto graph = workflowMode == "graph";
    const auto code = workflowMode == "code";

    codeSurface.setVisible(arrangement || code);
    mixerPanel.setVisible(mixer);
    automationPanel.setVisible(mixer);
    routeGraphPanel.setVisible(graph);
    routeListPanel.setVisible(graph);
    transportPanel.setVisible(true);
    timingInspector.setVisible(true);
    validationPanel.setVisible(false);
    logPanel.setVisible(arrangement || code);

    arrangementModeButton.setToggleState(arrangement, juce::dontSendNotification);
    mixerModeButton.setToggleState(mixer, juce::dontSendNotification);
    graphModeButton.setToggleState(graph, juce::dontSendNotification);
    codeModeButton.setToggleState(code, juce::dontSendNotification);
}
