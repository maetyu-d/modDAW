# modular-sc-daw

`modular-sc-daw` is a milestone-driven prototype modular DAW implemented as a JUCE-native desktop application.

## Current Migration State

- Prototype milestones `M1` through `M30` informed the original feature shape.
- Migration milestones `J1` through `J10` are now complete.
- The app has no runtime dependency on `sclang`, `scsynth`, or the old `sc-engine`.
- Global transport, clock domains, modules, audio, routing, mixer, scheduler, and persistence now live inside JUCE-native engine subsystems.
- Editable behaviour surfaces are parsed natively in C++ and applied safely at musical boundaries.
- Project save/load now writes and restores a JUCE-native snapshot at [projects/demo-project.json](/Users/user/Documents/ModDaw/modular-sc-daw/projects/demo-project.json).

## Conceptual Model Being Preserved

The migration keeps the conceptual system intact while changing the implementation substrate:

- polymorphic modules
- capability-based module model
- global transport
- clock domains
- local / derived / free-running timing modes
- global ruler with local overlays
- routing
- mixer
- structural / conductor behaviour
- editable behaviour surfaces

## Migration Approach

The migration was carried out in explicit stages from `J1` through `J10` so user-facing behaviour could stay coherent while the implementation substrate changed.

## Current Runtime

The app launches as a JUCE desktop application with:

- an internal engine status lifecycle: `offline -> booting -> ready`
- JUCE-native transport, clocks, modules, routing, mixer, scheduler, and audio
- JUCE-native project snapshot save/load
- no external process launch
- no SuperCollider runtime dependency

## Build

```sh
cmake -S /Users/user/Documents/ModDaw/modular-sc-daw/juce-host -B /Users/user/Documents/ModDaw/modular-sc-daw/juce-host/build
cmake --build /Users/user/Documents/ModDaw/modular-sc-daw/juce-host/build --config Debug
```

## Launch

```sh
open "/Users/user/Documents/ModDaw/modular-sc-daw/juce-host/build/modular_sc_daw_artefacts/Modular SC DAW.app"
```

## Docs

- [architecture.md](/Users/user/Documents/ModDaw/modular-sc-daw/docs/architecture.md)
- [protocol.md](/Users/user/Documents/ModDaw/modular-sc-daw/docs/protocol.md)
- [milestones.md](/Users/user/Documents/ModDaw/modular-sc-daw/docs/milestones.md)
