# U024: typed auto-dead switch on native builds

`LiveActor::kill` now reads `StageSwitchCtrl::mUsesAutoDeadSwitch` under
`GALAXY_NATIVE`. The original Wii statement is retained verbatim in `#else`.
No fields, layout, switch conditions, or calls change.

The actual DOL at `80237500..80237568` calls makeActorDead, checks the controller,
calls isValidSwitchDead, reloads the controller, then loads its byte at +0x10
before conditionally calling MR::onSwitchDead. The canonical class has four
pointers before that boolean. Its actual native offset is 0x20, so a raw +0x10
read observes a byte of mSW_Appear instead. That can falsely enable or disable
the actor's death switch and can also produce an invalid bool byte.

Validation:

- The actual canonical native header's offsetof is 32. A clang layout probe
  projected from that same class declaration gives 16 under powerpc-eabi and
  32 under arm64. An actual-header MWCC offsetof sample returns 16, independently
  cross-checking the PPC approximation against the game's compiler.
- `python3 native/tests/actor_dead_switch_native_test.py` extracts the actual
  production kill body and uses the actual StageSwitchCtrl header. Seventeen
  sanitizer checks cover all null/valid/auto-switch combinations, pointer bytes
  that disagree with the bool, call order, and controller reload after the query.
- The test fixtures only record death/query/switch calls; they do not replace
  providers in the game. Selecting the original raw-offset statement against the
  native layout rejects the functional negative control, even with 0/1 bytes.
- A fresh isolated LiveActor Wii compile passes current tools/team/objsame.py
  against its pre-edit baseline. The complete native TU passes compile audit.

The wider mechanical raw-offset inventory is separate tools/team/offsetaudit.py
work. This one-body correction does not certify other hard-coded offsets or
claim an in-game death-switch scenario was run on the data-free M1.
