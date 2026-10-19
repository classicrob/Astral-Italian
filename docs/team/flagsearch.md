# Scratch compiler-setting search

`python3 tools/team/flagsearch.py --unit Game/Util/BitArray --out /tmp/flags-bitarray --guard all` recompiles only isolated objects. It does not invoke configure or a build, change production settings, or write production objects. `--units-file` accepts one objdiff unit per line; `--jobs 1` is the M1 default. Each compile is niced. Prefer a short output prefix such as `/tmp/fs8`: MWCC can reject a source under a long scratch path with “specified file not found,” even when it exists. Such cases are run failures, not evidence that a compiler setting is unsupported; repeat them under a short output path.

The 25 individual variants cover plausible optimization, scheduling, peephole, inlining, code-generation and FP-contract settings. A runtime probe with diagnostics enabled rejects unsupported settings, with an unknown-pragma negative control. Variants are not combined.

`--placement global` changes the compiler command. `--placement file` adds a pushed/popped pragma after the preamble. `--placement range --range START:END` requires inclusive lines containing complete namespace-scope function definitions; partial functions, expressions, declarations, unsafe preprocessing boundaries and unsupported syntax fail closed. Only explicitly supplied ranges are supported. `-O4,p/s` variants are command-wide.

The primary score is a fresh isolated objdiff full report: a function receives exact-byte credit only at full-report 100%, and its byte total must equal the report unit matched_code. DiffJSON is retained as a diagnostic, since its literal-label score can differ. Target address and occurrence preserve duplicate local symbol identities.

Every existing full-report exact function is a hard constraint. `--guard all` also rejects any per-function fuzzy-score decrease; use this for zero-drop headline results. The default anchor-only view displays fuzzy regressions separately. Source function additions/removals and size changes are reported, not hidden. A qualifying score is not a behavioral/data equivalence proof.

Commands, input/compiler/dependency hashes, diagnostics, objects, per-function scores, source inventories and guard results remain under the requested scratch output. Inputs use separate read-only snapshots. The tool verifies production inputs and primary objects remain unchanged and fails on context drift; cached results are bound to the request and tool hashes. Do not edit the tool or source context during a run.

An improvement is only a lead. Adoption requires owner review, plausible original-file or pragma-span evidence, complete data/reference/native correctness checks, and normal full-build/all-unit guards. Per-function pragma tuning solely to force a match is not an acceptable production fix.

Tests: `python3 -m unittest discover -s tools/team -p test_flagsearch.py`. The actual MWCC smoke in shared `scratch-enemies/sol8/flagsearch-development` exercises global/file/range placements, illegal-pragma rejection, relative .inc lookup and an 88-byte exact-anchor negative.
