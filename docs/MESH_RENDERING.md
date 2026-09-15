# Mesh rendering

The runtime uses `NDSMESH 1` as a small project-owned intermediate mesh format.

The format currently contains positions, optional UV coordinates, and 16-bit triangle indices. Keeping the index type at 16 bits limits a mesh to 65,535 vertices, which is intentional for the GLES2-era hardware target.

Vertex lines may contain either `x y z` or `x y z u v`. Meshes without UV coordinates default them to `0,0`, allowing older/simple assets to continue loading.

Mesh assets are loaded by the engine mesh API and can be used by renderer backends without depending on Roblox formats at runtime.

The current render fixture is `game/content/meshes/triangle.ndsmesh`. It is intentionally tiny so it can be used for desktop and legacy-device smoke testing.

The renderer currently uses the same Part transform and color path for meshes. UV data is now preserved by the mesh loader and tested, while GPU texture sampling/material binding remains a separate renderer step. Normals, materials, LODs, and collision meshes are also separate steps so the base renderer stays small.
