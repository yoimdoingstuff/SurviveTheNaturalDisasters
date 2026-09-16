# tools/

Offline development/import/packaging utilities.

Nothing in this directory should require Roblox online services at runtime.

## Early map importer

`map_importer/nds_import.py` is a dependency-free first pass for locally obtained Roblox place/model files. It deliberately starts with the XML formats (`.rbxlx` / `.rbxmx`) so the importer can provide useful map data without pulling a large legacy parser into the runtime.

Suggested commands:

```text
python tools/map_importer/nds_import.py scan <source>
python tools/map_importer/nds_import.py import <source.rbxlx> --output data/maps/example/map.json
```

`scan` discovers `.rbxlx`, `.rbxmx`, `.rbxl`, and `.rbxm` files and reports binary formats as detected-but-not-yet-importable. `import` converts supported XML instances into the project-owned `nds-map` JSON format and records unsupported classes instead of failing the whole import.

## Roblox avatar and accessory importer

`avatar_importer/nds_avatar_import.py` converts locally exported Roblox avatar/model XML (`.rbxmx` / `.rbxlx`) into the project-owned `nds-avatar` JSON package. It recognizes avatar models, Humanoids, body parts, Accessories, MeshParts, CharacterMesh/SpecialMesh data, clothing asset references, and mesh/texture dependencies. Accessories remain explicitly represented so the runtime can attach them to the character rather than treating them as unrelated map parts.

Suggested command:

```text
python tools/avatar_importer/nds_avatar_import.py my_avatar.rbxmx --output data/avatars/my_avatar.ndsavatar.json
```

Binary `.rbxm` / `.rbxl` avatar files are detected but are not decoded by this dependency-free importer yet. Exporting the model as XML `.rbxmx` is currently the supported route. Asset downloading is deliberately not performed by the tool, keeping imports deterministic and offline.

The importer is intentionally repeatable: re-running it against the same source replaces the generated package. Source files remain outside generated/build output and are never copied automatically into distributable releases.

These importers are development tools, not guarantees that every historical Roblox property or asset format is supported. Unsupported data is preserved where possible as instance properties and can be reported for later compatibility work.
