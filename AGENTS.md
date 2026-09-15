# AGENTS.md

## Project purpose

Build a self-contained, offline-first Natural Disaster Survival-style runtime for legacy iPad hardware. The runtime must not depend on Roblox services or an internet connection for core gameplay.

## Non-negotiable priorities

1. iPad 4 / A6X feasibility comes before modern-engine convenience.
2. The project is mostly cross-platform: iOS 8/iOS 6 (where feasible), Android 4.0+ (API 14+), and Windows are Tier 1 targets.
3. iOS 8 is the primary legacy Apple target; iOS 6 compatibility is a secondary target.
4. Offline single-player must work without networking enabled.
5. Menu screen and local lobby creation are part of the base release, not post-launch updates.
6. Local multiplayer must work without a cloud backend.
7. Base gameplay must be complete before cosmetic/gear/update work begins.
8. Keep platform-specific code isolated.
9. Prefer simple, deterministic systems over elaborate abstractions.
10. Do not introduce dependencies that cannot reasonably be built for the legacy target without documenting a replacement/fallback.
11. Never assume a modern desktop result will also work on the iPad 4.

## Architecture rules

- Engine code must not directly depend on game-specific disaster logic.
- Game scripts must not access platform APIs directly.
- Networking must be an optional transport layer.
- Single-player should use the same simulation interfaces as multiplayer, but permit an in-process/local transport.
- Rendering should target OpenGL ES 2-era capabilities.
- Avoid requiring programmable features newer than the target GPU unless behind optional feature gates.
- Keep memory allocations predictable in hot loops.
- Avoid large runtime reflection systems where static registration is practical.

## Code style

- C or conservative C++ for core systems.
- Objective-C / Objective-C++ only at iOS boundaries.
- Android Java/Kotlin code is limited to platform glue; shared runtime code remains native C/C++.
- Windows platform code must not leak into shared gameplay/engine interfaces.
- Keep public interfaces small.
- Comment around hardware/OS compatibility decisions.
- Every new subsystem must state its expected CPU, memory, and graphics cost on A6X-class hardware.

## AI/agent workflow

Before implementing a task:

1. Read `ARCHITECTURE.md`.
2. Read the relevant section of `TODO.md`.
3. Check whether the task is a prerequisite for an earlier milestone.
4. Avoid implementing post-launch features while base-release blockers remain.
5. Add tests or a manual verification step.
6. Update `TODO.md` when the task is actually complete.

## Definition of done

A feature is not complete because it compiles on desktop. It must compile for the intended target (or have a documented legacy-port blocker), behave correctly in-game, and have a basic test/manual verification path.

## Feature priority

### Base release

- Runtime boot
- Renderer
- Input
- Scene/instances
- Physics
- Player/character
- Camera
- Audio
- Luau/Lua scripting
- Maps
- Disaster system
- Round flow
- Survival/win/loss logic
- Menu screen
- Local lobby creation
- Server settings
- Offline save/config handling
- Local multiplayer
- iOS 8 build
- Android 4.0+ build
- Windows native desktop build

### Post-completion updates

- Additional cosmetics/unlockables
- Gear such as balloon, apple, compass-style utility items
- More server settings
- Extra maps/disasters
- Expanded progression
- Optional UI polish

Do not promote update-only features into the critical path for the base release.

## Mod support rules

- Mod support is post-completion and must not block the base release.
- Prefer data-driven mods and sandboxed scripts over native plugins.
- Keep mods platform-neutral wherever possible.
- Validate manifests, dependencies, paths, package versions, and content IDs.
- Never allow a mod to require internet access for core operation.
- Gameplay-affecting mods must participate in multiplayer compatibility checks.
- A broken mod must fail without taking down the base game.
- Mod APIs must be versioned and documented before being treated as stable.


## Existing engine reuse

Before implementing a major Roblox-style subsystem from scratch:

1. Check `docs/ENGINE_REUSE.md`.
2. Search existing open-source Roblox-like projects for reusable code or reference implementations.
3. Verify the exact license and provenance before copying or vendoring anything.
4. Keep reused code behind project-owned interfaces/adapters.
5. Test compatibility on Windows first, then iOS 8, iOS 6/CoolBooter, and Android 4.0+ as applicable.

Do not copy proprietary Roblox code or assets, and do not assume that a project being open-source makes every associated asset or dependency reusable.
