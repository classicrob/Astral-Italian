#!/bin/bash
# SERVER-owned native test run. Pass an already configured GX test build.
# No default user cache root and no normal game launch occurs here.
set -euo pipefail
if [ "$#" -ne 1 ]; then
    echo 'usage: tools/port/test_pipeline_cache543.sh GX_TEST_BUILD' >&2
    exit 2
fi
repo=$(git rev-parse --show-toplevel)
build=$1
python3 -B "$repo/native/tests/pipeline_cache_identity_tests.py"
python3 -B "$repo/native/tests/pipeline_deadlines_tests.py"
cmake --build "$build" --target petari_cache_sha256_tests petari_pipeline_cache_compat_tests petari_pipeline_artifact_raw_tests petari_pipeline_artifact_shutdown_tests petari_pipeline_cache_dawn_keys_tests petari_pipeline_cache_dir_tests petari_app_smoke_storage_tests petari_gx_shader_tests -j 4
ctest --test-dir "$build" --output-on-failure -R '^native_(cache_sha256|pipeline_cache_(compat|dawn_keys|identity|patch|dir|seed_stamp)|pipeline_artifact_(raw|shutdown)|app_smoke_storage|gx_shader)$'
