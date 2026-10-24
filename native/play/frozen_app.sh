#!/bin/sh
# Shared frozen-build discovery and validation for the player launchers.
# The default frozen build (head promoted 5.4.1 on 2026-10-24); 5.4 and 5.3.2 stay as fallbacks in the menu (5.2.1 too).
frozen_default=build5.4.1
frozen_app_validate() {
    frozen_root=$(cd "$HOME/MacioGalaxy2-play" 2>/dev/null && pwd -P) || {
        echo "play: no frozen builds under ~/MacioGalaxy2-play" >&2; return 1;
    }
    frozen_app=$(cd "$1" 2>/dev/null && pwd -P) || {
        echo "play: no app at $1" >&2; return 1;
    }
    case "$frozen_app" in
    "$frozen_root"/*/Petari.app|"$frozen_root"/Petari.app) ;;
    *) echo "play: app must be a frozen build under ~/MacioGalaxy2-play" >&2; return 1 ;;
    esac
    [ -f "${frozen_app%/Petari.app}/FROZEN.txt" ] || {
        echo "play: build is not frozen (missing FROZEN.txt beside Petari.app)" >&2; return 1;
    }
    [ -x "$frozen_app/Contents/MacOS/Petari" ] && [ ! -L "$frozen_app/Contents/MacOS/Petari" ] || {
        echo "play: frozen Petari executable missing or is a symlink" >&2; return 1;
    }
    frozen_exe_dir=$(cd "$frozen_app/Contents/MacOS" 2>/dev/null && pwd -P) || return 1
    [ "$frozen_exe_dir" = "$frozen_app/Contents/MacOS" ] || {
        echo "play: executable directory escapes the frozen app" >&2; return 1;
    }
    printf '%s\n' "$frozen_app"
}

frozen_app_named() {
    case "$1" in
    default) frozen_app_validate "$HOME/MacioGalaxy2-play/$frozen_default/Petari.app" ;;
    "5.4.1"|build5.4.1) frozen_app_validate "$HOME/MacioGalaxy2-play/build5.4.1/Petari.app" ;;
    "5.4"|build5.4) frozen_app_validate "$HOME/MacioGalaxy2-play/build5.4/Petari.app" ;;
    "5.3.2"|build5.3.2) frozen_app_validate "$HOME/MacioGalaxy2-play/build5.3.2/Petari.app" ;;
    "5.2.1 with Cappy"|build5.2.1) frozen_app_validate "$HOME/MacioGalaxy2-play/build5.2.1/Petari.app" ;;
    "5.3"|"5.3 candidate"|build5.3) frozen_app_validate "$HOME/MacioGalaxy2-play/build5.3/Petari.app" ;;
    "5.2 with Cappy"|build5.2) frozen_app_validate "$HOME/MacioGalaxy2-play/build5.2/Petari.app" ;;
    build4) frozen_app_validate "$HOME/MacioGalaxy2-play/Petari.app" ;;
    ''|.|..|*[!A-Za-z0-9._-]*) echo "play: use default or a frozen build directory name" >&2; return 1 ;;
    *) frozen_app_validate "$HOME/MacioGalaxy2-play/$1/Petari.app" ;;
    esac
}

frozen_app_list() {
    frozen_seen=" "
    for frozen_candidate in "$HOME/MacioGalaxy2-play/$frozen_default/Petari.app" "$HOME/MacioGalaxy2-play/build5.4/Petari.app" \
        "$HOME/MacioGalaxy2-play/build5.3.2/Petari.app" \
        "$HOME/MacioGalaxy2-play/build5.2.1/Petari.app" "$HOME/MacioGalaxy2-play/build5.1/Petari.app" \
        "$HOME/MacioGalaxy2-play/Petari.app" "$HOME/MacioGalaxy2-play/"*/Petari.app; do
        case "$frozen_seen" in *" $frozen_candidate "*) continue ;; esac
        frozen_seen="$frozen_seen$frozen_candidate "
        if frozen_verified=$(frozen_app_validate "$frozen_candidate" 2>/dev/null); then
            if [ "$frozen_candidate" = "$HOME/MacioGalaxy2-play/Petari.app" ]; then
                printf '  build4  %s\n' "$frozen_verified"
            else
                frozen_name=${frozen_candidate%/Petari.app}
                if [ "${frozen_name##*/}" = "$frozen_default" ]; then
                    printf '  %s (default; %s)  %s\n' "${frozen_default#build}" "$frozen_default" "$frozen_verified"
                elif [ "${frozen_name##*/}" = build5.4 ]; then
                    printf '  5.4 (fallback; build5.4)  %s\n' "$frozen_verified"
                elif [ "${frozen_name##*/}" = build5.3.2 ]; then
                    printf '  5.3.2 (fallback; build5.3.2)  %s\n' "$frozen_verified"
                elif [ "${frozen_name##*/}" = build5.1 ]; then
                    printf '  5.1 (old fallback; build5.1)  %s\n' "$frozen_verified"
                elif [ "${frozen_name##*/}" = build5.2.1 ]; then
                    printf '  5.2.1 with Cappy (fallback; build5.2.1)  %s\n' "$frozen_verified"
                elif [ "${frozen_name##*/}" = build5.3 ]; then
                    printf '  5.3 candidate (not default; pad untested; build5.3)  %s\n' "$frozen_verified"
                elif [ "${frozen_name##*/}" = build5.2 ]; then
                    printf '  5.2 with Cappy (build5.2)  %s\n' "$frozen_verified"
                else
                    printf '  %s  %s\n' "${frozen_name##*/}" "$frozen_verified"
                fi
            fi
        fi
    done
}
