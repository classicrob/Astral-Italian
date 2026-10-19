#!/usr/bin/env python3
"""Advisory PPC constructor store screen; never a correctness certificate."""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict, deque
from dataclasses import dataclass
import hashlib
from pathlib import Path
import re
import time

UNKNOWN = None
MASK32 = 0xFFFFFFFF
STORES = {36: (4, False), 37: (4, True), 38: (1, False), 39: (1, True),
          44: (2, False), 45: (2, True), 52: (4, False), 53: (4, True)}
LOADS = {32: (4, False, False), 33: (4, True, False),
         34: (1, False, False), 35: (1, True, False),
         40: (2, False, False), 41: (2, True, False),
         42: (2, False, True), 43: (2, True, True)}


def signed(value, bits):
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


def integer(value):
    return ("int", value & MASK32)


def add(value, amount):
    if value is None:
        return UNKNOWN
    if value[0] == "int":
        return integer(value[1] + amount)
    if value[0] in ("this", "stack", "arg-address"):
        return (*value[:-1], value[-1] + amount)
    if value[0] == "arg":
        return ("arg-address", value[1], amount)
    if value[0] == "symbol":
        return ("symbol", value[1], value[2] + amount)
    return UNKNOWN


@dataclass(frozen=True)
class State:
    gpr: tuple
    fpr: tuple
    conditional: bool = False


def entry_state():
    gpr = [UNKNOWN] * 32
    gpr[1], gpr[3] = ("stack", 0), ("this", 0)
    for reg in range(4, 11):
        gpr[reg] = ("arg", reg)
    fpr = [UNKNOWN] * 32
    for reg in range(1, 9):
        fpr[reg] = ("argf", reg)
    return State(tuple(gpr), tuple(fpr))


def merge(left, right):
    if left is None:
        return right
    return State(tuple(a if a == b else UNKNOWN for a, b in zip(left.gpr, right.gpr)),
                 tuple(a if a == b else UNKNOWN for a, b in zip(left.fpr, right.fpr)),
                 left.conditional or right.conditional)


def scaffold(relocation):
    """Only exact Runtime entry identities; never recognize by substring."""
    if not relocation:
        return None
    if relocation.get("type") not in (10, 11):
        return None
    symbol = relocation.get("symbol", "")
    offset = relocation.get("addend", 0)
    if not isinstance(offset, int):
        return None
    match = re.fullmatch(r"_(save|rest)(gpr|fpr)_(1[4-9]|2[0-9]|3[01])", symbol)
    if match and offset == 0:
        return match[1], match[2], int(match[3])
    match = re.fullmatch(r"__(save|restore)_(gpr|fpr)", symbol)
    if match and offset % 4 == 0 and 0 <= offset <= 68:
        return ("rest" if match[1] == "restore" else "save", match[2], 14 + offset // 4)
    return None


def stored_value(value, width, floating=False):
    if value is None:
        return UNKNOWN
    if floating:
        if value[0] == "f32":
            if value[1] & 0x7F800000 == 0x7F800000 and value[1] & 0x7FFFFF:
                return UNKNOWN
            return ("bits", 4, value[1])
        if value[0] == "f64":
            if value[1] & 0x7FFFFFFFFFFFFFFF == 0:
                return ("bits", 4, (value[1] >> 32) & 0x80000000)
            return UNKNOWN
    elif value[0] == "int":
        return ("bits", width, value[1] & ((1 << (width * 8)) - 1))
    return ("value-source", width, value)


def symbolic_relocation(reloc):
    if not reloc or reloc.get("implicit_addend") or not isinstance(reloc.get("addend"), int):
        return False
    symbol = reloc.get("symbol")
    return (isinstance(symbol, str) and bool(symbol) and not symbol.startswith(("@", ".", "lbl_"))
            and reloc.get("symbol_binding") != 0)


def analyze_view(view):
    """Extract reachable static store occurrences with equal-or-unknown CFG joins."""
    words = view.get("words", [])
    issues = set()
    relocations = defaultdict(list)
    for reloc in view.get("relocations", []):
        relocations[int(reloc["offset"]) // 4].append(reloc)
    states = [None] * len(words)
    if not words:
        return {"stores": [], "issues": [], "reachable_instructions": 0,
                "instructions": 0, "calls": 0, "scaffold_calls": 0}
    states[0] = entry_state()

    def step(index, state, collect=False):
        word = words[index]
        op, rt, ra, rb = word >> 26, (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31
        xo, imm = (word >> 1) & 1023, signed(word & 0xFFFF, 16)
        gpr, fpr = list(state.gpr), list(state.fpr)
        successors = [index + 1] if index + 1 < len(words) else []
        conditional = state.conditional
        records = []
        local_issues = []
        relocs = relocations.get(index, [])
        reloc = relocs[0] if len(relocs) == 1 else None
        base = integer(0) if ra == 0 else gpr[ra]
        effective = add(base, imm)
        is_call = False
        helper = None

        def issue(kind):
            local_issues.append((index * 4, kind))

        def branch(displacement, absolute=False):
            if absolute:
                issue("absolute_branch_unsupported")
                return None
            destination = index * 4 + displacement
            if destination % 4 == 0 and 0 <= destination < len(words) * 4:
                return destination // 4
            issue("external_branch_or_tail_transfer")
            return None

        def load_value(width, sign=False):
            if effective and effective[0] in ("this", "arg", "arg-address", "symbol"):
                return ("load-source", effective, width, sign)
            return UNKNOWN

        if op in (14, 15):
            if reloc:
                if op == 15 and reloc.get("type") == 6 and symbolic_relocation(reloc):
                    gpr[rt] = ("symbol-ha", reloc.get("symbol"), reloc.get("addend", 0))
                elif (op == 14 and reloc.get("type") == 4 and symbolic_relocation(reloc)
                      and base and base[0] == "symbol-ha"
                      and base[1:] == (reloc.get("symbol"), reloc.get("addend", 0))):
                    gpr[rt] = ("symbol", reloc.get("symbol"), reloc.get("addend", 0))
                else:
                    gpr[rt] = UNKNOWN
                    issue("unresolved_address_relocation")
            else:
                gpr[rt] = add(base, imm << (16 if op == 15 else 0))
        elif op in (24, 25, 26, 27, 28, 29):
            value = gpr[rt]
            mask = (word & 0xFFFF) << (16 if op in (25, 27, 29) else 0)
            if relocs:
                gpr[ra] = UNKNOWN
                issue("unresolved_address_relocation")
            elif value and value[0] == "int":
                result = value[1] | mask if op in (24, 25) else value[1] ^ mask if op in (26, 27) else value[1] & mask
                gpr[ra] = integer(result)
            elif op in (24, 25, 26, 27) and mask == 0 and not relocs:
                gpr[ra] = value
            else:
                gpr[ra] = UNKNOWN
        elif op in LOADS:
            width, update, sign = LOADS[op]
            gpr[rt] = load_value(width, sign)
            if update:
                gpr[ra] = effective
        elif op in (48, 49, 50, 51):
            width = 4 if op in (48, 49) else 8
            literal = reloc.get("literal_bytes_hex") if reloc else None
            try:
                raw = bytes.fromhex(literal) if literal else b""
            except (TypeError, ValueError):
                raw = b""
            if (len(raw) == width and not reloc.get("literal_relocations_present", False)
                    and not reloc.get("implicit_addend", False)
                    and isinstance(reloc.get("addend"), int)):
                fpr[rt] = ("f32" if width == 4 else "f64", int.from_bytes(raw, "big"))
            else:
                fpr[rt] = load_value(width)
                if relocs:
                    issue("floating_literal_unresolved")
            if op in (49, 51):
                gpr[ra] = effective
        elif op in STORES or op in (54, 55, 60, 61):
            supported = op in STORES
            width, update = STORES[op] if supported else (8, op in (55, 61))
            if op in (60, 61):
                effective = add(base, signed(word & 0xFFF, 12))
            if effective and effective[0] == "this":
                if supported:
                    register_value = fpr[rt] if op in (52, 53) else gpr[rt]
                    value = stored_value(register_value, width, op in (52, 53))
                    records.append({"instruction_offset": index * 4, "offset": effective[1],
                                    "width": width, "value_source": value,
                                    "register_value": register_value,
                                    "conditional": conditional, "opcode": op})
                    if value is None:
                        issue("store_value_unknown")
                else:
                    issue("unsupported_this_store")
            elif effective is None:
                issue("unresolved_store_base")
            if update:
                gpr[ra] = effective
        elif op == 31 and xo in (444, 316, 28, 60, 124):
            left, right = gpr[rt], gpr[rb]
            if xo in (444, 28) and rt == rb:
                gpr[ra] = left
            elif xo == 316 and rt == rb:
                gpr[ra] = integer(0)
            elif left and right and left[0] == right[0] == "int":
                a, b = left[1], right[1]
                result = {444: a | b, 316: a ^ b, 28: a & b, 60: a & ~b, 124: ~(a | b)}[xo]
                gpr[ra] = integer(result)
            else:
                gpr[ra] = UNKNOWN
        elif op == 31 and xo in (266, 40):
            left, right = gpr[ra], gpr[rb]
            if xo == 266 and right and right[0] == "int":
                gpr[rt] = add(left, signed(right[1], 32))
            elif left and left[0] == "int":
                gpr[rt] = add(right, signed(left[1], 32) * (1 if xo == 266 else -1))
            else:
                gpr[rt] = UNKNOWN
        elif op == 31 and xo == 339:
            gpr[rt] = UNKNOWN
        elif op == 31 and xo in (0, 32, 467, 144, 19):
            if xo == 19:
                gpr[rt] = UNKNOWN
        elif op in (10, 11, 17) or (op == 19 and xo not in (16, 528)):
            if op == 17:
                gpr = [UNKNOWN] * 32
                fpr = [UNKNOWN] * 32
                issue("system_call_unsupported")
            elif op == 19 and xo not in (0, 33, 129, 193, 225, 257, 289, 417, 449):
                gpr = [UNKNOWN] * 32
                fpr = [UNKNOWN] * 32
                issue("unsupported_instruction")
        elif op == 18:
            if word & 1:
                is_call = True
            elif scaffold(reloc) and scaffold(reloc)[0] == "rest":
                helper = scaffold(reloc)
                successors = []
            else:
                destination = branch(signed(word & 0x3FFFFFC, 26), bool(word & 2))
                successors = [] if destination is None else [destination]
        elif op == 16:
            if word & 1:
                is_call = True
                conditional = True
                issue("conditional_call")
            else:
                destination = branch(signed(word & 0xFFFC, 16), bool(word & 2))
                always = ((word >> 21) & 0x14) == 0x14
                successors = [] if always else successors
                if destination is not None:
                    successors.append(destination)
                if not always:
                    conditional = True
        elif op == 19 and xo in (16, 528):
            if word & 1:
                is_call = True
            elif xo == 16:
                always = ((word >> 21) & 0x14) == 0x14
                if always:
                    successors = []
                else:
                    conditional = True
            else:
                successors = []
                issue("indirect_control_flow_unsupported")
        elif op == 63 and xo == 72:
            fpr[rt] = fpr[rb]
        elif op in (59, 63):
            fpr[rt] = UNKNOWN
            issue("floating_expression_unsupported")
        elif op == 21:
            gpr[ra] = UNKNOWN
            issue("rotate_expression_unsupported")
        elif op == 31 and xo in (23, 55, 87, 119, 279, 311, 343, 375):
            gpr[rt] = UNKNOWN
            if xo in (55, 119, 311, 375):
                gpr[ra] = UNKNOWN
            issue("indexed_load_unsupported")
        elif op == 47:
            if effective is None:
                issue("unresolved_store_base")
            elif effective[0] == "this":
                issue("unsupported_this_store")
        elif op == 46:
            for reg in range(rt, 32):
                gpr[reg] = UNKNOWN
            if not effective or effective[0] != "stack":
                issue("load_multiple_value_unknown")
        else:
            gpr = [UNKNOWN] * 32
            fpr = [UNKNOWN] * 32
            issue("unsupported_instruction")
        if is_call:
            helper = scaffold(reloc)
            if helper:
                action, kind, first = helper
                if action == "rest":
                    registers = gpr if kind == "gpr" else fpr
                    for reg in range(first, 32):
                        registers[reg] = UNKNOWN
            else:
                for reg in (0, *range(3, 13)):
                    gpr[reg] = UNKNOWN
                for reg in range(14):
                    fpr[reg] = UNKNOWN
        if collect:
            issues.update(local_issues)
        return State(tuple(gpr), tuple(fpr), conditional), successors, records, is_call, helper

    work = deque([0])
    queued = {0}
    while work:
        index = work.popleft()
        queued.remove(index)
        result, successors, _, _, _ = step(index, states[index])
        for destination in sorted(set(successors)):
            joined = merge(states[destination], result)
            if joined != states[destination]:
                states[destination] = joined
                if destination not in queued:
                    work.append(destination)
                    queued.add(destination)
    stores = []
    calls = helpers = 0
    for index, state in enumerate(states):
        if state is not None:
            _, _, records, call, helper = step(index, state, True)
            stores.extend(records)
            calls += bool(call)
            helpers += bool(helper)
    return {"stores": stores,
            "issues": [{"instruction_offset": off, "kind": kind} for off, kind in sorted(issues)],
            "reachable_instructions": sum(state is not None for state in states),
            "instructions": len(words), "calls": calls, "scaffold_calls": helpers}


def is_constructor(pair):
    name = pair.get("name", "")
    if re.match(r"^__ct__", name):
        return True
    demangled = pair.get("demangled_name") or ""
    before_args = demangled.split("(", 1)[0]
    parts = before_args.split("::")
    return len(parts) >= 2 and parts[-1] == parts[-2] and bool(parts[-1])


def fact_key(store):
    return store["offset"], store["width"], store["value_source"]


def compare_pair(pair):
    """Return advisory findings and extraction coverage for one constructor."""
    identity = {key: pair.get(key) for key in ("unit", "name", "demangled_name", "owner_lane", "target_va",
                                             "target_size", "source_path", "occurrence_id", "pair_status")}
    stats = Counter(constructors=1, matched_store_occurrences=0,
                    unmatched_target_store_occurrences=0)
    if pair.get("pair_status") != "PAIRED" or pair.get("source") is None or pair.get("target") is None:
        stats["coverage_gaps"] += 1
        return [{**identity, "kind": "coverage_gap", "rank": 10,
                 "pairing_reason": pair.get("pairing_reason"), "advisory": True,
                 "confidence": "unresolved"}], dict(stats)
    target, source = analyze_view(pair["target"]), analyze_view(pair["source"])
    stats["paired_constructors"] += 1
    for label, result in (("target", target), ("source", source)):
        stats[label + "_stores"] += len(result["stores"])
        stats[label + "_known_stores"] += sum(s["value_source"] is not None for s in result["stores"])
        stats[label + "_unknown_stores"] += sum(s["value_source"] is None for s in result["stores"])
        stats[label + "_issues"] += len(result["issues"])
        stats[label + "_instructions"] += result["instructions"]
        stats[label + "_reachable_instructions"] += result["reachable_instructions"]
        stats[label + "_scaffold_calls"] += result["scaffold_calls"]
    ours = defaultdict(deque)
    for store in source["stores"]:
        if store["value_source"] is not None:
            ours[fact_key(store)].append(store)
    missing = []
    for store in target["stores"]:
        key = fact_key(store)
        if store["value_source"] is None:
            continue
        if ours[key]:
            ours[key].popleft()
            stats["matched_store_occurrences"] += 1
        else:
            missing.append(store)
    unused = sorted((s for values in ours.values() for s in values), key=lambda s: s["instruction_offset"])
    unknown_source = [s for s in source["stores"] if s["value_source"] is None]
    findings = []
    for store in missing:
        unresolved = next((s for s in unknown_source if s["offset"] == store["offset"]
                           and s["width"] == store["width"]), None)
        samefield = [s for s in unused + unknown_source if s["offset"] == store["offset"]]
        candidate = next((s for s in samefield if s["width"] == store["width"]), None)
        if unresolved:
            candidate, kind, rank = unresolved, "source_value_unresolved", 30
            unknown_source.remove(unresolved)
        elif candidate:
            kind, rank = "value_source_change", 90
        elif samefield:
            candidate, kind, rank = samefield[0], "store_width_change", 95
        else:
            candidate = next((s for s in unused if s["width"] == store["width"] and s["value_source"] == store["value_source"]), None)
            if candidate:
                kind, rank = "store_offset_change", 90
            else:
                zero = store["value_source"][0] == "bits" and store["value_source"][2] == 0
                kind, rank = ("missing_zero_init", 100) if zero else ("missing_store", 70)
        if candidate and candidate in unused:
            unused.remove(candidate)
        if candidate and candidate in unknown_source:
            unknown_source.remove(candidate)
        uncertainty = []
        if store["conditional"] or (candidate and candidate["conditional"]):
            uncertainty.append("conditional_path")
        if target["issues"] or source["issues"]:
            uncertainty.append("incomplete_extraction")
            rank = min(rank, 45)
        if store["value_source"][0] != "bits":
            uncertainty.append("provenance_only_value")
            rank = min(rank, 60)
        if candidate and candidate["value_source"] is not None and candidate["value_source"][0] != "bits":
            uncertainty.append("source_provenance_only_value")
            rank = min(rank, 60)
        if store["conditional"] or (candidate and candidate["conditional"]):
            rank = min(rank, 55)
        findings.append({**identity, "kind": kind, "rank": rank, "advisory": True,
                         "target_store": store, "source_store": candidate,
                         "confidence": "limited candidate" if uncertainty else "direct store candidate",
                         "detail": describe_store(store) + " vs " + describe_store(candidate),
                         "uncertainty": uncertainty, "evidence": "static_store_multiset_difference"})
        stats["unmatched_target_store_occurrences"] += 1
    if target["issues"] or source["issues"]:
        findings.append({**identity, "kind": "extraction_incomplete", "rank": 15, "advisory": True,
                         "confidence": "unresolved",
                         "target_issues": target["issues"], "source_issues": source["issues"]})
        stats["constructors_with_incomplete_extraction"] += 1
    if not missing:
        stats["constructors_without_known_store_difference"] += 1
    target_keys = [fact_key(s) for s in target["stores"] if s["value_source"] is not None]
    source_keys = [fact_key(s) for s in source["stores"] if s["value_source"] is not None]
    if Counter(target_keys) == Counter(source_keys):
        stats["constructors_with_known_store_multiset_agreement"] += 1
        if target_keys != source_keys:
            stats["constructors_with_reordered_known_stores"] += 1
    stats["unconsumed_source_store_occurrences"] += len(unused)
    return findings, dict(stats)


def describe_store(store):
    if store is None:
        return "no recognized source store"
    offset = store["offset"]
    return f"this{offset:+#x}/{store['width']}B {store['value_source']} @+0x{store['instruction_offset']:x}"


def analyzer_pin():
    path = Path(__file__).resolve()
    return {"path": str(path), "sha256": hashlib.sha256(path.read_bytes()).hexdigest()}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path.cwd())
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--prefix", action="append", help="Repeatable unit prefix; default Game/")
    parser.add_argument("--cache-dir", type=Path)
    args = parser.parse_args(argv)
    initial_pin = analyzer_pin()
    try:
        from .mech_audit_common import iter_pairs, write_results
    except ImportError:
        from mech_audit_common import iter_pairs, write_results
    started = time.perf_counter()
    pairs = iter_pairs(args.root, prefixes=tuple(args.prefix or ("Game/",)), cache_dir=args.cache_dir)
    findings, algorithm = [], Counter()
    for pair in pairs:
        algorithm["records_seen"] += 1
        if is_constructor(pair):
            rows, stats = compare_pair(pair)
            findings.extend(rows)
            algorithm.update(stats)
    coverage = {"pairing": pairs.coverage, "algorithm": dict(sorted(algorithm.items())),
                "elapsed_seconds": round(time.perf_counter() - started, 6),
                "limits": ["Advisory static occurrences only; no correctness verdict or accepted proof.",
                           "Store multisets do not establish path, memory order, alias, or callee equivalence.",
                           "Unknown instructions, values, and indirect control reduce extraction coverage.",
                           "Only direct this-relative stw/stb/sth/stfs and update forms are compared."]}
    if analyzer_pin() != initial_pin:
        raise RuntimeError("Constructor audit analyzer changed during extraction; no results written")
    metadata = {**pairs.run_metadata, "analyzer": initial_pin}
    paths = write_results(args.root, args.output, "ctorstoreaudit", findings, coverage, metadata)
    print(paths)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
