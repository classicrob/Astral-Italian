# Output AudioUnit sink — design for PORT review

Status: **approved by PORT after 3fc03a50; implementation is opt-in and awaits
server build, production codegen and measurement review**. This supersedes the proposed
AQ-count discriminator as the next implementation direction; every retained
AQ/WG failure remains a failure. SOL A owns the native audio implementation;
server builds and subsequent measurement starts remain separately reviewed.
Implementation and the later device-switch binding are documented in
[OUTPUT-AUDIOUNIT-IMPLEMENTATION-543.md](OUTPUT-AUDIOUNIT-IMPLEMENTATION-543.md).
The historical design below is retained; its proposed automated default-device
helper is superseded by the manual-only check in that implementation note.

## Decision and evidence

Use an output-only Apple **DefaultOutput AudioUnit**
(`kAudioUnitType_Output`, `kAudioUnitSubType_DefaultOutput`,
`kAudioUnitManufacturer_Apple`). SDL remains responsible for everything else.
Keep the existing asynchronous guest-production workgroup and paced producer.
Replace the SDL playback device/AudioQueue consumer with the AudioUnit's
render callback. Initially select the sink explicitly, in one executable,
with `PETARI_AUDIO_SINK=sdl|output-au`; keep SDL as the default until reviewed
acceptance and a separate promotion decision. Never fall back silently.

The SDK's AUComponent.h describes DefaultOutput as AUHAL specialized to follow
the user's default output. It avoids implementing a raw IOProc or a second
device-selection policy. We will still monitor the selected device and rebuild
our format-dependent state on changes. We will not select the alerts/system
output, enable input, open a microphone, or change a device's nominal rate,
global buffer size, volume or default selection in ordinary operation.
[Apple output-unit overview](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/CoreAudioOverview/SystemAudioUnits/SystemAudioUnits.html)

The reason for this change is specific. In observer-neutrality-r3,
p2-on-second had five positive reports totaling **30,527 source frames**.
Its longest consecutive AQ-exit → AQ-entry gaps were 56.489, 73.933,
364.762, 201.552 and 103.450 ms. The ring was full at the highlighted arrivals;
callback bodies were under 0.060 ms and SDL lock waits under 0.001 ms.
Production continued at approximately 1 ms. Both guest/DSP members were joined;
the audio guest's maximum measured assigned-to-resume delay was 9.358 ms,
with no observed 175 ms event. This localizes the dominant observed delay
before AQ entry, rather than to the SDL locks or Petari fill. It does **not**
prove a particular CoreAudio/CFRunLoop or host-scheduling cause. DefaultOutput
removes that AQ/CFRunLoop dispatch stage; it cannot guarantee against all host
stalls or remove the guest producer's dependency on the CPU baton.

Evidence: `port53/m1g/campaigns/sola-543-aq-c127-r1/observer-neutrality-r3/`
`review-r1/{REPORT.md,P2-ATTRIBUTION.json,VERDICT.json}`. Do not substitute the
earlier nominal wall-clock context analysis for the ordered counter ledger.

## Data path and conversion

```text
guest JAS + DSP renderer (existing asynchronous workgroup / guest baton)
  -> existing AI DMA / paced producer (guest S16 stereo, 32 or 48 kHz)
  -> producer-owned AudioConverter + small bounded staging
  -> ONE SPSC ring (Float32, negotiated device rate / client channels)
  -> Petari AudioUnit input render callback (copy or zero-fill only)
  -> DefaultOutput -> selected output device
```

**SRC belongs to the producer, not to the render callback.** The producer uses
a persistent `AudioConverterRef` and `AudioConverterFillComplexBuffer` to
convert guest S16 stereo to non-interleaved Float32 at the actual device rate.
Configure a two-channel client input on bus 0 where supported; explicitly
negotiate/test mono downmix if the selected output cannot support that client
layout. Do not force Bluetooth into a microphone/telephony mode. Query rates,
layouts and accepted formats; 44.1/48 kHz are common cases, not assumptions.

This choice keeps our callback independent of SRC work, converter locks and
converter initialization. AUHAL's PCM conversion must not be assumed to perform
arbitrary SRC: Apple's AUHAL note specifically separates rate conversion from
simple PCM conversion. DefaultOutput can perform the remaining client/device
layout/PCM transformation; we do not depend on it for guest-rate SRC.
[Apple TN2091, audio formats](https://developer.apple.com/library/archive/technotes/tn2091/_index.html)

Create/configure/prewarm the converter off the render thread. Start with normal
SRC quality/complexity, query its prime information, and record the setting and
delay. No mastering-quality change or unmeasured quality shortcut. Converter
input comes only from already-budgeted producer staging: it must never pull
extra AI samples to satisfy a converter request. Temporary input exhaustion
returns a distinct temporary-no-data status with zero input packets; preserve
any output returned with that status and resume the same converter. It is not
EOF, and must not flush or reset history on every tick. True termination and
generation cancellation are separate states.
[Apple converter streaming semantics](https://developer.apple.com/documentation/audiotoolbox/encoding-and-decoding-audio)

Keep the current 1 ms guest producer granularity, 5 ms maximum timing credit,
1.05 normal / 1.25 low-water catch-up factors and 48 ms base cushion. Convert
ring occupancy and actual device demand to guest-time equivalents for the
existing pacing policy. Account for unconsumed source staging, converter-held
input/output and output already staged, so none creates a second hidden
prebuffer or permits extra AI advancement. Reserve enough output capacity
before supplying source samples; never overwrite/discard pending converted
output because the ring is full. Fractional rate accounting must persist
across ticks (44.1/32 is not rounded independently per callback).

The ring target is base time plus the **actual** largest device request, as in
today's policy. Its allocated capacity may cover MaximumFramesPerSlice; its
target must not be inflated to that capacity. For example, a 4096-frame safety
allocation is not permission to queue 4096 additional frames. Registration
wait remains the existing formula `clamp(48ms - request_ms - 3ms, 0, 40ms)`;
log any change in the derived value caused by a different actual I/O quantum.
Do not change the global device quantum to make a result pass.

Production still runs on its current host thread and calls AI::pull there.
The render callback **never** calls AI::pull, executes guest code, enters the
CPU baton, joins a workgroup, or promotes a thread. The guest workgroup's
membership remains tied to actual AI caller/HostThread generations, not a
priority number. Keep that source session across an output-device-only reopen;
do not mistakenly bind its guest identity to the new audio control thread.
If the >=175 ms assigned-unresumed case returns, stop and report the baton
dependency as an independent unresolved defect, rather than elevating every
game holder to real time.

## Render callback contract and proof

Use a C/C++ callback in a separate small translation unit, with an immutable
generation context supplied as refCon. The normal callback has only bounded
buffer validation, lock-free SPSC loads/stores, bounded copies/zero fills,
mute/generation flag reads and lock-free counters. No lazy initialization,
dynamic containers, shared_ptr destruction, thread_local constructors,
HostAllocationScope, SDL, AudioConverter, AudioUnit property calls, logging,
file capture, condition variables, Mach calls, QoS changes or guest hooks.
Use preallocated planar storage when AU supplies null buffer pointers; validate
all buffer counts/byte sizes and the configured maximum. An over-limit or
malformed call publishes a bounded fault and requests control-thread recovery;
it never allocates a larger buffer, walks beyond a provided buffer, or reports
an unfilled request as successful audio. Honor silence flags and the existing
background-smoke mute without disabling real production or the device.

Each ring and callback aggregate uses atomic types proven always lock-free
on the supported targets. Single consumer/producer invariants are tested;
reentrant callbacks fail closed without waiting. Pre-touch context, rings and
scratch buffers before start. No application render-time allocation, lock or
syscall is permitted, even on first call, shortfall, mute or error branches.
This contract is for **our callback and its reachable helpers**; it is not a
claim to have proved the internals of Apple's proprietary output unit/kernel.
[Apple real-time rendering guidance](https://developer.apple.com/documentation/avfaudio/creating-custom-audio-effects)

Required proof before measurement:

1. Inspect Release disassembly of the actual callback and every reachable
   helper, including ring wrap, error, mute and observer branches. Retain the
   executable/dSYM, addresses and allowlisted callees. Reject allocator/runtime
   initialization, mutex/condition/semaphore, syscall/Mach trap, unbounded retry
   or a TLS resolver on that path. Inspect linked copy/zero primitives too.
2. A callback test hook invokes this **same production callback** under a
   preinstalled test-only RT guard. Interpose/count prohibited allocation,
   free, mutex/wait, file, clock/trap-facing and framework entry points; a
   prohibited call fails the test. Preallocate the guard before entering it.
   Test first call, wrap, full/empty/partial reads, null AU storage, odd request
   lengths, mute, invalid/max+1 requests, generation retirement and observer
   trigger/wrap. No test-only alternate rendering implementation.
3. Static call-graph checks and the dynamic guard complement each other; symbol
   interposition alone does not prove absence of direct syscalls. The linked
   production codegen and actual clock implementation are required evidence.

## Cheap history and honest audio health

Preserve r3's bounded trigger history and producer/guest assignment-resume
records. Add distinct `au_render_entry/exit`, generation, render ticket,
requested/copied/zeroed device frames, ring-before/after, AU sample-time and
host-time validity flags, and converter source-consumed/output-produced
records. Do not invent AQ/SDL boundaries on the new path. Keep the legacy AQ
records unchanged for SDL control arms. Sequence/counter conservation, drops,
clipping, instability and retention checks remain fail-closed.

The existing `noteTransport`/`sampleHost` callback helpers are **not reusable**
unchanged: they perform thread-info/scheduling queries and first-use TLS work.
Add a bounded RT publication entry with only preallocated atomic writes. Host
policy/CPU/run-state readbacks and formatting stay on the reporter/control
thread, with sample times, errors and identity validity; do not claim a later
sample describes placement during a missing callback. Callback identity may
be a verified userspace pthread token, never a Mach query in the callback.

Actual entry/exit timestamps require a verified userspace monotonic clock
read (proposed mach_absolute_time fast path, initialized outside rendering,
with timebase conversion off-thread). Verify the **running OS implementation**
has no trap/lock/lazy initialization; this is a codegen gate, not an assumption
from the API name. If it cannot be proved, stop for review of a different clock
scheme. AU-provided mHostTime is separately tagged presentation/timeline data,
not mislabeled actual callback entry. Store paired Mach/steady anchors outside
rendering, use the native monotonic domain, and explicitly split sleep epochs;
do not recreate r3's wall-clock drift error. Off means one cached flag and no
clock/trace TLS/helper work before it. This instrument needs its own codegen
and overhead qualification; ccd/e2/AQ verdicts do not transfer.

### PORT clock-contract amendment after the e766d161 codegen stop

There is ONE explicit exception to the prohibition on unbounded retries:
the **OS commpage mode-3 `mach_absolute_time` consistency re-read**, only while
the observer is enabled. On the inspected M1 macOS 15.1, the actual loaded
arm64e libsystem_kernel UUID is `7A22548F-CBA9-3F17-8BA4-488E9EC688E7`.
It loads the offset at image offset 0x12c0, reads the counter with MRS at
0x12c4, reloads at 0x12c8 and compares at 0x12cc, then branches from 0x12d0
to 0x12c0 if the offset changed. PORT explicitly approved this lock-free OS
consistency loop; it is not permission for an application spin-wait, another
retry loop, or a syscall. The mode-0 svc fallback is outside the exception.
The actual bytes, UUID, runtime mode and addresses are retained under
`m1g/campaigns/sola-543-output-au-e766-r1/codegen/`; no stall is attributed to
the consistency re-read merely because it exists.

At each sink generation's open, if observation is requested, the control
thread performs a fallible self-task read of the Apple Silicon commpage mode
byte at 0xfffffc090. ONLY a successful complete one-byte read of mode 3 permits
the callback clock. Other modes, partial/error reads and unsupported targets
publish `clock=unavailable` and leave the callback's observer flag false and
clock/publisher pointers null. Audio keeps rendering; that diagnostic arm
fails validation rather than silently claiming observer coverage. No commpage
or Mach probe occurs in the render callback, and the off path remains
clock-free. This address/mode interpretation is valid only with the mandatory
loaded-library codegen check on each measurement target; a new layout does
not inherit qualification from the numeric mode alone.

Tests cover accepted mode 3, modes 0/1/2/4/unknown, failed/partial probes,
and actual guarded rendering without clock/publisher calls when unavailable.
The actual system observer clock in the render fixture is exercised only
when its real startup probe permits it; otherwise the observer branch and
actual publisher use the fixture clock with that limitation printed. The
independent exact-app/OS codegen check still applies. The original binary's
refused result is preserved; a rebuilt app must be rechecked and may receive
`passed=true` only with this named exception and the implemented mode gate.

Keep `[audio]` report count and **zero**-underrun acceptance. Publish exact
device-frame shortfalls, plus explicitly labeled source-equivalent accounting
with a persistent fractional remainder and a nonzero final remainder exposed
at shutdown. Never label device frames as 32 kHz ring frames or round small
shortfalls away. The common functional evaluator requires zero raw shortfalls
as well as the existing >=50 real-audio reports/zero-underrun policy. Changing
units/adding provenance must not relax the predicate or hide startup/tail data.

Also detect AU sample-timeline discontinuities within a running generation,
render errors and device overload indications where obtainable. A callback
can fail to run while the ring remains full: ring-shortfall zero alone must
not certify that case. Record expected/actual valid sample times and requested
frame counts; gaps/duplicates/backward steps are distinct health failures in
uninterrupted acceptance arms. Invalid timestamps are reported explicitly,
with no fabricated continuous timeline. A >20 ms arrival gap triggers history,
but is not automatically a missing-frame count (device cadence can vary).
Intentional device transitions have a separate measured outage record, never
a reset that erases an earlier audio failure.

## Lifecycle, device changes and recovery

One owned non-render control thread serializes create/configure/start,
listeners, stop/dispose, converter/ring reset and reporter lifetime. Initial
sink start retains the current host-blocking/CPU-baton release around slow
device operations. Keep the existing install/start/stop/active/mute/submitted
facade so app and tape glue need no unrelated behavior change. SDL's existing
subsystem bootstrap can remain; the AU branch opens **no SDL playback device**.

State machine:

```text
Stopped -> Opening -> Ready -> Running
Running -> ReconfigurePending -> Quiescing -> Opening
Opening/recovery failure -> Recovering (bounded retry) -> Running or Failed
any state -> Stopping -> Stopped
```

Open DefaultOutput, enable output bus 0 with input disabled, query its current
device, hardware/client format, accepted channels and MaximumFramesPerSlice,
configure the input render callback, allocate/prefault all buffers, initialize,
read back settings, prime the normal cushion and start. Read back the device
again around start; a racing default/format change invalidates that attempt.
MaximumFramesPerSlice and format listeners cause off-thread reassessment.
[Apple maximum-frame guidance](https://developer.apple.com/library/archive/qa/qa1533/_index.html)

Listen for default-output changes, current-device changes, device alive,
rate/stream-layout/buffer-size changes, and unit maximum-frame changes. Include
sleep/wake and audio-service/device invalidation. Listener callbacks only set
a generation-tagged atomic request; they never reopen, allocate, log or block.
Use a control-thread timed wait/poll (and normal non-RT event notification for
power events), so no render callback has to signal a semaphore or dispatcher.
Coalesce duplicate requests, then re-read current properties on the control
thread; a listener payload is not an authoritative final format.

On reconfigure, atomically close the generation's consume gate and unpublish
its data pointer from a stable callback-owner context. Callbacks register
in-flight **before** loading that pointer; callbacks arriving after unpublish
zero-fill without touching retired buffers. Stop the unit, wait for in-flight
readers, then uninitialize/dispose successfully before reclaiming the owner
context. A momentary zero reader count alone is not proof that a live unit
cannot call again. Validate this lifetime protocol against the API and race
fixtures before shipping; no callback refCon may refer to a freed generation.
Quiescence is waited for only by control, never by render. Tests cover a callback racing stop,
listeners racing disposal, repeated stop, default changes during opening, and
old-generation events after a replacement starts. A timeout retains the old
context and fails explicitly; it must not free live memory to continue.

Pause/join the old producer outside rendering before resetting converter/ring;
discard old-device queued PCM only as a recorded transition discontinuity.
Reopen with fresh negotiated geometry/converter state and the normal cushion,
then resume the same guest DMA stream/workgroup identity. Do not replay stale
PCM, globally reset AI/DSP, or keep an old-rate ring on a new-rate device.
Initial and recovery priming/silence counts are explicit.

If no device exists, report `Recovering/device-unavailable`, health=false and
wait for an actual available default. If a present, stable device cannot open,
use bounded retries (100/250/500/1000/2000 ms) and surface `Failed` with OSStatus
and an actionable retry/error through the non-RT app path. It cannot remain
apparently healthy but silent. Device-available-to-real-callback recovery is
measured; propose <=5 s for wired/built-in and <=15 s for an already connected
Bluetooth device, subject to PORT review. Bluetooth connection time itself is
reported separately. Sleep preserves an explicit epoch break, not a false
continuous latency statistic. No silent SDL fallback or automatic pass.

### Device-switch testing without privileges

Use public CoreAudio enumeration, default-output readback,
AudioObjectIsPropertySettable and AudioObjectSetPropertyData on
kAudioHardwarePropertyDefaultOutputDevice in a dedicated test helper. This
requires no input capture and is the proposed ordinary-user route; permission
and hardware availability remain **unverified on this machine**, since this
design performs no device switch. Do not promise a scriptable second device
from the existence of the API alone.
[Apple default-output setter](https://developer.apple.com/documentation/coreaudio/audiohardwaresystem/setdefaultoutputdevice(_:))

In the device-change test slot, enumerate already-connected eligible outputs,
retain the original UID, switch A -> B -> A during real output, require property
and render-generation/format readbacks plus renewed non-silent known-signal
output after each switch, and restore the original default by UID on success
or failure. Do not change default input, connect Bluetooth programmatically,
install a driver, use sudo, request TCC, or restart coreaudiod. Report a blocked
setter or fewer than two devices as **not tested**, not passed. Switching is
only in this predeclared isolated test, never in the acceptance timing arms.

Manual fallback/additions: built-in -> plugged headphones -> AirPods -> built-in,
rate/mono changes where the device exposes them, unplug active output,
sleep/wake, repeated rapid switches and quit during reconfiguration. Record
device UIDs/rates/generations, audible recovery, outage lengths, errors, resource
lifetime and final default restoration. Fake-device lifecycle tests cover all
states without requiring physical hardware; they do not replace these checks.

## Latency — before and after, without hidden buffering

At start and every device-generation change, record individually validated
CoreAudio values (frames, rate, units, query errors): device latency, each used
stream's latency, safety offset, actual buffer frame size, plus AU latency and
converter prime/delay. Report used streams individually and use the relevant
maximum path, not the sum of parallel streams or an arbitrary first stream.
Unknown properties remain unknown, not zero. Device plus stream latency is
additive; AU-reported values that encompass a stage must not be double-counted.
[Apple stream-latency semantics](https://developer.apple.com/documentation/coreaudio/audiohardwarestream/latency)

For the existing measured AQ geometry, queue capacity is
6*256/44100 = **34.830 ms**; the live target is
1722/32000 = **53.813 ms** (48 ms base plus largest request186). Their sum is
**88.643 ms before hardware/other conversion components**. This is capacity / a
budget estimate, not an acoustic latency measurement or an assertion that all
queued buffers always delay a sample by their full capacity.

The AU path removes the six-buffer queue. Preserve the 48 ms cushion; record
actual request-derived target, converter-held samples and all hardware stages.
With the same 186-source-frame equivalent request its target remains 53.813 ms;
the nominal removed budget is 34.830 ms, before new converter delay. A larger
actual AU quantum changes this calculation and must be reported, not hidden by
using the old AQ number. No additional latency is authorized merely to pass
zero-underrun tests.

Measure two things on matched device/rate/buffer/volume runs:

* **Reported hardware/processing latency**, broken out as above for SDL and AU.
* **End-to-end estimate**: source sample enqueue/AI block generation -> render
  consumption, linked by source/device sample counters and converter phase ->
  AU presentation host timestamp plus documented remaining downstream latency.
  Calibrate converter impulse delay offline. Report median/p95/max and bounds
  for queue position, unknown hardware/Bluetooth buffering and clock mapping.
  This is an estimate to DAC/presentation, not a measured acoustic result.

For SDL, retain the AQ enqueue/ticket/queue-position estimate and its uncertainty;
an AQ callback timestamp is not a hardware presentation time. Use comparable
component estimates and optional manual external loopback if available, rather
than claiming unequal timestamp semantics are an exact before/after measurement.
No microphone/TCC-dependent measurement is required for the scripted gate.
Acceptance requires no increase in comparable reported/estimated latency,
including startup. If intervals/unknowns prevent that conclusion, latency is
**unresolved** and the sink is not ready to promote.

## Implementation scope, tests and reviewed stops

Proposed small commits, only after this design is approved:

1. Native PCM/rate ledger, device-rate SPSC ring and producer-converter tests;
   preserve guest PCM and pacing behavior with byte-exact source captures.
2. DefaultOutput control/render implementation and fake-device lifecycle tests;
   retain the public sink facade and explicit backend selection. Link
   AudioToolbox/AudioUnit/CoreAudio in the native audio target. SDL remains
   pinned; no new SDL patch or root src/include/libs changes.
3. RT-safe bounded history, audio/latency reports and fail-closed validation;
   production callback/off-probe tests and source/codegen checks. No unrelated
   app/tape/glue edits; coordinate any required app error surface with PORT/B.

Server tests: identity PCM, 32/48 kHz guest to 44.1/48/96 kHz output plus mono
and low-rate Bluetooth-like formats; exact frame conservation with converter
prime/tail accounting; finite flush vs temporary starvation; chunk-invariant
SRC, impulse/DC/tone/channel-order/clipping checks; long fractional-rate/drift
simulation; variable callback sizes, capacity
limits and ring wrap; no extra AI advancement when output is full; producer
stalls and late DMA registration; mute; lifetime/error-injection/device races;
first-call RT guard; history wrap/concurrent publication/loss; report-unit and
zero-shortfall checks. Live unit tests keep assert enabled. Retain test exes
and disassembly. Run existing audio/DSP/pacing/boot/present, trace/history,
workgroup and tape fixtures; separately report unrelated baseline fixture rot.

Measurement plan **proposed for review, not authorized to launch by this note**:

1. Exact server app/dSYM/test provenance, M1 weak-import and Release codegen /
   RT guard PASS. Pin a single source/derived executable and resources before
   any arm; a diagnostic signing derivative needs its own authorized binding.
2. This callback observer's own overhead: six alternating history OFF/ON warm
   pairs, 12 arms, prime315/fixed300 + second315/fixed300, history on both
   launches of ON pairs, AU sink on every arm. Real audio/DIAG/WG=async; fixed
   same-mode geometry and exact cache handoffs. Report pacing distributions,
   callback/ring/timeline faults, positive arms/reports/frames and event rate.
   Zero ON positives alone proves neither neutrality nor suppression.
   **STOP for review**, no reuse of earlier observer qualification.
3. After that review, four long title pairs, **SDL / AU / SDL / AU**, each a
   same-sink prime315/fixed300 and second870/**fixed855** (8 arms). History on
   both launches of all pairs. Same physical output/rate, original cushion,
   real CoreAudio, DIAG and WG=async; no input or AQ-count override. Preflight
   each duration against the 870 parser bound and independent 900 cap. Keep
   full-run counters including startup/tail. Compare second positive arms/2,
   positive reports, exact raw frames per rate and source-equivalent frames;
   normalize descriptive rates per855 s by actual printed report exposure,
   with the same rounding/boundary caveat as r3. Primes separate. New-sink
   acceptance requires >=50 real reports, zero ring shortfalls, no render
   timeline/device errors and no stuck-silent recovery. A positive remains
   FAIL; continue only the remaining predeclared arms unless another stop
   fault occurs. **STOP for review**, no reruns until clean.
4. On passing review, device-switch/manual recovery validation and matched
   M4 SDL/AU audio/pacing/latency runs with the same four-pair 315/870 shape.
   Compare p99/post100 p99, >50 ms, max, game/retrace/draw lanes and audio
   health. No M4 regression required; immutable baseline positives retained.
5. Only then a server freeze candidate integrating accepted native changes:
   fresh full functional gates, including normal3/full-star3 real audio under
   ea7dc9d3, intro/pointer, Cappy, latency, image/warm/audio/seeds/macOS checks,
   required source/Wii proof and collector validation. No auto-promotion.

All arms nice0, detached, per-arm heavy -> native, unchanged quiet guard,
private perfm1a/cache/Metal namespace, exact handoffs and coreaudiod CPU samples;
camera background retained, no daemon restart or default user-cache access.
Predeclare all commands/stops; preserve positives. Stop on binding, process,
incomplete window, audio backend/report missing, converter/workgroup/API fault,
history loss/unknown retention/clipping, or >=175 ms audio-guest assigned-resume
delay. M1 copy/I/O/build coordination with PORT/B precedes launch. K/R waits,
AQ6/18 masking and further traced cohorts do not run automatically.
