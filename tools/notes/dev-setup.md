# Development Setup Notes

## Dependencies

- CMake 3.22+
- JUCE with CMake support
- SuperCollider `sclang`, found in one of these places:
- `SCLANG_BIN`
- a local build under `third_party/supercollider`
- platform-default install paths
- `PATH`

## Current Prototype Slice

Milestone 1 proves only:

- JUCE desktop app shell
- child-process launch of `sclang`
- line-based JSON messaging
- handshake and ping/pong supervision

The current host expects the repository root to be known at compile time through the JUCE host CMake target.

## Local Paths

- JUCE can be provided to CMake with `-DJUCE_SOURCE_DIR=/Users/user/Documents/Fabric`
- SuperCollider source is vendored at `third_party/supercollider`
- The engine launcher prefers a built local `sclang` if present, but it does not build SuperCollider automatically
