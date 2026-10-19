#!/usr/bin/env python3
"""Deterministic PPC immediate-stream advisory screen; no semantic verdicts.

Register numbers and assembler aliases do not participate in stream identity.
Unselected instructions, data identity, def-use and control-flow equivalence are
outside this screen. Nearby branches are context, not established consumers.
"""

import argparse
from collections import Counter
from difflib import SequenceMatcher
import hashlib
import json
from pathlib import Path


COMPARISON_FIELDS = {
    "compare-immediate": ("opcode", "signedness", "width", "immediate_bits"),
    "and-immediate": ("opcode", "width", "immediate_bits"),
    "rotate-mask": ("opcode", "width", "sh", "mb", "me", "rc"),
}


def ppc_mask(mb, me):
    """Return the inclusive, wrapping MSB-zero-numbered 32-bit PPC mask."""
    if not 0 <= mb < 32 or not 0 <= me < 32:
        raise ValueError("PPC mask bounds must be in 0..31")
    value, bit = 0, mb
    while True:
        value |= 1 << (31 - bit)
        if bit == me:
            return value
        bit = (bit + 1) % 32


def decode_word(word, offset=0):
    """Decode selected raw instructions, including all rlwinm aliases."""
    if not isinstance(word, int) or not 0 <= word <= 0xffffffff:
        raise ValueError("expected unsigned 32-bit instruction word")
    op = word >> 26
    row = {"offset": offset, "word": f"0x{word:08X}"}
    if op in (10, 11):
        signed = op == 11
        immediate = word & 0xffff
        row.update(kind="compare-immediate", opcode="cmpi" if signed else "cmpli",
                   signedness="signed" if signed else "unsigned",
                   width=64 if word & (1 << 21) else 32,
                   immediate_bits=immediate,
                   immediate=immediate - 65536 if signed and immediate & 0x8000 else immediate,
                   cr_field=(word >> 23) & 7)
    elif op in (28, 29):
        immediate = word & 0xffff
        row.update(kind="and-immediate", opcode="andi." if op == 28 else "andis.",
                   width=32, immediate_bits=immediate, immediate=immediate,
                   effective_mask=immediate << (16 if op == 29 else 0), cr_field=0)
    elif op == 21:
        sh, mb, me, rc = (word >> 11) & 31, (word >> 6) & 31, (word >> 1) & 31, word & 1
        row.update(kind="rotate-mask", opcode="rlwinm", width=32,
                   sh=sh, mb=mb, me=me, rc=rc,
                   effective_mask=ppc_mask(mb, me), cr_field=0 if rc else None)
    else:
        return None
    return row


def signature(event):
    return (event["kind"], *(event[k] for k in COMPARISON_FIELDS[event["kind"]]))


def nearby_branches(words, event, radius=5):
    """Report local conditional branches without pretending to trace CR uses."""
    result = []
    index = event["offset"] // 4
    for i in range(max(0, index - radius), min(len(words), index + radius + 1)):
        word = words[i]
        op, xo = word >> 26, (word >> 1) & 1023
        if op != 16 and not (op == 19 and xo in (16, 528)):
            continue
        bo, bi = (word >> 21) & 31, (word >> 16) & 31
        tests_cr, tests_ctr = not bool(bo & 16), not bool(bo & 4)
        if not tests_cr and not tests_ctr:
            continue
        row = {"offset": 4 * i, "distance_instructions": i - index,
               "word": f"0x{word:08X}",
               "opcode": "bc" if op == 16 else "bclr" if xo == 16 else "bcctr",
               "bo": bo, "bi": bi, "tests_cr": tests_cr, "tests_ctr": tests_ctr,
               "link": bool(word & 1), "same_cr_field": tests_cr and event["cr_field"] == bi // 4}
        if tests_cr:
            row.update(cr_field=bi // 4, cr_bit=("LT", "GT", "EQ", "SO")[bi % 4],
                       expected_bit=bool(bo & 8))
        result.append(row)
    return result


def extract_stream(view):
    words = view["words"]
    size = view["size"]
    if not isinstance(size, int) or size < 0 or size % 4 or len(words) * 4 != size:
        raise ValueError("function view has inconsistent/unaligned extent")
    relocations = view.get("relocations", [])
    result = []
    for i, word in enumerate(words):
        event = decode_word(word, i * 4)
        if event is None:
            continue
        event["stream_index"] = len(result)
        event["elf_section_offset"] = view.get("address", 0) + event["offset"]
        event["nearby_branches"] = nearby_branches(words, event)
        event["relocations"] = sorted(
            (dict(r) for r in relocations if event["offset"] <= r["offset"] < event["offset"] + 4),
            key=lambda r: (r["offset"], str(r.get("type")), str(r.get("symbol")), r.get("addend", 0)))
        result.append(event)
    return result


def _event_context(event):
    if event is None:
        return None
    return dict(event)


def _change(target, source, isolated):
    changed = {key: {"target": target[key], "source": source[key]}
               for key in COMPARISON_FIELDS[target["kind"]] if target[key] != source[key]}
    if target["kind"] == "compare-immediate":
        kind = "compare-immediate-drift" if "immediate_bits" in changed else "compare-mode-drift"
    elif target["kind"] == "rotate-mask":
        kind = "rotate-mask-drift"
    else:
        kind = "and-mask-drift"
    high = isolated and (set(changed) == {"immediate_bits"} or
                         target["kind"] == "rotate-mask" and set(changed) <= {"sh", "mb", "me"})
    row = {"kind": kind, "rank": 100 if high else 80 if isolated else 60,
           "confidence": "isolated-same-kind-change" if isolated else "ordered-same-kind-block",
           "changed_fields": changed, "target": _event_context(target), "source": _event_context(source)}
    if "effective_mask" in target and "effective_mask" in source:
        xor = target["effective_mask"] ^ source["effective_mask"]
        row["mask_xor"] = f"0x{xor:08X}"
        row["changed_mask_bits_lsb0"] = [bit for bit in range(32) if xor & (1 << bit)]
    if target["kind"] == "compare-immediate":
        mask = (1 << target["width"]) - 1
        row["same_threshold_bit_pattern"] = (target["width"] == source["width"] and
                                               target["immediate"] & mask == source["immediate"] & mask)
    row["branch_context_is_not_def_use_proof"] = True
    return row


def compare_streams(target, source):
    """Align exact canonical anchors; never zip across an insertion blindly."""
    tkeys, skeys = [signature(x) for x in target], [signature(x) for x in source]
    if tkeys == skeys:
        return []
    if Counter(tkeys) == Counter(skeys):
        return [{"kind": "stream-reordered", "rank": 30, "confidence": "stream-shape-only",
                 "target": target, "source": source,
                 "reason": "Same selected-event multiset, different order; no semantic conclusion."}]
    findings = []
    for tag, a, b, c, d in SequenceMatcher(None, tkeys, skeys, autojunk=False).get_opcodes():
        if tag == "equal":
            continue
        tblock, sblock = target[a:b], source[c:d]
        if tag == "replace" and len(tblock) == len(sblock) and all(
                t["kind"] == s["kind"] for t, s in zip(tblock, sblock)):
            findings.extend(_change(t, s, len(tblock) == 1) for t, s in zip(tblock, sblock))
        else:
            findings.append({"kind": {"insert": "source-extra-events", "delete": "source-missing-events"}.get(tag, "stream-shape-change"),
                             "rank": 25, "confidence": "stream-shape-only",
                             "target": tblock, "source": sblock,
                             "target_stream_range": [a, b], "source_stream_range": [c, d],
                             "reason": "Exact-anchor alignment; unmatched counts/kinds are not an immediate substitution proof."})
    return findings


def _sort_key(row):
    return (-row["rank"], row["unit"], row.get("target_va") or 0, row["name"],
            str(row.get("occurrence_id", "")), row["finding_index"])


def _detail(row):
    def positions(value):
        events = value if isinstance(value, list) else [value] if value else []
        return ",".join(f"+0x{event['offset']:X}" for event in events) or "none"
    text = f"target {positions(row['target'])}; source {positions(row['source'])}"
    if row.get("changed_fields"):
        text += "; " + ", ".join(f"{key} {value['target']} -> {value['source']}"
                                  for key, value in row["changed_fields"].items())
    if row.get("changed_mask_bits_lsb0"):
        text += "; mask bits (LSB0) " + ",".join(map(str, row["changed_mask_bits_lsb0"]))
    if row.get("target_event_count") is not None:
        text += f"; event counts {row['target_event_count']} -> {row['source_event_count']}"
    return text


def audit_pairs(pairs):
    """Consume the shared provider or synthetic fixtures without filesystem work."""
    coverage = Counter({key: 0 for key in (
        "registered_occurrences", "uncompared_occurrences", "invalid_view_occurrences",
        "paired_occurrences", "target_selected_events", "source_selected_events",
        "relocation_affected_occurrences", "compared_occurrences", "different_stream_occurrences",
        "equal_stream_occurrences", "zero_selected_event_occurrences")})
    statuses = Counter()
    findings = []
    for pair in pairs:
        coverage["registered_occurrences"] += 1
        statuses[pair["pair_status"]] += 1
        if pair["pair_status"] != "PAIRED" or pair.get("target") is None or pair.get("source") is None:
            coverage["uncompared_occurrences"] += 1
            continue
        try:
            tstream = extract_stream(pair["target"])
            sstream = extract_stream(pair["source"])
        except (ValueError, KeyError, TypeError) as error:
            coverage["invalid_view_occurrences"] += 1
            rows = [{"kind": "invalid-function-view", "rank": 0, "confidence": "uncompared",
                     "reason": str(error), "target": None, "source": None}]
        else:
            coverage["paired_occurrences"] += 1
            coverage["target_selected_events"] += len(tstream)
            coverage["source_selected_events"] += len(sstream)
            for event in tstream:
                if pair.get("target_va") is not None:
                    event["virtual_address"] = pair["target_va"] + event["offset"]
            relocated = [event for event in tstream + sstream if event["relocations"]]
            if relocated:
                coverage["relocation_affected_occurrences"] += 1
                rows = [{"kind": "relocated-immediate-stream-uncompared", "rank": 5,
                         "confidence": "uncompared",
                         "target": [event for event in tstream if event["relocations"]],
                         "source": [event for event in sstream if event["relocations"]],
                         "reason": "Selected instruction contains relocation bits; raw immediate comparison is deferred for this occurrence."}]
            else:
                rows = compare_streams(tstream, sstream)
                coverage["compared_occurrences"] += 1
                coverage["different_stream_occurrences" if rows else "equal_stream_occurrences"] += 1
                if not tstream and not sstream:
                    coverage["zero_selected_event_occurrences"] += 1
        for index, row in enumerate(rows):
            row.update({key: pair.get(key) for key in (
                "unit", "name", "demangled_name", "owner_lane", "target_va", "target_size",
                "source_path", "pair_status", "occurrence_id")})
            row.update(finding_index=index, advisory_only=True, semantic_bug_claimed=False)
            if row["kind"] != "invalid-function-view":
                row.update(target_event_count=len(tstream), source_event_count=len(sstream),
                           source_size=pair["source"]["size"])
            row["detail"] = _detail(row)
            findings.append(row)
    findings.sort(key=_sort_key)
    coverage["findings"] = len(findings)
    result = dict(sorted(coverage.items()))
    result["pair_status_counts"] = dict(sorted(statuses.items()))
    result["finding_kind_counts"] = dict(sorted(Counter(x["kind"] for x in findings).items()))
    return findings, result


def _tool_sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main(argv=None):
    tool_path = Path(__file__).resolve()
    tool_start_sha256 = _tool_sha256(tool_path)
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--output", type=Path, required=True,
                        help="report directory or explicit JSON path; also writes per-lane ranked Markdown")
    parser.add_argument("--prefix", action="append", help="repeatable unit prefix (default Game/)")
    args = parser.parse_args(argv)
    from mech_audit_common import iter_pairs, write_results
    pairs = iter_pairs(args.root, prefixes=tuple(args.prefix or ("Game/",)))
    findings, screen_coverage = audit_pairs(pairs)
    coverage = {"pairing": pairs.coverage, "branch_immediate_screen": screen_coverage}
    metadata = dict(pairs.run_metadata)
    metadata["screen_scope"] = "Advisory selected raw immediate stream only; no semantic verdict, full instruction equivalence, CR def-use or data proof."
    tool_end_sha256 = _tool_sha256(tool_path)
    if tool_start_sha256 != tool_end_sha256:
        raise RuntimeError("analyzer source changed during audit; report publication aborted")
    metadata["analyzer_source"] = {"path": str(tool_path), "sha256": tool_start_sha256,
                                   "start_sha256": tool_start_sha256, "end_sha256": tool_end_sha256,
                                   "stable": True}
    paths = write_results(args.root, args.output, "branchimmaudit", findings, coverage, metadata)
    if _tool_sha256(tool_path) != tool_start_sha256:
        raise RuntimeError("analyzer source changed during report writing; discard generated outputs")
    print(json.dumps({"output": paths, "coverage": screen_coverage}, sort_keys=True, default=str))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
