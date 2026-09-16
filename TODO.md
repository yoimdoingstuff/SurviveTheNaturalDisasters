# TODO.md

Legend:
- [ ] Not started
- [~] In progress
- [x] Complete
- [!] Blocked

## Reference-driven gameplay backlog

These tasks are informed by classic Roblox disaster-survival design and the maintainer-provided Natural Disaster Survival source/reference material. They are implementation goals, not instructions to copy proprietary Roblox code or assets.

- [~] Establish a larger, multi-zone map scale appropriate for survival gameplay
- [~] Make playable maps primarily from individually addressable blocks/parts
- [x] Make disaster damage visibly alter map structures
- [x] Show the active/incoming disaster name in-game
- [x] Roblox-style RMB camera capture/relative-look behavior on Windows
- [x] Add Roblox-style third-person camera zoom, head-height orbit, and usable first-person mode
- [ ] Add disaster-specific environmental presentation (falling debris, wind, water, fire, sky/light changes)
- [~] Add multiple distinct shelters/strategic routes per map
- [~] Add map-specific interactive props/buttons where the reference design calls for them
- [~] Add map validation rules for minimum size, spawn safety, block granularity, and disaster interaction
- [x] Build a reference-informed map import/port workflow for supplied Natural Disaster Survival maps
- [~] Port the first supplied/reference map into project-owned .ndsmap.json data after provenance/licensing review
- [~] Port additional classic maps using the same block-by-block workflow
- [~] Compare disaster behavior against supplied reference material and tune timing/forces
- [ ] Add game tips/tutorial prompts inspired by classic disaster-survival presentation
- [~] Add an in-game map editor with block/part placement, selection, transform, property editing, deletion, save/load, and playtest workflow
- [ ] Add interactive editor UI, gizmos/selection highlighting, and playtest-mode switching on top of the editor core
- [ ] Leave extension points for additional tools, achievements, and minigame-style modes after base gameplay is stable

## Phase 0 - Project foundation

### Cross-platform baseline
- [ ] Define Tier 1 platforms: iOS 8/iOS 6 where feasible, Android 4.0+ (API 14+), Windows
- [x] Define shared platform API for graphics, input, files, audio, sockets, timing, lifecycle
- [ ] Confirm legacy-compatible build toolchains for each Tier 1 platform
- [x] Establish Windows development build
- [ ] Establish Android 4.0+ development build
- [ ] Define exact legal/content source for game assets and scripts
- [ ] Choose license for original runtime code
- [ ] Freeze target baseline: iPad 4 / A6X
- [ ] Establish iOS 8 build environment
- [ ] Establish iOS 6 build feasibility test
- [ ] Create shared desktop development build
- [x] Create basic CI/build scripts
- [ ] Audit existing open-source Roblox-like engines before implementing major subsystems
- [ ] Create comparison matrix for OpenRBLX, RNR, Gargantuan, Novalume, and other candidates
- [ ] Verify licenses and provenance for every candidate subsystem
- [ ] Prototype reuse of the most promising Instance/scene/script components
- [ ] Validate reused components on Windows before deeper integration
- [ ] Validate legacy feasibility on iOS 8 before committing to a dependency
- [ ] Validate iOS 6/CoolBooter and Android 4.0+ constraints before locking the engine stack
- [ ] Keep third-party engine code behind project-owned adapter interfaces

### Current map-port status
- [x] Happy Home geometry ported
- [x] Island Ruins geometry ported
- [x] Downtown geometry ported
- [x] Coastal Village geometry ported
- [x] Arch Park geometry ported
- [x] Surf Central geometry ported
- [x] Fort Indestructible geometry ported
- [x] Trailer Park geometry ported
- [x] Glass Office geometry ported
- [x] Launch Land geometry ported
- [ ] Port remaining classic maps from the supplied/reference set

## Phase 1 - Native runtime core

### Early content import/bootstrap
- [x] Define an import workspace for locally obtained NDS game files
- [x] Add map/package discovery tool
- [x] Add initial Roblox place/model importer for locally available source files (.rbxlx/.rbxmx)
- [~] Import the base Natural Disaster Survival map set as development content
- [x] Preserve source files outside generated/build output
- [x] Convert imported XML maps into the project-owned map/package format
- [ ] Validate imported geometry, textures, spawn points, scripts, and metadata on Windows
- [x] Add placeholder/fallback metadata for unsupported source classes instead of failing the import
- [x] Build a first playable imported/development map before implementing every engine feature
- [x] Track unsupported source features instead of blocking the whole import
- [x] Add repeatable map re-import/update workflow
- [x] Add native runtime loader for the project-owned nds-map JSON format
- [x] Add native map-loader CTest coverage
- [x] Application bootstrap
- [x] Platform abstraction layer
- [x] File system abstraction
- [x] Timing/clock system
- [x] Main loop
- [~] Logging/debug console
- [x] Configuration system
- [x] Basic memory/performance instrumentation
