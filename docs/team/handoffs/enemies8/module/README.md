# MarioModule source-only forcing cleanup

One natural form removes the 16-byte MarioModule_FORCE_EMIT_STRING function (absent from the DOL), moves unchanged checkSquat before isInputDisable, and moves unchanged isSlipPolygon before isSlipFloorCode. No replacement consumer is introduced. The source-only OSReport tail call disappears; every surviving body and its actual calls, constants, branches and fields remain intact.

The proper .data ownership is 806DD6BC..806DD880 (452 bytes). Compiled data448B plus4B linker alignment matches that complete span and all91ADDR32 cells. The real P_PlanetMap prefix at806DD6B0..D6BC stays outside the unit; no global dead-object claim is made. Adjacent Messenger48B and Move24B retain complete original data/pointers.

Post-split validation binds all4944 target text bytes to original DOL, including121REL24/25SDA21/4HA/4LO relocations and actual startup r2/r13 bases. This is target binding, not a whole-source-match certificate. Source preservation covers107surviving bodies,159effective refs and four complete string reaches,72linked calls/53tail calls and25scalar loads.

Strict SG remains89/98; report matched code remains3892/4480B with0B gain. The unchanged default checker still rejects isInputDisable at ambiguous pool origins. Full local ninja/hash passes and no unit loses matched report bytes. Other-main rebase gains in root-report-delta are explicitly excluded from this change. The unit stays NonMatching.
