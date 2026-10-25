#!/bin/bash
# Exact M4 title arm: two immutable candidate directories containing Petari.app + BUILD.json.
set -uo pipefail
if [ "$#" -ne 4 ]; then
  echo "usage: $0 CANDIDATE_072_DIR CANDIDATE_542_DIR LABEL DISC_DIR" >&2
  exit 2
fi
SCRIPT_DIR=$(cd -- "$(dirname -- "$0")" && pwd)
case "$3" in ''|*[!A-Za-z0-9_-]*) echo 'invalid label' >&2; exit 2;; esac
OBS="$HOME/MacioGalaxy2-gamedata/port53/m1g/solb/$3"
HEAVY="$HOME/MacioGalaxy2-team/slots/server-heavy.lock"
NATIVE="$HOME/MacioGalaxy2-team/slots/native.lock"
BASE_ID=dev.petari.Petari.perf542title541
NEW_ID=dev.petari.Petari.perf542title542
python3 -B "$SCRIPT_DIR/prepare_title542.py" --candidate-dir "$1" --out "$OBS/apps/541" --bundle-id "$BASE_ID" || exit 2
python3 -B "$SCRIPT_DIR/prepare_title542.py" --candidate-dir "$2" --out "$OBS/apps/542" --bundle-id "$NEW_ID" || exit 2
status=0
for repeat in 1 2 3; do
  python3 -B "$SCRIPT_DIR/title542m.py" --tier m4 --app "$OBS/apps/541/Petari.app" --prep "$OBS/apps/541/PREP.json" \
    --out "$OBS/541-r$repeat" --label "server-072-title-r$repeat" --baseline --bundle-id "$BASE_ID" \
    --disc "$4" --heavy-lock "$HEAVY" --native-lock "$NATIVE" > "$OBS/541-r$repeat-driver.log" 2>&1 || status=1
  python3 -B "$SCRIPT_DIR/title542m.py" --tier m4 --app "$OBS/apps/542/Petari.app" --prep "$OBS/apps/542/PREP.json" \
    --out "$OBS/542-r$repeat" --label "server-542-title-r$repeat" --require-frontend-prepresent --bundle-id "$NEW_ID" \
    --require-deadline-stage FileSelect:69 --disc "$4" --heavy-lock "$HEAVY" --native-lock "$NATIVE" \
    > "$OBS/542-r$repeat-driver.log" 2>&1 || status=1
done
# Baseline split is intentionally unscorable. Its threshold exit is retained, not an acceptance claim.
python3 -B "$SCRIPT_DIR/gate542m.py" --tier m4 --window-seconds 300 --out "$OBS/541-SCORES.json" \
  "$OBS/541-r1" "$OBS/541-r2" "$OBS/541-r3" > "$OBS/541-scorer.log" 2>&1
baseline_score_exit=$?
echo "$baseline_score_exit" > "$OBS/541-scorer.exit"
python3 -B "$SCRIPT_DIR/gate542m.py" --tier m4 --title --require-frontend-prepresent --window-seconds 300 --require-deadline-stage FileSelect:69 \
  --out "$OBS/542-SCORES.json" "$OBS/542-r1" "$OBS/542-r2" "$OBS/542-r3" > "$OBS/542-scorer.log" 2>&1 || status=1
exit "$status"
