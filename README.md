# Natural Disaster Survival Local Runtime

A standalone, offline-first runtime intended to reproduce the feel and gameplay structure of a mid-2010s Roblox-style Natural Disaster Survival experience on legacy Apple hardware, with optional local multiplayer over LAN/Bluetooth-capable transports.

## Core goals

- Run the complete base game locally on-device.
- Target iPad 4 / A6X as a first-class legacy constraint.
- Be mostly cross-platform across iOS, Android 4.0+, and Windows, with shared engine/game code and thin platform backends.
- Support iOS 8 and an optional iOS 6/CoolBooter build where technically feasible.
- Use a lightweight native engine architecture suitable for OpenGL ES 2-era hardware.
- Provide a proper boot/menu flow and local lobby creation before post-launch feature updates.
- Support offline single-player first, then local multiplayer.
- Keep game content and engine code separate so the runtime can evolve without rewriting the whole project.

## Scope rule

This repository is a planned implementation framework. It does not contain proprietary Roblox binaries, proprietary Roblox source code, or copied game assets. Game data should be imported/recreated from material the project is legally allowed to use.

## Documents

- `AGENTS.md` - project rules and agent/developer guidance
- `ARCHITECTURE.md` - technical architecture and boundaries
- `TODO.md` - master implementation roadmap
- `docs/ROADMAP.md` - milestone sequencing
- `docs/IPAD_LEGACY.md` - iPad 4 / iOS 6 / iOS 8 constraints
- `docs/CROSS_PLATFORM.md` - iOS, Android 4.0+, Windows, and future-port architecture
- `docs/GAMEPLAY.md` - base-game feature specification
- `docs/LOCAL_MULTIPLAYER.md` - host/client design
- `docs/CONTENT_PIPELINE.md` - maps, assets, scripts, and data packaging
- `docs/UPDATE_MODEL.md` - post-completion feature/update strategy
- `docs/MODS.md` - versioned local mod package and sandbox design

## Suggested source layout

```text
engine/       Core runtime systems
networking/   Local multiplayer and host/session logic
game/         Natural-disaster gameplay layer
data/         Game definitions and packaged content
tools/        Offline asset/import/build utilities
platform/     iOS, Android, Windows, and future platform integration layers
tests/        Runtime and gameplay tests
```

## Development order

1. Native runtime boot + renderer
2. Early NDS map/content importer and first imported map
3. Scene/Instance model
4. Physics + characters + camera
5. Luau/Lua gameplay runtime
6. Base gameplay loop
7. Menu screen
8. Local lobby creation and server settings
9. Offline single-player polish
10. LAN/local multiplayer
11. Cross-platform desktop/mobile validation
12. Legacy iOS optimization and packaging
13. Android 4.0+ and Windows release validation
14. Base release lock
15. Post-completion updates such as cosmetics and gear
16. Mod system and sandboxed content extensions

## Development acceleration

Before implementing a large Roblox-style subsystem from scratch, check `docs/ENGINE_REUSE.md`. The project may selectively reuse compatible open-source Roblox recreations or use them as implementation references, while keeping all game code behind project-owned interfaces.
