# Native Physics

The runtime contains a project-owned rigid-body foundation in `engine/game/physics.h` and `engine/src/game/physics.c`.

## Current architecture

- Anchored Parts are static collision bodies.
- Unanchored Parts are dynamic bodies affected by gravity, forces and impulses.
- Dynamic/static and dynamic/dynamic box contacts are resolved.
- Rotated Parts use conservative world-space AABBs for broad and narrow collision checks.
- A sweep-and-prune broad phase sorts bodies by X bounds and rejects pairs that cannot overlap on X, Y or Z before running the contact solver.
- Broad-phase index storage is allocated once and reused between simulation steps to avoid per-frame heap churn.
- Contact resolution includes penetration correction, restitution and Coulomb-style tangential friction impulses.
- Physics uses fixed-size internal substeps and clamps excessive frame times for stability.

## Why sweep-and-prune

A naive solver checks every body against every other body. That becomes expensive on real NDS maps because most Parts are nowhere near each other. Sweep-and-prune keeps the implementation small and dependency-free while eliminating most obviously separated pairs. It is also straightforward to keep deterministic because the ordering and comparisons are controlled by the engine.

This is intentionally lighter than a general-purpose physics package. The eventual iPad 4/A6X target makes memory usage, predictable CPU cost and minimal dependencies more important than feature breadth.

## Solver limitations

The current solver does not yet simulate angular velocity or full oriented-box contact geometry. The rotated AABB is conservative, so it may report contacts that a true OBB solver would reject. Continuous collision detection, joints, sleeping, spatial trees and advanced constraint solving are also not implemented yet.

## Runtime ordering

During a PLAYING round, disaster effects are applied first, then the physics world integrates and resolves contacts, followed by the player controller. This allows disasters to disturb physical map geometry while keeping the simulation layer independent of the renderer.

## Testing

`engine/tests/physics_test.c` covers scene registration, gravity/landing, broad-phase separation of distant bodies, impulses and friction. The test is part of the native CTest suite.
