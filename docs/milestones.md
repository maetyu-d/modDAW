# Milestones

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

## Current Progress

`M1` through `M30` are implemented in this repository scaffold.

Current stable runtime note:

- repeated play/activate-next-bar/stop cycles are stabilized with engine-side throttling and minimal host redraw
- startup is cold and deterministic
- the host remains a renderer/requester rather than a live high-frequency mirror of engine internals

`M13` introduces an engine-owned route list with audio and control route families. JUCE displays routes and requests route creation/deletion, while `sclang` validates endpoints and remains the route authority.

`M14` introduces a minimal persistent project snapshot. JUCE can request save/load, but `sclang` serializes declarative project data and reconstructs runtime state from that snapshot.

`M15` introduces frozen timeline regions. JUCE can request a per-module freeze, while `sclang` creates the canonical region and replays the frozen demo material through `scsynth`.

`M16` introduces basic arrangement editing for frozen regions: move, trim, split, and delete. Edits operate on projected/frozen region material and do not rewrite the underlying module clock-domain semantics.

`M17` introduces a minimal automation lane for `Kick Pulse` mixer level. `sclang` owns breakpoint data, interpolation, and playback application; JUCE renders and requests point changes.

`M18` introduces multiple code surfaces per module. `sclang` owns surface-specific eval, diagnostics, active revisions, and fallback behavior on failure.

`M19` introduces a hardcoded structural conductor module. `sclang` emits phrase-boundary directives with section, density, sync cue, phrase reset, orchestration, and target-module data; JUCE renders a structural lane and inspector details from engine state.

`M20` introduces a route graph view over the same engine-owned route list model. JUCE renders modules/ports/connections, allows visual create/delete gestures, shows the required route families, and still sends all route mutations through `sclang` validation.

`M21` introduces richer engine-owned timing relations for derived clock domains: `tempoShared`, `meterShared`, `phaseShared`, `phaseOffset`, and `hardSync`. JUCE can request relation changes from the timing inspector, but `sclang` validates and applies or rejects them.

`M22` introduces phrase lengths on clock domains and phrase-level structural scheduling. JUCE can request demo scene transitions for `nextPhrase`, `afterNCycles`, or `externalCue`; `sclang` computes and owns the pending boundary and emits the applied structural directive.

`M23` introduces a minimal group/bus layer. `sclang` owns a demo `Drum Group`, module-to-group assignments, group level/mute, and effective module gain through module strip, group bus, and master.

`M24` introduces a minimal sends/returns layer. `sclang` owns demo send definitions, pre/post send mode, send level, and a shared `Space Return` FX path; JUCE displays and requests changes without owning aux signal flow.

`M25` introduces explicit region identity. Frozen regions are detached material, while live-linked regions are projections of module behaviour; both are engine-owned and have distinct UI rendering and edit policies.

`M26` introduces minimal performance input. JUCE maps keyboard/buttons to explicit performance macros, but sends them to `sclang` as requests; the engine owns kick accents, external scene cues, and density-lift structural directives.

`M27` introduces a minimal analysis/feedback module. `sclang` owns a `Kick Listener` analysis module that derives envelope, onset, density, and brightness proxy data from kick trigger events, emits `analysis.state`, updates module runtime feedback, and exposes an inspectable control route to `Texture Drift`.

`M28` introduces minimal recovery containment. Failed code-surface eval remains isolated to the surface, `sclang` tracks unsaved project state, writes a declarative recovery autosave after project mutations, can rehydrate from that autosave after interruption, and emits `engine.recoveryState` for JUCE to display.

`M29` introduces engine-owned offline render packages. JUCE can request a full-mix render or per-module stem export, but `sclang` reconstructs the render range from canonical tempo, meter, regions, mixer state, and module state, then writes deterministic JSON render artifacts under `sc-engine/renders/`.

`M30` introduces a small workflow consolidation pass. JUCE now has arrangement, mixer, graph, and code modes plus shortcuts for mode switching, render, performance input, and save; these modes reorganize existing engine-owned views without moving musical authority into the host.
