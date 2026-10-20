#!/usr/bin/env python3
"""Create/reload an SMG2 complete save through the running game's save code."""
import argparse
import csv
from datetime import datetime
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
UNLOCKED = Path.home() / "Library/Application Support/AstralItalian-unlocked"


def isolated(user):
    return UNLOCKED.resolve() == UNLOCKED and user.resolve().is_relative_to(UNLOCKED)


def ready_frames(trace, stage):
    with trace.open(newline="") as source:
        return sum(row["stage"] == stage and row["scene"] == "Game" and row["init_state"] == "6"
                   for row in csv.DictReader(source))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True, help="incrementally built petari_headless")
    parser.add_argument("--disc", type=Path, default=Path.home() / "MacioGalaxy2-gamedata/SB4E01-extracted")
    parser.add_argument("--inputs", type=Path, default=Path.home() / "MacioGalaxy2-gamedata/sweep/tape-30000.jsonl")
    parser.add_argument("--user", type=Path, default=UNLOCKED)
    parser.add_argument("--frames", type=int, default=30000)
    parser.add_argument("--verify-only", action="store_true", help="reload an existing generated save without awarding anything")
    args = parser.parse_args()
    if not isolated(args.user):
        parser.error("only AstralItalian-unlocked or a directory beneath it is allowed; live saves are forbidden")
    if args.frames < 18000:
        parser.error("acceptance needs at least five minutes of ready scene frames")
    if not args.verify_only and args.user.exists() and any(args.user.iterdir()):
        parser.error("creation requires a fresh empty separate user directory; refusing to overwrite")
    if args.verify_only and not (args.user / "complete-save.json").is_file():
        parser.error("no game-generated complete-save receipt; create the separate save first")
    output = Path.home() / "MacioGalaxy2-gamedata/unlocked-validation" / datetime.now().strftime("%Y%m%d-%H%M%S-%f")
    output.mkdir(parents=True, exist_ok=False)
    results = []
    for index, stage in enumerate(("WorldMap07Galaxy", "MarioFaceShipGalaxy")):
        create = index == 0 and not args.verify_only
        env = os.environ.copy()
        env.pop("ASTRAL_DEV_COMPLETE_SAVE", None)
        env.pop("ASTRAL_VERIFY_COMPLETE_SAVE", None)
        env["ASTRAL_DEV_COMPLETE_SAVE" if create else "ASTRAL_VERIFY_COMPLETE_SAVE"] = "1"
        trace = output / (stage + ".csv")
        command = [sys.executable, str(ROOT / "native/tools/headless.py"), "run",
                   "--executable", str(args.executable.resolve()), "--inputs", str(args.inputs.resolve()),
                   "--frames", str(args.frames), "--epoch-ticks", "0", "--disc", str(args.disc.resolve()),
                   "--user", str(args.user.resolve()), "--trace", str(trace), "--stage", stage]
        if not create:
            command.append("--existing-unlocked-user")
        log = output / (stage + ".log")
        with log.open("x") as stream:
            result = subprocess.run(["nice", "-n", "19", *command], env=env, stdout=stream, stderr=subprocess.STDOUT)
        if result.returncode:
            raise RuntimeError(f"{stage} failed ({result.returncode}); see {log}")
        count = ready_frames(trace, stage)
        marker = "ASTRAL COMPLETE SAVE RELOADED:" if create else "ASTRAL COMPLETE SAVE VERIFIED:"
        if count < 18000 or marker not in log.read_text(errors="replace"):
            raise RuntimeError(f"{stage}: missing regular save readback or five minutes ready; see {log}")
        results.append({"stage": stage, "ready_frames": count, "seconds_minimum": count / 60,
                        "log": str(log), "trace": str(trace)})
        print(f"{stage}: PASS {count} ready frames; {log}", flush=True)
    receipt = json.loads((args.user / "complete-save.json").read_text())
    if receipt.get("stars") != 242 or not receipt.get("game_save_reload_verified"):
        raise RuntimeError("game-written receipt is incomplete")
    report = {"user": str(args.user.resolve()), "file": 1, "results": results,
              "save_provider": "GameDataFunction/GameDataHolder + SaveDataHandler normal writer/loader",
              "save_receipt": receipt, "live_progress_untouched": True}
    (output / "acceptance.json").write_text(json.dumps(report, indent=2) + "\n")
    print(f"Pick file 1 with play.sh --unlocked. Acceptance: {output / 'acceptance.json'}")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, RuntimeError, KeyError) as error:
        print(f"complete-save: {error}", file=sys.stderr)
        sys.exit(2)
