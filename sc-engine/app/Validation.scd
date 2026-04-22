(
~validationAllowedMeterDenominators = [1, 2, 4, 8, 16, 32, 64];

~makeValidationFinding = { |severity, ruleId, message|
    (
        severity: severity,
        ruleId: ruleId,
        message: message
    )
};

~addValidationFinding = { |findings, severity, ruleId, message|
    findings.add(~makeValidationFinding.value(severity, ruleId, message));
};

~validateEngineState = {
    var findings;
    var clockDefinitions;
    var moduleDefinitions;
    var meterNumerator;
    var meterDenominator;
    var tempo;
    var globalClockCount;
    var clockIdCounts;
    var moduleIdCounts;
    var clockDefinitionById;
    var visited;
    var visiting;
    var hasCycle;

    findings = List.new;
    clockDefinitions = ~clockDomainDefinitions ? [];
    moduleDefinitions = ~moduleDefinitions ? [];
    meterNumerator = ~transportState[\meterNumerator] ? 0;
    meterDenominator = ~transportState[\meterDenominator] ? 0;
    tempo = ~transportState[\tempo] ? 0.0;

    globalClockCount = clockDefinitions.count({ |definition| definition[\kind] == "global" });
    if (globalClockCount != 1) {
        ~addValidationFinding.value(findings, "error", "clockdomains.single-global",
            "Expected exactly one global clock domain but found " ++ globalClockCount.asString ++ ".");
    };

    if (tempo <= 0.0) {
        ~addValidationFinding.value(findings, "error", "transport.positive-tempo",
            "Tempo must be positive.");
    };

    if (meterNumerator <= 0) {
        ~addValidationFinding.value(findings, "error", "transport.valid-meter",
            "Meter numerator must be positive.");
    };

    if (~validationAllowedMeterDenominators.includes(meterDenominator).not) {
        ~addValidationFinding.value(findings, "error", "transport.valid-meter",
            "Meter denominator must be one of 1, 2, 4, 8, 16, 32, or 64.");
    };

    clockIdCounts = Dictionary.new;
    clockDefinitions.do { |definition|
        var id = definition[\id];
        if (id.notNil) {
            clockIdCounts[id] = (clockIdCounts[id] ? 0) + 1;
        };
    };

    clockIdCounts.keysValuesDo { |id, count|
        if (count > 1) {
            ~addValidationFinding.value(findings, "error", "clockdomains.unique-ids",
                "Clock domain id '" ++ id.asString ++ "' is duplicated.");
        };
    };

    moduleIdCounts = Dictionary.new;
    moduleDefinitions.do { |definition|
        var id = definition[\id];
        if (id.notNil) {
            moduleIdCounts[id] = (moduleIdCounts[id] ? 0) + 1;
        };
    };

    moduleIdCounts.keysValuesDo { |id, count|
        if (count > 1) {
            ~addValidationFinding.value(findings, "error", "modules.unique-ids",
                "Module id '" ++ id.asString ++ "' is duplicated.");
        };
    };

    clockDefinitionById = Dictionary.new;
    clockDefinitions.do { |definition|
        var id = definition[\id];
        if (id.notNil) {
            clockDefinitionById[id] = definition;
        };
    };

    clockDefinitions.do { |definition|
        var parentId = definition[\parentId];
        if (parentId.notNil and: { clockDefinitionById[parentId].isNil }) {
            ~addValidationFinding.value(findings, "error", "clockdomains.parent-exists",
                "Clock domain '" ++ definition[\id].asString ++ "' references missing parent '" ++ parentId.asString ++ "'.");
        };
    };

    moduleDefinitions.do { |definition|
        var clockDomainId = definition[\clockDomainId];
        if (clockDefinitionById[clockDomainId].isNil) {
            ~addValidationFinding.value(findings, "error", "modules.clock-domain-exists",
                "Module '" ++ definition[\id].asString ++ "' references missing clock domain '" ++ clockDomainId.asString ++ "'.");
        };
    };

    visited = Dictionary.new;
    visiting = Dictionary.new;
    hasCycle = false;

    ~detectClockDomainCycle = { |clockId|
        var definition;
        var parentId;

        if (hasCycle) {
            true
        } {
            if (visiting[clockId] == true) {
                hasCycle = true;
                true
            } {
                if (visited[clockId] == true) {
                    false
                } {
                    definition = clockDefinitionById[clockId];

                    if (definition.isNil) {
                        false
                    } {
                        visiting[clockId] = true;
                        parentId = definition[\parentId];

                        if (parentId.notNil and: { clockDefinitionById[parentId].notNil }) {
                            ~detectClockDomainCycle.value(parentId);
                        };

                        visiting.removeAt(clockId);
                        visited[clockId] = true;
                        hasCycle;
                    };
                };
            };
        };
    };

    clockDefinitions.do { |definition|
        if (hasCycle.not and: { definition[\id].notNil }) {
            ~detectClockDomainCycle.value(definition[\id]);
        };
    };

    if (hasCycle) {
        ~addValidationFinding.value(findings, "error", "clockdomains.no-cycles",
            "Clock domain parent relations must not contain cycles.");
    };

    (
        isValid: findings.isEmpty,
        findingCount: findings.size,
        findings: findings.asArray
    )
};

~emitValidationState = { |correlationId = nil|
    var kind;
    var payload;

    kind = if (correlationId.notNil) { "response" } { "event" };
    payload = ~validateEngineState.value;
    ~engineState[\validation] = payload;
    ~sendEnvelope.value(kind, "validation.state", payload, correlationId);
};
)
