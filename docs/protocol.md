# Protocol

Milestones 1 through 30 use a simple newline-delimited JSON envelope over a localhost UDP socket bridge between the JUCE host and `sclang`.

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

Milestone 18 extends `modules.updateCodeSurfaceNextBar`, `modules.codeSwapScheduled`, and `modules.codeSwapApplied` with `surfaceId`.

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

Milestone 25 extends region messages:

- `regions.createLiveLinked`

Milestone 26 adds:

- `performance.requestState`
- `performance.state`
- `performance.triggerMacro`
- `performance.macroTriggered`
- `performance.error`

Milestone 17 adds:

- `automation.requestState`
- `automation.state`
- `automation.addPoint`
- `automation.pointAdded`
- `automation.resetDemo`
- `automation.reset`

Milestone 19 adds:

- `structural.requestState`
- `structural.state`

Milestone 20 does not add new message types. The graph view uses the M13 route messages:

- `routes.requestState`
- `routes.state`
- `routes.create`
- `routes.delete`
- `routes.error`

Milestone 21 adds:

- `clockdomains.setRelation`
- `clockdomains.relationChanged`
- `clockdomains.relationRejected`

Milestone 22 adds:

- `structural.scheduleSceneTransition`
- `structural.sceneTransitionScheduled`
- `structural.sceneTransitionApplied`
- `structural.sceneTransitionRejected`
- `structural.externalCue`
- `structural.externalCueReceived`

Milestone 23 extends mixer messages:

- `mixer.assignGroup`

Milestone 24 extends mixer messages:

- `mixer.setSendLevel`
- `mixer.setSendMode`

Milestone 27 adds:

- `analysis.requestState`
- `analysis.state`

Milestone 28 adds:

- `engine.recoveryRequestState`
- `engine.recoveryState`
- `engine.recovered`

Milestone 29 adds:

- `render.requestState`
- `render.state`
- `render.fullMix`
- `render.stems`
- `render.completed`

Milestone 30 does not add new engine message types. It consolidates host workflow modes over existing engine-owned state.

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

## Milestone 27 Analysis Payload

`analysis.state` reports the engine-owned analysis modules and their latest derived values:

```json
{
  "modules": [
    {
      "moduleId": "module.analysis.kick",
      "sourceModuleId": "module.kick",
      "targetModuleIds": ["module.texture", "module.conductor"],
      "envelope": 0.72,
      "onset": true,
      "density": 0.5,
      "brightness": 0.63,
      "lastBeat": 16.0,
      "routing": "module.kick.audio.out -> module.analysis.kick.control.out -> conductor/texture"
    }
  ]
}
```

JUCE may request this state with `analysis.requestState` and display it in the timing inspector. Feature extraction, downstream influence, and analysis-module lifecycle remain authored by `sclang`.

## Milestone 28 Recovery Payload

`engine.recoveryState` reports engine-owned containment and autosave state:

```json
{
  "engineOnline": true,
  "audioServerReady": true,
  "projectDirty": true,
  "recoveryPath": "/path/to/projects/recovery-autosave.json",
  "lastRecoverySnapshotAt": "2026-04-23 13:10:00",
  "lastReason": "module code surface queued",
  "moduleErrors": [
    {
      "moduleId": "module.kick",
      "surfaceId": "pattern",
      "diagnostic": "eval error: surface rejected by prototype safety guard | preserved previous revision"
    }
  ]
}
```

JUCE may request this state with `engine.recoveryRequestState`. `sclang` writes declarative recovery snapshots after accepted project mutations and may emit `engine.recovered` on startup if it rehydrates unsaved state from that recovery snapshot. Recovery snapshots intentionally persist project declarations rather than raw `scsynth` runtime.

## Milestone 29 Render Payload

`render.fullMix` and `render.stems` accept a render range:

```json
{
  "startBeat": 0.0,
  "lengthBeats": 16.0
}
```

`render.state` and `render.completed` report generated artifacts:

```json
{
  "status": "complete",
  "lastRenderId": "render.1",
  "outputDirectory": "/path/to/sc-engine/renders",
  "artifacts": [
    {
      "stemId": "full-mix",
      "path": "/path/to/render.1-full-mix.json",
      "eventCount": 16,
      "artifactKind": "deterministic-offline-render-package"
    }
  ]
}
```

The first M29 artifact is a deterministic render package, not a waveform file. It is authored by `sclang` from canonical project timing and is intended as the debuggable stepping stone before waveform/NRT rendering.

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
- `regions`
- `automation`
- `structural`
- `mixer`
- `sends`
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

## Milestone 18 Flow

1. JUCE displays engine-authored code surfaces for the selected module
2. JUCE sends `modules.updateCodeSurfaceNextBar` with `moduleId`, `surfaceId`, and `codeSurface`
3. `sclang` queues that surface-specific eval at the next canonical bar
4. On success, `sclang` updates only that surface's active code and revision
5. On failure, `sclang` records diagnostics and preserves the prior working revision for that surface
6. `modules.state` returns per-surface state, diagnostics, revision, pending code, and pending bar

`modules.state` now includes a `codeSurfaces` array per module:

```json
{
  "surfaceId": "pattern",
  "displayName": "Pattern",
  "role": "pattern",
  "code": "(...)",
  "pendingCode": "",
  "state": "active",
  "diagnostic": "using engine active code surface",
  "revision": 1,
  "pendingCodeSwapBarIndex": 0
}
```

The M18 prototype includes `pattern`, `synthDef`, `routine`, and `init` style surfaces. Only the `Kick Pulse` pattern surface currently changes sound parameters.

## Milestone 19 Flow

1. JUCE requests `structural.requestState`
2. `sclang` responds with `structural.state`
3. During playback, the hardcoded `Conductor` structural module emits phrase-boundary directives
4. `sclang` applies those directives to target module runtime state
5. `sclang` emits `structural.state` and refreshed `modules.state`
6. JUCE renders a structural lane, directive markers, and inspector details from returned engine state

## Structural Payload

`structural.state` uses this payload shape:

```json
{
  "directives": [
    {
      "directiveId": "directive.user.1",
      "emitterModuleId": "module.conductor",
      "beat": 0.0,
      "phraseIndex": 0,
      "section": "A sparse",
      "densityTarget": 0.35,
      "syncCue": "phrase-reset",
      "phraseReset": true,
      "orchestration": "open-space",
      "targetModuleIds": ["module.kick", "module.hat", "module.texture"]
    }
  ]
}
```

M19 keeps the conductor hardcoded. It demonstrates structural influence at phrase boundaries without adding a general structural editor, routing graph, or reusable conductor template system.

## Milestone 20 Flow

1. JUCE receives `routes.state`
2. JUCE renders the same canonical route data as both a route list and graph
3. JUCE validates obvious graph gestures locally for immediate feedback, such as direction and family mismatch
4. JUCE sends valid visual connection gestures as `routes.create`
5. JUCE sends visual deletion gestures as `routes.delete`
6. `sclang` performs the authoritative route validation and emits updated `routes.state` or `routes.error`
7. JUCE refreshes both graph and list from the returned route state

M20 extends the demo route endpoints so the graph can show `audio`, `control`, `event`, `structural`, and `sync` families. It does not add a new graph-specific route protocol.

## Milestone 21 Flow

1. JUCE displays the selected clock domain relation in the timing inspector
2. JUCE sends `clockdomains.setRelation` with `domainId`, `relationType`, and `phaseOffsetBeats`
3. `sclang` validates the requested relation
4. `sclang` emits `clockdomains.relationChanged` plus refreshed `clockdomains.state`, or `clockdomains.relationRejected`
5. JUCE refreshes displayed timing from the returned engine state

Allowed M21 relation types are:

- `tempoShared`
- `meterShared`
- `phaseShared`
- `phaseOffset`
- `hardSync`

`clockdomains.setRelation` payload:

```json
{
  "domainId": "domain.triplet",
  "relationType": "phaseOffset",
  "phaseOffsetBeats": 0.25
}
```

Invalid domain IDs, attempts to edit the global clock domain, unsupported relation types, and phase offsets outside `-64..64` beats are rejected by `sclang`.

## Milestone 22 Flow

1. `clockdomains.state` includes phrase length and phase fields authored by `sclang`
2. JUCE sends `structural.scheduleSceneTransition`
3. `sclang` computes the pending boundary from `nextPhrase`, `afterNCycles`, or waits for `externalCue`
4. `sclang` emits `structural.sceneTransitionScheduled` and updated `structural.state`
5. JUCE displays the pending boundary on the global ruler
6. When the canonical transport reaches the boundary, `sclang` applies the scene transition and emits `structural.sceneTransitionApplied`

`structural.scheduleSceneTransition` payload:

```json
{
  "quantizationTarget": "nextPhrase",
  "afterCycles": 0,
  "domainId": "global.main",
  "sceneName": "Next Phrase Scene"
}
```

Allowed M22 quantisation targets are `nextPhrase`, `afterNCycles`, and `externalCue`.

## Milestone 23 Flow

1. JUCE receives `mixer.state` with strips, group entries, and module strip assignments
2. JUCE may send `mixer.setLevel` or `mixer.setMuted` for module, group, or master strips
3. JUCE may send `mixer.assignGroup` for a module strip
4. `sclang` validates the target strip/group, updates the canonical mixer model, and emits `mixer.state`
5. Module sound uses effective gain: module strip level/mute, assigned group level/mute, then master level/mute

`mixer.assignGroup` payload:

```json
{
  "stripId": "strip.module.kick",
  "groupId": "group.drums"
}
```

M23 includes one hardcoded subgroup, `group.drums`. Inserts, sends, returns, and advanced bus topology are intentionally deferred.

## Milestone 24 Flow

1. JUCE receives `mixer.state` with strips, groups, and send entries
2. JUCE may send `mixer.setSendLevel` for an existing send
3. JUCE may send `mixer.setSendMode` with `pre` or `post`
4. `sclang` validates the send and emits updated `mixer.state`
5. Module sound derives aux gain from send level, pre/post mode, return strip level/mute, and master gain

`mixer.setSendLevel` payload:

```json
{
  "sendId": "send.kick.space",
  "level": 0.35
}
```

`mixer.setSendMode` payload:

```json
{
  "sendId": "send.kick.space",
  "mode": "post"
}
```

M24 includes one hardcoded shared FX return, `return.reverb`. Complex aux routing, multiple FX algorithms, and insert chains are deferred.

## Milestone 25 Flow

1. JUCE may request `regions.freezeModule`
2. `sclang` creates a `frozen` region with `editPolicy: detached-audio`
3. JUCE may request `regions.createLiveLinked`
4. `sclang` creates a `live-linked` region with `editPolicy: projection-only`
5. Region edits preserve `regionIdentity` and never mutate module clock-domain semantics
6. JUCE renders frozen and live-linked regions differently from returned `regions.state`

`regions.state` entries now include:

```json
{
  "regionId": "region.user.1",
  "moduleId": "module.kick",
  "regionIdentity": "live-linked",
  "editPolicy": "projection-only",
  "source": "live-link",
  "linkedModuleId": "module.kick"
}
```

Frozen regions are detached material. Live-linked regions are arrangement projections of ongoing module behaviour.

## Milestone 26 Flow

1. JUCE captures a keyboard/button performance input
2. JUCE sends `performance.triggerMacro`
3. `sclang` validates the macro ID and applies the mapped module or structural behaviour
4. `sclang` emits `performance.macroTriggered` and refreshed state where relevant

`performance.triggerMacro` payload:

```json
{
  "macroId": "kick.accent",
  "value": 1.0,
  "inputSource": "juce-host"
}
```

Initial mappings:

- `keyboard:1` -> `kick.accent`
- `keyboard:2` -> `scene.cue`
- `keyboard:3` -> `density.lift`

The host captures input, but the musical response is owned by `sclang`.
