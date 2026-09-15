# Map Sharing

Maps can be shared as a single `.ndsmap` file. The format is a ZIP container designed to keep sharing simple while leaving the native runtime independent of ZIP handling on legacy platforms.

## Package layout

```text
my_map.ndsmap
  package.json
  map.json
  assets/
    meshes/
    textures/
    ...
```

`map.json` is the normal project-owned `nds-map` document. Converted runtime assets can be bundled under `assets/` without including the original source `.rbxl/.rbxlx` files.

## Create a package

```text
python tools/map_importer/nds_package.py pack map.json --output my_map.ndsmap --asset-dir content/assets
```

The asset directory is optional. A map with no converted assets can still be shared as a package while development conversion is incomplete.

## Extract a package

```text
python tools/map_importer/nds_package.py unpack my_map.ndsmap --output extracted_map
```

The extractor validates the package manifest and rejects unsafe paths before writing files. This matters because apparently even map sharing requires us to defend against tiny ZIP gremlins.

## Distribution rule

Only bundle content the project is legally allowed to redistribute. Locally obtained proprietary source maps, Roblox binaries, or source assets should stay outside shared packages unless redistribution rights have been established.

## Future UI integration

The runtime/editor should eventually expose `Import Map` and `Export Map` actions so users can share one `.ndsmap` file without touching the command line. The same package format should work across Windows, Android, and iOS once platform file/share integration exists.
