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
        id: "return.reverb",
        displayName: "Space Return",
        kind: "return",
        targetModuleId: nil,
        assignedGroupId: nil,
        hasAudioPath: true,
        level: 0.65,
        muted: false
    ),
    (
        id: "group.drums",
        displayName: "Drum Group",
        kind: "group",
        targetModuleId: nil,
        assignedGroupId: nil,
        hasAudioPath: true,
        level: 1.0,
        muted: false
    ),
    (
        id: "strip.module.kick",
        displayName: "Kick Pulse",
        kind: "module",
        targetModuleId: "module.kick",
        assignedGroupId: "group.drums",
        hasAudioPath: true,
        level: 1.0,
        muted: false
    ),
    (
        id: "strip.module.hat",
        displayName: "Hat Grid",
        kind: "module",
        targetModuleId: "module.hat",
        assignedGroupId: "group.drums",
        hasAudioPath: false,
        level: 1.0,
        muted: false
    ),
    (
        id: "strip.module.texture",
        displayName: "Texture Drift",
        kind: "module",
        targetModuleId: "module.texture",
        assignedGroupId: "group.drums",
        hasAudioPath: false,
        level: 1.0,
        muted: false
    )
];

~sendDefinitions = [
    (
        sendId: "send.kick.space",
        displayName: "Kick -> Space",
        sourceStripId: "strip.module.kick",
        targetModuleId: "module.kick",
        returnStripId: "return.reverb",
        level: 0.0,
        mode: "post"
    ),
    (
        sendId: "send.hat.space",
        displayName: "Hat -> Space",
        sourceStripId: "strip.module.hat",
        targetModuleId: "module.hat",
        returnStripId: "return.reverb",
        level: 0.0,
        mode: "post"
    ),
    (
        sendId: "send.texture.space",
        displayName: "Texture -> Space",
        sourceStripId: "strip.module.texture",
        targetModuleId: "module.texture",
        returnStripId: "return.reverb",
        level: 0.0,
        mode: "post"
    )
];

~mixerRuntimeState = Dictionary.new;
~sendRuntimeState = Dictionary.new;
~mixerState = [];

~mixerDefinitionForStripId = { |stripId|
    ~mixerDefinitions.detect({ |definition| definition[\id] == stripId }, { nil })
};

~mixerDefinitionForModuleId = { |moduleId|
    ~mixerDefinitions.detect({ |definition| definition[\targetModuleId] == moduleId }, { nil })
};

~mixerGroupDefinitions = {
    ~mixerDefinitions.select({ |definition| definition[\kind] == "group" })
};

~mixerGroupDefinitionForId = { |groupId|
    ~mixerDefinitions.detect({ |definition| definition[\id] == groupId and: { definition[\kind] == "group" } }, { nil })
};

~sendDefinitionForId = { |sendId|
    ~sendDefinitions.detect({ |definition| definition[\sendId] == sendId }, { nil })
};

~ensureMixerRuntimeDefaults = {
    ~mixerDefinitions.do { |definition|
        if (~mixerRuntimeState[definition[\id]].isNil) {
            ~mixerRuntimeState[definition[\id]] = (
                level: definition[\level] ? 1.0,
                muted: definition[\muted] ? false,
                assignedGroupId: definition[\assignedGroupId]
            );
        };
    };

    ~sendDefinitions.do { |definition|
        if (~sendRuntimeState[definition[\sendId]].isNil) {
            ~sendRuntimeState[definition[\sendId]] = (
                level: definition[\level] ? 0.0,
                mode: definition[\mode] ? "post"
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
        assignedGroupId: runtime[\assignedGroupId] ? definition[\assignedGroupId],
        childCount: ~mixerDefinitions.count({ |candidate|
            var candidateRuntime;
            candidateRuntime = ~mixerRuntimeState[candidate[\id]];
            candidate[\kind] == "module" and: { (candidateRuntime[\assignedGroupId] ? candidate[\assignedGroupId]) == definition[\id] }
        }),
        hasAudioPath: definition[\hasAudioPath] ? false,
        level: runtime[\level] ? 1.0,
        muted: runtime[\muted] ? false
    )
};

~makeMixerPayload = {
    (
        strips: ~mixerDefinitions.collect { |definition|
            ~makeMixerEntry.value(definition)
        },
        sends: ~sendDefinitions.collect { |definition|
            var runtime;
            runtime = ~sendRuntimeState[definition[\sendId]];
            (
                sendId: definition[\sendId],
                displayName: definition[\displayName],
                sourceStripId: definition[\sourceStripId],
                targetModuleId: definition[\targetModuleId],
                returnStripId: definition[\returnStripId],
                level: runtime[\level] ? definition[\level] ? 0.0,
                mode: runtime[\mode] ? definition[\mode] ? "post"
            )
        },
        groups: ~mixerGroupDefinitions.value.collect { |definition|
            (
                id: definition[\id],
                displayName: definition[\displayName]
            )
        }
    )
};

~emitMixerState = { |correlationId = nil|
    var kind;
    var payload;

    kind = if (correlationId.notNil) { "response" } { "event" };
    payload = ~makeMixerPayload.value;
    ~mixerState = payload[\strips];
    ~engineState[\mixer] = ~mixerState;
    ~sendEnvelope.value(kind, "mixer.state", payload, correlationId);
};

~clampMixerLevel = { |level|
    level.asFloat.clip(0.0, 1.5)
};

~clampSendLevel = { |level|
    level.asFloat.clip(0.0, 1.0)
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
        if (~markProjectDirty.notNil) { ~markProjectDirty.value("mixer level changed") };
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
        if (~markProjectDirty.notNil) { ~markProjectDirty.value("mixer mute changed") };
    };
};

~setMixerSendLevel = { |sendId, level|
    var runtime;

    ~ensureMixerRuntimeDefaults.value;
    runtime = ~sendRuntimeState[sendId];

    if (runtime.isNil) {
        ~sendEnvelope.value("error", "mixer.error", (message: "Unknown mixer send", sendId: sendId));
    } {
        runtime.put(\level, ~clampSendLevel.value(level ? 0.0));
        ~emitMixerState.value;
        if (~markProjectDirty.notNil) { ~markProjectDirty.value("send level changed") };
    };
};

~setMixerSendMode = { |sendId, mode|
    var runtime;
    var modeText;

    ~ensureMixerRuntimeDefaults.value;
    runtime = ~sendRuntimeState[sendId];
    modeText = mode.asString;

    if (runtime.isNil) {
        ~sendEnvelope.value("error", "mixer.error", (message: "Unknown mixer send", sendId: sendId));
    } {
        if (["pre", "post"].detect({ |allowed| allowed == modeText }, { nil }).isNil) {
            ~sendEnvelope.value("error", "mixer.error", (message: "Send mode must be pre or post", sendId: sendId, mode: modeText));
        } {
            runtime.put(\mode, modeText);
            ~emitMixerState.value;
            if (~markProjectDirty.notNil) { ~markProjectDirty.value("send mode changed") };
        };
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

~mixerStripGain = { |stripId|
    var runtime;

    ~ensureMixerRuntimeDefaults.value;
    runtime = ~mixerRuntimeState[stripId];

    if (runtime.isNil or: { runtime[\muted] == true }) {
        0.0
    } {
        runtime[\level] ? 1.0
    }
};

~setMixerStripGroup = { |stripId, groupId|
    var definition;
    var runtime;
    var normalizedGroupId;

    ~ensureMixerRuntimeDefaults.value;
    definition = ~mixerDefinitionForStripId.value(stripId);
    runtime = ~mixerRuntimeState[stripId];
    normalizedGroupId = if (groupId.isNil) { "" } { groupId.asString };

    if (definition.isNil or: { runtime.isNil }) {
        ~sendEnvelope.value("error", "mixer.error", (message: "Unknown mixer strip", stripId: stripId));
    } {
        if (definition[\kind] != "module") {
            ~sendEnvelope.value("error", "mixer.error", (message: "Only module strips can be assigned to a group", stripId: stripId));
        } {
            if (normalizedGroupId != "" and: { ~mixerGroupDefinitionForId.value(normalizedGroupId).isNil }) {
                ~sendEnvelope.value("error", "mixer.error", (message: "Unknown mixer group", groupId: normalizedGroupId));
            } {
                runtime.put(\assignedGroupId, normalizedGroupId);
                ~emitMixerState.value;
                if (~markProjectDirty.notNil) { ~markProjectDirty.value("mixer group assignment changed") };
            };
        };
    };
};

~effectiveModuleBusGain = { |moduleId|
    var route;
    var stripId;
    var definition;
    var runtime;
    var moduleGain;
    var groupId;
    var groupGain;

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
    definition = if (stripId.notNil) { ~mixerDefinitionForStripId.value(stripId) } { nil };
    moduleGain = if (runtime.isNil or: { runtime[\muted] == true }) { 0.0 } { runtime[\level] ? 1.0 };
    groupId = if (runtime.notNil) { runtime[\assignedGroupId] ? if (definition.notNil) { definition[\assignedGroupId] } { nil } } { nil };
    if (groupId == "") { groupId = nil };
    groupGain = if (groupId.notNil) { ~mixerStripGain.value(groupId) } { 1.0 };

    moduleGain * groupGain
};

~effectiveModuleMixerGain = { |moduleId|
    ~effectiveModuleBusGain.value(moduleId) * ~masterMixerGain.value
};

~effectiveModuleSendGain = { |moduleId, returnStripId = "return.reverb"|
    var sendDefinition;
    var runtime;
    var sendLevel;
    var mode;
    var sourceGain;
    var returnGain;

    ~ensureMixerRuntimeDefaults.value;

    sendDefinition = ~sendDefinitions.detect({ |definition|
        definition[\targetModuleId] == moduleId and: { definition[\returnStripId] == returnStripId }
    }, { nil });

    if (sendDefinition.isNil) {
        0.0
    } {
        runtime = ~sendRuntimeState[sendDefinition[\sendId]];
        sendLevel = runtime[\level] ? sendDefinition[\level] ? 0.0;
        mode = runtime[\mode] ? sendDefinition[\mode] ? "post";
        sourceGain = if (mode == "pre") { 1.0 } { ~effectiveModuleBusGain.value(moduleId) };
        returnGain = ~mixerStripGain.value(returnStripId);

        sourceGain * sendLevel * returnGain * ~masterMixerGain.value
    }
};
)
