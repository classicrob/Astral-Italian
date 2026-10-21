# Per-galaxy visual reference pairs

Run after the Yoshi human-replay verdict. World 1 follows normal play order:
Sky Station, Yoshi Star, Spin-Dig, Fluffy Bluff, Flip-Swap, Rightside Down,
then Bowser Jr.'s Fiery Flotilla, with scenario selection recorded explicitly.
Resolve canonical names/scenarios from the source catalog before launching;
this ordering is a planned itinerary, not evidence that any entry already works.
Only the bounded Flip-Swap and Yoshi original-string stage selectors exist.
Other stages need a separately reviewed immutable-string selector and runtime
acceptance, or the original controller progression route. Never patch saves or
stage strings merely to fill the gallery.

For each accepted entry use the same 600-frame canonical connected-idle KPAD
suffix on channel 0 with Nunchuk, other channels disconnected. Retain the exact
actual delivery export and its normalized suffix hash. Bind every provider to
its actual post-endFrame Game/init6, exact stage and placed-scenario ready row;
keep natural seeds, source origins, state and clock differences visible. Capture
post-ready offsets +60, +300, +600 without input/temporal/image fitting.

Provider work remains: the native figure-probe screenshot counter begins at a
placed/nonzero player or steady-play condition, not the common ready seam. It
cannot establish these offsets. Add a small GX-side screenshot schedule linked
to the actual shared ready predicate, frame counter and presentation ID, with a
request/completion receipt and unmodified gameplay inputs. Dolphin's GDB seam
stops the guest, but a macOS window capture or a queued Dolphin screenshot alone
does not prove which XFB/presented frame it contains. Bind actual presentation
IDs/VI state and screenshot completion to the sampled source frame; disclose
latency rather than labeling an unknown image +N. This is still a provider gap,
not a reason to fabricate readiness or presentation equality.

Use the actual game viewport only, bound pixel/display aspect and color space, actual renderer/backend,
resolution, aspect, AA, color-space, app/executable/source/disc/tool hashes and
host in the receipt. No forced Null backend, render-setting shortcuts, host-clock
alignment or unrecorded camera repositioning. Preserve the baseline geometry
and input. If camera/state/RNG differs, retain that finding beside the images;
visual similarity does not make gameplay equal.

`visual_pairs.py --manifest PRIVATE/pairs.json --out PRIVATE/new-report --keep 10`
compares complete opaque sRGB game viewports on a fixed common grid (long side
at most256, no upsampling, declared display aspect). Pixel aspect must match
actual dimensions and bound display aspect. LANCZOS normalization and a fixed
0.75-grid-pixel Gaussian filter reduce small sampling/filter differences;
there is no image registration, crop, temporal shift or offset fitting.
It retains unfiltered RGB MAE/RMSE, SSIM8 and dHash, plus filtered metrics and
worst-tile RGB error. Ranking uses fixed color-RMSE, one-minus-SSIM and
worst-tile weights0.45/0.45/0.10; color changes cannot hide behind luminance.
This is heuristic review ordering, not a semantic equivalence threshold.
`report.html` and report.json rank measured pairs, retain the top original
image pairs plus common-grid/difference views, and show bound producer state,
source/binary/host, input and ready-frame provenance. Unknown image/provider
frames are NOTMEASURED/BLOCKED, with no metrics or fabricated image. Absent
+60/+300/+600 pairs are explicit missing slots. No gameplay PASS is emitted.
All screenshots, traces and reports stay under private gamedata. Commit only
tools and synthetic tests. After summaries, keep the top pairs and failed-provider
witnesses; prune only owned redundant images, never shared assets or user saves.

Manifest schema 1 contains `pairs[]` with stage/scenario/offset/input_kind=idle,
and native/dolphin objects. Each object includes `{path,sha256}` records for
image, sync, trace, input and presentation_receipt; capture_source_frame,
presentation_binding='frame-id-verified', host, executable_sha256,
source_revision, renderer, image_geometry and input_delivery_receipt. The trace must be dense from0 through the offset, with the same observed Game/init6 stage/scenario/epoch and valid player throughout; endpoint-only evidence is rejected.
The presentation receipt is an actual provider artifact, never a fixture
assertion for live data. No gameplay PASS or admission is emitted by this tool.

## Source adapter checkpoint

`visual_readiness.ReadySchedule` now accepts actual source observations from
both providers and produces requests only at the fixed offsets. It observes
contiguous post-endFrame seams from source frame zero, anchors the first exact
Game/init6 target, retains separately observed player validity, and cancels
rather than reanchors after a scene/epoch change. An unavailable player at the
actual anchor remains BLOCKED; it never moves the anchor to a later convenient
row. `presentation_receipt` validates real copy/readback IDs, source/epoch tags,
completion fence and exact private image hash. These helpers have synthetic
coverage. They are not a claim that renderer hooks have run.

Native integration points are `native/app/frame_seam.cpp` before CPU release
and `native/gx/present/present_aurora.cpp` after FIFO drain/readback. Existing
`Screenshot::request` means next presented game frame and must not be relabeled
as its request's sampled state. `LatencyProbe::frame` counts Aurora choices and
is incremented once during initial presentation installation; it is not a game
source-frame counter. Its current single-press `noteCopy` record cannot supply
a complete image schedule. Native latest-copy presentation also precedes retail
VI scan-out; keep that distinction explicit instead of changing the reference
renderer. Actual GXCopyDisp copy tags and readback IDs, or an explicitly
unqualified window-capture mode with unknown presentation lag, are still needed.


## Raw window diagnostic adapter

`visual_dolphin.py` wraps the existing entry/collector source observations and
requests a read-only owned-window screenshot at the actual source offsets. It
checks the stopped guest PC/LR, executable path/hash, process start identity,
screen-capture permission and an unambiguous window belonging to that PID.
An explicit window ID can narrow that owned set; it cannot select another app.
The capture receipt retains full window bounds, dimensions, timestamps and PNG
hash. It labels GPU/XFB age UNKNOWN and leaves render/readback IDs and fence
completion unavailable. This adapter therefore produces UNQUALIFIED window
witnesses; it cannot satisfy the stricter presentation-bound pair gate above.
Its window chrome is not silently cropped or treated as an equal viewport.

Readiness observation allows the actual pre-controller rows with unavailable
scene/init/scenario fields, without filling them with zero. An unavailable scene
after the accepted anchor cancels the schedule. Synthetic tests cover those
transitions, exact source offsets, observer restoration, PID reuse, permission
failure, image failure, ambiguity and rejection of another process's window.
The wrapper has not yet been used in a live visual reference run. Native GX
presentation binding is still a provider gap. These tools do not change any
emulation/render settings, inputs, saves or guest memory.


## Metric schema2 and stricter producer bindings

Existing manifest schema1 is retained; `metric_schema_version=2` identifies the
new comparison policy. The presentation artifact must bind all provider
host/executable_sha256/source_revision/renderer/image_geometry values and the
actual input_sha256, besides matching source/render/readback frame IDs, image
hash and completed fence. A role-mismatched method is rejected. image_geometry
is `{kind:"game-viewport",pixel_aspect_ratio:[p,q],display_aspect_ratio:[a,b],
color_space:"sRGB"}`; these are actual provider facts, not inferred crops.
Provider identity hashes must be full40/64 hexadecimal values. The sync artifact
must also retain its actual source_scene_epoch, separate from a rebased trace
epoch. Metadata and manifest hashes are rechecked after measurement; retained
original image copies must match their pinned hashes.

The input_delivery_receipt is a hashed producer artifact with schema_version1,
status captured, input_sha256, frames600, read_count2400, source_frame_start
ready+1 and source_frame_end ready+600. Method is native-consumed-read-log or
dolphin-actual-export, according to role. Actual JSONL records are still parsed
and must match between providers and the declared canonical connected-idle
contract; a receipt assertion alone does not replace the input tape.
`presentation_receipt(...,provenance=...)` can bind explicitly supplied facts;
without them its older core receipt remains insufficient for the pair gate.
Do not synthesize these producer facts to satisfy the comparator.

Synthetic controls cover missing objects, changed color (including matching
luminance), seven-pixel translation, resampling/filter differences, pixel aspect,
transparent/mismatched viewports, stale/unbound frames/identities/input, missing
providers and offsets, missing trace rows and a reload between equal endpoints.
Synthetic tolerances qualify only these fixtures. They are not thresholds for
accepting real gameplay/rendering. Existing Python3.14/Pillow12.1 are used;
no new dependency was installed.

Actual provider gaps remain unchanged: native GXCopyDisp source/copy tag to
completed renderer readback; Dolphin XFB/VI source tag to actual PNG readback;
actual same delivered canonical idle read history on both sides; bound viewport
extent/PAR/DAR/color metadata and executable/source provenance. Window captures
with unknown XFB age/chrome do not meet those gates. Native figure-probe timing
and queued screenshots cannot be relabelled as ready+N. No provider hook/app,
Game source, backend setting or runtime installation was changed in this task.
