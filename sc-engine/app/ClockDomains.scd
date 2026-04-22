(
~clockDomainDefinitions = [
    (
        id: "global.main",
        parentId: nil,
        kind: "global",
        displayName: "Global Main",
        ratioToParent: 1.0,
        phaseOffsetBeats: 0.0
    ),
    (
        id: "domain.half",
        parentId: "global.main",
        kind: "derived",
        displayName: "Half-Time",
        ratioToParent: 0.5,
        phaseOffsetBeats: 0.0
    ),
    (
        id: "domain.triplet",
        parentId: "global.main",
        kind: "derived",
        displayName: "Triplet Drift",
        ratioToParent: 1.5,
        phaseOffsetBeats: 0.25
    )
];

~clockDomains = [];

~makeClockDomainEntry = { |definition|
    var globalBeat;
    var meterNumerator;
    var localBeatPosition;
    var localBeatInBarZeroBased;

    globalBeat = ~captureTransportBeats.value;
    meterNumerator = ~transportState[\meterNumerator];
    localBeatPosition = (globalBeat * definition[\ratioToParent]) + definition[\phaseOffsetBeats];
    localBeatInBarZeroBased = localBeatPosition % meterNumerator;

    (
        id: definition[\id],
        parentId: definition[\parentId],
        kind: definition[\kind],
        displayName: definition[\displayName],
        ratioToParent: definition[\ratioToParent],
        phaseOffsetBeats: definition[\phaseOffsetBeats],
        localBeatPosition: localBeatPosition,
        localBarIndex: (localBeatPosition / meterNumerator).floor.asInteger + 1,
        localBeatInBar: localBeatInBarZeroBased + 1.0
    )
};

~makeClockDomainsPayload = {
    (
        domains: ~clockDomainDefinitions.collect { |definition|
            ~makeClockDomainEntry.value(definition)
        }
    )
};

~emitClockDomainsState = { |correlationId = nil|
    var kind;
    kind = if(correlationId.notNil) { "response" } { "event" };
    ~clockDomains = ~makeClockDomainsPayload.value[\domains];
    ~engineState[\clockDomains] = ~clockDomains;
    ~sendEnvelope.value(kind, "clockdomains.state", (domains: ~clockDomains), correlationId);
};
)
