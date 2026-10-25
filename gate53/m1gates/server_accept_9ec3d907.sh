#!/bin/bash
# One server command file, NO outer heavy lock. Requires three immutable inputs:
#   bash server_accept_9ec3d907.sh BASELINE_DIR CANDIDATE_DIR DISC_DIR
# Each DIR contains Petari.app + BUILD.json. All outputs/perf copies stay in m1g.
# Direct arms x3/stage, alternating541/542:
#   unchanged970-frame perf_sweep_move.script, DIAG0,nice0,truecold
#   gate533.py unchanged, plus gate542m explicit --tier m4
#   deadline counts YosshiHomeGalaxy:183/MokumokuValleyGalaxy:168/MarioFaceShipGalaxy:74
#   new candidate --require-frontend-prepresent AND --require-boot-responsiveness.
# The latter wraps Popen with a PID AppleEvent heartbeat + owned-window PNG.
# These are driver checks: running gate542m on cold533 CSVs ALONE cannot prove OS response.
# Title x3 alternates541/542; same300s comparison window, baselinePIDquit315s,
# new candidate self-end, --tier m4 --require-frontend-prepresent --require-boot-responsiveness.
# Then one fresh prime/immediate warm second launch per build, exact private cache
# + same Metal namespace retained (--keep-cache / --warm-from). Repaired observer both.
# Separate541/542 perf IDs here; neither user's nor frozen Metal caches are touched.
set -euo pipefail
if [ "$#" -ne 3 ]; then
  echo "usage: bash $0 BASELINE_DIR CANDIDATE_DIR DISC_DIR" >&2
  exit 2
fi
SCRIPT_DIR=$(cd -- "$(dirname -- "$0")" && pwd)
exec python3 -B "$SCRIPT_DIR/accept542.py" \
  --baseline-dir "$1" --candidate-dir "$2" --disc "$3" --tier m4 \
  --out "$HOME/MacioGalaxy2-gamedata/port53/m1g/solb/server-9ec3d907-acceptance-r1" \
  --baseline-id dev.petari.Petari.perf542title541 \
  --candidate-id dev.petari.Petari.perf542title542 \
  --heavy-lock "$HOME/MacioGalaxy2-team/slots/server-heavy.lock" \
  --native-lock "$HOME/MacioGalaxy2-team/slots/native.lock"
