(
~clockDomainDefinitions = [
    (
        id: "global.main",
        parentId: nil,
        kind: "global",
        displayName: "Global Main",
        relationType: "global",
        ratioToParent: 1.0,
        phaseOffsetBeats: 0.0,
        phraseLengthBars: 2
    ),
    (
        id: "domain.half",
        parentId: "global.main",
        kind: "derived",
        displayName: "Half-Time",
        relationType: "tempoShared",
        ratioToParent: 0.5,
        phaseOffsetBeats: 0.0,
        phraseLengthBars: 4
    ),
    (
        id: "domain.triplet",
        parentId: "global.main",
        kind: "derived",
        displayName: "Triplet Drift",
        relationType: "phaseOffset",
        ratioToParent: 1.5,
        phaseOffsetBeats: 0.25,
        phraseLengthBars: 3
    )
];

~clockDomains = [];
~clockDomainRelationTypes = ["tempoShared", "meterShared", "phaseShared", "phaseOffset", "hardSync"];

~clockDomainDefinitionById = { |domainId|
    ~clockDomainDefinitions.detect({ |definition| definition[\id] == domainId }, { nil })
};

~relationDescriptionFor = { |definition|
    var relationType;

    relationType = definition[\relationType] ? "tempoShared";

    case
    { relationType == "global" } { "canonical global domain" }
    { relationType == "tempoShared" } { "shares parent tempo with ratio " ++ definition[\ratioToParent].asString }
    { relationType == "meterShared" } { "shares parent tempo and meter, phase may drift by offset" }
    { relationType == "phaseShared" } { "shares parent tempo, meter, and phase" }
    { relationType == "phaseOffset" } { "shares parent timing with explicit phase offset" }
    { relationType == "hardSync" } { "hard syncs local phase to parent bar boundaries" }
    { "unknown relation" }
};

~makeClockDomainEntry = { |definition|
    var globalBeat;
    var meterNumerator;
    var localBeatPosition;
    var localBeatInBarZeroBased;
    var relationType;
    var ratio;
    var phaseOffset;
    var phraseLengthBars;
    var phraseLengthBeats;
    var phraseIndex;
    var phrasePhase;
    var nextPhraseBeat;

    globalBeat = ~captureTransportBeats.value;
    meterNumerator = ~transportState[\meterNumerator];
    relationType = definition[\relationType] ? "tempoShared";
    ratio = definition[\ratioToParent] ? 1.0;
    phaseOffset = definition[\phaseOffsetBeats] ? 0.0;
    phraseLengthBars = (definition[\phraseLengthBars] ? 2).asInteger.max(1);
    phraseLengthBeats = meterNumerator.asFloat * phraseLengthBars.asFloat;

    localBeatPosition = case
    { relationType == "global" } { globalBeat }
    { relationType == "tempoShared" } { (globalBeat * ratio) + phaseOffset }
    { relationType == "meterShared" } { globalBeat + phaseOffset }
    { relationType == "phaseShared" } { globalBeat }
    { relationType == "phaseOffset" } { globalBeat + phaseOffset }
    { relationType == "hardSync" } {
        var barStart;
        barStart = (globalBeat / meterNumerator).floor * meterNumerator;
        barStart + (globalBeat - barStart)
    }
    { (globalBeat * ratio) + phaseOffset };
    localBeatInBarZeroBased = localBeatPosition % meterNumerator;
    phraseIndex = (localBeatPosition / phraseLengthBeats).floor.asInteger;
    phrasePhase = (localBeatPosition % phraseLengthBeats) / phraseLengthBeats;
    nextPhraseBeat = (phraseIndex + 1).asFloat * phraseLengthBeats;

    (
        id: definition[\id],
        parentId: definition[\parentId],
        kind: definition[\kind],
        displayName: definition[\displayName],
        relationType: relationType,
        relationDescription: ~relationDescriptionFor.value(definition),
        ratioToParent: ratio,
        phaseOffsetBeats: phaseOffset,
        phraseLengthBars: phraseLengthBars,
        phraseLengthBeats: phraseLengthBeats,
        phraseIndex: phraseIndex,
        phrasePhase: phrasePhase,
        nextPhraseBeat: nextPhraseBeat,
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

~setClockDomainRelation = { |domainId, relationType, phaseOffsetBeats = 0.0, correlationId = nil|
    var definition;
    var findings;
    var phaseOffset;

    definition = ~clockDomainDefinitionById.value(domainId);
    findings = List.new;
    relationType = relationType.asString;
    phaseOffset = (phaseOffsetBeats ? 0.0).asFloat;

    if (definition.isNil) {
        findings.add((severity: "error", ruleId: "clockdomains.relation.domain-exists", message: "Clock domain does not exist."));
    } {
        if (definition[\kind] == "global") {
            findings.add((severity: "error", ruleId: "clockdomains.relation.global-immutable", message: "The global clock domain relation cannot be edited."));
        };
    };

    if (~clockDomainRelationTypes.detect({ |allowed| allowed.asString == relationType }, { nil }).isNil) {
        findings.add((severity: "error", ruleId: "clockdomains.relation.type", message: "Unsupported clock-domain relation type."));
    };

    if (phaseOffset.abs > 64.0) {
        findings.add((severity: "error", ruleId: "clockdomains.relation.phase-range", message: "Phase offset must be between -64 and 64 beats."));
    };

    if (findings.notEmpty) {
        ~sendEnvelope.value("error", "clockdomains.relationRejected", (
            domainId: domainId,
            relationType: relationType,
            findings: findings.asArray
        ), correlationId);
        nil
    } {
        ~clockDomainDefinitions = ~clockDomainDefinitions.collect { |item|
            if (item[\id] == domainId) {
                var updated;
                updated = item.copy;
                updated.put(\relationType, relationType);

                case
                { relationType == "tempoShared" } {
                    updated.put(\ratioToParent, item[\ratioToParent] ? 1.0);
                    updated.put(\phaseOffsetBeats, phaseOffset);
                }
                { relationType == "meterShared" } {
                    updated.put(\ratioToParent, 1.0);
                    updated.put(\phaseOffsetBeats, phaseOffset);
                }
                { relationType == "phaseShared" } {
                    updated.put(\ratioToParent, 1.0);
                    updated.put(\phaseOffsetBeats, 0.0);
                }
                { relationType == "phaseOffset" } {
                    updated.put(\ratioToParent, 1.0);
                    updated.put(\phaseOffsetBeats, phaseOffset);
                }
                { relationType == "hardSync" } {
                    updated.put(\ratioToParent, 1.0);
                    updated.put(\phaseOffsetBeats, 0.0);
                };

                updated
            } {
                item
            }
        };

        ~emitClockDomainsState.value;
        ~emitValidationState.value;
        ~sendEnvelope.value("event", "clockdomains.relationChanged", (
            domainId: domainId,
            relationType: relationType,
            phaseOffsetBeats: phaseOffset
        ), correlationId);
    };
};
)
