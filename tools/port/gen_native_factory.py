#!/usr/bin/env python3
"""Generate src/Game/NameObj/NameObjFactoryNative.cpp (GALAXY_NATIVE only) from the SB4E01 asm.

The Wii TU (NameObjFactory.cpp) declares Wii-sized stand-in classes so its createNameObj<T>
instantiations match without ~480 headers; natively those would under-allocate, so the native
build gets its own cCreateTable / cPostCreationTable built from the REAL class headers.
Classes without a usable real header get a named trap creator (PLACEHOLDER(port)).

  python3 tools/port/gen_native_factory.py [--exclude CLASS ...]   (needs build/SB4E01/asm)
Iterate with tools/team/nativecheck.py; classes whose real header does not compile natively
can be passed with --exclude (they become traps too).
"""
import argparse, re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ASM = ROOT / "build/SB4E01/asm"
OUT = ROOT / "src/Game/NameObj/NameObjFactoryNative.cpp"


def parse_objs():
    """label -> bytes for every .obj in the asm tree that holds string-ish data."""
    objs = {}
    for p in ASM.rglob("*.s"):
        txt = p.read_text(errors="replace")
        for m in re.finditer(r"^\.obj (\S+), \w+\n(.*?)^\.endobj", txt, re.S | re.M):
            data = bytearray()
            ok = True
            for line in m.group(2).splitlines():
                line = line.split(" # ", 1)[0].strip()
                if line.startswith(".string "):
                    s = line[len(".string "):].strip()[1:-1]
                    data += bytes(s, "latin-1").decode("unicode_escape").encode("latin-1") + b"\0"
                elif line.startswith(".4byte 0x"):
                    try:
                        data += b"".join(int(v, 16).to_bytes(4, "big") for v in line[7:].split(","))
                    except ValueError:
                        ok = False
                        break
                elif line.startswith(".2byte 0x"):
                    for v in line[7:].split(","):
                        data += int(v, 16).to_bytes(2, "big")
                elif line.startswith(".byte 0x"):
                    for v in line[6:].split(","):
                        data += int(v, 16).to_bytes(1, "big")
                elif line:
                    ok = False
                    break
            if ok:
                objs[m.group(1)] = bytes(data)
    return objs


def c_string(raw):
    raw = raw.split(b"\0", 1)[0]
    text = raw.decode("cp932")
    return '"' + text.replace("\\", "\\\\").replace('"', '\\"') + '"'


def demangle_creator(sym):
    m = re.match(r'"?(create\w+)<(\d+)(\w+)>__14NameObjFactoryFPCc_P7NameObj"?$', sym)
    if m and len(m.group(3)) == int(m.group(2)):
        return ("template", m.group(1), m.group(3))
    m = re.match(r"(\w+)__2MRFPCc$", sym)
    if m:
        return ("mr", m.group(1), None)
    m = re.match(r"(\w+?)__(\d+)(\w+)F(v|RC12JMapInfoIter)$", sym)
    if m and len(m.group(3)) >= int(m.group(2)):
        scope = m.group(3)[: int(m.group(2))]
        return ("post", f"{scope}::{m.group(1)}", m.group(4) == "v")
    raise SystemExit(f"unknown creator {sym}")


def table(txt, name):
    m = re.search(rf"^\.obj {name}__14NameObjFactory, global\n(.*?)^\.endobj", txt, re.S | re.M)
    words = [l.strip()[len(".4byte "):] for l in m.group(1).splitlines() if l.strip()]
    return list(zip(words[0::2], words[1::2]))


def header_index():
    """class name -> header (relative include path) that DEFINES it (class X ... {)."""
    idx = {}
    for p in sorted((ROOT / "include").rglob("*.hpp")):
        txt = p.read_text(errors="replace")
        for m in re.finditer(r"^\s*class\s+(\w+)\s*(?::[^;{]*)?\{", txt, re.M):
            idx.setdefault(m.group(1), str(p.relative_to(ROOT / "include")))
    return idx


def mr_header(fn):
    for p in sorted((ROOT / "include").rglob("*.hpp")):
        if re.search(rf"\b{fn}\s*\(", p.read_text(errors="replace")):
            return str(p.relative_to(ROOT / "include"))
    return None


def ctor_ok(header, cls, kind):
    txt = (ROOT / "include" / header).read_text(errors="replace")
    if kind == "createNameObj":
        return re.search(rf"\b{cls}\s*\(\s*const\s+char\s*\*\s*\w*\s*(=[^,)]*)?\)", txt) is not None
    return re.search(rf"\b{cls}\s*\(\s*(int|s32|u32)\s*\w*\s*,\s*const\s+char\s*\*\s*\w*\s*\)", txt) is not None


# Same special constructions as the Wii TU's explicit createNameObj<T> specializations.
SPECIAL = {
    "Togepin": "new Togepin(pName, nullptr)",
    "PoleObstacleParts": "new PoleObstacleParts(pName, 0, nullptr)",
    "MorphItemNeoBee": 'new MorphItemNeoBee("ハチオブジェ")',
    "MorphItemNeoFire": 'new MorphItemNeoFire("ファイアオブジェ")',
    "MorphItemNeoHopper": 'new MorphItemNeoHopper("ホッパーオブジェ")',
    "MorphItemNeoTeresa": 'new MorphItemNeoTeresa("テレサオブジェ")',
}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--exclude", nargs="*", default=[])
    a = ap.parse_args()
    txt = (ASM / "Game/NameObj/NameObjFactory.s").read_text()
    objs = parse_objs()
    create = table(txt, "cCreateTable")
    post = table(txt, "cPostCreationTable")
    hdr = header_index()
    forms = {"createCenterOriginCube": 1, "createBaseOriginCube": 0, "createSphere": 2, "createBaseOriginCylinder": 3}

    includes, traps, rows, missing = set(), {}, [], []
    for label, sym in create:
        name = "nullptr" if label == "0" else c_string(objs[label])
        kind, fn, cls = demangle_creator(sym)
        if kind == "mr":
            h = mr_header(fn)
            if not h:
                raise SystemExit(f"no header declares MR::{fn}")
            includes.add(h)
            ret = re.search(rf"(\w+)\s*\*\s*{fn}\s*\(", (ROOT / "include" / h).read_text(errors="replace"))
            if ret and ret.group(1) != "NameObj":  # e.g. Coin* MR::createDirectSetCoin: upcast explicitly
                rows.append((name, f"[](const char* pName) -> NameObj* {{ return MR::{fn}(pName); }}"))
            else:
                rows.append((name, f"MR::{fn}"))
            continue
        h = hdr.get(cls)
        if h and cls not in a.exclude and (cls in SPECIAL or ctor_ok(h, cls, fn)):
            includes.add(h)
            rows.append((name, f"{fn}< {cls} >"))
        else:
            traps[cls] = "no real header" if not h else ("ctor not declared" if cls not in a.exclude else "header does not compile natively")
            rows.append((name, f"trap{cls}"))
    postrows = []
    post_traps = set()
    for label, sym in post:
        kind, fn, noargs = demangle_creator(sym)
        scope, base = fn.split("::")
        h = mr_header(base) if scope == "MR" else hdr.get(scope)
        if not h:
            raise SystemExit(f"no header for {fn}")
        includes.add(h)
        hdrtxt = (ROOT / "include" / h).read_text(errors="replace")
        member = scope != "MR" and not re.search(rf"static\s+void\s+{base}\s*\(", hdrtxt)
        if member:  # declared non-static although the original calls it without an object
            post_traps.add(fn)
            call = f'[](const JMapInfoIter&) {{ trapMissingPostCreation("{fn}"); }}'
        else:
            call = f"[](const JMapInfoIter&) {{ {fn}(); }}" if noargs else fn
        postrows.append((c_string(objs[label]), call))

    o = ["// Generated by tools/port/gen_native_factory.py from the SB4E01 asm (cCreateTable/cPostCreationTable).",
         "// Native-only: the Wii TU NameObjFactory.cpp uses Wii-sized stand-in classes, which would under-allocate",
         "// on the host, so the native tables construct the real classes from their headers.",
         "#ifdef GALAXY_NATIVE",
         '#include "Game/NameObj/NameObjFactory.hpp"']
    o += [f'#include "{h}"' for h in sorted(includes)]
    o += ["#include <cstdio>", "#include <cstdlib>", "",
          "// createNameObj<T> (new T(pName)) comes from NameObjFactory.hpp's unnamed namespace.",
          "namespace {"]
    for k, f in forms.items():
        o += ["", "    template < typename T >", f"    NameObj* {k}(const char* pName) {{",
              f"        return new T({f}, pName);", "    }"]
    for cls, expr in SPECIAL.items():
        o += ["", f"    template <> NameObj* createNameObj< {cls} >(const char* pName) {{ return {expr}; }}"]
    o += ["", "    [[noreturn]] void trapMissingClass(const char* pClass, const char* pName) {",
          '        std::fprintf(stderr, "NameObjFactory: class %s (object \\"%s\\") has no native header yet\\n", pClass, pName);',
          "        std::abort();", "    }", "",
          "    [[noreturn]] void trapMissingPostCreation(const char* pFunction) {",
          '        std::fprintf(stderr, "NameObjFactory: post-creation %s is not callable natively yet\\n", pFunction);',
          "        std::abort();", "    }", ""]
    for fn in sorted(post_traps):
        o.append(f"    // PLACEHOLDER(port): {fn} is declared non-static but called without an object; resolve when it is made static (Wii symbol-table proof pending)")
    for cls, why in sorted(traps.items()):
        o.append(f"    // PLACEHOLDER(port): {cls}: {why}; resolve when its owner lane sources the class header")
        o.append(f'    NameObj* trap{cls}(const char* pName) {{ trapMissingClass("{cls}", pName); }}')
    o += ["}  // namespace", "", "namespace NameObjFactory {",
          f"    extern const Name2CreateFunc cCreateTable[{len(rows)}];",
          f"    const Name2CreateFunc cCreateTable[{len(rows)}] = {{"]
    o += [f"        {{{n}, {f}}}," for n, f in rows]
    o += ["    };", "", "    struct PostCreationEntry {", "        const char* mName;",
          "        void (*mFunction)(const JMapInfoIter&);", "    };",
          f"    extern const PostCreationEntry cPostCreationTable[{len(postrows)}];",
          f"    const PostCreationEntry cPostCreationTable[{len(postrows)}] = {{"]
    o += [f"        {{{n}, {f}}}," for n, f in postrows]
    o += ["    };", "}  // namespace NameObjFactory", "#endif", ""]
    OUT.write_text("\n".join(o))
    print(f"{len(rows)} create entries ({len(rows) - sum(1 for r in rows if r[1].startswith('trap'))} real, "
          f"{len(traps)} trap classes), {len(postrows)} post-creation entries -> {OUT.relative_to(ROOT)}")
    for cls, why in sorted(traps.items()):
        print(f"  trap {cls}: {why}")


if __name__ == "__main__":
    main()
