(
~structuralDirectives = [];
~structuralPendingTransitions = [];
~structuralNextDirectiveId = 1;
~structuralNextTransitionId = 1;
~structuralPoller = nil;
~lastStructuralPhraseIndex = -1;
~currentSceneName = "A sparse";
~maxStructuralDirectiveHistory = 24;

~structuralConductorModule = (
    id: "module.conductor",
    displayName: "Conductor",
    clockDomainId: "global.main",
    lifecycleState: "active",
    behaviourType: "structural-conductor",
    laneType: "structural",
    codeSurface: "(\nphraseBars: 2,\ndensityCycle: [0.35, 0.85],\ncue: 'phrase-reset'\n)",
    codeSurfaces: [
        (surfaceId: "pattern", displayName: "Pattern", role: "structural", code: "(\nphraseBars: 2,\ndensityCycle: [0.35, 0.85],\ncue: 'phrase-reset'\n)"),
        (surfaceId: "init", displayName: "Init", role: "init", code: "(\nready: true\n)")
    ]
);

~installStructuralModule = {
    if (~definitionForModuleId.value(~structuralConductorModule[\id]).isNil) {
        ~moduleDefinitions = ~moduleDefinitions.add(~structuralConductorModule);
    };
};

~makeStructuralDirectiveId = {
    var id;

    id = "directive.user." ++ ~structuralNextDirectiveId.asString;
    ~structuralNextDirectiveId = ~structuralNextDirectiveId + 1;
    id
};

~makeStructuralPayload = {
    (
        directives: ~structuralDirectives.keep(12),
        pendingTransitions: ~structuralPendingTransitions,
        currentScene: ~currentSceneName
    )
};

~emitStructuralState = { |correlationId = nil|
    var kind;
    var payload;

    kind = if (correlationId.notNil) { "response" } { "event" };
    payload = ~makeStructuralPayload.value;
    ~engineState[\structural] = payload[\directives];
    ~sendEnvelope.value(kind, "structural.state", payload, correlationId);
};

~applyStructuralDirective = { |directive|
    var kickRuntime;
    var soundSpec;
    var density;

    density = directive[\densityTarget] ? 0.5;
    kickRuntime = ~moduleRuntimeState[~kickModuleId];

    if (kickRuntime.notNil) {
        soundSpec = (kickRuntime[\activeSoundSpec] ? ()).copy;
        soundSpec.put(\amp, (0.35 + (density * 0.35)).clip(0.2, 0.9));
        soundSpec.put(\noiseMix, (0.02 + (density * 0.08)).clip(0.01, 0.12));
        kickRuntime.put(\activeSoundSpec, soundSpec);
        kickRuntime.put(\lastTriggeredStep, -1);
    };

    ["module.kick", "module.hat", "module.texture"].do { |moduleId|
        if (~moduleRuntimeState[moduleId].notNil) {
            ~moduleRuntimeState[moduleId].put(\lastStructuralDirective, directive[\directiveId]);
        };
    };
};

~phraseLengthBeatsForDomain = { |domainId|
    var definition;
    var bars;
    definition = ~clockDomainDefinitionById.value(domainId) ? ~clockDomainDefinitionById.value("global.main");
    bars = (definition[\phraseLengthBars] ? 2).asInteger.max(1);
    bars.asFloat * (~transportState[\meterNumerator] ? 4).asFloat
};

~nextPhraseBoundaryForDomain = { |domainId, afterCycles = 0|
    var phraseBeats;
    var currentBeat;
    var cycles;

    phraseBeats = ~phraseLengthBeatsForDomain.value(domainId);
    currentBeat = ~captureTransportBeats.value;
    cycles = afterCycles.asInteger.max(0);
    (((currentBeat / phraseBeats).floor + 1 + cycles) * phraseBeats).asFloat
};

~emitStructuralDirective = { |phraseIndex, sceneName = nil, quantizationTarget = "auto", scheduledBeat = nil|
    var density;
    var sectionName;
    var directive;

    density = if ((phraseIndex % 2) == 0) { 0.35 } { 0.85 };
    sectionName = sceneName ? if ((phraseIndex % 2) == 0) { "A sparse" } { "B lift" };
    ~currentSceneName = sectionName;

    directive = (
        directiveId: ~makeStructuralDirectiveId.value,
        emitterModuleId: ~structuralConductorModule[\id],
        beat: ~snapBeat.value(~captureTransportBeats.value),
        scheduledBeat: scheduledBeat ? ~snapBeat.value(~captureTransportBeats.value),
        phraseIndex: phraseIndex,
        section: sectionName,
        densityTarget: density,
        syncCue: "phrase-reset",
        phraseReset: true,
        quantizationTarget: quantizationTarget,
        orchestration: if (density > 0.5) { "kick-forward" } { "open-space" },
        targetModuleIds: ["module.kick", "module.hat", "module.texture"]
    );

    ~structuralDirectives = ~structuralDirectives.add(directive).keep(~maxStructuralDirectiveHistory);
    ~applyStructuralDirective.value(directive);
    ~emitStructuralState.value;
    ~emitModulesState.value;
};

~scheduleSceneTransition = { |quantizationTarget = "nextPhrase", afterCycles = 0, domainId = "global.main", sceneName = "Scene Change", correlationId = nil|
    var transition;
    var transitionId;
    var targetBeat;
    var allowedTargets;

    allowedTargets = ["nextPhrase", "afterNCycles", "externalCue"];
    quantizationTarget = quantizationTarget.asString;

    if (allowedTargets.detect({ |target| target.asString == quantizationTarget }, { nil }).isNil) {
        ~sendEnvelope.value("error", "structural.sceneTransitionRejected", (
            message: "Unsupported scene transition quantisation target",
            quantizationTarget: quantizationTarget
        ), correlationId);
        nil
    } {
        transitionId = "transition.user." ++ ~structuralNextTransitionId.asString;
        ~structuralNextTransitionId = ~structuralNextTransitionId + 1;
        targetBeat = if (quantizationTarget == "externalCue") {
            nil
        } {
            ~nextPhraseBoundaryForDomain.value(domainId, if (quantizationTarget == "afterNCycles") { afterCycles } { 0 })
        };

        transition = (
            transitionId: transitionId,
            domainId: domainId,
            sceneName: sceneName,
            quantizationTarget: quantizationTarget,
            afterCycles: afterCycles.asInteger.max(0),
            requestedBeat: ~snapBeat.value(~captureTransportBeats.value),
            targetBeat: targetBeat,
            state: if (targetBeat.isNil) { "waiting-external-cue" } { "pending-boundary" }
        );

        ~structuralPendingTransitions = ~structuralPendingTransitions.add(transition);
        ~emitStructuralState.value;
        if (~markProjectDirty.notNil) { ~markProjectDirty.value("scene transition scheduled") };
        ~sendEnvelope.value("event", "structural.sceneTransitionScheduled", transition, correlationId);
    };
};

~triggerExternalCue = { |cueName = "manual", correlationId = nil|
    ~structuralPendingTransitions = ~structuralPendingTransitions.collect { |transition|
        if (transition[\quantizationTarget] == "externalCue" and: { transition[\targetBeat].isNil }) {
            transition.copy.put(\targetBeat, ~snapBeat.value(~captureTransportBeats.value)).put(\state, "pending-external-cue")
        } {
            transition
        }
    };
    ~emitStructuralState.value;
    ~sendEnvelope.value("event", "structural.externalCueReceived", (cueName: cueName), correlationId);
};

~processSceneTransitions = {
    var currentBeat;
    var readyTransitions;

    currentBeat = ~captureTransportBeats.value;
    readyTransitions = ~structuralPendingTransitions.select { |transition|
        transition[\targetBeat].notNil and: { currentBeat >= transition[\targetBeat] }
    };

    readyTransitions.do { |transition|
        var phraseBeats;
        var phraseIndex;
        phraseBeats = ~phraseLengthBeatsForDomain.value(transition[\domainId]);
        phraseIndex = (transition[\targetBeat] / phraseBeats).floor.asInteger;
        ~emitStructuralDirective.value(phraseIndex, transition[\sceneName], transition[\quantizationTarget], transition[\targetBeat]);
        ~sendEnvelope.value("event", "structural.sceneTransitionApplied", transition.copy.put(\appliedBeat, ~snapBeat.value(currentBeat)));
    };

    if (readyTransitions.notEmpty) {
        ~structuralPendingTransitions = ~structuralPendingTransitions.reject { |transition|
            readyTransitions.any { |ready| ready[\transitionId] == transition[\transitionId] }
        };
        ~emitStructuralState.value;
    };
};

~processStructuralConductor = {
    var beatsPerBar;
    var phraseBeats;
    var phraseIndex;

    beatsPerBar = ~transportState[\meterNumerator].asFloat;
    phraseBeats = beatsPerBar * 2.0;
    phraseIndex = (~captureTransportBeats.value / phraseBeats).floor.asInteger;

    if (phraseIndex != ~lastStructuralPhraseIndex) {
        ~lastStructuralPhraseIndex = phraseIndex;
        ~emitStructuralDirective.value(phraseIndex);
    };
};

~startStructuralLoop = {
    if (~structuralPoller.notNil) {
        ~structuralPoller.stop;
        ~structuralPoller = nil;
    };

    ~structuralPoller = Routine({
        loop {
            if (~transportState[\isPlaying]) {
                ~processStructuralConductor.value;
                ~processSceneTransitions.value;
            };

            0.05.wait;
        }
    }).play(SystemClock);
};
)
