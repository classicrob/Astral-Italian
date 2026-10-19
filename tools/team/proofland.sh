#!/bin/sh
# Land a batch of source edits with a proof that is current at the moment of landing: take the landing lock
# FIRST (main cannot move), rebase onto main, prove every Wii object unchanged with two clean builds
# (tools/team/hookproof.sh main HEAD), then land with the lock still held. Usage: tools/team/proofland.sh
set -e
who=$(git rev-parse --abbrev-ref HEAD)-proofland
pid=$(python3 tools/team/landlock.py acquire "$who")
trap 'python3 tools/team/landlock.py release >/dev/null 2>&1 || true' EXIT INT TERM HUP
git rebase main || { git rebase --abort; echo "rebase conflict: resolve, then retry" >&2; exit 1; }
lockf -k "$HOME/MacioGalaxy2-team/slots/native.lock" sh tools/team/hookproof.sh build/hookproof-land main HEAD \
  | tee build/hookproof-land.txt
grep -q '^PROOF OK' build/hookproof-land.txt || { echo "proof failed: not landing" >&2; exit 1; }
LAND_LOCK_HELD=$pid sh tools/team/land.sh
