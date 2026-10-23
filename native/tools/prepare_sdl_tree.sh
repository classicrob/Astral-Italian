#!/bin/sh
# Offline/pinned builds: make the patched SDL source tree that FETCHCONTENT_SOURCE_DIR_SDL needs.
# FetchContent applies PATCH_COMMAND only to sources it downloads itself; a tree given with
# FETCHCONTENT_SOURCE_DIR_SDL is used as is. This runs the SAME patch driver (native/cmake/ApplySDLPatches.cmake,
# the repo's native/gx/sdl-patches/*.patch, with .petari-patches stamps) on a copy of a pristine release-3.4.10 tree
# (the tarball, or the tree Aurora's FetchContent populated, which already carries Aurora's two Android patches).
# Usage: native/tools/prepare_sdl_tree.sh <source tree> <output dir (must not exist)>
set -eu
src=$1; out=$2
here=$(cd "$(dirname "$0")/.." && pwd)
test -d "$src/src/joystick/hidapi" || { echo "not an SDL source tree: $src" >&2; exit 1; }
test ! -e "$out" || { echo "output exists: $out" >&2; exit 1; }
test ! -e "$src/.petari-patches" || { echo "source tree is already patched: $src" >&2; exit 1; }
cp -R "$src" "$out"
cmake "-DSDL_SOURCE_DIR=$out" "-DPATCH_DIR=$here/gx/sdl-patches" -P "$here/cmake/ApplySDLPatches.cmake"
ls "$out/.petari-patches"
