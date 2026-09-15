#!/usr/bin/env python3
"""Small, dependency-free Roblox place importer for development.

This importer intentionally targets XML place/model files (.rbxlx/.rbxmx)
first. Binary .rbxl/.rbxm files are discovered and reported as unsupported
rather than guessed at. The output is a project-owned JSON map description
that the native runtime can consume later.

Usage:
    python nds_import.py scan <source> [--output report.json]
    python nds_import.py import <source> --output <map.json>

Source files are treated as local development inputs and are never copied
into the repository automatically.
"""

from __future__ import annotations

import argparse
import json
import math
import os
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Any

SUPPORTED_XML = {".rbxlx", ".rbxmx"}
KNOWN_BINARY = {".rbxl", ".rbxm"}


def clean_value(value: str) -> Any:
    value = value.strip()
    if value in {"true", "false"}:
        return value == "true"
    try:
        if value.lower().startswith(("0x", "-0x")):
            return int(value, 16)
        if "." in value or "e" in value.lower():
            return float(value)
        return int(value)
    except ValueError:
        return value


def parse_vector(value: str, keys: tuple[str, ...]) -> list[float] | None:
    parts = value.replace(",", " ").split()
    if len(parts) != len(keys):
        return None
    try:
        return [float(part) for part in parts]
    except ValueError:
        return None


def parse_properties(item: ET.Element) -> dict[str, Any]:
    properties: dict[str, Any] = {}
    for child in item:
        tag = child.tag.split("}")[-1]
        name = child.attrib.get("name")
        if not name:
            continue
        text = child.text or ""
        if tag in {"Vector3", "CoordinateFrame"}:
            parsed = parse_vector(text, ("x", "y", "z"))
            properties[name] = parsed if parsed is not None else text
        elif tag in {"Vector2"}:
            parsed = parse_vector(text, ("x", "y"))
            properties[name] = parsed if parsed is not None else text
        elif tag == "bool":
            properties[name] = text.strip().lower() == "true"
        elif tag in {"int", "int64", "float", "double", "token"}:
            properties[name] = clean_value(text)
        elif tag in {"string", "Content", "Ref", "ProtectedString"}:
            properties[name] = text
        else:
            properties[name] = text
    return properties


def walk_items(root: ET.Element) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []

    def visit(node: ET.Element, parent: int | None) -> None:
        for item in node:
            if item.tag.split("}")[-1] != "Item":
                continue
            class_name = item.attrib.get("class", "Unknown")
            referent = item.attrib.get("referent")
            properties = parse_properties(item)
            index = len(result)
            result.append({
                "id": index,
                "referent": referent,
                "class": class_name,
                "name": properties.get("Name", class_name),
                "parent": parent,
                "properties": properties,
                "children": [],
            })
            if parent is not None:
                result[parent]["children"].append(index)
            for child in item:
                if child.tag.split("}")[-1] in {"Item", "Properties"}:
                    visit(child, index)

    visit(root, None)
    return result


def normalize_instances(instances: list[dict[str, Any]]) -> None:
    """Add engine-friendly transform hints without changing source values."""
    for instance in instances:
        props = instance["properties"]
        pos = props.get("Position")
        if isinstance(pos, list) and len(pos) == 3:
            instance["transform"] = {"position": pos}
        elif instance["class"] in {"Part", "SpawnLocation", "WedgePart", "CornerWedgePart", "TrussPart"}:
            instance["transform"] = {"position": [0.0, 0.0, 0.0]}


def import_xml(source: Path, destination: Path) -> dict[str, Any]:
    try:
        tree = ET.parse(source)
    except ET.ParseError as exc:
        raise ValueError(f"Invalid XML place/model: {exc}") from exc

    instances = walk_items(tree.getroot())
    normalize_instances(instances)

    classes: dict[str, int] = {}
    unsupported: list[str] = []
    for instance in instances:
        cls = instance["class"]
        classes[cls] = classes.get(cls, 0) + 1
        if cls not in {
            "DataModel", "Workspace", "Folder", "Model", "Part", "SpawnLocation",
            "WedgePart", "CornerWedgePart", "TrussPart", "Camera", "Terrain",
            "Script", "LocalScript", "ModuleScript", "Sound", "PointLight",
            "SpotLight", "SurfaceLight", "Decal", "Texture", "Sky", "Humanoid",
            "BodyColors", "CharacterMesh", "MeshPart", "SpecialMesh",
        }:
            if cls not in unsupported:
                unsupported.append(cls)

    package = {
        "format": "nds-map",
        "version": 1,
        "source": {
            "filename": source.name,
            "extension": source.suffix.lower(),
        },
        "importer": {
            "name": "nds_import",
            "version": "0.1",
        },
        "summary": {
            "instance_count": len(instances),
            "classes": classes,
            "unsupported_classes": unsupported,
        },
        "instances": instances,
    }

    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(json.dumps(package, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    return package


def scan(source: Path) -> dict[str, Any]:
    if source.is_file():
        files = [source]
    else:
        files = sorted(path for path in source.rglob("*") if path.is_file())

    report = {"source": str(source), "files": [], "counts": {"xml_supported": 0, "binary_detected": 0, "other": 0}}
    for path in files:
        suffix = path.suffix.lower()
        kind = "xml_supported" if suffix in SUPPORTED_XML else "binary_detected" if suffix in KNOWN_BINARY else "other"
        report["counts"][kind] += 1
        report["files"].append({"path": str(path), "extension": suffix, "kind": kind, "size": path.stat().st_size})
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description="Inspect/import Roblox XML place and model files.")
    sub = parser.add_subparsers(dest="command", required=True)

    scan_parser = sub.add_parser("scan", help="scan a file or directory")
    scan_parser.add_argument("source", type=Path)
    scan_parser.add_argument("--output", type=Path)

    import_parser = sub.add_parser("import", help="import one .rbxlx/.rbxmx file")
    import_parser.add_argument("source", type=Path)
    import_parser.add_argument("--output", type=Path, required=True)

    args = parser.parse_args()
    if not args.source.exists():
        print(f"error: source does not exist: {args.source}", file=sys.stderr)
        return 2

    if args.command == "scan":
        report = scan(args.source)
        encoded = json.dumps(report, indent=2, ensure_ascii=False) + "\n"
        if args.output:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(encoded, encoding="utf-8")
        else:
            print(encoded, end="")
        return 0

    if args.source.suffix.lower() not in SUPPORTED_XML:
        print("error: binary .rbxl/.rbxm import is not implemented yet; use scan to inspect the source.", file=sys.stderr)
        return 3

    try:
        package = import_xml(args.source, args.output)
    except (OSError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    print(f"Imported {package['summary']['instance_count']} instances to {args.output}")
    if package["summary"]["unsupported_classes"]:
        print("Unsupported classes (preserved as metadata):")
        for name in package["summary"]["unsupported_classes"]:
            print(f"  - {name}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
