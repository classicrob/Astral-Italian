#!/bin/bash
# Per-galaxy 60 fps / latency sweep (advisor r30): for each stage, one COLD pass (fills the run's own pipeline/Dawn
# caches) then one WARM pass measured with frame and on-screen present statistics (MTLDrawable presentedTime) and the
# input-latency probe (random A presses), both driven by perf_sweep_move.script (camera pans, walk, jump, spin); the
# app quits when the script ends (PETARI_INPUT_SCRIPT_QUIT=1). Each pass first waits up to QUIET_WAIT seconds for a
# quiet machine (1-minute load <= QUIET_LOAD and no other process above QUIET_CPU %) and records the load and the top
# processes, so noisy runs can be flagged. Then: native/tools/perf_table.py <out>.
#
#   native/tools/perf_sweep.sh <Petari.app> <seed.db|none> <out_dir> <stage> [stage...]
#   env: DISC (default ~/MacioGalaxy2-gamedata/SB4E01-extracted), QUIET_LOAD (2.5), QUIET_CPU (25), QUIET_WAIT (300)
# Use a dedicated copy of the app bundle: the seed is stamped into its Resources.
set -u
app=$1; seed=$2; out=$3; shift 3
here=$(cd "$(dirname "$0")" && pwd); repo=$(cd "$here/../.." && pwd)
disc=${DISC:-$HOME/MacioGalaxy2-gamedata/SB4E01-extracted}
key=$(ls "$repo"/build/*/native/gx/pipeline_cache_build_identity.key 2>/dev/null | head -1)
quiet() {  # $1 = log file; waits for a quiet machine, then records the state
  local waited=0
  while :; do
    local load busy
    load=$(sysctl -n vm.loadavg | awk '{print $2}')
    busy=$(ps -Ao pcpu=,comm= -r | awk -v lim="${QUIET_CPU:-25}" '$1 > lim && $2 !~ /Petari|ps$|awk$/ {print}' | head -3)
    if awk -v l="$load" -v m="${QUIET_LOAD:-2.5}" 'BEGIN{exit !(l<=m)}' && [ -z "$busy" ]; then echo "quiet=yes load=$load waited=${waited}s" >> "$1"; break; fi
    if [ "$waited" -ge "${QUIET_WAIT:-300}" ]; then echo "quiet=NO load=$load waited=${waited}s busy=[$(echo $busy)]" >> "$1"; break; fi
    sleep 10; waited=$((waited + 10))
  done
  ps -Ao pcpu=,comm= -r | head -6 | sed 's/^/  top: /' >> "$1"
}
mkdir -p "$out"
for stage in "$@"; do
  d=$out/$stage; rm -rf "$d"; mkdir -p "$d/user" "$d/cache"
  if [ "$seed" = none ]; then rm -f "$app/Contents/Resources/initial_pipeline_cache.db"
  else python3 "$repo/native/gx/pipeline_cache_identity.py" stamp "$seed" "$app/Contents/Resources/initial_pipeline_cache.db" "$key" || exit 1; fi
  for pass in cold warm; do
    quiet "$d/info.txt"
    echo "$pass start $(date +%T)" >> "$d/info.txt"
    extra=(); [ "$pass" = warm ] && extra=(PETARI_FRAME_STATS=1 PETARI_LATENCY_PROBE=1 PETARI_LATENCY_TEST=1)
    env SDL_AUDIO_DRIVER=dummy PETARI_CACHE_DIR="$d/cache" PETARI_INPUT_SCRIPT="$here/perf_sweep_move.script" \
      PETARI_INPUT_SCRIPT_STEADY=60 PETARI_INPUT_SCRIPT_QUIT=1 ${extra[@]+"${extra[@]}"} \
      "$app/Contents/MacOS/Petari" --disc "$disc" --user "$d/user" --stage "$stage" --scenario 1 > "$d/$pass.log" 2>&1 &
    pid=$!
    for _ in $(seq 1 240); do sleep 0.5; kill -0 $pid 2>/dev/null || break; done   # 120 s cap
    kill $pid 2>/dev/null; sleep 2; kill -9 $pid 2>/dev/null; wait $pid 2>/dev/null
    echo "$pass end $(date +%T) script_done=$(grep -c '\[input-script\] done' "$d/$pass.log")" >> "$d/info.txt"
  done
  rm -rf "$d/user" "$d/cache"
done
rm -f "$app/Contents/Resources/initial_pipeline_cache.db"
echo "perf_sweep: done; table: python3 $here/perf_table.py $out"
