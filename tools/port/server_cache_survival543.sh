#!/bin/bash
# Run in PORT's quiet M4 window, from bash at nice0. No host/AX input or TCC.
set -euo pipefail
if [ "$#" -ne 6 ]; then
    echo 'usage: server_cache_survival543.sh A_BUILD_DIR B_BUILD_DIR NEG_BUILD_DIR NEW_OUT_DIR DISC FIXTURE' >&2
    exit 2
fi
repo=$(git rev-parse --show-toplevel)
exec python3 -B "$repo/tools/port/cache_build_survival543.py" \
    --a-build-dir "$1" --b-build-dir "$2" --negative-build-dir "$3" \
    --out "$4" --disc "$5" --fixture "$6" \
    --heavy-lock "$HOME/MacioGalaxy2-team/slots/server-heavy.lock" \
    --native-lock "$HOME/MacioGalaxy2-team/slots/native.lock"
