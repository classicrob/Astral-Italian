# 5.4.2 server title and menu commands

Run from the checkout containing these harness scripts, at nice0. `CAND541` and
`CAND542` are the immutable directories with `Petari.app` and its exact
`BUILD.json` (07223918 baseline and the combined v14 candidate respectively).
The app/source/executable/resources and dedicated perf copies are bound in PREP.
The scripts refuse an existing output directory. Choose a new label per cohort.

```bash
TOOLS="$PWD/gate53/m1gates"
bash "$TOOLS/server_title542.sh" "$CAND541" "$CAND542" \
  m4-542-title-v14-r1 "$HOME/MacioGalaxy2-gamedata/SB4E01-extracted"
```

This prepares its own `perf542title541` / `perf542title542` copies and runs exactly
541-r1,542-r1,541-r2,542-r2,541-r3,542-r3. Each arm independently takes
`server-heavy.lock` then `native.lock`, waits for quiet, clears only its own Metal
namespace and uses a fresh private cache/user directory. Neither arm supplies
stage/mod/input-script/key/pointer input. Mods are explicitly disabled.

New-build environment: `PETARI_TITLE_OBSERVE=1`,
`PETARI_TITLE_OBSERVE_MIN_SECONDS=315`, `PETARI_PIPELINE_DIAG=0`,
`PETARI_TRACE_BOOT=0`, normal Fifo/windowed launch. It self-ends only after actual
UI-global100 plus30s and at least315s total; hard cap900s. The baseline has no
observer env and receives an orderly AppleEvent at315s, addressed to the exact
owned PID/app/bundle. Its normal exit, flushed CSV and neutral route are required.
No AX or controller events are sent. Both retain original tails and compare the
same first300s window from the first presented frame.

Results are under `port53/m1g/solb/m4-542-title-v14-r1/`: individual RESULT/launch/
quiet/process/app.log/frames.csv, PREP and aggregated541/542-SCORES.json. Baseline
split lines are honestly unscorable; its scorer exit is retained separately. The
new title arm requires FileSelect's exact validated69-config admission.

## Independent scoring

Title new (the server wrapper runs this too):

```bash
OBS="$HOME/MacioGalaxy2-gamedata/port53/m1g/solb/m4-542-title-v14-r1"
python3 -B "$TOOLS/gate542m.py" --tier m4 --title --window-seconds 300 \
  --require-deadline-stage FileSelect:69 --out "$OBS/542-SCORES.json" \
  "$OBS/542-r1" "$OBS/542-r2" "$OBS/542-r3"
```

Baseline title (no `--title`, no admission requirement; retain missing-signal
status and fixed_window.stats rather than claiming split acceptance):

```bash
python3 -B "$TOOLS/gate542m.py" --tier m4 --window-seconds 300 \
  --out "$OBS/541-SCORES.json" "$OBS/541-r1" "$OBS/541-r2" "$OBS/541-r3"
```

For EACH existing direct-stage noreg arm, score its actual run directory alongside
the unchanged gate533 invocation. The function's first argument is541 or542,
second is the actual stage, third is that arm's directory containing app.log and
frames.csv. Run it after each arm, or on the retained directories afterwards.

```bash
score542_direct() {
  local version="$1" stage="$2" run_dir="$3" count
  local require=()
  if [ "$version" = 542 ]; then
    case "$stage" in
      YosshiHomeGalaxy) count=183;;
      MokumokuValleyGalaxy) count=168;;
      MarioFaceShipGalaxy) count=74;;
      *) echo "unexpected noreg stage: $stage" >&2; return 2;;
    esac
    require=(--require-deadline-stage "$stage:$count")
  elif [ "$version" != 541 ]; then
    return 2
  fi
  python3 -B "$TOOLS/gate542m.py" --tier m4 --window-seconds 300 "${require[@]}" \
    --out "$run_dir/GATE542M.json" "$run_dir"
}
# In the corresponding arm of noreg542i3.sh:
# score542_direct 541 "$stage" "$out"
# score542_direct 542 "$stage" "$out"
```

Direct arms do not use `--title`: all non-gameplay frames after first presentation
must be<=250ms and gameplay<=100ms, with unsplit menu statistics retained. They
may honestly have no global100/post100 samples. Title arms carry the33ms while-
showing /22ms post100 lines. Every raw failure remains a failure; gate533 is
unchanged. Compare fixed_window.stats for title and unsplit/max/>50 plus load
numbers for direct against the same baseline cohort; never compare different
title observation lengths as if they were equal windows.

## Runtime sidecar admission evidence

The default-on native line is emitted by `petari_uber_admit_stage`, immediately
before registering/scheduling the complete stage batch:

```text
[gx first-use deadlines] stage=FileSelect policy=frame-then-cost reason=validated configs=69
[gx first-use deadlines] stage=YosshiHomeGalaxy policy=frame-then-cost reason=validated configs=183
[gx first-use deadlines] stage=MokumokuValleyGalaxy policy=frame-then-cost reason=validated configs=168
[gx first-use deadlines] stage=MarioFaceShipGalaxy policy=frame-then-cost reason=validated configs=74
```

`--require-deadline-stage Stage:count` requires at least one exact line and rejects
ANY line for that stage showing a fallback, wrong count or reason. The raw lines
and expected counts are retained in `deadline_admission`; a missing line fails.
This proves validated metadata admission, not GPU compilation completion. Missing
or invalid `.pfd` prints `policy=first-use-order reason=... configs=0`. Unknown and
explicit fallback stages are not misreported as admitted.
