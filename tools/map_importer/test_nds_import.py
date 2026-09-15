import json
import subprocess
import sys
from pathlib import Path


def test_xml_import(tmp_path: Path):
    source = tmp_path / "place.rbxlx"
    output = tmp_path / "map.json"
    source.write_text('''<roblox version="4"><Item class="DataModel"><Properties><string name="Name">Place</string></Properties><Item class="Workspace"><Properties><string name="Name">Workspace</string></Properties><Item class="Part"><Properties><string name="Name">Island</string><Vector3 name="Position"><X>1</X><Y>2</Y><Z>3</Z></Vector3><Vector3 name="Size"><X>10</X><Y>2</Y><Z>8</Z></Vector3><float name="Transparency">0.25</float><bool name="Anchored">true</bool></Properties></Item></Item></Item></roblox>''', encoding="utf-8")
    script = Path(__file__).with_name("nds_import.py")
    subprocess.run([sys.executable, str(script), "import", str(source), "--output", str(output)], check=True)
    package = json.loads(output.read_text(encoding="utf-8"))
    assert package["format"] == "nds-map"
    assert package["summary"]["instance_count"] == 3
    island = package["instances"][2]
    assert island["name"] == "Island"
    assert island["transform"]["position"] == [1.0, 2.0, 3.0]
    assert island["transform"]["size"] == [10.0, 2.0, 8.0]
    assert island["part"]["transparency"] == 0.25
    assert island["part"]["anchored"] is True


def test_scan_detects_binary(tmp_path: Path):
    (tmp_path / "map.rbxl").write_bytes(b"binary")
    (tmp_path / "map.rbxlx").write_text("<roblox />", encoding="utf-8")
    script = Path(__file__).with_name("nds_import.py")
    result = subprocess.run([sys.executable, str(script), "scan", str(tmp_path)], capture_output=True, text=True, check=True)
    report = json.loads(result.stdout)
    assert report["counts"]["binary_detected"] == 1
    assert report["counts"]["xml_supported"] == 1
