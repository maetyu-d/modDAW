(
~analysisState = (
    modules: []
);

~analysisKickWindow = [];
~analysisLastBeat = 0.0;
~analysisLastEnvelope = 0.0;
~analysisLastDensity = 0.0;
~analysisLastBrightness = 0.0;
~analysisLastOnset = false;

~analysisModuleDefinition = (
    id: "module.analysis.kick",
    displayName: "Kick Listener",
    clockDomainId: "global.main",
    lifecycleState: "active",
    behaviourType: "analysis-feedback",
    laneType: "analysis",
    codeSurface: "(\nsource: 'module.kick',\nfeatures: [envelope, onset, density, brightness]\n)",
    codeSurfaces: [
        (surfaceId: "analysis", displayName: "Analysis", role: "analysis", code: "(\nsource: 'module.kick',\nwindowBeats: 4,\nfeatures: [envelope, onset, density, brightness]\n)"),
        (surfaceId: "init", displayName: "Init", role: "init", code: "(\nready: true\n)")
    ]
);

~installAnalysisModule = {
    if (~definitionForModuleId.value(~analysisModuleDefinition[\id]).isNil) {
        ~moduleDefinitions = ~moduleDefinitions.add(~analysisModuleDefinition);
    };
};

~makeAnalysisPayload = {
    (
        modules: [
            (
                moduleId: "module.analysis.kick",
                sourceModuleId: "module.kick",
                targetModuleIds: ["module.texture", "module.conductor"],
                envelope: ~analysisLastEnvelope,
                onset: ~analysisLastOnset,
                density: ~analysisLastDensity,
                brightness: ~analysisLastBrightness,
                lastBeat: ~analysisLastBeat,
                routing: "module.kick.audio.out -> module.analysis.kick.control.out -> conductor/texture"
            )
        ]
    )
};

~emitAnalysisState = { |correlationId = nil|
    var kind;
    var payload;

    kind = if (correlationId.notNil) { "response" } { "event" };
    payload = ~makeAnalysisPayload.value;
    ~analysisState = payload;
    ~engineState[\analysis] = payload;
    ~sendEnvelope.value(kind, "analysis.state", payload, correlationId);
};

~recordKickAnalysisEvent = { |beat, soundSpec|
    var amp;
    var brightness;
    var windowStart;
    var runtime;
    var textureRuntime;

    amp = (soundSpec[\amp] ? 0.5).asFloat.clip(0.0, 1.5);
    brightness = (((soundSpec[\baseFreq] ? 74.0).asFloat / 200.0) + ((soundSpec[\noiseMix] ? 0.05).asFloat * 4.0)).clip(0.0, 1.0);
    windowStart = beat - 4.0;

    ~analysisKickWindow = ~analysisKickWindow.add(beat).select({ |eventBeat| eventBeat >= windowStart });
    ~analysisLastBeat = beat;
    ~analysisLastEnvelope = amp;
    ~analysisLastOnset = true;
    ~analysisLastDensity = (~analysisKickWindow.size / 4.0).clip(0.0, 1.0);
    ~analysisLastBrightness = brightness;

    runtime = ~moduleRuntimeState["module.analysis.kick"];
    if (runtime.notNil) {
        runtime.put(\lifecycleState, "listening");
        runtime.put(\analysisSummary, "env " ++ ~analysisLastEnvelope.round(0.001).asString
            ++ " | density " ++ ~analysisLastDensity.round(0.001).asString
            ++ " | brightness " ++ ~analysisLastBrightness.round(0.001).asString);
    };

    textureRuntime = ~moduleRuntimeState["module.texture"];
    if (textureRuntime.notNil) {
        textureRuntime.put(\lastAnalysisInfluence, "density=" ++ ~analysisLastDensity.round(0.001).asString);
    };

    if (~analysisLastDensity > 0.75) {
        if (~moduleRuntimeState["module.conductor"].notNil) {
            ~moduleRuntimeState["module.conductor"].put(\lastAnalysisInfluence, "dense kick stream");
        };
    };

    ~emitAnalysisState.value;
    ~emitModulesState.value;
};
)
