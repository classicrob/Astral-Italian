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


## Production-setting matrix and retained legacy controls

Every schema-2 conversion/replay launch explicitly records
`PETARI_PIPELINE_DIAG=0` and `PETARI_TRACE_BOOT=0`. Neither is inherited.
`TRACE_BOOT=1` enables pipeline diagnostics even when `DIAG=0`; comparing these
settings as if they were identical would be incorrect. A warm arm requires a
successful fresh arm on the exact same prepared executable and bundle path,
normal exit, a finished strict native replay, and these same production settings.
The prior launch, RESULT, process, native report and cache hashes are bound into
the warm launch. Every schema-2 warm arm retains the real-audio gate.

The completed frozen controls are retained at
`port53/m1g/solb/solb-543-legacy-controls-r2/`. They are legacy-v1 observations,
not schema-2 acceptance. Three stop at route row544 with map init End383:
frozen072 fresh and frozen002 fresh/warm. Frozen072 warm finishes strict replay
at init End382, but fails overall on 900 audio underrun frames (86 reports,
the nonzero report at process52.827s). Its prime was the failed short fresh arm;
that cache pairing is not eligible for the schema-2 matrix. No control is rerun
or relabelled. The command/env/source/report comparison is retained in
`HARNESS-COMPARISON.json` there. These production-diagnostics-off observations
show the legacy fixed-load-frame assumption is timing-sensitive on frozen5.4.1
as well as5.4.2; A's earlier diagnostics-on controls are a different setting.

After the metadata-corrected builds and their tests are approved, plan with
`--schema2-only` and explicit `--candidate-dir`, `--reference072-dir` and
`--reference002-dir`. This creates only the twelve conversion/matrix/reciprocal/
candidate arms; the four completed legacy controls are not repeated. A failed
conversion or fresh replay stops the driver before any dependent warm arm.


## Synchronous replay connection boundary (r3 finding)

The first production-setting conversion completed native state/pointer replay,
but the actual-read audit found device CORE0 at read11/0 where the source said
FREESTYLE1. All sample bytes/count/error/probe matched; exactly1 of17264 read
payloads differed. The capture remains unqualified at
`port53/m1g/solb/solb-543-tape-proof-r3/`; no subsequent arm ran.

`KpadReplayHooks::connection` is an optional synchronous provider. Main-app tape
replay installs it. After the pre-read marker has selected the input phase, the
raw KPAD hook obtains that frame's connection and applies real WPAD state and
callbacks under `Interrupts`, before sample consumption/copy. The actual-read
observer continues querying live WPAD state; canonical metadata is never used
as an observation. Neutral gaps/post-roll select the same neutral connection as
the sample reader. The old null-provider headless API remains unchanged.

The live GameRead path already invokes `tickChannel`'s connection/extension
callbacks on the KPAD/game thread under `Interrupts`. Synchronous replay uses
the same context. CORE connect produces the connect callback; later extension
attach produces the extension callback, in the same order and with the same
arguments as live GameRead. Re-registering a callback is observed once on the
next read. The host command service can complete commands, but cannot apply a
stale connection snapshot or move a recorded disconnect to a different read.

Lock order: the pre-read observer/phase logic finishes, then the raw KPAD hook
acquires the single native OS interrupt lock for WPAD changes/callbacks and
releases it before samples return. There is no separate WPAD mutex. Raw replay
returns before entering `KPADiRead` and its `readLocked` section. Nested SDK
`OSDisableInterrupts` calls reuse the current disabled state. The command thread
takes only the interrupt lock and never reads the tape/session. There is no
feeder publish mutex, cross-thread wait, CV, join or sleep in a read. The only
5ms sleep left is command service outside the game thread.

`native_tape_connection` tests actual WPADProbe before sample return and actual
post-read capture, a deliberately delayed/stale feeder, callback order and
arguments against live GameRead, extension detach/remote detach, late callback
registration, neutral state, nested interrupts and null-provider/live-off paths.
The native tape parser also directly rejects same-channel attach+detach within
one untimestamped frame before playback. `test_tape_anchors543.sh` builds/runs
these five native targets on the server; no M1 compiler is used. The source test
removes only the opt-in replay block and binds the remaining original input body
to the preserved002 hash, keeping the live/default-null implementation exact.


## r5 canonical and reciprocal bindings

r4 converted ref072 successfully with17028 exact actual reads/connections;
ref002's legacy conversion retained its initEnd383/row544 FAIL. Neither outcome
is replaced. `--schema2-only --canonical-source QUALIFIED_R4_REF072_CAPTURE`
starts a new ten-arm plan with no conversion retry. `TAPE-MANIFEST.json` seals
the canonical route CSV/KPD/sidecar/actions SHA256 plus launch/RESULT/native
qualification hashes before the first arm; each arm rechecks the bound canonical
before launch. Both references and the candidate use this ONE canonical.

Only a strict fully successful ref002 schema2 fresh replay admits a reciprocal
capture. Its independent proof must identify source schema2 and all actual
read/connection bytes; it is bound into `RECIPROCAL-ADMISSION.json` and the
manifest. Each reciprocal arm checks the admission unchanged. A failed schema1
capture can never supply this source. Canonical runs observe120s; reciprocal
runs observe150s to retain the new120s capture plus positive neutral post-roll.
There is no trimming, new anchor, retry, tolerance or predicate change. Once
r5 finishes or fails, release the M1 to SOL A's GPU trace overhead measurement.

## Explicit power-request events (5.4.3 lifecycle fix)

The r5 proof stopped at route row 4225 because its source requested power after
row 4224 but the tape contained no such event. `finish()` did not take another
sample: the real power callback froze the scene on the next update. The r5
failure, extra-read accounting and unchanged source tapes remain retained in
`m1g/solb/solb-543-tape-proof-r5/`.

New schema-2 recordings add `session_control_protocol: native-post-seam-power-v1`,
`session_control_error` and ordered `session_events`. Each event records its ID,
kind, phase ID, `after_route_row`, `before_input_frame=after_route_row+1`, boundary
`after-route-seam-before-next-update`, actual callback acknowledgement, and
observed neutral-tail seconds. A neutral-tail-begin may precede the one final
power-request. The request must occur in the last active input phase. Unknown,
duplicate, missing or ambiguous fields, an out-of-seam/cross-thread request, or
an unacknowledged game flag reject replay. Crash checkpoints remain incomplete.
Normal finalization only closes/flushes retained data; it does not sample state.

Replay uses the SAME existing phase/play-ready offset for the event and its
inputs/state. At the exact post-route snapshot, `Host::requestQuit()` calls the
real `Platform::Power::pressPowerButton()` and verifies the game's
`GameSystemResetAndPowerProcess::mIsValidPowerOff`. No cross-thread wait or new
clock is introduced. The existing power implementation owns the Interrupts
lock; no WPAD/KPAD lock is acquired by this new call. Every following state row,
including invalid teardown rows, is bit-exact. Raw shutdown KPAD bytes, read
shape and connections are replayed from a final retained raw segment on the
last phase's clock; `recorded-shutdown` read-map entries make this independently
auditable. Normal exit is successful only after all events, reads and state
rows finish exactly; early exit or an extra seam fails.

Reports label these tapes `game-phase-v2-power-v1`. Schema 1 stays `legacy-v1`;
older schema-2 tapes stay `game-phase-v2-legacy-no-power`. Neither is silently
upgraded. Live input is untouched. Recorder event work is behind its cached
active flag; replay event work is only enabled by the explicit protocol.

Head's lifecycle option (1) requires a NEW canonical with at least 150 seconds
of recorded neutral input before power-off. A strict legacy-prefix conversion
can record this without human input using
`PETARI_TAPE_PLAY_POST_ROLL_SECONDS=180` (the native observation minimum) and
actual-output recording. The original >=90-second wall/audio gates remain.
The new replay issues power at its recorded row; if it reaches power before the
requested observation is complete, it fails immediately without postponement
or padding. Old failed receipts are never used to prime a warm arm.

Pure Python tests cover event parsing, provenance and byte/connection audit.
`native_tape_power` drives the real native power callback through exact and
shifted existing phase clocks; early/late/missing/duplicate events, wrong
acknowledgement, early observation/exit, and missing/extra shutdown reads must
fail. `native_tape_record` checks normal `_Exit`, atexit, crash checkpoint,
ambiguous/missing power events, idempotence and that finish never resamples.
These tests establish code behavior; the fresh/warm reference matrix and
candidate runs establish runtime acceptance. Builds for this revision run only
on the M1, under heavy->native locks.
# Power tape open-loop admission

A complete power-protocol tape rejects any KPAD read at or beyond its declared
route seam count. The final open loop has no corresponding state row and cannot
be silently removed by segment selection. Both the native player and independent
Python audit enforce this before replay. Legacy tapes retain their declared-prefix
behavior. The positive power fixture pairs every read with a seam; a separate
negative retains the extra open-loop read and must fail admission.

## Native r7 producer (ref072 test instrument only)

`PETARI_INPUT_SCRIPT_RECORD_TAIL_SECONDS=180` requires the recorder and forbids
replay. Existing key lines remain unchanged. Recorder-only `Pointer:x,y` and
`Pointer:off` tokens use native `Input::mouseMoved/mouseLeft`, with no OS events.
`@valid N Pointer:x,y` applies at the post-Nth valid WorldMap01 row. The r7
script retains120 idle /10 W /1200 idle, plus120 Pointer:off for filter settling.
Its three pointer targets are400,300;900,450;700,600. The first is scheduled
after150 map-valid rows, then108 and107 rows apart (the retained v2 observed
spacing; native scheduling removes the host driver's polling latency).

On script completion, all keys and pointer are released and a tail is queued.
Only the next recorder post-row control seam records neutral-tail-begin. Its
steady-clock start is acknowledged after the actual recorder event. Every tail
seam rejects non-neutral native bound input. At >=180 wall seconds a control
seam invokes Host::requestQuit, hence the real power callback and strict shutdown.
The independent raw-read audit additionally rejects any non-neutral sample in the
declared tail. No frames, inputs or power metadata are synthesized at finish.
Absent the opt-in, normal script and live input behavior is retained.

This addition belongs ONLY to the072 recorder reference. Ref002 and the admitted
2be2e2f4 candidate consume the resulting existing power/phase protocol unchanged.
The reference has zero src/include/libs/native/gx/native/data delta from07223918.

Native tail audit policy `native-neutral-controls-v1` is explicit in the r7 plan,
launch and proof lineage. Every SDK sample must have zero core/extension button
masks, zero core/extension stick/trigger magnitude (no deadzone), no accel-speed
or angular-velocity event, and pointer invalid. Every channel's entire240-byte
KPADStatus must equal its first tail sample byte-for-byte: this includes accel,
pointer fields, extension union, MPLS and unknown unk_EC. No per-sample fields are
exempted as timestamps or sequence fields. Only read-envelope frame/ordinal,
returned sample count and vector length can advance; every actual sample and
connection remains recorded and is compared exactly on replay. The legacy
conversion path retains its full single-bootstrap payload check.

## Tape-bound native neutral tails (r8)

r7's literal native recording passed its origin audit (51,492 reads, 180.016 s
neutral tail, 33 shutdown rows). Its first replay stopped at source input frame
2055: three identical no-control samples were compared with the legacy synthetic
single-sample bootstrap. No subsequent matrix arm ran. The displayed position
delta followed the failed read; all prior compared state rows were exact. The
failed r7 receipts and unmodified tapes remain evidence.

New sidecars bind `neutral_tail_policy` under schema 2 and
`native-post-seam-power-v1`. Missing means `single-bootstrap-v1` and retains the
existing full bootstrap comparison. Unknown values, duplicate fields or a policy
without that versioned protocol fail closed. Only the native script producer
selects `native-neutral-controls-v1`; a replay capture inherits its parsed source
policy before recording, with no environment override. r8 records a new canonical;
it never adds a field to r7's tape.

For the native policy every sample must have no core or extension buttons, stick
or trigger magnitude, valid pointer, core/Nunchuk acceleration-speed event or MPLS
angular velocity (zero tolerance, no deadzone). All 240 sample bytes on each
channel equal that channel's first tail sample, including acceleration/gravity,
extension data and unknown bytes. There are no sample-field exemptions. Probe,
device, error and requested capacity are constant per channel. Only the read
envelope's frame, ordinal, returned count and sample count may advance. Every raw
sample and connection still passes the separate exact replay audit; validation
does not normalize or replace any byte. Power delivery and all shutdown checks
are unchanged.

The native power test covers a complete multi-sample session and legacy rejection
of the same shape, all policy parsing negatives, first-sample control negatives,
motion events, tiny stick values, pointer validity, changed acceleration and
unknown bytes, and each connection-envelope field. Recorder tests persist the
selected/default policy; source guards require strict captures to inherit it.
The Python audit independently checks policy binding and the same byte rules.

PORT builds r8's three exact source heads on the SERVER while SOL A owns the M1.
The planner accepts either M1 or these authorised SERVER builds only with
`native_tests_passed=true` and, for the candidate, `candidate_suite_passed=true`
after the M4 admission suite. BUILD/PREP, full source SHA, original/perf binary
hashes and the canonical's file hashes remain bound per arm. This supersedes the
earlier M1-only build note for this specific round; measurement stays on the M1.

## Defined game-thread teardown observation (r9)

r8's first replay matched all interactive state, native tail reads and the real
power request at row 12836. Both producer and replay froze at the same position
on the next update, ended controller-Normal input at 12839, and wrote 33 shutdown
rows. Only row 12840 differed: the native producer had no observable player;
replay still exposed the frozen player for one more row. All subsequent CSV rows
matched, but that later equality is diagnostic only: the strict run failed and
no dependent arm ran.

This was not a power-delivery offset. Both calls run in the same post-snapshot,
pre-next-update control seam, use the same synchronous callback and acknowledge
the same row/phase. Scene destruction runs asynchronously. The old observer
tested Initialize_End and the scene pointer even after reset had requested
destruction, so it observed a worker-owned lifetime rather than game state.

The shared native observer gate now checks the game's own `mIsResetProcessing`
FIRST, then Initialize_End, and only then the scene pointer. `prepareReset()`
raises that flag on the game thread before requesting destruction. Ordinary
`requestChangeScene()` already clears Initialize_End before deletion. A teardown
row retains the controller's stage/scenario and reports the default player
state: invalid, zero position, grounded=false, dead=false. It does not read the
scene, actor, camera or scene-object holder while the worker may free them.
Every shutdown row remains compared strictly, including invalid rows. No row is
skipped, no timing bound changes and no failed receipt becomes a pass. r9 records
a NEW canonical; the old r8 tape keeps its original values.

The same gate protects the pre-read tape observer, route/seam recorder, smoke
observation, stall-context telemetry, progress observer, and Host's read-only
play-ready query. The latter now becomes false at reset preparation rather than
at worker completion. No warp/player mutation, input delivery, game update,
power callback, comparison, SDK, GX, src/include/libs or data code changes.

The mandatory native power test runs a real host worker with both schedules:
it completes deletion before the 33 observations, or after all 33. The rows must
be identical and invalid in both schedules, and a counted scene slot must never
be read after the game-thread reset flag. Normal/initializing/missing-controller
cases and source guards protect the lifecycle ordering and every call site.
Native builds/tests remain on PORT's server; M1 work is released to SOL A until
the new exact, admitted builds and the next slot are available.
