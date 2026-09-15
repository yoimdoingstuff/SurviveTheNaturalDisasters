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

The importer is intentionally repeatable: re-running it against the same source replaces the generated package. Source files remain outside generated/build output and are never copied automatically into distributable releases.

The importer is a development tool, not a guarantee that every historical Roblox property or asset format is supported. Unsupported data is preserved where possible as instance properties and reported for later compatibility work.
