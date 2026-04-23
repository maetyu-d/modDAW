(
~protocolVersion = "1.0";
~engineSenderId = "sc-engine";
~hostRecipientId = "juce-host";
~protocolHostIp = "127.0.0.1";
~protocolHostPort = 57130;
~protocolEnginePort = 57131;
~protocolReplyAddr = nil;
~protocolRawRecvFunc = nil;

~configureProtocolPortsFromEnv = {
    var hostPortArg;
    var enginePortArg;

    hostPortArg = "MODDAW_HOST_PORT".getenv;
    enginePortArg = "MODDAW_ENGINE_PORT".getenv;

    if (hostPortArg.notNil) {
        ~protocolHostPort = hostPortArg.asInteger;
    };

    if (enginePortArg.notNil) {
        ~protocolEnginePort = enginePortArg.asInteger;
    };
};

~makeTimestamp = {
    Date.getDate.rawSeconds.asString
};

~makeMessageId = {
    "engine-" ++ UniqueID.next.asString
};

~jsonEscapeString = { |value|
    var escaped;

    escaped = value.asString;
    escaped = escaped.replace("\\", "\\\\");
    escaped = escaped.replace("\"", "\\\"");
    escaped = escaped.replace(Char.nl.asString, "\\n");
    escaped = escaped.replace("\r", "\\r");
    escaped = escaped.replace("\t", "\\t");

    "\"" ++ escaped ++ "\""
};

~jsonEncodeValue = { |value|
    var result;

    result = if (value.isNil) {
        "null"
    } {
        if (value.isKindOf(String) or: { value.isKindOf(Symbol) }) {
            ~jsonEscapeString.value(value.asString)
        } {
            if (value.isKindOf(Char)) {
                ~jsonEscapeString.value(value.asString)
            } {
                if (value.isKindOf(Integer) or: { value.isKindOf(Float) }) {
                    value.asString
                } {
                    if (value.isKindOf(True) or: { value.isKindOf(False) }) {
                        if(value.booleanValue) { "true" } { "false" }
                    } {
                        if (value.isKindOf(SequenceableCollection) and: { value.isKindOf(String).not }) {
                            "[" ++ value.collect({ |item| ~jsonEncodeValue.value(item) }).join(",") ++ "]"
                        } {
                            if (value.respondsTo(\keysValuesDo)) {
                                var pairs = List.new;

                                value.keysValuesDo { |key, item|
                                    pairs.add(~jsonEscapeString.value(key.asString) ++ ":" ++ ~jsonEncodeValue.value(item));
                                };

                                "{" ++ pairs.join(",") ++ "}"
                            } {
                                ~jsonEscapeString.value(value.asString)
                            };
                        };
                    };
                };
            };
        };
    };

    result
};

~envelopeValueAt = { |dictionary, key|
    var value;

    if (dictionary.isNil) {
        nil
    } {
        value = dictionary[key];

        if (value.isNil) {
            value = dictionary[key.asSymbol];
        };

        if (value.isNil) {
            value = dictionary[key.asString];
        };

        value
    }
};

~coerceBoolean = { |value, fallback = false|
    var text;

    if (value.isNil) {
        fallback
    } {
        if (value.isKindOf(True) or: { value.isKindOf(False) }) {
            value.booleanValue
        } {
            text = value.asString.toLower;
            (text == "true") or: { text == "1" };
        };
    };
};

~sendEnvelope = { |kind, type, payload = nil, correlationId = nil|
    var envelope;
    var jsonLine;

    if (payload.isNil) {
        payload = ();
    };

    envelope = (
        protocolVersion: ~protocolVersion,
        messageId: ~makeMessageId.value,
        timestamp: ~makeTimestamp.value,
        sender: ~engineSenderId,
        recipient: ~hostRecipientId,
        kind: kind,
        type: type,
        payload: payload
    );

    if (correlationId.notNil) {
        envelope.put(\correlationId, correlationId);
    };

    if (~protocolReplyAddr.isNil) {
        ~protocolReplyAddr = NetAddr(~protocolHostIp, ~protocolHostPort);
    };

    jsonLine = ~jsonEncodeValue.value(envelope) ++ "\n";
    ~protocolReplyAddr.sendRaw(jsonLine);
};

~handleEnvelope = { |envelope|
    var type;
    var messageId;
    var payload;
    var enabled;

    type = ~envelopeValueAt.value(envelope, "type");
    messageId = ~envelopeValueAt.value(envelope, "messageId");
    payload = ~envelopeValueAt.value(envelope, "payload");

    case
    { type == "engine.handshake" } {
        ~sendEnvelope.value("event", "engine.ready", (state: "ready"), messageId);
    }
    { type == "engine.ping" } {
        ~sendEnvelope.value("response", "engine.pong", (state: "alive"), messageId);
    }
    { type == "engine.recoveryRequestState" } {
        ~emitRecoveryState.value(messageId);
    }
    { type == "render.requestState" } {
        ~emitRenderState.value(messageId);
    }
    { type == "render.fullMix" } {
        ~renderFullMix.value(payload, messageId);
    }
    { type == "render.stems" } {
        ~renderStems.value(payload, messageId);
    }
    { type == "transport.requestState" } {
        ~emitTransportState.value(messageId);
    }
    { type == "transport.play" } {
        ~transportPlay.value;
    }
    { type == "transport.stop" } {
        ~transportStop.value;
    }
    { type == "clockdomains.requestState" } {
        ~emitClockDomainsState.value(messageId);
    }
    { type == "clockdomains.setRelation" } {
        ~setClockDomainRelation.value(
            ~envelopeValueAt.value(payload, "domainId"),
            ~envelopeValueAt.value(payload, "relationType"),
            ~envelopeValueAt.value(payload, "phaseOffsetBeats") ? 0.0,
            messageId
        );
    }
    { type == "modules.requestState" } {
        ~emitModulesState.value(messageId);
    }
    { type == "modules.activateNextBar" } {
        ~scheduleModuleActivateNextBar.value(~envelopeValueAt.value(payload, "moduleId"));
    }
    { type == "modules.updateCodeSurfaceNextBar" } {
        ~scheduleModuleCodeSurfaceUpdateNextBar.value(
            ~envelopeValueAt.value(payload, "moduleId"),
            ~envelopeValueAt.value(payload, "codeSurface"),
            ~envelopeValueAt.value(payload, "surfaceId") ? "pattern"
        );
    }
    { type == "mixer.requestState" } {
        ~emitMixerState.value(messageId);
    }
    { type == "mixer.setLevel" } {
        ~setMixerStripLevel.value(
            ~envelopeValueAt.value(payload, "stripId"),
            ~envelopeValueAt.value(payload, "level")
        );
    }
    { type == "mixer.setMuted" } {
        ~setMixerStripMuted.value(
            ~envelopeValueAt.value(payload, "stripId"),
            ~envelopeValueAt.value(payload, "muted")
        );
    }
    { type == "mixer.assignGroup" } {
        ~setMixerStripGroup.value(
            ~envelopeValueAt.value(payload, "stripId"),
            ~envelopeValueAt.value(payload, "groupId")
        );
    }
    { type == "mixer.setSendLevel" } {
        ~setMixerSendLevel.value(
            ~envelopeValueAt.value(payload, "sendId"),
            ~envelopeValueAt.value(payload, "level")
        );
    }
    { type == "mixer.setSendMode" } {
        ~setMixerSendMode.value(
            ~envelopeValueAt.value(payload, "sendId"),
            ~envelopeValueAt.value(payload, "mode")
        );
    }
    { type == "routes.requestState" } {
        ~emitRoutesState.value(messageId);
    }
    { type == "routes.create" } {
        enabled = ~envelopeValueAt.value(payload, "enabled");
        if (enabled.isNil) { enabled = true };

        ~createRoute.value(
            ~envelopeValueAt.value(payload, "family"),
            ~envelopeValueAt.value(payload, "source"),
            ~envelopeValueAt.value(payload, "destination"),
            enabled
        );
    }
    { type == "routes.delete" } {
        ~deleteRoute.value(~envelopeValueAt.value(payload, "routeId"));
    }
    { type == "regions.requestState" } {
        ~emitArrangementState.value(messageId);
    }
    { type == "regions.freezeModule" } {
        ~freezeModuleToRegion.value(~envelopeValueAt.value(payload, "moduleId"), messageId);
    }
    { type == "regions.createLiveLinked" } {
        ~createLiveLinkedRegion.value(~envelopeValueAt.value(payload, "moduleId"), messageId);
    }
    { type == "regions.move" } {
        ~moveRegion.value(
            ~envelopeValueAt.value(payload, "regionId"),
            ~envelopeValueAt.value(payload, "deltaBeats") ? 0.0,
            messageId
        );
    }
    { type == "regions.trim" } {
        ~trimRegion.value(
            ~envelopeValueAt.value(payload, "regionId"),
            ~envelopeValueAt.value(payload, "deltaBeats") ? 0.0,
            messageId
        );
    }
    { type == "regions.split" } {
        ~splitRegion.value(~envelopeValueAt.value(payload, "regionId"), messageId);
    }
    { type == "regions.delete" } {
        ~deleteRegion.value(~envelopeValueAt.value(payload, "regionId"), messageId);
    }
    { type == "automation.requestState" } {
        ~emitAutomationState.value(messageId);
    }
    { type == "automation.addPoint" } {
        ~addAutomationPoint.value(
            ~envelopeValueAt.value(payload, "laneId"),
            ~envelopeValueAt.value(payload, "value") ? 1.0,
            messageId
        );
    }
    { type == "automation.resetDemo" } {
        ~resetAutomationDemo.value(~envelopeValueAt.value(payload, "laneId"), messageId);
    }
    { type == "structural.requestState" } {
        ~emitStructuralState.value(messageId);
    }
    { type == "structural.scheduleSceneTransition" } {
        ~scheduleSceneTransition.value(
            ~envelopeValueAt.value(payload, "quantizationTarget") ? "nextPhrase",
            ~envelopeValueAt.value(payload, "afterCycles") ? 0,
            ~envelopeValueAt.value(payload, "domainId") ? "global.main",
            ~envelopeValueAt.value(payload, "sceneName") ? "Scene Change",
            messageId
        );
    }
    { type == "structural.externalCue" } {
        ~triggerExternalCue.value(~envelopeValueAt.value(payload, "cueName") ? "manual", messageId);
    }
    { type == "performance.requestState" } {
        ~emitPerformanceState.value(messageId);
    }
    { type == "performance.triggerMacro" } {
        ~triggerPerformanceMacro.value(
            ~envelopeValueAt.value(payload, "macroId"),
            ~envelopeValueAt.value(payload, "value") ? 1.0,
            messageId
        );
    }
    { type == "analysis.requestState" } {
        ~emitAnalysisState.value(messageId);
    }
    { type == "project.save" } {
        ~saveProject.value(~envelopeValueAt.value(payload, "path"), messageId);
    }
    { type == "project.load" } {
        ~loadProject.value(~envelopeValueAt.value(payload, "path"), messageId);
    }
    { type == "validation.requestState" } {
        ~emitValidationState.value(messageId);
    }
    { true } {
        ~sendEnvelope.value("error", "engine.error", (
            message: "Unknown message type",
            type: type
        ), messageId);
    };
};

~startSocketLoop = {
    var opened;

    ~configureProtocolPortsFromEnv.value;
    ~protocolReplyAddr = NetAddr(~protocolHostIp, ~protocolHostPort);

    opened = thisProcess.openUDPPort(~protocolEnginePort, \raw);

    if (opened.not) {
        ("Failed to open engine UDP port " ++ ~protocolEnginePort).postln;
        nil;
    };

    if (~protocolRawRecvFunc.notNil) {
        thisProcess.removeRawRecvFunc(~protocolRawRecvFunc);
    };

    ~protocolRawRecvFunc = { |msg, time, replyAddr, recvPort|
        var line;
        var parsed;

        ~protocolReplyAddr = NetAddr(replyAddr.ip, ~protocolHostPort);

        line = String.fill(msg.size, { |i| msg[i].asAscii });

        if (line.size > 0) {
            parsed = line.parseYAML;

            if (parsed.notNil) {
                ~handleEnvelope.value(parsed);
            };
        };
    };

    thisProcess.addRawRecvFunc(~protocolRawRecvFunc);
    ("Protocol socket listening on udp://127.0.0.1:" ++ ~protocolEnginePort).postln;
};
)
