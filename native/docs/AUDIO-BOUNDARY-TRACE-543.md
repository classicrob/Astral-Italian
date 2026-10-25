# Audio boundary attribution (diagnostic, not an acceptance result)

This adds to db43db0e's interrupt-mutex/baton instrument. It changes no audio
buffers, pacing constants, callbacks, RT policy, ring operations or report
counter semantics. It does not fix starvation. It does not revise a historical
PASS/FAIL. PETARI_PIPELINE_DIAG plus PETARI_GPU_TRACE still opt in; all events use
the existing bounded MPSC ring and non-realtime writer. Any drop, missing footer,
I/O error, binding mismatch or invalid frame join invalidates the window.

## Boundaries and their limits

* `audio_device_callback_entry` is at the first fill hook, before the existing
  allocation scope and first-call RT promotion. `audio_device_callback_exit` is
  at the normal end or SDL-enqueue failure return. Both have steady_ns, callback
  span, emitter identity and local QoS/priority/CPU sample. Host sampling happens
  AFTER the boundary timestamp and carries its own timestamp.
* Each boundary samples `ring.level()` and timestamps that sample separately.
  This is a concurrent producer/consumer snapshot, not an atomic snapshot of the
  entire callback. Requested frames use the original ceil(additionalBytes / 4)
  formula (non-positive bytes request zero).
* Previous entry and exit timestamps are trace-only atomic bookkeeping across
  callback threads, allowing inter-entry gaps and between-callback idle gaps to
  be computed even after thread migration. `active_callbacks_before` and
  `protocol_error` expose overlap/nesting; those windows are invalid. A trace
  that stops inside a callback has a right-censored final callback, not a zero
  duration. No state from this bookkeeping controls audio.
* `realtime_api_status` is the existing deviceThreadRealtime value: 0 before the
  attempt, 1 if the Mach time-constraint request was accepted, 2 if refused. It
  is NOT a continuous effective-policy readback. The first entry may be 0 and
  its exit 1. Emitter host policy samples are separate evidence.
* `audio_device_ring_shortfall` brackets the SAME single ring.read, emits only
  when it returned fewer frames, and links to the callback span. Missing frames
  are zero-filled by the original ring. This is before SDL enqueue; it is not
  hardware output time or the moment a listener heard a click.
* `audio_underrun_report_exchange` brackets the SAME single destructive
  takeUnderrunFrames exchange. Its count is exactly the count printed in the
  existing periodic/shutdown report. The exchange may race with the consumer's
  increment; overlapping timestamp brackets remain ambiguous.

The old `[audio] ... at N s process time` uses gettimeofday-derived process
elapsed time, while the new boundaries use the trace monotonic clock. A report
count spans the interval since the previous counter exchange. Do not assign an
aggregate such as 148 frames at 186.398 s to one guessed frame. At 32 kHz that is
4.625 ms of ring shortfall, not necessarily 4.625 ms of callback delay.

`audio_trace_report.py` consumes retained boundary records after independent
full-stream binding/drop/footer and frames.csv mapping checks. It preserves
possible frame intervals and report-exchange races, validates callback
entry/exit identity and chain, and reports between-callback gaps separately
from callback duration. It makes no acceptance claim. A long entry gap with a
short callback duration suggests delay outside the measured callback body; a
long callback body still requires its interior ring/SDL/baton/mutex evidence.
Neither observation alone proves hardware scheduling or interruptMutex cause.

## Off-path and validation contract

New callback bodies are noinline and invoked only inside PETARI_GPU_TRACE's
cached-flag / outlined-lambda gate. No trace TLS, clock, ring-level sample,
API-status load, host query or allocation may be hoisted before that gate.
The ring/read and report adapters retain literal original disabled operations.
Source erasure must reproduce every original SDL operation/token, including all
arguments, branch/return sites and the existing first-call promotion.

Pure Python checks cover that erasure and negative mutations, count/exchange
races, callback gaps, migration, invalid overlap/identity/clock/read linkage and
right-censoring. The new native_gpu_trace_audio fixture compares original and
instrumented PacedRing output, levels and destructive counters off/on, checks
serialized boundaries and zero transport drops. It opens no audio device.
Retain its executable and disassemble petari_audio_trace_read_off_probe,
petari_audio_trace_report_off_probe, and petari_audio_callback_off_probe.
Native compilation/tests and exact Release codegen are PENDING server delivery;
no M1 native job was used for this source change.

Retain/repeat the db43 production/probe codegen checks, plus fill and report:
worker_main, giveCpu/waitForCpu, OSDisable/OSEnable, hostWait instantiations,
tryForcePreempt, K helper, GPU probe and interrupt probe. A source-only erasure
PASS is not the Release codegen proof.

## Next-slot proposal, subject to PORT's run order

After item (a) releases the M1, first verify delivered source/binary/test hashes,
codegen and native fixtures. The new larger record and callback host samples
need their OWN repeated alternating same-exe off/on overhead qualification;
ccd/e2 conclusions do not carry. Preserve fixed first-300-second comparisons,
prime/second cache+Metal handoff, nice 0, per-arm heavy -> native, quiet rules and
all failures. Preserve the authorized independent K/R latency screen, unchanged
1.5 ms median AND p90 predicate and monitor wake-count accounting; no automatic
policy promotion.

For audio, propose bounded matched long production-like warm observations
(e.g. 900 s each, with the 300 s comparison retained), actual CoreAudio and
AUDIO_DIAG, PCM capture OFF, same namespace/cache lineage and exact executable.
Duration/count/order need a concrete plan before launch. Stop for review after
the approved cohort; never rerun until zero underruns. Correlate a captured
shortfall's bracket and callback entry/exit with the existing baton/mutex/holder
and VI records. Keep hardware timing unknown and callback-dispatch vs pinned
Dawn queue completion inseparable. No spin, alternate wait or backend patch.
