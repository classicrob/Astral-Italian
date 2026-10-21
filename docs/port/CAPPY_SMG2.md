# Cappy in SMG2

This is an opt-in `GALAXY_NATIVE` adaptation of MacioGalaxy's game-side Cappy,
not a Wii decompilation change. `Cappy` and `OdysseyMovement` both default off.
R/controller RB throws when both are enabled; stage reload is required after
enabling Cappy so the actors can be created safely during player initialization.
This port restores the Cappy menu option; the Odyssey movement dependency remains.

## Reused design and SMG2 differences

The donor is read-only `~/MacioGalaxy`: `MarioCappy.cpp/.hpp`, the Cappy flight
and contact model, and MarioJump's cap-hop/bounce integration. This checkout
already contained `native/include/petari/cappy.hpp`, the Odyssey air model,
input binding and movement-smoke tasks. The port connects those existing pieces.
No donor files or game assets are changed.

- Hook initialization at `MarioActor::initParts`, buffered input before the
  actor's action update, throw decisions before ordinary swing handling, and
  flight/bounce after the actor's normal movement. Calls return immediately
  without Cappy actors while the option was off at initialization.
- Use SMG2 `mAppliedGravityVec`, its explicit low-word flags (jump bit31,
  ground bit30, ground-pound bit20, crouch bit21), and `mPosHolder` rather than
  donor field names. The existing Odyssey state remains in SMG2's Mario unity
  TU; cap throw/bounce share its gravity-relative air integration.
- Restore the jump-press dive budget and airborne frame count on a cap bounce.
  The runtime observer counts an actual dive begun after a cap bounce; the
  movement-smoke loop fails if it merely sends a second dive input.
- Use SMG2's player modes as a strict domain: normal mode0 only. Yoshi mode10,
  Cloud mode12, Rock and all other suits recall the cap. Swimming, damage,
  hanging/climbing, talk/warp/slider/first-person states, bound rides, demos and
  disabled control do not start a throw. Power-up abilities remain the game's.
- SMG2 has both `MarioHair.arc` and `LuigiHair.arc`, with CapRoot/CapF/Cap2,
  Cap_v/Hair_v/Hair2_v. Both use their own model and exposed hair; the donor's
  Mario-only visual limitation is not needed here. The native game/resource
  APIs load these archives; nothing is copied into the repository.
- Keep the same map ray, enemy spin-punch and coin/Star-Bit item-get routes.
  Captures and Generic-character cap rendering are outside this adaptation.
- Flip-Swap's spin-switch area (`SpinSwitchArea`): a real Galaxy Spin always
  flips it once, that frame; each throw's cap flips it at most once. A Spin
  pressed before the cap of an earlier throw reaches the area takes that
  throw's pending contact there (`petariCappyConsumeSpinSwitchArea`), so the
  cap, ~8 frames after an air throw, does not flip it straight back. A throw
  issued after the Spin still counts once. A Spin pressed over a ground throw's
  borrowed animation (which Galaxy reads as a spin already running) is held
  until the cap has left the hand (frame 12), then starts the game's own Spin
  (`petariCappyGroundSpinOverThrow`); no forced status or swing edge. Run test (needs the disc):
  `native/tests/cappy_spin_switch_tests.py`.
- The unported Odyssey orbit camera keeps its own absent observer. Cappy's
  observers now come from the real game-side implementation.

## Validation and delivery gates

Native syntax checks cover Mario, MarioActor and MarioCappy. Existing Cappy
model tests cover throw windup/flight/hover/return, wall stop, contact bounds,
repeat hops and three cap-bounce trajectories. Odyssey model tests cover the
movement shared with the cap. Input tests cover off/default behavior, R/RB,
Odyssey dependency and preservation of the other spin button. The menu test
also retains the general unavailable-option behavior used by the interim fix.

The port owner runs the real GX app on a fresh private user directory with:

```
PETARI_MODS=OdysseyMovement,Cappy PETARI_SMOKE=movement \
PETARI_MOVEMENT_ONLY='cappy throws,cappy loop,cappy jump-dive loop' \
PETARI_CAPPY_TRACE=1 <scratch Petari executable> \
  --disc <private SMG2 disc> --user <fresh private test user> \
  --stage RedBlueExGalaxy
```

The inherited smoke driver must produce actual throws, a cap bounce and a
post-bounce second dive before reporting success. Geometry can require a safe
fixture location; no collision bypass or forced bounce is an acceptance test.
Keep full logs and visual cap/return evidence privately. Recheck Fluffy Bluff,
where the user first reported the missing feature, with mods off and on.
No gameplay runtime pass is claimed by the source/syntax/model tests alone.

Compare freshly compiled before/after Wii objects with corrected
`tools/team/objsame.py`, including Mario and MarioActor and the port's normal
whole-build proof. Then rerun the native baseline pair with mods off. Land as a
separate port commit after those gates; the queued build-6 save migration is
independent and remains unchanged.
