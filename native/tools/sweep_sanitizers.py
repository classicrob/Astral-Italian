#!/usr/bin/env python3
"""Group the sanitizer lines of a headless_sweep run (ASan/UBSan build) by site.

  python3 native/tools/sweep_sanitizers.py <sweep dir> [--top 40]

Reads runs/*/app.log. `runtime error:` (UBSan) lines are keyed by source file:line and message kind; an
AddressSanitizer report by its kind and first game frame. Prints one row per site: runs hit, total hits, one example
run and message, ranked by number of runs.
"""
import argparse
import re
from collections import defaultdict
from pathlib import Path

UB = re.compile(r"^(?P<loc>[^\s:]+:\d+)(?::\d+)?: runtime error: (?P<msg>.*)$")
ASAN = re.compile(r"ERROR: AddressSanitizer: (?P<kind>\S+)")
FRAME = re.compile(r"#\d+ 0x[0-9a-f]+ in (?P<fn>\S+(?: \S+)*?) (?P<file>\S+:\d+)")


def norm_msg(msg):
    msg = re.sub(r"0x[0-9a-f]+", "0xN", msg)
    msg = re.sub(r"\b\d+\b", "N", msg)
    return msg[:110]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("sweep", type=Path)
    ap.add_argument("--top", type=int, default=40)
    ap.add_argument("--common", type=float, default=0.9, help="skip sites hit in at least this fraction of runs (boot noise)")
    a = ap.parse_args()
    sites = defaultdict(lambda: {"runs": set(), "hits": 0, "example": None})
    for log in sorted((a.sweep / "runs").glob("*/app.log")):
        run = log.parent.name
        text = log.read_text(errors="replace")
        for line in text.splitlines():
            m = UB.match(line.strip())
            if m:
                key = (m["loc"].split("/")[-1] if "/" in m["loc"] else m["loc"], norm_msg(m["msg"]))
                site = sites[key]
                site["runs"].add(run)
                site["hits"] += 1
                site["example"] = site["example"] or (run, m["loc"])
        m = ASAN.search(text)
        if m:
            frames = [f for f in FRAME.findall(text) if "/src/" in f[1] or "/native/" in f[1]]
            where = frames[0][1].split("/")[-1] if frames else "?"
            key = (where, "ASAN " + m["kind"])
            site = sites[key]
            site["runs"].add(run)
            site["hits"] += 1
            site["example"] = site["example"] or (run, frames[0][0] if frames else "?")
    total_runs = len(list((a.sweep / "runs").glob("*/app.log")))
    rows = sorted(sites.items(), key=lambda kv: (-len(kv[1]["runs"]), -kv[1]["hits"]))
    common = [r for r in rows if len(r[1]["runs"]) >= a.common * total_runs]
    rows = [r for r in rows if len(r[1]["runs"]) < a.common * total_runs]
    print(f"{total_runs} runs; {len(common)} startup/common sites (in >= {a.common:.0%} of runs, skipped); {len(rows)} stage-dependent sites")
    for (where, msg), s in rows[: a.top]:
        print(f"{len(s['runs']):4d} runs {s['hits']:8d} hits  {where}  {msg}  e.g. {s['example'][0]}")


if __name__ == "__main__":
    main()
