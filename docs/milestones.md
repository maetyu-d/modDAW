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

## Current Progress

`M1` through `M17` are implemented in this repository scaffold.

`M13` introduces an engine-owned route list with audio and control route families. JUCE displays routes and requests route creation/deletion, while `sclang` validates endpoints and remains the route authority.

`M14` introduces a minimal persistent project snapshot. JUCE can request save/load, but `sclang` serializes declarative project data and reconstructs runtime state from that snapshot.

`M15` introduces frozen timeline regions. JUCE can request a per-module freeze, while `sclang` creates the canonical region and replays the frozen demo material through `scsynth`.

`M16` introduces basic arrangement editing for frozen regions: move, trim, split, and delete. Edits operate on projected/frozen region material and do not rewrite the underlying module clock-domain semantics.

`M17` introduces a minimal automation lane for `Kick Pulse` mixer level. `sclang` owns breakpoint data, interpolation, and playback application; JUCE renders and requests point changes.

`M18` through `M20` are intentionally not implemented yet.
