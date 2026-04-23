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
        routeId: "route.audio.drums.to.master",
        family: "audio",
        source: "group.drums.audio.out",
        destination: "master.main.audio.in",
        enabled: true
    ),
    (
        routeId: "route.audio.kick.to-space-return",
        family: "audio",
        source: "module.kick.audio.out",
        destination: "return.reverb.audio.in",
        enabled: true
    ),
    (
        routeId: "route.audio.space-return.to-master",
        family: "audio",
        source: "return.reverb.audio.out",
        destination: "master.main.audio.in",
        enabled: true
    ),
    (
        routeId: "route.control.texture.to.kick",
        family: "control",
        source: "module.texture.control.out",
        destination: "module.kick.control.in",
        enabled: false
    ),
    (
        routeId: "route.control.analysis.to.texture",
        family: "control",
        source: "module.analysis.kick.control.out",
        destination: "module.texture.control.in",
        enabled: true
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
    (id: "group.drums.audio.in", displayName: "Drum Group Audio In", family: "audio", direction: "input", ownerId: "group.drums"),
    (id: "group.drums.audio.out", displayName: "Drum Group Audio Out", family: "audio", direction: "output", ownerId: "group.drums"),
    (id: "return.reverb.audio.in", displayName: "Space Return Audio In", family: "audio", direction: "input", ownerId: "return.reverb"),
    (id: "return.reverb.audio.out", displayName: "Space Return Audio Out", family: "audio", direction: "output", ownerId: "return.reverb"),
    (id: "master.main.audio.in", displayName: "Master Audio In", family: "audio", direction: "input", ownerId: "master.main"),
    (id: "module.texture.control.out", displayName: "Texture Control Out", family: "control", direction: "output", ownerId: "module.texture"),
    (id: "module.analysis.kick.control.out", displayName: "Kick Listener Analysis Out", family: "control", direction: "output", ownerId: "module.analysis.kick"),
    (id: "module.kick.control.in", displayName: "Kick Control In", family: "control", direction: "input", ownerId: "module.kick"),
    (id: "module.hat.control.in", displayName: "Hat Control In", family: "control", direction: "input", ownerId: "module.hat"),
    (id: "module.texture.control.in", displayName: "Texture Control In", family: "control", direction: "input", ownerId: "module.texture"),
    (id: "module.kick.event.out", displayName: "Kick Event Out", family: "event", direction: "output", ownerId: "module.kick"),
    (id: "module.texture.event.in", displayName: "Texture Event In", family: "event", direction: "input", ownerId: "module.texture"),
    (id: "module.conductor.structural.out", displayName: "Conductor Structural Out", family: "structural", direction: "output", ownerId: "module.conductor"),
    (id: "module.kick.structural.in", displayName: "Kick Structural In", family: "structural", direction: "input", ownerId: "module.kick"),
    (id: "module.hat.structural.in", displayName: "Hat Structural In", family: "structural", direction: "input", ownerId: "module.hat"),
    (id: "module.texture.structural.in", displayName: "Texture Structural In", family: "structural", direction: "input", ownerId: "module.texture"),
    (id: "global.main.sync.out", displayName: "Global Sync Out", family: "sync", direction: "output", ownerId: "global.main"),
    (id: "module.conductor.sync.in", displayName: "Conductor Sync In", family: "sync", direction: "input", ownerId: "module.conductor")
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
        if (~markProjectDirty.notNil) { ~markProjectDirty.value("route created") };
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
        if (~markProjectDirty.notNil) { ~markProjectDirty.value("route deleted") };
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
