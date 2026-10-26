#!/usr/bin/env python3
"""Read-only status/history validation; no app launch and no automatic acceptance.

The functional audio predicate is ea7dc9d3's >=50 reports / zero underruns.
Duration, executable, input, cache and machine bindings belong to the campaign.
"""
import argparse
import json
from pathlib import Path
import re


def fields(line):
    return dict(re.findall(r"([a-zA-Z_0-9]+)=([^\s]+)", line))


def evaluate(text, mode="async", observation=True):
    reports = [int(n) for n in re.findall(r"^\[audio\] level .*?, underrun (\d+) frames,", text, re.M)]
    shutdown = [int(n) for n in re.findall(r"^SDL audio underrun: (\d+) silent frames", text, re.M)]
    statuses = [fields(line) for line in text.splitlines() if line.startswith("[audio-workgroup] ")]
    errors = []
    baton_dependency = any(status.get("over175ms", "0").isdigit() and int(status.get("over175ms", "0")) > 0
                           for status in statuses)
    def require(condition, message):
        if not condition:
            errors.append(message)
    require("[audio-backend] driver=coreaudio" in text, "actual CoreAudio driver not confirmed")
    require(len(reports) >= 50, "fewer than 50 real audio reports")
    require(sum(reports + shutdown) == 0, "nonzero underrun frames")
    if mode == "async":
        require(bool(statuses), "missing workgroup adoption statuses")
        for status in statuses:
            require(status.get("mode") == "async", "unexpected workgroup mode")
            for key in ("errors", "over175ms", "history_invalid"):
                require(status.get(key) == "0", f"nonzero/missing {key}")
            require(status.get("observing") == str(int(observation)), "observation mode mismatch")
        if statuses:
            final = statuses[-1]
            require(final.get("adopted") == "1", "workgroup not adopted for every session")
            require(final.get("qualifying") == "1", "workgroup arm refused")
            for key in ("guest", "host_generation", "guest_host", "renderer_host", "completed", "samples", "sessions",
                        "guest_effective_computation_us", "renderer_effective_computation_us"):
                try:
                    require(int(final.get(key, "0"), 16 if key in ("guest", "guest_host", "renderer_host") else 10) > 0,
                            f"missing/nonpositive {key}")
                except ValueError:
                    errors.append(f"invalid {key}")
            require(final.get("guest_host") != final.get("renderer_host"), "guest and renderer identities are not distinct")
    else:
        require(not statuses, "OFF arm unexpectedly adopted workgroup")
    config = None
    pending = None
    rows = 0
    dumps = 0
    previous_sequence = 0
    for line in text.splitlines():
        if line.startswith("[audio-workgroup-config]"):
            config = fields(line)
            require(config.get("schema") == "1" and config.get("clock") == "mach_absolute_time", "unknown history schema/clock")
        elif line.startswith("[audio-workgroup-history] begin"):
            require(pending is None, "nested history dump")
            pending = fields(line)
            rows = 0
            previous_sequence = 0
            require(config is not None, "history missing clock config")
            for key in ("drops", "unstable", "clipped"):
                require(pending.get(key) == "0", f"history {key}")
        elif line.startswith("[audio-workgroup-history] event"):
            require(pending is not None, "history row outside dump")
            record = fields(line)
            try:
                seq = int(record["seq"])
                require(seq > previous_sequence, "history sequence not increasing")
                previous_sequence = seq
                require(1 <= int(record["kind"]) <= 16, "unknown event kind")
                if pending and config:
                    trigger = int(pending["trigger_mach"])
                    require(max(0, trigger - int(config["pre_ticks"])) <= int(record["mach"]) <= trigger,
                            "event outside declared trigger window")
            except (KeyError, ValueError):
                errors.append("invalid history row")
            rows += 1
        elif line.startswith("[audio-workgroup-history] end"):
            try:
                declared = int(fields(line)["rows"])
                require(pending is not None and rows == declared == int(pending["rows"]), "history row count mismatch")
                require(rows <= 8192, "history dump exceeds bound")
            except (KeyError, ValueError, TypeError):
                errors.append("invalid history footer")
            pending = None
            dumps += 1
    require(pending is None, "incomplete history dump")
    if mode == "async" and observation:
        require(config is not None, "missing observation config")
    return {"evidence_valid": not errors, "acceptance_claimed": False, "mode": mode,
            "observation": observation, "audio_reports": len(reports),
            "audio_underrun_frames": sum(reports + shutdown),
            "audio_positive_reports": sum(n > 0 for n in reports),
            "workgroup_statuses": len(statuses), "last_status": statuses[-1] if statuses else None,
            "history_dumps": dumps, "errors": list(dict.fromkeys(errors)),
            "finding": "workgroup insufficient; baton dependency" if baton_dependency else None,
            "limitations": "No duration/binary/cache/input or overhead qualification; join with pinned campaign receipts."}


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("log", type=Path)
    parser.add_argument("--mode", choices=("off", "async"), required=True)
    parser.add_argument("--observation", choices=("off", "on"), required=True)
    args = parser.parse_args()
    result = evaluate(args.log.read_text(), args.mode, args.observation == "on")
    print(json.dumps(result, indent=2))
    raise SystemExit(0 if result["evidence_valid"] else 1)
