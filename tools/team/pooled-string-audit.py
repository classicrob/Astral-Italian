#!/usr/bin/env python3
"""Compare printable call arguments in existing SB4E01 target/source objects.

Usage: pooled-string-audit.py PREFIX [--tree DIR] [--mismatches-only]

This is a read-only candidate finder; it does not compile objects. It traces
relocated addresses through straight-line lis/addi/mr instructions and compares
the bytes at call arguments, rather than comparing literal-pool base addresses.
Only functions with the same sequence of direct callees are compared.

Results need a call-site review: the tracer does not follow control flow or know
callee argument types. Printable bytes can be a table, code, or an unused register,
and differently ordered branches can put equivalent calls in different positions.
Missing arguments and unaligned call sequences are not evidence of equality.
"""

import argparse
from collections import Counter
from dataclasses import dataclass
import json
from pathlib import Path
import re
import struct


VERSION = "SB4E01"
CAUTIONS = [
    "Uses existing objects; source/header changes may require a unit rebuild.",
    "Straight-line tracing does not follow branches or indirect calls.",
    "Printable bytes may be non-string data or unused arguments; review the callee signature.",
    "Only aligned direct-call sequences and arguments resolved on both sides are compared.",
]


@dataclass(frozen=True)
class Symbol:
    name: str
    address: int
    size: int
    section: int


@dataclass(frozen=True)
class Relocation:
    kind: int
    symbol: Symbol
    addend: int


class Dol:
    def __init__(self, path):
        self.data = path.read_bytes()
        if len(self.data) < 0x100:
            raise ValueError(f"Not a DOL header: {path}")
        self.sections = [
            (
                struct.unpack_from(">I", self.data, i * 4)[0],
                struct.unpack_from(">I", self.data, 0x48 + i * 4)[0],
                struct.unpack_from(">I", self.data, 0x90 + i * 4)[0],
            )
            for i in range(18)
        ]

    def read(self, address, size):
        for offset, start, extent in self.sections:
            if start <= address and address + size <= start + extent:
                file_offset = offset + address - start
                if file_offset + size <= len(self.data):
                    return self.data[file_offset:file_offset + size]
        return None


def cstring(data, offset):
    end = data.find(b"\0", offset)
    if end < 0:
        raise ValueError("Unterminated ELF name")
    return data[offset:end].decode(errors="replace")


class Elf:
    def __init__(self, path):
        self.data = path.read_bytes()
        if self.data[:6] != b"\x7fELF\x01\x02":
            raise ValueError(f"Expected a big-endian ELF32 object: {path}")
        header = struct.unpack_from(">16sHHIIIIIHHHHHH", self.data)
        section_offset, section_stride, section_count = header[6], header[11], header[12]
        self.sections = [
            struct.unpack_from(">IIIIIIIIII", self.data, section_offset + i * section_stride)
            for i in range(section_count)
        ]
        self.contents = [self.data[s[4]:s[4] + s[5]] for s in self.sections]
        section_strings = self.contents[header[13]]
        self.section_names = [cstring(section_strings, s[0]) for s in self.sections]
        self.functions = {}
        self.relocations = {}
        symbol_tables = {}
        for index, section in enumerate(self.sections):
            if section[1] != 2:  # SHT_SYMTAB
                continue
            strings = self.contents[section[6]]
            symbols = []
            for offset in range(0, section[5], section[9]):
                name, address, size, info, _, owner = struct.unpack_from(
                    ">IIIBBH", self.contents[index], offset
                )
                symbol = Symbol(cstring(strings, name), address, size, owner)
                symbols.append(symbol)
                if info & 15 == 2 and size and 0 < owner < section_count:
                    self.functions[symbol.name] = symbol
            symbol_tables[index] = symbols
        for index, section in enumerate(self.sections):
            if section[1] != 4:  # SHT_RELA
                continue
            symbols = symbol_tables[section[6]]
            for offset in range(0, section[5], section[9]):
                address, info, addend = struct.unpack_from(">IIi", self.contents[index], offset)
                self.relocations[(section[7], address & ~3)] = Relocation(
                    info & 255, symbols[info >> 8], addend
                )


class Resolver:
    def __init__(self, tree):
        # Use the same target image as the original scanner; no link/build occurs.
        self.dol = Dol(tree / f"build/{VERSION}/main.dol")
        self.symbols = {}
        for line in (tree / f"config/{VERSION}/symbols.txt").read_text().splitlines():
            match = re.match(r"(.+?) = \.\w+:0x([0-9A-Fa-f]+);", line)
            if match:
                self.symbols[match[1]] = int(match[2], 16)
        self.section_bases = {}
        unit = None
        for line in (tree / f"config/{VERSION}/splits.txt").read_text().splitlines():
            if line and not line[0].isspace() and line.endswith(":"):
                unit = line[:-1].removesuffix(".cpp").removesuffix(".c")
            match = re.search(r"^\s+(\.\w+)\s+start:0x([0-9A-Fa-f]+)", line)
            if match:
                self.section_bases[(unit, match[1])] = int(match[2], 16)

    def read(self, elf, relocation, size, unit, target, delta=0):
        symbol = relocation.symbol
        addend = relocation.addend + delta
        local = 0 < symbol.section < len(elf.sections)
        if target:
            address = self.symbols.get(symbol.name)
            if address is None:
                match = re.fullmatch(r"(?:lbl_|fn_)([0-9A-Fa-f]{8})", symbol.name)
                if match:
                    address = int(match[1], 16)
            if address is None and local:
                base = self.section_bases.get((unit, elf.section_names[symbol.section]))
                if base is not None:
                    address = base + symbol.address
            return None if address is None else self.dol.read(address + addend, size)
        if local:
            # SHT_NOBITS has no file bytes; sh_offset does not identify its value.
            if elf.sections[symbol.section][1] == 8:
                return None
            offset = symbol.address + addend
            data = elf.contents[symbol.section]
            if 0 <= offset and offset + size <= len(data):
                return data[offset:offset + size]
            return None
        address = self.symbols.get(symbol.name)
        return None if address is None else self.dol.read(address + addend, size)

    def string(self, elf, pointer, unit, target):
        if pointer is None:
            return None
        relocation, delta = pointer
        result = bytearray()
        for index in range(256):
            byte = self.read(elf, relocation, 1, unit, target, delta + index)
            if byte is None:
                return None
            if byte == b"\0":
                break
            result.extend(byte)
        else:
            return None
        if not result or any(byte < 32 or byte == 127 for byte in result):
            return None
        return bytes(result).hex()


def trace_calls(elf, function, unit, target, resolver):
    registers = {}
    calls = []
    for offset in range(0, function.size, 4):
        address = function.address + offset
        word = struct.unpack_from(">I", elf.contents[function.section], address)[0]
        opcode = word >> 26
        rt, ra, rb = (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31
        immediate = word & 65535
        if immediate & 32768:
            immediate -= 65536
        relocation = elf.relocations.get((function.section, address))
        if relocation and relocation.kind == 10:  # R_PPC_REL24, direct call/tail call
            arguments = {
                str(reg): value
                for reg in range(3, 11)
                if (value := resolver.string(elf, registers.get(reg), unit, target)) is not None
            }
            calls.append((offset, relocation.symbol.name, arguments))
            for reg in [0] + list(range(3, 13)):
                registers.pop(reg, None)
        elif opcode in (14, 15):  # addi/li, addis/lis
            if relocation:
                registers[rt] = (relocation, 0)
            elif opcode == 14 and ra and ra in registers:
                base, delta = registers[ra]
                registers[rt] = (base, delta + immediate)
            else:
                registers.pop(rt, None)
        elif opcode == 31 and ((word >> 1) & 1023) == 444:  # or/mr
            if rt == rb and rt in registers:
                registers[ra] = registers[rt]
            else:
                registers.pop(ra, None)
        elif opcode in (32, 33, 34, 35, 40, 41, 42, 43, 46, 7, 8, 12, 13):
            registers.pop(rt, None)
        elif opcode in (24, 25, 26, 27, 28, 29):
            registers.pop(ra, None)
        elif opcode == 31:
            registers.pop(rt, None)
    return calls


def audit(tree, prefix, selected_units=None):
    resolver = Resolver(tree)
    stats = Counter()
    rows = []
    units = json.loads((tree / "objdiff.json").read_text())["units"]
    for config in units:
        unit = config["name"].removeprefix("main/")
        if not unit.startswith(prefix) or not config.get("base_path"):
            continue
        if selected_units is not None and unit not in selected_units:
            continue
        target_path, source_path = tree / config["target_path"], tree / config["base_path"]
        if not target_path.exists() or not source_path.exists():
            continue
        target, source = Elf(target_path), Elf(source_path)
        for name, target_function in target.functions.items():
            source_function = source.functions.get(name)
            if source_function is None:
                continue
            target_calls = trace_calls(target, target_function, unit, True, resolver)
            source_calls = trace_calls(source, source_function, unit, False, resolver)
            if [call[1] for call in target_calls] != [call[1] for call in source_calls]:
                stats["call_sequence_unaligned"] += 1
                continue
            for target_call, source_call in zip(target_calls, source_calls):
                for register, target_hex in target_call[2].items():
                    source_hex = source_call[2].get(register)
                    if source_hex is None:
                        continue
                    stats["compared"] += 1
                    rows.append({
                        "unit": unit,
                        "function": name,
                        "target_site": hex(target_call[0]),
                        "ours_site": hex(source_call[0]),
                        "call": target_call[1],
                        "arg_register": register,
                        "target": bytes.fromhex(target_hex).decode("shift_jis", errors="replace"),
                        "ours": bytes.fromhex(source_hex).decode("shift_jis", errors="replace"),
                        "target_hex": target_hex,
                        "ours_hex": source_hex,
                        "equal": target_hex == source_hex,
                    })
    return {
        "stats": dict(stats),
        "comparisons": rows,
        "mismatches": [row for row in rows if not row["equal"]],
        "cautions": CAUTIONS,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("prefix", help="Unit prefix, for example Game/MapObj/")
    parser.add_argument("--tree", type=Path, default=Path.cwd(), help="Checkout root (default: current directory)")
    parser.add_argument("--mismatches-only", action="store_true", help="Omit the complete comparison list from JSON")
    parser.add_argument("--units-file", type=Path, help="Optional newline-separated unit allowlist; # comments allowed")
    args = parser.parse_args()
    try:
        selected = None
        if args.units_file:
            selected = {
                line.split("#", 1)[0].strip().removeprefix("main/")
                for line in args.units_file.read_text().splitlines()
                if line.split("#", 1)[0].strip()
            }
        result = audit(args.tree.resolve(), args.prefix.removeprefix("main/"), selected)
        if args.mismatches_only:
            result.pop("comparisons")
        print(json.dumps(result, indent=2))
    except (OSError, ValueError, KeyError, IndexError, struct.error) as error:
        parser.exit(1, f"pooled-string-audit: {error}\n")


if __name__ == "__main__":
    main()
