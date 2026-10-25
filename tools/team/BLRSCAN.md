# Orphan return-label census

```sh
python3 tools/team/blrscan.py --out /absolute/empty/output
python3 tools/team/blrscan.py --out /absolute/empty/output \
  --source-object Game/Scene/StageDataHolder=/absolute/frozen/StageDataHolder.o
PYTHONPATH=tools/team python3 -m unittest tools.team.test_blrscan
```

Read-only: no compiler, configure, split or build invocation. Defaults use the
current configured checkout and decomp3-land source objects. The output directory
must be empty. Repeat `--source-object UNIT=PATH` for frozen proposal objects;
their paths/hashes and full source/DOL instruction bytes are recorded.

The scanner inventories every opaque four-byte function, keeps only original
`blr` words, and checks the adjacent named predecessor. A proposal requires one
unambiguous predecessor, a source function exactly four bytes longer whose FULL
bytes equal the DOL, and zero static references to the orphan label.

References include aligned original words, relative/absolute direct and
conditional branches, conservative nearby lis/addi address pairs, and every
target relocation including HA/LO/SDA and section-symbol-plus-addend resolution.
Imports without named addresses are bounded by their original encoded operands;
any potentially matching operand counts as a reference. Any unbounded relocation
blocks proposals. This is a static census; unknown runtime pointer arithmetic is
not excluded, and no instruction/entry address changes are proposed.

`symbols.patch` and `proposal.json` are suggestions only. They extend the previous
function by four bytes and remove the false entry label. No input is edited.
`inventory.json` includes retained real stubs and their references;
`unresolved-operand-bounds.json` audits import exclusions; `meta.json` pins input
and target-object hashes. `candidates.tsv` gives the compact disposition.

After applying metadata, the lander must regenerate targets and run reports and
global dispatch. Do not manufacture source stubs for the orphan labels.
