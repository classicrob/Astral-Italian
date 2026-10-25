# Baton, QoS and wait attribution extension (source only)

Base instrument: ccd0a360, based on candidate-line 7fcc610c. This extension is
an instrument, not a candidate or scheduler fix. No scheduler constants,
priority policy, backoff, wait primitive, GPU commands, input or replay policy
change. PORT keeps Dawn pinned; no backend patch/build or Metal swizzle.

## What the previous data establishes

The on-second title/menu trace has three long wake cases: 7394 (48.789 ms),
12993 (32.947 ms), 13418 (40.782 ms). Frame 15306 instead has a 33.545 ms
baton absence during game work before capture; VI-ready to resume is 1.668 ms
and draw-done-ready to resume is 4.654 ms. Frame 10421 includes a 22.291 ms
pre-encode dispatch gap. Frame 14039 remains queue/GPU vs dispatch inseparable
at the pinned backend. These are observed boundary delays, not proof of which
thread or kernel policy caused them.

The ccd0a360/derived-04c14020 overhead verdict belongs only to that retained
cohort. This extension adds host queries and events and MUST get its own
same-binary off/on comparison before using its timing causally. Existing
753 MB trace and all failures stay on the M1 unchanged; no copies or reruns.

## Schema 2 additions

Every `baton_grant` reports old `holder`, selected `waiter`/subject, `held_ns`
and reason. `detail` says whether the outgoing assignment was running;
`value` is the receiver's emulated priority. Reasons distinguish initial/idle
assignment, reassignment before resume, yield, priority preemption, OS wait,
suspension, exit, explicit host blocking and forced preemption. Idle transitions
emit `baton_release`. Unsafe/system/unresolved-code refusals emit
`baton_refusal` without changing holder state. Other attempted-preemption
results (including try-lock busy/not-needed) remain separate result events;
they are not fabricated releases. A self-grant is still an assignment boundary.

`held_ns` is wall time since the preceding grant decision, **not CPU time**.
The existing `thread_baton_assigned` still marks the actual gCurrent store.
Trace queries in the critical section may perturb trace-on handoff latency;
the renewed overhead comparison is mandatory. No new locks, affinity, QoS
changes, polling threads, waits or spin-waits are added by these hooks.

Thread births retain lifetime IDs and now include emulated base priority and
entry-function address (offline dSYM symbolization). OSThread and pthread IDs
are distinct. Worker item types/pass indices, FIFO processing, GX delivery and
map/queue callback events identify their respective host roles. Force-resumed
holders cannot execute an inline trace hook at an arbitrary saved instruction:
`forced_holder_resume_requested` is a request, and
`holder_first_os_observation` is the first later OS entry, **not the exact
resume instruction**. Ordinary `waitForCpu` continues to mark actual resume.

Scheduling observations carry observer/target pthread, requested QoS via
pthread_get_qos_class_np, observer qos_class_self, relative QoS priority,
pthread scheduling policy/priority, CPU number and query return codes. CPU
number is sampled ONLY for the calling thread via pthread_cpu_number_np
(macOS 11+); a remote recipient's CPU is -1 until a local resume/worker/callback
observation. Missing pthread registration is explicitly unsampled, not replaced
with the waker's CPU. CPU samples are instantaneous and can be stale immediately.
Requested QoS does not reveal effective override QoS. Existing holder override
start/detach/end boundaries therefore carry the override token separately.

At configure, the header records hw.ncpu, hw.perflevel0/1.logicalcpu and names,
plus a per-CPU map from readable IODeviceTree:/cpus child properties
`logical-cpu-id` + exact `cluster-type` E/P. Counts never imply numbering.
Missing/unrecognized/conflicting properties leave that CPU unknown. Registry
availability/property counts, decoding source, capacity and conflicts are
recorded; nodes outside the reported/capacity range are not guessed. Mapping
uses explicit CFNumber or four-byte Darwin native-endian CFData IDs, and exact
single-letter CFString/CFData types. No reg/node-name-derived IDs or M1 ordering.
These IOKit/sysctl reads happen once, only after DIAG/path/binding validation.
The framework includes and query live in native/src/gpu_trace_host.cpp; the
shared petari_gpu_trace_host static target is linked by the platform and trace
tests. Producer headers expose no CoreFoundation/IOKit types.

## Waits and deadlines

Wait records use the SAME steady-clock nanoseconds as frames.csv markers.
`host_wait_begin/return` retain primitive, requested deadline (zero = untimed),
elapsed wait and nonnegative lateness past a timed deadline. A bounded TLS
stack handles nested semantic Wii waits and their underlying host waits;
overflow/unbalanced returns count as drops and invalidate the window.

| Site | Existing primitive / observation | Deadline meaning |
| --- | --- | --- |
| OS hostWait / waitForCpu | untimed std condition-variable predicate, interrupt mutex released | none; return includes predicate + mutex reacquire |
| OSSleepThread | semantic Wii OS queue, nested hostWait | none; not itself a host timer |
| Baton monitor | Mach semaphore_timedwait; std sleep_for fallback | timeout converted to a requested steady deadline before the unchanged call |
| OS alarm timer | untimed condvar or condvar wait_until | existing systemTimeToHost absolute deadline |
| VI normal timer | mach_wait_until | existing steady deadline used to compute Mach ticks |
| VI legacy timer | condvar wait_until | existing absolute next retrace deadline |
| GX delivery/progress/draw-done host waits | untimed predicate condition variables | none |
| GX token barrier host wait | unchanged one-second wait_for and original bounded loop | each iteration's relative timeout; no changed predicate or retry count |
| GX drain | existing one-ms std sleep_for | relative requested timeout |
| FIFO worker | std atomic wait on existing wake word | untimed; library/kernel implementation remains opaque |
| FIFO buffer-lock fallback | existing std mutex lock after original spin/yield policy | untimed; existing spin policy untouched |
| Render queue | not-full untimed condvar; not-empty wait_for | existing queue timeout; no modified predicate |
| Pipeline compile workers | existing untimed queue predicate; dispatch samples after existing QoS choice | none |
| Render synchronize / frame-slot pool | untimed condvar predicates | none |
| Splash wait | existing SDL_DelayNS | existing relative requested duration; SDL internals opaque |
| Forced-preempt suspend/state operations | begin/end/failure boundaries around existing Mach calls | untimed calls, not a new timeout |

Predicate waits can internally wake and re-wait. The trace brackets the existing
predicate call without changing its predicate-evaluation count. Timed lateness
includes kernel wake/dispatch, mutex reacquisition and predicate evaluation;
it does NOT prove timer coalescing alone. Relative deadlines are captured just
before the unchanged call and include its setup gap. SDL/Dawn/Metal internal
primitives remain opaque; no invented kernel timeout or dispatch timestamp.
Background waits still open at trace shutdown are explicitly right-censored.

## Monitor target: native_platform_os priority-handoff assertion

Read at 5e9069c8: 40 priority-16 computing-thread -> priority-2 audio handoffs,
no OS calls in the computing loop. An independent std thread timestamps before
OSSendMessage(NOBLOCK); audio timestamps after OSReceiveMessage returns. The
sender's 3 ms sleeps / 1 ms completion polls are outside that measured interval.
Audio hostEntry requests USER_INTERACTIVE (base <=8); monitor also requests it,
and the computing holder gets an existing temporary QoS override while waiting.
No affinity is set. Its <=1.5 ms median/p90 predicates stay unchanged.

The monitor retains its 0.5 ms target, 2 ms idle poll and 0.25 ms doubling retry
backoff capped at 2 ms. Each trace-on iteration gets a span connecting:
`monitor_wait_plan` -> host wait requested deadline -> `monitor_woke` (actual
wait-return timestamp) -> `monitor_preemption_target` (original pending-since
plus target) -> waiter identity/remote requested QoS -> suspend/state checks ->
result -> retry/backoff or not-due/poll-throttle decision. The next iteration
records the actual next planned wait. A successful semaphore wake is explicitly
marked; an unmarked return is non-success, not assumed to be a timeout rather
than an abort/error. Receiving-thread CPU comes from its own resume sample.

M1 main controls at build-reports/power-main790-controls-r1 reportedly fail
3/3 around 2.50 ms. That is a lead for attribution, not a measured fix here.
First fix target, per head: quiet-M1 <=1.5 ms with no M4 regression, fix at the
source and no spin-waits. This source-only instrument does not change timers
or claim that target achieved.

## Startup and completion attribution

Startup now marks collection, AppKit/SDL pump, Aurora event update, frontend
status, open-first-frame and packet-open phases. Packet opening brackets frame
slot and staging acquisition. Together with worker_end, the original packet
ID and CSV endpoints, this splits the previous worker-end -> packet-open gap
without assigning one numeric counter to every frame domain.

Queue callback entry/body/host-sample/body-end and map callback entry/body/end
are on the same monotonic clock. The entry timestamps are taken before host
sampling; host.sampled_ns exposes the later query completion. They separate
our callback body and sampling overhead, **not** backend completion from
callback dispatch. Header provenance says pinned Dawn; the 14039 ambiguity
remains explicit. No waiting query, GPU timestamp or extra submission is added.

## Source validation and deferred test plan

Run only inspected pure Python locally:

```
python3 -B native/tests/gpu_trace_patch_tests.py /path/to/pinned/aurora
python3 -B native/tests/gpu_trace_analysis_tests.py
python3 -B native/tests/gpu_trace_attribution_tests.py
```

The tests erase gated hooks and prove unchanged operation token streams for
14 native call paths and all four final Aurora patch kinds, reject malformed
patch anchors, keep original GPU/work calls, and exercise exact CSV mapping,
unknown remote CPUs, reassignment/refusal durations, nested/censored waits,
monitor lateness/backoff separation and fail-closed accounting. Native tests
are SOURCE only until the slot/priority ruling; no local compiler or app run.

Once PORT/head schedule a build, run native_gpu_trace (expanded fake-clock
holder/deadline/topology/disabled-argument/MPSC/writer tests),
native_gpu_trace_patch, native_gpu_trace_analysis and native_gpu_trace_attribution;
then unchanged native_platform_os, native_platform_vi, native_platform_gx_sync,
seam/pipeline/GX fixtures and tape regressions on the configured diagnostic
build. Inspect Release petari_gpu_trace_off_probe: one cached flag test before
any pointer/query access. Record compilation separately from execution.

When B releases the M1 AND head sets priority: quiet check, same pinned
instrumented executable for off/on, nice 0, per-arm heavy -> native, exact prime
cache/Metal handoff, fixed300s. Preserve each arm and compare all distributions,
>50 count/max/post100 p99, drops/footer and every CSV interval. No overhead
qualification from ccd carries over. A materially shifted distribution rejects
this instrument window before causal use. Priority-handoff runs retain <=1.5
ms predicates and M4 controls; no favorable re-run selection or predicate edits.
Read scheduling_attribution in the existing report for whole-stream validation,
then per-worst-row holder, primitive-wait, host/phase and monitor details; never
sum overlapping lanes. No measurement or acceptance is claimed by this branch.

## Build integration correction

The server d70ca297 build failed before tests: including CoreFoundation/IOKit in
all producers exposed MacTypes ::Rect in frame_seam.cpp, and including the
trace header from pipeline_worker.inc inserted its namespace inside Aurora's.
The correction isolates framework includes in gpu_trace_host.cpp, moves the
trace include to the pipeline generator's file-scope prelude and uses
::PetariNative::FrameTelemetry explicitly. New source checks reject either
header leakage or a nested trace include. No App::Rect qualification workaround
or scheduler behavior change is needed.

Local authorized clang -fsyntax-only checks passed for 11 production/test TUs,
including the regenerated cache, app frame seam/main, private query and trace
unit test, under heavy -> native. The local SDK is 15.1; SDK26 app build/link
and native test execution remain pending PORT's server rerun. The retained
syntax plan, exact commands/input hashes and logs are in
m1g/campaigns/sola-543-gpu-trace-buildfix-r1/. These are syntax results, not an
app build or executable test pass.
