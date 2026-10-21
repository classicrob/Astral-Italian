# Runtime ENTRY dependency identity contract

`entry_alias_candidate(root, Symbols(symbols_path), Splits(splits_path),
Dol(image_path), relation)` is a read-only, proposal-only checker. No CLI switch,
`Symbols.one` relaxation, production API contract extension or automatic alias admission is
added. Production `api_contracts` does not call it. Init remains UNKNOWN on its dependency
aliases, including after a supported candidate result.

The root must contain the pinned `src/Runtime/runtime.c`. The relation has exactly
these fields:

| Field | Required value |
| --- | --- |
| `kind` | `mwcc_runtime_entry_at_function_start` |
| `unit` | `Runtime/runtime.c` |
| `source` | `{path: "src/Runtime/runtime.c", sha256}` |
| `owner` | `{symbol, section, address, size, unit, sha256}` |
| `first_entry` | Source-derived first ENTRY symbol |
| `registers` | Contiguous integer GPR sequence within 14..31 |
| `dol_sha256` | Whole input image pin |
| `symbols_sha256`, `splits_sha256` | Complete canonical metadata file pins |
| `references` | Nonempty list of exact called-entry references |

Addresses are hex strings; byte sizes are integers. The owner is an independently
sized `.text` function. Each reference has exactly `caller` (the same six owner
fields), `pc`, `instruction_sha256`, `entry_symbol`, and `call_kind: linked|tail`.
The caller must be an unambiguous, fully owned and pinned function. Its exact
reachable branch must target that specific source-derived ENTRY address. Merely
pointing somewhere inside the parent is insufficient.

The result is `CANDIDATE_BOUND` or `UNKNOWN`, with `production_acceptance: false`
in both cases. It retains all overlapping original metadata records, each source
entry/prototype/macro chain/offset/label record and each exact reference. Unknown
results retain reasons and partial evidence. Original instruction words and code
payloads are never emitted. A label's internal zero-size sentinel is accompanied
by `size_explicit: false`; it does not acquire a function extent.

## Evidence closed by the candidate

1. Preserve one explicit-sized parent and every distinct, sizeless `type:label`
   entry record. Require full-range split ownership, including byte overlaps
   between entry addresses. Reject duplicates, unrelated same-address/interior
   symbols, conflicting functions/extents and missing ownership. All records
   remain in `Symbols`; nothing is filtered or coalesced.
2. Bind the unique active `static asm void NAME(void)` definition. Require a
   separately declared `void MACRO(N)(void)` entry prototype for every marker.
   Expand only unique active function macros with numeric arguments, token paste,
   the `entry` directive, and pinned object aliases for the base register. Unknown
   conditionals, includes, redefinitions, undefinitions and semantic-token macros
   fail closed. Names and suffixes are opaque; invented names work in fixtures.
3. Admit only `nofralloc`, an entry before each `stw` or `lwz`, the exact tagged
   GPR, its slot `4*(register-32)` through r11, and one final `blr`. Verify the
   entire source-derived encoding against the existing explicit-sized parent
   span and hash. Counting instructions checks an existing extent; it never
   invents an owner or suffix-entry size.
4. Preserve all 18 actual markers per real helper, r14..31. The first label is
   at offset 0. The actual called r29 entry is offset `0x3C`; its three-register
   suffix stays part of the full `0x4C` parent, with no independent label extent.
   Require the explicit reference to that exact entry. Synthetic fixtures use
   smaller contiguous register sets and invented addresses/names.

Only the observed Runtime GPR grammar is modeled. Unsupported valid assembly is
UNKNOWN, not a source defect. A source/body change that is consistently repinned
can still preserve an identity relation, but it cannot refresh an old API review.
Metadata completeness is a separate reviewed input requirement: a projected
selection cannot establish absence of unrelated records from the full catalogue.

## Admission remains a separate review

Before actual API admission, C must independently replay the enabled checker and its
rejections, bind the actual full metadata/source/image pins and exact r29 calls,
and review an explicit versioned relation receipt. Opt-in admission
is limited to those dependency relations; primary providers and callers do
not gain generic same-address equivalence. The full parent and all entry records
stay visible. Each internal target must still be bound to its own source offset.

The reviewed helper transport contract, source class/static-instance joins,
caller register requirements and ABI/normal-return domain assumptions all remain
separate obligations. Candidate identity proves no allocation success, lifetime,
gameplay, source equivalence, whole-game coverage or permission to run a sweep.
No original DOL audit or compiler invocation is part of this proposal.

Synthetic tests include save/restore candidates and fresh-pin rejecting cases for
wrong first/internal entry, changed address, absent/conflicting extent, sized
label, changed source/image body, unrelated or overlapping symbols, missing or
ambiguous ownership, absent/duplicate prototype, conditional/undefined semantic
macros and base-register changes. An integrated test confirms a bound candidate
still leaves its aliased API dependency UNKNOWN without separate opt-in reviews.

## Shared validator and report-only preview

C's exact bfc candidate replay passed 32 cases/37 independent checks; its exact
12e disabled-integration replay received a qualified pass (52 executed rows/53
checks, 12 extent controls, 113 shipped tests). Three source-class/static-instance/
normal-return obligations remain separate. The parent then authorized source-only
opt-in wiring for independent API-only review. No real identity/replay grant is
committed and actual acceptance remains pending that review. The same validator
is used by two separate report-only preview callables:

```
preview_api_contracts_with_entry_dependencies(
    api_manifest_path, root, Symbols(...), Splits(...), Dol(...), entry_reviews_path)
preview_dispatch_with_entry_dependencies(
    root, dol_path, symbols_path, splits_path, api_manifest_path, entry_reviews_path)
```

The transport preview returns only a report, never a live contract map. Its
`would_accept` list is conditional evidence, with `production_acceptance: false`.
The dispatch preview keeps all existing source/cast/receiver/static-instance
joins, reports `WOULD_MATCH`/`WOULD_MISMATCH` or `UNKNOWN`, and retains conditional
contradictions in `would_mismatches`. Neither preview publishes production MATCH
or accepts an actual helper. A transport preview alone cannot establish source
dispatch or lifetime.

The entry-review manifest is `{schema: 1, relations: [{relation, review}]}`.
`relation` is exactly the schema above; `review` pins a separate JSON receipt by
`{path, sha256}`. Relative receipt paths resolve against `root`. The identity
receipt must contain:

| Field | Required value |
| --- | --- |
| `schema`, `status` | `1`, `accepted` |
| `scope` | `runtime_entry_dependency_identity` |
| `reviewer` | Nonempty string |
| `api_claim_sha256` | Canonical hash of the exact existing API semantic claim |
| `relation_sha256` | Canonical hash of the exact ENTRY relation |
| `tool_sha256` | Hash of the currently executing complete tool file |
| `checker_replay` | `{path, sha256}` for a separate accepted replay receipt |

The replay receipt needs `schema: 1`, `status: accepted`, a nonempty `reviewer`,
`scope: entry_dependency_checker_replay`, and the same `tool_sha256`. Reviewers
attest the review/replay; hashes verify identity. These reviews do not create
the existing API transport-semantic receipt or satisfy its obligations. Synthetic
tests explicitly invent their own receipts; no real ENTRY identity/replay grant
is supplied in the repository.

Only an exact six-field `claim.dependencies` owner may use the relation, and
only when explicit ENTRY reviews are supplied. Primary providers and allowed
callers still require unique original records. Every relation reference must
pin the exact API provider as its caller. The reference set must exactly cover
all direct branches from that provider into this owner (B and BC are censused;
unsupported conditional references remain UNKNOWN). Duplicate or omitted calls,
other callers, unrelated owners and wrong internal entries reject. This first
proposal does not broaden into transitive or indirect call admission.

After that identity join, all original checks still apply: the independently
accepted semantic claim receipt, complete image/provider/dependency/source pins,
scoped exact caller PCs, branch roles, entry-derived register requirements, and
the normal-return/ABI domain assumptions. Missing, pending, stale, conflicting,
wrong-claim, wrong-owner or wrong-checker evidence leaves the alias unresolved.
The original metadata records are never coalesced or mutated.

C also found a pre-existing API extent gap using fictional inputs: bfc checked
only the first owned byte. The common validator now uses `Splits.owns_extent` for
every primary provider, dependency and allowed caller. One complete declared
range must cover the span and no other unit may overlap any byte. Start-byte-only
ranges, unowned tails and competing overlaps reject; touching endpoints do not
overlap. Reports retain per-role `extent_checks`. The same rule is shared with
the candidate checker. This strengthens the existing strict production path;
it does not allow ENTRY aliases. No actual split-input defect was demonstrated.

Integration controls include a viable fictional preview while strict production
continues to refuse, unchanged original symbol records, independent semantic/
identity/replay refusal, other-claim reuse, wrong/omitted references, bad PCs,
second-caller unknown/nonzero r5, source-header staleness, unavailable source
class/static instance, and an ordinary dispatch alias that remains UNKNOWN.

## Explicit opt-in wiring; actual acceptance pending review

The default remains alias-strict. Public Python entrypoints accept optional
review inputs without changing any previous positional arguments:

```
api_contracts(api_manifest_path, root, symbols, splits, image,
              entry_reviews=None)
audit(root, dol_path, symbols_path, splits_path, selected=None,
      inventory=False, contracts_path=None, catalogue_path=None,
      entry_reviews_path=None)
```

`api_contracts` returns the existing `(contracts, report)` pair. Only an explicitly
supplied, complete and accepted ENTRY review chain can resolve an aliased API
**dependency**. The candidate record's `production_acceptance: false` means that
identity evidence alone grants no API or dispatch acceptance; the separately
reviewed semantic claim and all shared-validator checks decide admission.
Omitting reviews after any opted-in call is still strict: no symbols are mutated,
no approval state is cached and no same-address equivalence becomes global.

The CLI adds `--entry-reviews PATH`, requiring `--api-contracts PATH` and a
non-inventory mode. Invalid combinations refuse before reading any image. Example
for an independently authorized bounded run only:

```
python3 tools/team/nervemap.py --root REPO --dol PRIVATE_DOL \
  --symbols FULL_SYMBOLS --splits FULL_SPLITS --class EXACT_CLASS \
  --api-contracts REVIEWED_API --entry-reviews REVIEWED_ENTRY \
  --output PRIVATE_REPORT
```

No CLI option mints reviews. All receipts retain the same schema described above
and must be refreshed by the reviewer against the exact enabled tool hash.
The report-only previews still return conditional `would_accept` and
`WOULD_MATCH`/`WOULD_MISMATCH`, with `production_acceptance: false`; they return no
live contract map. The downstream source class/static-instance/producer/consumer
joins have not changed. Transport acceptance establishes no source dispatch or
normal-return/lifetime theorem by itself.

The enabled checkpoint must receive C's independent API-only actual two-init/
five-setter checks and mutations before actual production acceptance. Full
original-DOL audit/sweep authorization remains a separate HOLD pending canonical
dispatch proof. Source-only wiring and synthetic tests release neither gate.
