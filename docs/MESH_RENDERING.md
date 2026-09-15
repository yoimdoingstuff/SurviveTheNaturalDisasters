# Mesh rendering

The runtime uses `NDSMESH 1` as a small project-owned intermediate mesh format.

The format currently contains positions, optional UV coordinates, and 16-bit triangle indices. Keeping the index type at 16 bits limits a mesh to 65,535 vertices, which is intentional for the GLES2-era hardware target.

Vertex lines may contain either `x y z` or `x y z u v`. Meshes without UV coordinates default them to `0,0`, allowing older/simple assets to continue loading.

Mesh assets are loaded by the engine mesh API and can be used by renderer backends without depending on Roblox formats at runtime.

The current render fixture is `game/content/meshes/triangle.ndsmesh`. It is intentionally tiny so it can be used for desktop and legacy-device smoke testing.

The renderer uses the same Part transform and color path for meshes. UV data is preserved by the mesh loader and uploaded as interleaved position/UV vertex data by the GLES2 backend. RGBA8 textures are uploaded on first use into a bounded GPU cache, sampled with `texture2D`, and modulated by the Part color. Linear filtering and clamp-to-edge wrapping keep the path within the GLES2 feature set. Texture GPU objects are released with the renderer.

Texture references still need to be resolved by the map/content pipeline. Normals, richer materials, LODs, and collision meshes remain separate steps so the base renderer stays small and suitable for legacy hardware.
