#!/usr/bin/env python3
"""Summarize a pinned snapshot of sealed JSON summaries; never open raw evidence."""
import hashlib
import json
import math
from pathlib import Path
import sys

SOURCE = "33e71ee880b426c0c7da03a724377200e874c417"
IMAGE = "edcb94cfcca1e772f66391945969637b23f357191f5b616c02bef84aa96e2dac"
KEY = "52916cc16dd7b585855dc564202029e6c03458740dbb1a14916ed313693f4729"
MANIFEST = "b25e906dd6454964506421989c241eb17fe9f1e159f1ec26b91ea48b1869459a"
THRESHOLDS = ("over_20ms", "over_25ms", "over_33_333ms", "over_50ms", "over_100ms")
CHANNELS = ("focused_source_gameplay_intervals", "presented_drawable_intervals")


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def aggregate(stats):
    if not stats or any(not isinstance(s, dict) for s in stats):
        return "NOT_MEASURED"
    for s in stats:
        assert s["rows"] > 0 and 0 <= s[THRESHOLDS[-1]] <= s["rows"]
        assert all(s[a] >= s[b] for a, b in zip(THRESHOLDS, THRESHOLDS[1:]))
        assert all(math.isfinite(s[k]) for k in ("p99_ms", "max_ms"))
    return {"rows": sum(s["rows"] for s in stats),
            **{k: sum(s[k] for s in stats) for k in THRESHOLDS},
            "per_run_p99_ms_range": [min(s["p99_ms"] for s in stats), max(s["p99_ms"] for s in stats)],
            "largest_recorded_interval_ms": max(s["max_ms"] for s in stats),
            "pooled_p99": "NOT_DERIVABLE_FROM_SUMMARIES"}


def main(input_path, output_dir):
    source = json.loads(input_path.read_text())
    assert len(source["batches"]) == 7
    batches, stages, pins, all_stats, exits = [], [], [], {k: [] for k in CHANNELS}, []
    for item in source["batches"]:
        summaries = {}
        for name in ("BATCH-RECEIPT.json", "FRAME-FEEL-25MS.json"):
            entry = item["summaries"][name]
            assert hashlib.sha256(entry["text"].encode()).hexdigest() == entry["sha256"]
            summaries[name] = json.loads(entry["text"])
            pins.append({"batch": item["batch_number"], "path": entry["path"], "sha256": entry["sha256"]})
        batch, feel = summaries["BATCH-RECEIPT.json"], summaries["FRAME-FEEL-25MS.json"]
        for summary in (batch, feel):
            assert summary["source_sha"] == SOURCE and summary["artifact_sha256"] == IMAGE and summary["renderer_key"] == KEY
        assert batch["manifest_sha256"] == MANIFEST and batch["supported_manifest_count"] == 60
        assert batch["logs_all_unread"] and batch["worldmap_not_run"] == 7
        rows = batch["case_receipts"]
        assert len(rows) == (6 if item["batch_number"] == 1 else 12)
        metrics = {(r["stage"], r["mode"]): r for r in feel["cases"]}
        assert len(metrics) == len(rows)
        by_stage, batch_stats = {}, {k: [] for k in CHANNELS}
        for row in rows:
            assert row["status"] == "BOUNDED_IDLE_OBSERVED" and row["logs_unread"]
            assert row["exit_code"] == 124 and row["mode"] in ("fresh", "warm")
            assert row["capture_counts"] == {"surface": 3, "xfb": 3}
            if row["mode"] == "warm":
                assert row["warm_initial_cache_matches_fresh_final"] and row["warm_initial_rows_match_fresh_final"]
            assert all(c["cache_key"] == KEY for c in row["cache_metadata"].values())
            metric = metrics[(row["stage"], row["mode"])]
            merged = {k: row[k] for k in ("stage", "mode", "exit_code", "elapsed_seconds", "valid_alive_streak", "frame_rows", "capture_counts")}
            merged["frame_csv_sha256_from_existing_summary"] = metric["frame_csv_sha256"]
            for channel in CHANNELS:
                merged[channel] = metric[channel]
                batch_stats[channel].append(metric[channel])
                all_stats[channel].append(metric[channel])
            by_stage.setdefault(row["stage"], {})[row["mode"]] = merged
            exits.append(row["exit_code"])
        for name, pair in by_stage.items():
            assert set(pair) == {"fresh", "warm"}
            stages.append({"batch": item["batch_number"], "stage": name, "runs": pair})
        assert set(by_stage) == set(batch["observed_fresh_and_warm_galaxies"])
        batches.append({"batch": item["batch_number"], "destinations": list(by_stage), "runs": len(rows),
            "source_summary": item["root"], "batch_elapsed_seconds": batch["elapsed_seconds"],
            "valid_alive_streak_range": [min(r["valid_alive_streak"] for r in rows), max(r["valid_alive_streak"] for r in rows)],
            **{channel: aggregate(batch_stats[channel]) for channel in CHANNELS}})
    assert len(stages) == len({s["stage"] for s in stages}) == 39 and len(exits) == 78
    result = {"status": "SEALED_SUMMARY_METADATA_ONLY", "source_sha": SOURCE, "artifact_sha256": IMAGE,
        "renderer_key": KEY, "manifest_sha256": MANIFEST, "supported_destinations": 60,
        "observed_fresh_and_warm_direct_entry_destinations": 39, "observed_runs": 78,
        "expected_bounded_exit_code": 124, "observed_exit_codes": sorted(set(exits)),
        "case_bound_seconds": 90, "selected_surface_files": 234, "selected_XFB_files": 234,
        "warm_cache_hash_and_row_matches": 39, "worldmap_destinations_not_run": 7,
        "batches": batches, "destinations": stages,
        "cumulative_intervals": {channel: aggregate(all_stats[channel]) for channel in CHANNELS},
        "source_summary_pins": pins, "input_snapshot_sha256": sha(input_path), "tool_sha256": sha(__file__),
        "qualification": "Direct --stage scenario1 neutral idle only; fresh/warm app cache, normal bundled seed and uncontrolled OS/driver caches. Fresh empty user each run; PETARI_MODS=none,-OdysseyJumpDive, PETARI_ODYSSEY_MOVEMENT unset. Expected timeout exit124 is not clean completion. Includes capture overhead. Special scenes/ship do not establish ordinary boot, normal entry or ending completion. Captures are scheduled independently, not certified same-game-frame pairs. Logs unread.",
        "interval_semantics": "CPU-labelled values are source focused-gameplay host frame intervals. Presented values are differences between presented drawable timestamps, not GPU duration or input latency. Strict >25 ms uses recorded three-decimal CSV precision; exactly 25.000 ms is excluded. Counts sum exactly; percentile ranges are ranges of per-run percentiles, not pooled percentiles.",
        "not_measured": ["GPU duration", "input-to-screen/scan-out latency", "qualified Dolphin visual difference", "normal route", "gameplay/mission/ending completion", "crash/log/FIFO first cause"],
        "evidence_reads": "Only this JSON snapshot and tool source; no new CSV/PNG/log/archive access or dependency on released162 bulk paths."}
    output_dir.mkdir()
    jp, md = output_dir / "CUMULATIVE.json", output_dir / "TABLE.md"
    jp.write_text(json.dumps(result, indent=2, allow_nan=False) + "\n")
    lines = ["**Sealed direct-entry idle coverage through batch 7**", "", "39/60 supported destinations; 78/78 fresh/warm runs bounded at 90 seconds, all with expected exit 124. No clean-completion claim. 234 surface and 234 XFB captures were recorded; all 39 warm cache copies matched fresh hashes and row metadata. All 7 WorldMap entries remain outside this sweep.", "", "Source `33e71ee8`; executable `edcb94cf`; renderer key `52916cc1`. Full pins are in CUMULATIVE.json.", "", result["qualification"], "", result["interval_semantics"], "", "GPU duration, input/scan-out latency, qualified Dolphin visual comparison, normal route and gameplay/ending completion: **NOT_MEASURED**. Logs unread.", "", "Counts below are source focused-gameplay rows. p99 ranges are per-run ranges, not pooled p99.", "", "|Batch|Destinations / runs|CPU >25 ms / rows|Presented >25 ms / rows|CPU per-run p99 range (ms)|Presented per-run p99 range (ms)|Valid alive streak range|", "|---|---:|---:|---:|---:|---:|---:|"]
    for row in batches:
        cpu, present = (row[k] for k in CHANNELS)
        band = lambda x: "–".join(f"{v:.3f}" for v in x)
        lines.append(f"|{row['batch']}|{len(row['destinations'])} / {row['runs']}|{cpu['over_25ms']} / {cpu['rows']}|{present['over_25ms']} / {present['rows']}|{band(cpu['per_run_p99_ms_range'])}|{band(present['per_run_p99_ms_range'])}|{row['valid_alive_streak_range'][0]}–{row['valid_alive_streak_range'][1]}|")
    cpu, present = (result["cumulative_intervals"][k] for k in CHANNELS)
    streaks = [r["valid_alive_streak"] for stage in stages for r in stage["runs"].values()]
    lines += [f"|Total|39 / 78|{cpu['over_25ms']} / {cpu['rows']}|{present['over_25ms']} / {present['rows']}|Not pooled|Not pooled|{min(streaks)}–{max(streaks)}|", "", "Each pair below is **fresh / warm**, and each row is direct-entry idle coverage only.", "", "|Batch|Destination|Valid alive streak|CPU >25 ms|Presented >25 ms|", "|---|---|---:|---:|---:|"]
    for row in stages:
        f, w = row["runs"]["fresh"], row["runs"]["warm"]
        lines.append(f"|{row['batch']}|{row['stage']}|{f['valid_alive_streak']} / {w['valid_alive_streak']}|{f[CHANNELS[0]]['over_25ms']} / {w[CHANNELS[0]]['over_25ms']}|{f[CHANNELS[1]]['over_25ms']} / {w[CHANNELS[1]]['over_25ms']}|")
    lines += ["", "Full source/image/renderer/manifest pins, all 14 input-summary hashes, >20/>25/>33.333/>50/>100 counts and individual source CSV hashes from summaries are in CUMULATIVE.json. No original summary was modified; no raw evidence was opened.", ""]
    md.write_text("\n".join(lines))
    seal = {"status": "SEALED_METADATA_ONLY", "input_snapshot_sha256": sha(input_path), "tool_sha256": sha(__file__),
            "outputs": [{"path": str(p), "sha256": sha(p)} for p in (jp, md)]}
    sp = output_dir / "SEAL.json"
    sp.write_text(json.dumps(seal, indent=2) + "\n")
    print(json.dumps({"seal": str(sp), "seal_sha256": sha(sp), "destinations": 39, "runs": 78,
        "CPU_over25": cpu["over_25ms"], "presented_over25": present["over_25ms"]}))


if __name__ == "__main__":
    assert len(sys.argv) == 3
    main(Path(sys.argv[1]), Path(sys.argv[2]))
