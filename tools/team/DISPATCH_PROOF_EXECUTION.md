# Dispatch correction: producer execution recipe and acceptance matrix

Status: PREPARATION ONLY. Canonical base and candidate are **UNBOUND**. This document
is neither a sanctioned external plan nor a certificate. It authorizes no build,
production verification or landing. Allocation and operator-reviewed plan pins are
required separately. L owns decoder/import implementation; P owns independent
producer evidence. Never execute producer scripts as importer authority.

## Inputs and hard stops before any heavy work

1. Wait for L's actual 5.3 landing and record its full commit as BEFORE; do not use
   a staging commit, anticipated main or old 6b78 baseline as the canonical base.
   GAME supplies a clean integrated candidate containing the sealed six 5.3 fixes
   plus the explicitly approved TU13/TU14 repairs. GAME resolves source conflicts;
   the overlapping TU13 EffectSystem correction must survive. Compare individual
   source/config deltas to the reviewed candidates and record every integration
   result. Source labels in old packets are advisory, not original ownership.
2. Pin BEFORE/AFTER full commits, all src/include/libs/config trees, source files,
   every changed path and exact symbols.txt changes. No JAS/Pad/factory/unapproved
   diagnostic additions. If PORT changes scope, stop for a new reviewed closure.
3. Generate an original canonical spec from the exact candidate source checkout
   with its local `main` pinned to BEFORE, using the reviewed command:

   ```sh
   python3 tools/team/remoteproof.py spec --output /ABS/NEW/dispatch-spec.json
   ```

   P independently captures all 2426 units, per-unit resolved flags/options,
   eight compiler versions where applicable, toolchain/support files, assembler,
   macros and generator/comparator hashes. Compare every field to L's independent
   spec. Do not edit a spec to reconcile it. Record actual before/after source
   Ninja graphs and generator outputs; symbols.txt changes can alter generated
   reference/split graphs, so no graph-equality assumption is allowed.
4. Separate external schema1 plans for TU13 and TU14, using dispatchproof.py's
   fields: schema/scope/bindings/units/symbols/references/symbol_map. The operator
   pins each canonical-JSON hash independently of uploaded receipt claims. Fill
   numeric ELF records only after isolated fresh candidate preflight is allocated
   and reviewed. New base observations must agree with the retained preflight or
   STOP. Unknown deltas reject before the full pair; never widen a plan afterward.
5. Before each lock/long phase write and mirror resumable STATE with host, exact
   source/spec/plan hashes, phase/output/PIDs and recovery action. Obtain explicit
   T2 release and PORT allocation; actual nonblocking POSIX native.lock acquisition
   must succeed. Exclusive nice19/j4 only when allocated; preserve >=50GiB free.
   Interrupted phases require new private outputs and a fresh clean pair, retaining
   failure evidence. No shared object output or dependency-object copying.

## Exact address/size and ownership contract

Original private DOL SHA256:
`abea6b0c498c4af90b17cfa08d54a01781186f097bee3e0761dbfb4838cbc281`.
No DOL/reference ELF/instruction dump belongs in a proof transfer.

The following are measured **6b78 baseline** ELF records, not an authorization to
reuse that baseline or predictions certified for a future integrated candidate.
Each execute has info=0x12, other=0, section=.text#0. Source paths are
`src/Game/System/WorldMapResultTU13.cpp` and `WorldMapResultTU14.cpp`.

| Scope/execute identity | BEFORE symbols.txt VA/size | AFTER VA/size | Baseline ELF value/size | Candidate expectation pending compile |
|---|---|---|---|---|
| TU13 GalaxyMap Start | 804F9710/8 | 804F9730/8 | 370/8 | 390/8 |
| TU13 GalaxyMap End | 804F9730/8 | 804F9710/8 | 390/8 | 370/8 |
| TU14 GrandGalaxyEnd Wait | 804F99D0/C | 804F99C0/8 | 240/C | 230/8 |
| TU14 GrandGalaxyEnd ZoomOut | 804F99C0/8 | 804F99D0/C | 230/8 | 240/C |

Names must be exact mangled execute identities in dispatchproof.SCOPES, unique
GLOBAL FUNC DEFAULT symbols. No wildcard prefix matching. Each symbols.txt record
must match its exact VA AND size; every other line, including line endings, stays
verbatim. Every other config file must be unchanged unless independently approved.

Baseline own .data#0 ADDR32 relocations (type1/addend0): TU13 End slot48, Start68;
TU14 Wait58, ZoomOut68. All 2426 nondebug object relocation summaries were searched;
these are the only named references. Future BEFORE and AFTER must independently
reproduce that closure. Resolve local/section-symbol targets too when checking
for effective aliases; a named-reference census alone cannot establish ownership.

Full original ownership, not spelling-based symbol matching:

| Owner | singleton | singleton ADDI PC | constructor BL PC -> constructor | vtable+8 | original wrapper -> behavior |
|---|---|---|---|---|---|
| TU13 Start | 807D5FD0 | 804F96A8 | 804F96B0 ->804F96E0 | 807031A0 | 804F9730/8 ->exeStart804F94E0 |
| TU13 Wait | 807D5FD4 | 804F96B4 | 804F96B8 ->804F96F0 | 80703190 | 804F9720/8 ->exeWait804F9530 |
| TU13 End | 807D5FD8 | 804F96BC | 804F96C0 ->804F9700 | 80703180 | 804F9710/8 ->exeEnd804F95B0 |
| TU14 ZoomOut | 807D5FE0 | 804F9978 | **804F9980** ->804F99A0 | 80703210 | 804F99D0/C ->setNerve804C4CC0(Wait singleton) |
| TU14 Wait | 807D5FE4 | 804F9984 | **804F9988** ->804F99B0 | 80703200 | 804F99C0/8 ->exeZoomOut804F9850 |

The old TU14 packet abbreviated ADDI9978/9984 as constructor calls; those roles are
incorrect. Producer selectors: TU13 init93A8/appear9480 ->5FD0, Start9514 ->5FD4,
Wait958C ->5FD8; TU14 init97B8 ->5FE4, appear9830 ->5FE0. Decode actual instructions,
bootstrap r13=807D7320, constructor stores and Spine vtable+8 dispatch. A wrapper
loads executor through **r4 (Spine)**, not r3; reject the wrong base register. TU14
12-byte wrapper also selects Wait via r13 before its unlinked tail branch. Preserve
all untouched producer/constructor/singleton/Wait joins. Nonmatching TU14 ELF offsets
must not be naively converted to linked DOL addresses by subtraction.

## Fresh pair and strict semantic comparison

Use a reviewed producer derived from the already exercised source-only graph
recipe, NOT the old fixed-name prove53_r1.py unmodified: that script asserts an
ordinary identity-only comparison and WorldMapPointParts control and therefore
cannot prove intentional dispatch deltas. Pin the new producer source/hash before
running. Its required sequence is:

1. Clean BEFORE output and dependency metadata; regenerate source-only graph from
   exact BEFORE; compile all_source with nice19/j4. Require rc0, exactly2426 unique
   expected ELFs, complete hashes, graph/options/tool hashes, clean tracked source.
2. Independently clean AFTER and dependency metadata; regenerate actual AFTER
   graph, compile all2426 with the same checks. No incremental or cached substitute.
3. Compare all **other2424 objects** with occurrence-keyed objsame, no exceptions.
   Runtime-relevant (allocated/nondebug) section data, relocations, symbol binding,
   visibility, size and effective target/addend records stay strict. Debug metadata
   policy is exactly the reviewed comparator's; do not invent a wider exclusion.
4. For each scoped object, independently check approved actual symbol records and
   owning reference closure, then normalize ONLY the two reviewed named symbol
   value/size fields in an in-memory semantic summary. Every remaining section byte,
   symbol record and relocation offset/type/target/addend stays identical. No body
   mask, offset translation, data/pool waiver or physical code change is permitted.
   Raw ELF symbol indices may renumber; effective named/local target identities
   remain strict. No normalization of arbitrary aliases is permitted.
5. Join actual compiled producer->singleton->constructor->vtable+8->execute body
   with exact phase symbols config and private original chain. BEFORE must exhibit
   the known two wrong dispatches per TU, AFTER the exact two original-correct
   dispatches. A symbol-only normalized comparison is insufficient. Preserve the
   TU13 Wait chain and every unrelated join. Do not treat baseline as a positive.

## Actual controls and acceptance matrix

Every semantic mutation below operates on fresh real objects/config artifacts in
isolated evidence copies and must reach the relevant component before outer digest
rejection; final archive verification separately checks original hashes. Synthetic
fixtures are useful but cannot discharge these rows.

| Case | Required outcome / acceptance boundary |
|---|---|
| Private original DOL full decode | Exact pinned DOL and five owner chains pass component checks; no production PASS implied |
| Known-bad baseline dispatch | Original-ownership correctness gate rejects; structural baseline census is still valid |
| Fresh integrated candidate | Only reviewed two-field-per-TU envelope, all other2424 strict, exact reference closure and original-correct joins |
| TU13 Start/End wrong or crossed owning target | Reject semantic ownership/target check |
| TU14 Wait/ZoomOut wrong or crossed owning target | Reject semantic ownership/target check |
| Keep old TU14 sizes after VA swap | Reject exact config and ELF size records |
| Correct size but wrong VA, or correct VA but wrong size | Reject independently; no address-only interpretation |
| Third/external/local alias effective reference | Reject full-tree closure; retain exact offending unit/section/offset/target |
| Missing/duplicate owning execute or slot reference | Reject uniqueness/closure |
| Nonzero relocation addend, wrong type, source offset or target | Reject exact relocation semantics |
| First text byte outside execute identity changes | Reject unchanged-remainder comparison |
| Data/pool/other section byte changes | Reject; no data or constant-pool waiver |
| Outside symbol value/size/type/binding/visibility changes | Reject unchanged-remainder comparison |
| Wrong wrapper load base register or branch kind/target | Reject DOL/compiled ownership decoder; no alternate permissive pattern |
| TU14 old packet ADDI presented as BL | Reject exact opcode/role contract |
| Stale/wrong source, symbols config, graph, tool, DOL or plan pin | Reject provenance/contract independently |
| Actual Wii-active source perturb in TU13 | Compile changed TU into fresh output; comparator rejects actual changed bytes |
| TU13 source restore | Fresh compile restores exact original candidate raw object SHA and passes component checks |
| Actual Wii-active source perturb in TU14 | Separate fresh compile/rejection; do not infer from TU13 control |
| TU14 source restore | Fresh compile restores exact original candidate raw SHA and passes again |
| Missing object, manifest entry, provenance or certificate | Reject full-package validation |
| Process interruption or failed compile/control | Preserve evidence; no fallback or certificate, start new fresh pair |

A Wii-active source perturb may add a reviewed uniquely named external constant to
the scoped CPP, changing allocated data without touching the sanctioned symbol
records; save exact bytes, compile and capture original/perturbed/restored hashes.
Choose and pin both perturbations before execution. Their rejection cannot be
claimed from syntax checks, synthetic fixtures or copied candidate objects.

Package transport for extra TU14 controls must be explicitly reviewed with L:
retain the official schema1 archive layout (2426+2426+3 primary control ELFs) and six
provenance fields unchanged. Additional real-object/control artifacts require a
separately manifest-bound regular-file external evidence payload accepted by the
reviewed extension; do not silently increase archive members or omit the second
control. If no such transport is approved, packaging is BLOCKED, not waived.

## Packaging, server verification and separate runtime gate

Keep original canonical spec/certificate immutable. Full SHA256 manifests bind all
4855 primary archive members, exact source/config/tool/graph/plan hashes, structural
comparison, original ownership measurement, real control logs and external payload.
Six provenance keys remain before_log/after_log/control_log/recipe/unit_manifest/
spec_comparison. Ordered2MiB base64 parts with per-part/full encoded/decoded hashes
may use the established transport. Server L independently rechecks private original
DOL and effective ownership; producer helpers are not executed as authority. No
production import until L's reviewed fail-closed extension exists; no invented CLI
flags or success status from component-only results. Final contract recheck and
incremental Wii guard remain required; P never runs the landing portion.

T2's allocated TU13-only/combined native A/B determines runtime consequences and
FIFO/viewport symptoms separately. Preserve binary/source/overlays, exact normal
host route, settings/save-copy, input events and frame provenance. Wii dispatch
correctness does not by itself prove world-map causality or fix nestedCALL/indexedXF.
No renderer/input fallback, weakened fatal or forced state is part of this recipe.
