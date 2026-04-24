# Architecture

## Migration State

The repository is in a staged migration from:

- JUCE shell
- `sclang` musical engine
- `scsynth` audio engine

to:

- JUCE-native application shell
- JUCE-native transport and timing
- JUCE-native module execution
- JUCE-native audio engine
- JUCE-native routing, mixer, scheduler, and persistence

`J1` removes the runtime requirement for the external SuperCollider engine while keeping the existing UI and state model alive through an internal JUCE compatibility layer.

`J2` moves global transport ownership into a JUCE-native `TransportEngine`.

`J3` moves clock-domain ownership into a JUCE-native `ClockDomainManager`.

`J4` moves module identity, timing attachment, lifecycle state, and demo behaviour ownership into a JUCE-native `ModuleRegistry`.

`J5` moves audible output into a JUCE-native `AudioEngine`.

`J6` moves routing and mixer authority into JUCE-native `RoutingGraph` and `MixerEngine` layers.

`J7` moves quantised action ownership into a JUCE-native `Scheduler`.

`J8` replaces passive code-surface text with JUCE-native editable behaviour parsing and application.

## Runtime Boundaries After J1

### JUCE application

The JUCE application now owns the running desktop process.

For `J1` through `J5`, it is responsible for:

- application startup
- UI rendering
- internal engine status
- JUCE-native global transport playback state
- JUCE-native clock-domain computation
- JUCE-native module registry and lifecycle state
- JUCE-native audio callback and sound generation
- JUCE-native routing and mixer state
- in-memory demo state for the remaining existing views
- user interaction and request routing

The current internal engine is intentionally a compatibility layer, not the finished native engine architecture yet.

### Legacy SuperCollider runtime

The repository still contains:

- `sc-engine/`
- `third_party/supercollider/`

These are legacy implementation artifacts during migration. They are no longer required to launch the app in `J1`.

## Authority Model During Migration

Before the migration, musical authority lived in `sclang`.

From `J1` onward, the migration direction is:

- JUCE becomes the sole runtime authority
- external `sclang` / `scsynth` processes are retired
- the conceptual model is preserved while engine ownership moves into C++

`J1` does not complete that authority transfer yet. It removes the runtime process dependency.

`J2` begins the real authority transfer by moving transport playback, bar/beat progression, and ruler timing into JUCE while the rest of the system still runs through compatibility-layer state.

`J3` continues that transfer by moving global, local, derived, and free-running clock-domain computation into JUCE. Domain relations and overlay timing now come from native code instead of placeholder host state.

`J4` continues it again by moving module identity and runtime ownership into C++ objects. The UI now receives module state from a native registry rather than from hand-built placeholder records.

`J5` continues it again by moving the first audible engine into JUCE. Sound is now rendered by a native audio callback instead of `scsynth`.

`J6` continues it again by moving route validation and mixer gain authority into JUCE. Audio routes, strip gain, group assignment, mute, and master level now affect the native sound path.

`J7` continues it again by moving boundary resolution into JUCE. Module activation and surface-apply requests now resolve at native beat, bar, and phrase boundaries.

`J8` continues it again by moving behaviour editing into typed C++ parsing. Invalid surface edits are rejected while the previous valid behaviour stays active.

## Target JUCE-Native Engine Shape

The intended native engine split is:

- `AudioEngine`
- `TransportEngine`
- `ClockDomain`
- `ClockDomainManager`
- `Module`
- `ModuleRegistry`
- `ModuleProcessor`
- `RoutingGraph`
- `MixerEngine`
- `Scheduler`
- `AutomationEngine`
- `ProjectState`

UI components are expected to remain mostly in place and be reconnected from the compatibility layer to these native engine objects as the migration progresses.

## What J1 Preserves

`J1` keeps the current prototype legible by preserving:

- the existing JUCE UI shell
- typed host-side state objects
- the current module, mixer, route, automation, region, analysis, structural, recovery, render, and validation views
- the existing request surface used by the UI

## What J1 Removes

`J1` removes the runtime requirement for:

- child-process launch of `sclang`
- socket communication with a separate SC engine
- SC runtime availability just to open the app

## Deferred To Later J Milestones

- JUCE-native persistence format
- retirement of legacy SC runtime files and old protocol docs
