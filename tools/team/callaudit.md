# Direct-call audit

Read-only advisory for registered PPC functions. Uses a self-contained big-endian ELF32 reader; no compiler or scratch-directory dependencies.

```sh
python3 tools/team/callaudit.py --root /absolute/built/tree --prefix Game/Player/ --out player.json
python3 tools/team/callaudit.py --root /absolute/built/tree --prefix Game/MapObj/ --out mapobj.json
python3 tools/team/test_callaudit.py
```

Requires objdiff.json, config/SB4E01/symbols.txt, orig/SB4E01/sys/main.dol and existing source/target objects. Calls no compiler, objdiff build, SG or git writer; git is read only for HEAD. No report.json or score input: masked100/strict100 functions are never excluded. Defined nonzero-size ELF functions also registered as function records in symbols.txt are paired by exact name. Undefined/zero-size ELF aliases do not supply an inspectable body. Target-only/unpaired and source-only registered definitions are listed explicitly; this is unit-occurrence coverage, not deduplicated game-function coverage.

Comparison is the ordered list and multiset of direct `BL` callee-name/addend identities. REL24 relocations are read verbatim, with no helper filtering, prefix aliases, symbol renaming or commutative normalization. A uniquely bounded section-local unrelocated BL can also resolve; unsupported/ambiguous cases are explicit. The target's actual linked DOL word is decoded with AA/LK/sign extension, providing site, destination and mapped identities when possible. Relocation/DOL disagreements are unresolved, not normalized away. Duplicate registered names retain all address candidates; ambiguous caller addresses refuse DOL lookup. Actual target names at an exact destination are preferred over containing-function offsets, which are otherwise reported explicitly.

`rows` contains mismatches and explicit unresolved/indirect coverage items. Every paired function, including zero-call/equal pairs, remains in per-unit `function_bindings` with body SHA256 and comparison flags. `ordered_direct_calls_equal` means equal known direct identity lists ONLY. It says nothing about call-local arguments, branch gates, callee effects or complete behavior. Unknown identities never compare equal. `target_only`/`source_only` preserve multiplicity; order-only changes are distinguished from multiset changes.

No automatic bug or equivalence claims. Inlining, same-address aliases, independent calls, ABI save/restore helpers and legitimate outline differences are common false positives. All direct op18/LK1 sites inside declared extents are scanned without reachability analysis. Tail calls are outside scope; linked indirect sites are separately inventoried. Embedded code/data or misleading extents can need manual review. A missing source-function name can be stale metadata or an ABI rename, not necessarily missing code. No table/PMF/call-argument/string/scalar equality is inferred, and no NOBITS file bytes are read.

Provenance: stable HEAD and config/symbol/DOL hashes before/after run; each unit's objects and configured own/first-level source/header/textual includes before/after its read. Missing paths, parser/duplicate-function errors, unstable inputs and unresolved destinations are retained. This is NOT an atomic global source snapshot, compiler/header closure, freshness proof or guard exception. Stable old objects can still be stale. Metadata/HEAD or per-unit input instability returns nonzero; inspect stats and errors even on exit0. `--jobs` defaults2. Scanner SHA256 is included.

The committed synthetic tests cover ordered/multiset differences, addends and multiplicity, unnormalized names, indirect/tail scope, unresolved branches, AA/LK decoding, duplicate catalog names, unknown non-equality, zero-call pairs and NOBITS refusal. Historical frozen Wait/ModelHolder regressions were also checked during promotion; they are not bundled with the game binary.
