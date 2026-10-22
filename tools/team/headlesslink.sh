#!/bin/sh
# Landing gate: petari_headless must still LINK (a GALAXY_NATIVE hook added to src/ with only a GX-side definition breaks
# the headless harness, as 5062b75f did). Incremental Release build (no sanitizers, no tests) of the one target in a
# persistent per-worktree directory, so later landings only rebuild what changed. Fails closed on configure or link errors.
# Usage (from the worktree root): sh tools/team/headlesslink.sh
set -u
dir=build/headless-link
log=build/headless-link.log
deps="$HOME/MacioGalaxy2-team/scratch-port/solA/build-ov-gx/_deps"
mkdir -p build
if [ ! -f "$dir/CMakeCache.txt" ]; then
  set -- -S . -B "$dir" -DCMAKE_BUILD_TYPE=Release -DPETARI_SANITIZERS=OFF -DBUILD_TESTING=OFF \
    -DPETARI_BUILD_HEADLESS=ON -DPETARI_BUILD_GX_PROBE=OFF
  if [ -d "$deps/imgui-src" ]; then
    set -- "$@" -DFETCHCONTENT_FULLY_DISCONNECTED=ON -DPETARI_IMGUI_SOURCE_DIR="$deps/imgui-src"
    for d in aurora dawn_prebuilt imgui sdl tracy; do
      u=$(echo "$d" | tr '[:lower:]' '[:upper:]')
      [ -d "$deps/$d-src" ] && set -- "$@" -DFETCHCONTENT_SOURCE_DIR_$u="$deps/$d-src"
    done
  fi
  # Older Apple clang (the M1's Xcode 16) needs these, as for every M1 native build.
  major=$(clang --version | sed -n 's/.*clang version \([0-9]*\).*/\1/p' | head -1)
  [ "${major:-99}" -lt 17 ] && set -- "$@" "-DCMAKE_CXX_FLAGS=-fexperimental-library -include TargetConditionals.h"
  nice -n 19 cmake "$@" > "$log" 2>&1 || { tail -20 "$log" >&2; echo "headless link gate: configure failed (log $log)" >&2; exit 1; }
fi
nice -n 19 cmake --build "$dir" --target petari_headless -j "${TEAM_BUILD_JOBS:-6}" >> "$log" 2>&1 \
  || { grep -E "Undefined symbols|undefined reference|error:|ld: " "$log" | tail -20 >&2; echo "headless link gate: petari_headless does not build/link (log $log)" >&2; exit 1; }
echo "headless link gate: petari_headless links"
