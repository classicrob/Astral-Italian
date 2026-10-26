# Recorder trigger review — source only

**Expected regression baseline: FAIL.** VsKoopa's retained257.916ms stall is the
missing first-use pipeline covered by the three deferred item(c) keys. Until
those keys are seeded, this new cold gate is expected to fail there. Island's
111.386ms stall is draw_done/uploads/game work and may fail for a different
reason. Record either failure as-is; no classification changes or retries.

No native build, syntax compile, app run or recording accompanies this diff.
The M1 remains SOL A's audio slot. The C++ tests below are written/registered,
not claimed to pass before review and build.

## Draft ancestry and review boundaries

`port/543-rare-stall-recorder` is a source-only staging integration of accepted
cache handoff `a7df3c38` and closed tape-contract line `fbff6485`, merge
`c02b35f1` (one documentation conflict, both sections retained). This is **not**
the selected integrated freeze head. Before any build/recording, the recorder
capability must be applied to the actual selected integrated freeze source,
including the final audio/main integration, with its full SHA bound.

Prerequisite `3b609bf6` ports the already-tested ref072 producer capability from
`0e0db298`: native script/pointer controls, tail state machine and post-row
tail/power hook. It does not alter any game/GX/data or replay code. The separate
trigger diff after that commit is the new work for review. Neither recorder
capability nor this draft is a new release candidate or an admitted canonical.

## Island mode

Set `PETARI_TAPE_RECORD_ISLAND_NORMAL_PACKETS=14400`,
`PETARI_TAPE_RECORD=<new-output>` and
`PETARI_INPUT_SCRIPT_RECORD_TAIL_SECONDS=180`, with no `PETARI_INPUT_SCRIPT` or
replay env. Actual launch is `--stage IslandFleetGalaxy --scenario 1`, fresh
empty user, no mods, no host input. The eventual harness retains
`PETARI_SMOKE_IGNORE_PHYSICAL=1`, DIAG0/TRACE_BOOT0, private caches/namespace,
warm recorder cache, source hashes and strict SDK audit as in the design.
The observation override accepts only this literal14400 case, not arbitrary
counts, a changed scene clock or a script readiness override.

The pre-read observer already used by schema2 records the existing Normal marker.
Its new recorder-only cached flag is false for ordinary play and any replay;
the ordinary non-tape path still installs no observer. From Island's first
`controller-normal` marker, count exactly14400 consecutive input-frame packets
in Game/IslandFleet/scenario1, without consulting play-ready/steady60 or reading
any new player pointer. Require at least13292 valid target packets, matching
the retained case's minimum coverage. Missing first-Normal, duplicate/skipped
packet, Normal restart/phase/stage/scenario change, observed death, or bound
controls/pointer becoming non-neutral records a producer failure. The actual
raw SDK samples are still recorded unchanged and independently audited.
The bound-controls snapshot takes the existing host-input mutex in the same
Interrupts -> host-input order already used by live GameRead's `tickChannel`.
It does not send SDK/input events while that mutex is held, add a feeder wait,
or acquire a game/scene lock. All control delivery stays outside the pre-read
hook at the established control seam.

At packet14400 the pre-read hook only sets a pending flag. It never sends input,
starts the tail, presses power, or advances readiness. At the existing post-row
control seam it releases controls/pointer, queues the tested Tail state machine,
and calls the existing `TapeRecord::neutralTailBegin`. Its clock begins there,
not at startup or at prefix packet1. After >=180 real seconds, the existing
control-seam code calls the real `Host::requestQuit`; producer and replay keep
the same power boundary. All shutdown rows remain strict. There is no input
sample normalization or metadata substitution.

The recording logs one begin and one complete with first/last input-frame and
packet/valid counts, plus tail queue/begin and real power evidence. The future
case harness must independently join those counts to sidecar `input_phases`
and the actual read map; it cannot admit on a log count alone. Existing
`native_tape_record543.py` has world-map-specific assumptions: this diff does
not pretend that it already admits these two new cases. Case-specific harness
and frozen plans follow trigger review, before any run.

## VsKoopa and off-path behavior

VsKoopa keeps `native/tools/perf_sweep_move.script` byte-for-byte (970 frames),
steady60 and the tested script-end -> neutral-tail -> real-power producer path.
It does not set the Island observation env. No consumer/feeding/comparison,
KPAD dispatch, seed, shader, queue scheduler, game source or readiness predicate
is changed. When new recorder flags are absent, the new state and hooks do not
touch game input or game state. Wrong recorder configuration fails rather than
silently selecting an easier path.

## Tests supplied for review

The new game-free `native_tape_normal_observation` C++ target checks the exact
14400th boundary, minimum valid coverage, frontend exclusion, missing first
Normal, duplicate/gap/restart, stage/overlay/scenario change, death and dirty
input, sticky rejection, completion emitted once, control-only tail begin,
and no power before180s or outside the control seam. It is registered and added
explicitly to `test_tape_anchors543.sh`, alongside the six existing tape tests.
No native test has been compiled or executed in this source-only handoff.

Python source guards check the default/replay-off hook, existing pre-read phase
ordering, no game reads/input/power in the count hook, the unchanged post-row
control scope, fixed count/config and native policy. They supplement the future
native test; they are not an app or replay acceptance claim.

Because VsKoopa is expected to fail, a single fail-first acceptance queue can
stop before Island. Do not claim Island coverage from an unrun arm. Any
separately scheduled one-shot Island baseline must have its own frozen plan;
it is not a retry of failed VsKoopa or continuation relabelled as acceptance.
Resolve those measurement plans with PORT after source review and slot release.
