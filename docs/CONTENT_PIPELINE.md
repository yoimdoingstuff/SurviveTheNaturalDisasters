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

## Maps

Maps should contain:

- visual geometry
- collision geometry
- spawn points
- map metadata
- disaster-specific markers/hooks

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
