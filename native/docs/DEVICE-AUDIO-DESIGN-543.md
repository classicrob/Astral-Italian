# Device-side audio investigation: design for PORT review

Status: PORT approved the history repair, default-off SDL boundary localization,
and a default-off count-only diagnostic switch after reviewing 88b587af.
Those source changes are now implemented; full server build/review, Release
codegen and this instrument's own overhead qualification remain pending.
No new app/audio run has occurred. The stopped 2293cdfe cohort stays failed.
The count-only long cohort still waits for build/overhead review; AUHAL is deferred.
Implementation contract: [AUDIO-TRANSPORT-DIAG-543.md](AUDIO-TRANSPORT-DIAG-543.md).

## What the pinned code actually does

The pin is SDL **release-3.4.10** (native/cmake/PadDependencies.cmake:10).
The retained local SDL_coreaudio.m SHA256 is
c1ed4d18205056f615ae3f7098b53cd69ccbb939e1e8f212f029c0e8c1cc44af,
byte-equal to the official tagged file retrieved for this review.

SDL creates its own in-process thread in COREAUDIO_OpenDevice (:985–986).
AudioQueueThreadEntry (:857) calls PrepareAudioQueue on that thread;
AudioQueueNewOutput (:728) receives CFRunLoopGetCurrent and kCFRunLoopDefaultMode.
The same thread runs CFRunLoopRunInMode (:878). Its PlaybackBufferReadyCallback
(:565) calls SDL_PlaybackAudioThreadIterate.

Thus the application callback runs on an **SDL-owned, in-process CFRunLoop
thread**, not the HAL IOProc thread, and not an automatically selected private
AudioQueue thread. Apple's API explicitly dispatches to the supplied run loop;
the internal-thread case applies when that parameter is null.
[Apple AudioQueueNewOutput](https://developer.apple.com/documentation/audiotoolbox/audioqueuenewoutput(_:_:_:_:_:_:_:))

Core Audio services can involve coreaudiod, but this source does not establish
that the callback body executes in that daemon. A high daemon CPU percentage
does not by itself identify why our in-process thread failed to call fill.

There is a second boundary distinction. Our existing “[audio-host] device
worst gap” is measured at **Petari's fill callback**, downstream of:

1. AudioQueue buffer-ready dispatch on SDL's CFRunLoop;
2. SDL_PlaybackAudioThreadIterate's device mutex (SDL_audio.c:1176);
3. SDL_GetAudioStreamDataAdjustGain's stream mutex and request/conversion path
   (SDL_audiocvt.c:1337–1393).

Our 51–61 ms gaps therefore are not independently measured raw AudioQueue
callback-entry gaps. Run-loop dispatch, SDL mutex contention, conversion/request
batching and host scheduling remain distinguishable hypotheses.

### Existing thread evidence and its limits

All four 2293 arms reported successful Mach time-constraint promotion from
inside fill. That is the existing promotion call's success flag, not ongoing
policy readback. The new workgroup history has pthread identity but no callback
QoS, sched-priority or CPU-number fields. The positive arms had history disabled.

The only current-cohort callback identity retained in history is the first
ON trigger: pthread **0x316dbf000**, seq **51532**, kind **6**, at observer
+25.101534 s, 22.076 ms fill-entry gap, ring level **1722**, request **186**.
Its QoS, sched priority and CPU are **unknown**. The earlier monitor's QoS33,
priority31 and P-core placement must not be reassigned to this audio thread.
The old de3 trace was invalidated by drops and is not qualification evidence.

The approved diagnostic adds AQ entry/exit, SDL device-lock wait, stream-lock
wait, get/conversion, fill entry/exit and actual put boundaries on steady_ns,
with callback pthread/Mach/QoS/priority/CPU readbacks at each AQ entry. AQ exit
bounds requeue work; it is not a separately timed HAL hardware consumption.
The exact local SDL patch and bounded-history contract are documented in the
implementation note. No mutex firehose, per-call formatting or RT allocation.

Further source correction: our producer **already uses a lock-free SPSC ring**.
`produce()` calls AI::pull and writes that ring. It does not call SDL. The
actual `SDL_PutAudioStreamData` is inside `fill()`, on the AQ callback thread.
SDL invokes fill with the stream mutex held, and the put acquires the same
recursive SDL mutex. The trace therefore labels it callback-side put, not a
producer-held SDL lock. A producer-to-SDL SPSC change would duplicate an
existing handoff; another thread's stream contention remains a measured question.

## Retained underruns and daemon CPU correlation

Every completed arm had both actual guest JAS and DSP renderer members joined,
adopted=1, API/ledger errors=0 and interval late=0. These readbacks do not cover
guest work before a DSP interval or later device consumption.

| Arm / report time | Missing 32k ring frames | Fill max gap | Report ring / target | Latest / oldest unanswered registration |
|---|---:|---:|---:|---:|
| p1-off-second / 58.090s | 120 | 51.258ms | 677 / 1722 | 4.754 / 4.754ms |
| p2-on-prime / 210.928s | 220 | 60.584ms | 1707 / 1722 | 44.229 / 44.229ms |
| p2-on-prime / 264.524s | 208 | 38.046ms | 1417 / 1722 | 12.903 / 177.907ms |

The third window had eight AI replays and up to nine raises per registration.
The 177.907 ms registration age is not a measured assigned-but-unresumed gap.
History was OFF in these positive arms, so no conclusion about the 175 ms
baton discriminator is available. The short ON arm observed only 27 s, max
assigned-to-resume 73us, before its diagnostic failure.

The per-five-second coreaudiod samples overlap those approximate report windows:

| Positive report | Covering CPU sample interval, relative to spawn | CPU-time delta / elapsed | End-sample ps %CPU | Same-arm interval median |
|---|---|---:|---:|---:|
| 58.090s | 53.149–58.559s | 12.33% | 10.6% | 12.56% |
| 210.928s | 208.511–213.947s | 11.96% | 9.5% | 13.43% |
| 264.524s | 262.553–267.972s | 11.56% | 12.5% | 13.43% |

These samples show no five-second daemon CPU spike at the positive windows.
They cannot resolve a 50 ms stall, identify scheduling latency, or exclude
daemon/driver service delays. Audio report process time uses kernel birth time
and gettimeofday; joins use the retained Popen wall-clock bracket. Sampling
itself spans pgrep/ps. This is coarse correlation, not an exact event join.

Whole-arm derived daemon CPU was 12.90%, 12.48%, 12.79%, 11.06%;
the corresponding ps peaks were 23.1%, 18.2%, 17.5%, 14.5%. PID1063 persisted,
with no sampling errors or restart. PORT/head's M4 77–96% no-game observation
has no synchronized interval dataset here; no M1-versus-M4 causal comparison
is claimed.

A read-only ordinary-user Core Audio property query succeeded, without sudo,
a tap, a stream, an input-device open or TCC request. The current snapshot has
**34 connected client-process objects excluding the query process, zero with
active output**. These are process counts, not AudioQueue/stream counts.
Historical arm client counts and actual stream counts are **unknown**.
The public selectors are kAudioHardwarePropertyProcessObjectList,
kAudioProcessPropertyPID and kAudioProcessPropertyIsRunningOutput, documented
in the SDK AudioHardware.h. This present-day snapshot is not backfilled into
the completed arm receipts.

Evidence: stopped cohort review-stopped-r1/design-r1/
COREAUDIOD-UNDERRUN-CORRELATION.json, CURRENT-CLIENT-PROCESSES.json and
SDL-SOURCE-BINDING.json under port53/m1g/campaigns/sola-543-async-wg-2293-r1.

## Existing SDL alternatives

The pinned macOS CoreAudio backend always constructs AudioQueue. Its driver
registration is “coreaudio”; the CMake CoreAudio option enables/disables that
backend, rather than selecting HAL versus AudioQueue. Searches of its backend,
driver registrations and hints found **no selectable AUHAL / HAL IOProc variant**.
Disk/dummy drivers do not exercise real output and are inapplicable.

SDL_AUDIO_DEVICE_SAMPLE_FRAMES is available, but SDL is allowed to choose a
different actual size. Petari already exposes PETARI_AUDIO_DEVICE_FRAMES and
sets that hint before opening. The actual format and frame count must be
read back, never assumed.
[SDL sample-frame hint](https://wiki.libsdl.org/SDL3/SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES)

The pinned queue count is hard-coded in PrepareAudioQueue (:816–830):
three buffers, or ceil(15ms / buffer_duration) * 2 for small buffers. There is
no queue-count hint in that code.

## Proposed device-buffer-only discriminator — diagnostic, not a fix

At the observed 44.1kHz device rate, the baseline is 256 frames × 6 AQ buffers:
**34.830ms of queue capacity**. The 1722-frame 32kHz source ring target is
**53.813ms**. These are different-rate storage budgets, not independent
guarantees of scheduler tolerance or measured end-to-end latency.

Simply requesting 1024 frames is **not** an isolated device-buffer arm:

- queue capacity becomes 1024 × 3 / 44100 = 69.660ms;
- the source request grows from about186 to744 frames;
- PacedRing target rises from1722 to2280 frames, adding17.438ms of ring storage;
- the registration wait formula changes from39.195ms to21.780ms.

Those changes confound producer behavior. Do not call this device-only.

Instead propose a default-off, diagnostic-only patch to the pinned SDL backend
that overrides only numAudioBuffers after its original calculation: baseline6
versus18, keeping the actual device quantum256 and the physical device/rate
unchanged. Queue capacity becomes **104.490ms**, a **69.660ms increase** in
potential queued latency. It also increases startup silence/priming. At four
or eight bytes per device frame the extra storage is12 or24KiB. None of this
is proposed for release.

Both arms must read back/log actual count, allocation bytes, device rate and
quantum; require the unchanged ring target1722, largest request186, source
rate32000, registration wait39.195ms, pacing constants, workgroup mode and
callback policy. A changed device or request geometry refuses comparability.

Proposed fixed cohort for approval: four900s warm title seconds, queue-count
6 / 18 / 6 / 18, each with its own315s same-case prime; observe915 and fixed900,
nice0, heavy→native, quiet guard, real CoreAudio, fixed WG=async, cheap verified
history on all seconds, daemon CPU/client-process samples on control threads.
Pin one executable and predeclare every command. Keep every positive. Stop on
binding/process/protocol/history faults or a measured >=175ms baton gap; do not
add replacement arms or rerun until clean. STOP for review afterward.

Disappearance of underruns would support buffering/slack sensitivity, **not
prove callback scheduling alone**: queued audio can also conceal production
delays. Distinct AQ/lock/fill/requeue boundaries and producer timing are needed
to choose between dispatch, locking, request batching and production.

## Structural direction if the transport hypothesis survives

Prefer an opt-in **output-only AUHAL sink**, with direct AudioDeviceIOProc as
a lower-level alternative. This removes the AudioQueue/CFRunLoop buffer-ready
stage; it does not bypass coreaudiod or guarantee the OS never stalls.
[Apple AUHAL overview](https://developer.apple.com/library/archive/technotes/tn2091/_index.html)
[Apple AudioDeviceIOProc](https://developer.apple.com/documentation/coreaudio/audiodeviceioproc)

Control-thread lifecycle:

- Create kAudioUnitSubType_HALOutput, enable output bus0 and leave input bus1
  disabled. Read the default output device, bind CurrentDevice, read actual
  format/maximum frames and install an output render callback.
- Do not change the user's nominal device rate, global buffer size, volume or
  defaults. A device listener posts a generation-tagged reconfiguration request.
- Preallocate conversion state, rings and maximum callback storage before start.
  Output stop waits for callback quiescence on the control thread before freeing
  the old generation; no callback allocation, lock, wait or guest execution.
- On device loss/change or initialization failure, report the failure explicitly.
  Stop/reconfigure outside the callback; never silently substitute another sink
  and pass the same arm. Shutdown removes listeners and joins only owned threads.

Replace SDL_OpenAudioDeviceStream/Resume/Destroy and the playback fill/requeue
transport in native/audio/sdl_sink.cpp; SDL remains for the window and input.
Keep the guest AI/DSP producer, semantic sample rate, DSP waveform, workgroup
roles and public sink start/stop/mute contract. The HAL callback only drains
preallocated device-format data and zero-fills actual deficits. It must never
call AI::pull or enter the game baton: that could block the RT callback behind
the historical assigned-unresumed holder.

AUHAL does not remove the need to design sample-rate conversion. Use a single
producer-owned converter outside the IO callback, potentially SDL_AudioStream
as an offline converter with no SDL device, and one lock-free device-format
output ring. Avoid adding a second full prebuffer. Account for source/device
frame conversion and demand feedback so guest timing remains source-clocked;
measure latency rather than hiding failures with more buffering. A new
converter or feedback bridge is a material functional risk and needs review.

Risks include 32/48k guest audio versus44.1/48k devices, planar/interleaved PCM,
channel maps, maximum-frame changes, drift, device migration, teardown races,
mute behavior and end-to-end latency. AUHAL's format conversion should not be
assumed to provide arbitrary sample-rate conversion. The callback's OS-managed
IO scheduling does not authorize blanket RT promotion of game threads.

Acceptance plan for a later reviewed implementation: native lifecycle and
rate/layout/conservation tests; exact source PCM/DSP and long drift checks;
default-off codegen and its own overhead; predeclared long M1 SDL-versus-HAL
arms including title and Yoshi, all positives retained; paired M4 pacing/audio;
then integrated normal/full-star >=50 real reports/zero underruns plus the
functional suite. Exact arm count/order requires PORT approval before launch.
The existing failed WG overhead does not qualify any new instrument or sink.
