# IPAD_LEGACY.md

## Hardware target

Primary hardware: iPad 4 with Apple A6X.

The project should assume limited memory, an older mobile GPU, and slower storage compared with modern devices.

## Rendering rules

- OpenGL ES 2.0 baseline.
- Avoid Metal-only features.
- Avoid geometry and shader complexity that cannot sustain acceptable frame rates on A6X.
- Prefer packed/atlased textures where useful.
- Keep post-processing optional or absent.
- Use aggressive but safe culling.

## Memory rules

- Avoid loading every map into memory at boot.
- Stream or load only the active map and required assets.
- Use compact binary/package formats where useful.
- Make caches bounded.
- Track texture memory separately from CPU memory.

## iOS 8

Primary shipping target for the first stable legacy build.

## iOS 6

Treat as a compatibility branch. Do not let iOS 6-specific compromises destabilize the iOS 8 build.

Every iOS 6 compatibility decision must be isolated behind platform wrappers where possible.

## CoolBooter

The project must not depend on CoolBooter at runtime. CoolBooter only supplies the old iOS environment. Installation and boot testing are external to the engine.
