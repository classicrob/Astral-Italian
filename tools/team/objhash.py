#!/usr/bin/env python3
"""Hash MWCC ELF objects without their debug information.

`-sym on` objects carry DWARF line tables (.line, .debug and their relocations) and a .comment section,
so moving source lines (e.g. inserting a GALAXY_NATIVE block) changes the object file even when code, data,
relocations and symbols are identical. This hashes every other section (name, type, flags, size and
contents, in order). Usage: objhash.py ROOT > list   (prints "<hash> <path relative to ROOT/..>" per .o)
"""
import hashlib
import os
import struct
import sys

SKIP = {".line", ".debug", ".rela.line", ".rela.debug", ".comment"}


def code_hash(path):
    d = open(path, "rb").read()
    if d[:4] != b"\x7fELF" or d[4] != 1 or d[5] != 2:
        raise ValueError(f"{path}: not an ELF32 big-endian object")
    shoff, = struct.unpack(">I", d[0x20:0x24])
    shentsize, shnum, shstrndx = struct.unpack(">HHH", d[0x2E:0x34])
    headers = [struct.unpack(">10I", d[shoff + i * shentsize:shoff + i * shentsize + 40]) for i in range(shnum)]
    stroff = headers[shstrndx][4]
    h = hashlib.sha1()
    for name_off, typ, flags, addr, off, size, link, info, align, entsize in headers:
        name = d[stroff + name_off:d.index(b"\0", stroff + name_off)].decode()
        if name in SKIP:
            continue
        h.update(f"{name}:{typ}:{flags}:{size}:{link}:{info}:{entsize}\n".encode())
        if typ != 8:  # SHT_NOBITS
            h.update(d[off:off + size])
    return h.hexdigest()


def main():
    root = sys.argv[1]
    base = os.path.dirname(os.path.abspath(root))
    rows = []
    for dirpath, _, names in os.walk(root):
        for n in names:
            if n.endswith(".o"):
                p = os.path.join(dirpath, n)
                rows.append((os.path.relpath(p, base), code_hash(p)))
    for rel, h in sorted(rows):
        print(h, rel)


if __name__ == "__main__":
    main()
