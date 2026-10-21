# Renderer FIFO first-fault capture recipe

Status: source-only design, no build/runtime or parser behavior change. P owns this
isolated recipe. T2 retains the heavy slot; GAME owns EnterGalaxy/J3D lifetime.
Shared parser/FIFO/API integration needs PORT ownership approval first. This is a
diagnostic contract, not authorization to recurse, skip, recover or invent arrays.

## Source pins and observed failure

Aurora declared pin `08122911e8621acb7ded6563813b264bec1494b5`:

- [command_processor.cpp](https://github.com/encounter/aurora/blob/08122911e8621acb7ded6563813b264bec1494b5/lib/gx/command_processor.cpp)
  SHA256 `ff3eeb1c06562c0cbbeb5cbad0daa45aeb4370733dd073ccd34f10f6303ba5ab`.
- [GXDispList.cpp](https://github.com/encounter/aurora/blob/08122911e8621acb7ded6563813b264bec1494b5/lib/dolphin/gx/GXDispList.cpp)
  SHA256 `4fbb909532e524f8a0d9d8d5115e7cac1b69293dcfc7bcff6f2b90a8e5517a1c`.
- [regs.cpp](https://github.com/encounter/aurora/blob/08122911e8621acb7ded6563813b264bec1494b5/lib/gx/regs.cpp)
  SHA256 `f2f005b4e7937db643feb21d8d3279114ac01f87bfcdedf65c83bbbf315a15d3`.

Dolphin exact oracle revision `409881c34357d0f105fd473167d15ab0bd9c1628`:
[OpcodeDecoding.h](https://github.com/dolphin-emu/dolphin/blob/409881c34357d0f105fd473167d15ab0bd9c1628/Source/Core/VideoCommon/OpcodeDecoding.h),
[OpcodeDecoding.cpp](https://github.com/dolphin-emu/dolphin/blob/409881c34357d0f105fd473167d15ab0bd9c1628/Source/Core/VideoCommon/OpcodeDecoding.cpp),
[CPMemory.cpp](https://github.com/dolphin-emu/dolphin/blob/409881c34357d0f105fd473167d15ab0bd9c1628/Source/Core/VideoCommon/CPMemory.cpp),
[XFStructs.cpp](https://github.com/dolphin-emu/dolphin/blob/409881c34357d0f105fd473167d15ab0bd9c1628/Source/Core/VideoCommon/XFStructs.cpp).

T2's private normal-host-route-diag-fgbukn_h app.log17713/17714 reproduces nestedCALL
warning followed by unmappedarray24 on a 5878-fixed build. Existing camera/route
markers lack raw command, worker byte offset and producer DL address/size. The
warnings cannot identify a caller, genuine nesting, lifetime defect or malformed
primitive by themselves. Host route brackets are not consumed-KPAD timestamps.

The extracted Aurora download has no nested Git checkout: `git rev-parse HEAD`
walks into the outer project and is not Aurora provenance. Compare exact source
hashes and the actual build.ninja compiled edge through native generated patches,
not whichever generated copy is nearest. Preserve binary and generated-file hashes.

## Parser contracts that drive the capture

| Concern | Aurora081229 | Pinned Dolphin | Evidence implication |
|---|---|---|---|
| Nonprimitive opcode | command_processor.cpp216-224 masks every byte with0xF8 before switch | OpcodeDecoding.h switches exact command; primitives only80..BF | warning CALL may be raw40..47; indexedD may be38..3F. Record raw byte BEFORE mask, not just decoded name |
| Primitive payload | calc_vtx_size356-379; draw_prim491-509 uses current/cached VAT-dependent size*BE16 count | OpcodeDecoding.h228-247 checks3+count*vertex_size and RunVertices asserts consumed size in cpp126-135 | preceding primitive boundary and active VCD/VAT state are essential; stale size can interpret float payload as commands |
| CALL_DL | GXDispList.cpp49-62 flattens producer bytes into FIFO; parser278-282 skips eight operand bytes with warning | h202-212 takes exact40, nine bytes total, address and size rounded DOWN32; cpp143-194 enters outer list once, warns/ignores genuine nested list | worker needs producer sidecar to recover original list scope; process() input span is not a DL boundary |
| Indexed XF | cpp251-274 maps A/B/C/D to GX matrix/light array21..24; BE16 index + BE16 count/dest | h177-199 maps exact20/28/30/38 to CPArray12..15; index16,dst12,count4+1 | Aurora array24 and Dolphin array15 both mean light source; different enum domains are not evidence of an off-by-nine bug |
| Array binding | regs679-704 logs raw CP A0..AF unsupported; B0..BF stride low8. handle_aurora593-611 consumes native64bit pointer/size/endian extension | CPMemory185-192 masks physical32bit base and low8stride; XFStructs259-295 reads base+stride*index then BE32 XF words; preprocess298-309 snapshots data | distinguish raw physical CP path from native extension; record both histories and effective data/size/stride/endian before failing read |

Native extension packets must be recognized during offline boundary reconstruction;
feeding the mixed native FIFO directly to an unmodified Dolphin decoder is invalid.
Capture their raw subtype/length and applied state, not a guessed hardware opcode.
Aurora direct XF header uses high16+1 while Dolphin bounds/masks4bits; record the
full header/actual consumed count rather than silently applying either to evidence.

## Minimum bounded capture: three joined records

All records share run UUID, binary/source/patch hashes, thread role, sequence and
**absolute stream byte positions**. A frame ordinal adds context but never substitutes
for byte identity. Use existing FIFO synchronization; no timing sleeps/new drain,
parser cursor changes, game traversal on FIFO worker, or unsynchronized cross-thread
pointer ring. Diagnostics may perturb timing; preserve the pristine failed run.

1. **Producer-list sidecar**, inserted after pending GX dirty/flush bytes are emitted
   and immediately around the original GXCallDisplayList write. Record sequence,
   original source address64, exact supplied byte length, list-recording flag,
   absolute FIFO [start,end) of the copied bytes, producer thread/frame, bounded
   caller PCs (symbolize offline against preserved binary). Record separate stream
   ranges for dirty/flush bytes; do not attribute them to the list. Resolve symbols
   or class/model on producer-side approved seams/offline, not by worker dereference.
   Register this sidecar BEFORE publish can expose the bytes. Store immutable bytes
   from the FIFO copy while valid; do not reread a possibly freed source later.
2. **Worker command ledger**, at process entry/each command and completion. Join
   process span to absolute position: process_to's processed + reader offset, with
   stream base/published target and worker start/end retained across DrawDone/token
   returns and compaction. Save raw opcode, masked classification, opcode start,
   operand/payload end, remaining span and overlapping producer sidecar(s). CP
   records include address/value and applied VCD/VAT/array transition sequence.
   For each primitive save VAT id, primitive, BE16 count, VCD descriptors, all selected
   VAT fields/each attribute byte contribution, lastVtxFmt/lastVtxSize/cache decision,
   computed bytes, payload [start,end), actual cursor after take and last size-cache
   invalidation command. Include matrix-index and NBT3 multi-index contributions.
   Record state only; independently recompute offline, never call a state-mutating
   size helper solely to obtain a diagnostic value.
3. **First nested-warning and first indexed-XF fault snapshots**, before original
   skip/assert. Record raw opcode and exact nine/five command bytes IF in-range,
   absolute command offset, original-list-relative offset/remaining boundary, and
   the preceding complete primitive/CP changes. For indexedXF record full BE32
   operand, index,destination,count, selected array enum, pointer64,size,stride,endian,
   computed source offset/byte count and bounds predicate. For CALL record original
   raw32 address/size and independently labelled Dolphin-aligned values. Never
   dereference a CALL target or unmapped array just to log. Leave original warning,
   assertion and fatal path intact; do not suppress it after capture.

Bounded implementation target: worker ledger64 completed commands plus two previous
primitive state snapshots, producer sidecars256 entries, private raw-byte ring1MiB;
maximum two frozen snapshots and2MiB total raw payload per run. Capture from a known
command/list start through the trigger, including the entire immediately preceding
primitive when it fits. If relevant history/bytes/sidecar is evicted or exceeds cap,
record exact truncation/LIMIT and do not claim a complete reconstruction. A small
hex window around the fault without preceding-state provenance is insufficient.
Worker freezes diagnostic copies while the existing buffer lock/range is valid;
producer ownership metadata needs separately reviewed synchronization before code.
File I/O/stack symbolization must not occur per vertex or while holding FIFO hot
locks. Preallocate fixed storage, flush only bounded snapshots on first trigger,
ensure output completion before the existing fatal, and document any unavoidable
perturbation. No self-deadlock or recursive allocator/GX logging calls.

Raw FIFO/list bytes and game addresses stay in private gamedata mode700 directory /
mode600 files. Only small source/field/hash/offset summaries travel to SERVER; no
raw gameplay bytes, DOL/reference data or captures in git/chat attachments.

## Distinguishing outcomes without guessing

| Measured outcome | Permitted conclusion |
|---|---|
| Raw opcode is41..47 or39..3F | Current masked parser classification differs from exact Dolphin; does not yet prove why cursor got there |
| Independent full-state decode puts warning inside preceding primitive payload | Command desynchronization localized; report earliest boundary/size divergence and per-attribute difference |
| Primitive payload ends outside originating DL but parser consumes next span | List/copy length or command boundary failure; identify exact crossing and producer, not nested semantics |
| Exact40 at a proven in-list command boundary, complete operands, original outer-list scope established | Genuine nested CALL candidate; pinned Dolphin ignores it. Do not recurse; inspect next real boundary separately |
| Exact38 at proven boundary with valid preceding state and raw CP AF binding but no native light pointer | Raw-array setup path gap candidate; physical/host mapping must be independently bounded, not invented |
| Exact38 with previously valid native extension binding that changed/nullified | Report last binding transition and chronology; lifetime ownership remains GAME's investigation |
| Snapshot lacks prior VAT or original list coverage | INCONCLUSIVE; request one narrower reviewed capture, no parser repair justified |

## Integration and acceptance before any runtime request

Prepare a separate scratch diagnostic generator only after PORT assigns hook-file
ownership. Input must be the exact actual generated source edge with SHA pins and
unique-match anchors; output only to an owned diagnostic build tree. Apply hooks
without changing cursor, packet decoding, publication, array state or fatal policy.
Do not overwrite T2 active output or shared downloaded Aurora. Keep diagnostics
inert by default behind one explicit env var. This recipe itself adds no hook.

Source-level review must prove original command/reader operations unchanged. Later
allocated tests: diagnosticsOFF byte/log behavior; synthetic exact40 versus masked
41, indexed38 versus39, truncated packets, preceding primitive size mismatch,
64command/256sidecar/ring eviction, compaction/DrawDone resume absolute offsets,
publication-sidecar ordering, absent output path and fault flushing. These are
PENDING; no C++ compile or runtime was performed to write this recipe.

First capture remains normal unlocked host route with exact source/binary/settings
and bounded T2 allocation; TU13-only/combined A/B takes priority. Preserve results
individually and do not conflate camera repair with FIFO repair. The dispatch proof
recipe remains UNBOUND until L's actual5.3 landing and has no renderer exception.
