#!/usr/bin/env python3
"""Name unnamed target functions (fn_XXXXXXXX) after the identical function in our build.

  namefn.py UNIT [UNIT...] [--write]

SMG2-only functions are often `fn_8020E640` in config/SB4E01/symbols.txt, so objdiff cannot pair them with the
member function we wrote (`Teresa::foo`), and they count as unmatched even when the code is identical.
For each unit: take our functions that have no same-named target symbol, and the target's fn_ symbols; compare
instruction words with relocated fields masked; if exactly one of ours equals a target fn_, rename the fn_ to our
mangled name in symbols.txt (--write). Then run `ninja` (re-split) and the function counts as matched.
Matches ignore relocation targets. Review callee/data identity and source order before using --write;
identical bodies alone cannot distinguish functions whose only differences are relocations.
"""
import os, re, struct, sys

ROOT = os.getcwd()
OD = f"{ROOT}/build/binutils/powerpc-eabi-objdump"
SYMS = f"{ROOT}/config/SB4E01/symbols.txt"


def funcs(obj):
    """Slice ELF function symbols using their section index and declared size."""
    data = open(obj, "rb").read()
    if data[:6] != b"\x7fELF\x01\x02":
        raise ValueError(f"{obj}: expected a big-endian ELF32 object")
    header = struct.unpack_from(">16sHHIIIIIHHHHHH", data)
    sections = [struct.unpack_from(">IIIIIIIIII", data, header[6] + i * header[11])
                for i in range(header[12])]
    contents = [data[s[4]:s[4] + s[5]] for s in sections]
    relocations = {}
    for index, section in enumerate(sections):
        if section[1] != 4:  # SHT_RELA
            continue
        for offset in range(0, section[5], section[9]):
            address, info, addend = struct.unpack_from(">IIi", contents[index], offset)
            relocations[(section[7], address & ~3)] = info & 255
    result, order = {}, {}
    for index, section in enumerate(sections):
        if section[1] != 2:  # SHT_SYMTAB
            continue
        strings = contents[section[6]]
        for offset in range(0, section[5], section[9]):
            name_offset, start, size, info, other, section_index = struct.unpack_from(
                ">IIIBBH", contents[index], offset)
            if info & 15 != 2 or not size or not 0 < section_index < len(sections):
                continue
            name_end = strings.index(b"\0", name_offset)
            name = strings[name_offset:name_end].decode("utf-8", errors="replace")
            body = contents[section_index][start:start + size]
            if len(body) != size or size % 4:
                continue
            sequence = []
            for instruction_offset in range(0, size, 4):
                word = struct.unpack_from(">I", body, instruction_offset)[0]
                relocation = relocations.get((section_index, start + instruction_offset))
                if relocation == 10:  # R_PPC_REL24
                    word &= 0xFC000003
                elif relocation:
                    word &= 0xFFFF0000
                sequence.append(word)
            while sequence and sequence[-1] == 0:
                sequence.pop()
            result[name] = tuple(sequence)
            order[name] = (section_index, start)
    funcs.order = order
    return result


def main():
    write = "--write" in sys.argv
    units = [a for a in sys.argv[1:] if not a.startswith("--")]
    lines = open(SYMS).read().split("\n")
    known = {l.split(" = ")[0] for l in lines if " = " in l}  # never reuse a name that already exists (weak dupes)
    renames = {}
    for u in units:
        u = re.sub(r"\.(cpp|c)$", "", u.removeprefix("src/").removeprefix("main/"))
        t, o = f"{ROOT}/build/SB4E01/obj/{u}.o", f"{ROOT}/build/SB4E01/src/{u}.o"
        if not (os.path.exists(t) and os.path.exists(o)):
            print(f"{u}: missing object"); continue
        T = funcs(t); torder = funcs.order
        O = funcs(o); oorder = funcs.order
        ours = {k: v for k, v in O.items() if k not in T and k not in known and not k.startswith(("@", "lbl_", "gap_"))}
        # group identical bodies; within a group pair target fn_ and ours in address (= source) order
        groups = {}
        for tn, tv in T.items():
            if tn.startswith("fn_"):
                groups.setdefault(tv, [[], []])[0].append(tn)
        for k, v in ours.items():
            if v in groups:
                groups[v][1].append(k)
        for tv, (tns, oks) in groups.items():
            if not oks:
                continue
            if len(tns) != len(oks):
                print(f"{u}: unequal group {tns} vs {oks[:4]}; skipped"); continue
            for tn, ok in zip(sorted(tns, key=lambda n: torder[n]), sorted(oks, key=lambda n: oorder[n])):
                renames[tn] = ok
                print(f"{u}: {tn} -> {ok}")
    if renames:
        print("Review relocation targets and identical-body source order before applying these mappings.")
    if write and renames:
        for i, l in enumerate(lines):
            m = re.match(r"(fn_[0-9A-F]{8}) = ", l)
            if m and m.group(1) in renames:
                lines[i] = renames[m.group(1)] + l[len(m.group(1)):]
        open(SYMS, "w").write("\n".join(lines))
        print(f"renamed {len(renames)} symbols; run ninja")


if __name__ == "__main__":
    main()
