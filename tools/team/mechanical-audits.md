# Mechanical bug screens

These tools compare compiled PowerPC target/source bodies without an LLM in the audit loop. They are advisory screens, not equivalence proofs or semantic gate exemptions. Build the checkout first; each run records the revision, object identities, source/config/report/DOL hashes and input stability. Default scope is every configured `Game/` target body, including changed-size and unpaired functions. Missing or ambiguous source bodies remain explicit coverage gaps.

```sh
python3 tools/team/ctorstoreaudit.py --root . --output /absolute/scratch/ctor
python3 tools/team/branchimmaudit.py --root . --output /absolute/scratch/branch
```

Repeat `--prefix` to narrow or expand the owner scope. Results contain deterministic ranked findings, coverage and per-lane lists. Send findings to the directory owner for a DOL/source check; a screen difference alone is not a demonstrated bug. An unchanged screen is not a correctness certificate.

The constructor screen follows conservative `this` aliases, argument/constants and supported value sources through each constructor. It compares target `stw`, `stb`, `sth` and `stfs` field-store occurrences with source occurrences at the same offset, width and value source. Store multiplicity is retained. Unknown instructions, calls, indirect control flow and path joins reduce extraction coverage. Store multisets do not establish path, access-order, alias or callee equivalence.

The branch screen decodes raw PowerPC instructions so assembler aliases do not hide masks. It compares ordered signed/unsigned compare immediates, `andi.`/`andis.` masks and `rlwinm` rotate/mask fields. Exact anchors align inserted or removed events; structural differences are distinguished from isolated immediate changes. Register allocation is ignored. Nearby branches are context, not a def-use proof; signed/unsigned comparisons may be equivalent for equality-only consumers.

Run the planted-bug and coverage controls before changing either screen:

```sh
python3 -m unittest discover -s tools/team -p 'test_*audit*.py'
```

The semclass/semdata tools remain separate metrics. These screens do not publish accepted proofs, discharge data findings, or extend their signed review scope.

The shared reader also inventories synthetic DTK `gap_XX_XXXXXXXX_text` padding symbols. They remain raw observations with their identities/statuses preserved, but are counted separately from code-function coverage. Coverage reports distinguish real source/identity gaps from these generated padding observations. No real unreported function is excluded by a fuzzy score or proof list.

The native library boundary is separate from the default Game scan. The port lead confirms JSystem and nw4r C++ files, libms C files, RVL_SDK mtx/mtx44/quat/vec, OSAlloc, arc and wenc are compiled natively. MSL_C, Runtime, MetroTRK, NDEV and other SDK hardware internals are platform replacements. Use the current native manifest when selecting a library run; a prefix alone does not establish native eligibility.
