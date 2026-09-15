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
- [~] OpenGL ES 2 renderer <!-- backend lifecycle + first Windows OpenGL vertical slice; true GLES2 shader/VBO path still pending -->
- [ ] Shader/material system compatible with GLES2
- [ ] Texture loading
- [ ] Mesh loading
- [ ] Basic lighting
- [x] Camera <!-- backend-neutral camera matrices added -->
- [ ] Frustum culling
- [~] Transparency <!-- Part transparency reaches renderer; alpha blending policy still pending -->
- [ ] UI rendering
- [ ] A6X performance test scene
- [ ] iOS 8 renderer validation
- [ ] Android GLES2 renderer validation
- [~] Windows OpenGL renderer validation <!-- native WGL context and first Part cube draw added; local runtime validation still required -->
- [ ] iOS 6 renderer compatibility test

## Phase 3 - Scene/Instance model

- [x] DataModel root (class supported by Instance tree)
- [x] Instance base type
- [x] Parent/child tree
- [x] Name lookup
- [x] Folder
- [x] Model
- [x] Part (scene class + position/size/rotation/visual/physics property storage)
- [x] Spawn point (scene class)
- [x] Camera object (scene class)
- [x] Character object (scene class)
- [x] Player object (scene class)
- [ ] Attributes/values needed by game scripts
- [ ] Scene serialization

## Phase 4 - Physics and characters

- [ ] Select legacy-compatible physics backend
- [ ] Integrate physics backend
- [ ] Static collision
- [ ] Dynamic rigid bodies
- [ ] Gravity
- [ ] Impulses
- [ ] Triggers/contact callbacks
- [ ] Character controller
- [ ] Humanoid-like health/state system
- [ ] Basic walking/jumping
- [ ] Respawn
- [ ] Disaster debris interaction
- [ ] Physics stress test on A6X

## Phase 5 - Script runtime

- [ ] Embed Luau or compatible Lua runtime
- [ ] Script loader
- [ ] Module loading
- [ ] Scheduler/timers
- [ ] Event/signals API
- [ ] Core Instance bindings
- [ ] Workspace binding
- [ ] Players binding
- [ ] ReplicatedStorage-like shared data binding
- [ ] ServerStorage-like local data binding if required
- [ ] RunService-like frame/tick events
- [ ] Tween support required by gameplay/UI
- [ ] Debris-style timed cleanup
- [ ] Input bindings
- [ ] Minimal compatibility layer for target scripts

## Phase 6 - Base game content

- [ ] Map format/package
- [ ] Base map loading
- [ ] Spawn/round setup
- [ ] Disaster framework
- [ ] Random disaster selection
- [ ] Round timer
- [ ] Intermission
- [ ] Survival detection
- [ ] Death/respawn flow
- [ ] Winner/survivor state
- [ ] Camera behavior
- [ ] Basic sound/music
- [ ] Disaster-specific effects
- [ ] Base UI/HUD
- [ ] End-of-round results

## Phase 7 - Menu and lobby (BASE RELEASE, BEFORE UPDATES)

- [ ] Boot splash/loading screen
- [ ] Main menu
- [ ] Play Offline button
- [ ] Create Local Lobby flow
- [ ] Join Local Lobby flow
- [ ] Lobby browser/discovery for LAN
- [ ] Lobby player list
- [ ] Ready/start flow
- [ ] Server/lobby settings screen
- [ ] Player limit setting
- [ ] Map/disaster pool settings
- [ ] Host-only controls
- [ ] Settings persistence
- [ ] Back/navigation flow
- [ ] Touch-friendly iPad UI
- [ ] Controller/input abstraction for future external controllers

## Phase 8 - Offline single-player complete

- [ ] Full game loop works with network disabled
- [ ] No server dependency in offline mode
- [ ] All base maps/disasters tested
- [ ] Save/load local settings
- [ ] Audio and UI pass
- [ ] Memory budget pass
- [ ] Startup time pass
- [ ] Crash/recovery handling
- [ ] iPad 4 sustained gameplay test

## Phase 9 - Local multiplayer

- [ ] LocalLoopbackTransport
- [ ] Host simulation authority
- [ ] LAN discovery
- [ ] LAN transport
- [ ] Client connection flow
- [ ] Player synchronization
- [ ] Physics synchronization strategy
- [ ] Disaster state synchronization
- [ ] Lobby synchronization
- [ ] Disconnect handling
- [ ] Host shutdown handling
- [ ] Reconnect handling where practical
- [ ] Optional Bluetooth transport investigation
- [ ] Multiplayer stress test with legacy devices

## Phase 10 - Cross-platform release validation

- [ ] Windows native release build
- [ ] Android 4.0+ release build
- [ ] Shared save/config compatibility test across Windows/Android/iOS
- [ ] Shared network protocol compatibility test across Windows/Android/iOS
- [ ] Cross-platform controller/input abstraction test

## Phase 11 - iOS 8 release

- [ ] iOS 8 packaging
- [ ] Touch controls polish
- [ ] Legacy GPU compatibility test
- [ ] Memory usage profiling
- [ ] Battery/thermal profiling
- [ ] Install/launch verification
- [ ] Offline launch verification
- [ ] Local multiplayer verification

## Phase 12 - iOS 6 / CoolBooter build

- [ ] Identify minimum compiler/API requirements
- [ ] Replace unavailable APIs
- [ ] iOS 6-compatible asset/resource paths
- [ ] GLES2 validation
- [ ] Input validation
- [ ] Audio validation
- [ ] Memory stress test
- [ ] CoolBooter installation test
- [ ] Offline gameplay test
- [ ] Local multiplayer test if supported

## Phase 13 - Base release lock

- [ ] Feature freeze
- [ ] Performance freeze
- [ ] Bug triage
- [ ] Save/config migration format locked
- [ ] Base release documentation
- [ ] Reproducible build documented
- [ ] Base release tagged

# POST-COMPLETION UPDATES

These must not block the base release.

## Update 1 - Cosmetics and progression

- [ ] Cosmetic item database
- [ ] Unlock state model
- [ ] Basic cosmetic inventory
- [ ] Equip/unequip UI
- [ ] Player appearance hooks
- [ ] Non-pay-to-win progression model
- [ ] Cosmetic save migration

## Update 2 - Gear

- [ ] Gear framework
- [ ] Green balloon-style item
- [ ] Red apple-style consumable
- [ ] Compass-style utility item
- [ ] Inventory/equipment UI
- [ ] Gear networking synchronization
- [ ] Gear balance pass

## Update 3 - More lobby/server settings

- [ ] Advanced disaster rules
- [ ] Custom map rotation
- [ ] Friendly/fire/team-related toggles if supported
- [ ] Respawn options
- [ ] Round timing presets
- [ ] Host moderation controls

## Update 4 - More content

- [ ] Additional maps
- [ ] Additional disasters
- [ ] More audio
- [ ] More environmental effects
- [ ] Additional cosmetic categories

## Future / research

- [ ] Better host migration
- [ ] Bluetooth transport
- [ ] Other legacy iOS devices
- [ ] Modern desktop build improvements
- [ ] Optional controller support
- [ ] Community-authored map/content format

# Blocked items

When an item becomes blocked, annotate it with:

`Blocked by:` reason

`Reasoning:` why the dependency is required and what workaround was considered

## POST-COMPLETION UPDATE 2 - Mod support

Mod support is an extension of the finished runtime and must not destabilize the base release.

### Mod loader and format

- [ ] Define versioned mod package format
- [ ] Define `mod.json` manifest
- [ ] Define mod IDs, names, versions, authors, and dependencies
- [ ] Define minimum/maximum runtime compatibility fields
- [ ] Define optional platform compatibility fields
- [ ] Implement local mod discovery
- [ ] Implement enable/disable mod list
- [ ] Implement mod load order and deterministic dependency resolution
- [ ] Implement safe failure handling for broken mods
- [ ] Implement mod configuration files

### Content extensions

- [ ] Allow mods to add maps
- [ ] Allow mods to add disasters
- [ ] Allow mods to add sounds/music
- [ ] Allow mods to add textures/materials/models
- [ ] Allow mods to add UI definitions
- [ ] Allow mods to add cosmetics
- [ ] Allow mods to add gear/items
- [ ] Allow mods to add gameplay configuration

### Script extensions

- [ ] Define sandboxed mod script API
- [ ] Expose only explicitly approved engine/game APIs
- [ ] Prevent unrestricted native code loading
- [ ] Add script time/instruction safeguards where practical
- [ ] Add mod API versioning
- [ ] Add multiplayer compatibility validation

### Multiplayer

- [ ] Include mod manifest/hash in lobby handshake
- [ ] Prevent incompatible mod sets from joining accidentally
- [ ] Host-controlled mod requirements
- [ ] LAN mod transfer/discovery investigation
- [ ] Deterministic content validation for gameplay-affecting mods
- [ ] Document trusted-host assumptions for local multiplayer

### Legacy support

- [ ] Test mod loader on Windows
- [ ] Test mod loader on Android 4.0+
- [ ] Test mod loader on iOS 8 where filesystem rules permit
- [ ] Test lightweight mod packages on iPad 4
- [ ] Ensure mods cannot force unsupported graphics features on legacy devices
