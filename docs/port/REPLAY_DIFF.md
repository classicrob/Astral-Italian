# SB4E01 replay differential gate

Design, 2026-10-19; implementation and live replay validation remain outstanding. Use the user's original SB4E01 disc/DOL as the Dolphin oracle, with its disc files
available to both runs. No game data, saves, movies, or savestates enter git.
`M/` below means the read-only model `/Users/roberthaisfield/MacioGalaxy/`; unprefixed source paths are in this SMG2 checkout. All offsets below are **Wii
32-bit offsets**, not arm64 layouts. Native observers use typed member access.

## Decision and frame contract

Record a Dolphin TAS movie and simultaneously export the actual KPAD samples consumed by the original game. Replay those samples natively; compare state
after each completed game loop. This first gate tests gameplay downstream of
KPAD. A second, raw WPAD-report gate must test the input implementation itself.
DTM bytes alone do not specify the game's filtered KPAD history.

Use a GDB remote-protocol collector against a pinned Dolphin build. Stop at
`waitForRetrace__17MainLoopFrameworkFv`, **0x804BE280**
(`config/SB4E01/symbols.txt:41319`), accepting only calls from GameSystem's loop.
The loop orders draw, update, calcAnim, capture, endFrame, waitForRetrace
(`src/Game/System/GameSystem.cpp:204`). This is equivalent to the model's native
seam between endFrame and waitForRetrace (`M/src/Game/System/GameSystem.cpp:347`),
where route recording runs first (`M/native/app/frame_seam.cpp:791`).
Count accepted stops as `frame` (u64, zero-based); record VI/tick counts separately.
Do not treat a rendered frame, retrace, or Mario update as this loop counter.
`frameLoop__10GameSystemFv` is **0x804B6A70** and `update__10GameSystemFv`
**0x804B6BA0** (`config/SB4E01/symbols.txt:40920`, `:40922`).
Log every loop, including loading/title/pause; attach `scene_epoch` and validity bits. Never silently drop invalid frames or realign after a death/divergence.

## State extraction map

Notation: `p32(a)` is a big-endian guest pointer read, `f32`/integers are decoded big-endian. Validate each pointer against mapped MEM1/MEM2, lengths and lifecycle;
an invalid read is an observer error, not a zero-valued state. No guest calls.
Only traverse scene/player chains when `C+0x130 == SceneInitializeState_End (6)`
and the current scene is `Game`; otherwise emit invalid player/group columns
(`include/Game/System/GameSystemSceneController.hpp:33`; model guard `M/native/app/smoke_game.cpp:215`).

Root and player chain:

* `G=p32(0x807D0DA4)`, symbol `sInstance__29SingletonHolder<10GameSystem>`
  (`config/SB4E01/symbols.txt:73541`; singleton access: `include/Game/Util/SingletonHolder.hpp:14`).
* `O=p32(G+0x20)`, `C=p32(G+0x24)` (object holder / scene controller:
  `include/Game/System/GameSystem.hpp:52`). `S=p32(C+0x12C)`,
  `H=p32(S+0x10)` (scene / SceneObjHolder:
  `include/Game/System/GameSystemSceneController.hpp:116`, `include/Game/Scene/Scene.hpp:23`).
* `MH=p32(H+4*0x15)`, `A=p32(MH+0x14)`, `P=p32(A+0x584)`
  (MarioHolder / MarioActor / Mario). Scene IDs and array base:
  `include/Game/Scene/SceneObjHolder.hpp:26`, `:173`; holder member:
  `include/Game/Player/MarioHolder.hpp:16` after NameObj's 0x14-byte Wii base
  (`include/Game/NameObj/NameObj.hpp:32`); Mario pointer:
  `include/Game/Player/MarioActor.hpp:880`. The accessor chain is confirmed by
  `src/Game/Player/MarioAccess.cpp:588`, `src/Game/Player/MarioHolder.cpp:31`.
* Corresponding function anchors: `getPlayerActor__11MarioAccessFv`
  **0x803B7310**, `getMarioHolder__2MRFv` **0x803EDFE0**
  (`config/SB4E01/symbols.txt:33257`, `:34238`). These are code, not pointer slots.

| Per-frame columns | Guest extraction and code evidence |
| --- | --- |
| `x,y,z` | `f32[3](A+0x14)`, LiveActor position (`include/Game/LiveActor/LiveActor.hpp:103`). Also log simulation position `f32[3](p32(P+0x138)+0x0C)` (`include/Game/Player/Mario.hpp:795`, `include/Game/Player/MarioPosHolder.hpp:19`). |
| `vx,vy,vz`, `vel_after_*` | `P+0x160`, `P+0x178` (`include/Game/Player/Mario.hpp:799`); also `A+0x38` actor velocity (`include/Game/LiveActor/LiveActor.hpp:106`). Distinguish these; the public velocity getter can return last-move instead (`src/Game/Player/MarioAccess.cpp:346`). |
| `front_*`, `up_*` | `P+0x208`, `A+0x618` (`include/Game/Player/Mario.hpp:813`, `include/Game/Player/MarioActor.hpp:895`). Getters confirm front and `_270` up (`src/Game/Player/MarioActor.cpp:2354`, `:2364`). |
| `gx,gy,gz`, `actor_gravity_*` | Air gravity `P+0x1D8`, actor gravity `A+0x44` (`include/Game/Player/Mario.hpp:809`, `include/Game/LiveActor/LiveActor.hpp:107`). Explicitly label air gravity: Mario's effective-gravity getter selects other vectors conditionally (`src/Game/Player/Mario.cpp:1930`). |
| `movement_lo/hi`, `flags14_lo/hi`, `draw`, `flags20`, `flags24_lo/hi`, `prev_draw` | Words at `P+0x0C/10`, `+0x14/18`, `+0x1C`, `+0x20`, `+0x24/28`, `+0x34` (`include/Game/Player/Mario.hpp:773`; word ordering `:709`). Read words, not host bitfield storage. |
| `state_flags3` | Three bool bytes `P+0xA90..A92` (`include/Game/Player/Mario.hpp:1145`, `:78`); compare individually. |
| `state_ids`, `pending_state_id`, `mode` | Ordered linked stack from `p32(P+0x980)`, next at node `+8`, status ID u32 at `+0x0C`; pending node `p32(P+0x984)`. Cap at 64, detect cycles, preserve null vs ID 0 (`include/Game/Player/Mario.hpp:1112`, `include/Game/Player/MarioState.hpp:78`; stack use `src/Game/Player/MarioState.cpp:33`, `:72`). Mode u16 `A+0x6F8` (`include/Game/Player/MarioActor.hpp:941`). IDs alone are not a complete action description; compare flags too. |
| `hp`, `dead` | HP u32 `A+0x6B0` (`include/Game/Player/MarioActor.hpp:920`, getter `:726`); dead byte `A+0x70` (`include/Game/LiveActor/LiveActor.hpp:116`, `include/Game/LiveActor/LiveActorFlag.hpp:9`). |
| `scene,stage,scenario,init_state` | Fixed 64-byte strings `C+0`, `C+0x40`; s32 `C+0x80`; initialization enum `C+0x130` (`include/Game/System/GameSystemSceneController.hpp:26`, `:108`, `:117`). |
| `rng_seed` | u32 `O+0x34`: embedded `JMath::TRandom_fast_`, not a standalone seed global (`include/Game/System/GameSystemObjHolder.hpp:61`; seed member `libs/JSystem/include/JSystem/JMath/random.hpp:32`). `MR::getRandom()` consumes exactly this generator (`src/Game/Util/MathUtil.cpp:43`). |
| `frame`, `stage_play_time` | Harness loop ordinal; optional s32 `R+0x64` play time (`include/Game/System/PlayResultInStageHolder.hpp:69`). Do not substitute the audio subframe counter. |

The up-vector and several inherited vectors have `guess-gap`/`inh` annotations in the headers above. Before enabling them as assertions, verify the accessors'
original DOL loads against these offsets and capture live sanity checks; header
annotations are the candidate map, not independent proof of layout.
Function anchors: `getUpVec__10MarioActorCFPQ29JGeometry8TVec3<f>`
**0x803B8560**, `getFrontVec__10MarioActorCFPQ29JGeometry8TVec3<f>`
**0x803B9000**, `getGravityVec__5MarioCFv` **0x8038C940**
(`config/SB4E01/symbols.txt:33281`, `:33310`, `:32665`).
RNG anchors: `getRandom__2MRFv` **0x8003B3F0**,
`__ct__Q25JMath13TRandom_fast_FUl` **0x8053A9B0**
(`config/SB4E01/symbols.txt:2524`, `:45920`). This covers the MR stream;
additional per-effect RNGs require separate fields if later evidence implicates them.

Coins and Star Bits are not MarioActor counters. In-stage chain: `D=p32(G+0x0C)`, `Q=p32(D+0)`, `I=p32(Q+0x10)`,
`T=p32(I+0x18)`, `R=p32(T+0x10)` (sequence director, sequence, in-game,
in-stage, result). Evidence: `include/Game/System/GameSystem.hpp:47`,
`GameSequenceDirector.hpp:21`, `GameSequence.hpp:94`, `GameSequenceInGame.hpp:68`,
`GameSequenceInStage.hpp:49` (last four under `include/Game/System/`).
Read signed `R+0x4C` Star Bits and `R+0x54` coins, clamp each to 0..999
to match getters (`include/Game/System/PlayResultInStageHolder.hpp:61`,
`src/Game/System/PlayResultInStageHolder.cpp:147`, `:156`).
For ship/world-map/no-pause-star stages, MR getters use stocked totals instead
(`src/Game/Util/EventUtil.cpp:114`, `:166`). Stock chain:
`Save=p32(D+8)`, `U=p32(Save+0x0C)`, `GD=p32(U)`, `PS=p32(GD+8)`;
read u16 `PS+0x0A` bits / `+0x0C` coins, clamp 0..9999
(`include/Game/System/GameSequenceDirector.hpp:23`, `SaveDataHandleSequence.hpp:88`,
`UserFile.hpp:43`, `GameDataHolder.hpp:39`, `GameDataPlayerStatus.hpp:67` under
`include/Game/System/`). Export both raw totals plus `counter_domain`; implement
the stage predicate from the MR getter, never infer it from a display label.
Anchors `getStarPieceNum__2MRFv` **0x80023960**, `getCoinNum__2MRFv`
**0x80023B70** (`config/SB4E01/symbols.txt:1466`, `:1470`);
`getPlayResultInStageHolder__20GameSequenceFunctionFv` **0x804D6FE0**
(`config/SB4E01/symbols.txt:42279`).

Actor counts: `L=p32(H+4*0x39)` is AllLiveActorGroup
(`include/Game/Scene/SceneObjHolder.hpp:62`, `src/Game/LiveActor/AllLiveActorGroup.cpp:35`).
Read registered count s32 `L+0x18`, capacity `L+0x14`, array `p32(L+0x1C)`
(`include/Game/NameObj/NameObjGroup.hpp:35`). Count living members by checking
each actor's dead byte at `+0x70`, matching `getLivingActorNum()`
(`src/Game/LiveActor/LiveActorGroup.cpp:26`). Add named group registered/living
counts when needed, with explicit scene-ID/owner chains; never compare addresses.
Anchors `getAllLiveActorGroup__2MRFv` **0x8022D950** and
`getLivingActorNum__14LiveActorGroupCFv` **0x80238330**
(`config/SB4E01/symbols.txt:20313`, `:20653`). The model's actor publication
store is capped at 256 and conditional on observation (`M/native/app/actor_observe.cpp:22`,
`:30`); its length cannot stand in for all registered/living actors.

## Dolphin choices and deterministic capture

Local inspection found `/Applications/Dolphin.app`, executable `Contents/MacOS/Dolphin`, Info.plist short version `v5.8-9`, bundle version `5.0`; `dolphin-emu` is absent
from PATH. This does not establish its upstream revision or GDB/scripting support.
Nothing was installed, launched, or configured for this design.

| Option | Per-game-frame reads and decision |
| --- | --- |
| TAS `.dtm` | Supplies replay input, not state observation. Current upstream Wii playback decodes length-prefixed serialized desired Wiimote states ([Movie.cpp:1212](https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/Core/Movie.cpp#L1212)). Do not assume the installed build has this format or one record per game frame. |
| GDB stub | Recommended first collector: execute breakpoints and CPU-thread RAM reads exist ([GDBStub.cpp:872](https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/Core/PowerPC/GDBStub.cpp#L872), [1004](https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/Core/PowerPC/GDBStub.cpp#L1004)). Stop, batch-read, then continue at the same game seam; no DOL patch required. Qualify interpreter first, then JIT separately. |
| MemoryWatcher / dolphin-memory-engine | Watcher sends changed values via datagrams ([MemoryWatcher.cpp:81](https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/Core/MemoryWatcher.cpp#L81)); it needs an added frame marker/handshake to form complete snapshots. External polling likewise needs a pause barrier. DME's macOS setup requires custom signing ([README:8](https://github.com/henriquegemignani/py-dolphin-memory-engine/blob/master/README.md#L8)). Neither is the first gate. |
| Frame dump / scripting forks | Image frames alone cannot expose these fields. Felk provides frame-advance and memory-breakpoint events ([Readme.md:48](https://github.com/Felk/dolphin/blob/scripting/Readme.md#L48)); generic frame events still need mapping to this game seam. Its README flags ARM Python packaging work ([121](https://github.com/Felk/dolphin/blob/scripting/Readme.md#L121)). Qualify a pinned fork if GDB overhead becomes material; do not assume stock Lua/Python APIs. |

The source links above were inspected 2026-10-19; pin actual revisions and verify line locations before implementation. GDB plan: enable a loopback-only stub in
a separate Dolphin user directory; implement RSP `Z0`/`m`/`c` with checksum/ACK,
stop-reason checks and bounded reads. Confirm breakpoints use emulator breakpoints,
not guest instruction writes ([GDBStub.cpp:872](https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/Core/PowerPC/GDBStub.cpp#L872)).

For each fixture pin emulator binary/revision/hash, DOL/disc hashes, region, language, video timing, controller extension/calibration, DSP/GPU/CPU settings and NAND/save
hashes. Turn dual core off; its independent CPU/GPU execution is a determinism
hazard ([Dolphin release 2412](https://dolphin-emu.org/blog/2024/12/02/dolphin-progress-report-release-2412/)).
Disable cheats, mods, real Wiimotes, network and host input during playback.
Fix RTC/OS time and native emulated clock epoch; DTM carries recording time
([Movie.h:107](https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/Core/Movie.h#L107);
[Movie.cpp:833](https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/Core/Movie.cpp#L833)).
Start from power-on with a fresh isolated save for the canonical fixture. For iteration, a fixed Dolphin savestate is only an oracle accelerator: native cannot
load it. Both must reach and match a declared initial-state checkpoint through
the same boot/save/input prefix. Never reset RNG or teleport to conceal mismatch.
Run the oracle twice and require identical semantic dumps before comparing native.

## Input interchange and native hooks (proposed)

Create `tools/port/replaydiff/` later with `capture_dolphin.py`, `schema.json`, `compare.py`, and a revision-specific DTM adapter. Store fixtures outside git;
store only schema/tests/manifests without game data in the repository.

Canonical bundle: `manifest.json`, original `input.dtm`, `inputs.jsonl`, `oracle.csv`, `native.csv`. Each input row has `frame,read_ordinal,channel`,
`returned_count,error`, the entire newest-first sample list, connection/extension
state and optional raw report events with guest-tick timestamps. Encode every
KPAD numeric field by name; keep float32 hex bits beside decimal values; exclude
padding from comparisons but retain a local raw blob for diagnosis.
SMG2's struct includes buttons, acceleration, pointer/horizon/distance, device/error,
Nunchuk union, MotionPlus and unknown `unk_EC` (`libs/RVL_SDK/include/revolution/kpad.h:38`,
`:79`); do not use SMG1's layout. Preserve unknown bytes locally, mark their
semantics unasserted. Include all four channels and zero/error returns, not only
player 1's newest stick/button values.
Array ordering is evidenced by the reverse copy (`src/RVL_SDK/kpad/KPAD.c:1450`).
Capture buffer slot 0 even for zero-count/error returns: KPADReadEx can retain or
modify it (`src/RVL_SDK/kpad/KPAD.c:1608`), and HOME reads the raw slot (`src/Game/System/WPadHolder.cpp:205`).

Original input path: `WPadHolder::updateReadDataOnly` calls `KPADReadEx` into
raw arrays, copies to project arrays, then calls `readPadData`
(`src/Game/System/WPadHolder.cpp:72`); `update()` then updates WPad objects
(`:116`), whose update feeds button/pointer/acceleration/swing/stick readers
(`src/Game/System/WPad.cpp:42`). Original built-in PadRecorder can intervene
(`src/Game/System/WPadHolder.cpp:172`); require its replay mode off.
Anchors `KPADReadEx` **0x8054B440**, `updateReadDataOnly__10WPadHolderFv`
**0x804CF840**, `updateProjectPadData__10WPadHolderFv` **0x804CF8F0**
(`config/SB4E01/symbols.txt:46169`, `:41911`, `:41912`).

Capture each KPADReadEx call's arguments and return buffer/count/error at its
return PC, keyed by thread/call ordinal; also stop before project update to verify
the copied samples. Follow `O+0x24` to WPadHolder; raw/project array pointers are
`+0x0C/+0x10`; each Wii WPadReadDataInfo has pointer/count at `+0/+4`
(`include/Game/System/GameSystemObjHolder.hpp:57`, `include/Game/System/WPadHolder.hpp:17`,
`:43`). Handle nested/other-thread reads; fail on unexpected sequence, never guess.

Native hook A: extend the existing route observer/writer with the state above, using the same game-thread lifecycle guards (`M/native/app/smoke_game.cpp:203`).
Keep current CSV column names, append versioned extras and validity fields; emit
float32 round-trip `%.9g` plus bits because current `%.3f` positions lose precision
(`M/native/app/route_record_writer.hpp:29`, `:43`). Existing `up_*` is derived
from negative normalized gravity (`:40`); retain it as `gravity_up_*` in v2 and
write actual actor up to explicitly named `actor_up_*` columns.
Existing recording stores remapped BoundState (`M/native/app/smoke_game.cpp:237`);
existing replay steers to positions and resyncs (`M/native/app/smoke_replay.cpp:193`,
`:197`), so add a separate strict input driver with no adaptive actions.

Native hook B: a GALAXY_NATIVE KPADReadEx return adapter loads the recorded samples/count/error before callers use them. The lead owns transferring the
guarded game/SDK hook; this plan changes no src/libs files. Reproduce connection
callbacks and WPADProbe results too; stop on missing/extra reads or incompatible
buffer sizes. Keep host events and smoke drivers from adding input.
This gives identical game-facing KPAD results without pretending to test KPAD.

Raw gate B: replay DTM in Dolphin, capture resulting WPAD reports plus calibration, format/extension commands and sampling ticks; feed these to a native report queue
before its KPAD sampling callback. The model compiles the SDK KPAD above its WPAD
implementation (`M/native/input/CMakeLists.txt:101`); WPADRead copies its current
report (`M/native/input/wpad_host.cpp:1254`). Use `setClock(Clock::Manual)` before
WPADInit and pump on recorded ticks (`:1076`), not wall time or a fixed reports/frame.
DTM -> Dolphin's pinned decoder/report generation -> WPAD -> game's KPAD gives
the forward conversion. KPAD -> DTM is lossy (filter history, calibration, timing);
require retained raw/desired-state events for reverse conversion. Refuse synthetic
round trips unless replay regenerates identical KPAD lists, counts and errors.

## Comparison, diagnosis and milestones

Proposed initial thresholds: position absolute 0.01 game units; velocity 0.001 units/loop; front/up/gravity components 0.0001; relative term 1e-6 for all floats:
`abs(a-b) <= abs_tol + 1e-6*max(abs(a),abs(b))`. Export ULP and vector errors too.
Require exact integers/strings/flags/state IDs/counts/RNG/input bits/validity and
frame sequence. Nonfinite values fail; do not tolerate NaNs silently. Record the
first bit difference even below threshold; fail on the first threshold crossing.
These are starting budgets to calibrate with repeated oracle runs and short
native segments, not evidence of demonstrated equivalence or permission to drift.

Build with automatic FMA contraction off; preserve deliberately fused operations. Pin ARM FPCR.FZ policy to model Gekko NI flush-to-zero, and document the accepted
frC reduced-precision difference. These are team requirements/known limitations
(`/Users/roberthaisfield/MacioGalaxy2-team/PORT-PLAN.md:17 and :18`), not measured replay results.
Never widen epsilon to hide changed RNG consumption, action transitions or collision.

`compare.py` should return 0 pass, 1 behavioral divergence, 2 invalid/incomplete run.
Report fixture/hash, first frame+field, both values and bits, error/tolerance,
preceding 120 frames, exact consumed input, stage/scenario/state stack/RNG/counts.
Keep secondary errors after the first; do not make a later reconvergence a pass.
Map fields to likely writers: input mismatch -> read/report timing; seed mismatch
-> RNG call order; state/count mismatch -> spawn/lifecycle/branch; position with
matching inputs/state -> Player integration/collision/FP. Trace only that first
window on rerun and map PPC PCs via SB4E01 symbols to decompiled functions.

Use semantic residue as a candidate list, not a verdict: realdiff emits `trial/REALDIFF-<tag>.md` (`/Users/roberthaisfield/MacioGalaxy2-team/realdiff.py:6`)
and filters proven-benign entries (`:62`). Inspect both residual output and the
unit-scoped `benign/*.txt` proof, plus current native changes; even a prior benign
Wii register/scheduling difference is not proof of native equivalence. Broaden
beyond residues when the writer trace points elsewhere. Do not run a rebuild here.

* **M0: boot-to-title sync.** Qualify installed/pinned GDB support; validate DOL
  hash, pointer loads and seam caller; record a fresh-save title movie; two oracle
  runs must match exactly. Native must match loop/input read sequences and title
  checkpoint. Diagnose async loading/time differences before enabling player fields.
* **M1: first-level position gate.** Fresh-save route through the opening and first
  playable Sky Station visit; walk, jump, spin, damage/coin/bit collection. Validate
  header candidates against DOL loads; strict KPAD replay plus all fields, 600+
  playable frames and repeated native runs. Add raw-report equivalence separately.
* **M2: CI gate.** Private local/authorized runner with user-supplied data; pinned
  manifests and fresh writable save copies; bounded frame/wall timeout; fail closed
  on schema/hash/gaps/observer errors; archive first-divergence artifacts privately.
  Require both input-adapter tests and representative gameplay fixtures. Replay
  passing certifies these recorded paths, not all game behavior.
