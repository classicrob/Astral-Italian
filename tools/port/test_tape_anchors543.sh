#!/bin/bash
# Caller owns host build lock then native (PORT also runs on SERVER); no game launch.
set -euo pipefail
if [ "$#" -ne 1 ]; then
  echo 'usage: bash tools/port/test_tape_anchors543.sh CONFIGURED_TEST_BUILD_DIR' >&2
  exit 2
fi
repo_dir=$(cd -- "$(dirname -- "$0")/../.." && pwd)
test_build=$1
cmake --build "$test_build" --target petari_tape_input_observer_tests petari_tape_record_tests petari_tape_phase_tests petari_tape_play_tests petari_tape_connection_tests petari_tape_power_tests
python3 -B "$repo_dir/native/tests/tape_observer_source_tests.py"
python3 -B "$repo_dir/native/tests/tape_power_source_tests.py"
python3 -B "$repo_dir/tools/port/test_tape_power_audit.py"
if [ -f "$repo_dir/tools/port/test_native_tape_record543.py" ]; then
  python3 -B "$repo_dir/tools/port/test_native_tape_record543.py"
fi
python3 -B "$repo_dir/tools/port/test_tape_phase_audit.py"
python3 -B "$repo_dir/native/tests/replay_tape_tests.py"
python3 -B "$repo_dir/tools/port/test_normal_tape_gate.py"
ctest --test-dir "$test_build" --output-on-failure --no-tests=error -R '^native_tape_(input_observer|record|phase|play|connection|power)$'
