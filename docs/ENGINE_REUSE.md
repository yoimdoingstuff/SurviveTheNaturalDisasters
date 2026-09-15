# Existing Roblox-Like Engine Reuse Strategy

## Purpose

Do not automatically build every Roblox-style subsystem from scratch.
Before implementing a subsystem, investigate whether an existing open-source Roblox recreation or compatible runtime can provide a usable starting point, reference implementation, parser, API model, or portable code.

The goal is to reduce engineering time while keeping the project standalone, legally clean, maintainable, and compatible with the legacy iPad target.

## Important rule

An existing project is a candidate dependency or reference, not permission to copy proprietary Roblox code, assets, scripts, trademarks, or decompiled material without appropriate rights.

Only use source code, assets, formats, or other material that the selected project's license and the project's own content rights permit.
Record the exact upstream repository, commit/tag, license, and files reused in `docs/DECISIONS.md`.

## Preferred reuse order

1. Reuse a mature open-source subsystem directly when its license and platform constraints fit.
2. Adapt a compatible open-source Roblox-like subsystem behind our interfaces.
3. Use the project as a reference for behavior/API structure and reimplement independently.
4. Build the missing subsystem ourselves only when the above options are unsuitable.

Do not force a dependency into the engine merely because it looks similar to Roblox.

## Candidate projects to investigate

These are investigation targets, not mandatory dependencies. Verify their current state, license, architecture, and legacy-platform feasibility before adopting anything.

### OpenRBLX

Potential uses:

- Roblox-style object/model concepts
- `.rbxl` / `.rbxlx` / `.rbxm` / `.rbxmx` parsing ideas
- lightweight C/C++ implementation patterns
- historical API behavior references

Questions to answer before reuse:

- Can the relevant code compile with a legacy-compatible toolchain?
- What license applies to each reused component?
- Which pieces are complete enough to save development time?
- Can the renderer/backend be replaced with OpenGL ES 2?

### RNR's Not Roblox

Potential uses:

- historical Roblox-style DataModel and engine concepts
- networking/replication ideas
- serialization and scripting compatibility research
- understanding older Roblox behavior

Likely role: reference or selective subsystem reuse rather than a direct base, because its target era and dependency stack may not match the project's 2013-2015 target or iOS 6 constraints.

### Gargantuan

Potential uses:

- modern Roblox-like engine architecture
- Luau integration
- Roblox-style Instance/API concepts
- understanding which APIs are useful versus unnecessary

Likely role: architecture/reference source unless its dependencies can be made to fit the legacy target.

### Novalume

Potential uses:

- historical/modernized Roblox client/server concepts
- serialization, replication, animation, and compatibility research

Likely role: reference or selective reuse. Do not assume that a modern platform target will work on iOS 6/8.

### Other open-source Roblox-compatible projects

Continue searching for projects with:

- permissive or otherwise compatible licenses
- C/C++ or similarly portable implementations
- OpenGL ES-era rendering paths
- Luau/Lua support
- scene/model serialization
- older device support

A smaller, older, portable project can be more valuable here than a feature-rich modern recreation.

## Adapter architecture

Never make game code directly depend on a third-party Roblox recreation.

Use adapters:

```text
                     NDS Game
                         |
                  Compatibility API
                         |
              +----------+----------+
              |                     |
      Native implementation    Reused subsystem
              |                     |
              +----------+----------+
                         |
                   Engine interfaces
```

Examples:

- `IInstanceRuntime`
- `IScriptRuntime`
- `ISceneLoader`
- `IPhysicsBackend`
- `IRenderBackend`
- `INetworkTransport`

This makes it possible to replace an imported subsystem later without rewriting the game.

## Recommended bootstrap strategy

### Step A - Search before writing

For every major subsystem, ask:

> Does an open-source Roblox recreation already have this working in a form we can legally and technically reuse?

Check:

- Instance tree
- `.rbxl/.rbxlx` parsing
- Lua/Luau runtime integration
- character controller
- physics integration
- networking
- replication
- UI primitives
- asset loading

### Step B - Build a tiny proof of concept

Do not import an entire project immediately.

Prove one useful slice first, such as:

```text
load place
 -> create Instance tree
 -> execute script
 -> render one Part
 -> move camera
```

If the candidate cannot survive this test on Windows, stop before integrating more of it.

### Step C - Test legacy feasibility early

Once a candidate works on Windows, immediately test the highest-risk platform:

iOS 8 first, then iOS 6/CoolBooter.

For Android 4.0+, verify that the same backend can build with GLES2-era APIs.

### Step D - Keep the game independent

The Natural Disaster Survival game package must remain a normal project that uses our compatibility API.
It must not require a specific third-party recreation at runtime unless that project is deliberately adopted as a core dependency.

## What not to recreate

The project does not need a full copy of Roblox.
Do not implement or emulate systems that are not required by the game or future planned features, including:

- Roblox account infrastructure
- website functionality
- marketplace/backend services
- global discovery
- proprietary moderation services
- advertisements
- unnecessary Studio tooling
- hundreds of unused API classes

## Success criterion

A reused subsystem is successful only if it measurably reduces project work without making the legacy builds, cross-platform architecture, licensing, or maintenance substantially worse.
