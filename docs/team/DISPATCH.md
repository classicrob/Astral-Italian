# Crossed-dispatch checks

A *crossed dispatch* is a binding in our source that selects a different function or target than the original game does while the
Wii objects still match byte for byte: a nerve `execute` wrapper or vtable slot that belongs to the wrong nerve class, a
`setNerve`/`isNerve` use site that names the wrong instance, a virtual declared in the wrong order. Weak nerve vtables sit in `.data`
that no split unit owns, so objdiff never compared them.

## The oracle: `tools/team/dispatchcheck.py`
Reads the original DOL (`orig/SB4E01/sys/main.dol`, sha1 checked against `config/SB4E01/build.sha1`) and compares, per site, what it
does with what our compiled objects (`build/SB4E01/src/**.o`) bind. Needs a built tree; ~20 s.
Layers: `data` (every ADDR32 relocation in data sections), `code` (relocations of byte-exact functions), `near`/`align` (relocated
nerve use sites of non-exact functions, paired by an equal masked instruction), `sem` (an `sInstance__` use site followed through the
original sinit -> ctor -> vtable -> execute wrapper chain and compared with our chain), `sinit` (instance/ctor pairs), `names`
(symbols.txt names along each original chain must name one class), `conflict` (one symbol bound to two original addresses).
Verdicts: `MATCH`, `CROSSED`/`WRONG`/`CONFLICT` (bugs), `ALIAS`, `MATCH_INFERRED`/`MATCH_LOCAL`/`CONTENT_*` (hypotheses; never
evidence), `LOCALDRIFT`, `UNRESOLVED:<reason>`.
`land.sh` runs it as a gate: `python3 tools/team/dispatchcheck.py --quiet --allow tools/team/dispatchcheck-allow.txt --strict`.
Any new CROSSED/WRONG/CONFLICT row (or a stale allow line) blocks the landing. `dispatchcheck-allow.txt` holds only reviewed
non-bugs (duplicate local copies of inline functions); see the comments there. Never re-baseline it to bless a new row.

## Fixing a family
Make the chain of every nerve class equal to the original's: class C's vtable slot 2 must be a wrapper whose body calls the exe
function the original chain of C's instance ends in; use sites must use the instance whose original chain ends in the exe function
the original site selects; define instances and execute wrappers separately when their orders differ; move the symbols.txt
addresses/names accordingly. `python3 <scratch>/chainreport.py` style tables (instance/ctor/vtable/execute per class) are easy to
rebuild from `Prog.dol_chains()`.

## Proving a dispatch-only change: `tools/team/dispatchproof.py`
`hookproof.sh` demands byte-identical objects, which a dispatch fix cannot give (symbol positions and vtable relocations move).
`dispatchproof.py --before ROOT --after ROOT --unit <changed unit>...` compares the linked image of every object (each symbol keyed
by its original address in that tree's symbols.txt): everything must be identical except relocation sites the oracle flips from
BAD to MATCH, plus same-address renames/first namings, exact duplicate moves and exact padding. It rejects code/data/padding/metadata
changes, coverage loss, new/removed symbols and unflipped relocation changes (`test_dispatchproof.py`: real-object controls).
`DISPATCH_PROOF=1 HOOKPROOF_JOBS=2 tools/team/proofland.sh` is the standard landing path for such batches: only
`src/Game/**`, `include/Game/**`, `config/SB4E01/symbols.txt` and the allow-list may change; two clean builds, the proof, the
perturbation control, the strict oracle and the final DOL sha1 are all required (`DISPATCH_PROOF_DRYRUN=1` stops before land.sh).
Changes that alter object content (a call target replaced, static-initialisation order with unanchored guards) are NOT dispatch-only:
land them with `land.sh` (full link must reproduce the original DOL sha1) and document why.

## Native checks
`native/tests/*dispatch*_tests.py` extract the production nerve declarations/wrappers and drive them through the canonical Spine/Nerve
with recording executors (each has a former-binding negative control); `tools/team/nativecheck.py` syntax-checks changed sources
against the headless compile database.
