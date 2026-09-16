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

### Early content import/bootstrap
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

## Phase 2 - Rendering
- [x] Define renderer interface and scene draw data
- [~] OpenGL ES 2 renderer
- [~] Shader/material system compatible with GLES2 <!-- basic shader path exists; textured material binding remains -->
- [~] Texture loading <!-- project-owned RGBA8 loader/cache exists; GPU texture upload/sampling remains -->
- [ ] Basic lighting
- [x] Camera
- [x] Frustum culling
- [~] Mesh loading <!-- NDSMESH parser, UV parsing, fixtures, map-reference resolution, CPU cache, draw-data propagation, and GPU buffer caching are implemented; source mesh conversion remains -->
- [x] Transparency
- [x] Persistent cube GPU buffers
- [ ] UI rendering
- [ ] A6X performance test scene
- [ ] iOS 8 renderer validation
- [ ] Android GLES2 renderer validation
- [~] Windows OpenGL renderer validation
- [ ] iOS 6 renderer compatibility test

## Phase 3 - Scene/Instance model
- [ ] Define Instance base class and class/type registry
- [ ] Parent/child hierarchy and lifecycle
- [ ] Name lookup and path traversal
- [ ] Property storage and typed properties
- [ ] Attributes/metadata
- [ ] Transform hierarchy and world/local transforms
- [ ] Part/BasePart property model
- [ ] Model/Folder/Workspace containers
- [ ] SpawnLocation and spawn-point support
- [ ] Script/LocalScript/ModuleScript instance types
- [ ] Clone/destroy/reparent operations
- [ ] Signals/events and connections
- [ ] Collection/tag support
- [ ] Instance serialization/deserialization
- [ ] Runtime mutation tracking
- [ ] Map-to-Instance validation tests

## Phase 4 - Physics and collision
- [~] Define physics abstraction suitable for legacy platforms
- [x] Static world collision
- [x] Primitive box collision
- [x] Broad-phase collision detection <!-- sweep-and-prune with reusable index storage -->
- [~] Narrow-phase collision tests <!-- conservative oriented-box AABB contact solver -->
- [ ] Raycasts
- [ ] Overlap queries
- [x] Gravity
- [x] Velocity/acceleration integration
- [x] Friction and restitution
- [x] Anchored/unanchored behavior
- [ ] Basic constraints/joints
- [x] Character collision controller
- [~] Physics update determinism <!-- fixed-ish substeps and deterministic broad-phase ordering; full replay validation remains -->
- [ ] Low-memory physics configuration for A6X
- [~] Physics stress test

## Phase 5 - Characters and humanoid systems
- [ ] Character model abstraction
- [ ] Humanoid/controller state machine
- [x] Walk/run/jump
- [x] Ground detection
- [x] Health/death states
- [x] Respawn system
- [x] Character physics integration
- [x] Camera follow/third-person controller
- [ ] Touch movement controls
- [x] Desktop keyboard/mouse controls
- [ ] Controller/gamepad input
- [ ] Character animation abstraction
- [ ] Basic legacy-compatible avatar rig
- [ ] Player/character ownership model
- [ ] Character replication hooks

## Phase 6 - Luau scripting/runtime
- [ ] Integrate project-approved Luau runtime
- [ ] Define script execution environment
- [ ] Bind Instance API to Luau
- [ ] Bind properties, methods, events and signals
- [ ] Script scheduler/coroutines
- [ ] Server Script execution
- [ ] LocalScript execution
- [ ] ModuleScript loading
- [ ] Require/module dependency handling
- [ ] Protected execution and error reporting
- [ ] Script time/memory limits
- [ ] Deterministic simulation-facing APIs
- [ ] Content/package script loading
- [ ] Unsupported Roblox API compatibility layer
- [ ] Script compatibility test suite

## Phase 7 - Asset/content pipeline
- [ ] Complete NDS map import pipeline
- [ ] Convert source meshes into project mesh format
- [ ] Complete texture conversion and references
- [ ] Material definitions
- [ ] Asset dependency manifest
- [ ] Asset IDs/path normalization
- [ ] Spawn/map metadata conversion
- [ ] Sound asset conversion
- [ ] Animation asset conversion
- [ ] Script extraction/import workflow
- [ ] Unsupported-feature report
- [ ] Deterministic package builds
- [x] Re-import/update command
- [~] Versioned game/content package format

## Phase 8 - Audio and presentation
- [ ] Audio abstraction
- [ ] Sound loading/decoding suitable for legacy devices
- [ ] 2D UI sounds
- [ ] 3D positional audio
- [ ] Music playback
- [ ] Sound volume/mixer controls
- [ ] Basic particle/effect abstraction
- [ ] Sky/lighting environment
- [ ] Basic post-processing only where legacy hardware allows
- [ ] Loading screens
- [~] In-game HUD rendering
- [ ] Debug overlay
- [ ] Pause/options UI

## Phase 9 - NDS gameplay prototype
- [~] Implement round state machine
- [~] Intermission/countdown
- [ ] Map selection
- [x] Disaster selection/randomization <!-- deterministic two-disaster rotation currently implemented -->
- [~] Disaster lifecycle API <!-- Earthquake and Windstorm are integrated -->
- [x] First disaster implementation <!-- Earthquake prototype -->
- [~] Second disaster implementation <!-- Windstorm prototype -->
- [x] Player survival/death tracking
- [~] Round completion/results
- [ ] One complete playable map
- [~] Multiple disaster prototypes
- [~] Offline single-player round using the same simulation interfaces intended for multiplayer
- [~] Disaster warning phase
- [ ] Validate complete end-to-end round

## Phase 10 - Base game systems
- [ ] Multiple NDS maps
- [~] Multiple disaster types
- [ ] Disaster configuration/settings
- [ ] Round timing configuration
- [ ] Spawn selection
- [ ] Player elimination/survival logic
- [ ] Spectating
- [ ] Results screen
- [ ] Base HUD
- [ ] Audio integration
- [ ] Save/config support
- [ ] Local settings persistence
- [ ] Basic statistics needed by the base game
- [ ] Complete recognizable NDS gameplay loop

## Phase 11 - Menu and local lobby
- [ ] Main menu
- [ ] Offline play entry
- [ ] Local lobby creation
- [ ] Local lobby joining
- [ ] Lobby browser/discovery
- [ ] Player list
- [ ] Server/lobby settings
- [ ] Map/disaster settings UI
- [ ] Loading screen flow
- [ ] Pause/options screen
- [ ] Lobby lifecycle/state machine
- [ ] Single-player lobby path
- [ ] Make menu and local lobby part of the base release
- [ ] Do not begin post-release cosmetic/progression work until this phase is complete

## Phase 12 - Local multiplayer
- [ ] Define host-authoritative simulation model
- [ ] Define network message/protocol format
- [ ] Local socket transport
- [ ] LAN host discovery
- [ ] Host/client connection lifecycle
- [ ] Player join/leave handling
- [ ] Input replication
- [ ] Character state replication
- [ ] Instance/property replication
- [ ] Disaster state replication
- [ ] Round state replication
- [ ] Reliable/unreliable message channels where required
- [ ] Snapshot/interpolation strategy
- [ ] Disconnect/error handling
- [ ] Local-only multiplayer validation
- [ ] Bluetooth transport investigation after LAN is stable

## Phase 13 - Cross-platform ports and validation
- [ ] Android 4.0+ native build
- [ ] Android GLES2 renderer
- [ ] Android input/touch layer
- [ ] Android audio/filesystem/socket validation
- [ ] iOS 8 native build
- [ ] iOS 8 GLES2 renderer/context
- [ ] iOS touch/input layer
- [ ] iOS audio/filesystem/socket validation
- [ ] Windows feature-parity validation
- [ ] Same content package loads across platforms
- [ ] Cross-platform save/config compatibility
- [ ] Cross-platform multiplayer compatibility
- [ ] iOS 6/CoolBooter feasibility build
- [ ] iOS 6 renderer/input/filesystem validation if feasible
- [ ] Document unsupported platform limitations

## Phase 14 - iPad 4 / A6X optimization
- [ ] Establish representative A6X benchmark scene
- [ ] Establish memory budget
- [ ] Establish texture/mesh budgets
- [ ] Reduce draw calls and state changes
- [ ] Batch compatible geometry
- [ ] Optimize transparency sorting/storage
- [ ] Reuse frame allocations/scratch buffers
- [ ] Reduce per-frame heap allocation
- [ ] Profile CPU/GPU frame time
- [ ] Profile loading time
- [ ] Test thermal behavior
- [ ] Test long-session stability
- [ ] Optimize touch UX for iPad
- [ ] Verify target frame rate on representative maps
- [ ] Verify low-memory behavior

## Phase 15 - Base release
- [ ] Feature freeze base release
- [ ] Complete base map/content validation
- [ ] Complete gameplay regression tests
- [ ] Complete renderer regression tests
- [ ] Complete multiplayer regression tests
- [ ] Crash/error handling pass
- [ ] Save/config migration/versioning
- [ ] Reproducible build process
- [ ] Release packaging for supported platforms
- [ ] Documentation for users and developers
- [ ] Licensing/provenance audit
- [ ] Final unsupported-feature report
- [ ] Base runtime release

## Phase 16 - Post-release updates
- [ ] Cosmetics framework
- [ ] Unlockables/progression framework
- [ ] Player progression data
- [ ] Green balloon-style gear
- [ ] Red apple-style gear
- [ ] Compass-style gear
- [ ] Additional gear/items
- [ ] Additional server/lobby settings
- [ ] Additional maps/content
- [ ] Additional disasters
- [ ] Keep update systems isolated from the base runtime

## Phase 17 - Modding and extensibility
- [ ] Versioned local mod package format
- [ ] Mod manifest/dependency system
- [ ] Content registration API
- [ ] Mod asset loading
- [ ] Sandboxed mod scripts
- [ ] Mod configuration/settings
- [ ] Mod compatibility/version checks
- [ ] Multiplayer mod compatibility validation
- [ ] Mod load order/dependency resolution
- [ ] Developer tooling for mods
- [ ] Documentation and example mods

## Phase 18 - Long-term engine/platform work
- [ ] Generalize runtime beyond NDS-specific systems
- [ ] Expand Roblox-style API compatibility where legally and technically appropriate
- [ ] Improve importer coverage for additional place/model formats
- [ ] Support reusable game packages independent of NDS
- [ ] Improve editor/tooling workflow
- [ ] Optional game creation/editor tooling
- [ ] Additional desktop platforms
- [ ] Additional mobile platforms where practical
- [ ] Improve controller/input abstraction
- [ ] Investigate broader local networking options
- [ ] Preserve stable legacy builds while adding modern targets

## Release rules
- Base release must include the main menu, offline play, local lobby creation/joining, lobby browser, and server settings.
- Offline play must use the same gameplay simulation interfaces intended for local multiplayer.
- LAN multiplayer comes before Bluetooth multiplayer.
- Cosmetics, progression, gear, extra settings, and other post-release additions stay out of the base release.
- iOS 8 is the primary legacy iOS target; iOS 6/CoolBooter is investigated after iOS 8 is stable.
- iPad 4/A6X performance is a first-class constraint, not a later optimization target.
- Source game files remain development-only unless redistribution rights are established.
