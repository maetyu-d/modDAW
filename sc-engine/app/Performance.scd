(
~performanceMappings = [
    (
        macroId: "kick.accent",
        displayName: "Kick Accent",
        input: "keyboard:1",
        target: "module.kick",
        description: "Trigger an immediate engine-owned kick accent."
    ),
    (
        macroId: "scene.cue",
        displayName: "Scene Cue",
        input: "keyboard:2",
        target: "structural.externalCue",
        description: "Release pending external-cue scene transitions."
    ),
    (
        macroId: "density.lift",
        displayName: "Density Lift",
        input: "keyboard:3",
        target: "module.conductor",
        description: "Emit a structural density lift directive now."
    )
];

~performanceRecentEvents = [];

~makePerformancePayload = {
    (
        mappings: ~performanceMappings,
        recentEvents: ~performanceRecentEvents.keep(12)
    )
};

~emitPerformanceState = { |correlationId = nil|
    var kind;
    var payload;

    kind = if (correlationId.notNil) { "response" } { "event" };
    payload = ~makePerformancePayload.value;
    ~engineState[\performance] = payload;
    ~sendEnvelope.value(kind, "performance.state", payload, correlationId);
};

~recordPerformanceEvent = { |macroId, value, result|
    var event;

    event = (
        macroId: macroId,
        value: value,
        beat: ~snapBeat.value(~captureTransportBeats.value),
        result: result
    );

    ~performanceRecentEvents = ~performanceRecentEvents.add(event).keep(12);
    event
};

~triggerKickAccentMacro = { |value = 1.0|
    var runtime;
    var soundSpec;
    var mixerGain;
    var ampScale;

    runtime = ~moduleRuntimeState[~kickModuleId];
    soundSpec = if (runtime.notNil) { runtime[\activeSoundSpec] ? () } { () };
    if (~recordKickAnalysisEvent.notNil) {
        ~recordKickAnalysisEvent.value(~captureTransportBeats.value, soundSpec);
    };
    mixerGain = ~effectiveModuleMixerGain.value(~kickModuleId);
    ampScale = value.asFloat.clip(0.0, 1.0);

    if (~audioServerReady and: { mixerGain > 0.0001 }) {
        Synth(\modDawKickPulse, [
            \amp, ((soundSpec[\amp] ? 0.5) * mixerGain * (1.0 + ampScale)).clip(0.0, 1.2),
            \baseFreq, (soundSpec[\baseFreq] ? 74.0) * (1.0 + (ampScale * 0.15)),
            \dropFreq, soundSpec[\dropFreq] ? 32.0,
            \noiseMix, ((soundSpec[\noiseMix] ? 0.05) + (ampScale * 0.04)).clip(0.0, 0.18),
            \decay, soundSpec[\decay] ? 0.22
        ]);
    };

    if (runtime.notNil) {
        runtime.put(\lastPerformanceMacro, "kick.accent");
        runtime.put(\lastPerformanceBeat, ~snapBeat.value(~captureTransportBeats.value));
        ~emitModulesState.value;
    };

    "kick accent triggered"
};

~triggerDensityLiftMacro = { |value = 1.0|
    var phraseBeats;
    var phraseIndex;
    var directive;

    phraseBeats = ~phraseLengthBeatsForDomain.value("global.main");
    phraseIndex = (~captureTransportBeats.value / phraseBeats).floor.asInteger;
    directive = ~emitStructuralDirective.value(phraseIndex, "Performance Lift", "performanceInput", ~snapBeat.value(~captureTransportBeats.value));
    "density lift emitted"
};

~triggerPerformanceMacro = { |macroId, value = 1.0, correlationId = nil|
    var macro;
    var result;
    var event;

    macroId = macroId.asString;
    macro = ~performanceMappings.detect({ |mapping| mapping[\macroId].asString == macroId }, { nil });

    if (macro.isNil) {
        ~sendEnvelope.value("error", "performance.error", (
            message: "Unknown performance macro",
            macroId: macroId
        ), correlationId);
        nil
    } {
        result = case
        { macroId == "kick.accent" } { ~triggerKickAccentMacro.value(value) }
        { macroId == "scene.cue" } {
            ~triggerExternalCue.value("performance", correlationId);
            "external cue sent"
        }
        { macroId == "density.lift" } { ~triggerDensityLiftMacro.value(value) }
        { "macro ignored" };

        event = ~recordPerformanceEvent.value(macroId, value, result);
        ~emitPerformanceState.value;
        ~sendEnvelope.value("event", "performance.macroTriggered", event, correlationId);
    };
};
)
