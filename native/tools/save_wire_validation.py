#!/usr/bin/env python3
"""Exercise Wii save round-trip and one-time native save migration in private test users."""
import argparse
from datetime import datetime
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
HOME = Path.home()
GAMEDATA = HOME / "MacioGalaxy2-gamedata"
TEST_USERS = HOME / "Library/Application Support/AstralItalian-unlocked/save-wire-validation"


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def source_save(path, legacy):
    path = path.expanduser().resolve(strict=True)
    allowed = (GAMEDATA.resolve(), (HOME / "Library/Application Support/AstralItalian-unlocked").resolve())
    if not any(path.is_relative_to(root) for root in allowed):
        raise ValueError("reference saves must stay in server game data or the isolated unlocked directory")
    data = path.read_bytes()
    order = "little" if legacy else "big"
    if len(data) < 16 or int.from_bytes(data[4:8], order) != 3:
        raise ValueError(f"not a {'legacy native' if legacy else 'Wii'} save: {path}")
    if not path.with_name("banner.bin").is_file():
        raise ValueError("the game's existing banner.bin is required beside each reference")
    return path, data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--reference-save", type=Path, required=True, help="read-only Dolphin GameData.bin")
    parser.add_argument("--legacy-save", type=Path, required=True, help="read-only pre-build6 native GameData.bin")
    parser.add_argument("--inputs", type=Path, required=True, help="explicit stage-start input tape")
    parser.add_argument("--frames", type=int, default=30000)
    parser.add_argument("--disc", type=Path, default=GAMEDATA / "SB4E01-extracted")
    args = parser.parse_args()
    if TEST_USERS.resolve() != TEST_USERS:
        raise ValueError("test user path must not redirect through a symlink")
    reference, wii = source_save(args.reference_save, False)
    legacy, old = source_save(args.legacy_save, True)
    originals = {str(path): digest(path) for path in (reference, legacy)}
    stamp = datetime.now().strftime("%Y%m%d-%H%M%S-%f")
    output = GAMEDATA / "save-wire-validation" / stamp
    output.mkdir(parents=True, exist_ok=False)
    results = []
    env = os.environ.copy()
    env.pop("ASTRAL_DEV_COMPLETE_SAVE", None)
    env.pop("ASTRAL_VERIFY_COMPLETE_SAVE", None)
    env["ASTRAL_SAVE_WIRE_ROUNDTRIP"] = "1"
    for name, source, expected in (("wii", reference, wii), ("legacy", legacy, old)):
        user = TEST_USERS / stamp / name
        data = user / "NAND/title/00010000/53423445/data"
        data.mkdir(parents=True, exist_ok=False)
        save = data / "GameData.bin"
        shutil.copyfile(source, save)
        shutil.copyfile(source.with_name("banner.bin"), data / "banner.bin")
        rounds = 2 if name == "legacy" else 1
        for round_index in range(rounds):
            label = f"{name}-{round_index + 1}"
            log = output / (label + ".log")
            trace = output / (label + ".csv")
            before = save.read_bytes()
            command = [sys.executable, str(ROOT / "native/tools/headless.py"), "run", "--executable", str(args.executable.resolve()),
                       "--inputs", str(args.inputs.expanduser().resolve()), "--frames", str(args.frames), "--epoch-ticks", "0",
                       "--disc", str(args.disc.expanduser().resolve()), "--user", str(user), "--existing-unlocked-user",
                       "--trace", str(trace), "--stage", "RedBlueExGalaxy"]
            with log.open("x") as stream:
                status = subprocess.run(["nice", "-n", "19", *command], env=env, stdout=stream, stderr=subprocess.STDOUT).returncode
            text = log.read_text(errors="replace")
            if status or "ASTRAL SAVE WII ROUNDTRIP: normal NAND file byte-identical" not in text:
                raise RuntimeError(f"{label}: game load/save check failed; see {log}")
            if "/Game/System/" in "\n".join(line for line in text.splitlines() if "runtime error:" in line):
                raise RuntimeError(f"{label}: save/system UBSan diagnostic; see {log}")
            after = save.read_bytes()
            if name == "wii" and after != expected:
                raise RuntimeError("Dolphin bytes changed through the game load/save")
            backup = save.with_name("GameData.bin.le-backup")
            if name == "legacy":
                if backup.read_bytes() != expected:
                    raise RuntimeError("migration did not preserve exact original backup")
                if int.from_bytes(after[4:8], "big") != 3:
                    raise RuntimeError("migration did not write Wii format")
                if round_index and (after != before or "ASTRAL SAVE MIGRATED:" in text):
                    raise RuntimeError("second load repeated or changed migration")
                if not round_index and "ASTRAL SAVE MIGRATED:" not in text:
                    raise RuntimeError("missing verified migration marker")
            elif backup.exists():
                raise RuntimeError("Wii-format input should not need a legacy backup")
            results.append({"test": label, "log": str(log), "trace": str(trace), "user": str(user),
                            "save_sha256": digest(save), "backup_sha256": digest(backup) if backup.exists() else None,
                            "normal_game_writer": "byte-identical round-trip marker"})
            print(f"{label}: PASS; {log}", flush=True)
    if any(digest(Path(path)) != sha for path, sha in originals.items()):
        raise RuntimeError("read-only source save changed")
    report = {"results": results, "reference_sources_unchanged": originals, "live_progress_touched": False,
              "sanitizers": {"ASAN_OPTIONS": env.get("ASAN_OPTIONS"), "system_save_diagnostics": "none",
                             "other_diagnostics": "consult full logs; no game-wide clean claim"}}
    (output / "acceptance.json").write_text(json.dumps(report, indent=2) + "\n")
    print(f"Save-wire acceptance: {output / 'acceptance.json'}")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, RuntimeError) as error:
        print(f"save-wire: {error}", file=sys.stderr)
        sys.exit(2)
