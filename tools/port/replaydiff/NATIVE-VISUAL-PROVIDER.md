# Native visual provider: source review and receipt proposal

Status: source proposal only; runtime measurement NOTMEASURED, qualification BLOCKED.
No renderer, Game, Aurora, or shared checkout changes are included. No app, build,
GPU capture or debugger was run. `native_visual_source_pins.json` binds the exact
native 33e71ee8 and upstream Aurora 08122911 blobs inspected in isolation.

## Source chain and missing joins

| Source location | Available evidence | Missing binding |
| --- | --- | --- |
| `GameSystem::frameLoop` | Draw, then update/calcAnim, then endFrame and host seam | The post-endFrame state is not automatically the state drawn by that frame |
| `MainLoopFramework::{beginRender,endRender,exchangeXfb_double,copyXfb_triple}` | Buffer-mode-dependent display copies; pre-render callback before copies | A uniform observed draw-state token. Double copies precede current draw; triple copies follow it. The callback is not a general draw boundary |
| `native/app/frame_seam.cpp` | Typed observation can occur before CPU release and smoke/input events | Exact visual scene/epoch/source-frame receipt, using the headless observer's placed scenario and init-state 6 contract |
| `patch_aurora_present.py` GXCopyDisp replacement | Source/destination rectangle, destination address, gamma/filter/clamp, ordered FIFO publication | Copy serial plus draw/source token carried with the operation, rather than an address-keyed association |
| Aurora `command_processor.cpp`, `GXFrameBuffer.cpp::copy_tex` | Destination, copy key, handle, revision; recorded resolve | Revision is CPU recording progress, not GPU completion. A reused handle is not immutable pixel content |
| `present_host.cpp::composeFrame`, `present_aurora.cpp::takeImpl` | VI state, latest/latched policy, selected texture, aspect | Selected copy serial/resource generation/revision and draw token. LatencyProbe counts composition, including installation, not Game frames |
| `present_aurora.cpp::{maybeDumpXfb,readXfb}` | Raw-XFB compute conversion, MapAsync completion, dimensions | Current screenshot work may enqueue before the selected frame's final submission; no per-copy submission/overwrite proof |
| Aurora `frame.cpp::end_frame`, `aurora.cpp::end_frame`, `render_worker.cpp` | Renderer packet and final encoder submission point | Tagged readback encoded after the selected resolve, protected against later overwrite, with validated completion |
| `png_write.hpp` | RGB IHDR/IDAT/IEND bytes | Checked write/flush/close, atomic publication and hash; no sRGB/gAMA/profile declaration exists |

The source queue order permits screenshot work to be enqueued before the current
frame's final submit. Earlier EFB segments can also submit work. This establishes
an ordering-evidence gap, not an observed stale image or a diagnosed renderer fault.
`gfx::finish()` finishes recording; it does not submit the current encoder.

The headless typed observer supplies the appropriate ready recipe: exact Game
scene, internal stage, **placed** scenario, init-state 6 and canonical player;
scene epoch must be observed. Smoke/progress validity and selected scenario cannot
replace it. First-ready anchors cannot be silently moved after invalidity.

## Geometry and color

Texture size/format, logical copy rectangle, scaled resolve rectangle, SC aspect,
effective editor aspect and presentation viewport exist at different points.
Window points, renderer surface pixels and raw-XFB pixels are distinct. Preserve
all relevant dimensions and PAR/DAR provenance; do not fit, crop chrome, or infer
source pixels from a window screenshot. Raw XFB excludes host overlays but also
excludes VI black/dimming. RmlUi replacement must be recorded explicitly.

Aurora render textures inherit the surface configuration format. The readback
shader texture-loads, clamps and rounds to RGB8; neither format alone nor a PNG
extension proves sRGB code values. A provider must bind actual texture/view format,
shader transfer and output encoding, then validate with a later allocated GPU
color-ramp control. Until then color-space qualification is unknown.

## Smallest bounded implementation proposal

1. Add an opt-in native visual observer in new native files. Reuse typed readiness
   and the existing ready schedule for +60/+300/+600; record actual consumed idle
   input and scene epoch. Default behavior stays unchanged.
2. Resolve the draw-state phase contract **before** implementing frame claims.
   Existing post-endFrame counters and pre-copy callbacks do not provide the
   required draw token. An independently reviewed draw-boundary producer or an
   explicitly revised phase contract is needed. No Game patch is granted or made
   here. Never compensate with a guessed +1/+2 or pixel alignment.
3. Carry a monotonic display-copy serial and observed draw token in FIFO order to
   the copy decoder. Preserve them with resource generation/revision through
   selection and renderer packets. Texture copies, resets and resize invalidate
   or replace tags explicitly. A destination-pointer side map is insufficient.
4. Encode capture compute/copy commands into the **same** renderer encoder after
   the tagged resolve and before Finish/Submit, before any overwrite. Record packet,
   operation indices and all resource writes in that interval. Map only after
   submission and completion; capture validation-scope/device-loss results.
   Queue Submit itself returns no success value. Async callback storage must remain
   alive through timeout/cleanup. Holding a texture handle does not freeze pixels.
5. Use a checked capture-only PNG writer: validate every write, flush and close,
   verify complete PNG/dimensions/hash and atomically rename from partial. Emit a
   receipt only after completion. Carry format/color, VI, viewport, renderer,
   executable/source and input hashes without invented defaults.

Review first; implementation and runtime require a separate allocation. Proposed
later controls: two tagged copies to the same resource, overwrite/reset/resize,
validation/device/map/write failures, GPU color ramp, then one bounded idle scene
with three exact requested offsets. Dolphin provider frame/color/readiness binding
remains a separate prerequisite to any paired qualification.

## Offline review tool

`native_visual_receipt.py --evidence PRIVATE/evidence.json --out PRIVATE/fresh.json`
checks a proposed joined event graph against a hash-bound dense source trace,
canonical idle tape and complete PNG. It checks resource versions, operation order,
selection, VI/overlay state, actual input receipt and checked publication metadata.
It cannot establish that a producer told the truth: it always returns
`runtime_provider_accepted=false`, `measurement_status=NOTMEASURED`, qualification
BLOCKED and null qualified counts. Even consistent synthetic evidence yields only
a `proposal-only` presentation candidate, rejected by the existing captured-provider
gate. The strict same-source-frame model is a review frontier, not an assertion
that current draw/copy phases satisfy it.

Synthetic controls cover missing objects in the evidence graph, queued-counter
substitution, copy/handle/version mismatch, overwrites, encoder order, completion
failures, malformed publication, geometry/color/VI gaps and target mismatch. They
exercise receipt logic only, not GPU behavior. Existing visual image controls
remain in `test_visual_resolution.py`.
