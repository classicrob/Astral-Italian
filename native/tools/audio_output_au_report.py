#!/usr/bin/env python3
"""Streaming, read-only AU evidence validator; never launches or accepts a build.

Join with duration/app/cache/codegen bindings. Zero ring misses alone cannot
pass: the independent timeline, native health and real-output provenance count.
"""
import argparse
import json
import math
from pathlib import Path
import re


def fields(line):
    return dict(re.findall(r"([a-zA-Z_0-9]+)=([^\s]+)", line))


def evaluate(lines, observation=True):
    if isinstance(lines, str):
        lines = lines.splitlines()
    errors = set()
    reports = positives = guest_missing = statuses = 0
    final = None
    finals = {}
    previous = {}
    clock_probes = {}
    backend = False
    config = None
    dump = None
    dump_rows = dumps = unique_rows = 0
    seen = {}
    latest_seq = 0
    callback = {}
    finished = {}
    longest = []
    shortfall_rows = []
    wg_final = None
    minimum_ms = math.inf
    def require(ok, message):
        if not ok:
            errors.add(message)
    try:
        for line in lines:
            f = fields(line)
            if line.startswith("[audio-backend]") and f.get("sink") == "output-au":
                backend = f.get("driver") == "coreaudio" and f.get("subtype") == "DefaultOutput"
            elif line.startswith("[audio-au-fault]") or line.startswith("[audio-au-recovery]"):
                errors.add("native sink fault/recovery in uninterrupted arm")
            elif line.startswith("[audio-au-observer-clock] "):
                gen = int(f["generation"])
                require(gen not in clock_probes, "duplicate observer clock probe")
                clock_probes[gen] = f
                require(f["requested"] == str(int(observation)), "clock observer request mismatch")
                require(f["enabled"] == str(int(observation)), "observer clock unavailable or mode mismatch")
                if observation:
                    require(f["clock"] == "mach_absolute_time" and f["mode"] == "3" and
                            f["read_error"] == "0" and f["read_bytes"] == "1" and
                            f["exception"] == "os-commpage-mode3-consistency-reread", "unapproved observer clock path")
            elif line.startswith("[audio] level"):
                match = re.search(r", underrun (\d+) frames,", line)
                require(match is not None, "malformed audio counter")
                if match:
                    n = int(match[1]); reports += 1; positives += n > 0; guest_missing += n
            elif line.startswith("[audio-au] "):
                statuses += 1
                gen = int(f["generation"])
                raw = int(f["raw_shortfall_device_frames"])
                require(raw == 0, "raw device-frame shortfall")
                for key in ("faults", "timeline_faults", "host_timeline_faults", "invalid_timestamps", "overloads"):
                    require(int(f[key]) == 0, f"nonzero {key}")
                require(f["healthy"] == "1", "native audio unhealthy")
                require(f["underrun_unit"] == "guest-equivalent-cumulative-ceil", "unidentified counter units")
                require(float(f["device_rate"]) > 0 and int(f["guest_rate"]) in (32000, 48000), "invalid rates")
                minimum_ms = min(minimum_ms, float(f["ring_min_ms"]))
                if gen in previous:
                    for key in ("callbacks", "requested_device_frames", "copied_device_frames", "raw_shortfall_device_frames",
                                "source_pulled", "source_accepted", "device_produced"):
                        require(int(f[key]) >= int(previous[gen][key]), f"counter went backwards: {key}")
                previous[gen] = f
                if f["final"] == "1":
                    require(gen not in finals, "duplicate generation final")
                    require(int(f["requested_device_frames"]) == int(f["copied_device_frames"]) + raw,
                            "final request/copy/shortfall conservation")
                    require(int(f["consumed_device_frames"]) == int(f["copied_device_frames"]), "final ring consumption mismatch")
                    require(int(f["source_accepted"]) <= int(f["source_pulled"]), "converter accepted unpulled input")
                    require(int(f["callbacks"]) > 0 and int(f["source_pulled"]) > 0, "no real device/production progress")
                    finals[gen] = f
            elif line.startswith("[audio-au-final]"):
                require(final is None, "multiple runtime final footers (join sessions explicitly)")
                final = f
                for key in ("raw_shortfall_device_frames", "guest_equivalent_frames", "health_errors", "transitions", "default_device_setter_calls"):
                    require(int(f[key]) == 0, f"final nonzero {key}")
                require(f["complete"] == "1" and f["healthy"] == "1", "incomplete/unhealthy final")
            elif line.startswith("[audio-workgroup] "):
                wg_final = f
                for key in ("errors", "over175ms", "history_invalid"):
                    require(f[key] == "0", f"workgroup {key}")
                require(f["observing"] == str(int(observation)), "observer mode mismatch")
            elif line.startswith("[audio-workgroup-config]"):
                config = f
                require(f.get("schema") == "2" and f.get("clock") == "mach_absolute_time", "unknown history config")
            elif line.startswith("[audio-workgroup-history] begin"):
                require(dump is None and config is not None, "history begin before config / nested dump")
                dump = f; dump_rows = 0
                for key in ("drops", "unstable", "clipped", "retention_unknown", "rejected", "rejected_omitted"):
                    require(int(f[key]) == 0, f"history {key}")
                require(int(f["rows"]) <= 24576, "history dump bound")
                require(int(f["trigger_mach"]) <= int(f["end_mach"]) <= int(f["trigger_mach"]) + int(config["max_post_ticks"]),
                        "history post window")
            elif line.startswith("[audio-workgroup-history] event"):
                require(dump is not None, "history row outside dump")
                dump_rows += 1
                seq, mach, ns = int(f["seq"]), int(f["mach"]), int(f["steady_ns"])
                require(ns > 0, "missing boundary timestamp")
                if dump is not None:
                    require(max(0, int(dump["trigger_mach"]) - int(config["pre_ticks"])) <= mach <= int(dump["end_mach"]),
                            "history row outside declared window")
                signature = tuple(sorted(f.items()))
                if seq in seen:
                    require(seen[seq] == signature, "conflicting duplicate history row")
                    continue
                seen[seq] = signature; unique_rows += 1; latest_seq = max(latest_seq, seq)
                kind, ticket, gen, value = int(f["kind"]), int(f["ticket"]), int(f["object"], 16), int(f["value"])
                if 43 <= kind <= 49 or kind == 53:
                    key = (gen, ticket)
                    c = callback.setdefault(key, {"generation": gen, "ticket": ticket})
                    c[{43: "entry_ns", 44: "exit_ns", 45: "ring_before", 46: "ring_after", 47: "shortfall", 48: "sample_bits", 49: "host_time", 53: "consumed_device_frames"}[kind]] = ns if kind in (43, 44) else value
                    if kind == 47:
                        shortfall_rows.append({"generation": gen, "ticket": ticket, "steady_ns": ns, "device_frames": value})
                    if kind == 44:
                        if "entry_ns" in c:
                            require(c["exit_ns"] >= c["entry_ns"], "negative render duration")
                            old = finished.get(gen)
                            if old and ticket == old["ticket"] + 1:
                                gap = (c["entry_ns"] - old["exit_ns"]) / 1e6
                                require(gap >= 0, "render callback overlap")
                                longest.append({**c, "gap_ms": gap, "body_ms": (c["exit_ns"] - c["entry_ns"]) / 1e6})
                                longest.sort(key=lambda row: row["gap_ms"], reverse=True)
                                del longest[32:]
                            finished[gen] = c
                        callback.pop(key, None)
            elif line.startswith("[audio-workgroup-history] reject"):
                errors.add("history rejected cell")
            elif line.startswith("[audio-workgroup-history] end"):
                require(dump is not None and dump_rows == int(f["rows"]) == int(dump["rows"]), "history row count/footer mismatch")
                dumps += 1; dump = None
                # Bounded overlap cache, not the entire multi-GB history.
                seen = {seq: value for seq, value in seen.items() if seq > latest_seq - 65536}
                if len(callback) > 24576:
                    errors.add("callback analysis retention bound exceeded")
                    callback.clear()
    except (ValueError, KeyError, TypeError, OverflowError):
        errors.add("malformed or missing required evidence field")
    require(backend, "real DefaultOutput provenance missing")
    require(reports >= 50, "fewer than 50 real audio reports")
    require(guest_missing == 0, "source-equivalent underrun frames")
    require(statuses > 0 and bool(finals), "missing native statuses/final generations")
    require(set(previous) == set(finals), "generation without final counters")
    require(set(previous) == set(clock_probes), "generation without observer clock provenance")
    require(final is not None, "missing runtime footer")
    require(dump is None, "incomplete history dump")
    require(wg_final is not None and wg_final.get("adopted") == "1" and wg_final.get("qualifying") == "1", "workgroup adoption not qualified")
    if observation:
        require(config is not None, "missing history config")
    return {"evidence_valid": not errors, "acceptance_claimed": False,
            "audio_reports": reports, "positive_reports": positives, "guest_equivalent_frames": guest_missing,
            "ring_min_ms": minimum_ms if math.isfinite(minimum_ms) else None, "history_dumps": dumps,
            "observer_clock_probes": clock_probes,
            "history_unique_rows": unique_rows, "longest_consecutive_render_gaps": longest,
            "retained_shortfalls": shortfall_rows, "errors": sorted(errors),
            "limitations": "Join duration/binary/cache/codegen/latency bindings; callback actor is logical, not sampled QoS/CPU."}


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("log", type=Path)
    parser.add_argument("--observation", choices=("off", "on"), required=True)
    args = parser.parse_args()
    with args.log.open() as stream:
        result = evaluate(stream, args.observation == "on")
    print(json.dumps(result, indent=2))
    raise SystemExit(0 if result["evidence_valid"] else 1)
