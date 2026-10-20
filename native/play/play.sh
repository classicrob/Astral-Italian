#!/bin/sh
# Play Astral Italian (the native Super Mario Galaxy 2 build) on the machine that holds the disc.
#
#   play.sh [GALAXY] [--scenario N] [--odyssey] [--audio on|off] [--dry-run]
#
# GALAXY: flip-swap (default), fluffy-bluff, sky-station, title (normal boot to the title screen), or any name
# `Petari --stage list` prints. Everything is configurable through the environment:
#   ASTRAL_APP   frozen app bundle    (default ~/MacioGalaxy2-play/Petari.app)
#   ASTRAL_DISC  extracted SB4E01 disc, the directory holding files/ (default ~/MacioGalaxy2-gamedata/SB4E01-extracted)
#   ASTRAL_USER  saves/settings/controls (default ~/MacioGalaxy2-play/user). Never the MacioGalaxy (SMG1) user
#                directory ~/Library/Application Support/Petari: the script refuses it.
#   ASTRAL_AUDIO on|off (default on; off uses SDL's dummy driver, which keeps the game's audio timing without a device)
#   ASTRAL_LOG   log file (default $ASTRAL_USER/logs/play-<galaxy>-<time>.log)
#   ASTRAL_CAPTURES  where F9 playtest captures go (default ~/MacioGalaxy2-play/captures)
set -u

app=${ASTRAL_APP:-$HOME/MacioGalaxy2-play/Petari.app}
disc=${ASTRAL_DISC:-$HOME/MacioGalaxy2-gamedata/SB4E01-extracted}
user=${ASTRAL_USER:-$HOME/MacioGalaxy2-play/user}
audio=${ASTRAL_AUDIO:-on}
galaxy=flip-swap
scenario=1
odyssey=0
dry=0

while [ $# -gt 0 ]; do
    case "$1" in
    --scenario) [ $# -ge 2 ] || { echo "play: --scenario needs a number" >&2; exit 2; }; scenario=$2; shift 2 ;;
    --odyssey) odyssey=1; shift ;;
    --audio) [ $# -ge 2 ] || { echo "play: --audio needs on or off" >&2; exit 2; }; audio=$2; shift 2 ;;
    --dry-run) dry=1; shift ;;
    -h|--help) sed -n '2,15p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    -*) echo "play: unknown option $1" >&2; exit 2 ;;
    *) galaxy=$1; shift ;;
    esac
done

# Internal stage names (from the disc's English GalaxyName.msbt; World 1).
case "$galaxy" in
flip-swap|flipswap|RedBlueExGalaxy) stage=RedBlueExGalaxy; label="Flip-Swap Galaxy" ;;
fluffy-bluff|fluffybluff|MokumokuValleyGalaxy) stage=MokumokuValleyGalaxy; label="Fluffy Bluff Galaxy" ;;
sky-station|skystation|IslandFleetGalaxy) stage=IslandFleetGalaxy; label="Sky Station Galaxy" ;;
title|boot) stage=; label="title screen" ;;
*) stage=$galaxy; label=$galaxy ;;
esac
case "$scenario" in ''|*[!0-9]*) echo "play: --scenario expects a number, got $scenario" >&2; exit 2 ;; esac
case "$audio" in on|off) ;; *) echo "play: audio must be on or off, got $audio" >&2; exit 2 ;; esac

exe="$app/Contents/MacOS/Petari"
[ -x "$exe" ] || { echo "play: no app at $app (set ASTRAL_APP to the frozen Petari.app)" >&2; exit 1; }
[ -d "$disc/files" ] || { echo "play: no extracted disc at $disc (expected $disc/files; set ASTRAL_DISC)" >&2; exit 1; }
# Refuse MacioGalaxy's (SMG1) user directory before creating anything (its saves must never be touched).
smg1_user=$(cd "$HOME/Library/Application Support" 2>/dev/null && pwd -P)/Petari
user_parent=$(cd "$(dirname "$user")" 2>/dev/null && pwd -P)
if [ "$user_parent/$(basename "$user")" = "$smg1_user" ]; then
    echo "play: $user is MacioGalaxy's (SMG1) user directory; use a separate ASTRAL_USER" >&2
    exit 1
fi
mkdir -p "$user/logs" || exit 1

set -- --disc "$disc" --user "$user"
[ -n "$stage" ] && set -- "$@" --stage "$stage" --scenario "$scenario"
log=${ASTRAL_LOG:-$user/logs/play-${stage:-title}-$(date +%Y%m%d-%H%M%S).log}

echo "Astral Italian: $label${stage:+ (mission $scenario)}; Odyssey movement $([ "$odyssey" = 1 ] && echo on || echo "as in mods.txt (off unless turned on in the Home menu: ` or F1, then Mods)"); audio $audio"
echo "  app  $app"
echo "  disc $disc"
echo "  user $user"
echo "  log  $log"
# Odyssey movement for this run only (PETARI_MODS overrides mods.txt); otherwise mods.txt in the user directory decides.
[ "$odyssey" = 1 ] && export PETARI_MODS=OdysseyMovement
[ "$audio" = off ] && export SDL_AUDIO_DRIVER=dummy
# F9 in the game saves a playtest capture (last 300 frames of input and Mario state, the state, a screenshot) here.
export PETARI_CAPTURE_DIR="${ASTRAL_CAPTURES:-$HOME/MacioGalaxy2-play/captures}" PETARI_CAPTURE_LOG="$log"
if [ "$dry" = 1 ]; then
    echo "  run  ${PETARI_MODS:+PETARI_MODS=$PETARI_MODS }${SDL_AUDIO_DRIVER:+SDL_AUDIO_DRIVER=$SDL_AUDIO_DRIVER }$exe $*"
    exit 0
fi
"$exe" "$@" > "$log" 2>&1
status=$?
if [ "$status" -ne 0 ]; then
    echo "The game exited with status $status. Last lines of $log:"
    tail -n 25 "$log"
fi
exit "$status"
