import json
import subprocess
import sys
from pathlib import Path


def test_xml_import(tmp_path: Path):
    source = tmp_path / "place.rbxlx"
    output = tmp_path / "map.json"
    source.write_text('''<roblox version="4"><Item class="DataModel"><Properties><string name="Name">Place</string></Properties><Item class="Workspace"><Properties><string name="Name">Workspace</string></Properties><Item class="Part"><Properties><string name="Name">Island</string><Vector3 name="Position"><X>1</X><Y>2</Y><Z>3</Z></Vector3></Properties></Item></Item></Item></roblox>''', encoding="utf-8")
    script = Path(__file__).with_name("nds_import.py")
    subprocess.run([sys.executable, str(script), "import", str(source), "--output", str(output)], check=True)
    package = json.loads(output.read_text(encoding="utf-8"))
    assert package["format"] == "nds-map"
    assert package["summary"]["instance_count"] == 3
    assert package["instances"][2]["name"] == "Island"
    assert package["instances"][2]["transform"]["position"] == [1.0, 2.0, 3.0]


def test_scan_detects_binary(tmp_path: Path):
    (tmp_path / "map.rbxl").write_bytes(b"binary")
    (tmp_path / "map.rbxlx").write_text("<roblox />", encoding="utf-8")
    script = Path(__file__).with_name("nds_import.py")
    result = subprocess.run([sys.executable, str(script), "scan", str(tmp_path)], capture_output=True, text=True, check=True)
    report = json.loads(result.stdout)
    assert report["counts"]["binary_detected"] == 1
    assert report["counts"]["xml_supported"] == 1
