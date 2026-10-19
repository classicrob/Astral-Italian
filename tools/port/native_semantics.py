#!/usr/bin/env python3
"""Inventory transferred replacement branches for a read-only semantic review.

No compiler, build, checkout, or game-source writes. Function/token matching is
lexical; the output is evidence for human review, not an equivalence proof.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import difflib
import hashlib
import json
from pathlib import Path
import re
import subprocess

from hooktriage import branch_view, functions, source_tokens


def digest(text):
    return hashlib.sha256(text.encode("utf-8", errors="surrogateescape")).hexdigest()


def native_blocks(text):
    stack, result = [], []
    offset = 0
    for number, line in enumerate(text.splitlines(keepends=True), 1):
        m = re.match(r"\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)", line)
        if m:
            command, expression = m.groups()
            if command in {"if", "ifdef", "ifndef"}:
                stack.append({"start": offset, "line": number, "native": bool(re.search(r"GALAXY_NATIVE|__MWERKS__", expression)),
                              "else": False, "condition": line.strip()})
            elif command in {"else", "elif"} and stack:
                stack[-1]["else"] = True
                stack[-1]["native"] |= "GALAXY_NATIVE" in expression
            elif command == "endif" and stack:
                block = stack.pop()
                if block.pop("native"):
                    block.update(end=offset + len(line), end_line=number)
                    result.append(block)
        offset += len(line)
    return sorted(result, key=lambda b: b["start"])


def payload_lines(text):
    result = set()
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    for line in text.splitlines():
        line = re.sub(r"//.*", "", line).strip()
        if line.startswith("#"):
            continue
        normalized = " ".join(source_tokens(line))
        if len(normalized) >= 12:
            result.add(normalized)
    return result


def git_text(root, revision, path):
    run = subprocess.run(["git", "-C", str(root), "show", f"{revision}:{path}"],
                         capture_output=True)
    return run.stdout.decode("utf-8", errors="surrogateescape") if run.returncode == 0 else None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--base", default="e5bc761c0")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    raw = args.report.read_text()
    report = json.loads(raw)
    records, unmatched, counts = [], [], Counter()
    source_hashes = {}
    for entry in report["files"]:
        selected = []
        occurrences = Counter()
        for index, group in enumerate(entry["groups"]):
            key = digest(group["text"])[:12]
            occurrences[key] += 1
            if group["kind"] in {"clean", "adapted", "relocated"}:
                counts["applied_groups"] += 1
                selected.append(dict(group, index=index,
                                     id=f'{entry["path"]}:{key}:{occurrences[key]}'))
        if not selected:
            continue
        original = entry["path"]
        targets = {g.get("target") or entry.get("smg2_path") for g in selected}
        base = git_text(args.reference, args.base, original)
        if base is None:
            unmatched.append({"path": original, "reason": "Petari base unavailable"})
            continue
        base_view = branch_view(base, False, True)
        base_functions = functions(base_view)
        by_name = defaultdict(list)
        for function in base_functions:
            by_name[function["name"]].append(function)
        for target in sorted(p for p in targets if p):
            path = args.root / target
            if not path.exists():
                unmatched.append({"path": target, "reason": "overlay path absent"})
                continue
            text = path.read_text(errors="surrogateescape")
            source_hashes[target] = digest(text)
            wii = branch_view(text, False, True)
            native = branch_view(text, True, False)
            wii_functions = functions(wii)
            blocks = native_blocks(text)
            for block in blocks:
                block["payload"] = payload_lines(text[block["start"]:block["end"]])
            groups = [g for g in selected if (g.get("target") or entry.get("smg2_path")) == target]
            for group in groups:
                additions = "\n".join(l[1:] for l in group["text"].splitlines() if l.startswith("+"))
                added_lines = payload_lines(additions)
                plausible = []
                for block in blocks:
                    block_native = native[block["start"]:block["end"]]
                    block_wii = wii[block["start"]:block["end"]]
                    if not source_tokens(block_native) or source_tokens(block_native) == source_tokens(block_wii):
                        continue
                    if not block["else"] and "return" not in source_tokens(block_native):
                        continue
                    hits = added_lines & block["payload"]
                    if hits:
                        plausible.append((block, hits))
                # Resolve generic repeated additions by the enclosing Petari symbol.
                base_here = [f for f in base_functions if f["line"] <= group["line"] <= f["end_line"]]
                expected = {f["name"] for f in base_here}
                if not expected and group.get("function"):
                    expected.add(group["function"])
                matched_blocks = []
                for block, hits in plausible:
                    block = {k: v for k, v in block.items() if k != "payload"}
                    enclosed = [f for f in wii_functions if
                                f["start"] <= block["start"] < f["end"] or
                                block["start"] <= f["start"] < block["end"]]
                    expected_short = {n.removeprefix("JGeometry::") for n in expected}
                    enclosed_short = {f["name"].removeprefix("JGeometry::") for f in enclosed}
                    if expected and enclosed and not expected_short & enclosed_short:
                        continue
                    if len(plausible) > 1 and len(hits) == 1 and expected and enclosed and not expected & {f["name"] for f in enclosed}:
                        continue
                    # A short shared line alone is weak evidence across symbols.
                    if len(hits) == 1 and max(map(len, hits)) < 28 and expected and enclosed and not expected & {f["name"] for f in enclosed}:
                        continue
                    matched_blocks.append((block, enclosed))
                checked, changed = [], []
                for block, enclosed in matched_blocks:
                    for function in enclosed:
                        candidates = by_name.get(function["name"], [])
                        if not candidates and original.endswith("/JGeometry/TVec.hpp"):
                            candidates = by_name.get("JGeometry::" + function["name"], [])
                        current = wii[function["start"]:function["end"]]
                        if not candidates:
                            changed.append({"function": function["name"], "line": function["line"],
                                            "reason": "No exact pristine symbol; review required",
                                            "wii_function": current,
                                            "native_block": native[block["start"]:block["end"]],
                                            "block": block})
                            continue
                        closest = max(candidates, key=lambda f: difflib.SequenceMatcher(None,
                                      source_tokens(base_view[f["start"]:f["end"]]), source_tokens(current),
                                      autojunk=False).ratio())
                        old = base_view[closest["start"]:closest["end"]]
                        different = source_tokens(old) != source_tokens(current)
                        checked.append({"function": function["name"], "line": function["line"],
                                        "different": different, "block_line": block["line"]})
                        if different:
                            changed.append({"function": function["name"], "line": function["line"],
                                            "base_line": closest["line"], "base_function": old,
                                            "wii_function": current,
                                            "native_block": native[block["start"]:block["end"]],
                                            "wii_block": wii[block["start"]:block["end"]],
                                            "native_function": native[function["start"]:function["end"]],
                                            "function_diff": "".join(difflib.unified_diff(
                                                [l+'\n' for l in old.splitlines() if l.strip()],
                                                [l+'\n' for l in current.splitlines() if l.strip()],
                                                fromfile="Petari", tofile="SMG2", n=2)), "block": block})
                if matched_blocks:
                    counts["groups_with_replacement_blocks"] += 1
                    record = {"id": group["id"], "path": target, "origin": original,
                              "origin_line": group["line"], "group_index": group["index"],
                              "kind": group["kind"], "text": group["text"], "checked": checked,
                              "changes": changed,
                              "blocks": [dict(b, functions=[f["name"] for f in fs]) for b, fs in matched_blocks]}
                    records.append(record)
                elif "#else" in additions or ("return" in additions and "GALAXY_NATIVE" in additions):
                    unmatched.append({"id": group["id"], "path": target, "origin": original,
                                      "line": group["line"], "reason": "No replacement block with matching payload",
                                      "text": group["text"]})
    counts["groups_with_changed_functions"] = sum(bool(r["changes"]) for r in records)
    counts["unmatched"] = len(unmatched)
    result = {"report": str(args.report.resolve()), "report_sha256": digest(raw),
              "root": str(args.root.resolve()), "base": args.base, "counts": dict(counts),
              "source_hashes": source_hashes, "records": records, "unmatched": unmatched}
    args.output.write_text(json.dumps(result, indent=2, ensure_ascii=True) + "\n")
    print(json.dumps(result["counts"]))


if __name__ == "__main__":
    main()
