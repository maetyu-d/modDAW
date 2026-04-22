(
~moduleDefinitions = [
    (
        id: "module.kick",
        displayName: "Kick Pulse",
        clockDomainId: "global.main",
        lifecycleState: "idle",
        behaviourType: "demo-sound",
        codeSurface: "(\namp: 0.5,\nbaseFreq: 74,\ndropFreq: 32,\nnoiseMix: 0.05,\ndecay: 0.22\n)",
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
        codeSurface: "(\npatternDensity: 0.5,\naccent: 0.2\n)"
    ),
    (
        id: "module.texture",
        displayName: "Texture Drift",
        clockDomainId: "domain.triplet",
        lifecycleState: "idle",
        behaviourType: "demo-pattern",
        codeSurface: "(\nspread: 0.35,\nmodDepth: 0.15\n)"
    )
];

~demoModules = [];
~moduleRuntimeState = Dictionary.new;
~audioServerReady = false;
~kickPoller = nil;
~moduleActionPoller = nil;
~kickModuleId = "module.kick";

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

~ensureModuleRuntimeDefaults = {
    ~moduleDefinitions.do { |definition|
        if (~moduleRuntimeState[definition[\id]].isNil) {
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
                activeSoundSpec: definition[\initialSoundSpec] ? ()
            );
        };
    };
};

~makeModuleEntry = { |definition|
    var runtime;

    ~ensureModuleRuntimeDefaults.value;
    runtime = ~moduleRuntimeState[definition[\id]];

    (
        id: definition[\id],
        displayName: definition[\displayName],
        clockDomainId: definition[\clockDomainId],
        lifecycleState: runtime[\lifecycleState],
        behaviourType: definition[\behaviourType],
        codeSurface: runtime[\activeCodeSurface],
        pendingCodeSurface: runtime[\pendingCodeSurface] ? "",
        codeSurfaceState: runtime[\codeSurfaceState] ? "active",
        lastCodeEvalMessage: runtime[\lastCodeEvalMessage] ? "",
        pendingCodeSwapBarIndex: runtime[\pendingCodeSwapBarIndex] ? 0
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

~evaluateModuleCodeSurface = { |moduleId, codeSurface|
    var interpreted;
    var evalError;
    var message;
    var spec;
    var result;

    evalError = nil;
    interpreted = try {
        codeSurface.interpret
    } { |error|
        evalError = error;
        nil
    };

    if (evalError.notNil) {
        result = (
            ok: false,
            message: "eval error: " ++ evalError.errorString
        );
    } {
        spec = if (interpreted.respondsTo(\at)) { interpreted } { nil };

        if (spec.isNil) {
            result = (
                ok: false,
                message: "code surface must evaluate to a dictionary-like object"
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
                    message: "applied module code surface on next bar",
                    params: ()
                );
            };
        };
    };

    result
};

~applyPendingCodeSwap = { |moduleId, currentBeat|
    var runtime;
    var result;

    runtime = ~moduleRuntimeState[moduleId];

    if (runtime.isNil or: { runtime[\pendingCodeSurface].isNil }) {
        nil
    } {
        result = ~evaluateModuleCodeSurface.value(moduleId, runtime[\pendingCodeSurface]);

        if (result[\ok]) {
            runtime.put(\activeCodeSurface, runtime[\pendingCodeSurface]);
            runtime.put(\codeSurfaceState, "active");
            runtime.put(\lastCodeEvalMessage, result[\message]);

            if (moduleId == ~kickModuleId) {
                runtime.put(\activeSoundSpec, result[\params]);
            };
        } {
            runtime.put(\codeSurfaceState, "eval-error");
            runtime.put(\lastCodeEvalMessage, result[\message]);
        };

        runtime.put(\pendingCodeSurface, nil);
        runtime.put(\pendingCodeSwapBeat, nil);
        runtime.put(\pendingCodeSwapBarIndex, 0);
        ~emitModulesState.value;
        ~sendEnvelope.value("event", "modules.codeSwapApplied", (
            moduleId: moduleId,
            appliedAtBeat: currentBeat,
            success: result[\ok],
            message: result[\message]
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
            if (runtime[\pendingCodeSwapBeat].notNil and: { currentBeat >= runtime[\pendingCodeSwapBeat] }) {
                ~applyPendingCodeSwap.value(moduleId, currentBeat);
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
};

~bootAudioEngine = {
    var server;

    server = Server.default;

    if (server.serverRunning) {
        ~audioServerReady = true;
        ~installDemoSynthDefs.value;
        ~setModuleLifecycle.value(~kickModuleId, "ready");
        ~emitModulesState.value;
    } {
        ~setModuleLifecycle.value(~kickModuleId, "booting-audio");
        ~emitModulesState.value;

        server.waitForBoot({
            ~audioServerReady = true;
            ~installDemoSynthDefs.value;
            ~setModuleLifecycle.value(~kickModuleId, "ready");
            ~emitModulesState.value;

            if (~transportState[\isPlaying]) {
                ~startKickModule.value;
            };
        });

        server.boot;
    };
};

~startKickModule = {
    if (~kickPoller.notNil) {
        ~kickPoller.stop;
        ~kickPoller = nil;
    };

    if (~audioServerReady.not) {
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

                            runtime.put(\lastTriggeredStep, currentStep);
                            soundSpec = runtime[\activeSoundSpec] ? ();
                            mixerGain = ~effectiveModuleMixerGain.value(~kickModuleId);

                            if (mixerGain > 0.0001) {
                                Synth(\modDawKickPulse, [
                                    \amp, (soundSpec[\amp] ? 0.5) * mixerGain,
                                    \baseFreq, soundSpec[\baseFreq] ? 74.0,
                                    \dropFreq, soundSpec[\dropFreq] ? 32.0,
                                    \noiseMix, soundSpec[\noiseMix] ? 0.05,
                                    \decay, soundSpec[\decay] ? 0.22
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

~scheduleModuleCodeSurfaceUpdateNextBar = { |moduleId, codeSurface|
    var runtime;
    var beatsPerBar;
    var currentBeat;
    var targetBeat;
    var targetBarIndex;

    ~ensureModuleRuntimeDefaults.value;
    runtime = ~moduleRuntimeState[moduleId];

    if (runtime.isNil) {
        ~sendEnvelope.value("error", "modules.error", (message: "Unknown module for code swap", moduleId: moduleId));
        nil
    } {
        beatsPerBar = ~transportState[\meterNumerator].asFloat;
        currentBeat = ~captureTransportBeats.value;
        targetBeat = ((currentBeat / beatsPerBar).floor + 1) * beatsPerBar;
        targetBarIndex = (targetBeat / beatsPerBar).asInteger + 1;

        runtime.put(\pendingCodeSurface, codeSurface);
        runtime.put(\pendingCodeSwapBeat, targetBeat);
        runtime.put(\pendingCodeSwapBarIndex, targetBarIndex);
        runtime.put(\codeSurfaceState, "queued-next-bar");
        runtime.put(\lastCodeEvalMessage, "pending engine eval on next bar");
        ~emitModulesState.value;
        ~sendEnvelope.value("event", "modules.codeSwapScheduled", (
            moduleId: moduleId,
            quantisation: "next-bar",
            targetBeat: targetBeat,
            targetBarIndex: targetBarIndex
        ));
    };
};
)
