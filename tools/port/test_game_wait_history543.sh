#!/bin/bash
# SERVER only during SOL A's exclusive M1 cohort. Caller owns the test slot.
set -euo pipefail
build=${1:?usage: test_game_wait_history543.sh GX-TEST-BUILD}
cmake --build "$build" --target petari_game_wait_history_tests petari_game_wait_runtime_tests petari_game_wait_codegen_probe
ctest --test-dir "$build" --output-on-failure -R '^native_game_wait_(history|runtime|codegen|patch|report)$'
