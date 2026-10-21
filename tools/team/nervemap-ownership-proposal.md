# Disabled ownership proposal checker

`nervemap_ownership_proposal.py` is a standalone, synthetic-only checker for the
sealed v1 proposal. It never supplies an effective split owner. The copied schema
and design remain byte-identical to the independently reviewed proposal; their
historical “no implementation” language describes that immutable design checkpoint.
The existing c23 nervemap.py/API/audit/CLI are unchanged and do not import this
module. There is no production option or live integration.

Call `check_proposal(envelope, SyntheticEvidence(blobs))`, where `blobs` maps opaque
pin.path IDs to explicit in-memory bytes. IDs never resolve against the filesystem.
The callable refuses a non-SYNTHETIC domain before evidence access. Imports cache
this module, the local schema/design and the unchanged c23 baseline source before
calls. The context tool pin must bind the actual c23 source bytes (SHA256
4df422b232f9fe23984ca952cb7fd0bbd12672a40fb984a9e5a44e0bf7b852bc), and
design_spec the exact copied design bytes. Caller-provided flags do not certify
an input's real-world provenance; only invented fixtures are authorized here.

The result status is INPUT_REFUSED, UNKNOWN, or ASSOCIATION_CANDIDATE_ONLY. Every
result has production_acceptance=false, effective_owners=[], original allocation,
selected original weak provider and unassigned tail UNKNOWN, and sourceclass/API/
lifetime NOT_GRANTED. A candidate checks consistency of the supplied reconstruction
and initializer association. It is not original link selection, storage allocation,
callback correctness, source-class proof, runtime, normal-return or lifetime proof.
Identity-only auxiliary receipts cannot grant those claims. Errors remain explicit
in structural_errors or per-relation reasons; coverage is only supplied relations.

The implementation uses strict types and the closed vocabulary of the exact sealed
schema. This dependency-free validator is not a general JSON Schema engine. It
rebinds the supplied ELF32BE section occurrence/header, full indexed symbol and
RELA entry identities. All selected relocation-width overlaps and the selected
.ctors cell are closed. NOBITS has no file payload; STN_UNDEF/null entries cannot
be defined providers. Named overlapping non-section definitions, including zero-size
STT_NOTYPE labels, remain ambiguous and are refused. Source anchors, the producing TU, source tree, map, split,
image and whole-envelope-minus-review hash are bound explicitly. Source pins check
identity and line ranges, not C++ semantics or original compiler provenance.

Original initializer consistency uses actual supplied image instructions. The
startup witness must begin at the image entrypoint. A final startup helper derives
r13 on all traced returns from empty facts. Intermediate helpers currently support
only a linear prefix to one direct BL and a NOP/mtlr/BLR return epilogue. Conditional
call skips, postcall base mutations and other unclosed interprocedural CFGs remain
UNKNOWN. The registered .ctors word, reachable singleton load and constructor BL,
receiver, final vptr, actual execute slot8 and full owned function spans are checked.
All startup and initializer memory writes remain UNKNOWN because there is no
general memory-effect model; this prevents stale supplied-image registration,
vtable or code reads. Constructor stores are restricted to the tracked four-byte
vptr, and BSS must not overlap loaded spans. Unknown stack stores remain UNKNOWN;
no ABI seed is invented. The trace is the unchanged reviewed c23 constant-only interpreter.

Auxiliary blobs use strict JSON (duplicate keys/floats rejected), schema=1:

* `synthetic-reconstruction-context`: object_sha256, source_commit, source_tree,
  source_unit_path, logical_split_unit, source_pins and original_selection_grant=false.
* `synthetic-source-context-bridge`: reconstructed_source_commit/tree,
  audited_source_commit/tree and source_pins. Both source pin lists must bind the
  producing TU and all supplied source anchors; different trees are unsupported.
* `synthetic-source-anchor`: source_sha256, line, end_line, role, qualified_name,
  declared_type. These pin the anchor; semantic preprocessing is not granted.
* `synthetic-initializer-cfg`: startup_chain (sealed span records) and
  initializer_invocation_domain=`registered_ctor_table_with_derived_startup_bases`.

Ownership witness blobs are identity-only and never replace actual full-span split
checks. Original selection/allocation witness blobs are likewise never semantic
admission receipts; even internally consistent supplied provider lists leave those
obligations UNKNOWN.

The optional standalone CLI accepts only one explicit synthetic bundle:

```
nice -n 19 python3 tools/team/nervemap_ownership_proposal.py \
  --synthetic-bundle invented-bundle.json --output proposal-result.json
```

The bundle is `{envelope: ..., blobs_base64: {opaque_id: base64_bytes}}`. There are
no original DOL, ELF, root or live-audit options. Exit status is zero only for a
candidate-only result; all outputs still deny production ownership.

Run invented controls with:

```
nice -n 19 python3 tools/team/test_nervemap_ownership_proposal.py -q
nice -n 19 python3 tools/team/test_nervemap.py -q
```

The authored suite has 89 test methods: 55 sealed cases instantiated against a
fresh byte-backed fixture plus 34 additional methods. Metadata mutation/structural
refusals are distinguished from actual byte/CFG controls; several methods contain
multiple probes. The five formerly note-only cases now have concrete supplied
provider/map/review/extent mutations. Receipt pins remain identity-only, including
the invented typed-object claim; their positives prove no original claim. Additional
byte controls close the actual unreachable/partial-store/startup CFG boundaries
rather than treating missing witness blobs as behavior proofs. All fixtures are
invented. Independent C replay binds the exact pushed checker separately; any
future implementation change requires a new replay. No actual input run is claimed.
