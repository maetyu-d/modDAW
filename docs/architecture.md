# Architecture

## Runtime Ownership

The application is now fully JUCE-native.

JUCE owns:

- application startup and desktop UI
- global transport and ruler timing
- clock-domain relations and free-running timing
- module identity, lifecycle, and behaviour execution
- audio generation
- routing and mixer state
- quantised scheduling
- project persistence

There is no runtime dependency on `sclang`, `scsynth`, or the old `sc-engine`.

## Engine Split

The current native engine is divided into these main subsystems:

- `TransportEngine`
- `ClockDomain`
- `ClockDomainManager`
- `Module`
- `ModuleRegistry`
- `AudioEngine`
- `RoutingGraph`
- `MixerEngine`
- `Scheduler`
- `ProjectState`

## Authority Model

Musical authority now lives entirely in JUCE.

The app preserves the conceptual model from the earlier prototype:

- polymorphic modules
- capability-based module behaviour
- global transport
- local, derived, and free-running clock domains
- global ruler with local overlays
- routing and mixer layers
- editable behaviour surfaces
- structural and conductor state

## Persistence

Project save/load persists declarative JUCE-native state:

- transport
- clock domains
- modules
- mixer
- routes
- regions
- automation lanes
- structural state

The saved file reconstructs the runtime; it does not persist transient audio callback state as canonical truth.

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
