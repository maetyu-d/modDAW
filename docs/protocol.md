# Protocol

Milestones 1 through 12 use a simple newline-delimited JSON envelope over a localhost UDP socket bridge between the JUCE host and `sclang`.

- One JSON object per line
- UTF-8 text
- Newline-delimited JSON payload strings carried over local raw UDP messages
- Explicit versioned envelope

## Envelope

Every message uses this shape:

```json
{
  "protocolVersion": "1.0",
  "messageId": "host-0001",
  "correlationId": "optional-related-message-id",
  "timestamp": "2026-04-22T12:00:00Z",
  "sender": "juce-host",
  "recipient": "sc-engine",
  "kind": "command",
  "type": "engine.handshake",
  "payload": {
    "hostName": "modular-sc-daw"
  }
}
```

## Envelope Fields

- `protocolVersion`: protocol version string
- `messageId`: unique message identifier
- `correlationId`: optional reference to the related initiating message
- `timestamp`: ISO-8601 UTC timestamp string
- `sender`: logical sender identifier
- `recipient`: logical recipient identifier
- `kind`: message kind
- `type`: explicit message type
- `payload`: structured JSON payload object

## Kinds

Initial supported kinds:

- `command`
- `ack`
- `event`
- `error`
- `query`
- `response`

## Initial Message Types

Milestone 1 defines these message types:

- `engine.handshake`
- `engine.ready`
- `engine.ping`
- `engine.pong`

Milestone 2 adds:

- `transport.requestState`
- `transport.state`
- `transport.play`
- `transport.stop`

Milestone 4 adds:

- `clockdomains.requestState`
- `clockdomains.state`

Milestone 5 adds:

- `modules.requestState`
- `modules.state`

Milestone 8 adds:

- `modules.activateNextBar`
- `modules.activationScheduled`

Milestone 9 adds:

- `modules.updateCodeSurfaceNextBar`
- `modules.codeSwapScheduled`
- `modules.codeSwapApplied`

Milestone 11 adds:

- `validation.requestState`
- `validation.state`

Milestone 12 adds:

- `mixer.requestState`
- `mixer.state`
- `mixer.setLevel`
- `mixer.setMuted`

## Milestone 1 Flow

1. JUCE launches `sclang`
2. JUCE sends `engine.handshake`
3. `sclang` responds with `engine.ready`
4. JUCE periodically sends `engine.ping`
5. `sclang` responds with `engine.pong`

Disconnect is detected by process exit or loss of engine socket traffic.

## Milestone 2 Flow

1. JUCE reaches engine-ready state
2. JUCE sends `transport.requestState`
3. `sclang` responds with `transport.state`
4. JUCE may send `transport.play` or `transport.stop`
5. `sclang` updates the canonical transport and emits `transport.state`

## Transport Payload

`transport.state` uses this payload shape:

```json
{
  "isPlaying": true,
  "tempo": 120.0,
  "meter": [4, 4],
  "beatPosition": 9.5,
  "barIndex": 3,
  "beatInBar": 1.5,
  "phaseWithinBar": 0.375
}
```

The payload is authored by `sclang`. JUCE displays it but does not compute authority from local UI state.

## Milestone 4 Flow

1. JUCE reaches engine-ready state
2. JUCE sends `clockdomains.requestState`
3. `sclang` responds with `clockdomains.state`
4. `sclang` may emit updated `clockdomains.state` events as transport changes

## Clock Domain Payload

`clockdomains.state` uses this payload shape:

```json
{
  "domains": [
    {
      "id": "global.main",
      "parentId": null,
      "kind": "global",
      "displayName": "Global Main",
      "ratioToParent": 1.0,
      "phaseOffsetBeats": 0.0,
      "localBeatPosition": 8.5,
      "localBarIndex": 3,
      "localBeatInBar": 0.5
    }
  ]
}
```

The domain graph is authored by `sclang`. JUCE only displays the current engine view of that graph.

## Milestone 5 Flow

1. JUCE reaches engine-ready state
2. JUCE sends `modules.requestState`
3. `sclang` responds with `modules.state`
4. `sclang` may emit updated `modules.state` events as module-owned engine state changes

## Module Payload

`modules.state` uses this payload shape:

```json
{
  "modules": [
    {
      "id": "module.kick",
      "displayName": "Kick Pulse",
      "clockDomainId": "global.main",
      "lifecycleState": "idle",
      "behaviourType": "demo-pattern",
      "codeSurface": "/* engine-owned placeholder */"
    }
  ]
}
```

The module graph is authored by `sclang`. JUCE only displays the engine-owned module view.

## Milestone 7 Behaviour

Milestone 7 does not add a new host message type. It extends engine-owned behaviour behind the existing transport and module state messages:

1. `sclang` boots `scsynth`
2. `sclang` installs one demo synth definition
3. `transport.play` starts the `module.kick` sound behaviour
4. `transport.stop` stops the `module.kick` sound behaviour
5. `modules.state` reflects lifecycle changes such as `booting-audio`, `ready`, `waiting-audio`, and `playing`

## Milestone 8 Flow

1. JUCE selects a module lane
2. JUCE sends `modules.activateNextBar`
3. `sclang` computes the next bar boundary from the canonical transport
4. `sclang` responds with `modules.activationScheduled`
5. `sclang` updates `modules.state` to show a queued lifecycle
6. `sclang` activates the module exactly when that bar boundary is reached

## Milestone 9 Flow

1. JUCE edits the selected module's code surface locally
2. JUCE sends `modules.updateCodeSurfaceNextBar`
3. `sclang` computes the next bar boundary from the canonical transport
4. `sclang` emits `modules.codeSwapScheduled`
5. `sclang` evaluates and applies the pending code surface exactly on that next bar boundary
6. `sclang` emits `modules.codeSwapApplied`
7. `modules.state` reflects queued, active, or eval-error status for the engine-owned code surface

## Milestone 11 Flow

1. JUCE reaches engine-ready state
2. JUCE sends `validation.requestState`
3. `sclang` evaluates the current prototype validation rules
4. `sclang` responds with `validation.state`
5. JUCE displays the engine-owned validation findings without becoming the validation authority

## Milestone 12 Flow

1. JUCE reaches engine-ready state
2. JUCE sends `mixer.requestState`
3. `sclang` responds with `mixer.state`
4. JUCE may send `mixer.setLevel` or `mixer.setMuted` for one strip
5. `sclang` updates the canonical mixer state and emits `mixer.state`
6. the `Kick Pulse` module uses that engine-owned mixer state when triggering sound through `scsynth`
