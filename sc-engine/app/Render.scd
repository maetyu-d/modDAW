(
~renderState = (
    status: "idle",
    lastRenderId: "",
    outputDirectory: ~engineRoot +/+ "renders",
    artifacts: []
);

~renderNextId = 1;

~makeRenderId = {
    var id;
    id = "render." ++ ~renderNextId.asString;
    ~renderNextId = ~renderNextId + 1;
    id
};

~renderRangeFromPayload = { |payload|
    var startBeat;
    var lengthBeats;

    startBeat = (~valueAtKey.value(payload, "startBeat") ? 0.0).asFloat.max(0.0);
    lengthBeats = (~valueAtKey.value(payload, "lengthBeats") ? 16.0).asFloat.max(1.0);
    (
        startBeat: startBeat,
        endBeat: startBeat + lengthBeats,
        lengthBeats: lengthBeats
    )
};

~moduleSoundSpecForRender = { |moduleId|
    var runtime;

    runtime = ~moduleRuntimeState[moduleId];
    if (runtime.notNil) {
        runtime[\activeSoundSpec] ? ()
    } {
        ()
    }
};

~renderEventsForModule = { |moduleId, range|
    var events;
    var beatsPerBar;
    var soundSpec;
    var firstBeat;
    var lastBeat;

    events = List.new;
    beatsPerBar = ~transportState[\meterNumerator].asFloat.max(1.0);
    soundSpec = ~moduleSoundSpecForRender.value(moduleId);

    if (moduleId == ~kickModuleId) {
        firstBeat = range[\startBeat].ceil.asInteger;
        lastBeat = (range[\endBeat] - 0.0001).floor.asInteger;
        if (lastBeat >= firstBeat) {
            firstBeat.for(lastBeat, { |beat|
                events.add((
                    beat: beat.asFloat,
                    seconds: beat.asFloat * (60.0 / ~transportState[\tempo].asFloat.max(1.0)),
                    moduleId: moduleId,
                    eventType: "kick-pulse",
                    soundSpec: soundSpec,
                    source: "live-module"
                ));
            });
        };
    };

    ~arrangementRegions.do { |region|
        var regionStart;
        var regionEnd;

        if ((region[\moduleId] == moduleId) and: { (region[\regionIdentity] ? "frozen") == "frozen" }) {
            regionStart = region[\startBeat] ? 0.0;
            regionEnd = regionStart + (region[\lengthBeats] ? beatsPerBar);

            if (regionEnd > range[\startBeat] and: { regionStart < range[\endBeat] }) {
                (region[\eventOffsets] ? []).do { |offset|
                    var eventBeat;
                    eventBeat = regionStart + offset.asFloat;
                    if (eventBeat >= range[\startBeat] and: { eventBeat < range[\endBeat] }) {
                        events.add((
                            beat: eventBeat,
                            seconds: eventBeat * (60.0 / ~transportState[\tempo].asFloat.max(1.0)),
                            moduleId: moduleId,
                            eventType: "frozen-region-event",
                            regionId: region[\regionId],
                            soundSpec: region[\soundSpec] ? soundSpec,
                            source: "frozen-region"
                        ));
                    };
                };
            };
        };
    };

    events.asArray.sort({ |a, b| (a[\beat] ? 0.0) < (b[\beat] ? 0.0) })
};

~makeRenderArtifact = { |renderId, stemId, range, events|
    var artifactPath;
    var stripId;
    var stripRuntime;
    var payload;
    var file;

    stripId = if (stemId == "full-mix") { "master.main" } { "strip." ++ stemId };
    stripRuntime = ~mixerRuntimeState[stripId];
    artifactPath = ~engineRoot +/+ "renders" +/+ (renderId ++ "-" ++ stemId.replace(".", "-") ++ ".json");

    payload = (
        schemaVersion: "moddaw.render.1",
        renderId: renderId,
        stemId: stemId,
        artifactKind: "deterministic-offline-render-package",
        note: "Prototype render package authored by sclang from canonical project timing; not a waveform file yet.",
        tempo: ~transportState[\tempo],
        meter: [~transportState[\meterNumerator], ~transportState[\meterDenominator]],
        startBeat: range[\startBeat],
        endBeat: range[\endBeat],
        lengthBeats: range[\lengthBeats],
        level: if (stripRuntime.notNil) { stripRuntime[\level] ? 1.0 } { 1.0 },
        muted: if (stripRuntime.notNil) { stripRuntime[\muted] ? false } { false },
        events: events
    );

    file = File(artifactPath, "w");
    file.write(~jsonEncodeValue.value(payload) ++ "\n");
    file.close;

    (
        stemId: stemId,
        path: artifactPath,
        eventCount: events.size,
        artifactKind: payload[\artifactKind]
    )
};

~makeRenderPayload = {
    ~renderState
};

~emitRenderState = { |correlationId = nil|
    var kind;

    kind = if (correlationId.notNil) { "response" } { "event" };
    ~engineState[\render] = ~renderState;
    ~sendEnvelope.value(kind, "render.state", ~makeRenderPayload.value, correlationId);
};

~renderFullMix = { |payload = nil, correlationId = nil|
    var renderId;
    var range;
    var events;
    var artifact;

    ~ensureModuleRuntimeDefaults.value;
    ~ensureMixerRuntimeDefaults.value;
    renderId = ~makeRenderId.value;
    range = ~renderRangeFromPayload.value(payload ? ());
    events = ~moduleDefinitions.collect { |definition|
        ~renderEventsForModule.value(definition[\id], range)
    }.flat.sort({ |a, b| (a[\beat] ? 0.0) < (b[\beat] ? 0.0) });
    artifact = ~makeRenderArtifact.value(renderId, "full-mix", range, events);

    ~renderState = (
        status: "complete",
        lastRenderId: renderId,
        outputDirectory: ~engineRoot +/+ "renders",
        artifacts: [artifact]
    );
    ~emitRenderState.value;
    ~sendEnvelope.value("event", "render.completed", ~renderState, correlationId);
};

~renderStems = { |payload = nil, correlationId = nil|
    var renderId;
    var range;
    var artifacts;

    ~ensureModuleRuntimeDefaults.value;
    ~ensureMixerRuntimeDefaults.value;
    renderId = ~makeRenderId.value;
    range = ~renderRangeFromPayload.value(payload ? ());
    artifacts = ~moduleDefinitions.collect { |definition|
        var events;
        events = ~renderEventsForModule.value(definition[\id], range);
        ~makeRenderArtifact.value(renderId, definition[\id], range, events)
    };

    ~renderState = (
        status: "complete",
        lastRenderId: renderId,
        outputDirectory: ~engineRoot +/+ "renders",
        artifacts: artifacts
    );
    ~emitRenderState.value;
    ~sendEnvelope.value("event", "render.completed", ~renderState, correlationId);
};
)
