# Native draw-state phase contract proposal

Status: SOURCE_PROPOSAL_ONLY; no runtime token producer, no uniform existing hook,
no patch generator, image/frame/GPU/color qualification BLOCKED. Checked-PNG leaf
is a separate isolated component and is unchanged. This review uses immutable
native `33e71ee880b426c0c7da03a724377200e874c417`; the Aurora pin remains
`08122911e8621acb7ded6563813b264bec1494b5`. No worker command change is proposed here.
`native_draw_phase_pins.json` contains the exact source hashes used below.

## Actual caller ordering

`GameSystem::frameLoop` calls, in source order:

```
beginRender -> draw -> endRender -> update -> calcAnim
-> screen-preserver capture -> endFrame -> native host seam -> wait
```

`draw()` is unconditional in this caller, including when `mDoRenderFrame` is false.
That flag alone must never become a claim of no draw/no EFB writes. The host seam
observes post-update/post-calcAnim state, which is distinct from the state entering
that iteration's draw. Do not relabel it as drawn state or apply a fixed +1/+2.

`MainLoopFramework::beginRender` first tests the **old** `mDoRenderFrame` for buffer
exchange, then increments `mRenderCounter` and computes the **new** flag against
`mRenderInterval`. Only the new flag controls the final clear/preGX and endRender
buffer actions. Record both flags if interpreting framework behavior.
`JUTXfb` constructs with buffer count 3; this is a source default, not a runtime
receipt. Modes 1/2/3 are explicit branches in the framework.

| Mode | Before current draw | After current draw | Display copy source/caveats |
| --- | --- | --- | --- |
| 1 | Old-flag branch sets drawing flag/index, may clear; new-flag branch may clear/preGX | New-flag `drawendXfb_single` prepares copy, arms/waits draw-done and publishes drawn index; it does not call GXCopyDisp | `JUTVideo::preRetraceProc` copies only when drawing flag 0, drawn index valid and black-countdown gates allow it. Retrace timing relative to a particular later draw is unbound |
| 2 | Old-flag `exchangeXfb_double`: if drawn==displaying and drawing index valid, optional pre-copy callback, prepareCopyDisp, GXCopyDisp(clear=true), then index exchange; otherwise clear branch | No mode-2 copy in endRender; endGX runs unconditionally; new flag gates buffer actions/endFrame waits | Copy occurs before this iteration's draw and before recomputing the flag. It may reflect retained EFB work, not a proven previous draw token |
| 3 | Old-flag `exchangeXfb_triple` direct-prints conditionally and exchanges indices; new-flag clear/preGX follows | New-flag `copyXfb_triple`, if drawing index valid: optional callback, prepareCopyDisp, GXCopyDisp(clear=false), GXPixModeSync | Copy occurs after draw and endGX, before update; this source order alone does not prove immutable resource/GPU/image identity |

`endRender::endGX` can draw console/assertion overlays after the GameSystem draw
bracket and before the triple copy. The bracket identifies the GameSystem draw
scope, not every EFB/XFB write or the final pixels.

`clearEfb` is drawing work and affects retained EFB provenance. Branch skips,
invalid indices, interval changes and mode changes must be explicit observations,
not dropped frames or assumptions that a copy exists. No address-keyed side map
can substitute for a draw/copy provenance chain.

The public zero-argument `mPreRenderCallback` is initialized null and invoked only
before conditional double/triple copies; it has no single-mode call. Repository
search at the pinned revision finds no other assignments. It is not a uniform
before-draw or after-draw callback. `PresentHooks::composeFrame` runs after
`gRelease.begin()` in the host seam, so it must consume detached metadata rather
than traverse mutable Game objects. VI retrace callbacks run in interrupt context. Any new observational callback
there must avoid mutable Game traversal, blocking, allocation, renderer operations
and forced state.

## Typed readiness is not executed-scene identity

Read immutable typed data on the Game CPU ownership boundary, before release:

- controller present, bounded `mCurrSceneControlInfo.mScene` and `mStage`;
- **placed** `mScenarioNo`, separately from `mSelectedScenarioNo`;
- init-state enum including exact 6/End; nonnull current scene;
- canonical MarioHolder actor validity, with initialization and object-holder gates;
- `getCurrentSceneForExecute()` result compared locally to `mScene`;
- sampled scene epoch with its explicit authority label.

`GameSystemSceneController::drawScene` draws `getCurrentSceneForExecute()`. This
returns `mScene` only when present and controller nerve Normal; otherwise it
returns Intermission. Init6 may be reached in InitializeScene before the
Invalidate/Ready/Normal transition, so existing scene_valid/init6/player_valid can
describe initialized Game storage that is not yet the scene chosen for drawing.
Conversely, requestChangeScene immediately marks initNotInit while the old Normal
scene may still be selected until update applies the request. Unready does not
mean no draw. Preserve both observations without changing existing startup-arrival
semantics or forcing the controller nerve/state.

The existing headless `scene_epoch` is a sampled pointer + scene/stage/placed-key
change counter, not an authoritative allocation/lifecycle generation. It omits
selected scenario and may miss unsampled transitions/address reuse. Keep this
sampled identity counter distinct from loop serial, draw serial and lifecycle ID.
A later authoritative lifecycle claim requires separately reviewed owner hooks;
neither pointer equality nor a matching sampled counter establishes it.

## Smallest missing hook and ownership contract

No current native callback supplies the required uniform draw boundary. Therefore
there is **no native-only hook patch/generator in this delivery**. GAME owns the
following proposed source callsites, subject to explicit review/authorization:

1. Immediately after beginRender returns, before `GameSystem::draw()`: an optional
   native-gated draw-entry callback. It assigns a run-local monotonic draw serial
   and observes typed metadata and the **actually selected scene** above.
2. Immediately after draw returns, before endRender: matching draw-exit callback,
   preserving observed changes/errors rather than asserting drawing is read-only.
3. If the contract also needs the old render flag/mode/interval/index state, one
   small loop-entry callback **before** beginRender is necessary. Reading only at
   draw entry sees the recomputed flag and cannot reconstruct the old flag.

The first two points are the minimum draw bracket. The third is needed for a
complete mode/control record; no previous-flag reconstruction is assumed. Existing
post-endFrame host observation remains a separate fourth phase. These additions
would be native-gated calls with default-off callbacks, no Wii behavior/layout
change, no scheduling, RNG, input, float math, settings or state mutation. Native
owns the callback implementation in new isolated files only after GAME supplies
and approves the callsites. No such calls are installed by this proposal.

Payloads should be fixed-size detached POD: run-local loop/draw/event serials,
mode and phase-local flags, bounded copied scene/stage, placed/selected scenarios,
init state, executed-scene relationship, canonical-player validity, sampled epoch
and authority. Pointers may be compared within the callback and discarded; they
are not output IDs. Use bounded integer/byte reads and no gameplay float operations
or global FP changes. Do not call the full trace observer merely to sample metadata.
Allocation, formatting or asynchronous I/O must not outlive live Game references.

Keep request time and draw observation distinct. Ready+60/+300/+600 requests refer
to the established post-endFrame anchor. A later draw-entry token may reference a
pending request by an **observed event ordering**, with its own snapshot and phase;
this does not prove that request-state bytes were drawn. The current image gate's
same-source-frame assumption must not be satisfied by relabeling that token. A
separate phase-contract decision shared with the Dolphin provider is required
before comparing images. No fixed lag, compensating frame deletion/rebase or
fitted alignment is proposed.

Draw brackets alone still leave EFB clear/mixed-work attribution, copy serial,
resource overwrite, GPU completion, viewport and color joins unbound. This task
neither implements nor reviews worker-command changes. The checked PNG leaf can
only publish bytes after a future valid producer has supplied them.

## Independent review and offline controls

GAME lead assigned E (`thr_ytqc72k9in`) independent read-only source review. E
confirmed the draw bracket, absent uniform hook, old/new flag distinction,
unconditional draw call, Intermission/Normal caveats, CPU-release boundary and
sampled epoch limitation. E reviewed GameSystem/MainLoop/JUT source semantics,
not native VI provider thread mechanics; the latter claim here is separately bound
to the pinned native VI interface. No Game defect or runtime evidence is claimed.

`native_draw_phase.py --source-root PRIVATE/immutable-native-source --out PRIVATE/fresh.json`
checks every pinned source blob before emitting the proposal and symbolic mode
branches. The symbolic branch guards are local conditions at their source sites,
not an executable simulation of index evolution or a record of runtime execution.
The tool cannot install hooks or issue visual admission. Every result remains
SOURCE_PROPOSAL_ONLY/BLOCKED with no runtime producer and null qualified counts.
Synthetic tests cover old/new flags, unconditional drawing, copy guards and order,
single asynchronous copy separation, callback non-uniformity, source drift,
actual-scene/player/phase/CPU/epoch gaps and selected-vs-placed scenario errors.
