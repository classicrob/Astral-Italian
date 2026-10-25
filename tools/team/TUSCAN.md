# TU-boundary scanner

`tuscan.py` reads existing target/source ELF objects, original DOL bytes, split
ranges, source inlining pins and a canonical objdiff report. It never compiles,
configures, edits inputs or proposes automatic split changes.

```sh
python3 tools/team/tuscan.py
python3 tools/team/tuscan.py --residue-only --out /absolute/empty/output
python3 tools/team/tuscan.py --unit Game/Player/MarioAnimator --out /absolute/empty/control
PYTHONPATH=tools/team python3 -m unittest tools.team.test_tuscan
```

Defaults use the current configured worktree for target/config/source text,
`~/MacioGalaxy2-wt/decomp3-land` for compiled source objects, the canonical
`d4d8efa9` report and `~/MacioGalaxy2-team/scratch-decomp3/tuscan` for output.
Override with `--root`, `--base-root`, `--report` and `--out`. The output directory
must be empty. Inputs are hashed; each parsed object has its own hash and path.
Live source objects need not reproduce the canonical report. Missing objects
are listed as skipped, while parse failures are recorded separately.

Signals:

- Different same-unit call counts, corroborated where a small leaf body appears
  inline in the opposite caller. The pattern retains opcodes, widths and offsets
  while masking register numbers; it is a heuristic, not equivalence proof.
- Distinct target copies of strings that the source pools. References and data
  initializer tables identify affected functions where available.
- A middle static initializer followed by a strong function, its `.ctors` slot,
  and differing zero alignment gaps. Ordinary padding alone is insufficient.
- Repeated referenced scalar pools with disjoint text-use ranges. These are weak
  clues because separately defined equal constants are legitimate.
- Uniform source-minus-target immediate shifts, computed per function so class
  groups with different deltas do not suppress one another. This also catches
  unassigned target data through externally named bases. Missing literals and
  incorrect table layout are alternative explanations.
- A possible MERGE: target accesses current-unit data through the immediately
  preceding unit's pool base. Confidence rises when the offset delta equals
  the preceding pool's size and the units' text ranges are adjacent.
- Source `auto_inline`, `dont_inline`, `noinline`, inline-depth/size pragmas and
  `*_NOINLINE`/`*_OUTLINE` macro definitions. These are weak boundary suspects;
  their presence does not justify removing the pins.

`candidates.tsv` ranks the sum of full canonical nonexact functions touched by
the observations. These bytes are investigation scope, **not predicted gains**.
Functions are deduplicated by name plus target address; duplicate names at
different addresses remain separate. Pin-only suspects are deliberately present,
including zero-residue units. Consult `signatures` and confidence before choosing
a unit. Padding and repeated constants alone do not create a candidate.

`candidates.json` contains full per-unit evidence, function weights and addresses.
`observations.jsonl` preserves individual observations, including weak clues.
`meta.json` records provenance, skips, parse errors and reference-analysis limits;
`summary.md` gives the first thirty ranked units. Unknown dataflow and changed-size
function pairs can hide offset patterns. A lander must establish section ownership,
whole-unit score preservation and dispatch identity before changing boundaries.
