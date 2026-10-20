#!/bin/sh
# Land a batch of source edits with a proof that is current at the moment of landing: take the landing lock
# FIRST (main cannot move), rebase onto main, prove every Wii object unchanged with two clean builds
# (tools/team/hookproof.sh main HEAD), then land with both locks still held (the landing build must not overlap
# another heavy build). A certificate (main SHA + src/include/libs tree hashes) skips re-proving an identical pair.
# Non-source WIP that helpers write into the shared worktree (native/, tools/, docs/) is committed before landing.
# Usage: tools/team/proofland.sh
set -e
who=$(git rev-parse --abbrev-ref HEAD)-proofland
lock="$HOME/MacioGalaxy2-team/slots/native.lock"
wip() {
  git add -A native tools docs CMakeLists.txt CMakePresets.json 2>/dev/null || true
  git diff --cached --quiet || git commit -q -m "port: helper WIP (native/tools/docs only)"
  [ -z "$(git status --porcelain --untracked-files=no)" ] || { git status --short; echo "unexpected dirty files" >&2; exit 1; }
}
pid=$(python3 tools/team/landlock.py acquire "$who")
trap 'python3 tools/team/landlock.py release >/dev/null 2>&1 || true' EXIT INT TERM HUP
wip
git rebase main || { git rebase --abort; echo "rebase conflict: resolve, then retry" >&2; exit 1; }
cert="$(git rev-parse main) $(git rev-parse HEAD:src) $(git rev-parse HEAD:include) $(git rev-parse HEAD:libs)"
if [ "$(cat build/hookproof-land.cert 2>/dev/null)" != "$cert" ]; then
  lockf -k "$lock" sh tools/team/hookproof.sh build/hookproof-land main HEAD | tee build/hookproof-land.txt
  grep -q '^PROOF OK' build/hookproof-land.txt || { echo "proof failed: not landing" >&2; exit 1; }
  echo "$cert" > build/hookproof-land.cert
else
  echo "proof certificate current for $cert"
fi
# Inside the build lock: commit helper WIP that appeared while waiting, re-check the proven sources, land.
LAND_LOCK_HELD=$pid CERT_TREES="${cert#* }" lockf -k "$lock" sh -c '
  git add -A native tools docs CMakeLists.txt CMakePresets.json 2>/dev/null || true
  git diff --cached --quiet || git commit -q -m "port: helper WIP (native/tools/docs only)"
  [ -z "$(git status --porcelain --untracked-files=no)" ] || { git status --short; echo "unexpected dirty files" >&2; exit 1; }
  [ "$(git rev-parse HEAD:src) $(git rev-parse HEAD:include) $(git rev-parse HEAD:libs)" = "$CERT_TREES" ] \
    || { echo "sources changed after the proof" >&2; exit 1; }
  sh tools/team/land.sh'

