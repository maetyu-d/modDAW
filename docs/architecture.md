# Architecture

## Process Boundaries

### JUCE host

The JUCE application is the desktop shell. It is responsible for:

- launching and supervising the engine process
- rendering UI
- presenting status and logs
- editing code surfaces
- requesting engine actions
- displaying engine-owned timing state

The JUCE host is not allowed to become the authority for musical time.

### sclang

`sclang` is the canonical musical engine. It is responsible for:

- transport state
- global and local clock domains
- timing relations
- ratios
- phase
- quantised timing boundaries
- module behaviour
- module lifecycle
- code evaluation results

All timing and musical decisions originate here.

### scsynth

`scsynth` is the audio engine. It is responsible for:

- sound synthesis
- audio execution
- server-side DSP graph execution

In later milestones, `sclang` will direct `scsynth`. The JUCE host will not bypass `sclang` to become the musical authority.

## Timing Authority Rules

- JUCE may render timing state, but does not author it.
- JUCE may request timing changes, but `sclang` decides them.
- `sclang` is the sole source of truth for transport, phase, quantisation, and timing boundaries.
- `scsynth` executes audio under engine control and does not define musical authority.

## UI vs Engine Responsibilities

### UI responsibilities

- visual ruler presentation
- lane presentation
- local timing overlays
- status indicators
- log display
- code editing surfaces
- user interaction

### Engine responsibilities

- authoritative transport state
- authoritative clock-domain graph
- module activation state
- module code evaluation
- quantised scheduling decisions
- timing inspection data

Milestone 2 adds one canonical global transport implemented in `sclang`. JUCE may request transport actions and render returned state, but it still does not author tempo, phase, beat position, or bar boundaries.

Milestone 4 adds plural clock domains, also authored in `sclang`. The host may inspect the domain graph and display derived timing values, but it still does not compute or own clock relationships.

Milestone 5 adds a minimal module model, also authored in `sclang`. JUCE may inspect module identity, attachment, lifecycle state, and placeholder code-surface text, but it does not own module lifecycle or behaviour.

Milestone 6 adds a selected-lane local timing overlay in JUCE. The host computes only the visual projection from engine-authored transport, module attachment, and clock-domain ratio data. It does not become the authority for those timing relationships.

Milestone 7 adds the first sound path. `sclang` boots and supervises `scsynth`, defines the demo synth behaviour, and triggers sound from the canonical transport. JUCE remains only a requester and renderer.

Milestone 8 adds the first quantised action. JUCE may request `activate next bar`, but `sclang` computes the bar boundary, queues the lifecycle change, and activates the module when the canonical transport reaches that boundary.

Milestone 10 adds a timing inspector in JUCE. The host may display timing relations for the selected lane, but it still derives those views from engine-authored transport, clock-domain ratio, and phase data rather than owning those values itself.

Milestone 11 adds minimal validation in `sclang`. The engine is responsible for deciding whether the current transport, clock-domain graph, and module references satisfy the prototype rules. JUCE may display findings, but it does not become the authority for those validation results.

Milestone 12 adds a minimal mixer owned by `sclang`. JUCE may display strips and request gain or mute changes, but the engine remains the source of truth for mixer state and for how those levels affect module sound output.
