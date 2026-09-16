# In-Game Map Editor

The map editor is planned as a native editor mode built on the same scene, part, rendering, input, and map-loader systems used by gameplay.

## Core workflow

1. Create a new map or open an existing `.ndsmap.json` map.
2. Select an existing part or choose a primitive part to place.
3. Move, rotate, and resize the selected part using editor gizmos or numeric fields.
4. Edit name, color, transparency, reflectance, anchored state, and collision state.
5. Duplicate or delete selected parts.
6. Save the edited map back to the project-owned map format.
7. Enter playtest mode without leaving the editor, then return to editing while preserving the map state.

## Selection and manipulation

- Click/tap selection with a visible selection outline.
- Multi-selection should be supported once the basic single-selection workflow is stable.
- Translation, rotation, and scale should use a configurable grid/snap increment.
- Numeric transform fields are required so maps can be edited precisely on small screens.

## Safety and validation

Before saving, the editor should report invalid or suspicious content such as missing spawn locations, zero-sized parts, invalid parent references, excessive part counts, and unsupported classes. Validation should warn rather than silently rewrite user content.

## Compatibility

The first implementation should stay inside the existing native engine architecture so it can run on Windows first and remain portable to the project's legacy iOS and Android targets. Avoid introducing a desktop-only UI dependency into the runtime/editor core.

## Persistence

The editor should use the existing `nds-map` JSON schema and preserve fields that the runtime does not currently edit. Automatic backups should be created before overwriting an existing map.
