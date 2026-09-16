import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


class AvatarImporterTests(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.tmp = Path(self.temp_dir.name)

    def tearDown(self):
        self.temp_dir.cleanup()

    def run_import(self, source, output):
        script = Path(__file__).with_name("nds_avatar_import.py")
        return subprocess.run([sys.executable, str(script), str(source), "--output", str(output)], capture_output=True, text=True, check=True)

    def test_avatar_and_accessory_import(self):
        source = self.tmp / "avatar.rbxmx"
        output = self.tmp / "avatar.ndsavatar.json"
        source.write_text(
            '<roblox version="4">'
            '<Item class="Model"><Properties><string name="Name">MyAvatar</string></Properties>'
            '<Item class="Humanoid"><Properties><string name="Name">Humanoid</string></Properties></Item>'
            '<Item class="Part"><Properties><string name="Name">Head</string><Color3 name="Color"><R>1</R><G>0.8</G><B>0.6</B></Color3></Properties></Item>'
            '<Item class="Accessory"><Properties><string name="Name">CoolHat</string></Properties>'
            '<Item class="MeshPart"><Properties><string name="Name">Handle</string>'
            '<Content name="MeshId"><url>rbxassetid://123</url></Content>'
            '<Content name="TextureID"><url>rbxassetid://456</url></Content>'
            '</Properties></Item></Item>'
            '</Item></roblox>', encoding="utf-8")
        self.run_import(source, output)
        package = json.loads(output.read_text(encoding="utf-8"))
        self.assertEqual(package["format"], "nds-avatar")
        self.assertEqual(package["avatar"]["name"], "MyAvatar")
        self.assertEqual(package["avatar"]["accessories"], [{"id": 3, "name": "CoolHat"}])
        self.assertEqual(package["assets"], {"meshes": ["rbxassetid://123"], "textures": ["rbxassetid://456"]})

    def test_binary_avatar_is_rejected(self):
        source = self.tmp / "avatar.rbxm"
        output = self.tmp / "avatar.json"
        source.write_bytes(b"binary")
        result = subprocess.run([sys.executable, str(Path(__file__).with_name("nds_avatar_import.py")), str(source), "--output", str(output)], capture_output=True, text=True)
        self.assertEqual(result.returncode, 3)
        self.assertIn("export as XML", result.stderr)


if __name__ == "__main__":
    unittest.main()
