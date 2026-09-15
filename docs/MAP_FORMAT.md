# Project Map Format

The runtime uses a small project-owned `nds-map` JSON package rather than reading Roblox place files directly at runtime.

The conversion boundary is:

`local source (.rbxlx/.rbxmx) -> importer -> nds-map -> native loader -> Instance tree -> draw/physics systems`

## Current format

The current loader supports:

- Instance IDs and parent IDs
- Instance class and name
- `transform.position`
- `transform.size`
- `transform.rotation`
- Part transparency
- Part reflectance
- Part color
- Part anchored/can-collide state
- SpawnLocation mapped to the runtime SpawnPoint class

Unsupported source properties are allowed to remain in the imported package as ignored metadata rather than making the entire import fail.

## Geometry roadmap

The current renderer represents a Part as a cube. This is intentional for the first vertical slice, but it is not sufficient for the real NDS maps.

The next map-format revision should add project-owned geometry references without coupling the runtime to Roblox's source representation:

```text
geometry
  type: box | mesh
  mesh: asset-id (when type is mesh)
  collision: asset-id (optional)
material
  texture: asset-id (optional)
  color: RGBA
  transparency: 0..1
```

Meshes and textures should be converted into legacy-friendly runtime assets during import. The runtime should never need to understand proprietary source asset containers just to render a map.

## Legacy constraints

Generated assets must support the GLES2 baseline and the iPad 4/A6X memory budget. The importer may keep high-resolution source assets for development, while the build pipeline produces smaller legacy variants.

The format should remain deterministic and versioned so the same map package can be tested on Windows, Android, and iOS without platform-specific map data.
