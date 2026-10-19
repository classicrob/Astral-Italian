# Argument-order advisory

`argorder.py` inventories C/C++ call sites and declaration-style/braced constructors whose **two or more distinct argument slots** contain lexical calls or `++`, `--`, assignment/compound-assignment operators. A nested call counts in its enclosing argument, not as an extra outer argument. Calls are potential effects; purity is not inferred.

```sh
python3 tools/team/argorder.py --root . --out /tmp/argorder.json --markdown /tmp/argorder.md
python3 tools/team/argorder.py --root . --paths src/JSystem/JParticle/JPAParticle.cpp --out /tmp/jpa-args.json
python3 tools/team/argorder.py --root . --prefix src/RVL_SDK/ --source-only --out /tmp/sdk-args.json
PYTHONPATH=tools/team python3 -m unittest discover -s tools/team -p test_argorder.py
```

It reads files and existing objects only. It does not compile, invoke a preprocessor, download dependencies, write source/config/proofs, or infer an evaluation order from source argument position. Only explicit output paths are written. Standard library plus the existing adjacent `callaudit.py` are used; `--source-only` needs no built files.

Each row contains file:line/column, unit, enclosing named function, call spelling/type hint, exact argument excerpts, call/mutation/effect counts, uncertainty reasons and binary status. JSON includes per-file parse coverage. Optional Markdown gives a readable inventory. All `.c/.cc/.cpp/.cxx/.h/.hpp/.inc/.inl` files beneath the prefix are attempted; assembly/binary fragments are outside the C/C++ lexer.

## Binary evidence

Where available, existing `build/SB4E01/{src,obj}/UNIT.o`, `symbols.txt`, and `orig/SB4E01/sys/main.dol` are read. The enclosing function must resolve uniquely, including a supported MW length-encoded owner when the source gives one. Target instruction opcode/register bits are checked against the original DOL; every actual direct callee destination is separately checked against its unique symbol address, including ABI helper labels. The report includes actual target call offsets/names/addresses and source ELF relocation identities mapped to original symbol addresses. Source mapped addresses are explicitly labeled as logical identity coordinates, not final source executable addresses. Object/source hashes and mtime are recorded; changing inputs and sources newer than objects refuse association. Header freshness is not asserted.

`resolved-direct-effect-call-order` means only that every involved call spelling and the outer call occur uniquely in the enclosing source function and uniquely among its direct compiled callees, and that their instruction interval is straight-line in each object. It reports the ordered argument indices of those explicit out-of-line calls. It does **not** prove full argument completion order, callback behavior, alias safety, purity, source equivalence, or correctness. Different callee owners/addresses are not normalized away. No source-order fallback exists.

Mutation effects, repeated callees, constructors requiring type confirmation, inline/missing/indirect calls, macro expansion, overloaded/ambiguous functions, unsupported body mapping or control flow give **unknown** order. Direct calls elsewhere in the function remain visible even when argument association is unknown. Current regular SDK objects have no DWARF line data; this first tool does not decode DWARF or invent line attribution.

## Lexer limits

Comments, escaped/character/raw strings, continued directives, balanced parentheses/brackets/braces, common template-ids/nested closers and constructor initializer lists are handled. Directive bodies are skipped; inactive branches are scanned. A conservative union of macro names from source/headers suppresses order attribution even for an inactive or undefined macro. Template/comparison ambiguity is annotated where detected. This is not a complete C++ parser: spliced identifiers, unnamed/assembly function bodies, implicit conversions/destructors, overloaded operators, vexing declarations and some template ambiguities can be missed or overcounted. Deferred lambda/unevaluated contexts are flagged. A candidate is an advisory, never a benign/proof exemption.

Tests cover mixed JPA RNG arguments, triple constructors, nested calls in one versus two slots, template/braced nesting, strings/comments/macros, mutations versus comparisons, indirect calls, repeated/inline/overloaded/ambiguous callees, control flow, stale input refusal, and synthetic original-DOL opcode/callee mismatch rejection.

## Priority ranking

The optional Markdown report now starts with a priority column and HIGH-first index; JSON has `priority.level`, per-argument reasons and evidence summaries.

- **HIGH:** at least two distinct argument slots contain an established mutation, checked RNG effect or directly proved storage write. Effects need not share storage; this is a review priority, not a sequencing defect claim.
- **LOW:** every nested argument call has a bounded pure-leaf body in source and original DOL, and unique actual call association is resolved. `const`, a getter-like name or absence of an obvious assignment is never sufficient. This version permits only memory-free integer/address leaves; memory-reading getters need additional volatility/ownership closure and remain unresolved. Opaque calls, branches, FP/special operations, frame stores and unknown instructions prevent purity. Provider source/header mtime and object stability checks also gate LOW.
- **MEDIUM:** insufficient established effects or incomplete purity/body evidence.
- **UNKNOWN:** macro, unevaluated/deferred/malformed context or unstable binary/effect-contract inputs prevent attribution.

Checked RNG contracts cover exact existing MR `getRandom`/`getRandomDegree`/`getRandomVector` implementations, JPARandom methods, JPABaseEmitter forwarding methods, and typed JMath `TRandom_fast_` `rand`/`getRand`/`getRandF`. Owning body shapes, seed writes, wrapper chains and the three JPA typed member paths are checked; input hashes are reported. A simple same-name type declaration must resolve the receiver without conflicting declarations. An arbitrary `rand*`/`getRandom*` name, untyped lookalike or shadowed receiver is not a contract. Inline/repeated RNG may therefore be HIGH while its evaluation order remains unknown.

Direct callee summaries use the existing source body and original DOL provider identified by split ranges, with object hashes and actual callee address. They inspect only a straight-line prefix; conditional/tail/return boundaries stop the write census. Frame writes are excluded from external-effect evidence. Incoming argument addresses and global/SDA storage are tracked; volatile argument provenance is killed at opaque calls. No mutable pointee-preservation claim or interprocedural purity inference is made. Relocated HA/LO addresses remain symbolic, including their offsets; ELF zero placeholders are never treated as final absolute addresses. Unmapped/null-derived stores are not promoted as global effects.

`--source-only` is useful while build metadata is changing: it ranks source mutations and the checked typed RNG contracts, without binary-body summaries. Changed metadata invalidates binary associations/rankings; changed owning RNG inputs invalidate the contract ranking. False invocation candidates from v1 block/function/control braces are removed in this incremental revision.
