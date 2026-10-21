# Live-budget lifecycle prototype — source review only

**FAIL_CLOSED / UNQUALIFIED. NO EXECUTION ALLOCATION.** Existing actual prototype
controls failed. Do not reuse this API for an unattended runner, start another
control, substitute a kill mechanism, mask EPERM, or change security/account settings.
The production full caller and packager have `execution_enabled=false` until a
separate reviewed qualification. No raw FIFO or gameplay logs are part of this work.

Existing failure packet:
M1+SERVER scratch-port/solP/dispatch-budget-source-controls/SOURCE-REVIEW-PACKET.json
SHA6078129748415c8b6df2b4ce88c5e4bcb2a83d2dc58f2b3aecfe4d1a160cb6e4.
Old r1/r2 records/source snapshots are immutable. Initial budget signal used OSError;
actual CPython3.15 _wait_kqueue catches it, so it can be swallowed. RuntimeError-based
budget/interrupt/deadline now escaped actual tiny-child waits, but cleanup returned
EPERM. Existing cleanup kept repr only; the exact killpg0/TERM/KILL/poll/wait site,
actual child PGID/SID/UID and per-op timestamps are UNKNOWN. Fixture temporary
metadata was auto-removed on failure. No retrospective identity/absence proof is
possible; later current ps snapshots are insufficient to qualify the absence gate.

## New, UNRUN source instrumentation

`dispatch_budget.LiveBudgetJob(work, seconds, root, min_free, max_growth, interval,
baseline=None)` extends the retained OwnedJob. It is a PROTOTYPE, not a qualified
reusable lifecycle API. It currently remains separate from the disabled full caller.

- Append-only600 NOFOLLOW syscall journal: an intent BEFORE each exact killpg0,
  killpgTERM/KILL, poll and wait; result or errno/error/full Python traceback after.
  Monotonic time, controller PID, lifecycle operation and observed identity accompany
  each event. Records use one os.write+close; process-crash persistence, not a claimed
  power-loss/fsync guarantee. Inability to journal rejects; no success placeholder.
- Disposable source fixture writes its ACTUAL getpid/getpgrp/getsid/getuid fields to
  a600 EXCL file then atomically renames it. Parent verifies actual kernel PGID/SID,
  freezes a copy and journals it. ParentUID/requested start_new_session never supplies
  child UID/identity. A missing/changed/mismatched observation blocks qualification.
  General compiler-child UID observation is NOT qualified by this fixture protocol.
- Fixture identity diagnostics may fail; original group signal/absence calls still
  execute, then identity-observation failure rejects qualification. All signal/probe
  EPERM errors propagate as uncertainty. No alternate kill mechanism is introduced.
- TERM/INT/HUP/deadline/budget exceptions derive RuntimeError to avoid OSError catches
  in the actual Python3.15 wait implementation. Spawn/registration and child mask
  restoration remain inherited from the reviewed single-thread OwnedJob.
- Failed-path exit disables the timer and goes directly to owned-group stop/reap/
  absence BEFORE any allocation-tree scan. It does not attempt a failed-path recursive
  disk scan. Counters/metadata serialize only after the inherited cleanup attempt.
- Fixture roots live under own scratch-port/solP/dispatch-budget-source-controls/;
  exceptions retain them, journals and exact child identity. Success removes only
  that disposable fixture directory after assertions. No old proof input is touched.

## Sampling limitations and future review gates

The monitor checks global free space and own-root allocated st_blocks without
following symlinks. One-shot ticks rearm AFTER a scan, so a scan is sampled rather
than uninterrupted supervision; filesystem syscall latency/scan duration is NOT
presently a hard wall-clock bound. This must remain an explicit review limitation;
do not claim a live50GiB floor/4GiB growth qualification from syntax or mock checks.
The future full/package watcher must preserve its initial allocated-byte baseline
across packaging, use a confined package directory under the same owned root, stop
children before output cleanup, and qualify actual crossing/termination behavior.

Only syntax parsing/py_compile ran on this new instrumented revision. The amended
`test_dispatch_budget.py` is source-only, UNRUN: actual child growth crossing,
synthetic disk-free crossing with a real child, Python work crossing and preserved
packaging baseline. Any new lifecycle control execution requires L source review,
a concrete diagnostic/cause/fix proposal and fresh explicit PORT allocation. A
new control must preserve exact operation, PID/PGID/SID/UID and timestamps; successful
normal child exit or source/model checks cannot substitute for real group termination.

## Source amendment after root review (controls still UNRUN)

For the exact disposable fixture, both child report and independent parent PGID/SID
observations must equal the registered Popen leader PID, not merely each other.
VERIFIED ownership is published only after that check. MISSING, OBSERVATION_FAILED
and CONTRADICTED remain distinct; failed/contradicted observations are sticky for
that fixture. No killpg probe/TERM/KILL is permitted for an unverified or contradicted
fixture target. The mismatch report and registered target remain preserved. This
is not a generic compiler UID protocol and does not explain the old unbound EPERM.

Exit metadata now records actual superclass cleanup success/error/traceback, its
base report and remaining registered leaders/requested groups/ownership. Failure
wording explicitly says success/absence was NOT established; it never unconditionally
claims all groups stopped. Failure-path cleanup still precedes allocation-tree scans.

Four additional source-model tests are prepared for wrong leader despite matching
report/kernel values, missing identity, fully verified leader, and cleanup failure
metadata. They are UNRUN, along with all amended lifecycle controls. Only syntax
compilation was checked; no new signals, child lifecycle executions or native builds.
