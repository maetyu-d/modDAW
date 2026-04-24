# Development Setup Notes

## Dependencies

- CMake 3.22+
- a JUCE checkout with CMake support

## Local Paths

- Provide JUCE with `-DJUCE_SOURCE_DIR=/Users/user/Documents/Fabric`
- The app writes its default native snapshot to `/Users/user/Documents/ModDaw/modular-sc-daw/projects/demo-project.json`

## Current Runtime

The app is JUCE-native at runtime:

- no `sclang`
- no `scsynth`
- no external engine process

## Build

```sh
cmake -S /Users/user/Documents/ModDaw/modular-sc-daw/juce-host -B /Users/user/Documents/ModDaw/modular-sc-daw/juce-host/build
cmake --build /Users/user/Documents/ModDaw/modular-sc-daw/juce-host/build --config Debug
```
