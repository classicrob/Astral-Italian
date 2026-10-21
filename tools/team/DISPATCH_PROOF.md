# Dispatch comparator scaffolding

This branch does not change remoteproof.py, proofland.sh, the certificate format, game source, or symbols.txt. Nothing here authorizes a landing. `production_status()` always returns PENDING with acceptance false. A component match is not a proof PASS.

Run the asset-free tests at low priority:

```
nice -n 19 python3 -m unittest tools/team/test_dispatchproof.py tools/team/test_dispatchownership.py
```

The synthetic big-endian ELF fixtures exercise the actual objsame ELF reader, section payload comparison, named symbol records, and relocation normalization. They are not substitutes for mutations of the real candidate objects.

## Proposed external plan schema 1

The caller supplies an independently reviewed SHA256 over canonical JSON (`sort_keys=True,separators=(',', ':')`). Never obtain that trust pin from the receipt being verified. Unknown top-level fields reject. A plan applies to ONE scope only, TU13 or TU14. A later combined verifier must invoke separate reviewed plans and still compare every other object strictly.

- `schema`: 1.
- `scope`: TU13 or TU14.
- `bindings`: exact keys base, tip, source_trees, source_files, config_files, toolchain, generators, graphs, dol_sha256. source_trees contains before/after src/include/libs/config IDs. source_files contains the exact one scoped CPP hash per phase. config_files contains exactly config/SB4E01/symbols.txt per phase. graphs contains both actual before/after graph hashes. Toolchain and generator maps must come from the original canonical spec. The importer must independently compute observed bindings from its source files/tools, not copy producer claims. All binding fields compare exactly.
- `units`: the exact original canonical unit paths. The component supports smaller fixtures; a production importer must require all 2426 actual manifest-bound objects in each phase, never a subset.
- `symbols`: exactly the two hardcoded execute names for the selected scope. Each has before/after `[info,other,section_occurrence,value,size]`. Only the pinned value/size fields can differ. Binding/type must be global function, visibility DEFAULT, and section occurrence fixed. TU13 size remains8; TU14 deliberately binds Wait12->8 and ZoomOut8->12. Actual object values must be supplied by P's fresh evidence and reviewed before the trusted plan pin is issued. Unexpected physical bytes/layout changes reject; no range masking exists here.
- `references`: exactly two `[unit,section_occurrence,offset,type,addend,"sym",target_name]` records, matching each owning table+8 slot. Both actual trees are searched across ALL supplied nondebug relocations for either execute identity. Extra, missing, crossed, duplicate, external or nonzero-addend references reject. Actual owning table symbols and compiled singleton/constructor joins are still pending qualification; the two numeric offsets alone do not establish ownership.
- `symbol_map`: those same two names, each with before/after `[original_VA,size]`. These VAs and sizes are independently hardcoded per scope. `symbol_map_check` extracts exactly those two records and compares every other line verbatim, retaining line endings. No other config change is permitted by this component. A production importer must independently check every other config file/tree too.

`classify_objects` takes actual ELF-derived semantic summaries, the plan, the trusted plan pin and independently observed bindings. It validates the complete reference closure, verifies both exact symbol records, normalizes ONLY the two after records in memory, and compares every remaining summary field without exceptions. It never edits an object, canonical spec or certificate. Its result is COMPONENT_COMPARISON_ONLY with production_acceptance false.

## Original DOL and ownership

Original DOL SHA256 is pinned in code. `dispatch-tu13-ownership-v1.json` contains only addresses, body/table hashes and the Start/Wait/End singleton -> constructor -> vtable -> wrapper -> body claims; its own file hash is pinned. No game bytes or instruction dumps are included. `audit_tu13_dol` reads the private local DOL, validates all pinned bodies/tables, and delegates to the now qualified bounded ownership decoder described below. It never returns proof acceptance.

The original r13/bootstrap/selector/constructor/Wait/Spine chain is now decoded in separate TU13 and TU14 contracts. Joining the actual fresh candidate symbols/relocations through the symbol map to those physical addresses remains PENDING. TU14 does not reuse TU13 ownership claims. Unknown instructions or unmapped/overlapping reads reject. The code must not be connected to production until these unfinished checks have real evidence and rejecting controls.

## Required work before import integration

1. P publishes fresh actual object deltas, raw/semantic classification and source/config/generated graph bindings. Review exact numeric records before pinning either plan. Retain sealed5.3 EffectSystem correction in any integrated candidate and use its actual new canonical contract.
2. Independently decode the complete original ownership chains and resolve actual compiled vtable/singleton/constructor/wrapper relationships. Verify two specific baseline mismatches improve to the original DOL targets; preserve Wait and all unrelated joins. No visual-causality claim follows.
3. Run real ELF mutations: wrong owning target, extra reference, body/data byte, outside symbol size/location/visibility, relocation addend and stale config/tool/source/DOL/graph. A hash mismatch alone is insufficient to claim semantic controls. Prove candidate restore compile byte-identical and passing again. The current restoration helper only checks supplied bytes; it does not prove compilation occurred.
4. Review fail-closed integration with full schema1 archive/certificate/six provenance and an explicit operator opt-in, mutually exclusive with Mario52. No producer scripts executed, old canonical spec edited, broad native exemption or whole-TU waiver. Existing final contract recheck and nj guard stay required.

No real candidate compilation, original-DOL audit, native.lock acquisition, production verifier execution or landing was performed to produce the synthetic test result.

## Qualified bounded ownership decoder

`dispatchownership.py` now implements both separate original-DOL chains. `decode_original_dol(data, scope)` requires the exact private original DOL SHA, then verifies r13 LIS/ORI bootstrap; init/appear (and TU13 Start/Wait) selectors and their linked initNerve/setNerve calls; sinit argument ADDIs and distinct constructor BLs; each constructor's vtable materialization/store; owning execute slot+8; exact executor load from Spine in r4; and the size-specific wrapper tail branch. TU14 additionally requires init's dead flag and its twelve-byte wrapper selecting the distinct Wait singleton. It binds TU14 argument PCs9978/9984 separately from actual BL PCs9980/9988. The earlier packet's mislabeled call positions reject.

The shared decoder follows initNerve's preserved argument into Spine construction, the executor's Spine field, setNerve's pending-nerve store, changeNerve's pending-to-current transfer/reset, and update's actual current-nerve vtable/slot8/CTR call with Spine in r4. This is a bounded instruction/dataflow verifier for the pinned chain, not a general PPC emulator or a whole-program behavioral proof. Unknown edge forms reject; the production entry also requires the entire original DOL hash. It emits only semantic addresses/scalars, never instruction bytes.

`decode_object(path, scope)` resolves the existing ELF's actual singleton/sinit/constructor/vtable/execute relationships, including effective local/section aliases for ownership relocations, and reports semantic method mismatches. It distinguishes pre-link SDA21 RA=0 from the original linked r13 instruction; it requires the exact relocation to the owned singleton. It never derives linked wrapper VAs by subtracting unrelated object offsets. `require_candidate` rejects those mismatches; matching this component alone still cannot accept an object or issue a proof certificate. Actual candidate-to-config VA/size joins and the full unchanged remainder remain independent required checks.

Both read-only server pre-fix objects reject as candidates with exactly two known dispatch reversals each. The pinned original TU13 and TU14 chains decode. Eight semantic mutations against the in-memory private-DOL reader reject without relying on outer hash rejection; no mutated DOL or payload fixture is written. Forty asset-free test methods cover both synthetic complete chains and rejecting controls, including the exact wrong r3-base assumption found during qualification. The test programs are emitted from semantic ISA operations, not copied instruction payloads.

Callable interfaces for independent challenge:

```
from dispatchownership import decode_original_dol, decode_object, require_candidate
original_report = decode_original_dol(private_dol_bytes, 'TU13')
baseline_report = decode_object(private_before_object_path, 'TU13')
require_candidate(private_before_object_path, 'TU13')  # expected ValueError
```

Use TU14 separately. `dispatchproof.audit_tu13_dol` additionally checks its pinned metadata bodies/tables before delegating the full decoder; `audit_tu14_dol` delegates its separate full chain. Every successful report retains production_acceptance=false. Full fresh candidate objects, integrated source/config bindings, complete effective-reference closure, actual source perturb/restore compilation and import integration remain PENDING. No live world-map observation replaces those checks.

`qualify_dispatchownership.py --dol PRIVATE_DOL --tu13 PRIVATE_BEFORE_TU13 --tu14 PRIVATE_BEFORE_TU14 --output NEW_SEMANTIC_RECEIPT` repeats the bounded read-only qualification. The two BEFORE object hashes are pinned to P's completed proof53-r1 baseline measurements. It refuses to overwrite an output and emits only hashes, addresses, mismatch names and control results. It uses no lock, compiler, runtime or proofland API; it does not change those original files. All generated instruction mutations exist only in memory.
