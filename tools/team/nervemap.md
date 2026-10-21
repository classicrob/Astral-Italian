# nervemap: conservative Nerve address-chain audit

`nervemap.py` reads source, symbols/splits and the private original DOL; it never
compiles, runs the game, downloads inputs, or consumes global linked symbol names
as object ownership. Run from any directory. DOL bytes stay local. JSON output
contains input paths/hashes, span addresses/sizes/hashes and bindings, not code,
raw words, asset or DOL payloads.

```
python3 tools/team/nervemap.py --inventory-only --output /private/path/inventory.json
python3 tools/team/nervemap.py --class Namespace::NerveClass --output /private/path/bounded.json
# Add --api-contracts /private/path/reviewed-helpers.json for accepted helper contracts.
# Add --entry-reviews /private/path/reviewed-entries.json to opt in to reviewed Runtime dependencies.
# Add --source-catalogue /private/path/CATALOGUE-CHECKPOINT.json for UNKNOWN partitions.
# Only after the team's accepted-correction/proof and head sweep authorization:
python3 tools/team/nervemap.py --sweep --output /private/path/sweep.json
python3 -m unittest tools/team/test_nervemap.py
```

Use `--root`, `--dol`, `--symbols`, `--splits` to pin different inputs without
changing any checkout or symbols. Exact `--class` selectors are repeatable. Exit
1 means proven dispatch mismatches; exit2 means invalid input. Exit0 can include
UNKNOWN: always inspect coverage. MATCH is limited to the modeled dispatch chain,
not gameplay correctness, full function equivalence or lifetime.

## Joins and evidence

1. Inventory the union of mapped execute classes with singleton/vtable identities
   and source Nerve classes. Preserve whole-universe denominator, selected count,
   excluded count, MATCH/MISMATCH/UNKNOWN counts, source and original producer
   counts, unsupported forms and unresolved spans. A bounded run is not a game
   sweep. Names identify requested source bindings; names never determine which
   callback owns an original object.
2. Read DOL addresses using its section headers, rejecting overlapping/unmapped
   spans. Parse full MW length-prefixed qualified owner names from symbols.txt;
   retain exact path/line/section/address/size. Same-address aliases are not
   silently chosen. Splits provide TU ownership, including actual .ctors records.
3. Recover r2/r13 startup constants from __init_registers. A bounded constant-only
   PPC CFG interpreter propagates direct addresses and joins differing path
   constants to unknown. Do not treat pointer loads as immutable file constants.
   Observe actual nerve-producer r4 and static-initializer r3 constructor arguments.
   Follow constructor writes to the selected object's vptr, not the constructor's
   state-name suffix. Preserve initializer registration/call/store/hash evidence.
4. Source extraction uses namespace/class scopes and owned include reachability.
   It understands inline Nerve subclasses, macro definitions that actually expand
   class/execute code (including NEW_NERVE, NERVE_DECL-style, WMR/WMS variants),
   and normal out-of-class execute definitions plus separate static instances.
   A method endpoint is taken from the actual source cast+member call, never
   inferred from a nerve class suffix. Unsupported source/state-controller forms
   remain UNKNOWN. This is lexical extraction, not a C++ compiler/preprocessor.
   The entire supported callback is one statement using the actual Spine formal
   and its cast receiver. Extra statements, unresolved semantic macro tokens,
   conditional/redefined macros and unresolved getter expansions fail closed.
   Getter names have no built-in meaning: their pinned definitions must expand
   to a bound singleton expression.
   A source class must have a unique direct Nerve base and a separate correctly
   typed static-instance definition. API receiver resolution currently accepts
   only a unique public single-base chain without a hiding local member; other
   inheritance/alias layouts remain UNKNOWN.
5. Compare actual vtable and execute slot ADDRESS to source class/symbol bindings,
   then actual wrapper behavior to source-bound method/transition endpoints.
   Preserve wrapper extent/size even when unequal. Same wrapper bytes under crossed
   symbol labels cannot pass. Unknown/thunk/adjusted-this receivers cannot become
   automatic bugs. Source/original producers pair only for a unique single call
   per API in an owned function; ambiguous multi-call source order is UNKNOWN.
   The Spine consumer uses entry-reachable symbolic receiver dataflow to the
   actual indirect slot call. A dead matching instruction sequence is insufficient.
   Partial/unaligned/unknown stores that may overlap a vptr invalidate the proof,
   including stores in an inline static initializer.
6. Only promote differences when every required chain/source ownership join is
   closed. Otherwise retain suspicious checks as partial evidence on an UNKNOWN
   row. This deliberately sacrifices coverage to avoid name-based false positives.

## Current limits and ownership

The tool does not consume freshly compiled/paired objects or Mach-O captures.
There is therefore no unscoped weak/strong, local-name, section-name or ASan-symbol
join. If an object-backed extension is added, both target and source references
must be pinned by TU+section occurrence+symbol-table identity/addend, never merged
by symbol spelling alone. Inherited/aliased implementations need an explicit,
fully bound relation or UNKNOWN. Unsupported state-controller wrappers, nonzero
Nerve subobjects, indirect constructors/writes, getters and control-flow thunks
are recorded as UNKNOWN; no source repair is generated.

This is a WIP tool. Independent review of the API contract and full source/paired
provider ownership remains an acceptance gate before any original-DOL verdict
publication. Synthetic MATCH controls establish the modeled test contract only.
The [Runtime ENTRY contract](nervemap-entry-proposal.md) documents opt-in
dependency identity admission. The default API/audit path keeps ENTRY aliases
UNKNOWN. Explicit identity and checker-replay receipts must bind the current
tool hash and exact API claim; all semantic, span, callsite and source gates
remain required. Report-only preview callables remain separate. The enabled
wiring awaits independent review; no actual identity receipt ships with the tool.

## Reviewed helper input

Without `--api-contracts`, original producer/transition helpers remain UNKNOWN.
The optional JSON manifest has `schema: 1` and a `contracts` list. Each item has
`claim` and `review`. These are separately reviewed evidence, never an automatic
semantic certificate created by matching hashes or trusting a function name.
The synthetic tests demonstrate the exact schema using invented inputs.

The claim binds `kind: member`, `this_register: 3`, `nerve_register: 4`,
`receiver_adjustment: 0`, `api: initNerve|setNerve`,
`behavior: selected_input_reaches_nerve_storage_unchanged`,
`dependencies_closed: true`, a nonempty string list of reviewed `preconditions`,
and the entire `dol_sha256`. Other ABI shapes,
including MR globals or unresolved adjustor/delegating thunks, are unsupported.
`provider` and every `dependencies` entry must identify a unique original
`symbol`, `section`, hex `address`, byte `size`, split `unit`, and full-span
`sha256`. The dependency list must include all callees/thunks needed for the
reviewed argument-to-storage fact. An empty list asserts no such dependencies.
The primary provider must be a function; dependencies may include data/vtables.

`allowed_callsites` is a required nonempty list. Each item has `caller` with the
same full-span fields above, hex `pc`, exact word `instruction_sha256`, and
`call_kind: linked|tail`, and `register_requirements`, a dictionary of GPR names
(`r0` through `r31`) to unsigned 32-bit constant values. The current init contract
subset requires `r5: 0`; a textual zero-state assertion cannot replace that field.
A setter without extra constant requirements uses an empty dictionary.
The caller must be a uniquely pinned function; the PC
must lie inside it and branch directly to the provider with that call kind.
Both producer calls and execute transition tails require an authorized exact PC.
The validator traces each caller from its actual entry with no manifest-seeded
register values. Every supported reachable path must establish the required
constants at that exact branch. Loads, conflicting path values, unsupported
instructions/control or an unreachable branch leave the requirement unproved.
Reports retain required and observed values, reachability, trace issues, and
verification status, including refused contracts. Even a freshly reviewed/hash
updated manifest cannot override a conflicting or unknown actual register value.

Reviewers separately attest helper semantics under explicit valid-receiver,
allocation and normal-return domain assumptions; the tool does not derive
lifetime/success facts from those free-text preconditions. Machine-checkable
zero-state facts are mandatory derived requirements, not such assumptions. Exact
callsite scope and actual `r5=0` proof prevent a zero-state constructor review from
authorizing other statekeeper paths. ADDI/load PCs cannot stand in for BL PCs.

`source` pins contain `path`, `sha256`, and `role`; canonical `declaration`,
`definition`, and relevant `header` evidence are all required. Source paths are
absolute or relative to `--root`. `review` pins the separate receipt's `path`
and `sha256`. The receipt must state `status: accepted`, nonempty `reviewer`,
`scope: nerve_argument_storage_transport`, `dependencies_closed: true`, and
`claim_sha256`: SHA256 of UTF-8 JSON of the claim with sorted keys and compact
separators (Python `json.dumps(..., sort_keys=True, separators=(',', ':'))`).
Its reviewer attests the complete transport fact and dependency closure; the
tool checks receipt identity and pins, not the reviewer's reasoning. No real
helper contract is supplied or preaccepted in this repository.

Missing, ambiguous, pending, stale, wrong-DOL or unsupported contracts block
affected verdicts. The report retains accepted/refused contract evidence and
keeps symbol ownership, source dispatch and producer relations separate. A
symbol ownership MISMATCH alone does not assert a source dispatch discrepancy.
Provider, dependency and caller ownership must cover every byte of the declared
span without a competing unit overlap; a split covering only the entry is
insufficient. Per-role `extent_checks` retain that evidence.

## Nonstandard coverage partitions

`--source-catalogue` accepts the independently frozen schema1 checkpoint used by
the source inventory owner: hash-pinned `partitions` and `source_pins`, each raw
partition containing unique row `id`, `form`, `evidence`, `audit_status: UNKNOWN`,
and an exact `row_count`. The adapter checks member hashes, source files and
symbol/split pins and retains occurrence IDs plus family counts in independent
UNKNOWN partitions. Historical `INCLUDED` means lexical extraction only.
Native-excluded forms remain marked. Stale source pins retain candidate rows as
UNKNOWN and mark their evidence stale. Extent catalogue records do not close
constructor/vtable/producer ownership and are never promoted into dispatch rows.

The Nerve class denominator and nonstandard occurrence denominators are separate;
neither claims a complete universe of all integer switches, aliases or hidden
state machines. MarioState is independent of Nerve and keeps its own partition.
Absent catalogues are disclosed as an unknown coverage frontier. Missing exact
class selectors are reported explicitly and cause CLI exit2.

Whole-game execution is a separate explicitly authorized step. Tool tests use
entirely invented synthetic DOL/symbol/source inputs in temporary directories.
Private accepted packets may be used for bounded checks outside the repository,
but neither those packets nor original code/data words are test fixtures.
