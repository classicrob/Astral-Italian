#!/bin/sh
# Play Astral Italian (the native Super Mario Galaxy 2 build) on the machine that holds the disc.
#
#   play.sh [--build NAME | --list-builds] [GALAXY] [--scenario N] [--unlocked] [--odyssey] [--audio on|off] [--dry-run]
#   play.sh --replay [TAPE-DIR] [--replay-any-build]   watch a recorded session replayed (newest tape if none given), see below
#
# GALAXY: flip-swap (default), fluffy-bluff, sky-station, title (normal boot to the title screen), or any name
# `Petari --stage list` prints. Everything is configurable through the environment:
#   ASTRAL_APP   frozen app bundle    (default ~/MacioGalaxy2-play/build5.4.1/Petari.app; frozen_app.sh frozen_default)
#   ASTRAL_DISC  extracted SB4E01 disc, the directory holding files/ (default ~/MacioGalaxy2-gamedata/SB4E01-extracted)
#   ASTRAL_USER  saves/settings/controls (default ~/MacioGalaxy2-play/user). Never the MacioGalaxy (SMG1) user
#                directory ~/Library/Application Support/Petari: the script refuses it.
#   ASTRAL_AUDIO on|off (default on; off uses SDL's dummy driver, which keeps the game's audio timing without a device)
#   ASTRAL_LOG   log file (default $ASTRAL_USER/logs/play-<galaxy>-<time>.log)
#   ASTRAL_TAPES  where the exact tape of each session goes (default ~/MacioGalaxy2-play/tapes); ASTRAL_TAPE=off disables it
#   ASTRAL_CAPTURES  where F9 playtest captures go (default ~/MacioGalaxy2-play/captures)
#   --replay: runs the tape in this window (physical input is ignored; close the window to stop). It runs the chosen build only if
#   that build's executable is the one that recorded the tape (exe_sha256 in the tape); otherwise it says which build is needed
#   and stops (--replay-any-build runs it anyway, and the drift report shows where it leaves your route). The replay uses a
#   throwaway copy of your save (ASTRAL_REPLAY_USER, default ~/MacioGalaxy2-play/user-replay), never your own, and writes
#   <tape>-replay-<time>/ (a copy of the tape plus divergence.json / divergence.txt) next to the tape.
set -u
here=$(cd "$(dirname "$0")" && pwd)
. "$here/frozen_app.sh"

app=${ASTRAL_APP:-$HOME/MacioGalaxy2-play/$frozen_default/Petari.app}
disc=${ASTRAL_DISC:-$HOME/MacioGalaxy2-gamedata/SB4E01-extracted}
user=${ASTRAL_USER:-$HOME/MacioGalaxy2-play/user}
audio=${ASTRAL_AUDIO:-on}
galaxy=flip-swap
scenario=1
odyssey=0
dry=0
unlocked=0
replay=
replay_any=0

while [ $# -gt 0 ]; do
    case "$1" in
    --build) [ $# -ge 2 ] || { echo "play: --build needs a name" >&2; exit 2; }; app=$(frozen_app_named "$2") || exit 1; shift 2 ;;
    --list-builds) frozen_app_list; exit 0 ;;
    --scenario) [ $# -ge 2 ] || { echo "play: --scenario needs a number" >&2; exit 2; }; scenario=$2; shift 2 ;;
    --odyssey) odyssey=1; shift ;;
    --unlocked) user="$HOME/Library/Application Support/AstralItalian-unlocked"; unlocked=1; shift ;;
    --audio) [ $# -ge 2 ] || { echo "play: --audio needs on or off" >&2; exit 2; }; audio=$2; shift 2 ;;
    --replay) replay=newest; shift
              case "${1:-}" in ''|-*) ;; *) if [ -d "$1" ]; then replay=$1; shift; fi ;; esac ;;
    --replay-any-build) replay_any=1; shift ;;
    --dry-run) dry=1; shift ;;
    -h|--help) sed -n '2,23p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
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

app=$(frozen_app_validate "$app") || exit 1
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
if [ "$unlocked" = 1 ]; then
    [ ! -L "$user" ] && [ -f "$user/complete-save.json" ] || {
        echo "play: no generated unlocked save in $user; run native/tools/complete_save.py first" >&2
        exit 1
    }
fi
mkdir -p "$user/logs" || exit 1

if [ -n "$replay" ]; then
    # Replay a recorded tape (native/PLAYTEST.md "Replaying your tape"): only with the executable that recorded it.
    tapes="${ASTRAL_TAPES:-$HOME/MacioGalaxy2-play/tapes}"
    if [ "$replay" = newest ]; then
        replay=$(python3 -I "$here/tape_info.py" newest "$tapes") || { echo "play: no tape under $tapes" >&2; exit 1; }
    fi
    replay=$(cd "$replay" 2>/dev/null && pwd) && [ -f "$replay/route.tape.json" ] || { echo "play: no tape at ${replay:-the given folder}" >&2; exit 1; }
    want=$(python3 -I "$here/tape_info.py" sha "$replay") || { echo "play: cannot read $replay/route.tape.json" >&2; exit 1; }
    have=$(shasum -a 256 "$exe" | cut -d' ' -f1)
    if [ "$want" != "$have" ]; then
        found=$(python3 -I "$here/tape_info.py" builds "$want" | tr '\n' ' ')
        echo "play: this tape was recorded by a different build (exe sha256 ${want%"${want#????????}"}...; this one is ${have%"${have#????????}"}...)." >&2
        if [ -n "$found" ]; then echo "      It needs: $found(run with --build NAME). Not using another build." >&2
        else echo "      The build that recorded it is not among the frozen builds. Not using another build." >&2; fi
        if [ "$replay_any" != 1 ]; then echo "      (--replay-any-build runs it anyway; expect drift, the report shows where.)" >&2; exit 1; fi
        echo "play: --replay-any-build: replaying with a different build; the divergence report shows the drift." >&2
    fi
    export PETARI_MODS="$(python3 -I "$here/tape_info.py" mods "$replay")"
    # A throwaway save: a replay can collect stars and write the file; it must never touch your own.
    ruser=${ASTRAL_REPLAY_USER:-$HOME/MacioGalaxy2-play/user-replay}
    if [ -f "$ruser/.astral-replay-user" ]; then rm -rf "$ruser"; fi
    [ ! -e "$ruser" ] || { echo "play: $ruser exists and is not a replay folder; set ASTRAL_REPLAY_USER" >&2; exit 1; }
    mkdir -p "$ruser" && cp -R "$user/NAND" "$ruser/NAND" 2>/dev/null; : > "$ruser/.astral-replay-user"
    mkdir -p "$ruser/logs"
    user=$ruser
    stage=; label="replay of $(basename "$replay") ($(python3 -I "$here/tape_info.py" summary "$replay"))"
    export PETARI_CACHE_DIR="${PETARI_CACHE_DIR:-$ruser/cache}"
    ASTRAL_TAPE=off
    set -- --disc "$disc" --user "$user" --replay-tape "$replay"
else
set -- --disc "$disc" --user "$user"
[ -n "$stage" ] && set -- "$@" --stage "$stage" --scenario "$scenario"
fi
log=${ASTRAL_LOG:-$user/logs/play-${stage:-title}-$(date +%Y%m%d-%H%M%S).log}

echo "Astral Italian: $label${stage:+ (mission $scenario)}; Odyssey movement $([ "$odyssey" = 1 ] && echo on || echo "as in mods.txt (off unless turned on in the Home menu: \` or F1, then Mods)"); audio $audio"
echo "  app  $app"
echo "  disc $disc"
echo "  user $user"
echo "  log  $log"
# Add Odyssey movement for this run, preserving the other choices in mods.txt or PETARI_MODS.
[ "$odyssey" = 1 ] && export PETARI_ODYSSEY_MOVEMENT=1
[ "$audio" = off ] && export SDL_AUDIO_DRIVER=dummy
# F9 in the game saves a playtest capture (last 300 frames of input and Mario state, the state, a screenshot) here.
# Exact tape (builds from 5.3 on; older builds ignore it): every session writes <tapes>/<stage>-<time>/route.csv, route.kpd (the
# game's own KPAD reads), route.actions (mod buttons) and route.tape.json, so it can be replayed headless to the same frames
# (tools/port/replay_tape.py, native/PLAYTEST.md). ~150 MB an hour. ASTRAL_TAPE=off turns it off; ASTRAL_TAPES moves the folder.
if [ "${ASTRAL_TAPE:-on}" != off ]; then
    tapes="${ASTRAL_TAPES:-$HOME/MacioGalaxy2-play/tapes}"
    # Bounded: keep the 19 newest session tapes plus this one; older ones are removed (copy a tape worth keeping elsewhere).
    if [ -d "$tapes" ]; then
        ls -1dt "$tapes"/*/ 2>/dev/null | tail -n +20 | while IFS= read -r old; do rm -rf "${old%/}"; done
    fi
    export PETARI_TAPE_RECORD="$tapes/${stage:-title}-$(date +%Y%m%d-%H%M%S)"
fi
export PETARI_CAPTURE_DIR="${ASTRAL_CAPTURES:-$HOME/MacioGalaxy2-play/captures}" PETARI_CAPTURE_LOG="$log"
if [ "$dry" = 1 ]; then
    echo "  run  ${PETARI_TAPE_RECORD:+PETARI_TAPE_RECORD=$PETARI_TAPE_RECORD }${PETARI_MODS:+PETARI_MODS=$PETARI_MODS }${PETARI_ODYSSEY_MOVEMENT:+PETARI_ODYSSEY_MOVEMENT=$PETARI_ODYSSEY_MOVEMENT }${SDL_AUDIO_DRIVER:+SDL_AUDIO_DRIVER=$SDL_AUDIO_DRIVER }$exe $*"
    exit 0
fi
"$exe" "$@" > "$log" 2>&1
status=$?
if [ "$status" -ne 0 ]; then
    echo "The game exited with status $status. Last lines of $log:"
    tail -n 25 "$log"
fi
exit "$status"
