# Original-game Dolphin collector

`dolphin_collect.py` observes the original **SB4E01** game over Dolphin's GDB
remote protocol. It does not call guest functions or write guest RAM/registers.
Every CSV row is one accepted `GameSystem::frameLoop` call to
`MainLoopFramework::waitForRetrace`: PC `804be280`, LR **`804b6ac4`**. VI frames,
movie polls, and the debugger's single steps are separate clocks.

Use Python 3.10+ and macOS `lockf`. No additional Python packages are required.
The installed 2017 Ishiiruka app lacks the GDB configuration strings. Initial
qualification uses a separate official **Dolphin 2609a**, revision
`409881c34357d0f105fd473167d15ab0bd9c1628`, in the team scratch directory.
The app in `/Applications` is read-only. See `docs/port/REPLAY_DIFF.md` for the
layout evidence, validation status, and remaining replay qualifications.

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
  --dolphin ~/MacioGalaxy2-team/scratch-port/solc/Dolphin-2609a.app/Contents/MacOS/Dolphin \
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
Do not pass the internal `--slot-held` argument manually. Without `--dolphin`,
it attaches to an already isolated, separately locked emulator; it does not
acquire the slot or stop that externally managed emulator.

The GDB endpoint is a private Unix socket, default
`~/MacioGalaxy2-team/scratch-port/solc/gdb.sock`. TCP GDB listeners are disabled:
Dolphin's TCP implementation binds all interfaces. The first stop can report
SIGTERM (`T0f`); subsequent continue/step operations must report SIGTRAP.
The collector verifies the original caller's `bl` instruction before running.
It removes the breakpoint, steps one instruction, then reinstalls it to avoid
recounting a stopped instruction. Unexpected stops, invalid nonnull pointers,
bad counts/strings/booleans, cycles, and nonfinite floats fail the capture.

Dolphin can leave its Unix socket after exiting. Confirm no emulator owns it
before removing a stale socket. A new output directory is required each run.
Connection timeout defaults to 600 seconds **after acquiring the shared slot**;
per-stop timeout is 120 seconds; capture wall timeout is 900 seconds. All can
be set on the command line. Exit 0 means the requested rows were captured,
not that determinism or native equivalence passed. Exit 2 means an incomplete
run. Partial rows and their error fields remain available for diagnosis.

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
Preserve the user directory alongside the run. Never label a run qualified
until a second fresh-save run with the same movie matches every semantic row.

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
  --dolphin ~/MacioGalaxy2-team/scratch-port/solc/Dolphin-2609a.app/Contents/MacOS/Dolphin \
  --user ~/MacioGalaxy2-team/scratch-port/writer-user \
  --socket ~/MacioGalaxy2-team/scratch-port/writer.sock \
  --movie ~/MacioGalaxy2-team/scratch-port/input.dtm \
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
