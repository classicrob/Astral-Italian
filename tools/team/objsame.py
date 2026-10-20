#!/usr/bin/env python3
"""Compare MWCC ELF objects semantically: same code, data, symbols and relocations, ignoring the
numbering of compiler-local labels.

objhash.py hashes .symtab/.strtab/.rela.* verbatim, so a header-only refactor that adds declarations
(for example replacing a duplicate class body with an #include of the canonical header) can change the
object hash without changing anything the linker produces: MWCC renumbers its local labels (@1234
literal labels, .dwarf.* labels) and the symbol table order, and relocations then point at different
symbol indices. This tool compares:
  - every section occurrence in file order, keyed by name + occurrence number (MWCC emits several sections
    with one name, e.g. one .text per weak function): type, flags, size, alignment and bytes;
  - named (non-compiler-local) symbols: name, binding/type, visibility (st_other), section, value and size;
  - every relocation outside debug sections, grouped by the section occurrence it applies to: offset, type,
    addend and its target resolved to a symbol name, or for compiler-local labels and section symbols to
    (section occurrence, value).
Debug information (.line, .debug, .comment and their relocations) is ignored, as in objhash.py.

Usage:
  objsame.py A.o B.o               exit 0 if equivalent; prints the differing parts otherwise
  objsame.py --tree DIR_A DIR_B    compares every .o present in either tree (paths relative to the dir)
"""
import os
import re
import struct
import sys

SKIP = {".line", ".debug", ".rela.line", ".rela.debug", ".comment"}
LOCAL_LABEL = re.compile(r"^(@\d+|@stringBase\d*|\.dwarf\..*|\.line\..*|\.\.\..*|lbl_[0-9A-Fa-f]{8})$")
SHT_SYMTAB, SHT_RELA, SHT_NOBITS, SHT_STRTAB = 2, 4, 8, 3


def read_object(path):
    d = open(path, "rb").read()
    if d[:4] != b"\x7fELF" or d[4] != 1 or d[5] != 2:
        raise ValueError(f"{path}: not an ELF32 big-endian object")
    shoff, = struct.unpack(">I", d[0x20:0x24])
    shentsize, shnum, shstrndx = struct.unpack(">HHH", d[0x2E:0x34])
    headers = [struct.unpack(">10I", d[shoff + i * shentsize:shoff + i * shentsize + 40]) for i in range(shnum)]
    stroff = headers[shstrndx][4]

    def cstr(base, offset):
        end = d.index(b"\0", base + offset)
        return d[base + offset:end].decode("latin-1")

    sections = []
    for h in headers:
        name_off, typ, flags, addr, off, size, link, info, align, entsize = h
        sections.append({"name": cstr(stroff, name_off), "type": typ, "flags": flags, "size": size, "link": link,
                         "info": info, "entsize": entsize, "align": align,
                         "data": b"" if typ == SHT_NOBITS else d[off:off + size]})
    symbols = []
    for s in sections:
        if s["type"] == SHT_SYMTAB:
            strtab = headers[s["link"]][4]
            for i in range(len(s["data"]) // 16):
                name_off, value, size, info, other, shndx = struct.unpack(">IIIBBH", s["data"][i * 16:i * 16 + 16])
                symbols.append({"name": cstr(strtab, name_off), "value": value, "size": size, "info": info,
                                "other": other, "shndx": shndx})
            break
    return sections, symbols


def section_keys(sections):
    """A stable key per section index: its name plus its occurrence number among sections of that name.
    MWCC objects can hold several sections with one name (e.g. one .text per weak/template function),
    so the name alone is not an identity."""
    keys, seen = [], {}
    for s in sections:
        n = seen.get(s["name"], 0)
        seen[s["name"]] = n + 1
        keys.append(f"{s['name']}#{n}")
    return keys


def section_name(sections, index, keys=None):
    if index == 0:
        return "UND"
    if index >= 0xFF00:
        return f"special{index:#x}"
    if index >= len(sections):
        return f"bad{index}"
    return keys[index] if keys is not None else sections[index]["name"]


def symbol_key(sections, sym, keys):
    """A stable identity for a relocation target (local labels: section occurrence + offset)."""
    if sym["name"] == "" or LOCAL_LABEL.match(sym["name"]) or (sym["info"] & 0xF) == 3:  # STT_SECTION
        return ("local", section_name(sections, sym["shndx"], keys), sym["value"])
    return ("sym", sym["name"])


def summarize(path):
    sections, symbols = read_object(path)
    keys = section_keys(sections)
    out = {}
    for index, s in enumerate(sections):
        if s["name"] in SKIP or s["type"] in (SHT_SYMTAB, SHT_STRTAB, SHT_RELA) or s["name"] == "":
            continue
        out["section " + keys[index]] = (s["type"], s["flags"], s["size"], s["align"], s["data"])
    named = sorted((sym["name"], sym["info"], sym["other"], section_name(sections, sym["shndx"], keys), sym["value"], sym["size"])
                   for sym in symbols
                   if sym["name"] and not LOCAL_LABEL.match(sym["name"]) and section_name(sections, sym["shndx"]) not in SKIP)
    out["named symbols"] = tuple(named)
    for index, s in enumerate(sections):
        if s["type"] != SHT_RELA or s["name"] in SKIP:
            continue
        target = section_name(sections, s["info"], keys)  # the section these relocations apply to
        if target.split("#")[0] in SKIP:
            continue
        rows = []
        for i in range(len(s["data"]) // 12):
            offset, info, addend = struct.unpack(">IIi", s["data"][i * 12:i * 12 + 12])
            sym = symbols[info >> 8] if (info >> 8) < len(symbols) else {"name": f"badsym{info >> 8}", "info": 0,
                                                                          "shndx": 0, "value": 0, "size": 0}
            rows.append((offset, info & 0xFF, addend, symbol_key(sections, sym, keys)))
        key = "relocations for " + target
        out[key] = out.get(key, ()) + tuple(sorted(rows))
    return out


def compare(a, b):
    sa, sb = summarize(a), summarize(b)
    return sorted(k for k in set(sa) | set(sb) if sa.get(k) != sb.get(k))


def objects(root):
    found = {}
    for dirpath, _, names in os.walk(root):
        for n in names:
            if n.endswith(".o"):
                p = os.path.join(dirpath, n)
                found[os.path.relpath(p, root)] = p
    return found


def main(argv):
    if len(argv) == 4 and argv[1] == "--tree":
        a, b = objects(argv[2]), objects(argv[3])
        bad = 0
        for rel in sorted(set(a) | set(b)):
            if rel not in a or rel not in b:
                print(f"MISSING {rel} ({'only in ' + argv[3] if rel in b else 'only in ' + argv[2]})")
                bad += 1
                continue
            diff = compare(a[rel], b[rel])
            if diff:
                print(f"DIFF {rel}: {', '.join(diff)}")
                bad += 1
        print(f"objsame: {len(set(a) | set(b))} objects, {bad} differ")
        return 1 if bad else 0
    if len(argv) == 3:
        diff = compare(argv[1], argv[2])
        if diff:
            print("DIFF: " + ", ".join(diff))
            return 1
        print("objsame: equivalent")
        return 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv))
