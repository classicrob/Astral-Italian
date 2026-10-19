#!/usr/bin/env python3
"""Summarize native syntax audits by directory and first diagnostic.

Usage: python3 tools/port/classify_audit.py build/native-audit-baseline.json \
    build/native-audit-sdk.json --output build/native-audit-classified.md
"""

import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import re


LANES = {
    **dict.fromkeys(("Util", "LiveActor", "NameObj", "System", "Scene", "Map",
                     "Camera", "Animation", "Gravity", "Effect", "Demo", "AudioLib",
                     "GameAudio", "Speaker", "NWC24"), "core"),
    **dict.fromkeys(("MapObj", "Ride", "AreaObj", "Boss"), "objects"),
    **dict.fromkeys(("Enemy", "RhythmLib", "Screen", "NPC"), "enemies"),
    "Player": "astra",
}
DIAGNOSTIC = re.compile(
    r"^(.*?):(\d+)(?::(\d+))?:\s+(?:fatal error|error):\s+(.*)$", re.MULTILINE
)


def relative_location(path, root):
    path = path.replace("\\", "/")
    if root and path.startswith(root.rstrip("/") + "/"):
        path = path[len(root.rstrip("/")) + 1:]
    if path.startswith("build/audit-sjis/"):
        path = path[len("build/audit-sjis/"):]
    return path


def normalize_message(message):
    message = re.sub(r"(?<![\w])/(?:[^\s:'\"<>]+/)*[^\s:'\"<>]+", "<path>", message)
    message = re.sub(r":\d+(?::\d+)?", ":<line>", message)
    return " ".join(message.split())


def classify(message):
    """Return the primary category and an unknown symbol, when present."""
    lower = message.lower()
    if "file not found" in lower or "no such file or directory" in lower:
        return "missing header", None
    if re.search(r"\b(?:asm|__asm|__asm__)\b|inline assembly|invalid output constraint|invalid input constraint", lower):
        return "inline asm / asm function", None
    if re.search(r"__(?:fabs|frsqrte|fres|fsel|fmadd|fmsub|fnmadd|fnmsub|cntlzw|rlwinm|sync|isync|eieio)\w*", message):
        return "Wii intrinsic", None
    if (re.search(r"(?:pointer|\*)", lower)
            and re.search(r"(?:cast|convert|conversion)", lower)
            and re.search(r"(?:smaller|precision|32|u32|s32|\bint\b|\blong\b)", lower)):
        return "pointer to integer (64-bit hazard)", None
    if re.search(r"__declspec|__attribute__|#pragma|0x[0-9a-f]+|address specification|mwcc", lower):
        return "MWCC-only syntax", None
    if re.search(r"override|overrid|virtual|return type.*(?:differs|covariant)", lower):
        return "override / virtual mismatch", None
    if re.search(r"ambiguous|overload|no matching (?:function|constructor|member function)|candidate function", lower):
        return "ambiguous / overload", None
    unknown = re.search(
        r"(?:unknown type name|use of undeclared identifier|undeclared identifier|"
        r"no (?:type|template|member) named|unknown register name) ['‘]([^'’]+)['’]", message
    )
    if unknown:
        return "unknown type / identifier", unknown.group(1)
    if re.search(r"template|specialization|instantiation", lower):
        return "template", None
    return "other", None


def first_error(diagnostics, root):
    match = DIAGNOSTIC.search(diagnostics)
    if match:
        path, line, _, message = match.groups()
        location = relative_location(path, root) + ":" + line
    else:
        match = re.search(r"^(?:.*?:\s*)?(?:fatal error|error):\s*(.*)$", diagnostics, re.MULTILINE)
        message = match.group(1) if match else "No error diagnostic captured"
        location = "<compiler>"
    message = normalize_message(message)
    category, symbol = classify(message)
    return {"location": location, "message": message, "class": category, "symbol": symbol}


def group_for(source):
    parts = Path(source).parts
    if len(parts) >= 4 and parts[0] == "src" and parts[1] in {"Game", "JSystem", "nw4r"}:
        return "/".join(parts[:3])
    return "/".join(parts[:2])


def lane_for(group):
    parts = group.split("/")
    if len(parts) >= 3 and parts[1] == "Game":
        return LANES.get(parts[2], "unassigned")
    if len(parts) >= 2 and parts[1] in {"JSystem", "nw4r", "RVL_SDK", "RVLFaceLib", "libms"}:
        return "m1"
    return "unassigned"


def read_reports(paths):
    results = {}
    reports = []
    for path in paths:
        report = json.loads(path.read_text())
        reports.append(report)
        for result in report["results"]:
            source = result["source"]
            if source in results:
                raise ValueError(f"duplicate source across input reports: {source}")
            entry = dict(result)
            if not entry["passed"]:
                entry["first_error"] = first_error(entry["diagnostics"], report.get("root", ""))
            results[source] = entry
    return reports, results


def cell(value):
    return str(value).replace("|", "\\|").replace("\n", " ")


def render(reports, results, overlay=None):
    groups = defaultdict(list)
    for source, result in results.items():
        groups[group_for(source)].append(result)
    for report in reports:
        for path in report.get("paths", []):
            if len(Path(path).parts) == 2:
                if not any(g == path or g.startswith(path + "/") for g in groups):
                    groups[path]
    passed = sum(r["passed"] for r in results.values())
    lines = [f"Checked **{len(results)}**; passing **{passed}**; failing **{len(results) - passed}**.", ""]
    headings = ["Directory", "Lane", "Checked", "Passing", "Failing", "Top 3 first-error classes"]
    if overlay is not None:
        headings.append("With transferred hooks (passing/checked)")
    lines += ["| " + " | ".join(headings) + " |", "| " + " | ".join("---" for _ in headings) + " |"]
    for group, entries in sorted(groups.items()):
        passing = sum(r["passed"] for r in entries)
        classes = Counter(r["first_error"]["class"] for r in entries if not r["passed"])
        top = "; ".join(f"{name}: {count}" for name, count in classes.most_common(3)) or "—"
        row = [group, lane_for(group), len(entries), passing, len(entries) - passing, top]
        if overlay is not None:
            matches = [r for s, r in overlay.items() if group_for(s) == group]
            row.append(f"{sum(r['passed'] for r in matches)}/{len(matches)}")
        lines.append("| " + " | ".join(cell(v) for v in row) + " |")
    failures = [r["first_error"] for r in results.values() if not r["passed"]]
    roots = Counter((e["location"], e["message"], e["class"]) for e in failures)
    lines += ["", "## Ranked root causes", "",
              "Counts group each file's first error by diagnostic location and normalized message. Later errors are masked; these counts do not establish how many files a fix will make pass.", "",
              "| Rank | Files | Header/source:line | First error | Class |", "| ---: | ---: | --- | --- | --- |"]
    for rank, ((location, message, category), count) in enumerate(roots.most_common(), 1):
        lines.append("| " + " | ".join(cell(v) for v in (rank, count, location, message, category)) + " |")
    messages = Counter(e["message"] for e in failures)
    lines += ["", "## Top 20 distinct first-error messages", "",
              "| Rank | Files | Normalized message |", "| ---: | ---: | --- |"]
    for rank, (message, count) in enumerate(messages.most_common(20), 1):
        lines.append(f"| {rank} | {count} | {cell(message)} |")
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reports", nargs="+", type=Path)
    parser.add_argument("--output", type=Path, help="Markdown destination; otherwise stdout")
    parser.add_argument("--json-output", type=Path, help="Per-file first errors and primary classes")
    parser.add_argument("--overlay", nargs="+", type=Path, help="Hooked audit reports to compare")
    args = parser.parse_args()
    reports, results = read_reports(args.reports)
    overlay = read_reports(args.overlay)[1] if args.overlay else None
    markdown = render(reports, results, overlay)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(markdown)
    else:
        print(markdown, end="")
    if args.json_output:
        args.json_output.parent.mkdir(parents=True, exist_ok=True)
        args.json_output.write_text(json.dumps({"results": [
            {k: v for k, v in r.items() if k != "diagnostics"} for r in results.values()
        ]}, indent=2) + "\n")


if __name__ == "__main__":
    main()
