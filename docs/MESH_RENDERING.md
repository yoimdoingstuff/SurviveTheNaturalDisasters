# Mesh rendering

The runtime uses `NDSMESH 1` as a small project-owned intermediate mesh format.

The format currently contains positions and 16-bit triangle indices. Keeping the index type at 16 bits limits a mesh to 65,535 vertices, which is intentional for the GLES2-era hardware target.

Mesh assets are loaded by the engine mesh API and can be used by renderer backends without depending on Roblox formats at runtime.

The current render fixture is `game/content/meshes/triangle.ndsmesh`. It is intentionally tiny so it can be used for desktop and legacy-device smoke testing.

The renderer will initially use the same Part transform and color path for meshes. Normals, UVs, textures, materials, LODs, and collision meshes are separate steps so the base renderer stays small.
