(
~automationLanes = [
    (
        laneId: "automation.kick.level",
        displayName: "Kick Level",
        targetType: "mixer",
        targetId: "strip.module.kick",
        parameterId: "level",
        interpolation: "linear",
        points: [
            (beat: 0.0, value: 1.0),
            (beat: 4.0, value: 0.35),
            (beat: 8.0, value: 1.0)
        ],
        currentValue: 1.0
    )
];

~automationPlaybackPoller = nil;

~automationLaneById = { |laneId|
    ~automationLanes.detect({ |lane| lane[\laneId] == laneId }, { nil })
};

~sortAutomationPoints = { |points|
    points.asArray.sort({ |a, b| (a[\beat] ? 0.0) < (b[\beat] ? 0.0) })
};

~makeAutomationPayload = {
    (
        lanes: ~automationLanes
    )
};

~emitAutomationState = { |correlationId = nil|
    var kind;
    var payload;

    kind = if (correlationId.notNil) { "response" } { "event" };
    payload = ~makeAutomationPayload.value;
    ~engineState[\automation] = payload[\lanes];
    ~sendEnvelope.value(kind, "automation.state", payload, correlationId);
};

~automationValueAtBeat = { |lane, beat|
    var points;
    var previous;
    var next;
    var result;

    points = ~sortAutomationPoints.value(lane[\points] ? []);

    if (points.isEmpty) {
        lane[\currentValue] ? 1.0
    } {
        previous = points[0];
        next = nil;

        points.do { |point|
            if ((point[\beat] ? 0.0) <= beat) {
                previous = point;
            };

            if (next.isNil and: { (point[\beat] ? 0.0) > beat }) {
                next = point;
            };
        };

        if (next.isNil) {
            result = previous[\value] ? 1.0;
        } {
            var span;
            var phase;

            span = ((next[\beat] ? 0.0) - (previous[\beat] ? 0.0)).max(0.0001);
            phase = ((beat - (previous[\beat] ? 0.0)) / span).clip(0.0, 1.0);
            result = ((previous[\value] ? 1.0) * (1.0 - phase)) + ((next[\value] ? 1.0) * phase);
        };

        result
    }
};

~applyAutomationLaneValue = { |lane, value|
    var runtime;

    if (lane[\targetType] == "mixer" and: { lane[\parameterId] == "level" }) {
        ~ensureMixerRuntimeDefaults.value;
        runtime = ~mixerRuntimeState[lane[\targetId]];

        if (runtime.notNil) {
            runtime.put(\level, ~clampMixerLevel.value(value));
        };
    };
};

~processAutomationPlayback = {
    var currentBeat;
    var changed;

    currentBeat = ~captureTransportBeats.value;
    changed = false;

    ~automationLanes = ~automationLanes.collect { |lane|
        var value;
        var previousValue;
        var updatedLane;

        value = ~automationValueAtBeat.value(lane, currentBeat);
        previousValue = lane[\currentValue] ? value;
        updatedLane = lane.copy;
        updatedLane.put(\currentValue, value);
        ~applyAutomationLaneValue.value(updatedLane, value);

        if ((value - previousValue).abs > 0.01) {
            changed = true;
        };

        updatedLane;
    };

    if (changed) {
        ~emitMixerState.value;
    };
};

~startAutomationPlaybackLoop = {
    if (~automationPlaybackPoller.notNil) {
        ~automationPlaybackPoller.stop;
        ~automationPlaybackPoller = nil;
    };

    ~automationPlaybackPoller = Routine({
        loop {
            if (~transportState[\isPlaying]) {
                ~processAutomationPlayback.value;
            };

            0.05.wait;
        }
    }).play(SystemClock);
};

~addAutomationPoint = { |laneId, value, correlationId = nil|
    var lane;
    var point;
    var beat;

    lane = ~automationLaneById.value(laneId);

    if (lane.isNil) {
        ~sendEnvelope.value("error", "automation.error", (message: "Automation lane not found", laneId: laneId), correlationId);
    } {
        beat = ~snapBeat.value(~captureTransportBeats.value);
        point = (beat: beat, value: value.asFloat.clip(0.0, 1.5));
        lane.put(\points, ~sortAutomationPoints.value((lane[\points] ? []).reject({ |existing| (existing[\beat] ? 0.0) == beat }).add(point)));
        ~emitAutomationState.value;
        if (~markProjectDirty.notNil) { ~markProjectDirty.value("automation point added") };
        ~sendEnvelope.value("event", "automation.pointAdded", (laneId: laneId, beat: beat, value: point[\value]), correlationId);
    };
};

~resetAutomationDemo = { |laneId, correlationId = nil|
    var lane;

    lane = ~automationLaneById.value(laneId);

    if (lane.isNil) {
        ~sendEnvelope.value("error", "automation.error", (message: "Automation lane not found", laneId: laneId), correlationId);
    } {
        lane.put(\points, [
            (beat: 0.0, value: 1.0),
            (beat: 4.0, value: 0.35),
            (beat: 8.0, value: 1.0)
        ]);
        lane.put(\currentValue, 1.0);
        ~emitAutomationState.value;
        if (~markProjectDirty.notNil) { ~markProjectDirty.value("automation reset") };
        ~sendEnvelope.value("event", "automation.reset", (laneId: laneId), correlationId);
    };
};
)
