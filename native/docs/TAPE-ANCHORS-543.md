# Tape schema 2: game-defined input phases

Implementation base: main `6b6e6631`; branch `port/543-tape-anchors`.
Root `src/`, `include/` and `libs/` are unchanged. This item changes neither GX
scheduling nor pacing nor seed-pack v14. Native app CMake participates in the
renderer identity, so adding the observer helper/tests changes the renderer key.
Each instrumented reference needs its own recorded build/key provenance.

## Boundary and off path

`GameSystem::update()` calls `checkRequestAndChangeScene()` before
`mObjHolder->update()` reads WPad, then executes the scene. The optional input
bridge observes channel 0 before its first KPAD result of that loop. A real,
initialized scene returned by `getCurrentSceneForExecute()` is controller
Normal; its first boundary must also satisfy `isFirstUpdateSceneNerveNormal()`.
The observer stores a separate **pre-read** state, never a fabricated previous
route row. Phase IDs distinguish same-stage reloads.

`ScenarioSelectScene::isExecForeground()` takes precedence over the underlying
Normal scene. Entry and resume are explicit phases. The helper observes the
existing world-map wait actor and `WorldMapSequence::isGalaxyMap()` as additional
ordered state/RNG checkpoints; these events do not move the input anchor.

An absent executable scene is an explicit neutral gap. Meaningful button,
pointer, stick or mod actions outside executable phases reject the new tape.
Both input feeding and exact state comparison start from the SAME pre-read
marker. Early/late exits fail. Marker displacement is bounded to 30 loops
relative to the preceding phase's anchor, and waiting to 36000 loops. There is no
init-End/identity fallback for schema 2. Arrival cinematics retain the approved
v1 play-ready state/RNG rule; feeding and comparison re-anchor together there.
Every later play-ready resume is independently checked from the captured route
and sidecar, without adding a resume-time RNG reset.

With no tape active the bridge is the original 002 input body behind one cached
`TapeObserver::enabled` branch. It performs no observation, allocation, clock
read or callback. `native_tape_input_observer` checks original side effects and
returns; the Python source test binds the original body SHA256 to 002. No input
subsystem/model or SDK read implementation changed.

## Format and labels

The JSON sidecar is schema 2, protocol `game-scene-pre-read-v1`. SMG2KPD1's binary
read/sample encoding, route CSV and mod-action encoding are unchanged.

`launch` retains actual direct-launch stage/scenario. `input_phases` is ordered,
with consecutive IDs, `[first_input_frame,end_input_frame_exclusive)`, an explicit
`phase-transition` or `recording-prefix` end, pre-read marker state and map
checkpoints. Marker state includes scene/kind/stage/scenario, first-Normal flag,
player-valid/grounded/dead, float32 position bits and RNG. Unknown formats,
missing state, recorder marker errors, duplicate checkpoints and impossible
ranges fail closed. The last incomplete read loop is never admitted into a
phase. The binary header still counts the open loop (`frames=seams+1`); all raw
reads remain retained and audited, even outside the completed state prefix.

New whole-session reports use `game-defined-scene-input-v2` / `game-phase-v2`.
Schema-1 tapes retain the existing player and `interactive-and-play-ready-v1`,
explicitly labelled `legacy-v1`; historical failures remain failures. Merely
recording an old replay does not qualify its output. Schema-2 segmented/headless
replay remains **play-ready segments only**, never whole-session proof.

## Scene-anchor audit

| Retained tape / scene | Legacy assumption | New recording / replay |
| --- | --- | --- |
| Boot Logo/title | Stage identity | Controller first Normal, before its first read |
| FileSelect (all tapes) | Async init End | Controller first Normal |
| Starship (normal/full-star) | Init End arrival, play-ready | Normal arrival; exact initial play-ready, audit every resume |
| WorldMap01 (normal/pointer) | Init End, which moved 382/383 | Normal input+state together; map-wait/GalaxyMap checkpoints |
| ScenarioSelect (normal) | Destination stage/scenario -1 identity | Explicit foreground overlay entry/resume, actual scene identity |
| IslandFleet (normal) | Init End arrival | Normal arrival plus exact play-ready |
| RedBlueEx (intro twins/full-star) | Init End arrival | Normal arrival plus exact play-ready; full-star pause/resumes retained |
| Direct stage boot | FileSelect then target init End | Recorded launch metadata, same game-defined phase predicates |
| Same-stage reload | Stage name / ready list | Ordered first-Normal phase IDs, explicit exits |
| Headless sweep | Play-ready segment | Same limited scope, schema recorded, unsupported whole-session anchor rejected |

This is the source audit. Real schema-2 route coverage remains pending the server
builds/tests and M1 replay receipts; no existing route is declared re-qualified.

## No-human canonical recording and independent proof

Use a **strictly passing** legacy-v1 replay on a fresh reference path (the map's
legacy init End is 382). `PETARI_TAPE_CAPTURE_REPLAY=1` records the ACTUAL buffer
AFTER the raw replay hook returned it, plus delivered mod actions. This fixes the
old empty replay KPD diagnostic without modifying historical files. The separate
read map binds actual frame/ordinal to canonical frame/ordinal/phase. Python
decodes both binary files independently and requires every returned count,
error, probe/device and all 240-byte samples to match the declared state anchor;
neutral gaps/post-roll must equal the documented neutral reader. It rejects
missing/extra reads, comparison-only shifts and omitted mod actions. First
pointer-valid/stick events are reported relative to each captured game marker.

`capture_schema2_tape.py` is an INNER worker. The caller certifies quiet/load,
Metal namespace policy and heavy->native locks, prepares its OWN perf app/PREP,
and uses bash/nice0. It refuses existing output, mismatched exe/PREP, incomplete
source without `--allow-incomplete`, and does not retry. Fresh private cache is
the default; `--warm-from SUCCESSFUL_SAME_EXE_ARM` copies that exact cache and
retains the same Metal bundle namespace. No cache is deleted by this tool.
Cold Metal must be certified by the outer driver (fresh owned namespace or
approved guarded cache preparation). Warm pair stays in the same namespace.
Input/source fixture and apps are hash-bound before/after. The map case requests
pointer coverage >=30 valid compared rows / >=3 positions and both arms retain
90 s observation/neutral post-roll. Warm real CoreAudio keeps >=50 reports and
zero underrun frames. No host keyboard/pointer driver or human playthrough is
needed for the known canonical routes. If a strict conversion fails, preserve it
and report it; do not pick an unlabelled retry or fabricate metadata.

## Build / proof order and exact worker commands

PORT builds and tests first. The two references are **07223918 game code + the
new native replay/record commits**, and **00221126 game code + the same commits**.
They are instrumented references, not frozen executables. Assert zero
`src/include/libs` delta against EACH reference base. Build 5.4.3 on 6b6e6631
with the same commits. Keep the immutable frozen 541/542 exes as legacy schema-1
controls with today's labels and outcomes.

On the SERVER, after configuring the normal test build, under server-heavy:

```sh
bash tools/port/test_tape_anchors543.sh "$TEST_BUILD"
```

The script builds the new observer/phase/recorder tests and existing tape-player
tests, checks the cached-off original body, and runs the Python audit/legacy
logic tests. C++ checks are server-owned. In particular, use `ctest -N` to verify
registration; a successful build alone is not a test pass.

Set these task-specific variables to the prepared reference paths on that host:
`TAPE_REF_APP`, `TAPE_REF_PREP`, `TAPE_DISC`, `TAPE_FIXTURE`, `TAPE_OUT`,
`TAPE_SOURCE`. `TAPE_FIXTURE` is the exact unlocked-242 fixture used by the
original pointer case; retain its tree hash/xattrs receipt. For the first
conversion `TAPE_SOURCE` is `port53/m1g/tapes/tape-worldmap-54v2` (read-only), and
`TAPE_OUT` is a NEW owned directory. Run under the host's heavy->native locks,
with its normal quiet guard, from bash (no zsh BG_NICE):

```sh
python3 -B tools/port/capture_schema2_tape.py \
  --app "$TAPE_REF_APP" --prep "$TAPE_REF_PREP" --disc "$TAPE_DISC" \
  --tape "$TAPE_SOURCE" --out "$TAPE_OUT" --user-seed "$TAPE_FIXTURE" \
  --stage WorldMap01Galaxy --scenario 1 --allow-incomplete \
  --observe-seconds 90 --bound 600 --require-pointer-evidence
```

For a schema-2 proof use the QUALIFIED capture directory as `TAPE_SOURCE`, omit
`--allow-incomplete`, and choose a new output. Fresh uses a certified cold owned
Metal namespace. Warm uses the same perf app/namespace and adds
`--warm-from "$TAPE_FRESH_OUT" --real-audio`. Do not wrap the whole campaign in a
heavy lock: every arm takes its own lock and quiet check. All M1 arms wait for
SOL A's pacing bisect to free the M1. The 4-arm matrix is 541-reference/542-reference
x fresh/warm with the SAME qualified schema-2 tape. Also cross-replay each
reference's qualified capture on the other reference. No failed source replay
can produce an admitted canonical tape; no failed legacy control is replaced.

The same conversion worker covers normal54, intro49/none and fullstar using
their exact original fixtures/mods/replay_env. Omit `--stage` for boot-launched
routes and omit `--require-pointer-evidence` for non-pointer cases. These audits
must cover each scene/overlay/resume in the table. Retain all RESULT, launch,
process, native report, route, KPD/actions, read-map and PREP/build bindings.

## Detached M1 map proof driver

`tools/port/run_tape_anchors543.py --plan-only --out NEW_OWNED_DIR` freezes the
Python harness and records a finite 16-arm plan without preparing apps or
launching a process. After SOL A's explicit release message, save that release
as a receipt and launch from bash at nice0:

```sh
python3 -B tools/port/run_tape_anchors543.py --detach \
  --out "$TAPE_PROOF_OUT" --release-receipt "$TAPE_RELEASE_RECEIPT"
```

The child uses nohup + setsid, detached stdio, and no outer heavy lock. Every arm
acquires heavy then native, proves the quiet/load precondition, and uses only
B's perfm1b Metal namespace. All five perf copies retain exact original/PREP,
resource and source hashes; references remain labelled test instruments and
legacy controls remain legacy-v1 observations. A legacy failure is retained,
never replaced. A failed strict conversion or schema2 dependency stops the plan
without a retry. Both byte and xattr inventories of the certified fixture and
its private ditto copy must match; no attributes are removed or rewritten.

Conversions observe90s. Since the resulting complete schema2 tape also contains
that recorded neutral tail, schema2 replays observe120s to retain the entire
strict prefix followed by a positive neutral post-roll. This is explicit in the
plan and receipt; >=90s, pointer30/3 and warm audio50reports/0underruns remain
unchanged. Converted tapes are never trimmed. Only the initial qualified captures
serve as canonical tapes for the matrix and reciprocal/candidate replay.
