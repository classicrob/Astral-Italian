# Opt-in DefaultOutput implementation: server handoff

Native implementation of PORT-approved design 3fc03a50. **SDL remains default**;
only `PETARI_AUDIO_SINK=output-au` selects the new sink. No new SDL patch or
prepared SDL tree is required: reuse the approved 0001/0002/0003 tree from
aq543-c1276e5b. No root src/include/libs or game/tape sources change.

## Implemented boundaries

* `output_au_control.cpp`: DefaultOutput creation/readback/listeners, non-render
  control CFRunLoop and power notifications, existing guest/DSP async workgroup
  binding on the actual AI-start caller, producer, reports and recovery.
* `output_au_converter.*`: producer-only AudioConverter SRC/format conversion,
  normal/high SRC, prewarm/reset, persistent temporary-starvation semantics and
  two input slots retaining buffers until the next input request. Equal-rate
  PCM conversion deliberately does not set unsupported SRC-only properties.
* `output_au_core.hpp`: one preallocated planar device-rate SPSC ring and a
  guest-time ledger that includes converter/staging input in source credit.
  Original 1 ms / 5 ms credit / 1.05 / 1.25 pacing is reused; 48 ms remains
  the default cushion. No global device rate/buffer-size mutation. Actual
  I/O geometry determines the existing registration-wait formula.
* `output_au_render.cpp`: the actual C callback. Validates bounded buffers,
  copies/zeros, updates lock-free counters and, only when enabled, publishes
  bounded records. No AI/DSP/baton, converter, SDL, allocation, locks, waits,
  scheduling, logging, file capture or framework property calls.
* `audio_workgroup_rt.cpp`: the real shared flight-history publisher, isolated
  so the prohibited-call fixture tests it as well as the production callback.
  All OS policy/adoption/report code remains outside this TU.

The SDL facade preserves start/stop/active/mute/submitted operations and the
existing SDL background-mute hint. Unrecognized selectors refuse explicitly;
there is no auto-promotion/fallback. Callback file capture and AQ-count overrides
are explicitly refused on AU rather than causing real-time I/O or a false arm.
The sink supports an initially selected guest rate of 32 or 48 kHz. An in-place
guest-rate change without a source/sink restart is explicitly refused; the
existing AI engine does not notify sinks from AISetDSPSampleRate. No unsupported
rate change is silently played at the old rate or claimed covered by tests.

## Health, recovery and latency evidence

Raw device-frame misses and source-equivalent cumulative-ceil accounting are
both reported. Small misses cannot round away. Missing/invalid timestamps,
sample-time gaps/repeats, malformed buffers, reentrancy, callback cessation,
converter errors and device overloads are independent sticky health failures;
zero ring misses cannot hide them. `native/tools/audio_output_au_report.py`
streams the log and checks these in addition to >=50 real CoreAudio reports /
zero underruns and workgroup/history integrity. **Every AU campaign/freeze
collector must join this validator**; the legacy ring-only predicate is
insufficient for AU. No threshold is reduced and no old receipt is relabeled.
Before promotion the common functional collector must require these AU health
checks on its AU arms too; this source handoff does not claim that integration
or acceptance has already happened.

The per-generation and final reports retain the ring minimum, with its basis
explicitly `consumer-snapshot`, in frames and milliseconds. All generations
remain visible. Reconfiguration closes/unpublishes the old generation, stops
the producer/unit and proves callback quiescence before freeing it. Numeric
listener tokens cannot dereference an old owner. Failed quiescence retains the
owner and stops; it never frees it to retry. Repeated failed recovery requests a
graceful game power exit instead of an apparently healthy silent app. Duplicate
unchanged notifications are re-read/acknowledged without repeatedly reopening.
Default/rate/layout/quantum changes, service restarts and sleep/wake reopen the
negotiated sink; no silent SDL substitution occurs.

Raw latency reports include device/stream/safety/IO-buffer components, errors,
rate, AU latency and converter prime information. Parallel output streams are
not summed; unknown channel routing/property support stays unknown. The live
estimate includes all source audio ahead of consumption (including converter
hold), with before/after consumer-index bounds. Reported hardware/AU overlap is
an interval, not a double-counted exact delay. It assumes steady continuation,
not immunity to a later producer stall, and is not an acoustic measurement.
Before/after latency acceptance still requires the matched baseline CoreAudio
snapshot and sample-ledger analysis from the approved measurement plan.

History adds kinds43–54 for AU entry/exit, ring levels, raw shortfall,
sample/presentation times, converter input/output, trigger, cumulative device
consumption and source enqueue. It retains the original guest assignment /
resume records. Actor0 is a **logical AU consumer**, actor1 a logical producer;
neither is fabricated physical thread/QoS/CPU evidence. The callback performs
no host policy queries. A control-path bracket maps Mach ticks to steady ns
for each device generation, including wake; the raw pair and uncertainty are
logged. The current OS mach_absolute_time fast path must still pass the actual
Release/callee no-trap check before measurement. No prior trace instrument's
qualification transfers.

## Device-switch restriction

**No default-device setter/helper is implemented or linked.** Public CoreAudio
setters can fail, and signal-time restoration of the user's original default
cannot be guaranteed safely. PORT binding3 therefore chooses the manual test,
rather than claiming an automated helper can restore on every signal/exit.
No automated cohort changes the user's system default.

After PORT's explicit approval **at that time**, record the original UID, use
System Settings / the output chooser to perform A -> B -> A during output,
verify the final UID, and record audible/known-signal recovery, generation,
rate, ring/health/outage evidence. Also check headphones/AirPods disconnect,
sleep/wake and quit during transition. Restore the original selection manually
even if the app/test fails, verify it, and log it. Without that approval or
physical device availability report NOT TESTED. Fake-device tests below use
only fake IDs and never call a system default setter.

## Validation and remaining gates

New native targets / ctest names:

| target | ctest | scope |
| --- | --- | --- |
| petari_audio_output_core_tests | native_audio_output_core | SPSC/conservation, fractional feedback, hidden staging, retirement/token protocol |
| petari_audio_output_converter_tests | native_audio_output_converter | real CPU-only 32/48k ->8/16/32/44.1/48/96k, mono/stereo, finite EOF vs temporary input, chunk invariance |
| petari_audio_output_render_tests | native_audio_output_render | SAME production callback + actual history publisher, first/error/observer paths, guard self-test, timeline/full-ring, mute/null buffers/retire |
| petari_audio_output_control_tests | native_audio_output_control | SAME control/producer/render, injected fake device, device/sleep recovery, duplicate notification, partial-open/start failure and safe failed retirement |
| (Python) | native_audio_output_source | renderer/publisher service boundary, default selector/no setter, guarded real callback |
| (Python) | native_audio_output_report | false-pass regressions for raw misses/timeline/errors/missing data/history loss |

The guard is a **separate dylib** because dyld exempts the interposer's own
image. Its positive self-test detects an inert guard. It intercepts allocation,
deallocation, mutex/condition/semaphore waits/notifies, sleeps, trap-facing clock
and Mach queries, file I/O, and audio-framework calls. Keep
`libpetari_audio_output_rt_guard.dylib` **beside** the retained render test exe
when copying; its CMake loader path is portable. Direct syscalls or helpers
missed by symbol interposition remain covered by the separately mandatory
production Release reachable-callee disassembly. Do not call the dynamic guard
alone a proof of the linked app's callback/clock behavior.

Local CPU-only source receipts (external nonblocking heavy -> native;
no app, device output, default switch, GPU build or cache activity) live under
`port53/m1g/build-reports/sola-output-au-source-r*/`. Earlier syntax/test stops
are retained, including the equal-rate unsupported-SRC-property defect and
the initially inert in-executable interposer. The fixes address their mechanism;
assertions and zero-underrun predicates are unchanged. Final source/executable
hashes and commands are in the newest completed receipt. This is source/fixture
validation, not hardware recovery, latency or pacing acceptance.

PORT server build/test request: normal build542 app/weak-import, the six new
ctests, then existing audio/DSP/pacing/boot/present, workgroup/history, GPU trace
set/patch, pipeline/title/frontend and tape fixtures. Retain app+dSYM plus the
new four test exes **and guard dylib**. Report unrelated baseline fixture rot
separately. No SDL tree change.

After that build is reviewed: exact app Release callback/helper/clock codegen,
off probes and retained tests; new observer's 12-arm overhead -> **STOP**;
then the approved SDL/AU/SDL/AU pairs (315/300 primes,870/855 seconds, history
both launches) -> **STOP**; then separately approved manual device recovery
and matched M4 no-regression. Full functional freeze gates and PORT+head
promotion sign-off come later. All positives remain real FAILs, with no
reruns-until-clean. No app/cohort/default switch was run for this source step.
