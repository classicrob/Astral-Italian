# 5.4.3(f) diagnostic: bounded game-thread off-CPU history

Diagnostic branch off main7fdb1c38, **never land as-is**. Native only;
zero src/include/libs delta. Independent of SOL A's audio history: its own
flag, fixed storage, writer, output and shutdown. Audio histories may coexist;
there is no reference/link to their transport.

This implements the approved 0bed0465 design. It does not fix pacing or claim
a cause for r3 CSV37269. The retained evidence/design is documented at
`m1g/solb/game-offcpu543-design-r1/`. No M1 native compile/run is authorized;
PORT builds/tests on the server, then schedules measurements after SOL A.

## Opt-in and output

```
PETARI_GAME_WAIT_HISTORY=1
PETARI_GAME_WAIT_HISTORY_FILE=/absolute/private/run/game-wait.jsonl
PETARI_FRAME_CSV=/absolute/private/run/frames.csv
PETARI_FRAME_STATS=1
```

Only exact1 enables. FILE must be an explicit absolute NEW file, opened `wx`;
existing evidence is refused. No default path or user-cache lookup. Initialization
is on main after Aurora's creation and before game entry. Main resolves the
public NSWindow.windowNumber only when opted in. The writer queries metadata
for that ONE WindowServer ID with CGWindowListOptionIncludingWindow, requires
its PID to match, and retains on-screen/alpha/bounds. No screenshots, private
API, Automation, screen-recording permission request, or WindowServer scheduler
claim. An absent/unavailable window is recorded as unknown.

The diagnostic is cached OFF otherwise. The header-only bridge calls the
original operation directly on its false branch; no token, clock, allocation,
writer, sampler or callback runs. Event macros also erase argument evaluation.
The headless path does not initialize it. All original wait predicates, timeouts,
lock ownership, input reads, callback order and scheduling policy remain intact.
The observer does not set any thread priority/QoS/affinity. Its actual timing
cost still needs the approved same-binary OFF/ON overhead measurements.

## Trigger, rate and clocks

The existing RETRACE_DONE/END_FRAME callbacks supply the SAME steady-ns mark
used by frames.csv. CPU is sampled on the calling/main thread at both marks
with CLOCK_THREAD_CPUTIME_ID. Only wall>100ms AND cpu<10% of that wall triggers
a window (integer, overflow-safe inequality; no tolerance). Exactly100ms or
exactly10% does not trigger. EVERY matched phase over50ms emits one summary:
`near-miss` unless qualifying, otherwise `trigger`. Counts are checked against
footer slow/trigger totals, independently of whether windows can be admitted.
CPU-heavy slow phases are therefore still counted. Missing/backward/invalid
CPU/marks are explicitly invalid, never silently zero CPU. Initial unmatched
END_FRAME is local invalid-phase evidence, not a poison flag for unrelated
later windows.

A trigger captures2s before its END_FRAME decision through250ms after; overlapping
requests coalesce while preserving all trigger frame IDs. An active wait stack
(32 fixed entries) supplies a mailbox for stalled phases. Native CPU/OS state
point samples start only after the current game phase or main native span is
>20ms old, at the writer's actual10ms polling cadence. No target suspension or
stack sampling. Samples include API errors, run_state, suspend_count and total
CPU, with actual call start/return ns; late samples are never backfilled.
RUNNING means runnable/running, not proof of execution or a kernel wait cause.

Process TASK_EVENTS_INFO faults/pageins and deltas are sampled during long
phases at most every100ms, with API result and actual sample brackets. They are
process-wide context ONLY; they cannot establish a main-thread page fault.
The same writer makes the public owned-window metadata query and records its
own cost as a WindowState span. No watcher/writer takes a production mutex.

## Boundaries and identities

- Game marks, exact CSV interval open/close ns, normal successful aurora_update
  + handleEvents separately, DisplayWindow update, compose, end/open-frame.
- Global interrupt-mutex request/acquire wrapper; owner intervals while an
  explicit main interrupt request is pending, or a completed hold>=250us.
  `InterruptOwner value=0` is **release intent before unlock**, including a CV's
  impending implicit release. It is NOT a kernel unlock/wake timestamp.
- Main CPU CV wait, ready/sleep, baton grant/reassignment/idle, actual resumed
  callback, host-blocking begin/end. Grants are not labelled execution.
  Holder transitions are retained while main depends on a baton/wait.
- Explicit GX token-delivery, processed-FIFO and draw-done wrappers preserve
  their original bodies. Token target, GPU-delivery position/ticket, EFB capture
  request/worker and existing map-callback entry/ready use actual identifiers.
- FIFO spin/slow-lock/work/CPU-reacquisition outer span and inner mutex/work;
  frame/staging slot acquisition, render queue backpressure and sync-CV wait;
  snapshot read (includes mutex/body), EFB segment submit/sync.
- Actual GetCurrentTexture (Dawn Metal nextDrawable boundary), Submit and
  Present calls on the render worker. These are API-side spans, not proof of
  pure GPU time or of WindowServer's internal wait. Existing broader CSV
  pipeline/disc/capture counters remain available; no shader/load cause is guessed.

Every event has numeric kind plus stable name, steady start/end, unique ring
sequence, host pthread plus generation, frame where known, span ID, object,
result/detail, CPU and actual packet/ticket if known. Thread-bind roster entries
associate OS thread/Mach port/priority with its host generation. IDs unknown at
that boundary stay zero; no log-proximity join. Reassignment and return-to-CPU
remain separate from call completion. Nested spans MUST NOT be summed.

Typed details (not every field is a duration):

| Kind | Meaning |
| --- | --- |
| phase / summary | cpu_ns=matched phase CPU delta; detail=verdict; id/frame=CSV interval |
| csv | object=CSV interval; value=exact open_ns; detail=exact close_ns |
| run-state | actor thread=writer; object=main host; value=Mach result; detail=run_state; cpu_ns=target total CPU; aux0=suspend_count; aux1=current baton OS holder |
| vm | object=Mach result; value=fault counter; detail=pageins; aux0/aux1=process counter deltas (UINT64_MAX=unavailable/wrap) |
| window-state | object=owned window ID; value bits valid1/onscreen2/alpha-one4; aux0/aux1=width/height; unknown explicitly zero |
| thread-bind | generation is host lifetime; nonzero object=OS identity, value=Mach port, detail=priority; zero object is an automatic host registration |
| baton-grant | object=OS holder or0 idle; value=priority; detail=host pthread; grant is not resumed execution |
| capture | object/packet=actual frame packet, value/ticket=actual capture ticket, detail=request0/worker1 |

## Bounded transport and refusal

65,536 atomic history cells +128 fixed binding cells, preallocated snapshot
storage, 32 active slots; compile-time sizeof(State)<32MiB. This is a cap on
recorder-owned fixed storage (not a claim about framework/kernel internals).
Initialization touches the storage before game entry. No producer allocation,
formatting, I/O, waiting or retry loop. A busy/stale cell increments a drop
instead of blocking. Short native spans<250us are counted/totalled, not emitted;
coarse marks/dependencies remain. The filter is NOT a frame gate change.

The writer polls and emits only >50ms summaries and requested windows. It waits
at most four polling passes for a reserved-but-not-yet-published sequence, then
marks a real hole; producers never wait. Snapshot uses atomic payload/version
checks; overwritten/missing/unstable prefix, drop, stack overflow, invalid phase
inside the window, binding overflow or early shutdown refuses attribution.
Coverage is reported explicitly, not inferred from zero drops. Sparse sampling
and unknown wait channels stay unknown even in an otherwise complete window.

Hard caps: <=16 windows and <=64MiB output/run. Exceeding either produces a
refusal or incomplete/no footer, never an eligible window. Near misses are
single summaries, not a continuous event stream. Shutdown is idempotent on the
real Host leave/forceQuit and atexit paths; no crash footer is invented. Writer
copy/write brackets and byte totals expose observer I/O. All native history
stays in private diagnostic run outputs; never read/clear user caches.

`native/tools/game_wait_report.py FILE [--out NEW.json]` independently validates
schema/kinds/generations, bounds, window integrity and counts, exact trigger
inequality, summaries and trigger-to-window coverage. It returns ELIGIBLE_BOUNDARIES
or COMPLETE_NO_TRIGGER, with root_cause=UNASSIGNED; incomplete windows are refused.
Missing matching native wait + only low CPU is not a diagnosed blocker.

## Server validation and later measurements

```
tools/port/test_game_wait_history543.sh /absolute/GX-TEST-BUILD
```

Explicit targets: petari_game_wait_history_tests (fake-clock trigger/near boundaries,
invalid samples, ring wrap/concurrency, cap/refusal, actual OFF callbacks/metadata,
exceptions/exactly-once original operation); petari_game_wait_runtime_tests (private
new-file protocol, no-cache synthetic wait, idempotent close, existing-file refusal);
petari_game_wait_codegen_probe (optimized -O2 LLVM Release IR). CTest rows:
native_game_wait_history/runtime/codegen/patch/report.

The Release checker requires the cached flag to dominate all observer work,
finds a path calling ONLY the original operation before returning, and proves
metadata argument evaluation is after the flag branch. A source-only check is
not this native/codegen PASS. The script also tests final generated Aurora
anchors and protocol negatives (dropped/capped/unknown/duplicate/tail-missing
records, threshold/ratio/count mismatches). Old SDK fixtures remain as main;
this diagnostic does not carry the four freeze-input fixes or repair test rot.

Python/source-only checks were run on M1; native/Release build results are
PENDING PORT SERVER. M1 OFF/ON/title experiments require PORT scheduling after
SOL A's sink work, nice0/per-arm heavy->native/private cache controls. No
negligible-overhead/causal/pacing acceptance claim before that cohort.

Current main still uses the old source/build-digest renderer key: this native
instrumentation changes that diagnostic build's key. There is no shader recipe,
WGSL, descriptor ABI, seed pack or gameplay change. The accepted cache-survival
line is a separate freeze input and is not silently merged into this diagnostic.

### Frame generation correction

The first server configure refused a double-instrumented frame TU: readback
appended history hooks from `efb_frame.inc` before the final wait pass. The
fragment is restored unchanged from main; the final `kind=frame` pass exclusively
adds both packet/slot and EFB segment hooks. Its already-instrumented refusal is
unchanged. The patch test now runs readback -> geometry -> ubershader -> wait
from both minimal anchors and the actual pinned Aurora frame source, checks
exactly one history include / each expected hook and preserved original call
counts, and refuses a second pass. Python generation is source-only; a native
configure/compile remains a separate server check.

### Release IR matcher correction (Apple clang 21)

The server's exact `game-wait-codegen.ll` is retained byte-for-byte as
`native/tests/fixtures/game_wait_apple_clang21.ll` (SHA256
`da17510923da227fb12b1809b9b89073dd768f50f31c2624470708cad7d59fbf`).
This is Apple clang 21.0.0, not a locally regenerated approximation.

In `gw_probe_guarded`, entry loads the enabled byte into `%3`, truncates to
`%4`, then branches true to `%7` / false to `%5`. Disabled block `%5` calls
`gw_work` exactly once into `%6`, then branches to shared block `%26`.
That block selects `[ %6, %5 ]` into PHI `%27` and returns it. No hook load,
clock, callback, token initialization/lifetime or destructor is on this edge.
Entry contains a static uninitialized `%struct.Exit` stack-slot reservation;
the lifetime start, token writes and callbacks are only on the enabled branch.
This IR check does not claim a measured cycle/stack cost or runtime neutrality.

The old checker reset its call count across the unconditional branch, so it
rejected the shared PHI return. The corrected checker evaluates the flag with
`enabled=0`, follows ONLY that successor, accumulates the original call across
blocks and resolves each PHI using the predecessor actually traversed. It
requires the returned value to be that original result, rejects all other OFF
instructions/calls/loads/stores and cycles, and checks the metadata OFF edge
returns without evaluating any argument. Unsupported LLVM shapes fail closed.
It no longer accepts an arbitrary clean successor when the actual disabled
successor contains observer work. Ten mutations of the real IR reject added
calls/hook loads, wrong PHI/return, duplicate original calls, cycles, inverted
OFF/ON edges, metadata evaluation and entry work. Production code is unchanged.
