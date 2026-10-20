#!/bin/sh
# 60-second smoke of a frozen Astral Italian build: it must reach a menu scene (logo/title/file select) and then play
# the requested galaxy, without crashing.
#
#   smoke_frozen.sh [GALAXY] [--seconds N] [--odyssey] [--min-gameplay-frames N]
#
# GALAXY as in play.sh (default flip-swap). Same ASTRAL_APP / ASTRAL_DISC as play.sh; every run uses a fresh user
# directory under ASTRAL_SMOKE_ROOT (default ~/MacioGalaxy2-play/smoke), never a real one. The window opens in the
# background with SDL's dummy audio driver (PETARI_SMOKE_BACKGROUND=1) and takes no physical input.
# Evidence: the app's per-frame phase CSV (PETARI_FRAME_CSV: startup, loading, menu, gameplay), written when the app
# quits after SIGTERM (SDL turns it into a quit request, the console power button). Exit status 0 = PASS.
set -u

app=${ASTRAL_APP:-$HOME/MacioGalaxy2-play/Petari.app}
disc=${ASTRAL_DISC:-$HOME/MacioGalaxy2-gamedata/SB4E01-extracted}
root=${ASTRAL_SMOKE_ROOT:-$HOME/MacioGalaxy2-play/smoke}
galaxy=flip-swap
seconds=60
min_gameplay=120
mods=none

while [ $# -gt 0 ]; do
    case "$1" in
    --seconds) seconds=$2; shift 2 ;;
    --min-gameplay-frames) min_gameplay=$2; shift 2 ;;
    --odyssey) mods=OdysseyMovement; shift ;;
    -h|--help) sed -n '2,13p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    -*) echo "smoke: unknown option $1" >&2; exit 2 ;;
    *) galaxy=$1; shift ;;
    esac
done
case "$galaxy" in
flip-swap|flipswap|RedBlueExGalaxy) stage=RedBlueExGalaxy ;;
fluffy-bluff|fluffybluff|MokumokuValleyGalaxy) stage=MokumokuValleyGalaxy ;;
sky-station|skystation|IslandFleetGalaxy) stage=IslandFleetGalaxy ;;
*) stage=$galaxy ;;
esac

exe="$app/Contents/MacOS/Petari"
[ -x "$exe" ] || { echo "smoke: no app at $app" >&2; exit 2; }
[ -d "$disc/files" ] || { echo "smoke: no extracted disc at $disc" >&2; exit 2; }
run="$root/$(date +%Y%m%d-%H%M%S)-$stage"
mkdir -p "$run/user" || exit 2
log="$run/app.log"
csv="$run/frames.csv"

echo "smoke: $stage for ${seconds}s (mods $mods) -> $run"
PETARI_SMOKE_BACKGROUND=1 PETARI_FRAME_CSV="$csv" PETARI_FRAME_STATS=1 PETARI_MODS=$mods \
    "$exe" --disc "$disc" --user "$run/user" --stage "$stage" --scenario 1 > "$log" 2>&1 &
pid=$!

elapsed=0
alive=1
while [ "$elapsed" -lt "$seconds" ]; do
    sleep 1
    elapsed=$((elapsed + 1))
    kill -0 "$pid" 2>/dev/null || { alive=0; break; }
done
status=
if [ "$alive" = 1 ]; then
    kill -TERM "$pid" 2>/dev/null
    waited=0
    while kill -0 "$pid" 2>/dev/null && [ "$waited" -lt 20 ]; do sleep 1; waited=$((waited + 1)); done
    if kill -0 "$pid" 2>/dev/null; then
        kill -KILL "$pid" 2>/dev/null
        echo "smoke: the app did not quit within 20 s of SIGTERM (killed)"
    fi
fi
wait "$pid" 2>/dev/null
status=$?

fail=
[ "$alive" = 1 ] || fail="${fail}the app exited after ${elapsed}s (status $status) before the deadline; "
if grep -q "AddressSanitizer\|UndefinedBehaviorSanitizer\|Segmentation fault\| in \".*\" on line [0-9]*\." "$log"; then
    fail="${fail}a sanitizer report or OSPanic is in the log; "
fi
grep -q "stage launch: requested $stage" "$log" || fail="${fail}no 'stage launch: requested $stage' line (file select / request seam not reached); "
menu=0
gameplay=0
if [ -s "$csv" ]; then
    menu=$(awk -F, 'NR > 1 && $2 == "menu"' "$csv" | wc -l | tr -d ' ')
    gameplay=$(awk -F, 'NR > 1 && $2 == "gameplay"' "$csv" | wc -l | tr -d ' ')
    [ "$menu" -gt 0 ] || fail="${fail}no menu frame (title/file select never ready); "
    [ "$gameplay" -ge "$min_gameplay" ] || fail="${fail}only $gameplay gameplay frames (< $min_gameplay); "
else
    fail="${fail}no frame CSV (the app did not exit cleanly); "
fi
echo "smoke: menu frames $menu, gameplay frames $gameplay, exit status $status"
if [ -n "$fail" ]; then
    echo "smoke: FAIL: $fail"
    echo "smoke: last log lines:"
    tail -n 20 "$log"
    exit 1
fi
echo "smoke: PASS ($run)"
exit 0
