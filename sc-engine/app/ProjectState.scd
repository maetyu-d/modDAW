(
~projectStateDefaultPath = ~engineRoot +/+ "projects" +/+ "demo-project.json";
~recoveryStateDefaultPath = ~engineRoot +/+ "projects" +/+ "recovery-autosave.json";
~projectDirty = false;
~lastRecoverySnapshotAt = "";
~lastRecoveryReason = "clean startup";

~arrayOrEmpty = { |value|
    if (value.isNil) { [] } { value }
};

~makeProjectSnapshot = {
    var modules;
    var mixer;
    var sends;
    var codeSurfaces;

    ~ensureModuleRuntimeDefaults.value;
    ~ensureMixerRuntimeDefaults.value;

    modules = ~moduleDefinitions.collect { |definition|
        var runtime;
        var surfaces;

        runtime = ~moduleRuntimeState[definition[\id]];
        surfaces = (runtime[\surfaces] ? []).collect { |surface|
            (
                surfaceId: surface[\surfaceId],
                displayName: surface[\displayName],
                role: surface[\role],
                code: surface[\activeCode],
                revision: surface[\revision] ? 1,
                state: surface[\state] ? "active",
                diagnostic: surface[\diagnostic] ? ""
            )
        };

        (
            id: definition[\id],
            displayName: definition[\displayName],
            clockDomainId: definition[\clockDomainId],
            lifecycleState: "idle",
            behaviourType: definition[\behaviourType],
            laneType: definition[\laneType] ? "module",
            codeSurface: runtime[\activeCodeSurface] ? definition[\codeSurface],
            codeSurfaces: surfaces
        )
    };

    sends = ~sendDefinitions.collect { |definition|
        var runtime;

        runtime = ~sendRuntimeState[definition[\sendId]];
        (
            sendId: definition[\sendId],
            level: runtime[\level] ? definition[\level] ? 0.0,
            mode: runtime[\mode] ? definition[\mode] ? "post"
        )
    };

    mixer = ~mixerDefinitions.collect { |definition|
        var runtime;

        runtime = ~mixerRuntimeState[definition[\id]];
        (
            stripId: definition[\id],
            level: runtime[\level] ? 1.0,
            muted: runtime[\muted] ? false,
            assignedGroupId: runtime[\assignedGroupId] ? definition[\assignedGroupId]
        )
    };

    codeSurfaces = modules.collect { |module|
        (~arrayOrEmpty.value(module[\codeSurfaces])).collect { |surface|
            (
                moduleId: module[\id],
                surfaceId: surface[\surfaceId],
                displayName: surface[\displayName],
                role: surface[\role],
                code: surface[\code],
                revision: surface[\revision] ? 1,
                state: surface[\state] ? "active",
                diagnostic: surface[\diagnostic] ? ""
            )
        }
    };
    codeSurfaces = codeSurfaces.flat;

    (
        schemaVersion: "moddaw.project.1",
        savedAt: Date.getDate.stamp,
        modules: modules,
        clockDomains: ~clockDomainDefinitions,
        timingAttachments: modules.collect { |module|
            (
                moduleId: module[\id],
                clockDomainId: module[\clockDomainId]
            )
        },
        routes: ~routeDefinitions,
        regions: ~arrangementRegions,
        automation: ~automationLanes,
        structural: ~structuralDirectives,
        mixer: mixer,
        sends: sends,
        codeSurfaces: codeSurfaces
    )
};

~moduleErrorSummaries = {
    var errors;

    ~ensureModuleRuntimeDefaults.value;
    errors = List.new;

    ~moduleRuntimeState.keysValuesDo { |moduleId, runtime|
        (runtime[\surfaces] ? []).do { |surface|
            if ((surface[\state] ? "active") == "eval-error") {
                errors.add((
                    moduleId: moduleId,
                    surfaceId: surface[\surfaceId],
                    diagnostic: surface[\diagnostic] ? "surface eval failed"
                ));
            };
        };
    };

    errors.asArray
};

~makeRecoveryPayload = {
    (
        engineOnline: true,
        audioServerReady: ~audioServerReady,
        projectDirty: ~projectDirty,
        recoveryPath: ~recoveryStateDefaultPath,
        lastRecoverySnapshotAt: ~lastRecoverySnapshotAt,
        lastReason: ~lastRecoveryReason,
        moduleErrors: ~moduleErrorSummaries.value
    )
};

~emitRecoveryState = { |correlationId = nil|
    var kind;
    var payload;

    kind = if (correlationId.notNil) { "response" } { "event" };
    payload = ~makeRecoveryPayload.value;
    ~engineState[\recovery] = payload;
    ~sendEnvelope.value(kind, "engine.recoveryState", payload, correlationId);
};

~writeRecoverySnapshot = { |reason = "state changed"|
    var snapshot;
    var file;

    snapshot = ~makeProjectSnapshot.value;
    ~lastRecoverySnapshotAt = Date.getDate.stamp;
    ~lastRecoveryReason = reason;
    snapshot.put(\recoveryMeta, (
        dirty: ~projectDirty,
        reason: reason,
        savedAt: ~lastRecoverySnapshotAt
    ));

    file = File(~recoveryStateDefaultPath, "w");
    file.write(~jsonEncodeValue.value(snapshot) ++ "\n");
    file.close;
};

~markProjectDirty = { |reason = "state changed"|
    ~projectDirty = true;
    ~writeRecoverySnapshot.value(reason);
    ~emitRecoveryState.value;
};

~markProjectClean = { |reason = "project saved"|
    ~projectDirty = false;
    ~writeRecoverySnapshot.value(reason);
    ~emitRecoveryState.value;
};

~applyProjectSnapshot = { |project|
    var modules;
    var clockDomains;
    var routes;
    var mixer;
    var sends;
    var codeSurfaces;
    var regions;
    var automation;
    var structural;

    modules = ~arrayOrEmpty.value(~valueAtKey.value(project, "modules"));
    clockDomains = ~arrayOrEmpty.value(~valueAtKey.value(project, "clockDomains"));
    routes = ~arrayOrEmpty.value(~valueAtKey.value(project, "routes"));
    mixer = ~arrayOrEmpty.value(~valueAtKey.value(project, "mixer"));
    sends = ~arrayOrEmpty.value(~valueAtKey.value(project, "sends"));
    codeSurfaces = ~arrayOrEmpty.value(~valueAtKey.value(project, "codeSurfaces"));
    regions = ~arrayOrEmpty.value(~valueAtKey.value(project, "regions"));
    automation = ~arrayOrEmpty.value(~valueAtKey.value(project, "automation"));
    structural = ~arrayOrEmpty.value(~valueAtKey.value(project, "structural"));

    if (clockDomains.notEmpty) {
        ~clockDomainDefinitions = clockDomains.collect { |domain|
            (
                id: ~valueAtKey.value(domain, "id"),
                parentId: ~valueAtKey.value(domain, "parentId"),
                kind: ~valueAtKey.value(domain, "kind"),
                displayName: ~valueAtKey.value(domain, "displayName"),
                relationType: ~valueAtKey.value(domain, "relationType") ? if ((~valueAtKey.value(domain, "kind") ? "") == "global") { "global" } { "tempoShared" },
                ratioToParent: (~valueAtKey.value(domain, "ratioToParent") ? 1.0).asFloat,
                phaseOffsetBeats: (~valueAtKey.value(domain, "phaseOffsetBeats") ? 0.0).asFloat,
                phraseLengthBars: (~valueAtKey.value(domain, "phraseLengthBars") ? 2).asInteger.max(1)
            )
        };
    };

    if (modules.notEmpty) {
        ~moduleDefinitions = modules.collect { |module|
            var moduleId;
            var savedSurfaces;
            var codeSurface;
            var definitionSurfaces;

            moduleId = ~valueAtKey.value(module, "id");
            savedSurfaces = codeSurfaces.select({ |surface| ~valueAtKey.value(surface, "moduleId") == moduleId });
            definitionSurfaces = if (savedSurfaces.notEmpty) {
                savedSurfaces.collect { |surface|
                    (
                        surfaceId: ~valueAtKey.value(surface, "surfaceId"),
                        displayName: ~valueAtKey.value(surface, "displayName") ? ~valueAtKey.value(surface, "surfaceId"),
                        role: ~valueAtKey.value(surface, "role") ? ~valueAtKey.value(surface, "surfaceId"),
                        code: ~valueAtKey.value(surface, "code"),
                        revision: ~valueAtKey.value(surface, "revision") ? 1,
                        state: ~valueAtKey.value(surface, "state") ? "active",
                        diagnostic: ~valueAtKey.value(surface, "diagnostic") ? ""
                    )
                }
            } {
                ~arrayOrEmpty.value(~valueAtKey.value(module, "codeSurfaces"))
            };

            if (definitionSurfaces.isEmpty) {
                definitionSurfaces = [
                    (surfaceId: "pattern", displayName: "Pattern", role: "pattern", code: ~valueAtKey.value(module, "codeSurface"))
                ];
            };

            codeSurface = (~surfaceById.value(definitionSurfaces, "pattern") ? definitionSurfaces.first)[\code];

            (
                id: moduleId,
                displayName: ~valueAtKey.value(module, "displayName"),
                clockDomainId: ~valueAtKey.value(module, "clockDomainId"),
                lifecycleState: "idle",
                behaviourType: ~valueAtKey.value(module, "behaviourType"),
                laneType: ~valueAtKey.value(module, "laneType") ? if ((~valueAtKey.value(module, "behaviourType") ? "").contains("structural")) { "structural" } { "module" },
                codeSurface: codeSurface,
                codeSurfaces: definitionSurfaces,
                initialSoundSpec: ()
            )
        };
    };

    if (routes.notEmpty) {
        ~routeDefinitions = routes.collect { |route|
            (
                routeId: ~valueAtKey.value(route, "routeId"),
                family: ~valueAtKey.value(route, "family"),
                source: ~valueAtKey.value(route, "source"),
                destination: ~valueAtKey.value(route, "destination"),
                enabled: ~coerceBoolean.value(~valueAtKey.value(route, "enabled"), false)
            )
        };
        ~routeNextId = ~routeDefinitions.size + 1;
    };

    ~arrangementRegions = regions.collect { |region|
        (
            regionId: ~valueAtKey.value(region, "regionId"),
            moduleId: ~valueAtKey.value(region, "moduleId"),
            displayName: ~valueAtKey.value(region, "displayName"),
            kind: ~valueAtKey.value(region, "kind"),
            regionIdentity: ~valueAtKey.value(region, "regionIdentity") ? if ((~valueAtKey.value(region, "source") ? "") == "live-link") { "live-linked" } { "frozen" },
            editPolicy: ~valueAtKey.value(region, "editPolicy") ? if ((~valueAtKey.value(region, "source") ? "") == "live-link") { "projection-only" } { "detached-audio" },
            source: ~valueAtKey.value(region, "source"),
            startBeat: (~valueAtKey.value(region, "startBeat") ? 0.0).asFloat,
            lengthBeats: (~valueAtKey.value(region, "lengthBeats") ? 4.0).asFloat,
            eventOffsets: ~arrayOrEmpty.value(~valueAtKey.value(region, "eventOffsets")),
            replayedEventIndexes: [],
            linkedModuleId: ~valueAtKey.value(region, "linkedModuleId") ? ~valueAtKey.value(region, "moduleId"),
            lastEditMessage: ~valueAtKey.value(region, "lastEditMessage") ? "",
            soundSpec: ~valueAtKey.value(region, "soundSpec") ? ()
        )
    };
    ~arrangementRegionNextId = ~arrangementRegions.size + 1;

    if (automation.notEmpty) {
        ~automationLanes = automation.collect { |lane|
            (
                laneId: ~valueAtKey.value(lane, "laneId"),
                displayName: ~valueAtKey.value(lane, "displayName"),
                targetType: ~valueAtKey.value(lane, "targetType"),
                targetId: ~valueAtKey.value(lane, "targetId"),
                parameterId: ~valueAtKey.value(lane, "parameterId"),
                interpolation: ~valueAtKey.value(lane, "interpolation") ? "linear",
                points: ~arrayOrEmpty.value(~valueAtKey.value(lane, "points")),
                currentValue: (~valueAtKey.value(lane, "currentValue") ? 1.0).asFloat
            )
        };
    };

    ~structuralDirectives = structural;
    ~structuralNextDirectiveId = ~structuralDirectives.size + 1;

    if (~installStructuralModule.notNil) {
        ~installStructuralModule.value;
    };
    if (~installAnalysisModule.notNil) {
        ~installAnalysisModule.value;
    };

    ~moduleRuntimeState = Dictionary.new;
    ~ensureModuleRuntimeDefaults.value;

    ~moduleDefinitions.do { |definition|
        var runtime;
        var result;

        runtime = ~moduleRuntimeState[definition[\id]];
        runtime.put(\surfaces, ~definitionSurfaces.value(definition).collect { |surface|
            ~makeRuntimeSurface.value(surface)
        });
        runtime.put(\activeCodeSurface, definition[\codeSurface]);
        runtime.put(\lifecycleState, if(definition[\id] == ~kickModuleId and: { ~audioServerReady }) { "ready" } { "idle" });

        if (definition[\id] == ~kickModuleId) {
            result = ~evaluateModuleCodeSurface.value(definition[\id], "pattern", definition[\codeSurface]);

            if (result[\ok]) {
                runtime.put(\activeSoundSpec, result[\params]);
                runtime.put(\codeSurfaceState, "active");
                runtime.put(\lastCodeEvalMessage, "loaded project code surface");
            } {
                runtime.put(\codeSurfaceState, "eval-error");
                runtime.put(\lastCodeEvalMessage, result[\message]);
            };
        };
    };

    ~mixerRuntimeState = Dictionary.new;
    ~ensureMixerRuntimeDefaults.value;
    mixer.do { |strip|
        var stripId;
        var runtime;

        stripId = ~valueAtKey.value(strip, "stripId");
        runtime = ~mixerRuntimeState[stripId];

        if (runtime.notNil) {
            runtime.put(\level, ~clampMixerLevel.value(~valueAtKey.value(strip, "level") ? 1.0));
            runtime.put(\muted, ~coerceBoolean.value(~valueAtKey.value(strip, "muted"), false));
            runtime.put(\assignedGroupId, ~valueAtKey.value(strip, "assignedGroupId"));
        };
    };

    sends.do { |send|
        var sendId;
        var runtime;

        sendId = ~valueAtKey.value(send, "sendId");
        runtime = ~sendRuntimeState[sendId];

        if (runtime.notNil) {
            runtime.put(\level, ~clampSendLevel.value(~valueAtKey.value(send, "level") ? 0.0));
            runtime.put(\mode, ~valueAtKey.value(send, "mode") ? "post");
        };
    };

    ~emitClockDomainsState.value;
    ~emitModulesState.value;
    ~emitRoutesState.value;
    ~emitArrangementState.value;
    ~emitAutomationState.value;
    ~emitStructuralState.value;
    ~emitMixerState.value;
    ~emitValidationState.value;
    ~emitRecoveryState.value;
};

~saveProject = { |path = nil, correlationId = nil|
    var projectPath;
    var file;
    var snapshot;

    projectPath = path ? ~projectStateDefaultPath;
    snapshot = ~makeProjectSnapshot.value;

    file = File(projectPath, "w");
    file.write(~jsonEncodeValue.value(snapshot) ++ "\n");
    file.close;

    ~sendEnvelope.value("event", "project.saved", (
        path: projectPath,
        schemaVersion: snapshot[\schemaVersion]
    ), correlationId);
    ~markProjectClean.value("project saved");
};

~loadProject = { |path = nil, correlationId = nil|
    var projectPath;
    var file;
    var text;
    var project;

    projectPath = path ? ~projectStateDefaultPath;

    if (File.exists(projectPath).not) {
        ~sendEnvelope.value("error", "project.error", (
            message: "Project file does not exist",
            path: projectPath
        ), correlationId);
        nil
    } {
        file = File(projectPath, "r");
        text = file.readAllString;
        file.close;
        project = text.parseYAML;

        if (project.isNil) {
            ~sendEnvelope.value("error", "project.error", (
                message: "Could not parse project file",
                path: projectPath
            ), correlationId);
            nil
        } {
            ~applyProjectSnapshot.value(project);
            ~sendEnvelope.value("event", "project.loaded", (
                path: projectPath,
                schemaVersion: ~valueAtKey.value(project, "schemaVersion")
            ), correlationId);
            ~markProjectClean.value("project loaded");
        };
    };
};

~recoverFromAutosaveIfAvailable = {
    var file;
    var text;
    var project;
    var recoveryMeta;

    if (File.exists(~recoveryStateDefaultPath)) {
        file = File(~recoveryStateDefaultPath, "r");
        text = file.readAllString;
        file.close;
        project = text.parseYAML;
        recoveryMeta = if (project.notNil) { ~valueAtKey.value(project, "recoveryMeta") } { nil };

        if (recoveryMeta.notNil and: { ~coerceBoolean.value(~valueAtKey.value(recoveryMeta, "dirty"), false) }) {
            ~applyProjectSnapshot.value(project);
            ~projectDirty = true;
            ~lastRecoverySnapshotAt = ~valueAtKey.value(recoveryMeta, "savedAt") ? "";
            ~lastRecoveryReason = "recovered unsaved autosave";
            ~emitRecoveryState.value;
            ~sendEnvelope.value("event", "engine.recovered", (
                path: ~recoveryStateDefaultPath,
                savedAt: ~lastRecoverySnapshotAt,
                reason: ~valueAtKey.value(recoveryMeta, "reason") ? "unsaved autosave"
            ));
        };
    };
};
)
