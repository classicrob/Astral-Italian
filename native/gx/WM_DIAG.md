# Scratch world-map diagnostic

Diagnostic branch only. Never merge this instrumentation into main. The independent
J3DModelX VCD/VAT size fix is **not included** so a controlled capture can observe the
original caller. Source base: origin/main 490472ea. Environment switch:
`PETARI_WM_DIAG=1` (any other value disables collection).

Apply the exported patch/cherry-pick to a separate copy of the desired frozen source,
then configure a separate build directory with its existing sanctioned settings. The
last include of native/gx/CMakeLists.txt composes wm_diag.cmake after production
transforms; it reads the three final active Aurora sources and writes new files below
that build's native/gx/wm-diag. It never edits the shared downloaded Aurora checkout.
Anchors and active-source counts fail closed. Do not run patch_wm_diag.py against
Aurora and overwrite its inputs. Keep pristine receipts and diagnostic receipts separate.
Execution/build ownership and the nonblocking lock (`lockf -t 0`) remain with PORT/T2.
No new input/host permission is required by this diagnostic.

The native game additions are under GALAXY_NATIVE. Six game TUs and all three
transformed Aurora TUs passed syntax-only compilation, including composition with
the exact generated files in T2's gx-llvm20-worldmap51 build. No full link or app run
was performed for this diagnostic. Static source checks do not establish runtime success.

## Collected evidence

- Original GXCallDisplayList pointer, byte size, FNV-1a64, first16 bytes, caller label,
  copied model name, sequence, 12 return addresses with image base/symbol. Hash covers
  the full list through1MiB and only32 bytes above that, with hash-bytes reported.
- Separate preflush and inline spans in absolute FIFO byte positions. The worker gets
  its absolute process base from process_to (including the native mirrored-FIFO path).
  The diagnostic adds no FIFO commands and preserves all decoder checks.
- On masked CALL_DL, failed indexed-XF range checks, primitive overrun, or unknown
  opcode: containing sequence and offset, previous3 callers, raw command history,
  latest draw format/count/vertex-size/remaining bytes, CP VCD/all8 VAT/base/stride
  values with offsets, extended array bases with offsets, and actual matrix/light
  array state for indexed-XF errors. Inspect raw bytes: mask0xF8 accepts aliases.
- Producer recording/size>1MiB/null/alignment anomalies print metadata before reading
  input bytes. A size threshold is an observation trigger, not a size validity rule.
- GameScene captures camera eye/up/forward/fovy/near/far from the active context,
  interpolated camera watch target and the WorldMapCameraHolder target actor position.
  Stage-start sample occurs before GX work; after-draw samples occur at frames0,60,120...
- PointParts actors, MiniRoutePoint and galaxy actor counts: total/alive/hidden/clipped.
  WorldMapOwner is a NameObj: report presence; it has no LiveActor alive/hidden flags.
  `WorldMapPointParts` names a translation unit/holder, not a LiveActor class.
- Draw count is distinct point indices whose actor or registered marker/galaxy child
  model reached a nonhidden J3DShapePacket draw that frame. This is submission evidence,
  not a count of rasterized/onscreen nodes. Culls and camera state explain differences.

Bounds:4096 recent DL records,8192 model names,1024 node model associations,16 worker
commands,32 detailed worker reports. Retention bounds/overflow are explicit. No full
model or FIFO payload is saved. PCs and paths belong in private runtime logs only.

## Interpretation

A worker offset inside inline span gives original DL-relative offset. Offset before
inline start identifies preflush. Zero containing-seq means direct commands, a command
published before its span was finalized, or an overwritten ring entry; do not assign it
the most recent producer automatically. Previous producer state is asynchronous.

`target` is CameraViewInterpolator::mTargetPosition, the watch point sent to
CameraContext by the regular world-map CameraDirector path; `forward`/eye/up are actual
active-context axes. Temporary wipe/minimap cameras can use a different target. Samples
are normal GameScene draw boundaries, not per render-pass camera captures.

Capture the original failing build first if scheduled; apply the one-line fix separately
for a comparison. Report raw command, original caller/name/size, relative offset, preceding
VCD/VAT/vertex-size and base state. The static truncation is established; do not infer
that it caused the user's failure without an affected caller in the capture.
