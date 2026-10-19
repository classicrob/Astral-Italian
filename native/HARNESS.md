# SMG2 headless logic harness

This harness does **not** validate rendering, EFB/depth/occlusion feedback, shader or
texture output, hardware FIFO timing, audible output/latency, HBM interaction,
existing saves/save migration, Mii selection/rendering, networking/WiiConnect24,
physical controllers, Bluetooth/WPAD filtering, or MotionPlus/Balance Board
hardware. It runs with a fresh isolated NAND directory. Native KPAD results are
injected after filtering, so input gate A does not test KPAD itself. Loading/title
frames remain in the trace; unsupported paths fail rather than inventing state.

Status on base `3c133d77`: headless configure passes, and the core/FPCR, SDK
decoder, and virtual-clock test executables compile with ASan/UBSan. The 20
Python harness tests and 18 collector contract/observer tests pass. C++ runtime
checks found that Darwin pthread startup resets FPCR.FZ; worker startup now
explicitly applies the selected policy. KPAD/virtual-clock/tools CTests pass;
the FP fix and full build are queued behind the shared build lock. No game
executable or emulator was run by this lane, and no game data was copied.

## Targets and source hooks

`PETARI_BUILD_HEADLESS=ON` creates `petari_headless` with no Aurora, SDL, Cocoa
window, display link, or Metal dependency. It is mutually exclusive with
`PETARI_BUILD_GX_PROBE` in one build directory. Common native OS, DVD, NAND, SC,
VI, AI/DSP, input, heaps, JSystem/nw4r, RFL and game objects remain in the link.
SDK GD sources still build display-list data on the CPU. The renderer is
`petari_gx_null`; audio is pulled and discarded without opening an output device.

`petari_headless_components` builds the independent replay/trace/FPCR/FIFO unit
test and, in a root headless configuration, the SDK decoder and virtual-clock
integration tests. The pure components can also configure from
`native/app/headless` without any SDK source generation or third-party download.

The lead must land these unapplied proposals from
`~/MacioGalaxy2-team/scratch-port/sola/patches/`:

| Patch | Seam |
| --- | --- |
| `04-kpad-replay-hook.patch` | `KPADReadEx` delegates to the installed replay before returning SDK data; inactive replay retains its original path |
| `05-headless-game-loop.patch` | Native game entry point and after-endFrame observation seam; headless advances one virtual frame instead of waiting in display pacing |
| `06-headless-framework-wait.patch` | Startup/blanking waits use the virtual retrace driver instead of real-time tick sleeps or a display message queue |

The proposals were rebased against worktree commit `3c133d77`, checked with
`git apply --check`, and compared with `unifdef -UGALAXY_NATIVE
-UGALAXY_HEADLESS -D__MWERKS__`: Wii output is unchanged. This is a preprocessing
check, not the lead's Wii object/DOL proof. No game/library/header files were
edited in the worktree or overlay. The KPAD patch exports a version marker;
without it the full harness cannot link and silently bypass the input hook.

## Clock, retraces and audio

The default rate is rational **60000/1001 Hz**, selectable through numerator and
denominator arguments; PAL can use 50/1. Fractional ticks are retained at the
SDK's 60,750,000 Hz timebase. `--epoch-ticks` is mandatory and must match the
oracle fixture's pinned RTC/timebase epoch; no host wall clock seeds OSGetTime.
Each completed game loop is recorded after endFrame, before its wait, at the same
seam specified by [REPLAY_DIFF.md](../docs/port/REPLAY_DIFF.md).

A host interrupt worker advances the clock once at the game-loop wait. It
services alarm deadlines in order, pumps native WPAD command completions at
200 Hz without generating host input reports, pulls exactly the elapsed AI sample
budget, drains AI interrupts, and signals a VI retrace. The main OS thread releases
its CPU baton while waiting for that worker. Startup/blanking waits before the
first recorded loop advance virtual time too; they do not increment `frame`.
Additional retrace waits inside a running loop can signal VI without advancing
that loop's fixed time. Native VI/tick counters are written to the separate
`.timing.csv` sidecar. The common schema v1 reserves `vi_count` and `ticks` as
blank columns until the Dolphin collector exports them too; a VI event is
never silently counted as a game-loop ordinal.

The SDK alarm queue is used; its host timer thread does not start in virtual
mode. Expired periodic alarms are dispatched at each exact deadline rather than
coalesced at the frame endpoint. Enabling virtual time after alarms exist is
rejected. Clock overflow/backward movement is rejected. Normal graphical builds
retain the wall clock and normal retrace behavior unless the new APIs are
explicitly selected.

AI/DSP and JAudio are still active. AI samples are pulled in at most 1 ms virtual
chunks and discarded, with elastic wall-clock registration waits disabled.
There is no SDL/CoreAudio device and no audio thread paced by the host device.
DSP renderer/mailbox and OS audio threads still exist: this is not a replacement
JAudio implementation or a claim of DSP equivalence.

FPCR.FZ is set on the main thread **before** worker/game threads are created.
`--no-ftz` clears it. Darwin resets FPCR when creating pthreads, so OS game
threads and native VI/DVD/NAND/GX/AI/DSP/alarm workers explicitly apply the selected
policy at entry. The native test multiplies two normal operands to produce a
denormal, checks gradual underflow with FZ off and zero with FZ on, tests worker
startup in both modes, and checks restoration of the original FPCR. A virtual-clock
integration test also checks real OSCreateThread workers in both modes. Explicit FMA
operations are not removed; automatic contraction remains disabled with
`-ffp-contract=off`.

## Replay interchange

Use [native/tools/headless.py](tools/headless.py) for the JSONL input contract and
launcher. The collector can import its `encode_sample`/`encode_rows` validators;
no DTM conversion or guessed WPAD-to-KPAD filtering occurs here. The shared trace
column list is [trace_schema.json](app/headless/trace_schema.json), generated by
`python3 native/tools/headless.py schema` from `trace_fields.inc`. Its 86 columns
and schema version 1 match `tools/port/replaydiff/dolphin_collect.py`; tests now
check the contract in both directions. That collector landed while this harness
was being implemented; filtered KPAD export is still pending. The strict trace is distinct from
the older, rounded waypoint route CSV and its adaptive smoke replay.

Each input JSONL row contains exactly:

- `frame`: zero-based completed-loop ordinal; boot reads before the first seam
  belong to frame 0 and must be present in that frame's ordered read prefix.
- `read_ordinal`: global read order within that frame, starting at 0; `channel`
  is independent and must match every actual read, including repeated reads.
- `requested_length`, `returned_count`, and signed `error`. SMG2 WPadHolder asks
  for **120** slots; the tape preserves that request and rejects any mismatch.
- `connection`: `probe_error` and `device_type`, used for WPADProbe and connection
  callbacks. Conflicting connection states within one frame are rejected; those
  transitions need timestamped raw input/event capture.
- `samples`: the complete newest-first list. For zero/error returns it has one
  entry containing the captured slot 0. The unused buffer tail is left alone.

Samples use the named members of SMG2 KPADStatus (`hold/trig/release`, vectors,
all pointer/distance/horizon values, device/error/format fields, the named
`ex_status.fs` or `.cl` union, all `mpls` vectors/axes, and `unk_EC`). Every float
has `{"value": <decimal>, "bits": "0x........"}`. Bits and decimal must agree
when rounded to float32, including signed zero. NaNs, infinities, duplicate JSON
keys, missing/extra sample fields, unknown extension unions and malformed tapes
fail with status 2. Core, Nunchuk and Classic sample unions are supported;
Balance Board/unknown unions are refused. MotionPlus fields are retained but no
MotionPlus hardware/service fidelity is claimed.

The launcher packs named JSONL to a bounded big-endian `SMG2KPD1` tape. This is a
local interchange cache, not a memcpy of host structures. The decoder assigns
fields by name and width, avoiding arm64 padding, endianness and union-layout
assumptions. No slot truncation, adaptive steering, resynchronization, RNG reset,
teleportation or missing-read padding is allowed. A missing, extra, wrong-channel,
wrong-capacity, or wrong-thread KPAD read invalidates the run. Built-in
PadRecorder playback is rejected at observation; its project-buffer override
must stay off for this gate.

Connection/extension transitions are applied before the next virtual retrace;
newly registered callbacks also receive the current attached state. Command
completions run at virtual 200 Hz. This scheduling policy still needs comparison
with captured callback/command timestamps before connection changes or device
management become equivalence assertions. Raw WPAD gate B remains separate.

## Trace and validity

The CSV writer emits every loop, starting at frame 0, and flushes each row. It
writes float32 round-trip decimals and a corresponding `_bits` column. Strings
are CSV escaped. It rejects nonfinite observed floats, invalid group bounds,
null registered actors, cyclic/oversized state stacks, and unterminated strings.
Observer errors produce an invalid/incomplete run, not a row of plausible zeros.

Columns cover scene/stage/scenario/init state, scene epoch, actor and simulation
position, Mario pre/post velocity, actor velocity, front, actual actor up,
Mario air gravity, actor gravity, raw movement/draw words, three state flags,
ordered state stack, pending state, mode, current animation name, HP/dead, MR RNG
seed, and registered/living actor counts. VI count/ticks are recorded separately
in the timing sidecar while the common v1 columns stay blank. The RNG is read without
calling rand; counts come from the actual AllLiveActorGroup, not the capped
native observation publication store. Pending null state is blank, distinct
from ID 0. `state_ids` is an ordered JSON array inside a CSV cell;
`state_flags3` is three decimal booleans separated by semicolons.

`scene_valid`, `player_valid`, `actor_count_valid`, `rng_valid` and
`animation_valid` gate their groups. Fields in invalid groups are empty. Player
and group pointers are traversed only after SceneInitializeState_End in the
`Game` scene. This is typed host member access; no Wii numeric offset is applied
to a host object. `actor_up_*` reads actor `_270`, and `anim_name` reads
MarioAnimator's `mCurrBck`. Their original-DOL mapping and lifecycle assumptions
still need live oracle qualification, as do the guessed/inherited fields noted
in REPLAY_DIFF.md. Scene epoch follows scene pointer/scene-stage-scenario changes;
addresses themselves are never emitted or compared.

The launcher writes a sidecar `.manifest.json` with input hash, rate, epoch, FZ,
column list, and status. Success requires exit 0 **and** the exact shared CSV
header, contiguous requested frame count, complete timing sidecar and matching
`.complete` frame-count marker. Early game exit, a crash, missing reads, observer
failure, or a missing marker leaves an invalid run. An incomplete CSV is never a
pass. Pin disc/DOL/NAND hashes, native revision and compiler settings alongside
this sidecar as required by REPLAY_DIFF.md; the launcher does not manufacture
an oracle qualification record.

## What is still nondeterministic or unqualified

- The existing OS priority/CPU-baton scheduler remains; wall-clock forced
  preemption is disabled in virtual mode. There is no instruction-count or
  cooperative single-thread rewrite of the game. Equal-priority readiness,
  asynchronous DVD/NAND I/O, DSP work and interrupt delivery can still vary.
- Fixed AI sample budgets and drained AI interrupts do not prove JAudio/DSP
  work completes at exactly the same virtual instant as Dolphin. No audio
  timing equivalence or repeated-run determinism has been measured yet.
- Boot waits advance virtual time on demand. Different boot paths/I/O completion
  ordering can change the boot tick/retrace prefix; preserve and compare it.
  Unsupported blocking paths without a virtual wait/event source may still hang
  and need an explicit seam; a wall-clock watchdog must never advance game time.
- GX state/draw calls intentionally discard rendering. CPU projection, texture
  metadata/size and vertex-format queries have implementations. Depth/EFB and
  other unsupported readbacks panic. Direct command/vertex bytes and display
  list capture/padding are counted separately from discarded state calls;
  display-list payloads are not executed as GPU programs.
- Null GX completes processing on submission, uses platform token/draw-done
  delivery, and reports breakpoint notifications immediately. It does not model
  a stalled hardware FIFO, GPU timing, token-time EFB snapshots, or rasterization
  side effects. These are explicit exclusions, not equivalence results.

## Build and run, only after the lead says “go”

All game data, extracted disc files, input logs and traces belong outside the
checkout. Numeric traces/input scripts may use team scratch. The newly landed
Dolphin collector additionally excludes disc data and NAND profiles from the
team upload tree, using `~/MacioGalaxy2-gamedata/`; use that private location for
shared oracle/native disc and save fixtures. The earlier native-only location
`~/MacioGalaxy2-team/scratch-port/game-data/` remains outside the checkout but
cannot be passed as a disc/profile path to that collector. No data was moved by
this task. Do not commit or upload game data. The harness/launcher
reject paths in the checkout, including through symlinks. The user directory
must be fresh; no existing save is overwritten.

The machine permits one heavy build or emulator run at a time. Keep the shared
lock and nice priority, check the 40 GiB free-space floor first, and use at most
six build jobs. **These commands have not been run for this task.**

```sh
df -h /
lockf -k ~/MacioGalaxy2-team/slots/native.lock nice -n 10 cmake --preset macos-arm64-headless
lockf -k ~/MacioGalaxy2-team/slots/native.lock nice -n 10 cmake --build build/macos-arm64-headless --target petari_headless_components -j 6
lockf -k ~/MacioGalaxy2-team/slots/native.lock nice -n 10 ctest --preset macos-arm64-headless
lockf -k ~/MacioGalaxy2-team/slots/native.lock nice -n 10 cmake --build build/macos-arm64-headless --target petari_platform_os_tests petari_platform_vi_tests petari_input_remap_tests -j 6
lockf -k ~/MacioGalaxy2-team/slots/native.lock nice -n 10 ctest --test-dir build/macos-arm64-headless -R 'native_(platform_os|platform_vi|input_remap)$' --output-on-failure
lockf -k ~/MacioGalaxy2-team/slots/native.lock nice -n 10 cmake --build --preset macos-arm64-headless -- -k
```

The component-only alternative is `cmake -S native/app/headless -B
build/headless-components -DBUILD_TESTING=ON`, with the same lock/priority and
`-j 6` on its build. It does not build the game or platform integration tests.

Generate the synthetic 600-idle/one-jump/release fixture outside the repository:

```sh
python3 native/tools/headless.py generate ~/MacioGalaxy2-team/scratch-port/game-data/idle-jump.jsonl
```

This is synthetic data for exercising the harness, **not** a captured Dolphin
fixture or proof that the game's boot/read sequence has exactly four reads per
loop. It fails if the actual read prefix differs, as a strict driver should.

After source hooks land and the full game links, use the launcher with actual
captured `inputs.jsonl`, frame count and epoch, for example:

```sh
python3 native/tools/headless.py run \
  --executable build/macos-arm64-headless/native/app/headless/petari_headless \
  --inputs ~/MacioGalaxy2-team/scratch-port/game-data/fixture/inputs.jsonl \
  --frames 602 --epoch-ticks 0 \
  --disc ~/MacioGalaxy2-gamedata/SB4E01 \
  --user ~/MacioGalaxy2-gamedata/fixture/native-user-new \
  --trace ~/MacioGalaxy2-team/scratch-port/game-data/fixture/native.csv
```

The launcher itself wraps the executable as `lockf -k <shared-lock> nice -n 10
<executable> ...`; do not add an outer copy of the same lock around this launcher.
Epoch 0 above is only an example, not the expected oracle epoch. No disc image,
Dolphin run, or full native run is launched by the unit tests.
