(
~mixerDefinitions = [
    (
        id: "master.main",
        displayName: "Master",
        kind: "master",
        targetModuleId: nil,
        hasAudioPath: true,
        level: 1.0,
        muted: false
    ),
    (
        id: "strip.module.kick",
        displayName: "Kick Pulse",
        kind: "module",
        targetModuleId: "module.kick",
        hasAudioPath: true,
        level: 1.0,
        muted: false
    ),
    (
        id: "strip.module.hat",
        displayName: "Hat Grid",
        kind: "module",
        targetModuleId: "module.hat",
        hasAudioPath: false,
        level: 1.0,
        muted: false
    ),
    (
        id: "strip.module.texture",
        displayName: "Texture Drift",
        kind: "module",
        targetModuleId: "module.texture",
        hasAudioPath: false,
        level: 1.0,
        muted: false
    )
];

~mixerRuntimeState = Dictionary.new;
~mixerState = [];

~mixerDefinitionForStripId = { |stripId|
    ~mixerDefinitions.detect({ |definition| definition[\id] == stripId }, { nil })
};

~mixerDefinitionForModuleId = { |moduleId|
    ~mixerDefinitions.detect({ |definition| definition[\targetModuleId] == moduleId }, { nil })
};

~ensureMixerRuntimeDefaults = {
    ~mixerDefinitions.do { |definition|
        if (~mixerRuntimeState[definition[\id]].isNil) {
            ~mixerRuntimeState[definition[\id]] = (
                level: definition[\level] ? 1.0,
                muted: definition[\muted] ? false
            );
        };
    };
};

~makeMixerEntry = { |definition|
    var runtime;

    ~ensureMixerRuntimeDefaults.value;
    runtime = ~mixerRuntimeState[definition[\id]];

    (
        id: definition[\id],
        displayName: definition[\displayName],
        kind: definition[\kind],
        targetModuleId: definition[\targetModuleId],
        hasAudioPath: definition[\hasAudioPath] ? false,
        level: runtime[\level] ? 1.0,
        muted: runtime[\muted] ? false
    )
};

~makeMixerPayload = {
    (
        strips: ~mixerDefinitions.collect { |definition|
            ~makeMixerEntry.value(definition)
        }
    )
};

~emitMixerState = { |correlationId = nil|
    var kind;

    kind = if (correlationId.notNil) { "response" } { "event" };
    ~mixerState = ~makeMixerPayload.value[\strips];
    ~engineState[\mixer] = ~mixerState;
    ~sendEnvelope.value(kind, "mixer.state", (strips: ~mixerState), correlationId);
};

~clampMixerLevel = { |level|
    level.asFloat.clip(0.0, 1.5)
};

~setMixerStripLevel = { |stripId, level|
    var runtime;

    ~ensureMixerRuntimeDefaults.value;
    runtime = ~mixerRuntimeState[stripId];

    if (runtime.isNil) {
        ~sendEnvelope.value("error", "mixer.error", (message: "Unknown mixer strip", stripId: stripId));
    } {
        runtime.put(\level, ~clampMixerLevel.value(level ? 1.0));
        ~emitMixerState.value;
    };
};

~setMixerStripMuted = { |stripId, muted|
    var runtime;

    ~ensureMixerRuntimeDefaults.value;
    runtime = ~mixerRuntimeState[stripId];

    if (runtime.isNil) {
        ~sendEnvelope.value("error", "mixer.error", (message: "Unknown mixer strip", stripId: stripId));
    } {
        runtime.put(\muted, ~coerceBoolean.value(muted, false));
        ~emitMixerState.value;
    };
};

~masterMixerGain = {
    var runtime;

    ~ensureMixerRuntimeDefaults.value;
    runtime = ~mixerRuntimeState["master.main"];

    if (runtime.isNil or: { runtime[\muted] == true }) {
        0.0
    } {
        runtime[\level] ? 1.0
    }
};

~effectiveModuleMixerGain = { |moduleId|
    var route;
    var stripId;
    var runtime;
    var moduleGain;

    ~ensureMixerRuntimeDefaults.value;

    if (~enabledAudioRouteForModule.notNil) {
        route = ~enabledAudioRouteForModule.value(moduleId);
        stripId = if (route.notNil) { ~mixerStripIdForAudioDestination.value(route[\destination]) } { nil };
    } {
        var definition;
        definition = ~mixerDefinitionForModuleId.value(moduleId);
        stripId = if (definition.notNil) { definition[\id] } { nil };
    };

    runtime = if (stripId.notNil) { ~mixerRuntimeState[stripId] } { nil };
    moduleGain = if (runtime.isNil or: { runtime[\muted] == true }) { 0.0 } { runtime[\level] ? 1.0 };

    moduleGain * ~masterMixerGain.value
};
)
