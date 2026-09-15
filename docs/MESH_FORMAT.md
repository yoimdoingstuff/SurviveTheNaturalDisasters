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

## Runtime loading

`engine/render/mesh.h` loads individual meshes. `engine/render/mesh_cache.h` provides a bounded cache for decoded meshes so repeated references to the same asset do not reparse the file. Cache entries own their decoded vertex/index memory and remain stable until the cache is cleared or destroyed.

The cache is deliberately bounded rather than growing forever. Legacy hardware has enough problems without us turning every map into an accidental memory-leak speedrun.

The current GLES2 draw path can consume an `nds_mesh` supplied by a Part. The remaining integration work is to make the map/content loader resolve project-owned geometry references into cached meshes automatically.

The importer/build pipeline can later convert source Roblox mesh data into this representation, with optional normals, UVs, materials, and compressed binary packaging added as separate versioned features.

The runtime should not read proprietary Roblox mesh containers directly. Source conversion belongs in the desktop import pipeline, while the device runtime consumes only project-owned assets.
