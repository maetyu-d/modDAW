(
~routeDefinitions = [
    (
        routeId: "route.audio.kick.to.kick-strip",
        family: "audio",
        source: "module.kick.audio.out",
        destination: "strip.module.kick.audio.in",
        enabled: true
    ),
    (
        routeId: "route.audio.hat.to.hat-strip",
        family: "audio",
        source: "module.hat.audio.out",
        destination: "strip.module.hat.audio.in",
        enabled: true
    ),
    (
        routeId: "route.audio.texture.to.texture-strip",
        family: "audio",
        source: "module.texture.audio.out",
        destination: "strip.module.texture.audio.in",
        enabled: true
    ),
    (
        routeId: "route.control.texture.to.kick",
        family: "control",
        source: "module.texture.control.out",
        destination: "module.kick.control.in",
        enabled: false
    )
];

~routeState = [];
~routeNextId = 1;

~routeEndpoints = [
    (id: "module.kick.audio.out", displayName: "Kick Pulse Audio Out", family: "audio", direction: "output", ownerId: "module.kick"),
    (id: "module.hat.audio.out", displayName: "Hat Grid Audio Out", family: "audio", direction: "output", ownerId: "module.hat"),
    (id: "module.texture.audio.out", displayName: "Texture Drift Audio Out", family: "audio", direction: "output", ownerId: "module.texture"),
    (id: "strip.module.kick.audio.in", displayName: "Kick Strip Audio In", family: "audio", direction: "input", ownerId: "strip.module.kick"),
    (id: "strip.module.hat.audio.in", displayName: "Hat Strip Audio In", family: "audio", direction: "input", ownerId: "strip.module.hat"),
    (id: "strip.module.texture.audio.in", displayName: "Texture Strip Audio In", family: "audio", direction: "input", ownerId: "strip.module.texture"),
    (id: "master.main.audio.in", displayName: "Master Audio In", family: "audio", direction: "input", ownerId: "master.main"),
    (id: "module.texture.control.out", displayName: "Texture Control Out", family: "control", direction: "output", ownerId: "module.texture"),
    (id: "module.kick.control.in", displayName: "Kick Control In", family: "control", direction: "input", ownerId: "module.kick"),
    (id: "module.hat.control.in", displayName: "Hat Control In", family: "control", direction: "input", ownerId: "module.hat")
];

~routeEndpointById = { |endpointId|
    ~routeEndpoints.detect({ |endpoint| endpoint[\id] == endpointId }, { nil })
};

~routeById = { |routeId|
    ~routeDefinitions.detect({ |route| route[\routeId] == routeId }, { nil })
};

~makeRouteId = {
    var routeId;
    routeId = "route.user." ++ ~routeNextId.asString;
    ~routeNextId = ~routeNextId + 1;
    routeId;
};

~validateRouteSpec = { |family, source, destination|
    var findings;
    var sourceEndpoint;
    var destinationEndpoint;

    findings = List.new;
    sourceEndpoint = ~routeEndpointById.value(source);
    destinationEndpoint = ~routeEndpointById.value(destination);

    if (sourceEndpoint.isNil) {
        findings.add((severity: "error", ruleId: "routes.source-exists", message: "Route source endpoint does not exist."));
    } {
        if (sourceEndpoint[\direction] != "output") {
            findings.add((severity: "error", ruleId: "routes.source-output", message: "Route source must be an output endpoint."));
        };

        if (sourceEndpoint[\family] != family) {
            findings.add((severity: "error", ruleId: "routes.source-family", message: "Route source family does not match route family."));
        };
    };

    if (destinationEndpoint.isNil) {
        findings.add((severity: "error", ruleId: "routes.destination-exists", message: "Route destination endpoint does not exist."));
    } {
        if (destinationEndpoint[\direction] != "input") {
            findings.add((severity: "error", ruleId: "routes.destination-input", message: "Route destination must be an input endpoint."));
        };

        if (destinationEndpoint[\family] != family) {
            findings.add((severity: "error", ruleId: "routes.destination-family", message: "Route destination family does not match route family."));
        };
    };

    findings.asArray
};

~makeRoutesPayload = {
    (
        routes: ~routeDefinitions,
        endpoints: ~routeEndpoints
    )
};

~emitRoutesState = { |correlationId = nil|
    var kind;
    var payload;

    kind = if (correlationId.notNil) { "response" } { "event" };
    payload = ~makeRoutesPayload.value;
    ~routeState = payload[\routes];
    ~engineState[\routes] = ~routeState;
    ~sendEnvelope.value(kind, "routes.state", payload, correlationId);
};

~createRoute = { |family, source, destination, enabled = true|
    var findings;
    var route;
    var enabledBool;

    findings = ~validateRouteSpec.value(family, source, destination);
    enabledBool = ~coerceBoolean.value(enabled, true);

    if (findings.notEmpty) {
        ~sendEnvelope.value("error", "routes.error", (
            message: "Route validation failed",
            findings: findings,
            route: (family: family, source: source, destination: destination)
        ));
        nil
    } {
        if (enabledBool) {
            ~routeDefinitions = ~routeDefinitions.collect { |existingRoute|
                if ((existingRoute[\family] == family) and: { existingRoute[\source] == source }) {
                    existingRoute.copy.put(\enabled, false)
                } {
                    existingRoute
                }
            };
        };

        route = (
            routeId: ~makeRouteId.value,
            family: family,
            source: source,
            destination: destination,
            enabled: enabledBool
        );

        ~routeDefinitions = ~routeDefinitions.add(route);
        ~emitRoutesState.value;
    };
};

~deleteRoute = { |routeId|
    var beforeCount;

    beforeCount = ~routeDefinitions.size;
    ~routeDefinitions = ~routeDefinitions.reject({ |route| route[\routeId] == routeId });

    if (~routeDefinitions.size == beforeCount) {
        ~sendEnvelope.value("error", "routes.error", (message: "Route not found", routeId: routeId));
    } {
        ~emitRoutesState.value;
    };
};

~enabledAudioRouteForModule = { |moduleId|
    var sourceId;
    sourceId = moduleId ++ ".audio.out";
    ~routeDefinitions.detect({ |route|
        route[\enabled] == true
        and: { route[\family] == "audio" }
        and: { route[\source] == sourceId }
    }, { nil })
};

~mixerStripIdForAudioDestination = { |destination|
    if (destination == "master.main.audio.in") {
        "master.main"
    } {
        destination.replace(".audio.in", "")
    }
};
)
