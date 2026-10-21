# Supplementary singleton data-ownership contract: disabled design

This packet specifies evidence and invented review controls only. The executing
tool remains c23df53d1d33f783528a82d88d1f2b01c7d94367, SHA256
4df422b232f9fe23984ca952cb7fd0bbd12672a40fb984a9e5a44e0bf7b852bc.
There is no checker, opt-in argument, effective owner map, split modification or
production admission path for this proposal. Independent review and separate head
authorization are required before any implementation. Existing bounded results
remain five UNKNOWN in each actual source/map arm; full sweep remains HOLD.

## Claims that must remain separate

| Claim | Necessary evidence | Existing E/P53 evidence and limit |
| --- | --- | --- |
| Reconstructed TU defines a weak object | Exact build/source/header pins; relocatable-object hash; complete section/symbol-table occurrence and symbol record; ELF definition kind; size/binding/type/visibility; source class and separately typed static definition | Retained reconstructed objects support this property for their pinned source contexts. It is not an original linker contribution record. |
| Original initializer contributes construction to address A | Original-image/map/splits pins; fully owned .ctors entry and initializer text; reachable singleton-load and actual constructor-call PCs; entry-derived address; exact selected constructor and final vptr store with no unsupported overlap | E has bounded original .ctors/initializer/ctor/vptr chains. This establishes initialization contribution to a physical address, subject to modeled-path assumptions. |
| Original storage allocation and exact object extent | Actual original contribution/allocation/type witness with exact address/extent, section and input selection; BSS container alone provides only containment | Actual splits provide no .sbss/.data owner here. A four-byte store proves a four-byte access, not the allocation size. |
| Selected original weak definition | Original link/input provenance for the selected occurrence, plus complete competing-provider/weak-resolution evidence or another independently reviewed original selection witness | Reconstructed object emission and a matching constructor chain do not identify the originally selected weak input. Global original provider/duplicate closure remains open. |
| Audited source static definition binds to the selected entity | Exact audited source context, active macro/declaration/definition/header pins, selected original-provider relation and independently checked object/relocation bridge | Source changes and reconstructed artifact coverage must be recorded per source/map arm. Wrapper bytes/names do not refresh this relation. |

A future result must report these dimensions separately. A reconstructed definition
and an original initializer association may be CANDIDATE_BOUND while original
storage owner, exact original allocation extent and original weak-provider
selection remain UNKNOWN. The proposal must never return one generic `owner` by
copying the initializer TU into the data allocation role.

## Required wire identity

Each relation binds an exact subject and one source/map pair: original image hash,
full symbol/split hashes, audited source commit/tree and declaration/definition/
header pins, checker hash, and separately reviewed claim/evidence receipt hashes.
Preserve the full mapped singleton record including declared/absent size. Do not
rewrite symbol records, coalesce aliases or add a synthetic split range.

Reconstruction artifacts must declare their role explicitly and pin the actual
object bytes, producing source/build profile and ELF ABI. Exact reconstructed
source-tree equality can bridge a commit label; it must be separately verified.
A constructor or callback byte match cannot establish source-tree equality.
A reconstruction link map remains a reconstruction artifact.

Provider identity is scoped by artifact hash, symbol-table section/index,
symbol ordinal, symbol-table string-table link/name offset, defining section
index/occurrence, value, size, binding, type and visibility. Full original records
remain present. Common/undefined/ambiguous symbols, duplicate weak candidates and
unresolved group/coalescing/discard decisions remain UNKNOWN. Spelling and the
nearest STT_FILE record cannot establish original selection. An exhaustive
provider-equivalence result may describe equivalence but cannot identify which
original input was selected.

Relocations pin their own section/index/occurrence, RELA type/entry size, sh_link,
sh_info, source offset, raw r_info/target symbol ordinal and signed explicit addend.
Resolve through the referenced symbol table in that same object. Preserve the
source symbol span containing the relocation. The required reconstructed chain
is .ctors ADDR32 -> sinit; sinit SDA21 -> exact singleton and REL24 -> exact ctor;
ctor HA16/LO16 -> exact vtable; vtable+8 ADDR32 -> execute. Unsupported relocation
forms, a different table/section occurrence, nonzero unexplained addend, unbound
receiver adjustment or indirect constructor remain UNKNOWN. These records are
identity/association evidence, not a proof that callback names or source actions
are correct; callback discrepancies remain a downstream audit obligation.

The original chain must separately bind actual instruction roles: singleton-load
PC, constructor BL PC, final vptr-store PC, and consumed slot+8. Each immutable
caller/initializer/constructor span has address/section/size/hash and exact full
ownership where that role is claimed. Derive register/address facts from reachable
instructions, rather than seeding manifest claims. A dead matching span, ADDI
labelled as BL, another constructor at matching bytes, partial/unknown stores or
an unresolved overwrite invalidates that chain. The contract's domain ends at
the reviewed initialization/identity relation; no allocation success, later
lifetime, execution, onEnd purity or gameplay claim follows.

## End: four independent extent observations

The original symbol map declares an eight-byte span for End. P53's reconstructed
ELF symbol has st_size four in a SHT_NOBITS .sbss section. The original ctor
writes a four-byte vptr at the selected address. The trailing four-byte interval
has no established allocation/provider owner or padding interpretation.

Retain `mapped_span`, `reconstructed_symbol_size`, `observed_access_extent` and
`original_allocation_extent` as separate fields, with an explicit unresolved tail.
NOBITS data has no file-backed bytes to hash. The DOL's BSS envelope and a
neighboring symbol do not supply an object boundary or prove padding. Never
replace map8 with ELF4, grow ELF4 to map8, or assign the tail to this TU by
alignment. If a future witness proves only a four-byte original object, the
broader mapped eight-byte span must still retain its unexplained tail. A consumer
requiring ownership of the full declared span must remain UNKNOWN until that
span's relation is independently resolved. No proposed path resolves it today.

## Original weak-selection witness

Before claiming original storage/provider ownership, require a pinned witness
whose provenance is original link/input selection, independently related to the
same original image. It must identify selected input object/archive member,
section contribution/address/extent, full symbol occurrence and resolution/group
identity; retain all competing/duplicate/weak/COMDAT providers and their actual
selection/discard dispositions. Record link recipe, order, script and selection
rules when needed to interpret the witness. Missing original records are gaps,
not permission to treat the two reconstructed objects as the complete link.

An original linked ELF may help if its provenance and contribution information
close those facts. A final symbol table or STT_FILE adjacency alone is insufficient.
If original selection cannot be recovered, keep that dimension UNKNOWN. Any
future weaker scope for physical initialization association must be proposed
under a new explicit scope; it cannot masquerade as full data ownership or
silently satisfy the current ownership gate.

## Admission and non-bypass obligations

The draft envelope has `mode: DISABLED_PROPOSAL` and `production_enabled: false`.
Structural validation only checks wire shape, never semantics. Receipt presence,
reviewer text, hashes, manifest status flags and a complete-looking synthetic
example cannot establish facts. A future checker must replay each bounded chain
and enforce separately reviewed scope; it must not trust input booleans for
reachability, extent, provider selection or equivalence.

Even a reviewed data relation must preserve original split owners and report the
supplement separately. It cannot admit an API helper, change constructor/execute
labels, choose an aliased function, bypass source class/static-instance/cast
receiver joins, expand the selected class/TU set, or publish MATCH with another
open dimension. Missing/stale/conflicting data evidence returns UNKNOWN; existing
split ownership remains authoritative and a conflict refuses the supplement.

`ownership-contract.schema.json` is a proposed structural envelope.
`synthetic-cases.json` is an invented semantic acceptance specification, NOT
executed checker results. It includes candidate-only positives and rejecting
cases for identity, provenance, extent, duplicate/selection, relocation and
source/receipt joins. No original words, objects or private payload are fixtures.
The report will pin E's final compact evidence seal and list exact unresolved
facts before any implementation is considered.

## Exact consistency and review-hash rules

Full source and target defining-section headers are part of symbol identity,
including distinct occurrences sharing .text/.data/.rela.text names. Preserve
section/symtab/string-table indices and raw fields; require all copies of one
scoped symbol occurrence to agree. A name is an opaque identifier, but renaming
one execute record without its pinned ELF and relocation target changes identity
and must return UNKNOWN. Callback correctness remains a different obligation.

Retain every relocation occurrence, including identical-looking duplicates.
Its entry ordinal and file offset must match section offset + ordinal*entrysize;
sh_info must name the actual source section and sh_link the actual target symbol
table. Decode target ordinal and type from raw r_info, retain the signed RELA
addend, and require the exact corresponding symbol-table record. Source offset
and access width must fit the source symbol/section. Do not collapse records by
(name, offset) or silently choose a table/section occurrence.

For original selection, selected_provider must occur exactly once in providers,
with disposition selected. No second selected or unresolved competing provider
may be hidden. Its contribution interval must contain the independently proved
original object extent; original record provenance and complete provider-census
closure remain independent requirements. A complete-looking array is not proof
that all original inputs were supplied.

Reject duplicate subject IDs and conflicting relation records. Context source,
map/image/schema and per-relation source/map/image/span records must agree under
explicit verified source-tree bridges. Reject non-allowlisted subjects. Verify
full ownership/conflict intervals; all address+size/value+size/offset+width sums
use checked nonwrapping arithmetic and must fit both the 32-bit target address
space and exact section/file bounds. An absent tail must not be synthesized.

The proposed review.claim_sha256 covers every root property except review:
{schema_version, mode, production_enabled, evidence_domain, context, relations}.
After strict parsing (duplicate JSON keys refused), serialize that entire value
as UTF-8 JSON with lexicographically sorted object keys, compact comma/colon
separators, unchanged array order and Unicode strings, no ASCII escaping and no
Unicode normalization; hash with SHA256. Only schema integers are admitted;
no floating values, NaN or Infinity. Context includes the full proposal-schema
hash and design-spec pin as well as source/map/image/tool identities. A receipt
for one relation, arm, subset, schema or scope cannot cover a different envelope.
Hashes bind separately reviewed assertions; they do not establish semantics.
The only permitted review states here are pending/reviewed_design_only, and
production_enabled is always false.

## Invented catalogue qualification

The fixture is declarative invented metadata; it contains no original addresses,
code bytes or real byte-bearing ELF/witness files. Its positive specifies only
ASSOCIATION_CANDIDATE_ONLY after independent fixture/artifact validation. Original
allocation and weak selection remain UNKNOWN even in that positive. A selection
fragment specifies hypothetical input shape and rejection requirements, not a
real selection witness. The catalogue has not run against a semantic checker;
there is no such checker. `note` mutations describe cases an independent runner
must instantiate rather than pretending to be executable JSON Patch.

Mutable-draft review corrections are retained: a single-record execute-name
change is a metadata-identity negative; repeated .rela.text sections use distinct
#0/#1/#2/#3 occurrences. The baseline fixture's copied symbol/header identities,
relative widths, r_info fields and relocation entry offsets were checked for
internal consistency only. No result from that hygiene check grants ownership.
