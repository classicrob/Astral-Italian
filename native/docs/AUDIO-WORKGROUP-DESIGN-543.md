# 5.4.3 audio scheduling: design for PORT review

Status: design only, 2026-10-25. Source inspection is against de3dfa77.
No workgroup, scheduling, buffer, callback, or gameplay change has been made.
The unfinished mutex flight recorder is preserved separately as uncommitted
work; it is not part of this proposal or this document's commit.

## Recommendation and the API constraint

Evaluate workgroup scheduling before K/R timer changes. However, do not simply
join the current producer chain to the device workgroup: its cadence is
asynchronous to the device's current I/O cycle. Apple distinguishes that case
explicitly from parallel rendering to the device deadline. The documented
mechanism for this architecture is a custom audio interval workgroup.

I recommend a staged **asynchronous-workgroup experiment**, retaining the ring
and the existing AI/DSP protocol, followed by the unchanged long audio gates.
This is a proposed source experiment, not a claim that workgroups fix the
underruns. Device-workgroup membership remains an alternative requiring a
different rendering contract, described below. PORT should review this
distinction before implementation.

Apple sources:

- [AudioWorkInterval.h](https://developer.apple.com/documentation/audiotoolbox/audioworkintervalcreate)
  and [asynchronous audio threads](https://developer.apple.com/documentation/audiotoolbox/adding-asynchronous-real-time-threads-to-audio-workgroups):
  independently paced workers use an application-created interval group.
- [Parallel audio threads](https://developer.apple.com/documentation/audiotoolbox/adding-parallel-real-time-threads-to-audio-workgroups):
  auxiliary work for the current device cycle joins the device group.
- [Device workgroup property](https://developer.apple.com/documentation/coreaudio/kaudiodevicepropertyiothreadosworkgroup)
  and the installed SDK's AudioHardware.h: the HAL returns an owned object.
- Installed SDK os/workgroup_object.h: join operates on the calling thread,
  returns a token; successful joins must be left on that thread in reverse
  order before exit. EALREADY and EINVAL are failures, not successful adoption.

These are scheduling contracts, not latency guarantees. In particular, joining
a workgroup does not grant the guest CPU baton or replace a mutex dependency.

## Current producer chain and identities

| Role | Source site at de3dfa77 | Work and current host scheduling | Proposed treatment |
| --- | --- | --- | --- |
| Ring producer | native/audio/sdl_sink.cpp:192-224 | 1 ms ticks; level-driven Pacer; copies registered AI blocks through Audio::pull; existing Mach RT request, UI QoS fallback | Its own asynchronous interval group; retain tick/rate/credit limits initially |
| AI interrupt dispatcher | native/platform/audio/ai_host.cpp:146 onward | Dispatch semaphore, counted DMA delivery, OSDisableInterrupts, guest DMA callback; existing Mach RT request | Explicit audio role; associate with the guest-production interval when contributing to that interval |
| Guest JAS audio thread | native/platform/os/os_thread.cpp:660-692; src/JSystem/JAudio2/JASAudioThread.cpp:34-101 (read only) | Handles DMA and DSP messages, updates DAC/voices and registers DMA; normally priority 2, host UI QoS only; needs baton | Identify actual registering thread, request RT scheduling for this role, join guest-production interval group |
| DSP renderer worker | native/platform/audio/dsp_device.cpp:327-344 | Mail queue, native DSP renderer; UI QoS, not currently Mach RT | Request RT for the bounded audio work; join guest-production group |
| DSP interrupt dispatcher | native/platform/audio/dsp_device.cpp:346-378 | Semaphore, interrupt mutex, guest DSP handler; existing Mach RT request | Join guest-production group while participating in its work |
| SDL/AudioQueue callback | native/audio/sdl_sink.cpp:227-262 | Drains ring, passes samples to SDL; existing Mach RT request | Keep bounded drain, do not manually nest a guessed device group |
| Reporter, main/FIFO, renderer, VI, generic OS threads | Various | Not audio synthesis workers | Do not blanket-promote or blanket-join |

Native AIRegisterDMACallback/AIStartDMA can bind the actual caller's OSThread
and HostThread generation to an audio role. The source path establishes that
JASAudioThread::run initializes AI and starts DMA. Use this explicit binding,
verified by subsequent AIInitDMA calls; do not classify every priority-2 thread
or infer a role from a symbol name. Test callbacks and headless sinks must not
silently become real-device audio roles. Runtime receipts record guest pointer,
host pthread/Mach identity, generation, callback identity, effective policy and
every role's adoption result. A reused OSThread address is a new generation.

The pipeline is:

    ring producer pulls AI -> AI raises DMA -> AI interrupt posts guest message
      -> guest audio thread obtains baton -> DAC/DSP work -> DSP worker
      -> DSP interrupt -> guest audio thread -> subsequent block registration

The DSP round trips mean promoting the ring copier alone is insufficient.
Likewise, a delayed device callback can starve independently of the producer.

## Device API evaluation: available HAL API, missing SDL identity bridge

The pinned SDL source actually creates AudioQueueNewOutput and explicitly
assigns a HAL device UID with kAudioQueueProperty_CurrentDevice. It does not
drive an AUHAL owned by our sink. Therefore kAudioOutputUnitProperty_OSWorkgroup
is not applicable to an existing output unit here.

kAudioDevicePropertyIOThreadOSWorkgroup is the applicable HAL property. Query
the actual AudioDeviceID, global scope/main element, with HasProperty, size and
OSStatus validation. Retain the returned os_workgroup_t until all members leave;
release it with the appropriate OS-object ownership API. Never cancel a
device-owned group.

Our pinned SDL public audio API exposes neither its AudioQueueRef nor its
resolved AudioDeviceID. SDL IDs are not HAL IDs. queryHalLatency currently asks
for the default device, which is useful diagnostic information but cannot
certify the opened queue's identity during routing/device changes. A name match
is also insufficient. Runtime property availability on built-in, aggregate,
Bluetooth and other outputs remains unmeasured; SDK availability is not proof
that a particular device returns a workgroup.

If PORT chooses a device-group experiment, propose a small, separately reviewed
patch through native/gx/sdl-patches, whose hash already participates in the
native SDL dependency identity. It must publish the resolved queue device UID,
HAL ID and generation from SDL's CoreAudio device lifecycle, and route changes,
without exporting a dangling queue pointer. Resolve/retain the device group
off the callback thread. Verify the queue UID against the HAL UID; races or
unknown identity disable adoption and fail the experiment's provenance check.
This bridge is a new dependency patch, not an API already exposed by the sink.
It is not needed for the recommended custom asynchronous groups.

No HAL queries, object creation/release, listener registration, formatting, or
heap allocation goes in the device callback or under interruptMutex.

## Proposed asynchronous work intervals and membership

Two independent cadences require separate groups:

1. **Host ring-copy group.** AudioWorkIntervalCreate with Mach absolute time;
   the existing producer is its primary and only member. Start/finish an
   interval around an actual producer tick, advertising the already planned
   next tick. A late tick is marked late, not assigned a deadline in the past.
   This is a scheduling hint, not a change to Pacer, pull count or DMA timing.
2. **Guest block-production group.** The explicitly bound guest audio thread
   controls intervals around successive DMA-production generations. Native
   AIInitDMA is the interception point; it must distinguish answering the
   preceding generation from opening the next one. Native DSP command/frame
   completion and AI raise/register generations establish which workers are
   contributing and when output is actually ready. Helpers join this group
   only if they share that block's production deadline; a different independent
   cycle must not be mislabeled as that work interval.

Before implementing the second group's interval start/finish calls, write a
fixture ledger for startup, steady DMA, all DSP subframe round trips, pause and
shutdown. It must prove one primary, no overlapping intervals, no premature
finish at an early AI registration while required DSP work is still pending,
and correct association of generated samples with the consuming block. If the
native generation mapping cannot establish that, stop and revise the interval
design rather than add synthetic start/finish calls on a 1 ms timer.

Deadlines come from the actual block size/rate and existing maximum Pacer
advance (currently 1.25x in low water), not an invented 500 us DSP deadline.
For example, a 560-frame/32 kHz block has 17.5 ms nominal audio time, 14 ms at
that maximum advance. Ring coverage and the existing elastic registration wait
are separate quantities. The fixture must derive the correct generation's
available budget, count missed deadlines, and never extend it after a miss to
make a receipt pass. Constants, buffering and logical interrupt order stay
unchanged in this experiment.

For each role: obtain a stable retained group generation off the RT path;
request/read back RT policy; then join from that thread, saving its own token.
The guest audio host currently is not RT, nor is the DSP renderer: those are
explicit scheduling changes, separately visible in evidence. Membership does
not itself promote a thread. Preserve previous scheduling state for teardown.
Other roles' current scheduling contracts are initially unchanged.

Join/leave and generation changes occur at host-only safe points, outside
interruptMutex, DSP queue locks and the AI registration lock. Guest control
wakes may service a host-only membership update while parked, but cannot run
guest instructions without the baton. Recheck the original baton predicate
after every such control wake. No timed-wait replacement or polling loop.

Stop: publish closing generation, stop new production, wake parked members to
leave on their own threads, finish/cancel application-owned intervals according
to their lifecycle, join producer/helper threads, and release the last group
reference only after membership acknowledgments. A guest thread that persists
past audio stop must leave before it resumes non-audio work; a terminating
guest must leave before pthread_exit. Teardown must not wait for an acknowledgment
while holding a lock that the member needs. Device callbacks are quiesced by
the sink lifecycle before their state is freed.

## Baton: the limitation a workgroup cannot remove

There are three distinct delays:

- Audio is ready while a lower-priority guest holder executes. Existing guest
  priorities, holder UI-QoS override, cooperative OS calls and safe forced
  preemption provide progress opportunities. They do not impose a hard bound
  when the holder is in unsafe/system code. A workgroup does not fix that.
- Audio is assigned the baton but has not resumed. The proposed RT scheduling
  and membership of the actual guest audio pthread target host scheduling of
  that recipient. They do not make interruptMutex fair or prove the thread can
  reacquire it promptly. The retained ~191 ms case is in this category; lost
  wakeup was refuted, scheduling versus mutex reacquisition remains unresolved.
- Audio resumes but waits for DSP, block registration or a guest resource. Join
  the real bounded audio workers; keep actual guest queue/priority semantics.

**Nothing in today's baton guarantees that a workgroup-joined producer cannot
be parked behind another holder.** This proposal therefore cannot honestly be
called a complete fix before the baton and long-run evidence. In particular,
do not join arbitrary baton holders to the audio group or turn all guest/game
work into RT work to hide that dependency. Do not run JAS concurrently with a
holder. No spin waits, force-unlock, unsafe suspension or relaxed replay checks.

Retain the existing release of the baton around host waits; audit any *audio
dependency actually shown to wait while holding it*. A proven missing release
is a separate native fix. If joined/RT audio still exhibits long assigned-to-
resume or ready-to-grant delay, reject workgroup-only success and report the
exact blocked resource/holder. An owner-aware baton dependency or different
execution architecture would then need its own design; K/R is not a substitute
for resolving it. No such scheduler rewrite is authorized by this document.

## Alternative: produce in the RT callback's pull

Reject simply moving produceTick/Audio::pull into fill. Requests arrive in
device-sized bursts; pull can return short while the guest prepares a block,
takes the registration os_unfair_lock, and raises interrupts whose answers need
the baton. It cannot wait for those answers in an RT callback. Moving JAS/DSP
there would introduce guest execution, message/heap dependencies and possible
blocking under SDL's callback locks. Existing paced_ring.hpp documents why
burst-driven DMA advance previously shortened the game's block deadline.

A genuine callback renderer would require a separate native rendering engine
that consumes immutable precomputed commands/samples without guest/baton locks,
with bounded execution and verified sample/interrupt timing. That is a larger
architecture change. Keeping the existing ring as the callback's bounded pull
is the smaller initial experiment. Device-group use becomes appropriate only
for auxiliary rendering proven to finish in that actual callback cycle; mere
callback notification of asynchronous future production does not establish it.

## Errors, routing changes, and verification

Creation/query/join/policy failure: retain audible legacy playback, emit one
off-RT status with exact API/error/role/generation, and mark the experimental
arm **not adopted / not qualifying**. Do not return silence or abort an RT
callback merely because an optional scheduling API failed. EALREADY is not
treated as proof of correct existing membership. Never leave using an invalid
or another thread's token; never retry in a tight loop.

Default-output/device-alive/rate/buffer change: a non-RT control worker resolves
the new output generation. The custom groups are application-owned, so a device
change does not turn them into that device's group. Quiesce/reconcile the sink's
current rate and block-generation contract using its normal reconfiguration
path; start a fresh verified generation if the cadence changes. A device-group
variant must leave the old device group on every member, retain it until those
acks, then join the newly resolved group; intermediate/mismatched generations
are unqualified. Listener removal and callbacks-in-flight need teardown tests.

Verification before any shipping claim:

1. Server native fixtures: role binding without priority heuristics; successful
   and failed policy/join; exact same-thread leave/token handling; generation
   ABA, cancellation, thread exit, device change during join/stop, listener races;
   sample/block/interrupt ledger unchanged; no allocation/blocking in callback.
   Compile against minimum deployment target and run M4 native admission tests.
2. Cheap default-off verification: production codegen plus retained off probes.
   Preserve the stopped firehose trace as failed evidence. New verification
   uses 1 Hz role/generation/interval/ring/AI/DSP aggregates, low-rate callback
   gaps and underruns, and a bounded trigger-only mutex history if required.
   No per-acquisition streaming. Count all drops/overwrites; incomplete windows
   cannot support causality. Qualify its own overhead before traced conclusions.
3. After PORT authorizes runs, same-exe structural OFF/ON long M1 arms in a fixed
   alternating cohort, identical real CoreAudio/cache/input routes; all positives
   retained, no rerun-until-clean. Include normal/full-star and long warm title
   coverage. Proposed duration: at least 15 minutes per long arm, because prior
   failures occurred beyond 180 s; freeze arm count/inputs before launch.
   Apply ea7dc9d3 unchanged: at least 50 actual audio reports, total underrun
   frames zero. Report AI replay/DSP holds too; ring silence is not success.
4. Paired M4 pacing/audio runs, same windows and configurations; report p99,
   post-100 p99, >50 ms count, max, CPU/wakes/energy or unknown. A long M1 audio
   pass plus M4 no-regression and strict functional/replay checks are required;
   native_platform_os improvement alone cannot qualify this audio fix.

No new M1 run, structural implementation, gate relaxation, buffer increase or
K/R combination follows automatically from this design. The next action is
PORT's review of the asynchronous/device-workgroup distinction and the proposed
interval/baton verification contract.
