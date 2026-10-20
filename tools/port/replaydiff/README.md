# Original-game Dolphin collector

`dolphin_collect.py` observes the original **SB4E01** game over Dolphin's GDB
remote protocol. Observation alone does not call guest functions or write guest
RAM/registers. Explicit `--input-script` enables the filtered input injection
described below.
Every CSV row is one accepted `GameSystem::frameLoop` call to
`MainLoopFramework::waitForRetrace`: PC `804be280`, LR **`804b6ac4`**. VI frames,
movie polls, and the debugger's single steps are separate clocks.

Use Python 3.10+ and macOS `lockf`. No additional Python packages are required.
The installed 2017 Ishiiruka app lacks the GDB configuration strings. Initial
qualification uses a separate official **Dolphin 2609a**, revision
`409881c34357d0f105fd473167d15ab0bd9c1628`, in `~/MacioGalaxy2-gamedata/`.
The app in `/Applications` is read-only. See `docs/port/REPLAY_DIFF.md` for the
layout evidence, validation status, and remaining replay qualifications.

M0 has two fresh-profile ARM64 JIT captures of 1800 loops through boot and file
select with **byte-identical CSVs**. They do not qualify playable-galaxy physics,
interpreter execution, filtered KPAD replay, or native equality. Private run
locations and the exact result are recorded in the design document.

## Private files and launch

Keep the user's disc, saves, caches, extracted data and screenshots of game
content in **`~/MacioGalaxy2-gamedata/`**, outside both the repository and
`~/MacioGalaxy2-team/` (the team tree is uploaded in archives). The collector
rejects disc/profile paths in the team tree and all artifact paths in the repo.
Numeric field traces, text logs/manifests and generated input scripts may stay
in team scratch; never put RAM dumps or game assets there. Never commit or
upload game data or traces containing it.
Check `df -h /` before copying data or running a large job; keep 40 GB free.

```sh
python3 tools/port/replaydiff/dolphin_collect.py prepare \
  --user ~/MacioGalaxy2-gamedata/dolphin-user-run01

python3 tools/port/replaydiff/dolphin_collect.py collect \
  --dolphin ~/MacioGalaxy2-gamedata/Dolphin-2609a.app/Contents/MacOS/Dolphin \
  --user ~/MacioGalaxy2-gamedata/dolphin-user-run01 \
  --disc ~/MacioGalaxy2-gamedata/SB4E01.iso \
  --output ~/MacioGalaxy2-team/scratch-port/solc/run01 \
  --frames 600
```

`prepare` requires an empty directory. Use a fresh directory for each canonical
power-on repeat. The default CPU core is interpreter (`0`); ARM64 JIT is `4`
and x64 JIT is `1`, selected with `prepare --cpu-core`. These modes must be
qualified separately. The profile disables dual core (`CPUThread=False`),
cheats, real Wiimotes, SD card, and audio output; fixes RTC to Unix epoch
946684800 (2000-01-01 UTC); enables DSP HLE and one emulated Wiimote with
Nunchuk, without host bindings. GameCube `SelectedLanguage=1` is not proof of
Wii SYSCONF language; preserve the generated SYSCONF for a fixture.
`Interface/DebugModeEnabled=True` is required: the GDB socket can accept `Z0`
while execute breakpoints are ignored when debugging is off. The collector
checks this setting before launching.

`collect --dolphin` re-executes itself as
`lockf -k ~/MacioGalaxy2-team/slots/native.lock nice -n 10 <collector command>`.
It holds that slot through hashing, the emulator run, and emulator termination.
Optional `--lock-timeout SECONDS` bounds queue waiting via `lockf -t`; it does
not extend the independently bounded locked run.
Do not pass the internal `--slot-held` argument manually. Without `--dolphin`,
it attaches to an already isolated, separately locked emulator; it does not
acquire the slot or stop that externally managed emulator.

The GDB endpoint is a private Unix socket, default
`~/MacioGalaxy2-team/scratch-port/solc/gdb.sock`. TCP GDB listeners are disabled:
Dolphin's TCP implementation binds all interfaces. The first stop can report
SIGTERM (`T0f`); subsequent continue/step operations must report SIGTRAP.
The collector verifies the original caller's `bl` instruction before running.
It removes the breakpoint, steps one instruction, then reinstalls it to avoid
recounting a stopped instruction. Experimental `--gui` instead continues to a
temporary execute breakpoint at seam+4; its full repeat qualification is pending.
If resuming to a temporary pc+4 breakpoint instead reaches another installed
KPAD or game-loop breakpoint, the collector processes that stopped event before
issuing another continue. It records these diversions in the manifest; unknown
stops or the original stopped PC still fail. This also covers a temporary
breakpoint inserted inside a JIT block already executing.
Record a new TAS movie without GDB first, then replay it in batch mode for collection.
Unexpected stops, invalid nonnull pointers,
bad counts/strings/booleans, cycles, and nonfinite floats fail the capture.

Dolphin can leave its Unix socket after exiting. Confirm no emulator owns it
before removing a stale socket. A new output directory is required each run.
Connection timeout defaults to 60 seconds **after acquiring the shared slot**;
per-stop timeout is 30 seconds; capture wall timeout defaults to 480 seconds
and cannot exceed that value. An independent 540-second alarm covers startup,
hashing and capture together; owned Dolphin termination has a 10-second grace
period. SIGTERM/SIGINT record an incomplete manifest and stop/reap the owned
emulator. Release the lock between runs; every locked run must stay below
10 minutes. The capture records its PIDs immediately and progress every 60 loops. Exit 0 means the requested rows were captured,
not that determinism or native equivalence passed. Exit 2 means an incomplete
run. A queue timeout returns lockf's nonzero status before an output directory
or emulator is created. Partial rows and the manifest/log error remain
available for capture failures.

The manifest's `timing_policy` distinguishes PPC time-base frequency from
Dolphin core scheduling ticks. Pinned Dolphin uses a 729 MHz Wii core and a
12:1 time-base ratio, giving `tick_hz=60750000`; this collector does not read
actual tick or VI registers. Rate numerator/denominator, `epoch_ticks` and host `fpcr_fz`
remain null, with explicit interpretation strings. Audio output backend and
DSPHLE setting are declared. Dolphin continues scheduling DSP slices and
audio DMA independently of the null sound backend. These declarations do not
qualify native timing, audio processing or floating-point equivalence. Sources:
[pinned SystemTimers.cpp](https://github.com/dolphin-emu/dolphin/blob/409881c34357d0f105fd473167d15ab0bd9c1628/Source/Core/Core/HW/SystemTimers.cpp),
[SystemTimers.h](https://github.com/dolphin-emu/dolphin/blob/409881c34357d0f105fd473167d15ab0bd9c1628/Source/Core/Core/HW/SystemTimers.h),
and [AudioCommon.cpp](https://github.com/dolphin-emu/dolphin/blob/409881c34357d0f105fd473167d15ab0bd9c1628/Source/Core/AudioCommon/AudioCommon.cpp).

## Read-only native input export

`--export-inputs` can accompany `--movie`; it observes original KPADReadEx
entry/return buffers and the verified SDK control-block fields queried by
WPADProbe, with no guest RAM/register writes. `inputs.jsonl` retains every returned sample in SDK order, or slot0
on a zero/error return, including actual requested capacity and result/error.
Connection is queried at stopped KPAD entry and return. The exported connection
is the actual post-read snapshot; differences are retained as before/after
records in manifest `connection_observation.within_read_transitions`. The native
tape does not represent intra-call transition timing, so these observations
cannot establish native connection timing equivalence. The observer verifies the complete original WPADProbe function
hash before reading its control-block array/status/device/ready fields. The
original function only reads these inside an interrupt guard and writes its
caller output; a stopped CPU permits an equivalent query without a guest call.
No connection is inferred from failed sample contents. Unsupported extensions and nonfinite fields fail rather
than replacing data. The native schema preserves named fields, excluding
inactive extension-union padding. One1800-loop M0 export passed exact accepted CSV identity and strict native
packing:7200 SDK reads/11008 samples. The actual SDK commonly returns3/4
samples per connected read and uses device255 during initialization. A single
within-call connection transition at source536 is retained in the manifest.
Filtered-tape repeat determinism and native behavior/timing remain unqualified.

The accepted M0 movie is private
`~/MacioGalaxy2-gamedata/accepted-input-m0.dtm`. Use a fresh CPU4 profile and
`collect --movie PATH --export-inputs --frames 1800` with the standard bounded
lock lifecycle. The expected accepted M0 CSV SHA256 is
`ab83e2716468324f33b5f1da955d15154b976e24a817e943eae460505b7265b1`.
A differing replay is new evidence, not the accepted route. After a successful
matching capture, `python3 native/tools/headless.py pack --frames 1800
PRIVATE_CAPTURE/inputs.jsonl PRIVATE_CAPTURE/inputs.tape` validates the native
interchange. Synthetic movie-to-button approximation cannot substitute for
this filtered SDK export. Exact operational paths are in Sol C's HANDOFF.

## Optional stage synchronization

`collect --sync-stage SkyIslandGalaxy --sync-scenario 1 --frames 300` emits
300 rows beginning at the **first** accepted post-endFrame seam with
`scene_valid=1`, scene `Game`, `init_state=6` (`SceneInitializeState_End`),
and the exact internal stage and placed `ScenarioNo`. It does not wait for
`player_valid`. That seam becomes CSV frame 0 and scene epoch 0; later epoch
changes remain visible. The 86-column schema is unchanged. An observer error
before synchronization fails the run, and missing RNG at the first matching
seam fails instead of selecting a later origin.

`oracle.csv.sync.json` records `schema_version=1`, `phase="post-endFrame"`,
`stage`, `scenario`, `source_frame`, `source_scene_epoch`, `source_ticks`,
`source_vi_count`, `rng_valid`, `rng_seed` (unsigned decimal integer), and
`rng_forced=false`. The seed is read from the untouched original game.
Sidecar ticks/VI are null when unobserved; `rng_valid` is boolean true. A sidecar beside a partial trace
is diagnostic only; require a complete `oracle.csv` and successful manifest.
The manifest retains the origin and number of accepted source loops.

With sync enabled, `--frames` is the output row count; a scripted input file
still declares the **full source-loop count**, including boot/navigation.
`inputs.jsonl` retains that clock, capacities and actual read ordinals. Source
frame `source_frame` was consumed before sync row 0; first shared gameplay
inputs belong to `source_frame+1` and affect sync row 1. The tape must extend
through all requested output rows. A complete 300-input gameplay suffix after
the anchor requires 301 emitted rows (anchor plus 300 completed loops). It is never silently rebased or regenerated
from the trimmed trace. Seed alignment does not equalize already-initialized
actors; row-0 non-RNG equality needs separate qualification.

`--sync-input-script inputs/stage-start.json` optionally freezes that shared
controller suffix at `source_frame+1`. Supply the ordinary `--input-script`
as the full navigation prefix and request suffix length+1 output rows. On
the selected seam the collector preserves all executed prefix intent, replaces
future navigation with the suffix, and disables live edits.
`input-script-final.json` records the complete derived source-frame script;
`sync_input_source_start` and suffix hash are in the manifest. The actual
exported KPAD JSONL remains authoritative, including trigger/release/acceleration
history across the boundary. Replay it directly; an independent intent-script
expansion is evidence only after matching the actual read stream.

`--save-state PATH` launches Dolphin with its `-s` option. Keep states outside
the repository and team upload tree; only their SHA256 enters the manifest.
Source frame 0 is the first accepted loop after restore, not the original
power-on ordinal. KPAD breakpoints are installed after that first seam to avoid
intercepting a return from a read entered before the restore; that first loop
has no newly injected input records. Preserve this distinction when constructing
an independent native boot tape. Savestate loading and sync are implemented but
remain unqualified on a live galaxy capture.

## CSV schema v1

The authoritative order is `dolphin_collect.COLUMNS`, checked against
`native/app/headless/trace_fields.inc` by a test. Decimal integers use no host
addresses. Floats use nine significant digits and an adjacent `_bits` column
containing `0x` and eight hex digits of the original IEEE float32. An empty
cell means unavailable, never numeric zero. `player_valid` refers to the player;
title/loading rows still count. Do not filter these rows before diffing.

| Columns | Meaning |
| --- | --- |
| `frame` | Zero-based accepted game-loop ordinal; schema version 1 is in the manifest |
| `scene_epoch` | Starts at 0; increments when scene pointer or scene/stage/scenario identity changes, matching the native observer |
| `scene,stage,scenario,init_state` | Current scene control strings, signed scenario, enum 0..6 |
| `scene_valid,rng_valid,player_valid,actor_count_valid,animation_valid` | Independent validity bits |
| `rng_seed` | Embedded GameSystemObjHolder MR random stream seed |
| `actor_registered,actor_living` | AllLiveActorGroup registered count and count with dead byte false |
| `hp,dead,mode` | MarioActor HP, LiveActor dead flag, u16 MarioActor mode |
| `state_ids,pending_state_id` | JSON array in linked-stack order; blank pending means null, distinct from ID 0 |
| `movement_lo,movement_hi,flags14_lo,flags14_hi,draw,flags20,flags24_lo,flags24_hi,prev_draw` | Raw Mario flag words |
| `state_flags3` | Bool bytes at Mario+A90..A92 joined with semicolons, matching the native observer |
| `anim_name` | MarioAnimator cached current BCK string at +114, when available |
| `x,y,z` | LiveActor position |
| `sim_x,sim_y,sim_z` | MarioPosHolder simulation position |
| `vx,vy,vz` | Mario velocity at +160 |
| `vel_after_x,vel_after_y,vel_after_z` | Mario velocity at +178 |
| `actor_vx,actor_vy,actor_vz` | LiveActor velocity |
| `front_x,front_y,front_z` | Mario front vector |
| `actor_up_x,actor_up_y,actor_up_z` | Actual MarioActor up vector |
| `gx,gy,gz` | Mario air gravity, not the conditional effective-gravity getter |
| `actor_gravity_x,actor_gravity_y,actor_gravity_z` | LiveActor gravity |
| `vi_count,ticks` | Reserved; currently blank on both observers |

Player and actor-group traversal requires scene `Game` and initialization
state 6. A missing nullable holder/actor leaves the corresponding group invalid;
an invalid nonnull pointer is an observer error. State stack cap is 64; actor
capacity cap is 32768. Animation string cap is 1024 bytes; scene/stage cap is 64.
All addresses are candidate Wii layouts except where independently checked
against original DOL instructions/live observations; see the design document.

The headless harness uses these exact columns. The inherited route writer is
a different format and is not a compatible replay oracle. Actual actor up uses
`actor_up_*`; the old route writer's `up_*` fields are gravity-derived. KPAD
export, guest tick timestamps, and a separate VI counter are not implemented
in this collector milestone.

Each successful run creates `oracle.csv`, `run.log`, `manifest.json`, and copies of the
initial available INI files. The manifest records disc/emulator hashes, CPU
core, RTC, input method, requested launch, captured row count and failure.
On observer failure the CSV remains `oracle.csv.partial`; errors appear in
the log and manifest, not an extra column that would break the shared schema.
Only compare successful runs, after checking their manifests. Never treat two
matching partial files as a passing replay.
Keep required save/checkpoint fixtures outside the team tree. Fresh M0 profiles
can be regenerated; their accepted traces and initial INIs are archived before
pruning. Never label a run qualified until a second fresh-save run with the same
movie matches every semantic row.

## Scripted filtered KPAD input

`collect --input-script tools/port/replaydiff/inputs/galaxy-route.json` runs a
frame-indexed script through the original game. It cannot be combined with
`--movie`. `--gui` permits menu-based private checkpoint creation during
exploration; it uses temporary execute breakpoints and remains unqualified. This is input gate A: it bypasses KPAD filtering and does
not qualify raw Wiimote/KPAD behavior.

The adapter verifies original DOL instructions at `KPADReadEx` (`8054b440`) and
its WPadHolder return (`804cf884`, call at `804cf880`). It lets the original SDK
read finish, then writes one complete synthesized 240-byte sample to its captured
output buffer, the captured error word, and GPR3 returned count. It checks all
three by reading back. Temporary execute breakpoints advance from the entry and
return instruction to pc+4, allowing pending interrupt handling before that
instruction is reached; unexpected stops still fail closed. No scene, player, RNG, save, instruction or PC writes are
issued. Unknown callers, nested reads, non-120 capacities and invalid pointers
fail closed. Channel 0 is connected Nunchuk with one sample; other channels have
zero samples returned, error -1 and an explicit disconnected slot 0. The native
adapter injects the same return values and samples at KPADReadEx.

Script v1 has `version: 1`, a declared `frames` count equal to `--frames`, and
ordered disjoint half-open `segments`. Gaps mean idle, no pointer. Each segment
has `start,end`, optional `buttons` (A, B, PLUS, MINUS, ONE, TWO, HOME, directions,
C, Z), `pointer: [x,y]` and `stick: [x,y]`. Both coordinate pairs use normalized
-1..1; pointer y increases downward, before the game's screen mapping. No world
positions or adaptive steering are part of the script. Button edges are generated
per channel read: trigger on 0->1, release on 1->0; repeated reads do not repeat
edges. Pointer validity is 2 while present, horizon (1,0), distance 2. MotionPlus
and unknown words are zero, with full upright acceleration and Nunchuk samples
from the native contract's `idle_sample`. Exact float32 values and bits are
exported for every field.

Every observed read is exported to `inputs.jsonl` using the strict native schema
from `native/tools/headless.py`, including boot reads before frame 0's seam,
per-frame global ordinals, actual capacity, all samples, errors and connection
state. The file remains `.partial` on failure. The run also pins script, adapter
and contract SHA256 hashes, and copies the script. The native harness consumes
that JSONL through its existing pack/run commands; it must not guess one read
per channel per frame. `python3 tools/port/replaydiff/kpad_script.py SCRIPT
/private/path/inputs.jsonl` expands the public script using the declared
WPadHolder contract (channels 0,1,2,3 in that order, capacity 120, once per loop).
This expansion must match the actual Dolphin export byte for byte over every
frame before it becomes an accepted shared input fixture. The expander alone
is synthetic input, not proof of the actual call schedule. The small public script under `inputs/` contains only
intentional controller input, with no game data. Original traces and expanded
read exports stay outside the checkout.

`--live-input-script` is exploration only. It reloads edits at input reads and
rejects any change to past/current frame intent, recording revision hashes and
the final script. Use a stable scratch copy while other lanes check files out
in the worktree. Copy the final script into `inputs/`, then run two fresh-profile
canonical repeats without this flag; a live exploration is not a repeatability
qualification.

## Deterministic input movies

Use `collect --movie /private/path/input.dtm` for a Dolphin TAS recording.
`make_movie.py` also encodes a small poll-indexed script for **2609a only**,
using its `DesiredWiimoteState` serialization. It writes a boot movie containing
one Wiimote and Nunchuk; it changes no guest code or game save. Input polls
are **not** game-loop frames and are not the game's filtered KPAD samples.

```json
[
  {"polls": 1200},
  {"polls": 30, "buttons": ["A", "B"], "ir": [512, 384]},
  {"polls": 600, "ir": [512, 384]}
]
```

Store that JSON outside the repository, then run:

```sh
python3 tools/port/replaydiff/make_movie.py /private/path/input.json /private/path/input.dtm
```

Supported buttons: `A,B,+,-,1,2,HOME,UP,DOWN,LEFT,RIGHT`; `stick` is two raw
0..255 Nunchuk values (default 128,128); `c,z` are bools; `ir` is the raw camera
center with two lights 100 pixels apart. Missing IR means no visible lights.
The Wiimote uses Dolphin's default upright acceleration; Nunchuk uses explicit
upright calibrated raw acceleration. This generator sets no movie config
override and requires the pinned profile. It is not a portable DTM adapter or
evidence that any particular menu/galaxy route has been reached.
The synthetic header uses upper bounds for frame/tick duration; input byte
count terminates playback. In particular `tickCount` must be nonzero: 2609a
ends playback once its emulated ticks exceed that header value. The manifest
pins the actual movie hash and observed game-loop count separately.

Format references at the pinned revision:
[Movie.h](https://github.com/dolphin-emu/dolphin/blob/409881c34357d0f105fd473167d15ab0bd9c1628/Source/Core/Core/Movie.h),
[DesiredWiimoteState.cpp](https://github.com/dolphin-emu/dolphin/blob/409881c34357d0f105fd473167d15ab0bd9c1628/Source/Core/Core/HW/WiimoteEmu/DesiredWiimoteState.cpp),
[GDBStub.cpp](https://github.com/dolphin-emu/dolphin/blob/409881c34357d0f105fd473167d15ab0bd9c1628/Source/Core/Core/PowerPC/GDBStub.cpp).

## Checks

```sh
python3 -m unittest discover -s tools/port/replaydiff -p 'test_*.py'
```

These exercise protocol failure handling and the state observer with synthetic
RAM, shared native column order, and movie encoding. Live boot/gameplay capture
and repeatability are separate qualifications.

## Trace comparison and first-divergence reports (Sol F)

`tracediff.py A.csv B.csv` compares every column. Column order may differ;
column sets must agree. It joins by the original zero-based `frame`, and compares
`scene_epoch`, scene, stage and scenario exactly at that frame. It never shifts
rows after a missing frame, scene change, death, or other mismatch. Both traces
must have ordered, contiguous frames beginning at zero. CSV cell count, duplicate
headers/keys, observer errors, malformed numeric data and nonfinite floats fail
closed. Matching title/loading rows with invalid player state are allowed;
validity bits must be populated and agree; other empty/unavailable cells must
agree. Invalidity never hides
populated columns from comparison.

```sh
python3 tools/port/replaydiff/tracediff.py \
  ~/MacioGalaxy2-team/scratch-port/oracle.csv \
  ~/MacioGalaxy2-team/scratch-port/native.csv \
  --expected-frames 600 --history 120 \
  --json ~/MacioGalaxy2-team/scratch-port/first-divergence.json
```

Exit **0** means all compared rows agree within their budgets, **1** means a
behavioral divergence, **2** means an invalid/incomplete comparison (including a
schema mismatch or missing frame). `--expected-frames` detects two equally
truncated traces; without a declared length, a CSV alone cannot prove either
producer reached its intended end. Preserve/check the producer manifests too.
A later recovery never changes a divergence into a pass.

Default float policy follows the design's provisional budgets:

| Columns | Absolute tolerance |
| --- | --- |
| Position `x,y,z`, `sim_*` | 0.01 |
| Velocity `vx,vy,vz`, `vel_after_*`, `actor_v*` | 0.001 |
| Front, up and gravity components | 0.0001 |

The test is `abs(a-b) <= abs_tol + rel_tol * max(abs(a), abs(b))`, with relative
tolerance `1e-6`. Known integers, validity bits, strings, state stacks, flags,
counts, RNG and all other columns are exact. Unknown columns default to exact;
new floating fields need an explicit policy entry. `--abs EPS --rel EPS` sets
global float budgets. `--policy /private/path/policy.json` provides per-column
rules, for example:

```json
{"vx": {"type": "float", "abs": 0.001, "rel": 0.000001},
 "custom_speed": {"type": "float", "abs": 0.0001},
 "rng_seed": {"type": "int"}}
```

Rules support `float`, `int`, and `exact`. Unknown policy column names and
negative/nonfinite tolerances are errors. Record intentional policy changes with
the fixture; wider budgets are not evidence of equivalence.

Float `_bits` accept both eight hex digits and `0x` plus eight hex digits. When
present, they must round-trip with the decimal float32 value, and be available
whenever the corresponding decimal is available. Bit differences below threshold
are reported without failing a tolerated float; unpaired bit columns remain
exact. The report preserves the first bit difference, even below threshold,
plus ULP error, both values/bits, numeric deltas/tolerances, vector errors, the
first diverging frame and columns, the previous N frames of those columns, and
later frame/column counts and recovery. If bit columns are absent, diagnostic
bits are reconstructed from the decimal float32 value and labeled accordingly.
`--json -` emits only JSON. Reports include source CSV paths/SHA-256 and policy;
keep reports outside git alongside their private traces.

Actor tables use the same comparison policy and one row per actor per frame:

```sh
python3 tools/port/replaydiff/tracediff.py /private/oracle-actors.csv \
  /private/native-actors.csv --actors --json /private/actor-diff.json
```

The default identity is `(actor_name, actor_id)`. Repeat `--actor-key COLUMN` to
choose other stable identity columns. IDs must identify the same actor in both
runs, never host/guest addresses or publication order. Reordering actors within
a frame is allowed; duplicate identities and inconsistent scene context within
a frame are errors. Missing actors are behavioral differences. At the first
bad frame, the first actor is selected by sorted stable identity (there is no
within-frame execution ordering in a CSV table). History is restricted to that
actor. Include the normal frame trace as the completeness gate; a sparse actor
table cannot describe a completely empty actor frame without an agreed sentinel.

## Writer tracing at the first divergence (Sol F)

`watchwriter.py` reuses `dolphin_collect.RSP` and its caller-filtered game seam.
It replays the same private movie/profile prefix, counts accepted loop stops to
**F-1**, arms a `Z2` write watchpoint, and records stops until accepted seam **F**.
It resolves Wii pointer chains at F-1; frame 0 has no previous seam and is
rejected. Supported `--field` recipes include position/simulation position,
velocity/after/actor velocity, front/up/gravity, Mario flag words, HP, dead,
mode, individual `state_flag0..2`, and MR `rng_seed`. Other fields, including
arbitrary actor-table rows, need a replay-specific `--address 0x... --size 4`.
The initial and final bytes, ordered events, last observed event and counts per
PC are recorded; symbols use sized functions from `config/SB4E01/symbols.txt`
and never attribute PCs in gaps to the previous function.

Prepare a **fresh isolated profile** with the collector as above, preserving the
fixture's exact save/NAND prefix. Use its socket argument consistently. For
example, after preparing a private `writer-user` with socket `writer.sock`:

```sh
python3 tools/port/replaydiff/watchwriter.py \
  --diff ~/MacioGalaxy2-team/scratch-port/first-divergence.json --field vx \
  --dolphin ~/MacioGalaxy2-gamedata/Dolphin-2609a.app/Contents/MacOS/Dolphin \
  --user ~/MacioGalaxy2-gamedata/writer-user \
  --socket ~/MacioGalaxy2-team/scratch-port/writer.sock \
  --movie ~/MacioGalaxy2-gamedata/accepted-input-m0.dtm \
  --output ~/MacioGalaxy2-team/scratch-port/writers.json
```

`--diff` supplies the first frame, and selects the first supported divergent
field unless `--field`/`--address` is explicit. Alternatively give `--frame F`.
Use the original fixture's movie and configuration; this tool does not establish
prefix determinism or restore a native checkpoint. The pointer recipe is
resolved again at F; object replacement invalidates the trace. Raw addresses
have no such lifecycle check and are the caller's responsibility.

Launching Dolphin automatically wraps the entire invocation in
`lockf -k ~/MacioGalaxy2-team/slots/native.lock nice -n 10 ...`, terminating the
child and removing owned breakpoints/watchpoints on exit. Attach mode requires
an already isolated, separately locked emulator and either `--initial-stop`
(assert boot stop before the first game seam) or `--start-frame N` (assert the
currently stopped accepted seam's already-counted ordinal). Attach never starts
or terminates the external emulator. Frame numbering is never guessed. Never
attach concurrently with the collector or another debugger. Private socket,
profile, disc, movie and output paths must be outside this repository; existing
reports are not overwritten. The optional launch log is `<output>.log`.

`--mode auto` tries `Z2`, then falls back to bounded instruction stepping if the
stub rejects it. `--mode watch` requires watchpoints; `--mode step` forces the
fallback. Stops with RSP `watch` addresses are checked against the requested
range. Sol C's saved 2609a GDBStub source accepts `Z2` but sends only PC/SP in its
`T05` reply; in an exclusively owned session, non-seam/non-candidate replies of
that exact form are recorded as **presumed memory checks**. Other unclassified
stops fail closed. The report keeps raw trap PCs: before/after-instruction
watchpoint timing and resume semantics must be qualified on the actual pinned
build before identifying a precise writer instruction from those PCs. Repeated
traps are bounded by `--max-stops`; use step mode if resume cannot progress.

The step fallback reports every observed byte change at the instruction stepped,
and recognizes common unconditional integer/floating stores (plus `stmw` and
`dcbz`) to include same-value writes. Same-value paired-single/string/conditional
stores can be missed; reports explicitly state this limitation. Repeat
`--candidate 0xPC` to record execution hits separately from proven write events;
watch mode uses removable instruction breakpoints and step mode checks each PC.
Candidate execution alone is never reported as a write.

Defaults: 2,000,000 steps, 100,000 stops, 900-second wall budget after connection,
60-second connection timeout, 120-second per-stop timeout. Wall budget checks
occur between debugger operations; a pending read can additionally consume one
per-stop timeout. Limits are configurable. Incomplete/error runs retain partial
events in the private report and exit **2**; a completed bounded window exits
**0**, with its stated qualification limits. Synthetic unit tests exercise the
comparator, watch traps, fallback, symbol bounds, pointer guards, cleanup and
private artifact flow; a live game replay remains a separate qualification.


Script segments may supply `acc: [x,y,z]` in raw KPAD g units, finite within
+/-8g. Default is `[0,-1,0]`. Exported `acc_value` is the vector magnitude and
`acc_speed` is distance from the preceding injected core sample, per actual
read. These synthesized filtered fields are explicit controller intent; they
do not claim natural WPAD/KPAD filtering equivalence. See inputs/stage-start.json
for the proposed300-loop idle/walk/jump/spin suffix (not live-qualified).

## Stage-sync comparison qualification (Sol F)

`tracediff.py` automatically validates adjacent `<trace>.sync.json` files if either
exists. `--require-sync` requires both even when neither exists. Each v1 sidecar
must describe `phase: "post-endFrame"`, the canonical `stage`, positive placed
`scenario`, original `source_frame`/`source_scene_epoch`, nullable u64
`source_ticks`/`source_vi_count`, observed u32 `rng_seed`, and boolean
`rng_valid: true`/`rng_forced`. Row 0 must be frame 0, scene epoch 0, valid scene
`Game`, initialization state 6, and match the declared target and observed seed.
`player_valid` remains an observation; it does not shift the selected origin.
Both post-sync seeds must agree. `rng_forced` may differ when native explicitly
injects Dolphin's observed seed once; it remains recorded as provenance.

```sh
python3 tools/port/replaydiff/tracediff.py oracle.csv native.csv \
  --require-sync --manifest-a oracle/manifest.json \
  --manifest-b native.csv.manifest.json --json comparison.json
```

Absent overrides, the comparator looks for `<trace>.manifest.json`, then
`manifest.json` beside that CSV. Each manifest needs an explicit flat
`timing_policy` object with observed `tick_hz`, `rate_numerator`,
`rate_denominator`, `epoch_ticks`, boolean `fpcr_fz`, and strings `audio_output`
and `audio_processing`. Tick rate and rational rate terms are positive integers;
epoch ticks may be zero. Additional substantive policy fields must match too.
A sibling `rtc_epoch`, when declared, also participates. Descriptive
`tick_hz_basis`, `rate_basis`, and `fp_policy` strings remain provenance rather
than equality requirements. Unknown/null policy values, missing metadata,
incomplete manifests, and incompatible target/phase/seed/clock/FZ/audio settings
return **2**. The current Dolphin collector's unobserved VI rate and FZ therefore
leave cross-platform comparisons explicitly unqualified; native defaults are
never substituted for Dolphin observations.

The report retains full state differences and an `initial_non_rng_state` check
even when metadata validation returns 2. Check that initial state before
attributing later divergence. Raw source clocks/origins remain in the report;
CSV clocks and other columns retain their usual comparison rules. Synchronizing
a global RNG seed cannot establish equal initialization, other RNGs, or
scheduling. Row 0 describes inputs that happened before the sync seam; common
post-sync gameplay reads start the next loop. Input tapes remain explicit.

For native O0/O2 diagnostics, keep settings, clock epoch, assets and input tapes
identical apart from optimization. A mismatch diagnoses native/compiler-order,
undefined-behavior, or nondeterminism issues; it does not by itself establish a
decompilation bug. A compatible metadata report is only a metadata check, not a
runtime replay qualification. Compare the normal frame trace as the gate for
actor-table comparisons; actor tables are not accepted as sync origin records.
Legacy CSVs without sidecars still compare, explicitly marked `not-requested`
for synchronization.

Diagnostic placeholder binaries cannot qualify a synchronized replay, even when
all recorded state and policies match. A manifest with
`diagnostic_placeholder: true` or a `diagnostic-*` status returns 2 while retaining
the initial-state and full-diff diagnostics.
