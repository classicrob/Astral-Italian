# No-XFB pilot lifetime review packet

Status: SOURCE PROPOSAL, independent review pending. Installation and execution
remain BLOCKED. This packet changes no callback, reader, scheduler, input, VI or
Game source. `lifetime_source_pins.json` binds the exact source used below.

The component baseline is 39c71f41abcbbe44ba0151557974b6b4d70e9324,
including the 3ceebf4a amendment. The separately owned GAME brackets remain
046e6a71afea72c219e1f5d454e606131f99a7e7. Platform/Game semantics are read
from immutable 33e71ee880b426c0c7da03a724377200e874c417 Git blobs. In
particular, the older OS source in the visual branch is NOT substituted for33.
The pins distinguish these revisions, and bind the reviewed r4 command metadata
ddac833b086fe0cee342610f4b4a48c640b45e1a7345a399ba1458f778ae0414.
No statement here establishes the instructions or stack layout of a compiled app.

## Actual execution and ownership path

`app_main.cpp` calls startOS, openFirstFrame, then runGame on the same host
thread. startOS invokes OSInit; `__OSThreadInit` registers that thread as
DefaultThread, priority16, and binds it to the CPU baton. runGame calls
petari_game_main. Game initialization constructs the controller and framework
before the persistent frameLoop. This is the source path for the proposed owner;
it is not proof that an unimplemented installer actually follows it.

B046's three synchronous borrowed callbacks bracket beginRender and draw in
frameLoop. There is no callback queue or asynchronous observer thread. Each
callback finishes before the next Game statement. draw and endRender remain
unconditional. The later update/calcAnim/endFrame occur before the GX host seam.
The proposed production-only seam call is before recordRouteFrame and before
gRelease.begin. The already installed release callbacks map to
petari_os_begin_host_blocking/end_host_blocking. composeFrame, presentation and
events occur while released; reacquisition waits for this same OS thread's baton
before frameLoop continues. No reader is proposed in composeFrame.

The seam is GX-specific. The headless seam and its virtual clock are not this
installer or a proof of GX owner lifetime. The core accessor still requires real
all-consumer linkage; the existing manual partial-link integration is a HOLD.

## Async17 publication and retirement

`exeInitializeScene` queues initializeScene through FunctionAsyncExecutor at
priority17. Its worker is an OSThreadWrapper: native hostEntry binds the worker
and waits for its own baton before invoking its run function. execute temporarily
sets the requested priority. initializeScene writes init state, current names and
scenarios, creates/publishes mScene, then initializes its holder and scene.
Publishing mScene therefore does NOT publish a fully initialized scene.

After the functor returns, the worker sends the completion message and sets
mIsEnd. `tryEndFunctionAsyncExecute` first polls and then waits for/removes the
completion record. Only its successful owner-side return causes
exeInitializeScene to set init6. Init6 still precedes Ready/Normal and is not a
selected-draw certificate. The sampler's holder/player traversal is after the
init6 test; its names/init/current/selected reads are before that test.

For destruction, requestChangeScene sets initNotInit. exeDestroyScene queues
destroyScene at17. That worker clears draw-sync scene callbacks, saves mScene
locally, publishes mScene=nullptr, deletes the saved scene, clears holders and
resources, and destroys scene heaps. The owner later consumes completion before
advancing the controller nerve. The pointer value itself never grants a lease.
No callback may carry current/holder/actor pointers over a baton handoff or into
export. A pointer sampled before retirement could otherwise become dangling.

getCurrentSceneForExecute only reads mScene and controller nerve/Spine state and
returns mScene when Normal, otherwise Intermission. It does not acquire a scene
lease. During initialization a snapshot may legitimately describe initialized
Game storage while the selected scene is Intermission. After a new request, an
unready old Game may still be selected until the nerve changes. Both relations
must remain separate from readiness.

## Baton visibility is necessary, not a whole-callback lock

Cooperative handoffs occur under interruptMutex. giveCpu stores gCurrent with
release ordering; waitForCpu waits with the same mutex and loads gCurrent with
acquire ordering. OS interrupt enable/disable can deliver pending preemption.
The reader/observer/bridge source contains no OS scheduling call, host-release
call, wait, allocation or publication callback in its sampling traversal.
HostAllocationScope in the seam is an allocation policy, NOT a CPU lock.

Forced preemption also exists. tryForcePreempt suspends a holder and accepts only
a stack chain classified Game/Library through ThreadStart. Host, Anonymous,
unknown and external-library frames do not authorize transfer; unknown symbols
are learned after resumption. hostCode recognizes the PetariNative:: prefix, so
the named Visual reader/dispatch/host frames are source candidates for refusal.
This is the relevant additional source mechanism; "one CPU" alone is insufficient.

This refusal argument is conditional on the actual complete retained stack and
symbol classification at every pointer read, including inside the Game getter
or an inlined library operation. The r4 standalone argv have no LTO flag, but
they are not the future integrated app's object/link/symbol evidence. Inlining,
tail calls, aliases, missing unwind frames or different flags cannot be waived
by naming the namespace. No compiled-frame refusal proof is supplied here, and
no change to scheduling or interrupt masking is proposed to manufacture one.

The accepted source contract must therefore state BOTH that the owner holds the
baton and that no handoff can interrupt the live traversal. If E cannot establish
that latter property at source scope, scene observations take the unavailable
proposal below. Neither first-ready nor a sampled epoch repairs the gap.

## Registration, stop and storage retirement

The following are obligations on the future installer, not enforced thread-ID
checks in the current API:

1. Allocate fixed bridge storage before callbacks can run; keep it alive through
   process termination. Attach once on the registered DefaultThread after OSInit
   and before its first frameLoop/host seam. No host-thread registration race,
   reset/destruction, callback reentry or singleton replacement is permitted.
2. With no active observer, the borrowed accessor returns nullptr. The inactive
   host seam returns before either singleton lookup or typed sampling.
3. At the first matching Game/init6/current/player RedBlueEx placed1 seam, the
   bridge begins and installs the observer synchronously. The preceding draw was
   not observed and must not be relabelled. The anchor does not require Normal.
4. Subsequent loopEntry/drawEntry/drawExit and postEndFrame are consecutive on
   that same owner. Each snapshot is detached POD before callback return. Local
   pointers do not enter events. Key epochs are sampled identity counters, not
   allocation generations or evidence of transitions between samples.
5. At seam anchor+616, postEndFrame first appends the final interval, then stop
   uninstalls the observer. The host clears activePilot/stopCallback before
   invoking the mandatory callback. A returning callback aborts before interval617.
   A protocol/capacity/snapshot fault also requests a terminal stop, never reanchor.
6. If uninstall fails, the result is FaultStopRequired. Export must refuse an
   installed observer and its backing storage must remain alive. The terminal
   callback may only inspect detached data after successful removal; no Game
   pointers, normal Game continuation or deferred destruction while installed.

There is no detach API for an attached-but-not-yet-ready bridge. Its storage must
also survive indefinitely while waiting for ready. Waiting has no internal boot
timeout. A future owned outer deadline must terminate the process without freeing
installed storage. No real stop callback, CPU-stop acknowledgement, KPAD receipt,
GPU drain or publication implementation exists in this packet.

616 is an observed loop-interval limit, not proof of616 consumed input records.
An exact KPAD/read census must independently bind those intervals, including the
prefix and tail. Uninstall-before-callback is not proof that later rendering
workers have finished; GPU/image providers remain outside this contract.

## Separate unavailable-field proposal

Do not silently install a partially safe reader or represent unavailable values
as observed false/zero. Pending review, installation stays blocked. If traversal
serialization cannot be proved, a separately reviewed variant must omit ALL
controller-derived reads: controller/current/selected presence and equality,
scene/stage/placed/selected key, init state, initializedGame and player validity.
It must declare each unavailable with reason "async scene lifetime not proven".
The present Snapshot schema has no per-field availability for these observations;
this packet does not reinterpret its zero values or broaden that schema.

The remaining framework flag/counter/interval also require constructed borrowed
framework lifetime and no reset/destruction. setForOSResetSystem deletes sManager;
callbacks after that are unsupported. XFB/VI fields stay explicitly unavailable
as already proposed, with no XFB pointer lookup. Without the scene fields the
current bridge cannot establish its ready/key anchor and cannot produce a visual
pilot. The honest outcome is no pair, not a synthetic ready snapshot.

## Exact review questions and remaining gates

E is asked to review the pinned source for: async17 publication/completion and
retirement order; cooperative visibility versus forced-preemption refusal limits;
whether a source-only noninterruptible live read can actually be established;
and the same-owner install/uninstall/stop/storage obligations above. If not,
identify the exact unavailable subset or smallest owner-side lifetime witness.
No new Game hook, scheduler primitive or field-availability API is requested here.

B046 and the native component remain frozen. No compiler, controls, configure,
installer, native lock, app, GPU or runtime ran for this packet. Python/source-pin
verification is not a memory-order or concurrency test. The standalone caller
remains hard-disabled at39c7;71679 actual environment qualification failure is
unchanged. Actual owner binding, consumed inputs, final linkage, real stop,
provider frame/copy/readback/color and pixels remain unqualified. Qualified frame
counts stay null and visual qualification stays BLOCKED.
