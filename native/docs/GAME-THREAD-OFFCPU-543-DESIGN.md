# 5.4.3(f): game-thread off-CPU stall — source review and proposed history

Status: DESIGN FOR PORT REVIEW. No implementation, native build, app run,
measurement, scheduling change or acceptance claim. SOL A retains the M1.
The pending freeze-head stall-tape plans remain unchanged.

## Finding and evidence boundary

The retained r3 frame identifies an unexplained game-thread **wall** interval;
it does not yet identify its blocker. In the FileSelect menu, CSV37269 is
564.534 ms: game_work500.426, retrace63.759, draw_done0.055. The stall line
reports game/render CPU2.9/3.1 ms. Pipeline wait/resolves, uploads and token
barrier are zero; drawable acquisition0.063, submit0.205, present call0.009,
VI late0.031 and VI lock wait0.003 ms. EFB capture maximum125.278 ms overlaps
these observations; it is not a separate additive cost or proof of a GPU stall.
The displayed present interval174.999 ms is also distinct from Present's call
wall time. These short measured calls do not explain500 ms of game-work wall.

The source-bound run is c1276e5b8e29b00efcae8bc7d13fc44c8007b69f,
original exe53fe7904 / authorized derivative55020acd. Audio history was ON;
full GPU trace was OFF. Existing GPU-trace hook sites therefore describe
available instrumentation, NOT a retained full boundary timeline for r3.

A's native audio ledger proves a364.762 ms absence between consecutive AQ
callbacks, then a full ring when callback delivery returns; the producer tick
remains about1 ms. Reconstructed frame end is1.884 ms after the arriving AQ
entry, with18.635 ms accumulated CSV rounding bound. This is a coincident
host episode, not an exact drop/frame assignment and not proof of a shared
cause. A's audio-guest assignment/resume history excludes neither main-thread
baton starvation nor another interrupt-lock holder.

Earlier e2 r2 CSV16972 (627.556 ms) supplies the complementary case: game_work
223.143, draw_done392.502; a269.657 ms VI lock request/acquire span,253.089 ms
VI lateness and261.075 ms snapshot-ready-to-GX-delivery delay overlap.
Grant-to-resume and post-draw-done host waits matter. The trace was structurally
audited, but overhead was NOT qualified; one pair does not prove neutrality.
The earlier aud-de3 trace emitted~101,730 boundaries/s,~12.58 GB in316 s,
64.6% interrupt-mutex boundaries, and has a producer sequence gap. That is a
transport/observer warning, not a valid causal reconstruction to reuse.

Local intake, with small-report hashes and exact source blob hashes:
`m1g/solb/game-offcpu543-design-r1/{EVIDENCE,SOURCE-MAP}.json`.
Raw reports/logs/CSVs are retained in their original directories; no raw trace
was loaded or copied. The r3 report is
`campaigns/sola-543-aq-c127-r1/observer-neutrality-r3/review-r1/REPORT.md`;
e2 is `campaigns/sola-543-traceh-e2-r2/REPORT.md`; aud is
`campaigns/sola-543-aud-de3-r1/review-stopped-r1/`.

## What the game-work lane actually covers

All source citations below refer to the exact c127 commit above; SOURCE-MAP
binds their blobs. Read with `git show c1276e5b:<path>` for stable line numbers.

`src/Game/System/GameSystem.cpp:235-262` is the controlling order:

1. After waitForRetrace, RETRACE_DONE is marked; the native token-delivery
   barrier follows immediately (:255-261).
2. The next frameLoop performs beginRender, draw, endRender, update, calcAnim
   and the screen-preserver capture (:235-241).
3. END_FRAME is marked before MainLoopFramework::endFrame/GXDrawDone
   (:243-247); the native presentation seam follows.
4. The seam releases the emulated OS CPU, composes/presents/pumps/opens the
   next packet, and reacquires the CPU; then the game waits for retrace again.

`native/app/frame_seam.cpp:277-288` subtracts marks to form the wall lanes.
The native token barrier thus belongs to game_work, not retrace or draw_done.
The current game/render CPU numbers (:166-211) are seam-to-seam Mach CPU
counter deltas, NOT CPU bounded by RETRACE_DONE/END_FRAME. They strongly
suggest little execution, but a new trigger must use matching phase endpoints.
The game thread is the main/SDL thread (app_main.cpp:1042), distinct from
Aurora's render worker. AppKit and Metal waits on the latter cannot be assigned
to the game thread without a real queue/handoff dependency.

## Blocking-point inventory and gaps

This is a map of native boundaries and the game paths reaching them, not a
claim that an arbitrary library instruction can never block.

| Path / possible block | Existing timing at c127 | Proposed missing evidence |
| --- | --- | --- |
| Retrace / VI message queue: MainLoopFramework.cpp:472-507 uses OSReceiveMessage(JUTVideo queue), or OSSleepTicks when tick-paced | CSV retrace/wake; VI timer/lock maxima; trace sleep/wake/ready/grant/resume sites | Tag main's actual queue and reason; capture message-ready -> grants/reassignments -> actual resume. Do not substitute VIWaitForRetrace or treat first grant as execution. |
| Any SDK OS sleep, message, mutex, suspend, yield or preemption point | os_thread.cpp:483-680,2474-2505 has baton/sleep/wake/resume trace; hostWait in os_internal.hpp:61-74 | Main wait reason/object and ready epoch; transitions of the real holder while main waits. SDK queues/mutex-owner data may be read ONLY where the existing interrupt lock already protects them. |
| Acquiring the global interrupt mutex, including after a condition-variable wake | os_interrupts.cpp:20-44 traces request/acquire/release; interrupt_trace.hpp instruments original CV predicate calls; last release timing exists | Compact main request/acquire plus owner intervals during pending main wait. A CV predicate-return is an observed reacquisition, NOT a kernel wake timestamp. Distinguish another holder from a runnable main that has not reacquired. |
| Explicit native host work and return to emulated CPU | os_thread.cpp:2530-2628 traces host-blocking begin/end; end waits for CPU | Name the enclosing call, separate its returned result from readiness and baton reacquisition. Current generic begin/end does not name the callee. |
| GX FIFO buffer mutex, short try/yield loop, slow host-blocking path | patch_aurora_sync.py:99-119; full trace patch names FIFO processing/buffer waits | Main-side lock attempt, spin end, slow wait, actual work, unlock and later CPU reacquisition. Pair with processed watermark; no new spin/retry behavior. |
| Waiting for FIFO processed frontier or GPU draw-done | platform/gx_sync/gx_sync.cpp:565-592,646-673; sync_bridge.cpp:94-105; full trace draw-done/frontier events | Exact main wait target/queue, processed/report/interrupt-delivered frontier, main resumed. Generic OS sleep alone cannot name FIFO progress. |
| Token delivery after RETRACE_DONE | gx_sync.cpp:595-643 times TokenBarrierWait, traces target and return | Inner host condition wait, callback-ready/GX delivery, and separate end-host-blocking baton reacquisition. The aggregate can overlap other work. |
| Recorder staging/frame-slot/render queue backpressure or sync handoff | efb_frame.inc:26-96 records staging/enqueue/worker; patch_aurora_gpu_trace.py:21-68 names slot/staging/queue/sync waits | Reuse compact packet/ticket/span IDs at actual waits, including mutex acquisition before CV wait. Distinguish game-side FIFO wait from recorder/render-worker waits. |
| EFB capture/readback/snapshot mutex | efb_recording.inc:8-38; efb_snapshot.cpp:245-320 has request/encode/map/CPU-ready; captures are asynchronous | Compact ticket chain plus snapshotMutex wait/hold around game-facing readSnapshot, without serializing asynchronous capture. Encode/API return -> callback-entry is completion-plus-dispatch, never pure GPU duration. |
| Specialised/fallback pipeline and preparation mutex/CV | pipeline_runtime.inc:266-278,317+; existing CSV PipelineWait and compile IDs | Name actual pipeline key, mutex vs ready-CV wait, dispatch/ready times for only the waited key. r3 has zero such wait; do not guess a shader cause. |
| Native disc/loader completion and SDK mutex queues | dvd_host.cpp:278 waitUntilIdle,310-331 read timing/pread; scheduler sleep/ready | Label DVD command/loader queue and completing thread. DiscRead measures host I/O, not every later guest handoff. No disc read is reported for the target frame. |
| Main-thread HBM/ImGui compose, DisplayWindow update, SDL/AppKit event pump and begin-frame retry | frame_seam.cpp:730-739,957-1117; CSV seam composition/end/begin/reacquire; only failed-open event pumps are individually traced | Bracket NORMAL successful-frame aurora_update + handleEvents separately, plus DisplayWindow/compose/end/open; normal AppKit work must not disappear inside a broad begin-frame bucket. For r3 those seam lanes are short; they cannot simply be relabelled as the500ms game-work cause. |
| Render worker encoder/Submit/GetCurrentTexture/Present and in-flight limit | patch_aurora_gpu_trace.py:81-97; patch_aurora_present.py:322-355; CSV acquire/submit/present; present stats | Preserve worker IDs and packet mapping; brackets around actual API and any preceding worker mutex/in-flight wait. nextDrawable/WindowServer remains an API-boundary attribution until deeper evidence exists. |
| Allocation/page faults, libc/driver locks, stderr/file I/O, or host descheduling in an otherwise uninstrumented call | CPU counters; optional spike_profiler.cpp:106-139 suspends to sample stacks; baton diagnostic may sample holders | No continuous main run-state/fault cause in r3. Add NON-suspending main run-state/CPU samples during long phases. Task-wide fault/page-in deltas are contextual only, never proof of a fault on this thread. Keep unknown residual explicit. |

## Proposed implementation for review

Native-only, default OFF: `PETARI_GAME_WAIT_HISTORY=1`. It is independent of
PETARI_GPU_TRACE and audio-history enablement. Ordinary play pays one cached
flag branch at each selected native boundary; no clock reads, sampler, ring,
allocation, callback ordering, lock, timeout, priority or input change when off.
No new hooks in src/include/libs, no game frame skipping and no renderer/cache
identity or seed-pack input. Native Aurora patch scripts add boundary calls;
they do not rewrite Aurora scheduling, queue capacity or present behavior.

### Trigger and exact frame/phase identities

At the existing RETRACE_DONE and END_FRAME native callbacks, take steady-clock
ns and current-thread CPU ns at matching endpoints. Trigger ONLY on
`wall_ns > 100000000 && cpu_valid && cpu_delta_ns * 10 < wall_ns`.
Use overflow-safe arithmetic. Exactly100ms or exactly10% is not this trigger.
A missing/ambiguous mark, invalid CPU read or negative delta is UNCLASSIFIABLE,
never zero CPU or an invented match. Preserve the ordinary frame gates/logs.
Keep the broader seam/frame phase spans too, so main event pumping/present
outside game_work can be examined without changing the requested trigger.

Every record carries a versioned kind, steady ns (same clock as frame CSV),
producer sequence, thread identity/generation, actual CSV interval/route seam
where known, packet/capture/submission IDs where available, span and object ID,
reason/result and holder/ready epoch. Unknown IDs stay explicit zero/unknown.
Publish the exact CSV open/close ns rather than reconstructing by rounded sums.
A typed phase stack allows nested waits without flattening them into a sum.

### Cheap boundary history, not the prior firehose

Use the audio history's tested atomic bounded-publication/retired-cell retention
protocol, with separate game, scheduler and worker lanes. Initial hard budget:
65,536 total event cells, <=32 MiB for current/retired cells + preallocated dump
buffers, <=16 trigger dumps and <=64 MiB output/run. Size assertions enforce
these caps. Allocate and prefault on explicit diagnostic startup, not at a
stall. Enum/integer payloads only on producers; no strings, symbols, JSON, I/O,
heap allocation, sleeping or retry loops. Bounded failed publication increments
a counter, never blocks. Off mode does not allocate or initialize the history.

Do NOT log every interrupt disable/enable or every successful uncontended
mutex operation. Keep cheap active-span mailboxes for selected main boundaries;
retain completed wait spans >=250us, all coarse frame/phase endpoints and all
main ready/grant/reassign/resume transitions. While a main interrupt/baton wait
is pending, retain relevant holder epochs/transitions and completed owner hold
spans, including other threads. Snapshot owner identity at request/acquire;
never dereference an arbitrary remote OSThread from the writer. Short omitted
waits have per-kind count/total/max diagnostics, explicitly marked filtered;
the250us is a transport filter, not a changed frame/stall gate.

A trigger freezes the prehistory sequence frontier and requests2s before +
250ms after. A low-priority writer drains only requested snapshots. Headers
record actual coverage and configured caps; overlapping triggers coalesce and
retain each frame/trigger ID. No unconditional stream, no JSON-per-frame and no
writer I/O on main/interrupt/audio/render callbacks. Bounded storage cannot
promise2s at an arbitrary event rate: clipping, overwritten history, sequence
holes, unstable cells, rejected payloads, exhausted producers or output cap
make that window INCOMPLETE and refuse causal attribution. Never silently
sample away a missing dependency. Writer wake/serialize/write intervals and
bytes are retained so its own pressure is visible. Complete snapshots survive
orderly exit; a crash does not fabricate a complete footer.

### Run-state companion (also default off)

Boundary events identify our waits; they cannot by themselves prove that a
thread with no events was blocked versus runnable-unscheduled. Bind the main
Mach port, pthread/OS identity and generation once at startup. A low-priority
watcher checks the atomic current phase every10ms; only when a phase has gone
>20ms without progress does it read non-suspending THREAD_BASIC_INFO and the
main CPU counter, at most once per10ms. Store actual sample begin/end ns,
run_state, suspend_count, CPU deltas and API errors. Also sample known current
holder/render identities only for the pending dependency; no task-wide thread
enumeration per poll, PC/stack suspension or scheduling intervention.

Each point sample is a point sample. RUNNING means runnable/running, not proof
of CPU execution. A WAITING sample does not identify the kernel wait channel.
Sampler lateness is recorded, not backfilled. At trigger boundaries collect
bounded task-level fault/page-in/I/O context where supported and label it
process-wide; it cannot alone identify VM or WindowServer as root cause.
If the critical span has no named wait and sparse/unknown run state, report
UNRESOLVED host wait/descheduling. A heavier stack/kernel trace would be a
separate scope decision, not an assertion made from CPU scarcity.

### Lock order and attribution rules

Recorder publication uses only lock-free words; it never takes the interrupt,
WPAD, GX, render, SDL, allocator or writer mutex. Hooks observe state already
protected by an existing lock and copy immutable IDs/epochs; they acquire no
second production lock. Watcher/writer consume atomic snapshots and never
request production locks. Do not replace a CV wait/predicate with polling or a
new wait. CV implicit release/reacquisition is labelled as such, preserving
unknown transition intervals; the observer never invents a kernel wake time.

Reports present the actual nested critical chain: request -> acquired/ready ->
grant(s) -> resume -> first subsequent boundary. A grant later revoked is not
execution. Link a worker/driver dependency only with a matching packet/ticket/
watermark, never log proximity. Long return-to-callback spans include dispatch
latency. Do not add overlapping lane/capture/lock/CPU spans. Neither a common
AQ gap nor a core's E/P label is sufficient causal proof.

## Tests and measurement plan (NOT authorized to run yet)

- Fake-clock boundary tests: >100ms/<10%, exact boundaries, CPU-heavy work,
  invalid/missing clocks and out-of-order/duplicate marks. Assert exact CSV and
  raw phase linkage; existing CSV arithmetic/gates remain unchanged.
- Controlled wait fixtures: mutex holder, original CV wait/reacquisition,
  host-work then CPU reacquisition, ready/reassigned/resumed, token/FIFO and
  render queue dependency, normal event-pump span. Early/late worker completion
  must preserve IDs and distinguish callback dispatch from API return.
- Ring adversaries: wrap while frozen, concurrent writers, stale/busy cell,
  trigger coalescing/caps, missing footer, incomplete/overwritten prehistory,
  unknown state and late watcher; reports fail attribution instead of filling
  gaps. Assert storage/output bounds and no producer allocation or I/O.
- Off-path proof: compiled branch/callback audit and identical observable
  input/scheduler/callback results with history absent vs OFF. No production
  predicate or priority changes. Existing tape and affected native tests pass.
- After approval only: a separate instrument build, production/probe binding
  and alternating same-binary OFF/ON warm title pairs on the freed M1, nice0,
  per-arm heavy->native/private caches and the existing fixed windows. Both
  pacing/audio tails, watcher/writer cost and evidence completeness are reported.
  Prior e2/aud observers were NOT overhead-qualified; no neutrality claim from
  central medians or one clean arm. No freeze/tape run is replaced by this work.

Requested review: approve the hook/filter set, matched-phase trigger, bounded
ring and non-suspending companion before source implementation. No scheduler
or pacing fix is proposed until this evidence names the stalled boundary.
