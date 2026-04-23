(
~arrangementRegions = [];
~arrangementRegionNextId = 1;
~arrangementPlaybackPoller = nil;

~makeRegionId = {
    var id;

    id = "region.user." ++ ~arrangementRegionNextId.asString;
    ~arrangementRegionNextId = ~arrangementRegionNextId + 1;
    id
};

~makeArrangementPayload = {
    (
        regions: ~arrangementRegions
    )
};

~findRegion = { |regionId|
    ~arrangementRegions.detect({ |region| region[\regionId] == regionId }, { nil })
};

~snapBeat = { |beat|
    beat.asFloat.round(1.0).max(0.0)
};

~replaceRegion = { |updatedRegion|
    ~arrangementRegions = ~arrangementRegions.collect { |region|
        if (region[\regionId] == updatedRegion[\regionId]) { updatedRegion } { region }
    };
};

~emitArrangementState = { |correlationId = nil|
    var kind;
    var payload;

    kind = if (correlationId.notNil) { "response" } { "event" };
    payload = ~makeArrangementPayload.value;
    ~engineState[\regions] = payload[\regions];
    ~sendEnvelope.value(kind, "regions.state", payload, correlationId);
};

~freezeModuleToRegion = { |moduleId, correlationId = nil|
    var moduleDefinition;
    var runtime;
    var currentBeat;
    var beatsPerBar;
    var startBeat;
    var region;
    var eventOffsets;

    ~ensureModuleRuntimeDefaults.value;
    moduleDefinition = ~definitionForModuleId.value(moduleId);
    runtime = ~moduleRuntimeState[moduleId];

    if (moduleDefinition.isNil or: { runtime.isNil }) {
        ~sendEnvelope.value("error", "regions.error", (
            message: "Unknown module for freeze",
            moduleId: moduleId
        ), correlationId);
        nil
    } {
        if (moduleId != ~kickModuleId) {
            ~sendEnvelope.value("error", "regions.error", (
                message: "Only Kick Pulse can freeze to a replayable region in M15",
                moduleId: moduleId
            ), correlationId);
            nil
        } {
            beatsPerBar = ~transportState[\meterNumerator].asFloat;
            currentBeat = ~captureTransportBeats.value;
            startBeat = (currentBeat / beatsPerBar).floor * beatsPerBar;
            eventOffsets = [0.0, 1.0, 2.0, 3.0];

            region = (
                regionId: ~makeRegionId.value,
                moduleId: moduleId,
                displayName: moduleDefinition[\displayName] ++ " Freeze",
                kind: "frozen-demo-audio",
                source: "freeze",
                startBeat: startBeat,
                lengthBeats: beatsPerBar,
                eventOffsets: eventOffsets,
                replayedEventIndexes: [],
                soundSpec: runtime[\activeSoundSpec] ? ()
            );

            ~arrangementRegions = ~arrangementRegions.add(region);
            ~emitArrangementState.value;
            ~sendEnvelope.value("event", "regions.created", (
                regionId: region[\regionId],
                moduleId: moduleId,
                startBeat: startBeat,
                lengthBeats: beatsPerBar
            ), correlationId);
        };
    };
};

~moveRegion = { |regionId, deltaBeats = 0.0, correlationId = nil|
    var region;
    var updatedRegion;

    region = ~findRegion.value(regionId);

    if (region.isNil) {
        ~sendEnvelope.value("error", "regions.error", (message: "Region not found", regionId: regionId), correlationId);
    } {
        updatedRegion = region.copy;
        updatedRegion.put(\startBeat, ~snapBeat.value((region[\startBeat] ? 0.0) + deltaBeats.asFloat));
        updatedRegion.put(\replayedEventIndexes, []);
        ~replaceRegion.value(updatedRegion);
        ~emitArrangementState.value;
        ~sendEnvelope.value("event", "regions.edited", (regionId: regionId, edit: "move"), correlationId);
    };
};

~trimRegion = { |regionId, deltaBeats = 0.0, correlationId = nil|
    var region;
    var updatedRegion;
    var newLength;

    region = ~findRegion.value(regionId);

    if (region.isNil) {
        ~sendEnvelope.value("error", "regions.error", (message: "Region not found", regionId: regionId), correlationId);
    } {
        newLength = ((region[\lengthBeats] ? 1.0) + deltaBeats.asFloat).round(1.0).max(1.0);
        updatedRegion = region.copy;
        updatedRegion.put(\lengthBeats, newLength);
        updatedRegion.put(\eventOffsets, (region[\eventOffsets] ? []).select({ |offset| offset < newLength }));
        updatedRegion.put(\replayedEventIndexes, []);
        ~replaceRegion.value(updatedRegion);
        ~emitArrangementState.value;
        ~sendEnvelope.value("event", "regions.edited", (regionId: regionId, edit: "trim"), correlationId);
    };
};

~splitRegion = { |regionId, correlationId = nil|
    var region;
    var splitOffset;
    var leftRegion;
    var rightRegion;

    region = ~findRegion.value(regionId);

    if (region.isNil) {
        ~sendEnvelope.value("error", "regions.error", (message: "Region not found", regionId: regionId), correlationId);
    } {
        splitOffset = ((region[\lengthBeats] ? 0.0) / 2.0).round(1.0).max(1.0);

        if (splitOffset >= (region[\lengthBeats] ? 0.0)) {
            ~sendEnvelope.value("error", "regions.error", (message: "Region is too short to split", regionId: regionId), correlationId);
        } {
            leftRegion = region.copy;
            rightRegion = region.copy;

            leftRegion.put(\regionId, ~makeRegionId.value);
            leftRegion.put(\displayName, region[\displayName] ++ " A");
            leftRegion.put(\lengthBeats, splitOffset);
            leftRegion.put(\eventOffsets, (region[\eventOffsets] ? []).select({ |offset| offset < splitOffset }));
            leftRegion.put(\replayedEventIndexes, []);

            rightRegion.put(\regionId, ~makeRegionId.value);
            rightRegion.put(\displayName, region[\displayName] ++ " B");
            rightRegion.put(\startBeat, (region[\startBeat] ? 0.0) + splitOffset);
            rightRegion.put(\lengthBeats, (region[\lengthBeats] ? 0.0) - splitOffset);
            rightRegion.put(\eventOffsets, (region[\eventOffsets] ? []).select({ |offset| offset >= splitOffset }).collect({ |offset| offset - splitOffset }));
            rightRegion.put(\replayedEventIndexes, []);

            ~arrangementRegions = ~arrangementRegions.reject({ |candidate| candidate[\regionId] == regionId });
            ~arrangementRegions = ~arrangementRegions.add(leftRegion).add(rightRegion);
            ~emitArrangementState.value;
            ~sendEnvelope.value("event", "regions.edited", (regionId: regionId, edit: "split", leftRegionId: leftRegion[\regionId], rightRegionId: rightRegion[\regionId]), correlationId);
        };
    };
};

~deleteRegion = { |regionId, correlationId = nil|
    var beforeCount;

    beforeCount = ~arrangementRegions.size;
    ~arrangementRegions = ~arrangementRegions.reject({ |region| region[\regionId] == regionId });

    if (~arrangementRegions.size == beforeCount) {
        ~sendEnvelope.value("error", "regions.error", (message: "Region not found", regionId: regionId), correlationId);
    } {
        ~emitArrangementState.value;
        ~sendEnvelope.value("event", "regions.deleted", (regionId: regionId), correlationId);
    };
};

~playRegionEvent = { |region|
    var mixerGain;
    var soundSpec;

    if (region[\moduleId] == ~kickModuleId and: { ~audioServerReady }) {
        mixerGain = ~effectiveModuleMixerGain.value(~kickModuleId);
        soundSpec = region[\soundSpec] ? ();

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

~processArrangementPlayback = {
    var currentBeat;

    currentBeat = ~captureTransportBeats.value;

    ~arrangementRegions = ~arrangementRegions.collect { |region|
        var playedIndexes;
        var updatedRegion;

        playedIndexes = region[\replayedEventIndexes] ? [];
        updatedRegion = region.copy;

        (region[\eventOffsets] ? []).do { |offset, index|
            var eventBeat;

            eventBeat = region[\startBeat] + offset;

            if ((currentBeat >= eventBeat) and: { currentBeat < (eventBeat + 0.25) } and: { playedIndexes.includes(index).not }) {
                ~playRegionEvent.value(region);
                playedIndexes = playedIndexes.add(index);
            };
        };

        if (currentBeat < region[\startBeat]) {
            playedIndexes = [];
        };

        updatedRegion.put(\replayedEventIndexes, playedIndexes);
        updatedRegion;
    };
};

~startArrangementPlaybackLoop = {
    if (~arrangementPlaybackPoller.notNil) {
        ~arrangementPlaybackPoller.stop;
        ~arrangementPlaybackPoller = nil;
    };

    ~arrangementPlaybackPoller = Routine({
        loop {
            if (~transportState[\isPlaying]) {
                ~processArrangementPlayback.value;
            };

            0.01.wait;
        }
    }).play(SystemClock);
};
)
