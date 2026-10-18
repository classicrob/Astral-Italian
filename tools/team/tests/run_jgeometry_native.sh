#!/bin/sh
# Run actual-header native tests and check MWERKS preprocessing against HEAD.
set -eu
repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
cd "$repo_root"
test_dir=$(mktemp -d "${TMPDIR:-/tmp}/jgeometry-native.XXXXXX")
trap 'rm -rf "$test_dir"' EXIT HUP INT TERM
# CXX names an executable; use a wrapper script if extra compiler arguments are needed.
cxx=${CXX:-clang++}
for mode in optimized sanitized; do
    if [ "$mode" = optimized ]; then
        set -- -O2
    else
        set -- -O2 -g -fsanitize=address,undefined -fno-sanitize-recover=all
    fi
    "$cxx" -std=c++11 -ffp-contract=off "$@" \
        -Iinclude -Ilibs/JSystem/include -Ilibs/RVL_SDK/include \
        tools/team/tests/jgeometry_native.cpp \
        tools/team/tests/jgeometry_native_second.cpp -o "$test_dir/$mode"
    printf '%s: ' "$mode"
    "$test_dir/$mode"
done
# Only the baseline comparison uses a HEAD overlay. Native tests above use none.
mkdir -p "$test_dir/head/JSystem/JGeometry"
for header in TVec.hpp TUtil.hpp; do
    git show "HEAD:libs/JSystem/include/JSystem/JGeometry/$header" \
        > "$test_dir/head/JSystem/JGeometry/$header"
done
printf '#include "JSystem/JGeometry/TVec.hpp"\n' > "$test_dir/preprocess.cpp"
"$cxx" -std=c++11 -E -P -D__MWERKS__ -I"$test_dir/head" \
    -Iinclude -Ilibs/JSystem/include -Ilibs/RVL_SDK/include \
    "$test_dir/preprocess.cpp" > "$test_dir/head.i"
"$cxx" -std=c++11 -E -P -D__MWERKS__ \
    -Iinclude -Ilibs/JSystem/include -Ilibs/RVL_SDK/include \
    "$test_dir/preprocess.cpp" > "$test_dir/current.i"
cmp "$test_dir/head.i" "$test_dir/current.i"
printf '%s\n' 'MWERKS preprocessing: identical to HEAD'
