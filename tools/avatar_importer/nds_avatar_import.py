#!/usr/bin/env python3
"""Import Roblox XML avatar/accessory models into a portable NDS avatar package."""
from __future__ import annotations
import argparse, json, sys
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Any

XML_FORMATS = {".rbxmx", ".rbxlx"}


def tag(node: ET.Element) -> str:
    return node.tag.rsplit("}", 1)[-1]


def scalar(text: str) -> Any:
    text = text.strip()
    if text.lower() in {"true", "false"}:
        return text.lower() == "true"
    try:
        return float(text) if any(c in text for c in ".eE") else int(text)
    except ValueError:
        return text


def properties(item: ET.Element) -> dict[str, Any]:
    result: dict[str, Any] = {}
    node = next((c for c in item if tag(c) == "Properties"), None)
    if node is None:
        return result
    for prop in node:
        name = prop.attrib.get("name")
        if not name:
            continue
        children = {tag(c): (c.text or "").strip() for c in prop}
        if "url" in children:
            result[name] = children["url"]
        elif set(children) >= {"X", "Y", "Z"}:
            try:
                result[name] = [float(children["X"]), float(children["Y"]), float(children["Z"])]
            except ValueError:
                result[name] = prop.text or ""
        elif set(children) >= {"R", "G", "B"}:
            try:
                result[name] = [float(children["R"]), float(children["G"]), float(children["B"])]
            except ValueError:
                result[name] = prop.text or ""
        else:
            result[name] = scalar(prop.text or "") if tag(prop) in {"bool", "int", "int64", "float", "double", "token"} else (prop.text or "")
    return result


def walk(root: ET.Element) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []

    def visit(container: ET.Element, parent: int | None) -> None:
        for item in container:
            if tag(item) != "Item":
                continue
            props = properties(item)
            index = len(result)
            result.append({
                "id": index,
                "class": item.attrib.get("class", "Unknown"),
                "name": props.get("Name", item.attrib.get("class", "Unknown")),
                "parent": parent,
                "properties": props,
            })
            visit(item, index)

    visit(root, None)
    return result


def asset_ref(value: Any) -> str | None:
    if not isinstance(value, str) or not value.strip():
        return None
    return value.strip()


def build_package(source: Path) -> dict[str, Any]:
    try:
        root = ET.parse(source).getroot()
    except ET.ParseError as exc:
        raise ValueError(f"invalid XML: {exc}") from exc

    instances = walk(root)
    accessories = []
    body_parts = []
    meshes: set[str] = set()
    textures: set[str] = set()

    for entry in instances:
        cls = entry["class"]
        props = entry["properties"]
        if cls == "Accessory":
            accessories.append({"id": entry["id"], "name": entry["name"]})
        if cls in {"Part", "MeshPart", "CharacterMesh", "SpecialMesh"}:
            body_parts.append({"id": entry["id"], "class": cls, "name": entry["name"]})
        for key in ("MeshId", "MeshID", "TextureID", "TextureId", "Texture", "ShirtTemplate", "PantsTemplate", "Graphic"):
            ref = asset_ref(props.get(key))
            if ref:
                if "Mesh" in key:
                    meshes.add(ref)
                else:
                    textures.add(ref)

    return {
        "format": "nds-avatar",
        "version": 1,
        "source": {"filename": source.name, "extension": source.suffix.lower()},
        "avatar": {
            "name": next((i["name"] for i in instances if i["class"] in {"Model", "Humanoid"}), source.stem),
            "accessories": accessories,
            "body_parts": body_parts,
        },
        "assets": {"meshes": sorted(meshes), "textures": sorted(textures)},
        "instances": instances,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Import Roblox .rbxmx/.rbxlx avatar and accessory models.")
    parser.add_argument("source", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if not args.source.exists():
        print(f"error: source does not exist: {args.source}", file=sys.stderr)
        return 2
    if args.source.suffix.lower() not in XML_FORMATS:
        print("error: binary .rbxm/.rbxl avatar import is not implemented yet; export as XML .rbxmx first", file=sys.stderr)
        return 3
    try:
        package = build_package(args.source)
    except (OSError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(package, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"Imported avatar {package['avatar']['name']} with {len(package['avatar']['accessories'])} accessories to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
