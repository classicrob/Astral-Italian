#!/usr/bin/env python3
"""Cappy x Spin on Flip-Swap's spin-switch area (SpinSwitchArea), run in the real game.

Acceptance runner (needs the private extracted disc and a built Petari; not part of CTest):

  python3 native/tests/cappy_spin_switch_tests.py --app Petari.app/Contents/MacOS/Petari \
      --disc ~/MacioGalaxy2-gamedata/SB4E01-extracted --out /private/scratch/dir

Pass --headless with --app pointing to petari_headless to run the same keyboard input
through virtual WPAD reports and the real SDK KPAD, without a window or rendering.

Each modded case boots RedBlueExGalaxy with OdysseyMovement + Cappy into a fresh user directory, types a key script
(PETARI_INPUT_SCRIPT) with Mario standing inside the stage's one SpinSwitchArea, and reads the
"[spin-switch] <name> source=cappy|spin before= after=" trace lines (PETARI_CAPPY_TRACE=1).

The rule: a real Spin flips the area once, that frame, whatever Cappy is doing; each throw flips it at most once.
A cap bounce restores Galaxy's airborne Spin allowance. Mods-off cases retain the original one-air-Spin limit.
A Spin pressed before the cap of an earlier throw reaches the area takes that contact with it (otherwise the
cap, arriving a few frames later, flips the area straight back and the Spin looks like it did nothing).
"""
import argparse
import hashlib
import json
import platform
import re
import subprocess
import sys
import time
from pathlib import Path

LINE = re.compile(r"\[spin-switch\] \S+ source=(cappy|spin) before=(\d) after=(\d)")
PRELUDE = 30  # idle frames after the script starts (the game is steady 60 frames before that)


def timeline(events, total):
    """{key: [(start, length)]} -> script lines '<frames> [keys]'."""
    frames = [set() for _ in range(total)]
    for key, spans in events.items():
        for start, length in spans:
            for f in range(start, min(start + length, total)):
                frames[f].add(key)
    out, cur, n = [], None, 0
    for keys in frames:
        text = " ".join(sorted(keys))
        if text == cur:
            n += 1
        else:
            if cur is not None:
                out.append(f"{n} {cur}".rstrip())
            cur, n = text, 1
    out.append(f"{n} {cur}".rstrip())
    return "\n".join(out) + "\n"


def script(air, throw, spin_after, total=170):
    if spin_after is not None and spin_after < 0:  # Spin first, the throw -spin_after frames later
        return timeline({"F": [(PRELUDE, 2)], "R": [(PRELUDE - spin_after, 2)]}, total)
    """Ground or air (jump held 30 frames, R 10 frames in). F is pressed spin_after frames after R, or alone."""
    ev = {}
    base = PRELUDE
    if air:
        ev["Space"] = [(PRELUDE, 30)]
        base = PRELUDE + 10
    if throw:
        ev["R"] = [(base, 2)]
    if spin_after is not None:
        ev["F"] = [(base + (spin_after if throw else 0), 2)]
    return timeline(ev, total)


# The cap flips the area at its release (throw frame 12); the Spin's own swing edge follows 2 frames after the Spin starts.
SPIN_OVER_THROW = ["cappy", "spin"]

# name, air, throw, spin_after, expected toggle sources in order. A ground Spin (pressed at all) must also start
# Galaxy's Magic status exactly once ("[cappy] mario spin (Magic) starts"): a real Spin, not a borrowed animation.
CASES = [
    ("ground_spin_alone", False, False, 0, ["spin"]),
    ("ground_throw_alone", False, True, None, ["cappy"]),
    ("ground_throw_then_spin_50", False, True, 50, ["cappy", "spin"]),  # the spin is after the throw's animation
    ("ground_spin_then_throw_50", False, True, -50, ["spin", "cappy"]),  # a throw after the spin counts once
    # Spin pressed over the ground throw: held until the cap has left the hand (throw frame 12), then Galaxy's own Spin
    ("ground_throw_then_spin_2", False, True, 2, SPIN_OVER_THROW),
    ("ground_throw_then_spin_6", False, True, 6, SPIN_OVER_THROW),
    ("ground_throw_then_spin_11", False, True, 11, SPIN_OVER_THROW),
    ("ground_throw_then_spin_12", False, True, 12, SPIN_OVER_THROW),
    ("ground_throw_then_spin_14", False, True, 14, SPIN_OVER_THROW),
    ("ground_throw_then_spin_20", False, True, 20, SPIN_OVER_THROW),
    ("air_spin_alone", True, False, 0, ["spin"]),
    ("air_throw_alone", True, True, None, ["cappy"]),
    ("air_throw_then_spin_2", True, True, 2, ["spin"]),   # spin first: it takes the cap's pending contact
    ("air_throw_then_spin_4", True, True, 4, ["spin"]),
    ("air_throw_then_spin_6", True, True, 6, ["spin"]),   # same frame as the cap's contact: one toggle, not two
    ("air_throw_then_spin_10", True, True, 10, ["cappy", "spin"]),  # the cap had already flipped it
    ("air_throw_then_spin_20", True, True, 20, ["cappy", "spin"]),
]



def bounce_script(rethrow=None, spin=100):
    # Running air throw, ground pound then Jump to dive onto the hovering cap.
    # The actual contact is around script frame 79; it recalls Cappy immediately.
    events = {"W": [(30, 70)], "Space": [(30, 10), (52, 2)], "R": [(36, 2)],
              "LeftShift": [(48, 2)], "F": [(spin, 2)]}
    if rethrow is not None:
        events["R"].append((rethrow, 2))
    return timeline(events, 155)


EXTRA_CASES = [
    ("air_bounce_then_spin", bounce_script(), ["cappy", "spin"], 1, 1, 1, "OdysseyMovement,Cappy"),
    ("air_bounce_rethrow_then_spin_2", bounce_script(90, 92), ["cappy", "spin"], 1, 2, 1, "OdysseyMovement,Cappy"),
    ("air_bounce_rethrow_then_spin_10", bounce_script(90, 100), ["cappy", "cappy", "spin"], 1, 2, 1, "OdysseyMovement,Cappy"),
    ("vanilla_ground_spin", script(False, False, 0), ["spin"], 0, 0, 0, "none"),
    ("vanilla_air_spin_once", timeline({"Space": [(30, 30)], "F": [(40, 2), (65, 2), (85, 2)]}, 105), ["spin"], 0, 0, 1, "none"),
    ("vanilla_throw_key_spins", script(True, True, None), ["spin"], 0, 0, 1, "none"),
]


def run(app, disc, out, name, text, *, headless=False, mods="OdysseyMovement,Cappy"):
    work = out / name
    (work / "user").mkdir(parents=True, exist_ok=True)
    (work / "cache").mkdir(exist_ok=True)
    keys = work / "keys.txt"
    keys.write_text(text)
    env = {"PATH": "/usr/bin:/bin", "HOME": str(Path.home()), "PETARI_CAPPY_TRACE": "1", "PETARI_MODS": mods, "PETARI_INPUT_SCRIPT": str(keys),
           "PETARI_SMOKE": "", "PETARI_SMOKE_BACKGROUND": "1", "PETARI_FAST_BOOT": "1", "SDL_AUDIODRIVER": "dummy",
           "PETARI_SMOKE_FRAMES": "100000", "PETARI_CACHE_DIR": str(work / "cache"), "PETARI_STAGE_GATE_MS": "0"}
    log = work / "app.log"
    command = [app, "--disc", disc, "--user", str(work / "user"), "--stage", "RedBlueExGalaxy", "--scenario", "1"]
    if headless:
        # Headless creates an isolated user itself; mods are explicit in the environment.
        (work / "user").rmdir()
        command += ["--keyboard-script", str(keys), "--trace", str(work / "trace.csv"), "--epoch-ticks", "0"]
    with log.open("wb") as f:
        proc = subprocess.Popen(command,
                                stdout=f, stderr=subprocess.STDOUT, env=env, start_new_session=True)
    try:
        deadline = time.time() + 400
        while time.time() < deadline and proc.poll() is None:
            if not headless and re.search(r"\[input-script\].*(done|stopped)", log.read_text(errors="replace")):
                break
            time.sleep(1)
    finally:
        if proc.poll() is None:
            proc.terminate()
        try:
            proc.wait(10)
        except subprocess.TimeoutExpired:
            proc.kill()
    text = log.read_text(errors="replace")
    if "[input-script] start" not in text:
        raise SystemExit(f"{name}: the script never started; see {log}")
    if headless and (proc.returncode != 0 or not (work / "trace.csv.complete").exists()):
        raise SystemExit(f"{name}: headless run did not complete successfully; see {log}")
    if "[input-script] done" not in text:
        raise SystemExit(f"{name}: script did not finish; see {log}")
    return [(m.group(1), int(m.group(2)), int(m.group(3))) for m in LINE.finditer(text)], len(re.findall(r"\[cappy\] mario spin \(Magic\) starts", text))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--app", required=True)
    ap.add_argument("--disc", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--headless", action="store_true", help="use petari_headless with real keyboard/WPAD input")
    ap.add_argument("--only", help="comma separated case names")
    a = ap.parse_args()
    out = Path(a.out)
    only = set(a.only.split(",")) if a.only else None
    failed = 0
    results = []
    for name, air, throw, spin_after, expect in CASES:
        if only and name not in only:
            continue
        toggles, spins = run(str(Path(a.app).resolve()), a.disc, out, name, script(air, throw, spin_after), headless=a.headless)
        sources = [t[0] for t in toggles]
        chained = all(t[2] == 1 - t[1] for t in toggles) and all(toggles[i][2] == toggles[i + 1][1] for i in range(len(toggles) - 1))
        want_spins = 1 if (not air and spin_after is not None) else 0
        ok = sources == expect and chained and spins == want_spins
        failed += not ok
        results.append({"name": name, "passed": ok, "toggles": toggles})
        print(f"{'PASS' if ok else 'FAIL'} {name}: toggles {toggles} expected sources {expect}; Magic starts {spins} expected {want_spins}", flush=True)
    for name, keys, expect, want_bounces, want_throws, want_air_spins, mods in EXTRA_CASES:
        if only and name not in only:
            continue
        toggles, spins = run(str(Path(a.app).resolve()), a.disc, out, name, keys, headless=a.headless, mods=mods)
        text = (out / name / "app.log").read_text(errors="replace")
        bounces = text.count("[cappy] bounce (")
        throws = text.count("[cappy] throw (")
        air_spins = text.count("[cappy] mario spin (air) starts")
        sources = [t[0] for t in toggles]
        chained = all(t[2] == 1 - t[1] for t in toggles) and all(toggles[i][2] == toggles[i + 1][1] for i in range(len(toggles) - 1))
        # These new air actions must follow a real contact and air throw in the run.
        after_bounce = not want_bounces or 0 <= text.find("[cappy] bounce (") < text.find("[cappy] mario spin (air) starts")
        ok = (sources == expect and chained and bounces == want_bounces and throws == want_throws and
              air_spins == want_air_spins and after_bounce and
              (not want_bounces or text.count("[cappy] throw (air)") == want_throws) and
              (want_throws < 2 or text.find("[cappy] bounce (") < text.rfind("[cappy] throw (air)") < text.find("[cappy] mario spin (air) starts")))
        failed += not ok
        results.append({"name": name, "passed": ok, "toggles": toggles})
        print(f"{'PASS' if ok else 'FAIL'} {name}: sources {sources} expected {expect}; bounces {bounces}/{want_bounces}, throws {throws}/{want_throws}, air Spins {air_spins}/{want_air_spins}", flush=True)
    receipt = {"host": platform.node(), "headless": a.headless, "app": str(Path(a.app).resolve()),
               "app_sha256": hashlib.sha256(Path(a.app).read_bytes()).hexdigest(),
               "passed": len(results) - failed, "failed": failed, "results": results}
    (out / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")
    print(f"{len(results) - failed}/{len(results)} passed; receipt: {out / 'receipt.json'}")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
