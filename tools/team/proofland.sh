#!/bin/sh
# Dispatch mode has its own fail-closed contract; the ordinary path is unchanged.
if [ "${DISPATCH_PROOF:-}" = 1 ]; then
  exec python3 tools/team/dispatchlanding.py land "$@"
fi
# Land a batch of source edits with a proof that is current at the moment of landing: take the landing lock
# FIRST (main cannot move), rebase onto main, prove every Wii object unchanged with two clean builds
# (tools/team/hookproof.sh main HEAD), then land with both locks still held (the landing build must not overlap
# another heavy build). A certificate (main SHA + src/include/libs tree hashes) skips re-proving an identical pair.
# Non-source WIP that helpers write into the shared worktree (native/, tools/, docs/) is committed before landing.
# Usage: tools/team/proofland.sh
set -e
who=$(git rev-parse --abbrev-ref HEAD)-proofland
lock="$HOME/MacioGalaxy2-team/slots/native.lock"
state() {
  [ -z "${PROOFLAND_STATE_HELPER:-}" ] || python3 "$PROOFLAND_STATE_HELPER" "$1" "${PROOFLAND_STATE_OUTPUT:-build/hookproof-land}" "$$" "" "$2"
}
wip() {
  git add -A native tools docs CMakeLists.txt CMakePresets.json 2>/dev/null || true
  git diff --cached --quiet || git commit -q -m "port: helper WIP (native/tools/docs only)"
  [ -z "$(git status --porcelain --untracked-files=no)" ] || { git status --short; echo "unexpected dirty files" >&2; exit 1; }
}
state waiting-landing-lock "resume proofland after confirming no other owned job survives"
pid=$(python3 tools/team/landlock.py acquire "$who")
trap 'python3 tools/team/landlock.py release >/dev/null 2>&1 || true' EXIT INT TERM HUP
wip
git rebase main || { git rebase --abort; echo "rebase conflict: resolve, then retry" >&2; exit 1; }
cert="$(git rev-parse main) $(git rev-parse HEAD:src) $(git rev-parse HEAD:include) $(git rev-parse HEAD:libs)"
if [ -n "${REMOTE_PROOF_RECEIPT:-}" ]; then
  [ -n "${REMOTE_PROOF_ROOT:-}" ] && [ -n "${REMOTE_PROOF_HOST:-}" ] || { echo "remote proof root and trusted host required" >&2; exit 1; }
  state waiting-remote-proof "verify genuine remote objects before consuming certificate"
  set -- python3 tools/team/remoteproof.py verify --receipt "$REMOTE_PROOF_RECEIPT" --proof-root "$REMOTE_PROOF_ROOT" --host "$REMOTE_PROOF_HOST"
  [ -z "${REMOTE_PROOF_EQUIVALENT_BASE:-}" ] || set -- "$@" --equivalent-base "$REMOTE_PROOF_EQUIVALENT_BASE"
  [ "${REMOTE_PROOF_APPROVED_NATIVE_52:-}" != 1 ] || set -- "$@" --approved-native-52
  [ "${REMOTE_PROOF_MARIO_52:-}" != 1 ] || set -- "$@" --mario-52
  lockf -k "$lock" nice -n 19 "$@"
  state remote-proof-accepted "continue final source guard and incremental landing"
  export REMOTE_PROOF_CONTRACT=build/remote-proof-contract.json
fi
if [ "$(cat build/hookproof-land.cert 2>/dev/null)" != "$cert" ]; then
  state waiting-local-proof "rerun clean local proof if interrupted"
  lockf -k "$lock" nice -n 19 sh tools/team/hookproof.sh build/hookproof-land main HEAD | tee build/hookproof-land.txt
  grep -q '^PROOF OK' build/hookproof-land.txt || { echo "proof failed: not landing" >&2; exit 1; }
  echo "$cert" > build/hookproof-land.cert
else
  echo "proof certificate current for $cert"
fi
# Inside the build lock: commit helper WIP that appeared while waiting, re-check the proven sources, land.
state waiting-incremental-landing "recheck contract, then incremental Wii build/hash guard and land"
LAND_LOCK_HELD=$pid CERT_TREES="${cert#* }" lockf -k "$lock" nice -n 19 sh -c '
  git add -A native tools docs CMakeLists.txt CMakePresets.json 2>/dev/null || true
  git diff --cached --quiet || git commit -q -m "port: helper WIP (native/tools/docs only)"
  [ -z "$(git status --porcelain --untracked-files=no)" ] || { git status --short; echo "unexpected dirty files" >&2; exit 1; }
  [ "$(git rev-parse HEAD:src) $(git rev-parse HEAD:include) $(git rev-parse HEAD:libs)" = "$CERT_TREES" ] \
    || { echo "sources changed after the proof" >&2; exit 1; }
  if [ -n "${REMOTE_PROOF_CONTRACT:-}" ]; then
    python3 tools/team/remoteproof.py check --contract "$REMOTE_PROOF_CONTRACT" || exit 1
  fi
  sh tools/team/land.sh'


state landing-complete "resume paused sweep controller after verifying processes and hashes"
