import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


class NdsImporterTests(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.tmp_path = Path(self.temp_dir.name)

    def tearDown(self):
        self.temp_dir.cleanup()

    def run_importer(self, *args):
        script = Path(__file__).with_name("nds_import.py")
        return subprocess.run(
            [sys.executable, str(script), *args],
            capture_output=True,
            text=True,
            check=True,
        )

    def test_xml_import(self):
        source = self.tmp_path / "place.rbxlx"
        output = self.tmp_path / "map.json"
        source.write_text(
            '<roblox version="4"><Item class="DataModel"><Properties><string name="Name">Place</string></Properties>'
            '<Item class="Workspace"><Properties><string name="Name">Workspace</string></Properties>'
            '<Item class="Part"><Properties><string name="Name">Island</string>'
            '<Vector3 name="Position"><X>1</X><Y>2</Y><Z>3</Z></Vector3>'
            '<Vector3 name="Size"><X>10</X><Y>2</Y><Z>8</Z></Vector3>'
            '<float name="Transparency">0.25</float><bool name="Anchored">true</bool>'
            '</Properties></Item></Item></Item></roblox>',
            encoding="utf-8",
        )

        self.run_importer("import", str(source), "--output", str(output))
        package = json.loads(output.read_text(encoding="utf-8"))

        self.assertEqual(package["format"], "nds-map")
        self.assertEqual(package["summary"]["instance_count"], 3)
        island = package["instances"][2]
        self.assertEqual(island["name"], "Island")
        self.assertEqual(island["transform"]["position"], [1.0, 2.0, 3.0])
        self.assertEqual(island["transform"]["size"], [10.0, 2.0, 8.0])
        self.assertEqual(island["part"]["transparency"], 0.25)
        self.assertIs(island["part"]["anchored"], True)

    def test_mesh_and_texture_references_are_preserved(self):
        source = self.tmp_path / "mesh.rbxlx"
        output = self.tmp_path / "mesh.json"
        source.write_text(
            '<roblox version="4"><Item class="DataModel"><Properties><string name="Name">Place</string></Properties>'
            '<Item class="MeshPart"><Properties><string name="Name">IslandMesh</string>'
            '<Content name="MeshId"><url>rbxassetid://123</url></Content>'
            '<Content name="TextureID"><url>rbxassetid://456</url></Content>'
            '</Properties></Item></Item></roblox>',
            encoding="utf-8",
        )

        self.run_importer("import", str(source), "--output", str(output))
        package = json.loads(output.read_text(encoding="utf-8"))
        mesh = package["instances"][1]
        self.assertEqual(mesh["geometry"], {"type": "mesh", "mesh": "rbxassetid://123"})
        self.assertEqual(mesh["material"]["texture"], "rbxassetid://456")

    def test_scan_detects_binary(self):
        (self.tmp_path / "map.rbxl").write_bytes(b"binary")
        (self.tmp_path / "map.rbxlx").write_text("<roblox />", encoding="utf-8")

        result = self.run_importer("scan", str(self.tmp_path))
        report = json.loads(result.stdout)
        self.assertEqual(report["counts"]["binary_detected"], 1)
        self.assertEqual(report["counts"]["xml_supported"], 1)


if __name__ == "__main__":
    unittest.main()
