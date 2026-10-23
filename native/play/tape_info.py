#!/usr/bin/env python3
"""Facts about a recorded tape for the launchers (play.sh --replay): tape_info.py {newest|sha|mods|summary|builds} <dir>.

  newest <tapes>   the newest tape folder under <tapes> (one that has a route.tape.json)
  sha <tape>       the sha256 of the executable that recorded the tape
  mods <tape>      the PETARI_MODS value giving exactly the recorded mods (default-on mods that were off explicitly off)
  summary <tape>   one line: segments, frames, complete
  builds <sha>     the frozen builds under ~/MacioGalaxy2-play whose executable has that sha256 (names, one per line)
"""
import hashlib
import json
import os
import sys

DEFAULT_ON = ("OdysseyJumpDive",)  # on without a mods.txt line (native/input/mods.cpp kDefaults); same as tools/port/replay_tape.py


def sidecar(path):
    p = os.path.join(path, "route.tape.json")
    with open(p) as f:
        return json.load(f)


def main(argv):
    if len(argv) != 3 or argv[1] not in ("newest", "sha", "mods", "summary", "builds"):
        sys.stderr.write(__doc__)
        return 2
    cmd, arg = argv[1], argv[2]
    if cmd == "newest":
        tapes = [os.path.join(arg, d) for d in os.listdir(arg) if os.path.isfile(os.path.join(arg, d, "route.tape.json"))]
        if not tapes:
            return 1
        print(max(tapes, key=lambda t: os.path.getmtime(os.path.join(t, "route.tape.json"))))
    elif cmd == "sha":
        print(sidecar(arg).get("exe_sha256", "unavailable"))
    elif cmd == "mods":
        mods = list(sidecar(arg).get("mods", []))
        items = mods + [f"-{m}" for m in DEFAULT_ON if m not in mods]
        print(",".join(items) if items else "none")
    elif cmd == "summary":
        meta = sidecar(arg)
        segs = meta.get("segments", [])
        frames = sum(max(0, g["end_row"] - g["start_row"]) for g in segs)
        print(f"{len(segs)} segment(s), {frames} frames ({frames / 60:.0f} s), {'complete' if meta.get('complete') else 'not closed cleanly'}")
    else:
        root = os.path.expanduser("~/MacioGalaxy2-play")
        for d in sorted(os.listdir(root)):
            exe = os.path.join(root, d, "Petari.app", "Contents", "MacOS", "Petari")
            if os.path.isfile(exe):
                h = hashlib.sha256()
                with open(exe, "rb") as f:
                    for chunk in iter(lambda: f.read(1 << 20), b""):
                        h.update(chunk)
                if h.hexdigest() == arg:
                    print(d)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
