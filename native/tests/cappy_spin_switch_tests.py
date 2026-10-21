#!/usr/bin/env python3
"""Cappy x Spin on Flip-Swap's spin-switch area (SpinSwitchArea), run in the real game.

Acceptance runner (needs the private extracted disc and a built Petari; not part of CTest):

  python3 native/tests/cappy_spin_switch_tests.py --app Petari.app/Contents/MacOS/Petari \
      --disc ~/MacioGalaxy2-gamedata/SB4E01-extracted --out /private/scratch/dir

Each case boots RedBlueExGalaxy with OdysseyMovement + Cappy into a fresh user directory, types a key script
(PETARI_INPUT_SCRIPT) with Mario standing inside the stage's one SpinSwitchArea, and reads the
"[spin-switch] <name> source=cappy|spin before= after=" trace lines (PETARI_CAPPY_TRACE=1).

The rule: a real Spin flips the area once, that frame, whatever Cappy is doing; each throw flips it at most once.
A Spin pressed before the cap of an earlier throw reaches the area takes that contact with it (otherwise the
cap, arriving a few frames later, flips the area straight back and the Spin looks like it did nothing).
"""
import argparse
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


def run(app, disc, out, name, text):
    work = out / name
    (work / "user").mkdir(parents=True, exist_ok=True)
    (work / "user/mods.txt").write_text("OdysseyMovement=on\nCappy=on\n")
    keys = work / "keys.txt"
    keys.write_text(text)
    env = {"PATH": "/usr/bin:/bin", "HOME": str(Path.home()), "PETARI_CAPPY_TRACE": "1", "PETARI_INPUT_SCRIPT": str(keys),
           "PETARI_SMOKE": "", "PETARI_SMOKE_BACKGROUND": "1", "PETARI_FAST_BOOT": "1", "SDL_AUDIODRIVER": "dummy",
           "PETARI_SMOKE_FRAMES": "100000", "PETARI_STAGE_GATE_MS": "0"}
    log = work / "app.log"
    with log.open("wb") as f:
        proc = subprocess.Popen([app, "--disc", disc, "--user", str(work / "user"), "--stage", "RedBlueExGalaxy", "--scenario", "1"],
                                stdout=f, stderr=subprocess.STDOUT, env=env, start_new_session=True)
    try:
        deadline = time.time() + 400
        while time.time() < deadline and proc.poll() is None:
            if re.search(r"\[input-script\].*(done|stopped)", log.read_text(errors="replace")):
                break
            time.sleep(1)
    finally:
        proc.terminate()
        try:
            proc.wait(10)
        except subprocess.TimeoutExpired:
            proc.kill()
    text = log.read_text(errors="replace")
    if "[input-script] start" not in text:
        raise SystemExit(f"{name}: the script never started; see {log}")
    return [(m.group(1), int(m.group(2)), int(m.group(3))) for m in LINE.finditer(text)], len(re.findall(r"\[cappy\] mario spin \(Magic\) starts", text))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--app", required=True)
    ap.add_argument("--disc", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--only", help="comma separated case names")
    a = ap.parse_args()
    out = Path(a.out)
    only = set(a.only.split(",")) if a.only else None
    failed = 0
    for name, air, throw, spin_after, expect in CASES:
        if only and name not in only:
            continue
        toggles, spins = run(str(Path(a.app).resolve()), a.disc, out, name, script(air, throw, spin_after))
        sources = [t[0] for t in toggles]
        chained = all(t[2] == 1 - t[1] for t in toggles) and all(toggles[i][2] == toggles[i + 1][1] for i in range(len(toggles) - 1))
        want_spins = 1 if (not air and spin_after is not None) else 0
        ok = sources == expect and chained and spins == want_spins
        failed += not ok
        print(f"{'PASS' if ok else 'FAIL'} {name}: toggles {toggles} expected sources {expect}; Magic starts {spins} expected {want_spins}")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
