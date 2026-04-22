(
~transportNowSeconds = {
    Date.getDate.rawSeconds;
};

~transportDefaultTempo = 120.0;
~transportDefaultMeterNumerator = 4;
~transportDefaultMeterDenominator = 4;

~transportState = (
    isPlaying: false,
    tempo: ~transportDefaultTempo,
    meterNumerator: ~transportDefaultMeterNumerator,
    meterDenominator: ~transportDefaultMeterDenominator,
    accumulatedBeats: 0.0,
    startedAtSeconds: nil
);

~captureTransportBeats = {
    var beats;
    var elapsedSeconds;

    beats = ~transportState[\accumulatedBeats];

    if (~transportState[\isPlaying]) {
        elapsedSeconds = ~transportNowSeconds.value - ~transportState[\startedAtSeconds];
        beats = beats + (elapsedSeconds * (~transportState[\tempo] / 60.0));
    };

    beats;
};

~makeTransportPayload = {
    var beatPosition;
    var meterNumerator;
    var beatInBarZeroBased;
    var beatInBar;
    var barIndex;
    var phaseWithinBar;

    beatPosition = ~captureTransportBeats.value;
    meterNumerator = ~transportState[\meterNumerator];
    beatInBarZeroBased = beatPosition % meterNumerator;
    beatInBar = beatInBarZeroBased + 1.0;
    barIndex = (beatPosition / meterNumerator).floor.asInteger + 1;
    phaseWithinBar = beatInBarZeroBased / meterNumerator;

    (
        isPlaying: ~transportState[\isPlaying],
        tempo: ~transportState[\tempo],
        meterNumerator: ~transportState[\meterNumerator],
        meterDenominator: ~transportState[\meterDenominator],
        beatPosition: beatPosition,
        barIndex: barIndex,
        beatInBar: beatInBar,
        phaseWithinBar: phaseWithinBar
    )
};

~emitTransportState = { |correlationId = nil|
    var kind;
    kind = if(correlationId.notNil) { "response" } { "event" };
    ~sendEnvelope.value(kind, "transport.state", ~makeTransportPayload.value, correlationId);
};

~transportPlay = {
    if (~transportState[\isPlaying].not) {
        ~transportState.put(\startedAtSeconds, ~transportNowSeconds.value);
        ~transportState.put(\isPlaying, true);
    };

    ~startKickModule.value;
    ~emitTransportState.value;
    ~emitClockDomainsState.value;
};

~transportStop = {
    if (~transportState[\isPlaying]) {
        ~transportState.put(\accumulatedBeats, ~captureTransportBeats.value);
        ~transportState.put(\startedAtSeconds, nil);
        ~transportState.put(\isPlaying, false);
    };

    ~stopKickModule.value;
    ~emitTransportState.value;
    ~emitClockDomainsState.value;
};

~startTransportBroadcastLoop = {
    Routine({
        loop {
            if (~transportState[\isPlaying]) {
                ~emitTransportState.value;
                ~emitClockDomainsState.value;
            };

            0.25.wait;
        }
    }).play(SystemClock);
};
)
