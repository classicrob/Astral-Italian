import shutil
import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import scan_bundle as sb

APP = Path("/Users/roberthaisfield/MacioGalaxy2-archive/MacioGalaxy2-play/build5.2.1-preservation-7cea/contents/app/Petari.app")


class Bundle(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name) / "Astral Italian.app" / "Contents"
        (self.root / "Resources").mkdir(parents=True)
        (self.root / "Info.plist").write_bytes(b"<plist/>")

    def put(self, rel, data):
        p = self.root / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_bytes(data)

    def problems(self, **kw):
        return sb.scan(self.root.parent, **kw)[0]

    def test_clean(self):
        self.put("Resources/notes.txt", b"hello")
        self.put("Resources/seed.db", b"SQLite format 3\x00" + bytes(100))
        self.assertEqual(self.problems(), [])

    def test_extension_and_directory(self):
        self.put("Resources/Font.arc", b"abc")
        self.put("Resources/files/x.txt", b"abc")
        found = " ".join(self.problems())
        self.assertIn(".arc", found)
        self.assertIn("files", found)

    def test_magic_in_renamed_file(self):
        self.put("Resources/a.txt", b"Yaz0" + bytes(100))
        self.put("Resources/b.txt", bytes(0x18) + b"\x5d\x1c\x9e\xa3" + bytes(100))
        self.put("Resources/c.png", b"xx SB4E01 xx")
        found = self.problems()
        self.assertEqual(len(found), 3, found)

    def test_unknown_type_and_bad_db(self):
        self.put("Resources/x.weird", b"1")
        self.put("Resources/seed.db", b"not sqlite")
        self.assertEqual(len(self.problems()), 2)

    def test_total_size(self):
        self.put("Resources/a.txt", bytes(2 * 1048576))
        self.assertEqual(len(self.problems(max_total_mb=1)), 1)

    @unittest.skipUnless(APP.is_dir(), "preserved build not present")
    def test_unbundled_app_is_rejected(self):
        shutil.copytree(APP / "Contents" / "MacOS", self.root / "MacOS")
        found = " ".join(self.problems(min_macos="14.0"))
        self.assertIn("/opt/homebrew", found)
        self.assertIn("minos", found)


if __name__ == "__main__":
    unittest.main()
