#!/bin/sh
# Double-click in Finder: a small menu that starts Astral Italian (native Super Mario Galaxy 2) in a galaxy.
# See PLAY.md next to this file. The paths can be changed with ASTRAL_APP / ASTRAL_DISC / ASTRAL_USER.
here=$(cd "$(dirname "$0")" && pwd)
play="$here/play.sh"
. "$here/frozen_app.sh"
app=$(frozen_app_validate "${ASTRAL_APP:-$HOME/MacioGalaxy2-play/build5.1/Petari.app}") || exit 1
export ASTRAL_APP="$app"
odyssey=
audio=${ASTRAL_AUDIO:-on}
save=
while :; do
    clear 2>/dev/null
    echo "Astral Italian (Super Mario Galaxy 2, native)"
    echo
    echo "  1) Flip-Swap Galaxy      (World 1)"
    echo "  2) Fluffy Bluff Galaxy   (World 1)"
    echo "  3) Sky Station Galaxy    (World 1, fallback)"
    echo "  4) Title screen          (normal start, file select)"
    echo
    echo "  Frozen build: ${ASTRAL_APP%/Petari.app}"
    echo "  c) 5.2.1 with Cappy"
    echo "  b) Choose another frozen build"
    echo "  o) Odyssey movement: $([ -n "$odyssey" ] && echo ON || echo off)"
    echo "  a) Audio: $audio"
    echo "  u) Save: $([ -n "$save" ] && echo 'everything unlocked (separate file 1)' || echo 'normal progress')"
    echo "  q) Quit"
    echo
    echo "  In the game: \` (the key between Escape and Tab) opens the Home menu: controls, mods."
    echo
    printf "Choice: "
    read -r choice || exit 0
    case "$choice" in
    1) galaxy=flip-swap ;;
    2) galaxy=fluffy-bluff ;;
    3) galaxy=sky-station ;;
    4) galaxy=title ;;
    c|C) selected_app=$(frozen_app_named build5.2.1) || exit 1
         export ASTRAL_APP="$selected_app"
         continue ;;
    b|B) frozen_app_list
         printf "Frozen build name (default or directory name): "
         read -r build_name || exit 0
         if selected_app=$(frozen_app_named "$build_name"); then
             export ASTRAL_APP="$selected_app"
         else
             printf "Press Return to continue. "
             read -r _ || exit 0
         fi
         continue ;;
    o|O) if [ -n "$odyssey" ]; then odyssey=; else odyssey=--odyssey; fi; continue ;;
    a|A) if [ "$audio" = off ]; then audio=on; else audio=off; fi; continue ;;
    u|U) if [ -n "$save" ]; then save=; else save=--unlocked; fi; continue ;;
    q|Q) exit 0 ;;
    *) continue ;;
    esac
    echo
    sh "$play" "$galaxy" $save $odyssey --audio "$audio"
    echo
    printf "Press Return for the menu. "
    read -r _ || exit 0
done
