# MODS.md

## Goal

The runtime should eventually support user-created mods without requiring users to rebuild the entire engine. Mods are local, versioned packages that extend the finished game with data and sandboxed scripts.

Mod support is intentionally a post-completion feature. The base game must be complete, stable, and independently playable before the mod API is considered stable.

## Mod package

Suggested structure:

```text
MyMod/
├── mod.json
├── assets/
│   ├── textures/
│   ├── models/
│   └── sounds/
├── maps/
├── disasters/
├── gear/
├── cosmetics/
├── scripts/
└── config/
```

A packaged distribution could use a simple archive format:

```text
MyMod.ndsmod
```

The runtime should treat the package as data and extract/cache only what is needed.

## Manifest

Example:

```json
{
  "id": "example.mod",
  "name": "Example Mod",
  "version": "1.0.0",
  "author": "Example Author",
  "runtime": {
    "min": "1.0.0"
  },
  "dependencies": [],
  "content": [
    "maps",
    "disasters",
    "scripts"
  ]
}
```

The final schema should be versioned so future runtime releases can validate old mods safely.

## Types of mods

### Data-only mods

Preferred for legacy devices. These can add or replace:

- maps
- textures
- models
- audio
- disaster definitions
- configuration values
- UI layouts
- cosmetics
- gear definitions

### Scripted mods

Sandboxed Luau/Lua scripts can extend gameplay through a documented API. Scripts must not receive unrestricted native access.

### Native plugins

Native code plugins should **not** be part of the initial mod system. They would destroy portability, create security problems, and make iOS/Android/Windows support unnecessarily painful.

If native extensions are ever considered, they should be optional platform-specific plugins with explicit warnings and strict versioning.

## Load order

Recommended order:

```text
Base game
   ↓
Core content patches
   ↓
Required dependency mods
   ↓
User-enabled mods
   ↓
Local overrides
```

Conflicts should be deterministic. The loader must report which package won when two mods attempt to replace the same asset or definition.

## Multiplayer compatibility

A lobby must know which gameplay-affecting mods are active.

The host should advertise something like:

```text
Mod ID + version + content hash
```

Clients with mismatched required mods should be rejected before joining a round rather than discovering the mismatch halfway through a tornado.

For data-only visual mods that do not affect simulation, the host may permit clients to use their own local appearance.

## Cross-platform requirements

Mods should normally be platform-neutral and use shared formats. The same mod package should work on:

- Windows
- Android 4.0+
- iOS 8 where supported by the sandboxed filesystem and package implementation
- iOS 6 only where the storage/runtime implementation remains practical

Mods must be able to declare optional platform restrictions when a feature genuinely cannot work everywhere.

## Security and stability

Mods are untrusted input.

The loader must:

- validate manifests
- validate paths and reject path traversal
- validate package structure
- enforce package/resource limits where practical
- isolate scripted APIs
- reject unsupported runtime/API versions
- fail gracefully when an asset or script is broken

The engine should never require internet access to install or use a mod.
