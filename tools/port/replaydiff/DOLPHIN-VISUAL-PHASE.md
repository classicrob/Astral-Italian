# Dolphin visual phase and shared sampling proposal

SOURCE PROPOSAL ONLY. No emulator, Game, worker, debugger, GPU, build, app or replay
was modified/run. Qualification remains BLOCKED with null qualified counts. The
checked-PNG leaf and native draw proposal are separate dependencies, not installed
providers. Dolphin source is pinned to `409881c34357d0f105fd473167d15ab0bd9c1628`;
original SB4E01 DOL SHA256 is
`abea6b0c498c4af90b17cfa08d54a01781186f097bee3e0761dbfb4838cbc281`.
`dolphin_visual_source_pins.json` binds primary public source files and small
retail instruction guards. Public source base:
https://github.com/dolphin-emu/dolphin/tree/409881c34357d0f105fd473167d15ab0bd9c1628
Native semantics remain bound to 33e71 in `NATIVE-DRAW-PHASE.md`. These are source
facts and proposed future observations, not evidence that any site executed.

## XFB copy, VI selection and PNG path

| Pinned source | Actual source behavior | Required distinction |
| --- | --- | --- |
| `VideoCommon/BPStructs.cpp`, display-copy trigger | Calls CopyRenderTargetToTexture with XFB format, source/destination/stride/height, gamma/filter/clamp; triggers after_frame_event; optionally ImmediateSwap | A display-copy event is only an approximate frame end. Multiple copies may form one XFB. It is not Game draw/update or VI selection |
| `TextureCacheBase.cpp::CopyRenderTargetToTexture` | Allocates/copies a render target, records XFB entry metadata, can queue RAM readback | Entry ID increments on allocation, not per Game draw. Destination address is not a cross-run key or unique copy generation |
| `TextureCacheBase.cpp::GetXFBTexture` | Reuses unlocked cached copies, or decodes RAM and stitches referenced copies; returns display rectangle | Need complete copy provenance, entry generation/content-lock state and all stitch references; address, hash or entry ID alone cannot name a draw |
| `Core/HW/VideoInterface.cpp::OutputField`, `VideoBackendBase::Video_OutputXFB` | VI-derived address/geometry/ticks enter an asynchronous ViSwap request after FIFO synchronization unless ImmediateXFB is active | Emulated VI field/ticks, CPU source seam and host presentation are independent. FIFO synchronization is not a PNG-completion proof |
| `Present.cpp::{FetchXFB,ViSwap,ImmediateSwap}` | Chooses and locks XFB content; tracks unique frame count separately from present count; repeated XFB may skip presentation/dumping. ImmediateSwap bypasses VI selection | Record actual reason, duplicate flag, XFB identity/references, present and unique-frame counters independently; preserve settings |
| `Present.cpp::ProcessFrameDumping` | Selects raw-XFB, aspect-corrected or window-derived target geometry, may round dimensions for encoder LCM, removes target black borders, then requests DumpCurrentFrame | Dump target is not automatically raw resolution or window screenshot. Record source/target rectangles and actual output policy, never infer crop/aspect |
| `FrameDumper.cpp::DumpCurrentFrame` | Scales when dimensions differ, copies into staging texture, stores ticks and frame-number state; marks work pending | Queued staging copy is not mapped/completed pixels. Pending state needs a matching per-dump identity, including overwrite/resize coverage |
| `FrameDumper.cpp::FlushFrameDump` | Waits previous encoder work, swaps staging buffers, Flushes, Maps and queues FrameData; FinishFrameData later waits and unmaps | There is a useful buffer-lifetime/completion seam, but no current guest draw-to-dump receipt |
| `FrameDumper.cpp::FrameDumpThreadFunc`, `Common/Image.cpp` | Screenshot flag consumes a frame, libspng encodes RGB PNG. Screenshot-completed event is set even if saving returned false; image counter advances independently | Request completion or a filename/counter is insufficient. Successful encode, flush/close, atomic final publication and hash need explicit evidence |

The present event's frame_count and the frame number sent to dumping are also
separate fields: ViSwap increments its unique-frame counter while constructing
PresentInfo, then ProcessFrameDumping receives the current member counter. Record
both actual values; do not manufacture an arithmetic offset to a Game frame.
The dump path samples selected XFB texture content, not the host OSD/chrome surface
or necessarily postprocessed scanout. Its scaling path explicitly lacks gamma-
correct scaling. Common/Image sets image dimensions/type but no explicit sRGB/gAMA
profile here. RGBA8 or PNG extension alone is not a color-encoding proof.
`IOFile` destructor closes after SavePNG returns; the return value does not carry
that close result. The existing path is unchanged by this work.

## Backend-specific completion

- Metal staging CopyFromTexture retains the command buffer containing its blit.
  Flush ends/submits encoders and waits if needed; Map is always true. Flush does
  not propagate final command-buffer error/status into FrameData before clearing
  its wait-buffer reference. A future receipt needs the actual completed/no-error
  result, not just Map=true or wait returned.
- OpenGL staging uses a GPU fence with persistent mappings, or a blocking mapping
  transfer without persistent storage. Flush's ClientWaitSync return is not
  captured here; Map may already be true. Capture actual completion/error status
  for the exact transfer/resource, not a blanket backend success assertion.
- Vulkan staging associates the transfer with a fence counter; Flush submits/waits
  or waits for that counter and invalidates CPU cache. Map is always true. The
  counter/result/error must be bound to the readback; no Map-only completion claim.

The source binding includes these implementations, not a claim about which backend
any historical run actually used. A future producer must record its actual backend,
configuration and binary/patch identity. No backend/core/DSP/timing changes are
permitted to make a capture easier. This is not a new diagnosis of any GPU fault.

## Retail-observable counterpart to native draw brackets

Original-DOL direct-call decoding plus symbol hashes establish these static sites:

| Site | Original address | What may be observed later |
| --- | --- | --- |
| GameSystem frameLoop draw call | `804b6a90` | Before BL to draw; R3 has the GameSystem argument. Matches proposed native after-beginRender/before-draw observation |
| Return from GameSystem draw | `804b6a94` | Next instruction before endRender call; matches proposed native draw-exit bracket |
| Return of getCurrentSceneForExecute inside drawScene | `804ba798` | R3 is the actually selected scene. Compare locally with current controller scene, record relationship rather than addresses |
| Accepted post-endFrame seam | `804be280`, LR `804b6ac4` | Entry to waitForRetrace after the frameLoop endFrame call returned; existing ready anchor remains separate from draw |

These are candidate read-only logical-breakpoint observation sites using the
existing pinned GDB bridge, not injected guest calls or writes. Preserve existing
GDB p-register limitations and all current acceptance guards. Do not use g/G to
claim preservation of unexposed PS lanes or modify FP state. Breakpoint stop does
not establish GPU quiescence, selected XFB age, timing equivalence or zero observer
overhead. No breakpoint/observer is installed in this task.

Use bounded typed/layout-guarded metadata: current scene/stage strings, **placed**
scenario at controller+0x80 versus selected scenario+0x84; init state, canonical
player and sampled scene epoch. Existing collector root/controller/scene guards
remain mandatory. Attach actual-selected-scene observation within the same observed
draw interval. Init6 alone can precede Normal and still select Intermission;
unready can still draw old Game. Preserve entry and exit snapshots separately.
Sample-derived pointer/key epochs are not authoritative lifecycle generations.
Draw/loop serials are local continuity tokens, never guest/host address IDs.

## Shared definition v2: proposed, not an admission change

The existing same-source-frame image gate is not changed or bypassed. The following
explicit alternative requires joint owner approval and producer implementation:

1. Each provider independently observes every post-endFrame source seam from zero.
   Anchor its first exact Game/init6, stage and placed-scenario seam. Preserve player
   validity at that anchor, natural RNG and source/sampled epoch; do not move a bad
   anchor to a convenient later row. Global boot origins need not match.
2. Schedule requests at that provider's actual anchor+60/+300/+600 source seams.
   Retain request-state snapshots. A request is not a claim those bytes were drawn.
3. Observe the **first next GameSystem draw scope** after each request using complete
   event history. Require matching eligible Game/player identity at entry/exit and
   actual selected scene. If that first scope is ineligible, missing or interrupted,
   BLOCK the slot; do not search forward, fit, rebase, delete frames or guess a lag.
4. For the first bounded pair support observed mode3 only: require current render
   flag true, valid drawing index, proven clear/writer lineage and its display copy
   after draw/endGX. EndGX console/assertion work is outside the draw bracket but
   before the copy; retain it in provenance. Missing copy or mixed/unbound EFB work
   blocks the slot. Never force buffer mode, visibility, camera or Game state.
5. Bind target copy content to actual unchanged-provider-policy selection, then to
   readback and PNG. The proposed Dolphin subset requires first matching nonduplicate
   VI selection with no stitching; Immediate/duplicate/stitched cases remain unsupported.
   Native latest-copy selection and Dolphin VI selection are not equal scanout times.
   A content pair can only claim the observed copy/draw relationship, not simultaneous
   display, equal update count, or shared clock. A missed/overwritten selection blocks.
6. Use a predeclared 600-frame canonical connected-idle channel0+Nunchuk suffix and
   **16 additional matched idle observation-tail frames**, solely to allow the request
   at +600 to reach a later draw/VI/readback. The tail is part of recorded input, not
   an alignment shift. Stop at the bound; no stitching or cap lift. If pending work
   cannot complete, retain an incomplete slot. Existing 600-input/601-row artifacts
   cannot supply this future draw automatically and must not be relabelled.

Keep draw count, update count, source-seam count, VI field/ticks, unique-present,
composition and dump counters separate even if numbers happen to match. Missing
counter/phase data is unknown. Preserve actual KPAD read ordinals/samples/connection
state and the complete original export including pre-anchor history. Verify actual
suffix records against the canonical 616-frame input using each actual anchor's
explicit source mapping; no on-disk record rewrite or probe-state substitution.
An input-delivery mismatch is not a gameplay or visual divergence.

## Remaining owners and smallest real-pair allocation

There is no current complete provider on either side. Required owner work:

- GAME supplies the native-gated GameSystem draw brackets (and loop-entry point if
  old-flag interpretation is needed); PORT implements detached typed native snapshots.
  Authoritative lifecycle identity remains a separately reviewed contract.
- PORT owns a separately reviewed optional Dolphin host receipt bridge joining
  retail read-only draw observations to ordered XFB copy provenance, selected entry/
  references, dump/staging identity and final publication. CPU/GPU dual-thread order
  cannot be inferred from clocks or addresses. This bridge and its ordered transport
  are **not designed as an installed worker patch here**; no emitter exists yet.
- PORT must bind backend completion/error status, output geometry/color evidence and
  checked publication. Any instrumented Dolphin build gets its own executable/source/
  patch hashes; upstream409881 is its base, not a substitute binary identity.
- L/PORT lifecycle review and explicit runtime allocation precede any fixture/provider
  launch. Checked-PNG runner remains hard-gated; its old encoder tests are not shared
  process-lifecycle or new native-control qualification.

After these reviews, propose one Flip-Swap scenario1 initial-scene pair using the
already supported original-string entry route, actual mode3 and unchanged settings.
First allocate small GPU synthetic transfer/overwrite/resize/error/color controls;
then allocate one continuous **616-input + ready-row** capture per provider, three
image slots, natural seeds and complete export/receipts. Suggested requested runtime
caps are existing Dolphin480s and nativeGX120s, excluding a separately allocated
instrumented build. These are stop bounds, not performance estimates or permission
to run. If a bound or supported-condition gate fails, report incomplete and stop.
No human tape, Yoshi divergence, worldmap evidence or original saves are involved.
A phase-v2 adapter and independent review are still needed before the existing
visual comparator can consume such receipts; image diagnostics cannot imply gameplay
or timing equivalence. Actual GPU/frame/color qualification stays BLOCKED.

## Offline tool and controls

`dolphin_visual_contract.py` verifies source/DOL/symbol hashes and static guards;
with optional synthetic evidence it checks ready/epoch history, original actual
idle-read export, next-draw selection and copy/VI/dump/readback/PNG links. A fully
consistent fixture produces only `PROPOSAL_CHAIN_CONSISTENT` and a proposal-only
candidate, NOTMEASURED/BLOCKED, null qualified counts and no accepted runtime provider.
It cannot create a real receipt by asserting fixture fields. Missing links, shifted
requests, later-draw substitution, selected-vs-placed confusion, input errors,
Immediate/duplicate/stitched XFBs, wrong IDs, Map-only completion, unchecked PNG close
or unknown color all block. It never launches or patches anything.

## Tightened stop and original-export witnesses

The source validator now requires the final observed source seam to be exactly
anchor+616, not merely at least that far. A separately pinnable CPU-stop artifact
records that seam, terminal source counter, zero resumes and no emulation advance
after stop. Draw entry/exit and copy are witnessed between the actual request seam
and its observed adjacent next seam. VI selection carries its originating source
seam within the bound. GPU/readback/PNG completion may occur later in host event
order only under the unchanged no-emulation-advance stop witness. This is observed
seam adjacency, not an image lag or fitted +1 alignment.

Input proof now requires original raw-export identity plus a separately pinnable
closed collector receipt and a read census covering every source seam0..stop,
including zero-read seams. The entire original file is checked for ordered frames,
dense per-frame read ordinals and census counts; only then is the canonical suffix
compared. Removing the original prefix cannot be hidden by updating the working
export's hash/counts. Output claims only hash-verified equality to the pinned
original and complete per-seam census agreement; it no longer asserts unconditional
original_prefix_preserved or records_rewritten booleans. Checksums do not authenticate
a maliciously rewritten collection of all producer artifacts. Runtime producer
code/receipt review and qualification remain absent.
