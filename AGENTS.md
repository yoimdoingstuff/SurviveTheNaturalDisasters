# AGENTS.md

## Project purpose

Build a self-contained, offline-first Natural Disaster Survival-style runtime for legacy iPad hardware. The runtime must not depend on Roblox services or an internet connection for core gameplay.

The project is also a preservation-oriented reimplementation. It should capture the feel of classic Roblox disaster-survival games while using project-owned code, data, and assets. Do not depend on Roblox being available at runtime.

## Design references

Use classic Roblox disaster-survival games as gameplay references, not as a requirement to reproduce proprietary implementation details.

### The Disaster Games

The Disaster Games [Alpha Access] was a competitive branch of the classic Natural Disaster Survival formula. Its documented design goals included short survival rounds, game tips, a shop/tool system, achievements, minigame-style modes, additional maps, and additional disasters. The project can take inspiration from these ideas, especially:

- Make each round immediately communicate the current disaster and the survival objective.
- Keep rounds readable and fast to understand.
- Treat maps as gameplay spaces with distinct routes, shelters, hazards, and destructible structures rather than decorative scenery.
- Design disasters so they materially change the map and force players to react.
- Leave room in the architecture for additional maps, disasters, achievements, tools, and alternate minigame modes.

Reference: https://www.roblox.com/games/136187555/The-Disaster-Games

The reference page currently describes the game as a competitive branch of Natural Disaster Survival and lists planned features including game tips, a shop with tools/costumes, achievements, minigame modes, new maps, and new disasters. Treat those as inspiration only, not as a specification copied from the original game.

### Natural Disaster Survival source/reference material

A source archive supplied by the project maintainer may be used as a reference for gameplay behavior, map construction, disaster sequencing, and legacy design details. When using supplied material:

- Prefer understanding behavior and porting concepts into project-owned systems.
- Port maps/data into the NDS map format instead of depending on Roblox runtime objects.
- Rewrite or adapt code when necessary to fit this engine's architecture.
- Do not copy proprietary Roblox assets, scripts, or code into the repository unless the project maintainer has clear rights to redistribute them.
- Preserve provenance for any reused third-party code or data and check its license before committing it.
- Favor original implementations when provenance or redistribution rights are unclear.

The maintainer-provided archive link is documented in the task conversation; the development environment may require the archive to be uploaded directly if the external host is not accessible to tooling.

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
12. Maps must be built from multiple meaningful parts/blocks. Avoid single giant monolithic map pieces when destructibility or traversal depends on individual structures.
13. Destruction is a core gameplay feature, not visual dressing. Important structures should be able to break, move, collapse, or become unsafe during disasters.
14. Every playable round must clearly identify its active or incoming disaster in the HUD and/or other in-game presentation.

## Architecture rules

- Engine code must not directly depend on game-specific disaster logic.
- Game scripts must not access platform APIs directly.
- Networking must be an optional transport layer.
- Single-player should use the same simulation interfaces as multiplayer, but permit an in-process/local transport.
- Rendering should target OpenGL ES 2-era capabilities.
- Avoid requiring programmable features newer than the target GPU unless behind optional feature gates.
- Keep memory allocations predictable in hot loops.
- Avoid large runtime reflection systems where static registration is practical.
- Maps should expose block/part-level properties such as anchoring, collision, transforms, visibility, and destruction state so disasters can interact with individual structures.
- Disaster implementations should use shared scene/physics interfaces so new disasters can affect maps without hard-coding individual map names.

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
4. Review this file's disaster-survival design references when changing maps, disasters, rounds, UI, camera, or destruction.
5. Avoid implementing post-launch features while base-release blockers remain.
6. Add tests or a manual verification step.
7. Update `TODO.md` when the task is actually complete.

When porting a reference map:

1. Identify the major gameplay zones, shelters, open areas, elevated structures, escape routes, and likely disaster failure points.
2. Break those into separate parts/blocks with sensible collision and anchoring.
3. Preserve the map's gameplay identity while adapting scale and geometry to this engine.
4. Mark intentionally indestructible foundation/safety elements explicitly.
5. Verify the map works with more than one disaster type.

## Definition of done

A feature is not complete because it compiles on desktop. It must compile for the intended target (or have a documented legacy-port blocker), behave correctly in-game, and have a basic test/manual verification path.

For a map, "done" additionally means:

- The map is large enough for meaningful movement and disaster avoidance.
- Structures are composed of multiple individually simulated/destructible parts where appropriate.
- At least one disaster can visibly alter the map.
- The player can identify the current disaster without relying on external logs.
- Camera controls behave consistently with the intended desktop control scheme.

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
