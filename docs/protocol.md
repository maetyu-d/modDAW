# Protocol

Milestones 1 through 17 use a simple newline-delimited JSON envelope over a localhost UDP socket bridge between the JUCE host and `sclang`.

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

Milestone 13 adds:

- `routes.requestState`
- `routes.state`
- `routes.create`
- `routes.delete`
- `routes.error`

Milestone 14 adds:

- `project.save`
- `project.load`
- `project.saved`
- `project.loaded`
- `project.error`

Milestone 15 adds:

- `regions.requestState`
- `regions.state`
- `regions.freezeModule`
- `regions.created`
- `regions.error`

Milestone 16 adds:

- `regions.move`
- `regions.trim`
- `regions.split`
- `regions.delete`
- `regions.edited`
- `regions.deleted`

Milestone 17 adds:

- `automation.requestState`
- `automation.state`
- `automation.addPoint`
- `automation.pointAdded`
- `automation.resetDemo`
- `automation.reset`

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

## Milestone 13 Flow

1. JUCE reaches engine-ready state
2. JUCE sends `routes.requestState`
3. `sclang` responds with `routes.state`
4. JUCE may send `routes.create` or `routes.delete`
5. `sclang` validates route endpoints and route family compatibility
6. `sclang` emits `routes.state` after accepted route changes, or `routes.error` after rejected changes
7. the minimal mixer/audio path consults the engine-owned audio route model before applying strip gain

## Route Payload

`routes.state` uses this payload shape:

```json
{
  "routes": [
    {
      "routeId": "route.audio.kick.to.kick-strip",
      "family": "audio",
      "source": "module.kick.audio.out",
      "destination": "strip.module.kick.audio.in",
      "enabled": true
    }
  ],
  "endpoints": [
    {
      "id": "module.kick.audio.out",
      "displayName": "Kick Pulse Audio Out",
      "family": "audio",
      "direction": "output",
      "ownerId": "module.kick"
    }
  ]
}
```

`routes.create` payload:

```json
{
  "family": "audio",
  "source": "module.kick.audio.out",
  "destination": "strip.module.hat.audio.in",
  "enabled": true
}
```

`routes.delete` payload:

```json
{
  "routeId": "route.user.1"
}
```

For this prototype slice, creating an enabled route for a source disables the previous enabled route for that same source. This gives a simple, debuggable reroute operation without adding fan-out semantics yet.

## Milestone 14 Flow

1. JUCE sends `project.save`
2. `sclang` builds a declarative project snapshot from canonical engine state
3. `sclang` writes the snapshot to the default demo project file
4. `sclang` emits `project.saved`
5. JUCE may later send `project.load`
6. `sclang` parses the saved snapshot, reconstructs runtime state, and emits updated engine states
7. `sclang` emits `project.loaded`, or `project.error` if the file is missing or invalid

## Project Snapshot Payload

M14 saves the default demo project at `sc-engine/projects/demo-project.json`.

The snapshot is declarative project state only. It includes:

- `schemaVersion`
- `modules`
- `clockDomains`
- `timingAttachments`
- `routes`
- `mixer`
- `codeSurfaces`

It intentionally does not persist raw `scsynth` nodes, server runtime state, transport position, or host-derived UI timing projections.

## Milestone 15 Flow

1. JUCE renders a `Freeze` action on each module lane
2. JUCE sends `regions.freezeModule` for the selected module action
3. `sclang` creates a canonical frozen region anchored to the global beat ruler
4. `sclang` emits `regions.state` and `regions.created`
5. JUCE renders the returned region block in the module lane
6. During playback, `sclang` replays the frozen region material through `scsynth`

## Region Payload

`regions.state` uses this payload shape:

```json
{
  "regions": [
    {
      "regionId": "region.user.1",
      "moduleId": "module.kick",
      "displayName": "Kick Pulse Freeze",
      "kind": "frozen-demo-audio",
      "source": "freeze",
      "startBeat": 0.0,
      "lengthBeats": 4.0,
      "eventOffsets": [0.0, 1.0, 2.0, 3.0]
    }
  ]
}
```

M15 freezes only the `Kick Pulse` demo module into a replayable one-bar region. The region is declarative arrangement material owned by `sclang`; JUCE only renders it.

## Milestone 16 Flow

1. JUCE selects a frozen region block in a module lane
2. JUCE sends an explicit edit command such as `regions.move`, `regions.trim`, `regions.split`, or `regions.delete`
3. `sclang` applies the edit with global-beat snapping
4. `sclang` emits `regions.state`
5. `sclang` emits `regions.edited` or `regions.deleted`
6. JUCE re-renders the returned arrangement state

M16 edits frozen/projected region material only. These edits do not silently rewrite module clock-domain relationships or underlying module semantics.

## Milestone 17 Flow

1. JUCE requests `automation.requestState`
2. `sclang` responds with `automation.state`
3. JUCE may request `automation.addPoint` or `automation.resetDemo`
4. `sclang` snaps point insertion to the canonical current beat
5. During playback, `sclang` interpolates breakpoint values and applies them to the target mixer parameter
6. JUCE displays returned automation lane state without owning parameter motion

## Automation Payload

`automation.state` uses this payload shape:

```json
{
  "lanes": [
    {
      "laneId": "automation.kick.level",
      "displayName": "Kick Level",
      "targetType": "mixer",
      "targetId": "strip.module.kick",
      "parameterId": "level",
      "interpolation": "linear",
      "currentValue": 1.0,
      "points": [
        { "beat": 0.0, "value": 1.0 },
        { "beat": 4.0, "value": 0.35 },
        { "beat": 8.0, "value": 1.0 }
      ]
    }
  ]
}
```

M17 supports one automatable parameter: `strip.module.kick.level`. Point insertion uses the engine's current canonical beat.
