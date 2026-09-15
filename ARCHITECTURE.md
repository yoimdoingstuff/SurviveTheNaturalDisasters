# ARCHITECTURE.md

## 1. Design target

The project is a standalone game runtime with a Roblox-like object/script model and a mid-2010s presentation target. It is not a full recreation of every Roblox service.

The engine implements only the platform/runtime capabilities required by the target game and its future local-multiplayer features.

## 2. High-level stack

```text
+----------------------------------------------------------+
| Game / UI                                                |
|  Menus | Lobby | Rounds | Disasters | Progression      |
+----------------------------------------------------------+
| Gameplay API                                             |
|  Instances | Events | Services | Script bindings      |
+----------------------------------------------------------+
| Simulation                                               |
|  Physics | Characters | Camera | Audio | Timers        |
+----------------------------------------------------------+
| Runtime                                                  |
|  Scene graph | Asset manager | Luau/Lua VM | Scheduler |
+----------------------------------------------------------+
| Platform abstraction                                     |
|  Files | Input | Audio | Threads | Sockets | Graphics  |
+----------------------------------------------------------+
| Platform backends                                          |
| iOS | Android 4.0+ | Windows | optional Linux               |
+----------------------------------------------------------+
```

## 3. Execution model

### Offline single-player

The game simulation, script runtime, physics, networking interfaces, and rendering all exist in one process. No socket is required.

```text
Game loop
  -> simulation tick
  -> local server/simulation authority
  -> gameplay scripts
  -> physics
  -> render extraction
  -> OpenGL ES frame
```

### Local host

One device becomes authoritative. Its local client is attached to the same simulation.

```text
Host process
  Simulation authority
       |
       +-- Host player client
       |
       +-- LAN/Bluetooth-capable transport
              |
              +-- Remote player 1
              +-- Remote player 2
```

Remote clients render locally. The network transports game state, inputs, and relevant events rather than sharing GPU rendering work.

## 4. Core subsystems

### 4.1 Instance system

Provide a lightweight object tree inspired by Roblox's Instance/DataModel concepts.

Required types for the first release:

- Instance
- Folder
- Model
- Part
- SpawnPoint
- Camera
- Character
- Player
- Value/object containers as needed

Avoid implementing hundreds of APIs that the game never uses.

### 4.2 Scene and rendering

Primary rendering target: OpenGL ES 2-era hardware.

Requirements:

- batched static geometry where practical
- simple materials
- texture atlasing where beneficial
- frustum culling
- low-overhead transparency path
- fixed maximums for expensive effects
- quality tiers suitable for A6X

The renderer must remain independent of gameplay logic.

### 4.3 Physics

Use a small deterministic physics layer. Candidate backend: Bullet or another engine proven to compile for the legacy target. Keep collision geometry simpler than visual geometry.

Required first-release capabilities:

- static collision
- dynamic rigid bodies
- gravity
- impulses
- triggers/contact events
- character collision
- debris/disaster interactions

### 4.4 Script runtime

Embed Luau or a compatible Lua runtime behind a narrow scripting API.

Do not bind the entire Roblox API. Implement only required classes/services/events.

Script domains:

- game/server simulation scripts
- client/UI scripts
- optional shared modules

### 4.5 Scheduler

Provide:

- fixed simulation tick
- frame callback
- delayed callback
- repeating timer
- cancellation

Avoid allocating temporary objects per callback on every frame.

### 4.6 Audio

Provide sound playback, looping ambient audio, volume groups, and basic 3D positional audio where performance allows.

### 4.7 Asset manager

Load packaged assets from a local bundle.

Asset types:

- textures
- meshes
- collision meshes
- sounds
- maps
- scripts
- UI definitions
- configuration

Cache limits must be configurable for legacy RAM constraints.

## 5. Game layer

The game layer owns:

- round state
- map selection
- disaster selection
- survival logic
- respawn logic
- lobby rules
- server settings
- score/progression hooks
- gear hooks
- cosmetic hooks

The engine does not know what a "natural disaster" is.

## 6. Menu and lobby architecture

The menu is part of the base release.

Flow:

```text
Boot
  -> Main Menu
       -> Play Offline
       -> Create Local Lobby
       -> Join Local Lobby
       -> Settings
       -> Credits
```

Lobby creation settings for the base release should include only practical settings such as:

- lobby name
- player limit
- selected map pool
- disaster selection mode
- round length/options that the gameplay supports
- host/join visibility

More exotic settings can arrive later.

## 7. Networking

Transport API:

```text
ITransport
  send()
  receive()
  connect()
  disconnect()
  poll()
```

Implementations can include:

- LocalLoopbackTransport
- LAN/UDPTransport
- future Bluetooth/local transport

Game logic should never directly depend on UDP/Bluetooth APIs.

## 8. Persistence

Save only small local data first:

- settings
- unlocked base progression
- cosmetic inventory
- last-used lobby settings

Never make cloud accounts a dependency.

## 9. Legacy iOS strategy

### iOS 8
Primary target.

### iOS 6
Secondary target. The project should avoid making iOS 8-only APIs mandatory in common code.

Platform code must live behind wrappers so an iOS 6-compatible implementation can be supplied where feasible.

## 10. Threading

Start single-threaded except for OS-managed audio/network/file operations. Add worker threads only after profiling identifies a real need.

The A6X is old; throwing threads at every problem is not a personality trait we need to cultivate.

## 11. Update model

The first complete release ends after:

- menu
- lobby creation
- server settings
- offline gameplay
- local multiplayer
- iOS 8 packaging
- legacy performance pass

Post-release updates may add:

- cosmetics
- unlockables
- gear
- additional disasters/maps
- extra settings
- optional progression systems


## 12. Cross-platform strategy

The common engine must be usable from iOS, Android 4.0+, and Windows without changing game logic. Platform code lives behind interfaces for graphics, input, files, audio, timing, threading, sockets, and application lifecycle.

Baseline graphics: OpenGL ES 2 on iOS/Android and an OpenGL 2.1-class backend on Windows. A higher-quality Windows renderer may be added later without changing game code.

Android minimum: API 14 / Android 4.0. Native C/C++ should contain the engine and gameplay runtime; Java is limited to Android lifecycle/integration glue.

Windows builds must be native desktop builds and must not depend on a browser, Electron, or web-only runtime.

The project should prefer conservative C/C++ and portable libraries so one codebase can be built with platform-specific toolchains.

## 13. Mod system

Mod support is planned after the base release. The mod layer sits above the engine and game API rather than modifying platform backends.

```text
Base Runtime
    |
    +-- Base Game
    |     |
    |     +-- Mod Manager
    |           |
    |           +-- Manifest/Dependency Resolver
    |           +-- Content Registry
    |           +-- Script Sandbox
    |           +-- Mod Configuration
    |           +-- Multiplayer Compatibility
    |
    +-- Platform Abstraction
```

Mods should be portable data packages whenever possible. Native code plugins are explicitly out of scope for the initial mod system because they would break much of the cross-platform model and create additional security/support problems.

### Content registration

Mods should register content through stable IDs rather than replacing engine internals directly.

Examples:

```text
map.example_newmap
disaster.example_storm
gear.example_item
cosmetic.example_hat
```

The game layer decides how these definitions behave; the mod manager handles discovery, dependency ordering, validation, and enable/disable state.

### Script sandbox

Mod scripts use the embedded Luau/Lua runtime and receive only documented, whitelisted APIs. They must not call arbitrary native functions or load platform libraries.

### Multiplayer handshake

The host includes required gameplay-mod IDs, versions, and content hashes during lobby negotiation. A client should be rejected before starting a round when required gameplay content does not match.
