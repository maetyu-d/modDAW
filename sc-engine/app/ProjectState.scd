(
~projectStateDefaultPath = ~engineRoot +/+ "projects" +/+ "demo-project.json";

~arrayOrEmpty = { |value|
    if (value.isNil) { [] } { value }
};

~makeProjectSnapshot = {
    var modules;
    var mixer;
    var codeSurfaces;

    ~ensureModuleRuntimeDefaults.value;
    ~ensureMixerRuntimeDefaults.value;

    modules = ~moduleDefinitions.collect { |definition|
        var runtime;

        runtime = ~moduleRuntimeState[definition[\id]];
        (
            id: definition[\id],
            displayName: definition[\displayName],
            clockDomainId: definition[\clockDomainId],
            lifecycleState: "idle",
            behaviourType: definition[\behaviourType],
            codeSurface: runtime[\activeCodeSurface] ? definition[\codeSurface]
        )
    };

    mixer = ~mixerDefinitions.collect { |definition|
        var runtime;

        runtime = ~mixerRuntimeState[definition[\id]];
        (
            stripId: definition[\id],
            level: runtime[\level] ? 1.0,
            muted: runtime[\muted] ? false
        )
    };

    codeSurfaces = modules.collect { |module|
        (
            moduleId: module[\id],
            surfaceId: "main",
            code: module[\codeSurface]
        )
    };

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
        mixer: mixer,
        codeSurfaces: codeSurfaces
    )
};

~applyProjectSnapshot = { |project|
    var modules;
    var clockDomains;
    var routes;
    var mixer;
    var codeSurfaces;
    var regions;
    var automation;

    modules = ~arrayOrEmpty.value(~valueAtKey.value(project, "modules"));
    clockDomains = ~arrayOrEmpty.value(~valueAtKey.value(project, "clockDomains"));
    routes = ~arrayOrEmpty.value(~valueAtKey.value(project, "routes"));
    mixer = ~arrayOrEmpty.value(~valueAtKey.value(project, "mixer"));
    codeSurfaces = ~arrayOrEmpty.value(~valueAtKey.value(project, "codeSurfaces"));
    regions = ~arrayOrEmpty.value(~valueAtKey.value(project, "regions"));
    automation = ~arrayOrEmpty.value(~valueAtKey.value(project, "automation"));

    if (clockDomains.notEmpty) {
        ~clockDomainDefinitions = clockDomains.collect { |domain|
            (
                id: ~valueAtKey.value(domain, "id"),
                parentId: ~valueAtKey.value(domain, "parentId"),
                kind: ~valueAtKey.value(domain, "kind"),
                displayName: ~valueAtKey.value(domain, "displayName"),
                ratioToParent: (~valueAtKey.value(domain, "ratioToParent") ? 1.0).asFloat,
                phaseOffsetBeats: (~valueAtKey.value(domain, "phaseOffsetBeats") ? 0.0).asFloat
            )
        };
    };

    if (modules.notEmpty) {
        ~moduleDefinitions = modules.collect { |module|
            var moduleId;
            var savedCode;
            var codeSurface;

            moduleId = ~valueAtKey.value(module, "id");
            savedCode = codeSurfaces.detect({ |surface|
                (~valueAtKey.value(surface, "moduleId") == moduleId)
                and: { ~valueAtKey.value(surface, "surfaceId") == "main" }
            }, { nil });
            codeSurface = if (savedCode.notNil) {
                ~valueAtKey.value(savedCode, "code")
            } {
                ~valueAtKey.value(module, "codeSurface")
            };

            (
                id: moduleId,
                displayName: ~valueAtKey.value(module, "displayName"),
                clockDomainId: ~valueAtKey.value(module, "clockDomainId"),
                lifecycleState: "idle",
                behaviourType: ~valueAtKey.value(module, "behaviourType"),
                codeSurface: codeSurface,
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
            source: ~valueAtKey.value(region, "source"),
            startBeat: (~valueAtKey.value(region, "startBeat") ? 0.0).asFloat,
            lengthBeats: (~valueAtKey.value(region, "lengthBeats") ? 4.0).asFloat,
            eventOffsets: ~arrayOrEmpty.value(~valueAtKey.value(region, "eventOffsets")),
            replayedEventIndexes: [],
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

    ~moduleRuntimeState = Dictionary.new;
    ~ensureModuleRuntimeDefaults.value;

    ~moduleDefinitions.do { |definition|
        var runtime;
        var result;

        runtime = ~moduleRuntimeState[definition[\id]];
        runtime.put(\activeCodeSurface, definition[\codeSurface]);
        runtime.put(\lifecycleState, if(definition[\id] == ~kickModuleId and: { ~audioServerReady }) { "ready" } { "idle" });

        if (definition[\id] == ~kickModuleId) {
            result = ~evaluateModuleCodeSurface.value(definition[\id], definition[\codeSurface]);

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
        };
    };

    ~emitClockDomainsState.value;
    ~emitModulesState.value;
    ~emitRoutesState.value;
    ~emitArrangementState.value;
    ~emitAutomationState.value;
    ~emitMixerState.value;
    ~emitValidationState.value;
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
        };
    };
};
)
