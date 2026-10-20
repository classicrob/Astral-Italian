#!/usr/bin/env python3
"""Regression tests for objsame.py (python3 -m unittest tools/team/test_objsame.py).

Synthetic ELF32 big-endian objects with two `.text` sections (as MWCC emits for weak functions): a byte
change in EITHER occurrence, a changed alignment, or a relocation retargeted in the second occurrence must
be reported; renumbering a compiler-local label must not. Also the objects lane's negative control on a real
MWCC object (2026-10-20): flip one byte at the first .text+3 of build/SB4E01/src/Game/Enemy/Snakehead.o
(5 .text sections), skipped when that object is not built.
"""
import os
import shutil
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import objsame  # noqa: E402


def build_elf(path, text1=b"\x60\x00\x00\x00", text2=b"\x4e\x80\x00\x20", label="@100", reloc_target=1,
              align2=4, func_other=0):
    """Sections: 0 null, 1 .text, 2 .text, 3 .rela.text (for section 2), 4 .symtab, 5 .strtab, 6 .shstrtab.
    Symbols: 0 null, 1 local label in section 1 at 0, 2 global `func` in section 2 at 0."""
    shstr = b"\0.text\0.rela.text\0.symtab\0.strtab\0.shstrtab\0"
    name = {n: shstr.index(n.encode() + b"\0") for n in (".text", ".rela.text", ".symtab", ".strtab", ".shstrtab")}
    strtab = b"\0" + label.encode() + b"\0func\0"
    label_off, func_off = 1, 2 + len(label)
    symtab = (b"\0" * 16
              + struct.pack(">IIIBBH", label_off, 0, 4, 0x00, 0, 1)    # local NOTYPE in .text#0
              + struct.pack(">IIIBBH", func_off, 0, 4, 0x12, func_other, 2))  # global FUNC in .text#1
    rela = struct.pack(">IIi", 0, (reloc_target << 8) | 10, 0)        # R_PPC_REL24 at .text#1+0
    blobs = [b"", text1, text2, rela, symtab, strtab, shstr]
    offsets, data = [], b""
    base = 0x34
    for blob in blobs:
        offsets.append(base + len(data))
        data += blob
        data += b"\0" * (-len(data) % 4)
    shoff = base + len(data)
    headers = [
        (0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
        (name[".text"], 1, 6, 0, offsets[1], len(text1), 0, 0, 4, 0),
        (name[".text"], 1, 6, 0, offsets[2], len(text2), 0, 0, align2, 0),
        (name[".rela.text"], 4, 0, 0, offsets[3], len(rela), 4, 2, 4, 12),
        (name[".symtab"], 2, 0, 0, offsets[4], len(symtab), 5, 2, 4, 16),
        (name[".strtab"], 3, 0, 0, offsets[5], len(strtab), 0, 0, 1, 0),
        (name[".shstrtab"], 3, 0, 0, offsets[6], len(shstr), 0, 0, 1, 0),
    ]
    ehdr = (b"\x7fELF\x01\x02\x01" + b"\0" * 9
            + struct.pack(">HHIIIIIHHHHHH", 1, 20, 1, 0, 0, shoff, 0, 0x34, 0, 0, 40, len(headers), 6))
    with open(path, "wb") as f:
        f.write(ehdr + data + b"".join(struct.pack(">10I", *h) for h in headers))


class ObjsameTests(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp()
        self.base = os.path.join(self.dir, "base.o")
        build_elf(self.base)

    def tearDown(self):
        shutil.rmtree(self.dir)

    def variant(self, **kwargs):
        path = os.path.join(self.dir, "variant.o")
        build_elf(path, **kwargs)
        return objsame.compare(self.base, path)

    def test_identical(self):
        self.assertEqual(self.variant(), [])

    def test_local_label_renumbering_is_equivalent(self):
        self.assertEqual(self.variant(label="@987"), [])

    def test_byte_change_in_first_text_occurrence(self):
        self.assertIn("section .text#0", self.variant(text1=b"\x60\x00\x00\x01"))

    def test_byte_change_in_second_text_occurrence(self):
        self.assertIn("section .text#1", self.variant(text2=b"\x4e\x80\x00\x21"))

    def test_alignment_change(self):
        self.assertIn("section .text#1", self.variant(align2=8))

    def test_relocation_retarget_in_second_occurrence(self):
        self.assertIn("relocations for .text#1", self.variant(reloc_target=2))

    def test_symbol_visibility_change(self):
        self.assertIn("named symbols", self.variant(func_other=2))  # STV_HIDDEN

    def test_real_mwcc_object_first_text_flip(self):
        root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
        real = os.path.join(root, "build", "SB4E01", "src", "Game", "Enemy", "Snakehead.o")
        if not os.path.exists(real):
            self.skipTest("Snakehead.o not built")
        sections, _ = objsame.read_object(real)
        self.assertGreater(sum(1 for s in sections if s["name"] == ".text"), 1, "fixture needs several .text sections")
        data = bytearray(open(real, "rb").read())
        shoff, = struct.unpack(">I", data[0x20:0x24])
        es, n, si = struct.unpack(">HHH", data[0x2E:0x34])
        first_text = next(i for i, s in enumerate(sections) if s["name"] == ".text" and s["size"] > 3)
        offset = struct.unpack(">I", data[shoff + first_text * es + 16:shoff + first_text * es + 20])[0]
        data[offset + 3] ^= 0x01
        flipped = os.path.join(self.dir, "Snakehead.flipped.o")
        open(flipped, "wb").write(bytes(data))
        self.assertEqual(objsame.compare(real, real), [])
        self.assertIn(f"section .text#{sum(1 for s in sections[:first_text] if s['name'] == '.text')}",
                      objsame.compare(real, flipped))


if __name__ == "__main__":
    unittest.main()
