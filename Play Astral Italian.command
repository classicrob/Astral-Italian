#!/bin/zsh
# Double-click to play. Optional first argument: a galaxy (internal name, alias or English name),
# e.g.  "Play Astral Italian.command" fluffy-bluff-galaxy   (default: Flip-Swap Galaxy).
# Extra arguments are passed to the game (e.g. --scenario 2). See PLAY.md.
set -eu

repo_dir="$(cd -- "$(dirname -- "$0")" && pwd -P)"
app="$repo_dir/build/macos-arm64-release/native/app/Petari.app/Contents/MacOS/Petari"
disc="${GALAXY_GAME_DIR:-$HOME/MacioGalaxy2-gamedata/SB4E01-extracted}"
user_dir="${GALAXY_USER_DIR:-$HOME/MacioGalaxy2-gamedata/play-user}"
galaxy="RedBlueExGalaxy"
if [[ $# -gt 0 && "$1" != --* ]]; then
    galaxy="$1"
    shift
fi

if [[ ! -x "$app" ]]; then
    print -u2 -- "Build the game first (see PLAY.md): cmake --build build/macos-arm64-release --target petari -j 8"
    exit 1
fi
if [[ ! -d "$disc/files" ]]; then
    print -u2 -- "Extracted disc not found at $disc (expected files/ and sys/). Set GALAXY_GAME_DIR to its location."
    exit 1
fi

mkdir -p -- "$user_dir"
cd -- "$repo_dir"
exec "$app" --disc "$disc" --user "$user_dir" --stage "$galaxy" "$@"
