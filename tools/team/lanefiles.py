#!/usr/bin/env python3
"""Group paths (stdin, one per line) by owning lane, using progress.py's Game dir map; libraries go to m1.
Usage: git diff --name-only | python3 tools/team/lanefiles.py [--lane LANE]"""
import argparse
import sys

LANES = {"core": "Util LiveActor NameObj System Scene Gravity".split(),
         "objects": "MapObj Ride AreaObj Boss".split(),
         "enemies": ("Enemy RhythmLib Screen NPC Demo Camera Animation Effect AudioLib GameAudio Speaker NWC24 "
                     "Map").split(),
         "astra": ["Player"]}
INV = {d: lane for lane, dirs in LANES.items() for d in dirs}


def lane_of(path):
    parts = path.split("/")
    if parts[0] in ("src", "include") and len(parts) > 2 and parts[1] == "Game":
        return INV.get(parts[2], "core")
    if parts[0] == "include":
        return "core"
    if parts[0] in ("src", "libs"):
        return "m1"
    return "port"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--lane")
    args = ap.parse_args()
    groups = {}
    for line in sys.stdin:
        p = line.strip()
        if p:
            groups.setdefault(lane_of(p), []).append(p)
    if args.lane:
        print("\n".join(groups.get(args.lane, [])))
        return
    for lane in sorted(groups):
        print(f"## {lane} ({len(groups[lane])})")
        print("\n".join(groups[lane]))


if __name__ == "__main__":
    main()
