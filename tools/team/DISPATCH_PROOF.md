# Dispatch comparator scaffolding

This branch does not change remoteproof.py, proofland.sh, the certificate format, game source, or symbols.txt. Nothing here authorizes a landing. `production_status()` always returns PENDING with acceptance false. A component match is not a proof PASS.

Run the asset-free tests at low priority:

```
nice -n 19 python3 -m unittest tools/team/test_dispatchproof.py
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

Original DOL SHA256 is pinned in code. `dispatch-tu13-ownership-v1.json` contains only addresses, body/table hashes and the Start/Wait/End singleton -> constructor -> vtable -> wrapper -> body claims; its own file hash is pinned. No game bytes or instruction dumps are included. `audit_tu13_dol` reads the private local DOL, validates all pinned bodies/tables, and decodes the two physical Start/End vtable words plus executor-load/tail-branch instructions. It returns PARTIAL_DOL_EDGES_ONLY, never acceptance.

Full r13 bootstrap, init/appear selection, sinit/constructor stores, Wait and Spine decoding remains PENDING, as does joining the actual fresh compiled symbols/relocations through the symbol map to those physical addresses. TU14 has no DOL decoder yet and no implicit reuse of TU13 ownership claims. Unknown instructions or unmapped/overlapping reads reject. The code must not be connected to production until these unfinished checks have real evidence and rejecting controls.

## Required work before import integration

1. P publishes fresh actual object deltas, raw/semantic classification and source/config/generated graph bindings. Review exact numeric records before pinning either plan. Retain sealed5.3 EffectSystem correction in any integrated candidate and use its actual new canonical contract.
2. Independently decode the complete original ownership chains and resolve actual compiled vtable/singleton/constructor/wrapper relationships. Verify two specific baseline mismatches improve to the original DOL targets; preserve Wait and all unrelated joins. No visual-causality claim follows.
3. Run real ELF mutations: wrong owning target, extra reference, body/data byte, outside symbol size/location/visibility, relocation addend and stale config/tool/source/DOL/graph. A hash mismatch alone is insufficient to claim semantic controls. Prove candidate restore compile byte-identical and passing again. The current restoration helper only checks supplied bytes; it does not prove compilation occurred.
4. Review fail-closed integration with full schema1 archive/certificate/six provenance and an explicit operator opt-in, mutually exclusive with Mario52. No producer scripts executed, old canonical spec edited, broad native exemption or whole-TU waiver. Existing final contract recheck and nj guard stay required.

No real candidate compilation, original-DOL audit, native.lock acquisition, production verifier execution or landing was performed to produce the synthetic test result.
