# Interrupt mutex / assigned-recipient trace (SOL A)

Source-only diagnostic atop port/543-monitor-deadline-fix6878a2bb
(app09703560 + test-only71350946/6878a2bb). PORT authorized mutex/notify/Mach
attribution after the e2 r2 overhead review. Bounds + run-state samples are
sufficient for this round; no alternate CV, interposition, timeout, spin,
policy promotion or scheduling fix is introduced. Root src/include/libs,
input/tape/Dawn and the original<=1.5ms OS predicate are unchanged.

## Observed problem, not a fix claim

r2 CSV16972:192.077ms host cond-predicate wait nested in guest OSSleepThread;
same queue wakes~1.07ms later, but after a grant the target takes~191ms to
return. Same interruptMutex is acquired by VI repeatedly during that gap.
Later a VI request takes269.657ms. Source excludes the proposed mismatched
mutex/CV lost-wakeup mechanism on that path, but original markers do not
stamp notify_all itself. Mutex reacquisition contention and host scheduling
remain unresolved. The interval also includes long host progress gaps.
Possible gap-copy write-back remains open. e2 r2 overhead is NOT QUALIFIED;
its earlier contaminated r1 and all gate failures remain preserved.

## Default-off contract

Uses the existing startup/DIAG-gated GpuTrace::enabled cached atomic only.
Each boundary checks that flag before an outlined helper. TLS-bearing
interrupt_trace.cpp helper bodies are in a separate TU, never evaluated by
a disabled caller. HostWait's enabled branch outlines the complete observed
wait; its disabled fallback is the original cv.wait(lock,predicate).
Trace state is not initialized, sampled or touched on that false edge.
Extra static zero-initialized storage is not a claim of zero bytes/cycles;
flag branches and ordinary compiler spills remain. Production Release
codegen is mandatory before any default-off landing or measurement claim.

Same MPSC ring, utility writer, explicit drops and complete footer as the
existing transport; no producer writes or waits on the writer. Record size
in the header changes automatically. More events can perturb timing: the
new instrument needs its OWN exact-exe off/on qualification. Neither the
old ccd negligible-overhead result nor the rejected e2 result qualifies it.

## Events, counters and clock meaning

All boundary timestamps use the existing steady_clock_ns. Stable emitter
thread ID plus host pthread identity identify each lock participant.
The one observed production mutex is interruptMutex; nested mutex.address
and mutex.cv do not reuse GPU packet/ticket IDs. Span is a lock-observation
interval ID, not a baton assignment ID.

| Event | Meaning / limit |
| --- | --- |
| interrupt_mutex_request | Before explicit lock/try_lock; thread may be descheduled before entering the primitive. |
| interrupt_mutex_acquired | After successful acquisition; definite ownership begins no later than this boundary. |
| interrupt_mutex_try_failed | Original try failed; closes that explicit request, no retry added. |
| interrupt_mutex_release_begin | Before explicit unlock; acquired..release_begin is a definite-held lower bound. |
| interrupt_mutex_released | After unlock; completion upper bound, possibly after another thread already acquired. held_ns here is an upper-bound observation, not continuous CPU execution or proof of ownership until this timestamp. |
| interrupt_mutex_release_pending | RAII unique_lock destruction follows; unlock completion unobserved. Used for monitor tryForcePreempt without replacing its destructor or native operation. |
| interrupt_mutex_cv_release_pending | False predicate (or existing alarm wait) will enter the ORIGINAL CV wait; implicit unlock has not yet been observed. |
| interrupt_mutex_cv_reacquired_bound | Next predicate entry or alarm wait return, AFTER mutex reacquisition; never called a CV wake. |
| interrupt_cv_predicate | Original predicate evaluated once at this entry; value is ready/false. Already-true initial predicates create no fake release/reacquire. Spurious/false iterations retain bounds each time. |
| interrupt_cv_notify_begin / return | Immediately around the original notify_all, recording the exact HostThread CV and recipient. Also covers virtual-clock and alarm CV notifications. No added notify. |
| assigned_unresumed_mach_sample | Existing monitor state-poll cadence only; target borrowed Mach port and guest subject, raw thread_info error/state/suspend count/cumulative user+system CPU, sample clock bracket and publication validation. |

mutex.explicit_pending_requests counts traced requests not yet returned or
failed. It is not a kernel waiter count. mutex.cv_waits_reacquire_unknown
counts pending implicit CV cycles, including sleepers that may not have been
notified; it is NOT added to an asserted contender total. Each counter is an
independent concurrent snapshot. The local protocol rejects impossible
trace state by an explicit protocol-error boundary + transport drop, which
invalidates the window without altering the real lock/scheduler behavior.

Coverage includes OSDisable/OSEnable, host exit, the monitor's try-lock RAII
release, bounded hang-report try/unlock paths, hostWait's predicate cycles,
and the alarm thread's original timed/untimed CV waits. Scheduler native
operation token erasure now also covers os_interrupts.cpp.

Actual implicit unlock and kernel CV wake are opaque in std::condition_variable.
Do not label a predicate-reacquired timestamp as wake or infer that an entire
uncovered mutex interval was FREE. Explicit releases bound actual unlock;
CV/RAII release-pending has no direct completion upper bound. New acquisitions
and definite-held intervals from other threads can constrain these gaps.
The pure analysis reports uncovered time as UNKNOWN, never FREE. Sum only
nonoverlapping definite-held lower bounds; do not sum nested waits or add
baton wall duration as CPU execution.

## Mach sample and CPU limits

No new sampling thread, timer, wake or poll cadence is added. Sampling occurs
only at the original monitor state-poll seam, for an assigned/unresumed
holder. Raw errors and unstable samples are retained. A trace-only even/odd
publication counter brackets publishHolder; the sampler re-reads its own
holder/port/hand tuple and validates equal even publication, hand, port,
recipient and unresumed state after thread_info. No seqlock spin/retry.
Publication work exists only inside the enabled gate.

TH_STATE_RUNNING means runnable OR executing; it alone cannot prove
runnable-but-not-running. Cumulative CPU-time deltas plus wall brackets are
supporting evidence, with microsecond source resolution. CPU time is not
physical CPU identity. There is no public remote pthread CPU-number query;
remote_cpu=-1 remains explicit. The target's own later hostWait-return /
thread_cpu_resumed sample identifies its CPU/QoS then, not throughout the gap.
A monitor that is itself starved leaves a sample gap; do not interpolate
unobserved states or turn generation-stability checks into kernel atomics.

## Tests and build/run handoff

M1 work in this change is source/pure-Python only. No local native compile,
syntax check, native unit, app or new trace was executed while B owns r9.
Server compilation and native results are PENDING.

* native_interrupt_trace: real original untimed std::condition_variable,
  already-true predicate, wait/notify/reacquire under the SAME std::mutex,
  serialized counters/bounds, no fake CV-wake marker, pure fake-clock state
  tests (try failure, repeated false/spurious cycles, invalid transitions),
  null-pointer disabled probe. Retain petari_interrupt_trace_tests for
  Release disassembly of petari_interrupt_trace_off_probe.
* native_interrupt_mutex_trace_report: synthetic validation rejects overlapping
  definite owners, invalid held durations, producer gaps/protocol errors,
  invented remote CPU, unstable publication; uncovered time stays UNKNOWN.
* native_gpu_trace_patch retains exact original trace-only proof and the
  independently approved097 native monitor delta. Literal observed-wait
  erasure restores only the original untimed fallback; negative control
  changing that fallback fails. No MonitorWait blanket line removal.
* Existing native_gpu_trace tests cover new Mach serialization and error /
  unknown-CPU fields. Existing source/analysis/attribution regressions remain.

Server plan: app Release M1_COMPAT + retained dSYM; native_interrupt_trace,
_gpu_trace/_patch/_analysis/_attribution, _interrupt_mutex_trace_report,
monitor_wait2, original platform_os L/K/R x3, VI/GX/seam and tape fixtures.
Keep app, GPU probe, new interrupt probe and original/traced OS test exes
with source/compile/UUID/hash metadata. No acceptance from compile success.

Next SOL A slot, AFTER B r9 and PORT's priority handoff: exact production
codegen worker_main/giveCpu/waitForCpu plus OSDisable/OSEnable/hostWait
instantiations/tryForcePreempt/new K helper, both retained off probes.
No trace TLS/clock/host query/allocation before a false enabled branch.
Then PORT's repeated alternating off/on overhead comparisons (same e2 binary
where specified, separately label this new build); each has exact prime-cache
handoff/fixed300/nice0/heavy->native and a stop for review. The original
approved L/K/R screen remains9 original+9 matched trace arms, unchanged40
handoffs/median ANDp90<=1.5ms. Preserve expected legacy FAIL and stop on
API/drop/missing-handshake errors; no auto-combination or promotion. Native
run order/counts for new overhead arms must be bound before their launch.

Only a new clean, adequately qualified instrument can support production
trace cost conclusions. New mutex/Mach evidence is attribution, not a fix;
head's no-spin/source-fix/no-regression/wake-cost requirements remain.
