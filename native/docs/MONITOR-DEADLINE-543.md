# Forced-preempt monitor deadline proposal (SOL A, 2026-10-25)

Status: DESIGN / source-only. No timer or scheduler fix is implemented by this
commit. Base e2f71c485c794bd9283f58f962a1baff08045379. The current e2 overhead
cohort runs unchanged; the next measurement slot belongs to SOL B r8. Any
implementation and A/B execution follow PORT review and the assigned slot.

## Measured problem and scope

Quiet M1 Max / macOS15.1, exact e2 M1 Release original executable 0bbf6a1e:
native_platform_os full test fails its unchanged <=1.5ms handoff assertion at
median2.50ms/p902.53ms. The unchanged priority-only fixture gives off2.49/2.53
and trace-on2.50/2.52ms. Each processes40 messages and records40 forced
preemptions. The trace-only adapter enables transport before the ORIGINAL
fixture main; it changes no assertion or workload. Both exit1, retained.

Of42 unsignaled Mach semaphore waits requesting <=0.5ms, requested median is
0.492896ms, observed return median2.472812ms and lateness median1.979521ms
(p901.991583,max2.024293ms). All42 monitor return samples have QoS33,
priority31, CPUs2..5. IODeviceTree reports these exact logical IDs as P cores.
This rejects E-core placement/low requested QoS for these observed returns.
The timestamps include kernel wake AND dispatch/observation: timer
coalescing is strongly supported, not isolated as a pure kernel measurement.

Evidence: port53/m1g/campaigns/sola-543-traceh-e2-r1/platform-os/
RESULT.json, ATTRIBUTION.json, TIMED-WAIT-SUMMARY.json, raw gpu-trace.jsonl,
COMPILE.json and LINK.json.1752 boundaries,0drops,complete footer,
independent holder/wait/deadline audit errors=[]. No fix claim.

## Recommendation and alternatives

Try a critical one-shot deadline on the existing monitor thread FIRST. Keep
its ordinary QoS and compare RT independently; do not give an unrestricted
real-time contract to the monitor's stack inspection/name-learning code by
default. Preserve one blocking wait and the existing request notification;
no spinning, extra periodic helper, faster nominal poll or busy retry.

| Arm | Proposed mechanism | What it tests / limitation |
| --- | --- | --- |
| L | Current semaphore_timedwait, existing UI QoS | Same-exe control; all original constants and wake semantics |
| K | kevent64, one EVFILT_TIMER deadline and EVFILT_USER request notification; NOTE_MACHTIME + NOTE_ABSOLUTE + NOTE_LEEWAY with ext[1]=0, add NOTE_CRITICAL only while a forced-preempt request is pending | Explicitly reduces timer coalescing without changing monitor scheduling priority. No dispatch callback thread. NOTE_CRITICAL minimizes coalescing; it is not a guarantee of zero dispatch delay. Idle polling remains a normal, coalescible2ms deadline, not a critical timer. |
| R | Original semaphore wait with THREAD_TIME_CONSTRAINT_POLICY on only the monitor | Separates real-time thread policy from the timer primitive. Proposed period500us, computation250us, constraint500us, preemptible=true, converted to Mach units with overflow/rounding checks. The SDK raises computation below constraint/2; a supposed50us budget with a500us constraint would be misleading. This nominal50% contract is a significant scheduling change, even if actual CPU use is small. Record requested/effective policy and errors. |
| KR | K plus the exact R policy | Only if neither single change meets the target; do not promote a combination unless evidence requires both. Same latency/cost conditions apply. |

R's500us period matches the EXISTING forced-preempt threshold, not a new
periodic500us timer. The existing idle interval stays2ms. The RT monitor
currently may perform thread_info, suspension/state reads, stack/code
inspection and locks; no claim is made that all that work fits250us CPU.
Measure it, record policy readback/demotion observations and reject a budget
overrun or a degraded audio/game workload. API denial is a failed RT arm,
not a silently relabelled legacy pass. No entitlement/root request planned.

Two other strict-primitive choices were considered:

* DISPATCH_TIMER_STRICT with zero requested leeway remains best effort under
  Apple's contract. A dispatch-source handler adds queue/callback scheduling
  before the monitor can continue, or a second thread that signals the old
  semaphore. That adds an unmeasured handoff and wake cost. Keep it as a
  fallback if K proves unsuitable, not another automatic cohort.
* mach_wait_until expresses an absolute deadline but has no explicit user
  leeway argument or integration with the existing semaphore notification.
  A bare replacement loses early request wakeups; an additional helper that
  interrupts the sleep adds a handoff. No assertion that this call alone
  bypasses coalescing; do not select it on its name alone.

## K implementation contract (for review before code)

Native platform files only, primarily native/platform/os/os_thread.cpp plus
an isolated monitor-wait helper and its fixture. Do not touch root
src/include/libs, game logic, input, GPU/Dawn or tape sources. A cached,
startup-selected diagnostic mode chooses L/K/R/KR. L remains the default
through the experiment. Bind requested AND effective mode in every receipt.

Initialize the kqueue and persistent EVFILT_USER before publishing its
notification handle or starting the monitor. Only the existing monitor owns
timer registration, cancellation and event draining. setPreemptPending keeps
its existing false->true transition and gMonitorWakePending coalescing:
notify the chosen backend once, clear the notification flag BEFORE querying
current pending/holder state, as the original receive path does. Do not send
to both backends (which would leave a stale semaphore token).

The timer represents the ORIGINAL computed wait/deadline. Do not recompute
a later deadline after EINTR or a spurious/stale event. Process simultaneous
request+timer events in one monitor iteration. Cancel/retire the old timer
when a request wakes it; tag generations and reject stale completions. A
request received before the blocking call must remain observable. Recheck
pending generation/holder state before sleeping; prove the before-block,
event-clear and pending-clear/new-pending races in fixtures. No notification
edge may depend on a periodic backup poll for correctness.

Rearm normal idle waits at the unchanged2ms cadence. Use a critical timer
only for the existing pending deadline/refusal wait. A cleared pending
request may leave a short stale deadline; consume/cancel it without a second
poll/work pass or duplicate notification. Existing refusal steps remain
0.25,0.5,1,2ms and NeedNames retry0, with the same safe-point rules. A due
zero wait means the original immediate attempt, not a busy retry loop.

Reuse the existing atfork quiesce order. In the child invalidate/close only
its inherited backend descriptor and clear the published notification handle;
no monitor or helper survives there. Test fork while a deadline is armed,
notifications racing a fork, and failure during initialization. Start failure
can retain L for production robustness, but an experimental K/KR receipt with
fallback is FAIL for that requested mode. Runtime backend errors stop the
measurement arm; never rerun past them to obtain success.

Instrument per wait: requested/effective backend, absolute Mach deadline and
its explicit conversion to the existing steady_ns trace clock, requested
leeway/critical flag, wait-entry/return, kernel result or event flags,
notification/timer/both/EINTR cause, generation and stale-event accounting.
Do not subtract an unverified cross-clock offset. Preserve current baton,
QoS/CPU, requested deadline, actual monitor wake, attempt and backoff events.
Every trace producer remains behind the accepted cached flag/noinline guard;
default-off codegen is required again on the implementation build.

## A/B plan after the current cohort, SOL B r8, and PORT approval

One pinned Release diagnostic binary contains L/K/R/KR, with an original L
source/control binding. M1_COMPAT=1 on M1; server M4 build by PORT. Pin each
host's executable/compiler/SDK/UUID separately. Experimental modes do not
silently alter the default. No rebuild between modes on a host.

1. Source/fixture proof first: fake wait backend covers exact unchanged
   deadlines/backoff, event generation, notification races, simultaneous
   events, EINTR, cancellation, error/fallback and fork. Existing OS, VI,
   GX/seam and trace tests run unchanged. Retain the known stale title fixture
   failure separately. Production and retained probe off-codegen must pass.
2. Cheap latency screen on quiet M1: cyclic L/K/R, three independent runs
   each (9 arms), each using the ORIGINAL priority-handoff fixture and its
   unchanged40 messages/<=1.5ms median ANDp90 assertions. Run matching
   trace-enabled adapters for requested-deadline/return distributions,
   separately bound and labelled; never substitute them for the unmodified
   unit result. Do not run KR unless both single mechanisms fail and PORT
   approves that added arm. Preserve failures; an unmatched input/error stops
   its arm, with no favourable rerun substituted.
3. PORT repeats the same cyclic3x L/K/R unit/trace screen on a quiet M4.
   No M4 baseline may be substituted from the earlier source/compiler.
4. Only a mechanism with all3 quiet M1 untraced unit passes, reduced traced
   lateness in all3, and all3 M4 passes proceeds to game cost checks. Use
   unchanged same-cache prime->warm-second title pairing, nice0, fixed300s
   after first present, observation>=315s. Cyclic L/winner x2 pairs initially;
   four pairs/eight arms, no extra builds. Report every prime and second,
   exact cache/Metal continuity, frame distributions and startup/audio checks.
   A trace-on spot may follow ONLY after its own off/on overhead review.

All native builds/tests/measurements are detached and take per-job
m1-heavy->native. Same quiet guard: load1/load5<=8, compiler CPU samples10s
apart, idle allowed, camera load retained. B namespaces/caches untouched;
owned perfm1a only. No outer lock across cohorts. Stop at each review point;
no automatic candidate acceptance or timer-policy promotion.

## CPU, wake and energy cost: required evidence, not an assumption

No500us periodic source is added. Strict timing is limited to actual pending
requests; this is intended to avoid an idle wake increase, but must be
MEASURED. Record raw monitor iterations/wake count from the existing counter,
request and timer returns, notifications, retries, stale events, syscall
count, monitor user+system CPU, and process CPU for each equal wall window.
Show raw counts/rates AND per40-handoff figures; do not hide increased wakes
behind normalization or a shorter test duration. The fixed300s app window is
the main wake-cost comparison. A higher raw monitor wake count fails head's
no-rise condition; do not reinterpret faster expiration as a free fix.

Use matching start/end monitor-thread CPU samples and
proc_pid_rusage process CPU/interrupt_wkups/pkg_idle_wkups. Check supported
rusage version before accessing fields. V6 ri_energy_nj, when available and
credible, is an OS process energy estimate; zero/unsupported/error becomes
unknown, not zero joules. Hold machine power mode, charger state and
background load fixed and record them. If no direct energy estimate is
available, report CPU/wake proxies with their limit and send that gap to
PORT. Do not introduce a privileged powermetrics run or claim energy parity
from a single noisy sample. Record trace writer CPU separately; cost arms
use trace OFF, with only bounded start/end probes. Head has specified no
numeric CPU/energy tolerance, so report deltas for review rather than invent
one. All existing gameplay/audio/frame predicates stay unchanged.

## Primary API sources and uncertainties

Read against the local macOS15.1 SDK:

* mach/thread_policy.h:120-165, period/computation/constraint units and the
  computation>=constraint/2 adjustment; requested RT contract is not an
  execution-time guarantee.
* sys/event.h:301-318, NOTE_ABSOLUTE/LEEWAY/CRITICAL/MACHTIME.
* dispatch/source.h:332-344,696-722, strict timers retain a minimum possible
  delay and may cost power. [Apple dispatch timer contract](https://developer.apple.com/documentation/dispatch/dispatch_source_set_timer).
* [Apple XNU event flags](https://github.com/apple-oss-distributions/xnu/blob/main/bsd/sys/event.h)
  and [timer filter implementation](https://github.com/apple-oss-distributions/xnu/blob/main/bsd/kern/kern_event.c)
  support the critical-vs-normal timer distinction. These current-main
  sources are explanatory, NOT proof of this M1's exact shipped kernel or
  actual leeway. The measured distributions remain the decisive evidence.

Open questions for implementation proof: whether K actually removes this
M1's~2ms return delay without adding monitor wakeups; RT policy acceptance
and effective budget under code learning; error/fork behavior; M4 latency;
measurable CPU/energy cost. No success is assumed from an API name.

## Implementation submitted for server compilation (source-only)

`PETARI_BATON_WAIT_MODE=legacy|critical|realtime`, read once at monitor startup;
unset is legacy. No combined mode exists. Neither a policy nor timer choice
is promoted as a default. Only K creates a kqueue, only R requests RT policy.
Experimental setup/runtime errors print FATAL with native error/readback and
abort the arm; no fallback can masquerade as a successful requested mode.

The K backend publishes its notification availability only after the
persistent user filter is registered and acknowledged. The monitor primes
an already-pending startup request once before its first wait. Notification
coalescing/clear-before-state-inspection is shared with the original semaphore
path. User+timer delivery is one work iteration. A generation-tagged one-shot
is cancelled/acknowledged after delivery; ENOENT after automatic one-shot
removal is normal. EINTR re-enters a blocking receive with no deadline change;
stale generations cannot complete a current wait. Unexpected/error results
are exposed, not silently retried. Child close is limited to inherited
kqueue and lock-free atomics under the existing atfork ordering.

The K trace distinguishes raw `monitor_kevent_return` (before event decoding
and cancellation) from `host_wait_return`/`monitor_woke` (whole helper,
including cancellation). Mach samples are bracketed by two explicit steady_ns
samples. Lower/upper mapped deadline events expose that clock-conversion
uncertainty; the wait's deadline field is labelled as the UPPER bound, never
an assumed exact cross-clock offset. Raw Mach deadline, cause, return count,
interrupts and stale events are retained. Idle timers omit NOTE_CRITICAL;
requested leeway remains0 and nominal waits/backoff are unchanged.

R logs requested500/250/500us and actual tick fields, set/get/port-release
errors and default/count readback. Trace-on R reads the same policy at every
monitor wake and records its fields, API result and host sample. A failed or
mismatching readback fails the experimental arm. This public readback is
reported as such, not a guarantee that the kernel never transiently demoted
the thread between samples. No RT policy change is made to any other thread.

New server targets/tests:

* `petari_monitor_wait_protocol_tests` / `native_monitor_wait_protocol`: real
  production protocol with a fake blocking backend; conversion/overflow,
  no combined mode, RT contract/default mismatch, generation rollover,
  notification coalescing before/after clear and publication, before-block
  and cancellation races, simultaneous timer+user, unchanged deadline across
  EINTR/stale delivery, immediate retry, arm/receive/cancel/cleanup errors,
  empty/oversized/malformed batches. Assertions explicitly enabled in Release.
* `petari_monitor_wait_backend_tests` / `native_monitor_wait_backend`: actual
  Darwin critical backend, sticky pre-wait notification, cancelled-timer
  isolation, immediate timer, asynchronous user event and fork-child descriptor
  isolation. No new latency threshold; the original OS timing test remains
  authoritative. Assertions enabled, serial,15s external test cap.

No native compile, syntax check, fixture execution or measurement was run on
the M1 for this change: SOL B owns r8. Source review/whitespace checks only;
server compile and all native results are PENDING. The existing source-only
proposal and e2 diagnostic receipts remain preserved. Required server checks:
new2 tests; unchanged native_platform_os x3 legacy; trace/probe/patch/analysis/
attribution; VI/GX/backend/seam and tape suite. Then, only in the assigned M1
slot, inspect exact-production off-codegen again, rerun the PORT-contaminated
e2 overhead cohort, and conduct approved L/K/R measurement arms. No success
or <=1.5ms claim is made by this source submission.

New K helper codegen must also be inspected: its single explicit cached-flag
branch outlines the complete paired-clock/trace body; the false branch only
calculates the normal Mach timer plan. No zero-initialized trace clock locals
are placed before that flag. Actual new compiler placement remains PENDING.
