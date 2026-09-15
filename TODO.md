# TODO.md

Legend:

- [ ] Not started
- [~] In progress
- [x] Complete
- [!] Blocked

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

## Phase 1 - Native runtime core

### Early content import/bootstrap (do this before full gameplay)

- [x] Define an import workspace for locally obtained NDS game files
- [x] Add map/package discovery tool
- [x] Add initial Roblox place/model importer for locally available source files (.rbxlx/.rbxmx)
- [ ] Import the base Natural Disaster Survival map set as development content
- [x] Preserve source files outside generated/build output
- [x] Convert imported XML maps into the project-owned map/package format
- [ ] Validate imported geometry, textures, spawn points, scripts, and metadata on Windows
- [x] Add placeholder/fallback metadata for unsupported source classes instead of failing the import
- [ ] Build a first playable imported map before implementing every engine feature
- [x] Track unsupported source features instead of blocking the whole import
- [x] Add repeatable map re-import/update workflow so imported maps can be refreshed as the importer improves
- [x] Add native runtime loader for the project-owned nds-map JSON format
- [x] Add native map-loader CTest coverage
- [x] Application bootstrap
- [x] Platform abstraction layer <!-- interface + Windows backend done; other platform backends still pending -->
- [x] File system abstraction
- [x] Timing/clock system
- [x] Main loop
- [~] Logging/debug console <!-- leveled console logging done; interactive in-game debug console still pending -->
- [x] Configuration system
- [x] Basic memory/performance instrumentation

## Phase 2 - Rendering

- [x] Define renderer interface and scene draw data
- [~] OpenGL ES 2 renderer <!-- GLES2-style shader/VBO path is implemented; native GLES2 platform contexts remain pending -->
- [ ] Shader/material system compatible with GLES2
- [ ] Texture loading
- [~] Mesh loading <!-- NDSMESH parser, fixture, map-reference resolution, CPU cache, draw-data propagation, and GPU buffer caching are implemented; source mesh conversion remains -->
- [ ] Basic lighting
- [x] Camera <!-- backend-neutral camera matrices added -->
- [ ] Frustum culling
- [~] Transparency <!-- Part transparency reaches renderer; alpha blending policy still pending -->
- [ ] UI rendering
- [ ] A6X performance test scene
- [ ] iOS 8 renderer validation
- [ ] Android GLES2 renderer validation
- [~] Windows OpenGL renderer validation <!-- native WGL context and first Part/mesh draw added; local runtime validation still required -->
- [ ] iOS 6 renderer compatibility test

## Phase 3 - Scene/Instance model

- [ ] Complete Instance property/value containers
- [ ] Script-visible hierarchy API
- [ ] Attributes/tags compatibility layer
- [ ] Collection/service lookup API
- [ ] Scene serialization/deserialization validation

## Phase 4 - Physics and characters

- [ ] Physics backend selection and legacy build test
- [ ] Static collision
- [ ] Dynamic bodies
- [ ] Gravity
- [ ] Character controller
- [ ] Spawn/respawn
- [ ] Basic humanoid state
- [ ] Raycasts
- [ ] Touch/triggers
- [ ] Debris interaction

## Phase 5 - Script/runtime

- [ ] Select and integrate Luau/Lua runtime
- [ ] Script scheduler
- [ ] Shared/server/client script contexts
- [ ] Narrow gameplay API
- [ ] Protected script execution
- [ ] Runtime errors/logging
- [ ] Script asset loading

## Phase 6 - Natural Disaster Survival gameplay

- [ ] Round state machine
- [ ] Map rotation
- [ ] Disaster selection
- [ ] Disaster implementations
- [ ] Survival/death handling
- [ ] Respawn flow
- [ ] Player state
- [ ] Win/survival rewards
- [ ] Round countdown/intermission
- [ ] Spawn selection

## Phase 7 - Menu and local lobby

- [ ] Main menu
- [ ] Offline play flow
- [ ] Create local lobby
- [ ] Join local lobby
- [ ] Lobby name setting
- [ ] Player limit setting
- [ ] Map pool setting
- [ ] Disaster selection setting
- [ ] Round length setting
- [ ] Visibility setting
- [ ] Settings/credits screens

## Phase 8 - Local multiplayer

- [ ] Host-authoritative simulation
- [ ] Loopback transport
- [ ] LAN transport
- [ ] Player/input synchronization
- [ ] Round/disaster synchronization
- [ ] Spawn/death synchronization
- [ ] Basic disconnect handling
- [ ] Bluetooth transport investigation

## Phase 9 - Legacy/mobile/platform validation

- [ ] iOS 8 packaging
- [ ] iOS 6/CoolBooter feasibility
- [ ] Android 4.0+ packaging
- [ ] Windows release packaging
- [ ] iPad 4/A6X performance pass
- [ ] Low-memory/content quality tiers
- [ ] Device input validation
- [ ] Lifecycle/suspend/resume handling

## Phase 10 - Base release completion

- [ ] Full offline gameplay loop
- [ ] Menu + offline play
- [ ] Local lobby creation/join
- [ ] Server/lobby settings
- [ ] LAN multiplayer
- [ ] iOS 8 validation
- [ ] Performance target met on iPad 4/A6X
- [ ] Base content validation
- [ ] Release packaging

## Phase 11 - Post-completion updates

- [ ] Cosmetics/inventory
- [ ] Progression expansion
- [ ] Gear/items
- [ ] Additional lobby options
- [ ] Additional maps/disasters
- [ ] Mod package system
