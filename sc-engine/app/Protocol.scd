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

    type = ~envelopeValueAt.value(envelope, "type");
    messageId = ~envelopeValueAt.value(envelope, "messageId");
    payload = ~envelopeValueAt.value(envelope, "payload");

    if (type == "engine.handshake") {
        ~sendEnvelope.value("event", "engine.ready", (state: "ready"), messageId);
    } {
        if (type == "engine.ping") {
            ~sendEnvelope.value("response", "engine.pong", (state: "alive"), messageId);
        } {
            if (type == "transport.requestState") {
                ~emitTransportState.value(messageId);
            } {
                if (type == "clockdomains.requestState") {
                    ~emitClockDomainsState.value(messageId);
                } {
                    if (type == "modules.requestState") {
                        ~emitModulesState.value(messageId);
                    } {
                        if (type == "mixer.requestState") {
                            ~emitMixerState.value(messageId);
                        } {
                            if (type == "mixer.setLevel") {
                                ~setMixerStripLevel.value(
                                    ~envelopeValueAt.value(payload, "stripId"),
                                    ~envelopeValueAt.value(payload, "level")
                                );
                            } {
                                if (type == "mixer.setMuted") {
                                    ~setMixerStripMuted.value(
                                        ~envelopeValueAt.value(payload, "stripId"),
                                        ~envelopeValueAt.value(payload, "muted")
                                    );
                                } {
                        if (type == "validation.requestState") {
                            ~emitValidationState.value(messageId);
                        } {
                        if (type == "modules.activateNextBar") {
                            ~scheduleModuleActivateNextBar.value(~envelopeValueAt.value(payload, "moduleId"));
                        } {
                            if (type == "modules.updateCodeSurfaceNextBar") {
                                ~scheduleModuleCodeSurfaceUpdateNextBar.value(
                                    ~envelopeValueAt.value(payload, "moduleId"),
                                    ~envelopeValueAt.value(payload, "codeSurface")
                                );
                            } {
                                if (type == "transport.play") {
                                    ~transportPlay.value;
                                } {
                                    if (type == "transport.stop") {
                                        ~transportStop.value;
                                    } {
                                        ~sendEnvelope.value("error", "engine.error", (message: "Unknown message type"), messageId);
                                    };
                                };
                            };
                        };
                                };
                            };
                        };
                        };
                    };
                };
            };
        };
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
