#!/usr/bin/env python3
"""Create and extract portable .ndsmap sharing packages."""
from __future__ import annotations

import argparse
import json
import sys
import zipfile
from pathlib import Path, PurePosixPath

PACKAGE_FORMAT = "nds-map-package"
PACKAGE_VERSION = 1
MAP_NAME = "map.json"


def _safe_zip_name(name: str) -> str:
    path = PurePosixPath(name)
    if path.is_absolute() or ".." in path.parts:
        raise ValueError(f"unsafe package path: {name}")
    return str(path)


def pack(map_path: Path, output: Path, asset_dir: Path | None = None) -> int:
    try:
        package = json.loads(map_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"error: invalid map JSON: {exc}", file=sys.stderr)
        return 1
    if package.get("format") != "nds-map":
        print("error: input is not an nds-map package", file=sys.stderr)
        return 1

    output.parent.mkdir(parents=True, exist_ok=True)
    files = [(MAP_NAME, map_path)]
    if asset_dir:
        if not asset_dir.is_dir():
            print(f"error: asset directory does not exist: {asset_dir}", file=sys.stderr)
            return 2
        for path in sorted(p for p in asset_dir.rglob("*") if p.is_file()):
            relative = path.relative_to(asset_dir).as_posix()
            files.append(("assets/" + _safe_zip_name(relative), path))

    manifest = {
        "format": PACKAGE_FORMAT,
        "version": PACKAGE_VERSION,
        "map": MAP_NAME,
        "asset_count": sum(1 for name, _ in files if name.startswith("assets/")),
    }
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        archive.writestr("package.json", json.dumps(manifest, indent=2) + "\n")
        for name, source in files:
            archive.write(source, name)
    print(f"Packed map to {output} ({len(files) - 1} bundled assets)")
    return 0


def unpack(package_path: Path, output_dir: Path) -> int:
    try:
        with zipfile.ZipFile(package_path, "r") as archive:
            names = archive.namelist()
            if "package.json" not in names or MAP_NAME not in names:
                print("error: not a valid nds-map sharing package", file=sys.stderr)
                return 1
            manifest = json.loads(archive.read("package.json").decode("utf-8"))
            if manifest.get("format") != PACKAGE_FORMAT or manifest.get("version") != PACKAGE_VERSION:
                print("error: unsupported nds-map package version", file=sys.stderr)
                return 1
            for name in names:
                safe_name = _safe_zip_name(name)
                if safe_name != name:
                    print(f"error: unsafe package path: {name}", file=sys.stderr)
                    return 1
            output_dir.mkdir(parents=True, exist_ok=True)
            for name in names:
                target = output_dir / name
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(archive.read(name))
    except (OSError, zipfile.BadZipFile, json.JSONDecodeError, UnicodeDecodeError) as exc:
        print(f"error: could not read map package: {exc}", file=sys.stderr)
        return 1
    print(f"Unpacked map package to {output_dir}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Pack/extract portable .ndsmap sharing packages.")
    sub = parser.add_subparsers(dest="command", required=True)
    ppack = sub.add_parser("pack", help="bundle a map and optional converted assets")
    ppack.add_argument("map", type=Path)
    ppack.add_argument("--output", type=Path, required=True)
    ppack.add_argument("--asset-dir", type=Path)
    punpack = sub.add_parser("unpack", help="extract a shared map package")
    punpack.add_argument("package", type=Path)
    punpack.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    source = args.map if args.command == "pack" else args.package
    if not source.exists():
        print(f"error: source does not exist: {source}", file=sys.stderr)
        return 2
    if args.command == "pack":
        return pack(args.map, args.output, args.asset_dir)
    return unpack(args.package, args.output)


if __name__ == "__main__":
    raise SystemExit(main())
