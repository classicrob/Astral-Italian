# M1 compile/control packet and opt-in bridge source

Status: SOURCE ONLY, EXECUTION BLOCKED. The original31a component is unchanged.
E paired source support covers31a + GAME046e6a71, not this new bridge. E report
9fcb8e0b159bdd343e70abbcc31abad055dfef1aeafdabfd3ee1a8c16e812e1a
retains sampler/lifetime/installation gaps. L closed the adapter alpha parity
finding at2720a2af. No existing checkout/source/build file is modified here.

## Exact bounded compiler plan

T2 supplied actual effective M1 toolchain6a97301f6a64258b4e67991e1ef22c2a07abca3b7662478a702aec5b0ed9e828
and canonical GameSystem argv context3c123262017672c6ddf256715645a64d23b613e20d6964a1d3f52c69688918e7.
The effective external toolchain is not inferred identical to its historical git
parent. Actual-link context829db9316284652a9ab1ed393b57c4937eb297b77fe1117a2e26bf8b2d9ab89c
binds explicit matched static libc++/ABI/unwind. `m1_draw_context_pins.json` records
the eight actual compiler/linker/runtime/config/SDK metadata hashes. No compiler
or linker was invoked to prepare this packet.

The source-only generator emits fourteen exact argv arrays:

1. Compile core, exporter, core tests, bridge, bridge tests as five separate TUs.
2. Compile-only original reader, no-XFB reader and host adapter using the actual
   canonical GameSystem native defines/includes/flags. Only TU/output/dependency
   paths change; Game compile policy remains intact. This is compatibility with
   that retained M1 context, not proof of the future integrated executable's source.
3. Link and run two standalone controls, and inspect each binary with otool.
   Explicit static libc++.a/libc++abi.a/libunwind.a, LLVM20 lld, arm64, SDK15.1 and
   macOS15 deployment are retained. Assertions stay enabled in controls. No app,
   Game link, GPU, image, installer or actual host callback runs.

```
python3 tools/port/replaydiff/plan_draw_observer_m1.py \
  --context PRIVATE/M1-GAMESYSTEM-COMPILE-CONTEXT.json \
  --source PRIVATE/frozen-source \
  --output PRIVATE/fresh-m1-output --plan PRIVATE/fresh-command-plan.json
```

The generator cannot execute anything or acquire a lock. The explicit command
packet must be materialized on M1 with matching frozen source bytes and existing
T2 managed environment; server-only paths are not implicitly M1 snapshots. Verify
all tool/source/SDK pins there before any spawn. Pin all candidate consumed headers
(including extensionless headers, encoded include roots, LLVM resource headers and
SDK headers) before compilation; reject any depfile path absent from that inventory
or any changed hash afterward. Recheck the actual tools/archives and inspect
linked libraries for unexpected dynamic C++ runtime. SDKSettings alone does not
hash every SDK header. The otool executable needs an actual M1 pin too.

Estimated45–90s, UNMEASURED; whole job hard120s, nice19, sequential/j1, <=256MiB
private output. Stop and retain evidence on timeout; no extension or retry in the
same allocation. Core native controls remain UNRUN; bridge native controls remain
UNRUN; all three typed TU compiles remain UNRUN. Python plan controls establish
only command-generation rejects, not C++ correctness or actual tool ownership.

P confirmed no reviewed generic compiler runner/API exists for this context.
Disposable v7 child controls do not qualify arbitrary compiler descendants. No
checked-PNG/abc/ec806 runner or lookalike replacement is supplied here. Execution
requires reviewed exact child identity/ack-before-exec/tool/env/whole-group cleanup
and an explicit P/root M1 handoff. Preserve persistent lock files. No slot is taken.

## Small source integration diff

`pilot-integration-33e71.patch` changes only root all-consumer linkage and the GX
host seam: guarded inactive bridge is called before CPU release, after endFrame.
It does not install GAME brackets; apply B046e6a71 separately on33e71 and the
new component files only, never another branch's parent chain blindly. No shared
Aurora/FIFO/worker/copy changes exist. The patch is proposed/unapplied.

`PilotIntegration.cmake` adds the core dependency through the actual
petari_game_objects target for both GX/headless normal consumers. It links the
optional host adapter privately through petari_app_host. Typed sampler/host TUs
consume native_config privately; only SDK-free bridge/core interfaces propagate.
PRIVATE PETARI_DRAW_PILOT_HOST_SEAM enables the guarded include/call on the real
app-host target only; direct-source seam tests retain their baseline body and
need no Game provider or shim. The definition is absent from native_config. Manual partial-link trap mode
fails configuration explicitly until its archive inventory is separately bound;
there is no unresolved-symbol fallback or silent GX-only accessor. Native link
checks in all supported final consumers remain UNRUN.

`DrawPilotBridge` is passive until configured by an explicit owner call. The GX
host attachment must occur on the eventual Game owner BEFORE first host seam,
with fixed storage alive until process exit and a mandatory stop callback. No CLI,
environment, startup call or registration is installed. Late attachment is refused;
default inactive seam reads no Game objects. This source does not silently choose
an executable, capture directory, seed or input schedule.

The bridge waits for first actual typed Game/init6/current/player-valid exact
RedBlueExGalaxy placed1 seam. It deliberately does not require selected-current
at the anchor: readiness and selected draw remain separate. It installs the
observer only then. All subsequent loop/draw/post-endFrame phases are retained,
including key changes or unready states; no rebasing/reanchor. Each next seam must
be consecutive. After exactly616 observed loop intervals it uninstalls and returns
StopRequired. Those intervals are NOT proof that616 input records were consumed;
the future canonical idle KPAD/census/export/stop receipts must establish that.

The host adapter invokes the mandatory stop callback before releasing CPU and
never returns to Game on StopRequired/FaultStopRequired. A callback that returns
aborts the owned process rather than executing input617. This is an opt-in harness
stop policy, not scheduling mutation inside the observer. The external stop callback
must preserve unresolved installed observer storage, publish detached evidence and
halt Guest while any allowed GPU drain advances no Game CPU. No stop callback,
publication routine, ownership runner or GPU drain is implemented/qualified here.
Abort/host lifetime branches are not exercised by the standalone core controls.
The callback must not traverse Game objects or destroy storage still installed.

## Explicit sampler preconditions and unavailable frontier

The original31a reader still reads VI/drawDone-written XFB fields; it remains
frozen with that installation gate OPEN. New `readDrawSnapshotNoXfb` removes the
entire JUTXfb lookup/read block. xfbPresent is0; zero mode/index values are therefore
UNAVAILABLE, not observed mode0. It cannot satisfy phase-v2 mode3/copy selection or
produce a pixel candidate. No race is hidden as a benign value.

E's source frontier8fb62216ae89e51baccceab91fdcf0cc4d252247f43eb55b37ed9a7d7d8ef7b0
found only ctor/beginRender named writers of framework flag/counter/interval in
immutable33, subject to complete framework publication/lifetime and no concurrent
reset/destruction. This is not arbitrary alias/current-provider certification.
The new reader still needs actual async17/OS-baton visibility and retirement proof
for controller names/scenarios/init/current and selected scene, including reads
BEFORE init6. Same Game thread or first-ready anchor alone does not establish it.
Installation remains BLOCKED until E/owner close that exact boundary or those scene
observations are separately unavailable. Source compile must never imply sampling
safety. No observer is installed by this packet.

## Next executable step and pilot boundary

After exact new source review, root/L can review the fourteen-argv packet and the
new compiler runner context. Only then request P's actual M1 handoff for one<=120s
standalone control/compile allocation. It is separate from app/link/runtime
admission. Next is actual all-consumer link/default-off controls and the reviewed
same-owner seam/input-stop integration, then one bounded metadata-only pilot.
Real copy/clear/endGX/resource/selection/GPU/readback/viewport/PAR/color/publication
and Dolphin host bridge evidence are still absent. No actual pixel comparison can
be qualified from this packet or its compile-only outputs.

Source correction after6c52 review: canonical argv now use -MD, not -MMD, so actual
future depfiles include system headers; pre-inventory alone does not establish
consumed dependencies. Native compile and installation remain blocked. The old
r2 arrays/source mirror are frozen; use separately pinned r4 arrays for new review.

M1 preparation now has exact r4 source bytes and fourteen arrays at team
scratch-port/solD/draw-observer-m1-plan-r1/command-plan-r4.json
SHA ddac833b086fe0cee342610f4b4a48c640b45e1a7345a399ba1458f778ae0414.
Candidate-input inventory397305e1ed271187cec5080b02edf2164cd697e827a4d01b00cf421f9e10bc63
contains21002 resolved source/tool/SDK/resource files and566102319 read bytes.
It is candidate preflight evidence only; no actual consumed dependency or native
execution is observed. Actual /usr/bin/otool pin is
3573dc149c40b6dbba10a2465334e080da2e5f598b2f14178074ab8ddd013307.
The immutable inventory/summary live under the approved M1 private source packet;
no game assets or raw captures were copied. Every pin must be revalidated before
any future execution, and actual -MD paths must all join the pre-inventory.
