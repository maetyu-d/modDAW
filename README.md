# modular-sc-daw

`modular-sc-daw` is a prototype modular DAW with a fixed split:

- JUCE is the desktop shell
- `sclang` is the canonical musical engine
- `scsynth` is the audio engine

JUCE may render, supervise, edit, and request changes. It does not own musical time. `sclang` remains the source of truth for transport, clock domains, timing relations, quantised boundaries, module behaviour, lifecycle, and code-eval results.

## Current State

Milestones `M1` through `M30` are implemented in prototype form.

The current runtime is intentionally minimal and stability-first:

- cold engine startup
- one supervised `sclang` child process
- explicit stopped transport on launch
- engine-owned transport, modules, routes, mixer, arrangement, automation, structural state, recovery, and render state
- JUCE workflow views for arrangement, mixer, graph, and code
- throttled playback updates so repeated play/stop cycles stay stable

## Stable Playback Baseline

The current baseline favors efficiency over constant live redraw:

- the host UI timer runs slowly and only updates changed engine state
- playback logging is filtered to important lines
- analysis, automation, and module-state playback emissions are throttled
- transport stop resets back to a clean bar-1 start

This keeps the process boundary intact while avoiding the host becoming a high-frequency mirror of engine internals.

## What This Prototype Is

- explicit and debuggable
- milestone-driven
- hardcoded where that keeps the architecture clear
- engine-first rather than host-first

## What It Is Not

- a polished production DAW
- a plugin host
- a full mixer environment
- a waveform-heavy arranger
- a host-owned timing system

## Run

Build the JUCE app:

```sh
cmake -S /Users/user/Documents/ModDaw/modular-sc-daw/juce-host -B /Users/user/Documents/ModDaw/modular-sc-daw/juce-host/build
cmake --build /Users/user/Documents/ModDaw/modular-sc-daw/juce-host/build --config Debug
```

Launch:

```sh
open "/Users/user/Documents/ModDaw/modular-sc-daw/juce-host/build/modular_sc_daw_artefacts/Modular SC DAW.app"
```

## Docs

- [architecture.md](/Users/user/Documents/ModDaw/modular-sc-daw/docs/architecture.md)
- [protocol.md](/Users/user/Documents/ModDaw/modular-sc-daw/docs/protocol.md)
- [milestones.md](/Users/user/Documents/ModDaw/modular-sc-daw/docs/milestones.md)
