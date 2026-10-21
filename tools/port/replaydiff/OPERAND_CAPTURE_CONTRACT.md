# Yoshi orientation operand contract v1

Static review only. No executed writer, operand equality, FP cause or source
defect has been established. Target post-endFrame trace rows are 887–889 and
903–904. The sealed first-cause packet remains authoritative for raw888 and
default904, with the original-GX742 departure a separate provisional comparison.
No NI1-revert arm is part of this contract.

`operand_sites_v1.json` pins the retail DOL, native059e binary, native6b78 source,
Dolphin409881c3 source, full candidate-function hashes and reviewed call/return
instruction hashes. `operand_contract.py` independently checks the immutable
DOL and optional native binary **offline**. It launches nothing, acquires no
lock and installs no breakpoint. The `ReadPort`/`passive_snapshot` component
only reads an already-stopped transport supplied by a future owner; it cannot
connect, continue, step, add breakpoints, write registers/memory or call guest
functions. Synthetic tests are not runtime acceptance.

## Bound write candidates

The native observer's actor_up is **MarioActor::_270**, not Mario::mHeadVec.
Retail actor+0x618 is bound by original argument setup and writes; front is
Mario+0x208. Native member offsets differ: the frozen native front normalizer
uses Mario+0x21c, while the camera fallback uses actor+0x7f4 and target+0x824.
Use actual ABI argument pointers or verified native type/layout metadata;
header offset comments and retail offsets are not native layout evidence.

| Candidate | Retail binding and original operation | Source at6b78 |
|---|---|---|
| Camera update |803c6670..67d7; BL803c6788→8003f840 writes actor+618, conditional copy803c679c→8000e740; counter+660 decremented at6724/6728, target+648 copied from Mario+1f0 at66fc, timer from table+7a0 |MarioActor::updateForCamera |
| Forced matrix |803bdb40..dbff; BL803bdbac→8000e260 extracts matrix Y into actor+618 |MarioActor::forceSetBaseMtx; **not fn_803C5820** |
| Access override |803b5560..5617; BL803b55e0→8000e740 copies Mario+1f0 into actor+618 after KeepUp |MarioAccess::setFrontVecKeepUp |
| Camera reset |803bfbb0..fc03; BL803bfbdc→8000e740 copies target+648 into+618; clears+660/+662 |MarioActor::fn_803BFBB0 |
| Initial placement |803b7bb0..7ccb; BL803b7c30→8000e740 copies target+648 into+618 |MarioActor::initAfterPlacement |
| Initial member zero |803bef00..f6eb; BL803bf07c→8000dbb0 zeroes actor+618 |MarioActor::initMember in MarioActorSMG2_A.inc |
| Front assignment/normalization |8038b000..b153; copy BL sites b108,b118,b128→8000e740, then mandatory BLb130→8003e500 |Mario::setFrontVec |
| Constructor front |80386dbc/dc0/dc4 store Mario+208/+20c/+210 |Mario constructor |

This enumerates reviewed direct actor-up writers in the selected MarioActor/
MarioAccess sources and the central front writer. It is not a whole-program
alias/write-coverage proof. Other state classes have similarly named mFrontVec
members; do not mistake MarioSwim's or MarioMove's member for Mario's. Generic
helpers taking vector references may write aliases. An unexplained write or
missing expected invocation is a coverage HOLD, not evidence of no execution.

The bound front setters call through KeepUp8038b3d0/KeepSide8038b540 and other
callers; upstream head producers include setHead8038afc0, gravity fix803898a0,
head blend803b23e0 and quaternion head803b1ef0. In particular,803b2414 calls
vecBlendSphere before normalizing head at803b2448. Their presence in the
candidate set does not show which one executed at888 or904.

## Smallest useful pre/post observations

At803c6788, r3 is destination=actor+618, r4 aliases that same old-up vector,
r5 is target=actor+648 and f1 carries the rate in PS0 binary64. Stop **before**
BL; preserve both vectors and destination bytes before the aliased write.
Stop at803c678c after the original return and read the saved destination,
r3 boolean return, and unchanged context. Capture actor/Mario identity through
the canonical holder, head/side/front vectors, target, counter/timer, remote
matrix flag and the caller chain. Sample camera entry803c6670 as a separate
context point if counter reload/rate production needs localization. A zero
counter or remote flag may bypass this blend; observe bypass instead of
assuming the site occurred. Conditional fallback pre/post679c/67a0 distinguishes
blend output from the subsequent overwrite.

At8038b130/8038b134, save r3 front pointer and its12 bytes before/after the
original normalization; do not read post-call r3 as the original pointer.
SetFront entry8038b000 captures `this` r3 and argument r4 before clobbering.
Copy pre/post b108/b10c,b118/b11c,b128/b12c identifies the chosen source vector
and whether its first difference already predates normalization. KeepUp
side-normalize sites b49c/b4a0 and b4b0/b4b4 are the next bounded expansion if
front input differs. Capture only actually reached sites, with nesting/caller
identity, counts and raw bits. Never fill an unvisited site with zeros.

For forced-matrix Y extraction use803bdbac/bdbb0: r3 matrix48 bytes, r4 saved
output. For direct copies above, preserve r3 output/r4 source before BL and
read the saved output at PC+4. This avoids guessing which alternate writer
last supplied the seam value.

## Native bridge and FP preservation

In binary059e, file virtual addresses **1016ca3a4/1016ca3a8** bracket the camera
BL to stub102db1bb8. Mach-O indirect-symbol metadata resolves the stub to
`MR::vecBlendSphere(const from&,const to&,dst*,float)`; native x0/x1/x2 are
from/to/dst and s0 is f32 rate. Retail8003f840 is the **output-first** implementation.
The native wrapper at100be50b4 forwards to output-first10226b58c. Capturing or
comparing x0 as destination would be wrong. File addresses **1015132cc/1015132d0**
bracket front normalization to1022583ac, with x0 pointing at the in-place vector.
All addresses require actual module-slide resolution, verified stopped PC and
binary hash; no copied file address is a live process address by itself.

The immutable binary's offline DWARF camera lookup returned no DIE. Typed
member access is therefore **not demonstrated**; do not borrow current build
objects/debug maps or evaluate getter expressions in the inferior. ABI-level
argument captures at the reviewed BL sites remain usable without DWARF. The
minimum native bridge is an external debugger observing those two call/return
pairs and raw argument memory. Runtime attachment/permission, hardware-execute
breakpoint capacity and stop/resume behavior are UNACCEPTED. Native software
breakpoints normally write target instruction bytes; they must not be silently
called a no-write observer. Require reviewed hardware-execute support or a
separate explicit instrumentation decision; no Game source hook or fallback
is implemented here. Existing headless post-endFrame hooks cannot provide
intra-frame operands.

Read native FPCR **and FPSR** before and after each passive observation, using
debugger register reads. Copy integer bytes only; no target function calls,
expressions, float arithmetic/formatting, FZ toggles, FPSR clearing or restoring
a guessed state. Existing FloatingPointMode restores FPCR only; State::real
is not a sufficient FPSR-preservation mechanism for a new inline observer.
Actor trace has a separate status guard, but that does not authorize borrowing
it as a complete intra-function context saver. For any later native in-process
provider, an integer-only fixed buffer and an audited full FPCR/FPSR guard
would be required, with formatting outside gameplay and bitwise main-trace
negative controls. That provider is not implemented or authorized by this file.

## Dolphin bridge and mode/read-group constraints

Pinned GDBStub.cpp reads `p20..p3f` as FPR PS0 binary64 and `p46` as FPSCR;
PS1 is unexposed. Individual p/m accessors directly read stored PPC state/RAM.
Never use g/G, injected mffs/stores, guest function calls or P/M writes for this
observer. Raw binary64 f1 must be retained; compare its single-precision value
only after proving exact f32 representability, not via host rounding/FZ.
The entry FPSCR4 receipt is not the mode at these sites: observe p46 there,
with PC/phase. Equal vectors with unknown mode are not an FP-cause proof.

Dolphin's execute Z0/Z1 uses its breakpoint table, not guest instruction patching;
Z2..Z4 use memchecks. The source supports memchecks but their runtime pre/post
PC semantics, range coverage and JIT effects are not accepted. Do not assume
a write watchpoint is an exact pre-store operand sample. The smallest reviewed
route uses original BL pre/post PCs, temporarily removing/rearming the pre
trap and preserving all existing input/seam traps; no PC/LR modification.
No installer or resume dispatcher is supplied in this change.

A later collector adapter must dispatch existing input.handle **first**, then
operand stops, then the normal seam/unexpected-PC path. It must retain
`deferred_stop` and seam+4 diversion handling, service input traps during nested
calls, and never count operand stops as endFrame or consume a tape row. After
accepted row k, events before the next accepted seam belong to producing row
k+1. Thus arm after row886 for887..889, and after902 for903..904; record producer
source frame, latest accepted row, producing row, input frame=producing row−1,
read-group progress and an invocation stack. Match pre/post by thread/stack/
callsite nesting and saved output pointer; return registers alone are unsafe.
Local addresses are continuity only, not cross-run object keys. Different
invocation counts, caller paths, FP modes or aliases are explicit HOLDs.

ReadPort verifies every exposed register before/after a stopped observation;
it preserves bits including NaNs/subnormals without conversion. This does not
prove PS1 state equality or global atomicity of other emulator workers. Pauses
and debugger instrumentation can alter wall-clock async behavior; require the
unchanged main trace and input/read-group history as control evidence, with
observer overhead disclosed. Keep pending_state unavailable, qualification
BLOCKED, and original recording alignment unproven.

The discriminator is conditional: if saved operands/aliasing/mode already
differ, follow their earlier producer. If complete reviewed input/read sets
and mode agree but output differs, localize an operation/instruction-sequence
difference and test that primitive separately. Neither outcome alone proves
that arithmetic caused the old recording departure or failure to clear.

Native frame binding can use a **memory read**, not an inferior call, of
`gSourceFrame` at file address10374c060 plus verified slide. Driver::wait(true)
stores the next mSourceFrame after the preceding seam; bind it to the actual
sync origin and reject bootstrap/unknown context. Replay::consume entry at
file1025edf58 is a possible extra read-group observation point: x0=this,
w1=channel,w2=capacity. An external per-thread counter can reconcile each call
with the complete tape, without calling methods or guessing private member
offsets. This read-progress bridge is **not implemented/runtime accepted**;
argument-only captures cannot claim exact intra-frame read-group progress.
It requires another execution trap and must fit the verified hardware-trap
capacity or receive a separate native-only instrumentation decision. Do not
omit that coverage gap when comparing call invocations.

Source-only verification:

```sh
nice -n 19 python3 tools/port/replaydiff/operand_contract.py \
  --dol /private/SB4E01-extracted/sys/main.dol \
  --native /private/native-059e/petari_headless --out /private/fresh-sites.json
python3 -m unittest discover -s tools/port/replaydiff -p test_operand_contract.py
```

The pinned LLVM disassembler used during review does not reliably identify
Gekko paired-single save/load words (some print as VSX or unknown). No FP
instruction claim here relies on those labels: reviewed BL destinations and
integer register/address setup were checked against original words. Native
stub targets were resolved using Mach-O indirect symbols, not objdump's generic
`_wmemchr+...` labels.
