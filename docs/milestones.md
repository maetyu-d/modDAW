# Milestones

## Legacy Prototype Track

- `M1` host + engine handshake
- `M2` canonical transport in `sclang`
- `M3` global ruler
- `M4` clock domains
- `M5` minimal module model
- `M6` local timing overlay
- `M7` first sound
- `M8` quantised activation
- `M9` code eval
- `M10` timing inspector
- `M11` minimal validation
- `M12` minimal mixer
- `M13` explicit routing
- `M14` persistent project state
- `M15` record/freeze to timeline regions
- `M16` proper arrangement editing
- `M17` automation and control lanes
- `M18` multi-surface code model
- `M19` structural/conductor modules
- `M20` route graph UI
- `M21` richer timing relations
- `M22` phrase-level quantisation and scene boundaries
- `M23` group / bus architecture
- `M24` sends / returns and spatial aux logic
- `M25` better region identity: live-linked vs frozen
- `M26` external control and performance input
- `M27` analysis and feedback modules
- `M28` robust error containment and recovery
- `M29` offline render and stem export
- `M30` UI consolidation and workflow pass

## JUCE-Native Migration Track

- `J1` remove runtime SC process dependency
- `J2` JUCE-native transport engine
- `J3` JUCE-native clock-domain engine
- `J4` JUCE-native module model
- `J5` JUCE-native audio engine
- `J6` JUCE-native routing and mixer
- `J7` JUCE-native scheduler and quantised actions
- `J8` replace SC code surfaces with JUCE-native editable behaviours
- `J9` persistence migration
- `J10` clean docs and retire legacy SC runtime files

## Current Progress

- Legacy prototype milestones `M1` through `M30` exist in prototype form.
- Migration milestones `J1` through `J10` are implemented.
- The app now launches without requiring `sclang` or `scsynth` at runtime.
- The current UI is backed by JUCE-native engine subsystems for transport, clocks, modules, audio, routing, mixer, scheduling, and persistence.
- Global transport playback and ruler timing now come from a JUCE-native `TransportEngine`.
- Clock domains now come from a JUCE-native `ClockDomainManager`.
- Modules now come from a JUCE-native `ModuleRegistry`.
- Audio now comes from a JUCE-native `AudioEngine`.
- Mixer and routes now come from JUCE-native `MixerEngine` and `RoutingGraph` layers.
- Quantised next-bar actions now come from a JUCE-native `Scheduler`.
- Code surfaces now compile into native module behaviour blocks instead of remaining passive text.
- Project save/load now restores JUCE-native runtime state from disk.
- Legacy SC runtime files and docs are retired.

## J1 Scope

`J1` intentionally does only this:

- removes child-process launch of the SuperCollider runtime
- removes the runtime requirement for local SC installation
- replaces external engine status with internal JUCE engine status
- keeps the UI alive by serving placeholder in-memory state through the existing typed state model

## Still Deferred After J1

- transport authority migration
- clock-domain execution migration
- native module execution
- native audio generation
- native routing and mixer execution
- native scheduler authority
- native behaviour/code-surface execution
- native persistence
- retirement of legacy SC source trees and old protocol docs

## J2 Scope

`J2` adds the first real native subsystem:

- `TransportEngine`
- JUCE-owned play / stop
- JUCE-owned tempo and meter
- JUCE-owned beat / bar / phase progression
- transport revision updates that drive the ruler and transport UI without any SC runtime

## Still Deferred After J2

- native module execution
- native audio generation
- native routing and mixer execution
- native scheduler authority
- native behaviour/code-surface execution
- native persistence
- retirement of legacy SC source trees and old protocol docs

## J3 Scope

`J3` adds the next native subsystem:

- `ClockDomain`
- `ClockDomainManager`
- JUCE-owned global, local, derived, and free-running demo domains
- relation editing that updates native domain state
- selected-lane overlay timing driven from native clock-domain computation

## Still Deferred After J3

- native module execution
- native audio generation
- native routing and mixer execution
- native scheduler authority
- native behaviour/code-surface execution
- native persistence
- retirement of legacy SC source trees and old protocol docs

## J4 Scope

`J4` adds the next native subsystem:

- `Module`
- `ModuleRegistry`
- explicit demo module classes
- lifecycle state changes without SC
- native timing attachment per module
- native code-surface ownership for the current prototype UI

## Still Deferred After J4

- native routing and mixer execution
- native scheduler authority
- native behaviour/code-surface parsing beyond the current placeholder text model
- native persistence
- retirement of legacy SC source trees and old protocol docs

## J5 Scope

`J5` adds the next native subsystem:

- `AudioEngine`
- JUCE audio callback output
- transport-following demo tone output
- free-running drone output
- audio readiness tracked inside the host runtime

## Still Deferred After J5

- native scheduler authority
- native behaviour/code-surface parsing beyond the current placeholder text model
- native persistence
- retirement of legacy SC source trees and old protocol docs

## J6 Scope

`J6` adds the next native subsystems:

- `MixerEngine`
- `RoutingGraph`
- mixer strip level and mute applied to native audio
- group/master gain applied to native audio
- route creation/deletion validated natively
- route enablement gates audible module output

## Still Deferred After J6

- native scheduler authority beyond the current transport/module prototype
- native behaviour/code-surface parsing beyond the current placeholder text model
- native persistence
- retirement of legacy SC source trees and old protocol docs

## J7 Scope

`J7` adds the next native subsystem:

- `Scheduler`
- JUCE-owned quantised boundary resolution
- immediate, next beat, next bar, and next phrase policies in the runtime scheduler API
- next-bar module activation through native scheduling rather than per-module transport polling

## Still Deferred After J7

- richer UI exposure for policies beyond next bar
- native persistence
- retirement of legacy SC source trees and old protocol docs

## J8 Scope

`J8` adds the next native subsystem:

- `BehaviourParser`
- parseable native behaviour surfaces
- safe rejection of invalid edits
- keep last valid behaviour active on parse failure
- behaviour parameters now affect native audio output

## J9 Scope

`J9` adds:

- JUCE-native project snapshot save/load
- runtime rehydration for transport, clock domains, modules, mixer, routes, regions, automation, and structural state
- default saved project restore on launch

## J10 Scope

`J10` finishes the migration:

- README and architecture docs now describe the JUCE-native system
- protocol docs are rewritten as internal engine contracts rather than SC IPC
- legacy SuperCollider runtime files are removed from the repo
- dead IPC source files are removed from the build graph

## Remaining Deferred Work

- richer scripting than the current minimal behaviour parser
- more advanced DSP and module processor variety
- broader workflow polish beyond the migration close-out
