# Legacy Mesh Format

`ndsmesh` is a small project-owned intermediate mesh format for the runtime. It is intentionally text-based during development so generated assets are easy to inspect and debug.

The runtime format is:

```text
NDSMESH 1
vertex_count 3
index_count 3
vertices
0 0 0
1 0 0
0 1 0
indices
0 1 2
```

Vertices currently contain position only. Indices are 16-bit and must be a multiple of three. This keeps the format directly usable by the GLES2-era renderer and avoids requiring a large general-purpose asset library on legacy devices.

The importer/build pipeline can later convert source Roblox mesh data into this representation, with optional normals, UVs, materials, and compressed binary packaging added as separate versioned features.

The runtime should not read proprietary Roblox mesh containers directly. Source conversion belongs in the desktop import pipeline, while the device runtime consumes only project-owned assets.
