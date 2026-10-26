# Priority 3 design note: native rare-stall regression tapes

Cache compatibility's runtime mechanism passed M4 r5 (8/8). This is the design
handoff for the next priority, for PORT review before implementation/recording.
No recording/build/run is queued. SOL A retains the M1 audio slot. The VsKoopa
key-seeding change is separate; first bind the regression paths below.

1. Bind the actual retained VsKoopaLv1 / IslandFleet cold-stall receipt first:
   original stage/scenario, launch path, fixture/save, mods, input script and
   renderer frame/readiness evidence. Do not change the start state, skip an
   introduction or switch to an unlocked fixture just to get an easier pass.
2. On the integrated (a) marker/power player, use a recorder instrument with the
   self-contained native script capability from the ref072 producer, no AX/host
   pointer/TCC. Record the same deterministic stage/input path with a compiled
   cache. Every actual SDK read/connection is canonical; no legacy conversion.
   If a first-visit conversation requires inputs unavailable before play-ready,
   document the exact required native input seam before extending the producer.
3. New schema2 tapes bind direct-launch metadata, every game-defined phase,
   play-ready state/RNG and explicit native neutral-tail/power policy. Keep the
   already approved >=180s producer neutral tail and all strict shutdown rows;
   no clipping or metadata edits. Strict origin audit admits each canonical.
4. Fresh-cache replay uses an own perf bundle, a truly cold own Metal namespace
   and empty private pipeline/Dawn cache, quiet/nice0, per-arm heavy->native. The
   full input/state/RNG/connection/power/shutdown replay must be exact AND the
   original gameplay stall threshold must pass: **zero gameplay frames>100ms**.
   Classify from the unchanged native frame phase, not a fitted frame-number
   window. Record every violating frame, pipeline wait/readiness where available,
   active config keys and cache/seed/admission identity. No retry-until-clean.
5. Maintain separate functional-exact and cold-performance results, both
   required. A tape that does not reach the recorded target gameplay interval is
   a failure, not a fast pass. Verify runtime coverage against the retained
   original case; source hashes may differ but are informational, not a shortcut
   past equality. No universal claim that one reproduction proves all play.
6. Proposed acceptance after canonical qualification: three alternating true-cold
   repeats per stage, VsKoopa / Island / VsKoopa / Island / VsKoopa / Island,
   fail-first. Keep the existing independent functional acceptance; no
   stall/menu/audio threshold or classification change.
   The 5.4.3 normal/full-star functional audio gate remains independently
   required. Keep old rare-stall FAIL receipts as regression evidence.

## Source-bound cases (read-only intake, 2026-10-26)

Both original spot arms launched directly with `--stage <name> --scenario 1`,
mods none, a fresh empty user directory (not the unlocked fixture), DIAG0,
TRACE_BOOT0 and nice0. The generic driver requested
`native/tools/perf_sweep_move.script`, steady60, quit-on-script-end. Its SHA256
is `29a21a31ba634ed586d6573d3a204b6ae4d021eeb7b7001107b34dbec6566678`,
unchanged from the retained driver worktree. It has 16 lines /970 script frames.

Primary retained source is frozen00221126's spot pair below, under M1
`port53/m1g/runs/`. These are source-bound historical failures, not tapes to
convert or fresh passes:

| Case | Receipt folder | Native CSV gameplay failure | Input outcome |
| --- | --- | --- | --- |
| VsKoopaLv1Galaxy | `solb-00221126-acceptance-r1-542-spot-10-VsKoopaLv1Galaxy-r1` | f731,257.916ms; pipeline_wait250.781ms | script completed970 frames; normal exit |
| IslandFleetGalaxy | `solb-00221126-acceptance-r1-542-spot-9-IslandFleetGalaxy-r1` | f765,111.386ms; draw_done56.911ms, uploads14.960ms, game_work39.411ms | script never started;240s driver timeout |

The two RESULT SHA256s, in that order, are
`165174aed882454640aa35ef92e00be4f75d1876ba1b018627c173be8a0e3a2a`
and `14c7b7b5e7e07d7766bb3291018d2391d4e2b1a061996755e9f378e0c6ee9788`.
Their frames.csv SHA256s are
`13fa1f4d659c32680c471ab99d13f79ffe7f5105f7f52b9424d663bf676c7176`
and `cd4eb9eb98f47b47b09c7395e128832957c5f579c3ac0c2da6b1e6a410ddaba7`.

VsKoopa's sidecar observed play-ready at route1288,1822 valid target rows and
2345 total seams. Island's observed play-ready seam was790, with13292 valid
target rows /13800 seams; that one seam did **not** make the steady60 script
start. Do not equate “script never started” with “no play-ready ever observed”.
Its retained route has zero move/button/camera-control/shoot rows and zero
pointer-valid rows (world-camera yaw is derived state, not a control).
The frame IDs include historical bootstrap frames; new coverage joins by the
schema2 scene marker/game packet, not absolute CSV row numbers.

Earlier52fc spot receipts are supplementary: VsKoopa f499394.313ms with
pipeline_wait387.394ms; Island f2496/f2519/f7731 at151.669/213.385/901.321ms.
The Island observations have different dominant lanes; its root cause is not
proved to be a missing shader. Lanes overlap and must not be summed. A native
tape fixes inputs/state and exercises the path; it cannot promise to replay
the original OS scheduling delay itself.

## Canonical production and the Island coverage distinction

Use a separate recording instrument derived from the selected integrated5.4.3
head, with the approved native script/tail producer capability from ref072.
Port only native recorder/script capability, default-inert; no game/GX/data
change, guessed ranks, seed changes or old schema1 conversion. A source delta
and exact build identity accompany the instrument; the final app remains the
replay consumer. Warm the owned cache before canonical recording and retain
any pipeline waits/pop-in as a qualification failure, not a reason to edit data.

For VsKoopa, run the unchanged970-frame movement script after steady60, then
release controls at the post-row tail seam, record >=180 wall seconds of the
strict native-neutral-controls-v1 tail and invoke the real power callback at
the existing control boundary. Admission requires all970 frames and the original
arrival/gameplay path, all schema2 markers/checkpoints and clean exact shutdown.

For Island, preserve the original first-visit **no-input** path. Do not add A,
skip talk, change to an unlocked save, or claim970 script frames completed.
Proposed recorder-only capability: observe14400 target controller-Normal game
packets with neutral input, independent of the script's steady60 start. This
covers beyond the historical13800 total-seam prefix and its13292 valid target
rows. Then mark the real tail seam, retain >=180 wall seconds neutral and request
power normally. This needs an explicit recorder-only bounded-observation trigger
if the existing producer cannot start its tail without script completion; show
that small native change to PORT before building. It must not alter game input,
play-ready tests or the replay path. The canonical must retain target scene,
observed play-ready/RNG seams and valid-player coverage across that prefix.

The recording receipt binds start/user/script/producer hashes, actual consumed
KPAD bytes+connections, schema2 input_phases, tail policy, native power event,
complete footer and raw SDK audit. Qualification includes a fully exact replay
with all strict shutdown rows; no partial canonical is admitted. No human play,
host/AX input, screenshot permission or TCC request is required by this design.

## Proposed gate and campaign contract

Add `rare_stall_tape_gate.py --case VsKoopaLv1Galaxy|IslandFleetGalaxy` using the
existing strict schema2 replay/audit and frame scorers. PLAN binds canonical
SHA256/admission, exact app+PREP+source, scene/coverage contract and private-cache
inventory. Legacy schema1 cannot satisfy this new gate. A different exe hash is
provenance; raw reads, connection transitions, feeding/comparison marker, every
compared state/RNG/power/shutdown row remain exact without tolerance.

Each arm gets a never-used B perf bundle/Metal namespace and empty private
recipe/Dawn root, isolated HOME, and a fresh empty user. No tests enumerate or
open normal `~/Library/Caches/AstralItalian*`, clear another arm's cache, or
borrow the recording instrument's warm cache. Use quiet/nice0 and per-arm
heavy->native; launch the finite fail-first cohort detached only after SOL A
releases the M1. Preserve every failed arm, including audio-only failures.

Require strict replay completion **and zero native gameplay frames>100ms** over
the whole tape, including its neutral tail; no fitted f731/f765 exclusion or
reclassification. Keep the existing menu/load scorer unchanged and report it
separately. Apply the existing tape observation/audio checks (>=90s, real audio,
>=50 reports,0 underrun frames) and the native tail/power/shutdown checks without
padding. Coverage must pass before timing can pass; zero/short gameplay is not
a fast run. Record every over-threshold row and raw lanes, marker-to-CSV mapping,
sidecar/seed/admission/key context and any diagnostics enabled. DIAG0 acceptance
does not infer exact fallback readiness from nearby log lines.

Produce separate functional-exact, cold-performance and real-audio results,
all required. SOL A owns generic collector integration; coordinate the case
schema before edits. Proposed negatives cover wrong stage/start/save/script,
missing phase/checkpoint, changed raw read/connection, truncated970-frame or
14400-packet coverage, warm-cache substitution, absent gameplay, a100.001ms
gameplay row, and a positive audio underrun. The bound must allow the full
Island observation plus180s tail; a timeout remains a failure, never clipping.

The first deliverable after PORT's review is the source/plan, not a recording.
Only after qualified canonicals do we schedule the alternating six true-cold
arms. Any remaining rare stall is reported before proposing the independent
VsKoopa seed addition; cache-compatibility survival alone is not its fix.
