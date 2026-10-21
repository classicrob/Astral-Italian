#!/bin/sh
# Shared frozen-build discovery and validation for the player launchers.
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
    default) frozen_app_validate "$HOME/MacioGalaxy2-play/build5.2.1/Petari.app" ;;
    "5.2.1 with Cappy"|build5.2.1) frozen_app_validate "$HOME/MacioGalaxy2-play/build5.2.1/Petari.app" ;;
    "5.3"|"5.3 candidate"|build5.3) frozen_app_validate "$HOME/MacioGalaxy2-play/build5.3/Petari.app" ;;
    "5.2 with Cappy"|build5.2) frozen_app_validate "$HOME/MacioGalaxy2-play/build5.2/Petari.app" ;;
    build4) frozen_app_validate "$HOME/MacioGalaxy2-play/Petari.app" ;;
    ''|.|..|*[!A-Za-z0-9._-]*) echo "play: use default or a frozen build directory name" >&2; return 1 ;;
    *) frozen_app_validate "$HOME/MacioGalaxy2-play/$1/Petari.app" ;;
    esac
}

frozen_app_list() {
    frozen_seen_default=
    frozen_seen_fallback=
    for frozen_candidate in "$HOME/MacioGalaxy2-play/build5.2.1/Petari.app" "$HOME/MacioGalaxy2-play/build5.1/Petari.app" "$HOME/MacioGalaxy2-play/Petari.app" "$HOME/MacioGalaxy2-play/"*/Petari.app; do
        [ "$frozen_candidate" != "$frozen_seen_default" ] && [ "$frozen_candidate" != "$frozen_seen_fallback" ] || continue
        if frozen_verified=$(frozen_app_validate "$frozen_candidate" 2>/dev/null); then
            if [ "$frozen_candidate" = "$HOME/MacioGalaxy2-play/Petari.app" ]; then
                printf '  build4  %s\n' "$frozen_verified"
            else
                frozen_name=${frozen_candidate%/Petari.app}
                if [ "${frozen_name##*/}" = build5.1 ]; then
                    frozen_seen_fallback=$frozen_candidate
                    printf '  5.1 (fallback; build5.1)  %s\n' "$frozen_verified"
                elif [ "${frozen_name##*/}" = build5.2.1 ]; then
                    frozen_seen_default=$frozen_candidate
                    printf '  5.2.1 with Cappy (default; build5.2.1)  %s\n' "$frozen_verified"
                elif [ "${frozen_name##*/}" = build5.3 ]; then
                    printf '  5.3 candidate (not default; build5.3)  %s\n' "$frozen_verified"
                elif [ "${frozen_name##*/}" = build5.2 ]; then
                    printf '  5.2 with Cappy (build5.2)  %s\n' "$frozen_verified"
                else
                    printf '  %s  %s\n' "${frozen_name##*/}" "$frozen_verified"
                fi
            fi
        fi
    done
}
