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
2. Scene/Instance model
3. Physics + characters + camera
4. Luau/Lua gameplay runtime
5. Base gameplay loop
6. Menu screen
7. Local lobby creation and server settings
8. Offline single-player polish
9. LAN/local multiplayer
10. Cross-platform desktop/mobile validation
11. Legacy iOS optimization and packaging
12. Android 4.0+ and Windows release validation
13. Base release lock
14. Post-completion updates such as cosmetics and gear
15. Mod system and sandboxed content extensions
