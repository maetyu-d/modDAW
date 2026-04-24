# Internal Engine Contract

The app no longer uses a runtime IPC protocol.

This document now describes the internal JUCE-native state and request contract that the UI uses when talking to engine subsystems through `EngineProcessManager`.

## Request Surface

The UI currently issues explicit requests for:

- transport play / stop / refresh
- clock-domain relation updates
- module activation on next bar
- behaviour surface apply on next bar
- mixer strip level / mute / group changes
- send level / send mode changes
- route create / delete
- project save / load
- region edits
- automation point creation / reset
- structural scene transitions and cues
- render requests

These are regular in-process C++ method calls, not serialized network messages.

## State Surface

The UI consumes typed snapshots of:

- `TransportState`
- `ClockDomainState`
- `ModuleState`
- `MixerState`
- `RouteState`
- `RegionState`
- `AutomationState`
- `AnalysisState`
- `StructuralState`
- `RecoveryState`
- `RenderState`
- `ValidationState`

Each state family has a revision counter so the UI can refresh only when a specific engine state changes.

## Persistence Format

Project save/load uses a JUCE-native JSON snapshot at [projects/demo-project.json](/Users/user/Documents/ModDaw/modular-sc-daw/projects/demo-project.json).

The snapshot persists declarative state:

- transport
- clock domains
- modules and behaviour surfaces
- mixer strips, groups, and sends
- route endpoints and routes
- regions
- automation lanes
- structural directives and pending transitions

The snapshot does not treat transient audio callback state as canonical.

## Scheduler Contract

Quantised actions are resolved by `Scheduler` with these policies:

- `immediate`
- `nextBeat`
- `nextBar`
- `nextPhrase`

The current UI primarily exposes `nextBar`, but the native engine contract already supports the wider set.

## Behaviour Surface Contract

Editable behaviour surfaces are interpreted in JUCE-native code.

Current supported shapes:

- `steps: [1, 0, 1, 0]`
- scalar blocks such as:
  - `frequency: 220`
  - `accent: 0.85`
  - `density: 0.25`
  - `spread: 1.5`

Invalid edits are rejected safely and the previous valid behaviour remains active.
