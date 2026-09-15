# CONTENT_PIPELINE.md

## Goals

Keep game content independent of platform binaries so the same package can feed iOS 8, iOS 6, and desktop development builds.

## Suggested package

```text
content/
  maps/
  meshes/
  textures/
  sounds/
  scripts/
  ui/
  config/
```

## Early map import

Map import is intentionally one of the first practical milestones. The goal is to get the original/base Natural Disaster Survival map content into the runtime format early, so engine work can be driven by real content requirements.

Recommended workflow:

```text
local NDS source files
  -> inspect/index
  -> import
  -> normalize
  -> project map package
  -> test in desktop runtime
  -> optimize for iPad/Android
```

The importer should be incremental. A map can become playable even when some advanced source features are unsupported. Missing features should fall back to placeholder geometry, default materials, or compatibility shims and be reported in an import log.

Keep source files in a separate local-only workspace when they cannot legally be redistributed. The generated project format should contain only content the project is permitted to ship.

## Maps

Maps should contain:

- visual geometry
- collision geometry
- spawn points
- map metadata
- disaster-specific markers/hooks

## Sharing maps

Portable maps use the `.ndsmap` sharing package format. It is a ZIP container containing the project-owned `map.json`, a package manifest, and optional converted assets:

```text
my_map.ndsmap
  package.json
  map.json
  assets/
    meshes/
    textures/
```

The dependency-free `tools/map_importer/nds_package.py` tool can pack and unpack these files. The package format deliberately avoids requiring ZIP support inside the legacy native runtime. A platform UI can import a package, extract it to its content directory, and then pass the normal `nds-map` JSON to the runtime.

Only legally redistributable content should be bundled. Proprietary source files remain outside shared packages unless redistribution rights are established.

## Scripts

Scripts should use the project's documented compatibility API rather than direct platform calls.

## Asset optimization

Provide an offline conversion step for:

- texture resizing/compression appropriate to target
- mesh optimization
- collision mesh generation
- audio conversion
- package indexing

Maintain source assets separately from generated legacy assets.
