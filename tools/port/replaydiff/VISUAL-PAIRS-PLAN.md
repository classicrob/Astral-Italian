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

Use the actual game viewport only, equal dimensions, actual renderer/backend,
resolution, aspect, AA, color-space, app/executable/source/disc/tool hashes and
host in the receipt. No forced Null backend, render-setting shortcuts, host-clock
alignment or unrecorded camera repositioning. Preserve the baseline geometry
and input. If camera/state/RNG differs, retain that finding beside the images;
visual similarity does not make gameplay equal.

`visual_pairs.py --manifest PRIVATE/pairs.json --out PRIVATE/new-report --keep 10`
compares same-size full images without cropping, registration or temporal shift.
It reports normalized RGB MAE/RMSE, 8x8-block luminance SSIM and dHash distance.
The block SSIM is explicitly a heuristic, not a full-window standard SSIM or
proof of semantic equivalence. Rank by lowest block SSIM and retain the top
image pairs with exact provenance and state. Capture metadata must bind real
image, sync, trace, delivered-input and presentation receipts by SHA256; unknown
presentation/ready data is BLOCKED. A declared capture offset alone is rejected.
All screenshots, traces and reports stay under private gamedata. Commit only
tools and synthetic tests. After summaries, keep the top pairs and failed-provider
witnesses; prune only owned redundant images, never shared assets or user saves.

Manifest schema 1 contains `pairs[]` with stage/scenario/offset/input_kind=idle,
and native/dolphin objects. Each object includes `{path,sha256}` records for
image, sync, trace, input and presentation_receipt; capture_source_frame,
presentation_binding='frame-id-verified', host, executable_sha256,
source_revision and renderer. The trace must contain ready and offset rows.
The presentation receipt is an actual provider artifact, never a fixture
assertion for live data. No gameplay PASS or admission is emitted by this tool.
