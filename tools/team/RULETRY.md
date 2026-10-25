# Rule-guided source trials

```sh
python3 tools/team/ruletry.py Game/Enemy/Unbaba calcUnbabaHermiteSegment__Ffffffff \
  --budget 20 --out /absolute/empty-output
python3 tools/team/ruletry.py Game/LiveActor/ActorStateParamScale \
  calcHostVelocity__20ActorStateParamScaleFv --trial-compiler Wii/1.3 \
  --binding bool:mIsOnSwitch --binding 'LiveActor*:mHost' \
  --out /absolute/another-empty-output
```

The tool reads the function's cluster/rule/action from `scratch-decomp2/rules/v2/residue-ranked.tsv`. `--rule` chooses an explicit supported family; `--rules-file` supplies another mined list. `--selector` disambiguates the source definition. `--plan-only` requires a source selector and writes candidates without compiling. The default budget is 20; accepted budgets are 5..20. Fewer safe candidates are reported honestly. No random mutations fill the budget.

Recipes are selected by rule: GPR lifetime uses declaration order, preserved initializer order and integral operands; aggregate lifetime uses eligible vector initialization/declarations and nonescaping consumer scopes; scalar scope uses nonescaping temporary scopes and typed operation boundaries with value/reference and argument binding order variants while retaining arithmetic operand order; field reload and argument binding can use typed reference predicates/accessors; boolean continuation uses early return/else and bool staging. Field types require explicit `--binding TYPE:EXPRESSION`; no field types or external accessor identities are guessed. Pointer bindings retain pointer width. Pool-layout has no automatic source recipe. A reported rule with no eligible sites produces zero variants.

All compiles are sequential and hold the shared `decomp-quiet.lock`, including outside quiet windows. Production sources/objects are never written. Private overlays preserve includes and whole-file IPA. Full-report parity uses `functionRelocDiffs=none` and guards every function occurrence, every noncode section, unit fuzzy score and matched code/data bytes. Helpers retain the original arithmetic operation/grouping. Macro/effect/type uncertainty fails closed.

`--trial-compiler Wii/1.3` retains the configured compiler for the baseline, then adds an unchanged-source 1.3 diagnostic control and scores source candidates under 1.3. This can investigate a pinned 1.0 unit without modifying configure.py. A selected-function improvement is insufficient if another function drops.

Numbered `.cpp`/`.patch` files, all scores and mined evidence are retained in `result.json`. `best.patch` is emitted for a selected-function improvement with no whole-unit drop, or for a no-drop `--trial-compiler` unpin candidate (including equal-score restoration). These are candidates for matcher review: dispatch, semantics and native syntax must be checked before adoption. No patch is applied.

Validation: eleven recipe/refusal tests and seven CFG preservation tests; existing Game/library harness tests also pass (34 tests total). Real smoke: Unbaba's 80-byte Hermite helper, five source variants, all 94.0 -> 94.0. ActorStateParamScale configured-1.0 versus 1.3 source/accessor smoke rejects all seven rows because its 136-byte overload falls 100 -> 83.911766; improving the other overload cannot unpin the unit.
