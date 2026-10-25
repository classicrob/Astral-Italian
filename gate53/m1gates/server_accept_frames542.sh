#!/bin/bash
# Server's ONE entrypoint: baseline/candidate Petari.app+BUILD dirs, disc, fresh output.
# Direct541/542 alternating x3 forYoshi/Fluffy/Ship, title541/542x3, warm prime/second
# each build. Nice0 asserted; per-arm heavy->native, no outer lock. No TCC requests.
# Native v2 arms REQUIRE frontend-ready + <=100ms main-loop gap + owned on-screen
# window + actual in-app surface PNG text/bar + same-CSV presentation coverage.
# AppleEvent sends are no-prompt observations only. M4 tier stays<=22ms post100.
# gate533 unchanged; gate542m --tier m4, exact target deadline counts retained.
set -euo pipefail
if [ "$#" -ne 4 ]; then
  echo "usage: bash $0 BASELINE_DIR CANDIDATE_DIR DISC_DIR NEW_OUTPUT_DIR" >&2
  exit 2
fi
SCRIPT_DIR=$(cd -- "$(dirname -- "$0")" && pwd)
exec python3 -B "$SCRIPT_DIR/accept542.py" \
  --baseline-dir "$1" --candidate-dir "$2" --disc "$3" --tier m4 --out "$4" \
  --baseline-id dev.petari.Petari.perf542title541 \
  --candidate-id dev.petari.Petari.perf542title542 \
  --heavy-lock "$HOME/MacioGalaxy2-team/slots/server-heavy.lock" \
  --native-lock "$HOME/MacioGalaxy2-team/slots/native.lock"
