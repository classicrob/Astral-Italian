# First20 compiler-setting experiment

All500 global variants compiled/scored:25 individual settings on20 units, including16 Util/System units. Two evidence-backed range trials add46 source-pragma variants.

No unit gains exact matched bytes without per-function regressions. No exact-gain candidate survives even the weaker exact-anchor-only guard. No production compiler-setting change is proposed.

Two no-drop fuzzy-only improvements add zero exact bytes: WorldMapResultTU07 `opt_propagation off` (+29.600 weighted fuzzy bytes) and ActorInitUtil `opt_loop_invariants off` (+10.760). These are diagnostics, not byte gains or correctness proofs.

Range trials: the full five-function AreaObjUtil late tail and the two explicit ActorInitUtil texture overloads. Six header-defined ActorInit tail functions remain outside the range. No justified BellyDragon internal boundary was found, so none was invented. Neither tested range improves without drops.

The first long scratch path caused MWCC file-not-found/load failures. Every such case was repeated under short `/tmp` paths. All repeated baseline function scores/source inventories and common input hashes match; all500 global and46 range cases are finally scored. Successful paths reached254 characters; failures began at255. Initial exit1 and repair exit0 receipts remain separate. Use a short `--out` prefix.

Tool tests:11 passed. Actual MWCC smoke covers global/file/range placement, illegal-pragma negative control, source-relative `.inc` lookup and rejection of an88B exact-anchor loss. Full-report100/matched_code is authoritative; DiffJSON remains diagnostic. All production source/config/compiler inputs and primary objects were unchanged during search.

Frozen benchmark context: main237165c9dc2ac1506ee53b4e23c2c77b9ebdb73c; tool SHA1225ffad9429af4d52d8fa07d67da0633428511a3041effd6d17635130897952. Shared artifacts at `scratch-enemies/sol8/flagsearch`: REPORT.md, FINAL-SUMMARY.json, combined-validated-report.json, original requests, range-selection.json and repair receipts. Production adoption would still need original-file evidence, owner review and normal data/native/full-build checks.
