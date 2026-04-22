# modular-sc-daw

`modular-sc-daw` is a prototype repository for a modular DAW built around a strict architectural split:

- JUCE is the shell, editor, supervisor, and renderer
- SuperCollider `sclang` is the canonical musical engine
- SuperCollider `scsynth` is the audio engine

The core rule is non-negotiable: JUCE must never become the authority over musical time. The host may request, display, and supervise state, but `sclang` remains the sole source of truth for transport state, clock domains, timing relations, ratios, phase, quantised timing boundaries, module behaviour, module lifecycle, and code evaluation results.

## Prototype Scope

This repository currently implements Milestones 1 through 12:

- a desktop JUCE app shell
- a connection status indicator
- a log panel
- launched `sclang` child process management
- newline-delimited JSON envelope messaging over a localhost socket bridge
- `engine.handshake`
- `engine.ready`
- `engine.ping`
- `engine.pong`
- one canonical transport model in `sclang`
- JUCE transport requests for `play`, `stop`, and state refresh
- transport state display rendered from engine-owned data
- one primary global ruler rendered in JUCE from engine transport state
- a visual playhead derived from the canonical engine beat position
- one global clock domain and multiple derived local clock domains in `sclang`
- JUCE clock-domain inspection rendered from engine-authored state
- a minimal engine-owned module model with hardcoded demo modules
- JUCE module inspection rendered from engine-authored module state
- three visible module lanes in JUCE derived from engine-owned modules
- one selected-lane local timing overlay rendered against the global ruler
- one demo module that produces sound through `scsynth`
- one quantised activation path: activate selected module on next bar
- one JUCE code surface editor for the selected module
- one engine-owned code-swap path: queue code changes and apply them on the next bar in `sclang`
- one read-only timing inspector for the selected lane, derived from engine-authored transport and clock-domain state
- one engine-owned minimal validation pass with explicit findings for transport, clock-domain, and module-reference structure
- one engine-owned minimal mixer with a master strip and per-module strips
- JUCE mixer controls that request level and mute changes without becoming the mixer authority
- one live audio-mixer path where the `Kick Pulse` strip and master strip affect the audible `scsynth` output

These first slices prove the process boundary, protocol shape, and the timing-authority rule before timeline or audio behaviour is added.

## Not Built Yet

The following are intentionally out of scope for this prototype slice:

- plugin hosting
- full mixer
- automation editor
- clip editing
- persistence and save/load
- waveform rendering
- route graph UI
- full validation framework
- advanced module templates
- multiple code surfaces per module
- multiple clock attachments per module

## Development Style

This project is milestone-driven only.

- We build the smallest useful slice that preserves the architecture.
- We do not jump ahead to later systems.
- We prefer explicit, debuggable code over abstraction-heavy infrastructure.
- We use hardcoded demo behaviour before serialization or generalization.
- We keep process boundaries clear.
- We keep message types explicit and typed.

See [docs/milestones.md](/Users/user/Documents/ModDaw/modular-sc-daw/docs/milestones.md) for the staged plan and [docs/architecture.md](/Users/user/Documents/ModDaw/modular-sc-daw/docs/architecture.md) for the authority model.
