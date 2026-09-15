#!/usr/bin/env python3
"""Dependency-free first-pass importer for Roblox XML place/model files."""
from __future__ import annotations
import argparse, json, sys
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Any

XML_FORMATS = {".rbxlx", ".rbxmx"}
BINARY_FORMATS = {".rbxl", ".rbxm"}
SUPPORTED_CLASSES = {
    "DataModel", "Workspace", "Folder", "Model", "Part", "SpawnLocation",
    "WedgePart", "CornerWedgePart", "TrussPart", "Camera", "Terrain",
    "Script", "LocalScript", "ModuleScript", "Sound", "PointLight", "SpotLight",
    "SurfaceLight", "Decal", "Texture", "Sky", "Humanoid", "BodyColors",
    "CharacterMesh", "MeshPart", "SpecialMesh",
}

def tag(node: ET.Element) -> str:
    return node.tag.rsplit("}", 1)[-1]

def scalar(text: str) -> Any:
    text = text.strip()
    if text.lower() in {"true", "false"}: return text.lower() == "true"
    try:
        return float(text) if any(c in text for c in ".eE") else int(text)
    except ValueError:
        return text

def property_value(node: ET.Element) -> Any:
    # Roblox XML vectors are normally encoded as child X/Y/Z elements.
    children = {tag(c): (c.text or "").strip() for c in node}
    if set(children) >= {"X", "Y", "Z"}:
        try: return [float(children["X"]), float(children["Y"]), float(children["Z"])]
        except ValueError: pass
    if set(children) >= {"X", "Y"} and len(children) == 2:
        try: return [float(children["X"]), float(children["Y"])]
        except ValueError: pass
    text = node.text or ""
    return scalar(text) if tag(node) in {"bool", "int", "int64", "float", "double", "token"} else text

def parse_properties(item: ET.Element) -> dict[str, Any]:
    props: dict[str, Any] = {}
    properties = next((c for c in item if tag(c) == "Properties"), None)
    if properties is None: return props
    for node in properties:
        name = node.attrib.get("name")
        if name: props[name] = property_value(node)
    return props

def walk(root: ET.Element) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    def visit(container: ET.Element, parent: int | None) -> None:
        for item in container:
            if tag(item) != "Item": continue
            props = parse_properties(item)
            index = len(result)
            entry = {"id": index, "referent": item.attrib.get("referent"),
                     "class": item.attrib.get("class", "Unknown"),
                     "name": props.get("Name", item.attrib.get("class", "Unknown")),
                     "parent": parent, "properties": props, "children": []}
            result.append(entry)
            if parent is not None: result[parent]["children"].append(index)
            visit(item, index)
    visit(root, None)
    return result

def import_xml(source: Path, destination: Path) -> dict[str, Any]:
    try: root = ET.parse(source).getroot()
    except ET.ParseError as exc: raise ValueError(f"invalid XML: {exc}") from exc
    instances = walk(root)
    unsupported = sorted({i["class"] for i in instances if i["class"] not in SUPPORTED_CLASSES})
    for i in instances:
        position = i["properties"].get("Position")
        if isinstance(position, list) and len(position) == 3:
            i["transform"] = {"position": position}
    classes: dict[str, int] = {}
    for i in instances: classes[i["class"]] = classes.get(i["class"], 0) + 1
    package = {"format": "nds-map", "version": 1,
               "source": {"filename": source.name, "extension": source.suffix.lower()},
               "importer": {"name": "nds_import", "version": "0.2"},
               "summary": {"instance_count": len(instances), "classes": classes,
                           "unsupported_classes": unsupported}, "instances": instances}
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(json.dumps(package, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    return package

def scan(source: Path) -> dict[str, Any]:
    files = [source] if source.is_file() else sorted(p for p in source.rglob("*") if p.is_file())
    report = {"source": str(source), "files": [], "counts": {"xml_supported": 0, "binary_detected": 0, "other": 0}}
    for p in files:
        suffix = p.suffix.lower()
        kind = "xml_supported" if suffix in XML_FORMATS else "binary_detected" if suffix in BINARY_FORMATS else "other"
        report["counts"][kind] += 1
        report["files"].append({"path": str(p), "extension": suffix, "kind": kind, "size": p.stat().st_size})
    return report

def main() -> int:
    parser = argparse.ArgumentParser(description="Inspect/import Roblox XML place and model files.")
    sub = parser.add_subparsers(dest="command", required=True)
    pscan = sub.add_parser("scan"); pscan.add_argument("source", type=Path); pscan.add_argument("--output", type=Path)
    pimp = sub.add_parser("import"); pimp.add_argument("source", type=Path); pimp.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if not args.source.exists(): print(f"error: source does not exist: {args.source}", file=sys.stderr); return 2
    if args.command == "scan":
        text = json.dumps(scan(args.source), indent=2) + "\n"
        if args.output: args.output.parent.mkdir(parents=True, exist_ok=True); args.output.write_text(text, encoding="utf-8")
        else: print(text, end="")
        return 0
    if args.source.suffix.lower() not in XML_FORMATS:
        print("error: binary .rbxl/.rbxm import is not implemented yet; use scan first", file=sys.stderr); return 3
    try: package = import_xml(args.source, args.output)
    except (OSError, ValueError) as exc: print(f"error: {exc}", file=sys.stderr); return 1
    print(f"Imported {package['summary']['instance_count']} instances to {args.output}")
    for name in package["summary"]["unsupported_classes"]: print(f"unsupported class: {name}")
    return 0

if __name__ == "__main__": raise SystemExit(main())
