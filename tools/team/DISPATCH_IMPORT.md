# Independent dispatch import and final-hook contract

This branch is a review proposal. `dispatch_import.py` is an offline library; it
has no CLI, certificate installer, production-hook mutation or landing action.
Neither `remoteproof.py` nor `proofland.sh` is changed. Full-package qualification
and an explicit installation/allocation review remain required. Synthetic tests
and six-object component review do not establish a full proof or runtime result.

## Authority and modes

The operator supplies the original canonical spec FILE bytes, independently
reviewed TU13/TU14 canonical plan hashes, exact caller-contract hash, proof host,
AFTER revision and local verifier-file hash map. These are not receipt fields
that can authorize themselves. Source/config bytes come from immutable local Git
objects through `GitSources`, never from uploaded scripts. The private original
DOL is read only in memory and must match the fixed original hash. No instruction
payloads or assets are emitted by the adapter.

The operator must separately qualify command-token expectations for both complete
phase graphs and both source-control targets using independent official graph
construction/dry-run inspection. The adapter rejects missing/empty expectations;
it never treats uploaded `commands.txt`, recipe, helper, certificate or reported
`rejected=true` as executable verifier authority. This independent command
qualification remains an actual-import prerequisite, not a passing placeholder.

Only schema1 `DISPATCH_PROOF_V1`, status
`COMPLETE_AWAITING_INDEPENDENT_IMPORT`, is admissible. The exact outer field set
excludes generic PASS, Mario52/native exemptions, equivalent-base substitutions
and all other modes. Unknown/missing fields reject. The original canonical spec
and both operator-pinned plans stay unchanged. No generic hookproof certificate
is emitted or reused.

## Independent checks

The adapter confines/hash-checks metadata, rejects duplicate JSON keys/nonfinite
values, and accepts primary EXACT4855 regular ELF members plus secondary EXACT3
TU14 control members. Traversal, symlink components, tar links/directories, PAX
metadata, duplicate/missing/extra members, overwrites and hash/size mismatches
reject. Temporary extracted files are owned by the verifier and cleaned on exit.

It independently re-reads every object through the reviewed occurrence-keyed
comparator, enforces2424 strict objects, and normalizes only the exact four
reviewed execute value/size records. Full named/local/config alias references
across2426 objects and entire execute spans must equal the four owning zero-addend
vtable slots. Wrong target/kind/addend/slot, midbody aliases or additional references
reject even when present in BOTH phases. Original DOL singleton/ctor/vtable/Spine
chains and actual object ownership are decoded with the pinned local decoder;
known-bad BEFORE must exhibit the two reversed owners per scope.

Both actual-source perturb/restore controls must bind the reviewed append bytes,
local Git source hashes, exact object baseline and byte-identical restoration,
independently expected compiler commands and every header/dependency hash. All26
separate real-ELF negative artifacts are independently reconstructed byte-for-byte,
then rejected by semantic comparison (alias/slot negatives also by the census).
All8 config negatives are independently reconstructed/rejected. Missing controls,
stale metadata, wrong paths or unsupported receipt shapes reject. Uploaded
producer helpers are never imported, evaluated or executed.

The recipe, source/config, phase graphs, object inventories, command/header/tool
provenance, secondary archive and tagged certificate are joined to operator pins.
A verified result is `VERIFIED_NOT_LANDED`; it is not promotion or runtime approval.
Actual full proof/package execution remains unrun until a separately allocated
package is available and the importer itself has independent review.

## Exact final hook proposal, not installed

After independent verification, retain the canonical import-record SHA externally
in the reviewed landing invocation. Immediately before any future landing, read
that record and re-capture the current canonical spec and verifier tool hashes.
`final_hook(record, pinned_record_sha256, current_spec, current_tools, current_tip)` requires
exact record/hash/base/tip/source/tool/mode equality and returns only
`FINAL_CONTRACT_MATCH_ONLY`, `landing_authorized=false`. Any drift rejects.

A future separately reviewed production wrapper must additionally confirm the
actual base/tip and exact staged trees, leave the canonical spec/cert immutable,
use a distinct dispatch-tagged final certificate path, exclude concurrent exception
modes, recheck tool/input hashes immediately before the normal incremental Wii
guard, and call land.sh only under its normal allocation. It must not copy a
DISPATCH_PROOF_V1 certificate into the generic unchanged-proof certificate path.
This branch intentionally does not install that wrapper or weaken the existing
generic gate. Final-hook record integrity is pinned by the operator; an unpinned
JSON file is not an authorization token.

Run asset-free controls:

```
python3 -m unittest discover -s tools/team -p test_dispatch_import.py
```

The tests qualify transport, synthetic semantic inventory, failure rejection and
pure hook contracts. They do not certify a real2426-object package, candidate
compilation, source perturb execution, original-DOL runtime behavior or promotion.
