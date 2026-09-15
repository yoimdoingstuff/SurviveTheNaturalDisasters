import json
import subprocess
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path


class NdsPackageTests(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.tmp = Path(self.temp_dir.name)

    def tearDown(self):
        self.temp_dir.cleanup()

    def run_tool(self, *args, check=True):
        script = Path(__file__).with_name("nds_package.py")
        return subprocess.run([sys.executable, str(script), *args], capture_output=True, text=True, check=check)

    def make_map(self):
        path = self.tmp / "map.json"
        path.write_text(json.dumps({"format": "nds-map", "version": 1, "instances": []}), encoding="utf-8")
        return path

    def test_pack_and_unpack_with_assets(self):
        source = self.make_map()
        assets = self.tmp / "assets"
        (assets / "meshes").mkdir(parents=True)
        (assets / "textures").mkdir(parents=True)
        (assets / "meshes" / "island.ndsmesh").write_text("mesh", encoding="utf-8")
        (assets / "textures" / "island.ndstex").write_bytes(b"texture")
        package = self.tmp / "island.ndsmap"
        extracted = self.tmp / "extracted"

        self.run_tool("pack", str(source), "--output", str(package), "--asset-dir", str(assets))
        with zipfile.ZipFile(package) as archive:
            self.assertEqual(sorted(archive.namelist()), ["assets/meshes/island.ndsmesh", "assets/textures/island.ndstex", "map.json", "package.json"])
            manifest = json.loads(archive.read("package.json"))
            self.assertEqual(manifest["format"], "nds-map-package")
            self.assertEqual(manifest["asset_count"], 2)

        self.run_tool("unpack", str(package), "--output", str(extracted))
        self.assertEqual((extracted / "map.json").read_text(encoding="utf-8"), source.read_text(encoding="utf-8"))
        self.assertEqual((extracted / "assets" / "textures" / "island.ndstex").read_bytes(), b"texture")

    def test_rejects_unsafe_paths(self):
        package = self.tmp / "bad.ndsmap"
        with zipfile.ZipFile(package, "w") as archive:
            archive.writestr("package.json", json.dumps({"format": "nds-map-package", "version": 1, "map": "map.json"}))
            archive.writestr("map.json", "{}")
            archive.writestr("../escape.txt", "no")
        output = self.tmp / "out"
        result = self.run_tool("unpack", str(package), "--output", str(output), check=False)
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse((self.tmp / "escape.txt").exists())

    def test_rejects_wrong_map_format(self):
        source = self.tmp / "not_map.json"
        source.write_text(json.dumps({"format": "something-else"}), encoding="utf-8")
        result = self.run_tool("pack", str(source), "--output", str(self.tmp / "bad.ndsmap"), check=False)
        self.assertNotEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main()
