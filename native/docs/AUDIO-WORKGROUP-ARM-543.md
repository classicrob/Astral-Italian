# Asynchronous audio workgroup arm (source implementation, not acceptance)

PORT approved design 67afcd63 with exactly two members: the actual guest audio
pthread and native DSP renderer. This arm is disabled unless
`PETARI_AUDIO_WORKGROUP=async`. It does not alter the producer, dispatchers,
Pacer, buffer sizes, sample values, game priorities or K/R mode. It adds RT
policy to the two named host roles, then joins them to an application-owned
`AudioWorkIntervalCreate` group. Each successful adoption includes exact Mach
time-constraint policy readback (period 0, computation 500 us, constraint 2 ms,
preemptible). Readback is at adoption, not continuous proof against later
kernel demotion. Device workgroups and SDL dependency patches remain deferred.

## Source contract

- AI callback registration binds its actual caller's OSThread and monotonic
  HostThread generation. AIStartDMA must confirm that same caller/callback.
  The real SDL sink must confirm `SDL_GetCurrentAudioDriver()==coreaudio` before
  creating a session. Priority values and symbol-name guesses classify nothing.
- SDL startup already releases the baton for host work. The guest joins there.
  DSP joins outside its mailbox lock. Both save their prior policy and leave
  using their own join token. Errors are cumulative and refuse the arm; there
  is no successful silent fallback. Last-error codes preserve Mach versus
  workgroup API values and identify the failing operation.
- A DSP render command (command 0x02) is the production cycle. Provenance travels
  alongside the unchanged mail queues only in the enabled arm. Its header and
  arguments must come from the bound guest/session. The header's actual enqueue
  Mach time is the interval start; all requested subframes (80 samples each)
  define the sample budget. Deadline = start + frames / (source rate * 1.25),
  using the existing Pacer maximum, without deadline extension after a miss.
- The DSP renderer is the interval primary. It finishes only after writing the
  final subframe, before the existing final acknowledgement mails. This covers
  the guest/DSP subframe handoffs, not all guest preparation before the command
  or guest processing after the final acknowledgement. It does not claim a
  direct address identity between planar DSP output and a later mixed DMA
  buffer. AIInitDMA records its separate generation/address/sample count and
  **cannot finish a DSP interval**: JASDriver::updateDac registers an earlier
  mixed buffer while the DSP pipeline can still have pending work.
- Pause (AIStopDMA) requests cancellation on the owning renderer at its next
  native safe point; native mail/output processing is unchanged. Late work
  remains counted even if cancelled. Resume allows subsequent production
  intervals. Stop invalidates the session and wakes both members to leave on
  their own threads. The original untimed guest wait and baton predicate stay
  authoritative after a host-only control wake. Thread exit leaves before
  pthread_exit. Retired groups are retained until quiescent and collected by
  the reporter/control thread, not freed on a realtime path. A failed finish
  poisons retention and refuses the arm; it is not counted as completed work.
- These groups are independent of output-device identity. SDL route migration
  and resampling retain the stream's guest source rate, so they do not require
  joining a different device group. Reopening the sink creates a new session;
  role tokens leave the old one first. A changed guest source rate without a
  sink reopen is a refused arm, not an invented new deadline. Lifecycle tests
  cover session replacement/closed-during-join and same-address/new-generation.

## Cheap verification, separate from the invalid GPU/mutex firehose

`PETARI_AUDIO_DIAG=1` emits cumulative workgroup adoption/errors, cycle/sample
counts, deadlines and member identities from the existing reporter once per
second. `[audio-backend] driver=coreaudio` records the actual SDL driver.

`PETARI_AUDIO_WORKGROUP_DIAG=1` additionally enables a 16,384-cell in-memory
MPSC history. Events: DSP cycle/shape/destinations/deadline/subframe/finish, AI
registration, audio callback ring level/request, exact read shortfall, guest
baton assignment/resume. Payloads are atomic; a producer makes one cell-claim
attempt with no waiting, allocation or formatting. An occupied/stale cell is
an explicit drop. Observations and workgroup TLS helpers are behind cached
flags; TLS helpers are out of line. Off-probe/codegen proof is still required.

Triggers: assigned-to-resume >20 ms, callback entry gap >20 ms, actual ring-read
shortfall. The reporter dumps up to 4,096 events from the preceding two seconds
at most once per periodic report, plus a final shutdown dump. Trigger count is
an interval aggregate, not an atomic snapshot of a particular event window.
Overwrites, unstable payloads and clipping are explicit and invalidate that
history. No mutex acquisitions are streamed or captured by this smaller
instrument. It cannot by itself distinguish mutex reacquisition from host
scheduling. A >=175 ms assigned-to-resume episode (the conservative retained
case's lower bound) refuses the structural arm: **workgroup insufficient;
baton dependency**. Missing observation is never proof that it did not recur.

The reader `native/tools/audio_workgroup_report.py` preserves the ea7dc9d3
>=50 reports / zero underrun rule, validates adoption/history and makes no
automatic acceptance claim. Exact shortfall comes from the existing single
ring.read return value, not a sampled level or a second underrun-counter take.
The old diagnostic trace remains untouched, but **PETARI_GPU_TRACE must be
unset in this cohort**: its per-mutex streaming was invalidated and is not this
verification instrument. No ccd/e2/de3 overhead verdict carries forward.

## Predeclared next measurement plan (no launch authorization inferred)

Server first: build app + protocol/backend/history fixtures + existing audio,
DSP, OS, GPU trace and tape tests. Backend fixture fakes scheduling/workgroup
APIs but uses the actual native OS parked-guest control wake. No real device or
RT promotion in those lifecycle tests. Retain test executables. App codegen
must show no workgroup TLS/API work before the cached guards in guest wait,
grant, enable, DSP worker and callback/read paths; retain return-value off probe.

After server validation and an explicit measurement handoff:

1. **Own verification overhead:** six alternating DIAG-OFF/DIAG-ON warm pairs,
   twelve arms. Every arm uses the same signed/pinned executable, real CoreAudio,
   workgroup=async and AUDIO_DIAG=1; only the ON second enables WORKGROUP_DIAG.
   Prime/second preserve exact private shader + Metal handoff, fixed300 metrics,
   observe315, nice0, heavy->native and unchanged quiet guard. Full GPU trace
   unset. Report every pair and pooled distributions, callback gaps, CPU/wakes,
   stream bytes/events and all audio positives. STOP for PORT overhead review.
2. **Long structural cohort:** exactly four warm-second arms, mode sequence
   OFF / ASYNC / OFF / ASYNC. Each has its own 315 s prime of the same case and a 915 s
   production-like arm with a fixed first900 window; same private-cache handoff
   and immutable exe. Arms 1/2 use the same fixed title/no-input case; arms 3/4
   use direct `--stage YoshiEggGalaxy --scenario 1`, no input. Commands and
   source/hashes freeze with the campaign manifest before launch. AUDIO_DIAG=1 on all;
   WORKGROUP_DIAG=1 on ASYNC only. No changes after inspecting a result.
   Stop immediately for binding/namespace/process/protocol/adoption/invalid-
   history failure, or recurrence of the long assigned-unresumed discriminator;
   report the first failure. An audio positive alone is retained, fails its
   arm, and does not add/remove any of the remaining predeclared arms. No
   replacement or rerun-until-clean. STOP after the cohort for review.
3. Paired M4 OFF/ASYNC audio/pacing cohort: same cases/windows/order and original
   predicates, retaining every positive. Report p99/post100 p99/>50 ms/max,
   underruns, AI replays/DSP holds, CPU/wake/energy deltas or unknown.
4. Normal/full-star functional gates retain ea7dc9d3 audio rules and strict
   replay. They run on the integrated accepted 5.4.3 source line, not on an
   older diagnostic's tape implementation. This branch's native delta is based
   on de3dfa77 plus design 67afcd63; it is not a candidate or main merge.

K/R stays deferred. A successful join or native fixture does not establish the
player-facing fix. Long M1 zero-underrun evidence, no M4 pacing regression,
strict functional evidence and PORT review remain required.
