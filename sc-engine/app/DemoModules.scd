(
~moduleDefinitions = [
    (
        id: "module.kick",
        displayName: "Kick Pulse",
        clockDomainId: "global.main",
        lifecycleState: "idle",
        behaviourType: "demo-sound",
        codeSurface: "(\namp: 0.5,\nbaseFreq: 74,\ndropFreq: 32,\nnoiseMix: 0.05,\ndecay: 0.22\n)",
        codeSurfaces: [
            (surfaceId: "pattern", displayName: "Pattern", role: "pattern", code: "(\namp: 0.5,\nbaseFreq: 74,\ndropFreq: 32,\nnoiseMix: 0.05,\ndecay: 0.22\n)"),
            (surfaceId: "synthDef", displayName: "SynthDef", role: "synthDef", code: "(\nname: 'modDawKickPulse',\nengineOwned: true\n)"),
            (surfaceId: "routine", displayName: "Routine", role: "routine", code: "(\nstepsPerBar: 4,\naccentEvery: 4\n)"),
            (surfaceId: "init", displayName: "Init", role: "init", code: "(\nready: true\n)")
        ],
        initialSoundSpec: (
            amp: 0.5,
            baseFreq: 74.0,
            dropFreq: 32.0,
            noiseMix: 0.05,
            decay: 0.22
        )
    ),
    (
        id: "module.hat",
        displayName: "Hat Grid",
        clockDomainId: "domain.half",
        lifecycleState: "idle",
        behaviourType: "demo-pattern",
        codeSurface: "(\npatternDensity: 0.5,\naccent: 0.2\n)",
        codeSurfaces: [
            (surfaceId: "pattern", displayName: "Pattern", role: "pattern", code: "(\npatternDensity: 0.5,\naccent: 0.2\n)"),
            (surfaceId: "routine", displayName: "Routine", role: "routine", code: "(\ndivision: 2,\nactive: false\n)"),
            (surfaceId: "init", displayName: "Init", role: "init", code: "(\nready: true\n)")
        ]
    ),
    (
        id: "module.texture",
        displayName: "Texture Drift",
        clockDomainId: "domain.triplet",
        lifecycleState: "idle",
        behaviourType: "demo-pattern",
        codeSurface: "(\nspread: 0.35,\nmodDepth: 0.15\n)",
        codeSurfaces: [
            (surfaceId: "pattern", displayName: "Pattern", role: "pattern", code: "(\nspread: 0.35,\nmodDepth: 0.15\n)"),
            (surfaceId: "routine", displayName: "Routine", role: "routine", code: "(\ncycle: 'triplet',\nactive: false\n)"),
            (surfaceId: "init", displayName: "Init", role: "init", code: "(\nready: true\n)")
        ]
    )
];

~demoModules = [];
~moduleRuntimeState = Dictionary.new;
~audioServerReady = false;
~synthDefsInstalled = false;
~kickPoller = nil;
~moduleActionPoller = nil;
~kickModuleId = "module.kick";
~lastModulesStateEmitSeconds = 0.0;
~minLiveModulesStateEmitInterval = 0.2;

~definitionForModuleId = { |moduleId|
    ~moduleDefinitions.detect({ |definition| definition[\id] == moduleId }, { nil })
};

~valueAtKey = { |dictionary, key|
    var value;

    if (dictionary.isNil) {
        nil
    } {
        value = dictionary[key];

        if (value.isNil) {
            value = dictionary[key.asSymbol];
        };

        if (value.isNil) {
            value = dictionary[key.asString];
        };

        value
    }
};

~coerceFloat = { |value, fallback|
    if (value.isNil) {
        fallback
    } {
        if (value.isKindOf(Integer) or: { value.isKindOf(Float) }) {
            value.asFloat
        } {
            fallback
        }
    }
};

~definitionSurfaces = { |definition|
    definition[\codeSurfaces] ? [
        (surfaceId: "pattern", displayName: "Pattern", role: "pattern", code: definition[\codeSurface])
    ]
};

~makeRuntimeSurface = { |surface|
    (
        surfaceId: surface[\surfaceId],
        displayName: surface[\displayName],
        role: surface[\role],
        activeCode: surface[\code],
        pendingCode: nil,
        pendingCodeSwapBeat: nil,
        pendingCodeSwapBarIndex: 0,
        state: surface[\state] ? "active",
        diagnostic: surface[\diagnostic] ? "using engine active code surface",
        revision: surface[\revision] ? 1
    )
};

~surfaceById = { |surfaces, surfaceId|
    surfaces.detect({ |surface| surface[\surfaceId] == surfaceId }, { nil })
};

~replaceRuntimeSurface = { |runtime, updatedSurface|
    runtime.put(\surfaces, (runtime[\surfaces] ? []).collect { |surface|
        if (surface[\surfaceId] == updatedSurface[\surfaceId]) { updatedSurface } { surface }
    });
};

~primarySurface = { |runtime|
    var surfaces;
    surfaces = runtime[\surfaces] ? [];
    ~surfaceById.value(surfaces, "pattern") ? surfaces.first
};

~ensureModuleRuntimeDefaults = {
    ~moduleDefinitions.do { |definition|
        if (~moduleRuntimeState[definition[\id]].isNil) {
            var surfaces;

            surfaces = ~definitionSurfaces.value(definition).collect { |surface|
                ~makeRuntimeSurface.value(surface)
            };

            ~moduleRuntimeState[definition[\id]] = (
                lifecycleState: definition[\lifecycleState],
                lastTriggeredStep: -1,
                pendingActivationBeat: nil,
                activeCodeSurface: definition[\codeSurface],
                pendingCodeSurface: nil,
                pendingCodeSwapBeat: nil,
                pendingCodeSwapBarIndex: 0,
                codeSurfaceState: "active",
                lastCodeEvalMessage: "using engine active code surface",
                surfaces: surfaces,
                activeSoundSpec: definition[\initialSoundSpec] ? ()
            );
        };
    };
};

~makeModuleEntry = { |definition|
    var runtime;
    var primarySurface;
    var surfaces;

    ~ensureModuleRuntimeDefaults.value;
    runtime = ~moduleRuntimeState[definition[\id]];
    primarySurface = ~primarySurface.value(runtime);
    surfaces = (runtime[\surfaces] ? []).collect { |surface|
        (
            surfaceId: surface[\surfaceId],
            displayName: surface[\displayName],
            role: surface[\role],
            code: surface[\activeCode],
            pendingCode: surface[\pendingCode] ? "",
            state: surface[\state] ? "active",
            diagnostic: surface[\diagnostic] ? "",
            revision: surface[\revision] ? 1,
            pendingCodeSwapBarIndex: surface[\pendingCodeSwapBarIndex] ? 0
        )
    };

    (
        id: definition[\id],
        displayName: definition[\displayName],
        clockDomainId: definition[\clockDomainId],
        lifecycleState: runtime[\lifecycleState],
        behaviourType: definition[\behaviourType],
        laneType: definition[\laneType] ? "module",
        lastStructuralDirective: runtime[\lastStructuralDirective] ? "",
        analysisSummary: runtime[\analysisSummary] ? "",
        lastAnalysisInfluence: runtime[\lastAnalysisInfluence] ? "",
        codeSurface: primarySurface[\activeCode],
        pendingCodeSurface: primarySurface[\pendingCode] ? "",
        codeSurfaceState: primarySurface[\state] ? "active",
        lastCodeEvalMessage: primarySurface[\diagnostic] ? "",
        pendingCodeSwapBarIndex: primarySurface[\pendingCodeSwapBarIndex] ? 0,
        codeSurfaces: surfaces
    )
};

~makeModulesPayload = {
    (
        modules: ~moduleDefinitions.collect { |definition|
            ~makeModuleEntry.value(definition)
        }
    )
};

~emitModulesState = { |correlationId = nil|
    var kind;
    kind = if(correlationId.notNil) { "response" } { "event" };
    ~demoModules = ~makeModulesPayload.value[\modules];
    ~engineState[\modules] = ~demoModules;
    ~sendEnvelope.value(kind, "modules.state", (modules: ~demoModules), correlationId);
};

~emitModulesStateThrottled = { |correlationId = nil, force = false|
    var now;

    if (correlationId.notNil) {
        force = true;
    };

    now = Date.getDate.rawSeconds;

    if (force or: { (now - ~lastModulesStateEmitSeconds) >= ~minLiveModulesStateEmitInterval }) {
        ~lastModulesStateEmitSeconds = now;
        ~emitModulesState.value(correlationId);
    };
};

~setModuleLifecycle = { |moduleId, lifecycleState|
    ~ensureModuleRuntimeDefaults.value;

    if (~moduleRuntimeState[moduleId].notNil) {
        ~moduleRuntimeState[moduleId].put(\lifecycleState, lifecycleState);
    };
};

~clearModuleActivationRequest = { |moduleId|
    ~ensureModuleRuntimeDefaults.value;

    if (~moduleRuntimeState[moduleId].notNil) {
        ~moduleRuntimeState[moduleId].put(\pendingActivationBeat, nil);
    };
};

~evaluateModuleCodeSurface = { |moduleId, surfaceId, codeSurface|
    var interpreted;
    var evalError;
    var message;
    var spec;
    var result;

    evalError = nil;
    if (codeSurface.contains("FAIL") or: { codeSurface.contains("throw") }) {
        evalError = Error("surface rejected by prototype safety guard");
    } {
        interpreted = try {
            codeSurface.interpret
        } { |error|
            evalError = error;
            nil
        };
    };

    if (evalError.notNil) {
        result = (
            ok: false,
            message: "eval error: " ++ evalError.errorString
        );
    } {
        if (surfaceId != "pattern") {
            result = (
                ok: true,
                message: "applied " ++ surfaceId.asString ++ " surface on next bar",
                params: ()
            );
        } {
            spec = if (interpreted.respondsTo(\at)) { interpreted } { nil };

            if (spec.isNil) {
                result = (
                    ok: false,
                    message: "pattern surface must evaluate to a dictionary-like object"
                );
            } {
                if (moduleId == ~kickModuleId) {
                result = (
                    ok: true,
                    message: "applied kick code surface on next bar",
                    params: (
                        amp: ~coerceFloat.value(~valueAtKey.value(spec, "amp"), 0.5),
                        baseFreq: ~coerceFloat.value(~valueAtKey.value(spec, "baseFreq"), 74.0),
                        dropFreq: ~coerceFloat.value(~valueAtKey.value(spec, "dropFreq"), 32.0),
                        noiseMix: ~coerceFloat.value(~valueAtKey.value(spec, "noiseMix"), 0.05),
                        decay: ~coerceFloat.value(~valueAtKey.value(spec, "decay"), 0.22)
                    )
                );
                } {
                    result = (
                        ok: true,
                        message: "applied pattern surface on next bar",
                        params: ()
                    );
                };
            };
        };
    };

    result
};

~applyPendingCodeSwap = { |moduleId, surfaceId, currentBeat|
    var runtime;
    var surface;
    var updatedSurface;
    var result;

    runtime = ~moduleRuntimeState[moduleId];
    surface = if (runtime.notNil) { ~surfaceById.value(runtime[\surfaces] ? [], surfaceId) } { nil };

    if (runtime.isNil or: { surface.isNil } or: { surface[\pendingCode].isNil }) {
        nil
    } {
        result = ~evaluateModuleCodeSurface.value(moduleId, surfaceId, surface[\pendingCode]);
        updatedSurface = surface.copy;

        if (result[\ok]) {
            updatedSurface.put(\activeCode, surface[\pendingCode]);
            updatedSurface.put(\state, "active");
            updatedSurface.put(\diagnostic, result[\message]);
            updatedSurface.put(\revision, (surface[\revision] ? 1) + 1);

            if (moduleId == ~kickModuleId and: { surfaceId == "pattern" }) {
                runtime.put(\activeSoundSpec, result[\params]);
            };
        } {
            updatedSurface.put(\state, "eval-error");
            updatedSurface.put(\diagnostic, result[\message] ++ " | preserved previous revision");
        };

        updatedSurface.put(\pendingCode, nil);
        updatedSurface.put(\pendingCodeSwapBeat, nil);
        updatedSurface.put(\pendingCodeSwapBarIndex, 0);
        ~replaceRuntimeSurface.value(runtime, updatedSurface);

        if (surfaceId == "pattern") {
            runtime.put(\activeCodeSurface, updatedSurface[\activeCode]);
            runtime.put(\pendingCodeSurface, nil);
            runtime.put(\pendingCodeSwapBeat, nil);
            runtime.put(\pendingCodeSwapBarIndex, 0);
            runtime.put(\codeSurfaceState, updatedSurface[\state]);
            runtime.put(\lastCodeEvalMessage, updatedSurface[\diagnostic]);
        };

        ~emitModulesState.value;
        if (~markProjectDirty.notNil) {
            ~markProjectDirty.value("module code surface evaluated");
        };
        ~sendEnvelope.value("event", "modules.codeSwapApplied", (
            moduleId: moduleId,
            surfaceId: surfaceId,
            appliedAtBeat: currentBeat,
            success: result[\ok],
            message: updatedSurface[\diagnostic],
            revision: updatedSurface[\revision]
        ));
    };
};

~processPendingModuleActions = {
    var currentBeat;

    currentBeat = ~captureTransportBeats.value;
    ~ensureModuleRuntimeDefaults.value;

    ~moduleDefinitions.do { |definition|
        var moduleId;
        var runtime;

        moduleId = definition[\id];
        runtime = ~moduleRuntimeState[moduleId];

        if (runtime.notNil) {
            (runtime[\surfaces] ? []).do { |surface|
                if (surface[\pendingCodeSwapBeat].notNil and: { currentBeat >= surface[\pendingCodeSwapBeat] }) {
                    ~applyPendingCodeSwap.value(moduleId, surface[\surfaceId], currentBeat);
                };
            };

            if (moduleId == ~kickModuleId and: { runtime[\pendingActivationBeat].notNil and: { currentBeat >= runtime[\pendingActivationBeat] } }) {
                runtime.put(\pendingActivationBeat, nil);
                runtime.put(\lastTriggeredStep, -1);
                ~setModuleLifecycle.value(~kickModuleId, "playing");
                ~emitModulesState.value;
            };
        };
    };
};

~startModuleActionLoop = {
    if (~moduleActionPoller.notNil) {
        ~moduleActionPoller.stop;
        ~moduleActionPoller = nil;
    };

    ~moduleActionPoller = Routine({
        loop {
            if (~transportState[\isPlaying]) {
                ~processPendingModuleActions.value;
            };

            0.01.wait;
        }
    }).play(SystemClock);
};

~installDemoSynthDefs = {
    SynthDef(\modDawKickPulse, { |out = 0, amp = 0.5, baseFreq = 74, dropFreq = 32, noiseMix = 0.05, decay = 0.22|
        var env;
        var pitchEnv;
        var signal;

        env = Env.perc(0.001, decay, amp, curve: -6).ar(doneAction: 2);
        pitchEnv = Env([baseFreq, dropFreq], [0.045], curve: -5).ar;
        signal = SinOsc.ar(pitchEnv, 0, env);
        signal = signal + (BPF.ar(WhiteNoise.ar(noiseMix), 1800, 0.5) * Env.perc(0.001, 0.02, 0.18).ar);
        signal = LeakDC.ar(signal);
        Out.ar(out, (signal ! 2));
    }).add;

    SynthDef(\modDawSpaceReturnHit, { |out = 0, amp = 0.12, freq = 180, decay = 1.2|
        var env;
        var signal;

        env = Env.perc(0.01, decay, amp, curve: -4).ar(doneAction: 2);
        signal = BPF.ar(PinkNoise.ar, freq, 0.35) * env;
        signal = FreeVerb2.ar(signal, signal, 0.45, 0.78, 0.35);
        Out.ar(out, signal);
    }).add;
};

~bootAudioEngine = {
    var server;
    var onReady;

    server = Server.default;
    ~audioServerReady = false;
    ~synthDefsInstalled = false;

    onReady = {
        ~installDemoSynthDefs.value;
        server.sync;
        ~synthDefsInstalled = true;
        ~audioServerReady = true;
        ~setModuleLifecycle.value(~kickModuleId, "ready");
        ~emitModulesState.value;

        if (~transportState[\isPlaying]) {
            ~startKickModule.value;
        };
    };

    if (server.serverRunning) {
        onReady.value;
    } {
        ~setModuleLifecycle.value(~kickModuleId, "booting-audio");
        ~emitModulesState.value;

        server.waitForBoot({
            onReady.value;
        });

        server.boot;
    };
};

~startKickModule = {
    if (~kickPoller.notNil) {
        ~kickPoller.stop;
        ~kickPoller = nil;
    };

    if (~audioServerReady.not or: { ~synthDefsInstalled.not }) {
        ~setModuleLifecycle.value(~kickModuleId, "waiting-audio");
        ~emitModulesState.value;
    } {
        ~kickPoller = Routine({
            var currentBeat;
            var currentStep;
            var runtime;
            var soundSpec;

            loop {
                if (~transportState[\isPlaying].not) {
                    0.02.wait;
                } {
                    runtime = ~moduleRuntimeState[~kickModuleId];
                    currentBeat = ~captureTransportBeats.value;

                    if (runtime.notNil and: { runtime[\lifecycleState] == "playing" }) {
                        currentStep = currentBeat.floor.asInteger;

                        if (currentStep > runtime[\lastTriggeredStep]) {
                            var mixerGain;
                            var sendGain;

                            runtime.put(\lastTriggeredStep, currentStep);
                            soundSpec = runtime[\activeSoundSpec] ? ();
                            if (~recordKickAnalysisEvent.notNil) {
                                ~recordKickAnalysisEvent.value(currentBeat, soundSpec);
                            };
                            mixerGain = ~effectiveModuleMixerGain.value(~kickModuleId);
                            sendGain = ~effectiveModuleSendGain.value(~kickModuleId, "return.reverb");

                            if (mixerGain > 0.0001) {
                                Synth(\modDawKickPulse, [
                                    \amp, (soundSpec[\amp] ? 0.5) * mixerGain,
                                    \baseFreq, soundSpec[\baseFreq] ? 74.0,
                                    \dropFreq, soundSpec[\dropFreq] ? 32.0,
                                    \noiseMix, soundSpec[\noiseMix] ? 0.05,
                                    \decay, soundSpec[\decay] ? 0.22
                                ]);
                            };

                            if (sendGain > 0.0001) {
                                Synth(\modDawSpaceReturnHit, [
                                    \amp, ((soundSpec[\amp] ? 0.5) * sendGain).clip(0.0, 0.6),
                                    \freq, (soundSpec[\baseFreq] ? 74.0) * 2.5,
                                    \decay, ((soundSpec[\decay] ? 0.22) * 5.5).clip(0.3, 2.5)
                                ]);
                            };
                        };
                    };

                    0.01.wait;
                };
            }
        }).play(SystemClock);
    };
};

~stopKickModule = {
    if (~kickPoller.notNil) {
        ~kickPoller.stop;
        ~kickPoller = nil;
    };

    ~clearModuleActivationRequest.value(~kickModuleId);
    if (~moduleRuntimeState[~kickModuleId].notNil) {
        ~moduleRuntimeState[~kickModuleId].put(\lastTriggeredStep, -1);
    };
    ~setModuleLifecycle.value(~kickModuleId, if(~audioServerReady) { "ready" } { "idle" });
    ~emitModulesState.value;
};

~scheduleModuleActivateNextBar = { |moduleId|
    var beatsPerBar;
    var currentBeat;
    var targetBeat;
    var targetBarIndex;

    ~ensureModuleRuntimeDefaults.value;

    if (moduleId != ~kickModuleId) {
        ~sendEnvelope.value("error", "modules.error", (message: "Only module.kick supports sound activation in M8", moduleId: moduleId));
        nil
    } {
        beatsPerBar = ~transportState[\meterNumerator].asFloat;
        currentBeat = ~captureTransportBeats.value;
        targetBeat = ((currentBeat / beatsPerBar).floor + 1) * beatsPerBar;
        targetBarIndex = (targetBeat / beatsPerBar).asInteger + 1;

        ~moduleRuntimeState[moduleId].put(\pendingActivationBeat, targetBeat);
        ~setModuleLifecycle.value(moduleId, if(~audioServerReady) { "queued-next-bar" } { "waiting-audio" });
        ~emitModulesState.value;
        ~sendEnvelope.value("event", "modules.activationScheduled", (
            moduleId: moduleId,
            quantisation: "next-bar",
            targetBeat: targetBeat,
            targetBarIndex: targetBarIndex
        ));

        if (~transportState[\isPlaying] and: { ~kickPoller.isNil }) {
            ~startKickModule.value;
        };
    };
};

~scheduleModuleCodeSurfaceUpdateNextBar = { |moduleId, codeSurface, surfaceId = "pattern"|
    var runtime;
    var surface;
    var updatedSurface;
    var beatsPerBar;
    var currentBeat;
    var targetBeat;
    var targetBarIndex;

    ~ensureModuleRuntimeDefaults.value;
    runtime = ~moduleRuntimeState[moduleId];
    surface = if (runtime.notNil) { ~surfaceById.value(runtime[\surfaces] ? [], surfaceId) } { nil };

    if (runtime.isNil) {
        ~sendEnvelope.value("error", "modules.error", (message: "Unknown module for code swap", moduleId: moduleId));
        nil
    } {
    if (surface.isNil) {
        ~sendEnvelope.value("error", "modules.error", (message: "Unknown surface for code swap", moduleId: moduleId, surfaceId: surfaceId));
        nil
    } {
        beatsPerBar = ~transportState[\meterNumerator].asFloat;
        currentBeat = ~captureTransportBeats.value;
        targetBeat = ((currentBeat / beatsPerBar).floor + 1) * beatsPerBar;
        targetBarIndex = (targetBeat / beatsPerBar).asInteger + 1;

        updatedSurface = surface.copy;
        updatedSurface.put(\pendingCode, codeSurface);
        updatedSurface.put(\pendingCodeSwapBeat, targetBeat);
        updatedSurface.put(\pendingCodeSwapBarIndex, targetBarIndex);
        updatedSurface.put(\state, "queued-next-bar");
        updatedSurface.put(\diagnostic, "pending engine eval on next bar");
        ~replaceRuntimeSurface.value(runtime, updatedSurface);

        if (surfaceId == "pattern") {
            runtime.put(\pendingCodeSurface, codeSurface);
            runtime.put(\pendingCodeSwapBeat, targetBeat);
            runtime.put(\pendingCodeSwapBarIndex, targetBarIndex);
            runtime.put(\codeSurfaceState, "queued-next-bar");
            runtime.put(\lastCodeEvalMessage, "pending engine eval on next bar");
        };

        ~emitModulesState.value;
        if (~markProjectDirty.notNil) {
            ~markProjectDirty.value("module code surface queued");
        };
        ~sendEnvelope.value("event", "modules.codeSwapScheduled", (
            moduleId: moduleId,
            surfaceId: surfaceId,
            quantisation: "next-bar",
            targetBeat: targetBeat,
            targetBarIndex: targetBarIndex
        ));
    };
    };
};
)
