# GPU boundary trace: proposed design for 5.4.3 / 5.4.4

Status: design for PORT review; no implementation, build, run, or causal finding.
SOL A, 2026-10-25. Requirement: item (e) of the synced BRIEF-543.md.
SOL B retains the M1 for tape proof; all proposed builds/tests are server-side
through PORT. Measurement follows design review and the tape-proof handoff.

The completed warm-pacing bisect found the burst on uncontaminated 5.4.1 too.
It established neither a build breakpoint nor a fix from the uber-off pair.
This trace observes the remaining mechanism before any scheduling/render fix.

## Scope and clock

Add native diagnostic hooks only, including native patch scripts for the pinned
Aurora source. No root src/include/libs edits, dependency revision change,
gate-predicate change, replay-anchor change, or timing-policy change.

Enable only when BOTH `PETARI_PIPELINE_DIAG=1` and a new
`PETARI_GPU_TRACE=/absolute/output.jsonl` are configured. Read configuration
once before renderer/VI/game threads start. The disabled hook is one cached
atomic flag load and branch; all argument construction, clocks, IDs, lookups,
callback registration and buffering occur inside the enabled branch. No new
per-event getenv, function-local-static initialization, allocation, or I/O on
the disabled path. Existing telemetry continues unchanged.

All event timestamps use FrameTelemetry::nowNs(), the existing steady_clock
nanosecond clock. VI deadlines use the same steady_clock time-point conversion.
Record `clock=steady_clock_ns`, run ID, source/dependency/executable hashes and
trace schema version in the header. Do not mix raw CACurrentMediaTime,
MTLDrawable.presentedTime or GPU timestamp-query values into that timeline.

The first implementation uses Dawn OnSubmittedWorkDone, supported by the pinned
header, immediately after each real capture-segment and final-frame Submit on
the serial submission path. It adds no commands, submission, wait, polling or
forced synchronization. The callback only timestamps/enqueues a record.
Its event is named `gpu_queue_done_observed`: it certifies completion of the
submitted queue prefix, with callback status, at a CPU observation time.
**Submit-to-callback includes queue/GPU time AND callback dispatch latency.**
It is not an exact hardware GPU-end timestamp. A MapAsync callback can be
observed before the work-done callback; host callback order is not GPU order.

No native Metal command-buffer completion hook currently exists in our source;
Dawn is prebuilt and exposes no command buffer at these call sites. The existing
CAMetalLayer nextDrawable/presented-handler hook is presentation, not capture
completion. Do not add a global Metal swizzle. If the first trace leaves a long
submit-to-observation gap unresolved, a separately reviewed backend hook or
calibrated GPU timestamp investigation is needed before naming it GPU execution.

## Identity and frames.csv contract

Each event carries its originating frame identity, not the current frame read
later in an asynchronous callback. Use existing Aurora FramePacket.frameId and
frameIndex, capture ticket, FIFO position/token, draw-done issued/delivered
count and VI retrace count. Add trace-only submission/segment IDs, wait IDs and
thread lifetime IDs. Reused packet slots and OSThread addresses are not IDs.
Copy identity before the Aurora end-frame worker resets its packet. Retain
immutable identity in callbacks; bound its lifetime through completion/error.

`frames.csv` is an interval series. Add trace-only interval-open/close markers
at the exact timestamps already used by recordFrame, naming the actual CSV
frame.index. The initial seed timestamp creates no CSV row. Emit explicit
packet-open/packet-close-to-interval links, route-frame links where available,
startup phase and the existing prep_csv/game_csv_offset/route_offset mapping.
Do not assume Aurora frameIndex, LatencyProbe::frame-1, route row, and CSV
index are numerically identical. In particular, 5.4.2 splash rows precede game
rows, and a single delayed packet can span several CSV intervals.

For VI events without a unique render origin, emit the verified VI sequence ID
and determine the observed CSV interval by timestamp. Do not invent a render
frame for an independent retrace. Every final joined boundary record contains
`origin_csv_frame` (nullable only for independent/pre-CSV events) and
`observed_csv_frame`. A missing required origin or join invalidates attribution.
The raw record carries the stable identity; the offline join emits the
CSV-verified IDs rather than consulting mutable current-frame state.

## Existing seams and proposed events

Line references below are from frozen source
0022112615c9a39cbbc508f2eba0c5580d159c06, not the older shared-worktree line
numbers. Re-resolve them on PORT's selected integration commit.

| Boundary / proposed events | Existing site | Addition |
| --- | --- | --- |
| CSV interval open/close, seam enter/exit, game/retrace/end-frame marks | native/app/frame_seam.cpp:246, :777, :791, :925, :939 | Exact interval markers and packet/route joins, including splash and first seed |
| Capture request; recorder enqueue; staging wait begin/end | native/gx/efb_recording.inc:7; efb_frame.inc:25 | Ticket-to-packet and FIFO identity, request/enqueue and staging boundaries |
| Render worker dequeue; capture encode begin/end; encoder Finish begin/end | native/gx/efb_frame.inc:52; efb_snapshot.cpp:242 | Separate queue residence, resource/encode work and Finish time |
| Submit begin/return; queue completion observed | native/gx/efb_frame.inc:57; patch_aurora_present.py:344 | Submission ID, packet/segment membership, optional Dawn completion callback for each actual Submit |
| Map request; map callback enter; readback copy/lock work complete; ready callback enter/return | native/gx/efb_snapshot.cpp:277, :303 | Distinguish callback latency from mapped-data copy, snapshot locks and ready publication |
| Snapshot marked ready; GX event selected; interrupt-lock requested/acquired; delivery callback begin/end | native/platform/gx_sync/gx_sync.cpp:130, :366, :395, :468 | Ticket/FIFO identities and exact readiness-to-delivery intervals |
| GX wait begin/sleep/return, including draw-done and token barrier separately | native/platform/gx_sync/gx_sync.cpp:542, :566 | Wait kind, requested count/frontier, thread and event dependencies |
| Thread made ready; baton assigned; actual waitForCpu return; outer GX/VI wait return | native/platform/os/os_thread.cpp:475, :525, :2235, :2248 | Correlate wake request with readiness, scheduler delay and actual continuation |
| VI deadline; timer wake; interrupt-lock request/acquire; retrace handler enter/exit; waiter return | native/platform/vi/vi_host.cpp:136, :213, :443 | Same-clock VI count, due time, lock and delivery/wake boundaries |
| Drawable acquire begin/end; Surface.Present begin/end; existing presentation callback observation | native/gx/patch_aurora_present.py:330, :347; native/app/latency_report.cpp:56, :68 | Link present/acquire delays to packet/submission and CSV; preserve presentedTime as separately labelled raw clock data |

GX draw-done can bypass an unfinished token with the current default policy.
Therefore no unconditional capture-to-draw-done dependency is inferred. Trace
both draw-done wait and frame-boundary token wait, their actual completion
frontiers, and the snapshot event that really gates each waiter. Keep current
strict-token option, callback order, lost-draw-done accounting, abort behavior
and virtual-ticket behavior unchanged. Label synthetic/virtual completions.

OSWakeupThread marks readiness, not execution. GiveCpu can assign a baton that
is subsequently reassigned before execution; waitForCpu returns only when the
thread actually holds it. Capture that distinction. Already-running/self and
force-parked resume cases must be explicitly identified; never claim an
unobserved forced resume timestamp. The enclosing GX/VI wait return confirms
actual progress. Do not enable the intrusive PETARI_BATON_DIAG PC sampler.

Aurora frame hooks must include ordinary command encoding as well as snapshot
encoding, so a render-worker backlog before the snapshot is visible. Patch
begin_frame/enqueue_begin_frame, the ordered pass/task encoding dispatcher and
end_frame, preserving their existing closures and execution order when off.
Existing FramePacket.frameId is assigned at frame.cpp:635 and copied before the
packet reset in end_frame; snapshot tickets alone cannot identify presentation.

## Transport and error handling

Use fixed-size records in preallocated bounded buffers, with bounded,
nonblocking producer insertion suitable for spontaneous Dawn callbacks and
interrupt-lock contexts. No trace mutex, file formatting/I/O, allocation,
callback reentry into Dawn, or waiter notification from these hooks. One trace
writer created only when enabled emits one JSONL line per boundary. The writer
is diagnostic overhead and must be recorded as such in later measurements.

Each line includes event name, timestamp ns, producer sequence/thread ID,
identity fields and status; begin/end events pair by ID. Writer line order is
not asserted to be callback execution or timestamp order. Trace-only context
tables are preallocated and bounded; exhaustion, record drops, missing links,
callback failure/device loss, pending callbacks at termination and incomplete
CSV intervals are explicit. Any loss on a worst-row dependency invalidates its
attribution. No silent sampling or timing threshold used to omit events.

Callback state must outlive registered callbacks without retaining a frame slot
or changing resource reuse. At shutdown stop accepting new trace requests,
preserve pending counts and drain the writer without introducing a GPU wait;
late callbacks safely see run-owned lifetime state. An incomplete trace remains
incomplete. Do not change existing shutdown semantics to obtain a clean footer.

## Attribution of a worst row

1. Join its exact CSV interval boundaries, route mapping and origin packets;
   verify the interval duration against CSV rounding. Include crossing events
   from earlier frames rather than truncating them at the row boundary.
2. Follow the actual blocking wait's dependency frontier to its ticket/submit
   and thread. Separate request-to-enqueue/staging, worker queue, encoding,
   Finish, Submit call, queue-completion observation, map/readback CPU work,
   snapshot-ready-to-GX delivery, ready-to-baton and baton-to-resume.
3. In parallel follow VI due-to-wake, wake-to-interrupt-lock, handler work,
   wake/scheduling and actual game continuation. Include acquire/present spans
   when the frame seam blocks there.
4. Report the critical blocking chain and overlapping spans. Do not add
   capture latency, game-work, draw-done and present lane totals as independent
   work. A long submit-to-observed-completion span is labelled GPU/queue/
   completion-dispatch unresolved until evidence separates those causes.

The report names one boundary only when a complete dependency path supports
it. Otherwise report the smallest evidenced interval and the missing evidence.

## Proof required before measurement (PORT server builds)

* Disabled equivalence: run existing deterministic seam/capture/GX/VI fixtures
  against baseline and tracing compiled in but off. Compare ordered Submit,
  MapAsync, ready, GX delivery, VI and thread-transition transcripts plus exact
  state/RNG/readback outputs. A spy fails on any new clock call, allocation,
  file/thread creation, callback registration or trace argument evaluation
  while off; inspect release hook codegen for the one flag load/branch.
  Test DIAG-only, trace-path-only and neither; none may enable tracing.
* Trace-on tests use fake clocks/queues and controlled callback order: delayed
  completions across frames, slot/address reuse, callbacks before registration
  returns, map-before-workdone, multiple captures/submissions, failed callbacks,
  draw-done bypass, VI lock delay, baton reassignment, shutdown and overflow.
  Each must preserve the baseline callback/scheduling API transcript apart from
  the explicitly added trace observer, and refuse incomplete attribution.
* Mapping tests cover initial seed, splash rows, first game route row, ordinary
  frames and delayed callbacks. Require unique packet origins, exact event
  counts/identities and interval_ns equality to frames.csv within its printed
  rounding; reject missing/duplicate joins. Validate the existing present ID
  relation rather than baking LatencyProbe's offset into the new contract.
* Integration: one server trace-on run validates actual frame joins against
  frames.csv and render/route/checkpoint evidence. Trace-off baseline comparison
  preserves inputs, state/RNG and outputs; live wall timings are not claimed
  bit-identical. Existing functional predicates remain unchanged.

No tests have been executed for this proposal. Following review, implement in
small native-only commits, have PORT build/test, then coordinate diagnostic
M1 arms after SOL B's tape proof. Preserve original warm-pacing receipts and
label traced arms separately; trace overhead cannot be presented as baseline.

## Inspected dependency contract

Aurora revision 08122911e8621acb7ded6563813b264bec1494b5:
lib/gfx/frame_packet.hpp FramePacket, lib/gfx/frame.cpp begin_frame/end_frame.
Dawn prebuilt v20260807.225922 / revision
1155e0ed531126f33a1279afa029349651ca1c93:
include/dawn/webgpu_cpp.h:1902-1904 exposes
Queue::OnSubmittedWorkDone(CallbackMode, callback[, userdata]); use
AllowSpontaneous with a trace-only callback. These local sources/headers were
read, not rebuilt or modified. API availability is source evidence, not a
completed runtime proof.
