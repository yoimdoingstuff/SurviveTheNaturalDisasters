# CROSS_PLATFORM.md

## Goal

The runtime should be **mostly cross-platform**. Game code, scene data, scripts, physics, networking protocols, UI definitions, and save formats should be shared wherever possible. Platform-specific code belongs behind small interfaces.

## Tier 1 targets

### iOS
- iPad 4 / A6X
- iOS 8 primary legacy target
- iOS 6 secondary target where technically feasible
- OpenGL ES 2 baseline
- Objective-C/Objective-C++ platform layer
- Touch-first input

### Android
- Android 4.0+ / API 14+ target
- OpenGL ES 2 baseline
- Native C/C++ core through the Android NDK
- Java only for Android application/lifecycle glue where required
- Touch-first input
- Avoid APIs introduced after the minimum supported Android API unless behind compatibility wrappers

### Windows
- Native desktop build
- C/C++ core
- OpenGL 2.1-class baseline renderer, with an optional higher-quality path later
- Keyboard/mouse support
- Optional controller support
- Windows networking using the same transport abstraction as mobile builds
- No web runtime or Electron dependency

## Tier 2 / easy-port targets

The architecture should make Linux and other OpenGL ES/OpenGL-capable systems practical, but these are not release blockers unless explicitly promoted into the roadmap.

## Shared architecture

```text
                    Shared game project
                           |
              +------------+------------+
              |                         |
         Shared engine core       Shared content
              |                         |
      +-------+-------+---------+------+-------+
      |               |         |              |
    iOS            Android   Windows         Linux*
      |               |         |              |
 Obj-C++          JNI/NDK    Win32/SDL*     native*
      |               |         |              |
      +---------------+---------+--------------+
                      Platform API
```

`*` Linux/SDL-style support is optional and should not create dependencies that hurt Tier 1 targets.

## Graphics abstraction

The renderer should expose a small engine-level API rather than allowing gameplay code to call OpenGL directly.

Baseline feature set:

- vertex/index buffers
- textures
- 2D/3D transforms
- simple materials
- basic lighting
- alpha blending
- depth testing
- render targets only if the target platform supports them efficiently

The common renderer targets the capabilities shared by GLES2 and the chosen Windows desktop backend. Modern graphics APIs may be added later as optional backends.

## Input abstraction

```text
InputDevice
  touch
  mouse
  keyboard
  controller
  accelerometer (optional)
```

The gameplay layer should consume actions rather than platform-specific events. This lets the same game use touch on iOS/Android and keyboard/mouse/controller on Windows.

## Audio abstraction

Common API for:

- sound effects
- music
- looping sounds
- volume groups
- positional audio

Backend selection is platform-specific.

## Files and saves

Use a virtual path system so the game can refer to:

```text
assets/...
saves/...
config/...
cache/...
```

without knowing whether the physical path is an iOS application container, Android app storage, or a Windows directory.

## Networking

The network protocol must be platform-independent. Only the transport implementation changes.

Possible transports:

- LocalLoopbackTransport
- LAN UDP/TCP transport
- future Bluetooth transport where OS APIs permit it

The base game must not require networking.

## CPU and memory tiers

All platforms should share the same content where practical, but asset quality may vary by device tier.

### Legacy mobile

- low texture resolution
- reduced effect counts
- smaller maximum player counts
- conservative physics object counts
- aggressive asset caching limits

### Windows/high-performance devices

- higher texture resolutions
- more particles/effects
- larger lobby limits where game logic permits
- optional visual enhancements

A game save should never become incompatible solely because a device uses a different quality tier.

## Build separation

The repo should eventually have something like:

```text
platform/
├── common/
├── ios/
├── android/
├── windows/
└── linux/          # optional
```

The engine/game directories must remain platform-neutral.

## Cross-platform rules

- Do not put iOS, Android, or Windows calls into gameplay scripts.
- Do not require a platform's proprietary networking API in shared code.
- Keep serialization deterministic across architectures.
- Keep integer/floating-point assumptions explicit.
- Test save files on at least one mobile and one desktop build.
- Prefer file formats and algorithms with portable implementations.
- Any new third-party dependency must be checked against every Tier 1 target.

## Mod portability

Mods should normally contain portable data and sandboxed Luau/Lua scripts. A single mod package should be usable across Windows, Android 4.0+, and iOS where the target runtime permits the required resource APIs. Platform-specific mod declarations are allowed only when a feature genuinely cannot be portable.
