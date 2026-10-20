# OdysseyMovement mod on SMG2 (game-side hooks)

Ported from MacioGalaxy (SMG1 port) by the core lane, 2026-10-19 night. The SMO movement model itself
(native/include/petari/odyssey_move.hpp) and the mods switch (native/input/mods.cpp, `OdysseyMovement`,
`OdysseyJumpDive`) were already in the tree; this adds the game-side hooks in Mario.cpp (+ MarioSMG2_A.inc),
MarioWall.cpp and MarioActor.cpp, all inside `#ifdef GALAXY_NATIVE`. Cappy and the Odyssey camera are separate mods
and are not ported (Cappy call sites are marked with one-line comments).

Wii build: unchanged code and data. For Mario.o, MarioWall.o and MarioActor.o every ELF section except `.line`
(DWARF source line numbers, which move with any added line) is byte-identical to origin/main.
Native: the three TUs pass native/tools/audit_compile.py with the same warnings as origin/main; the model's
62 unit checks pass (`native/tests/odyssey_move_tests.cpp`, registered as `native_odyssey_move`).
Lead review changes: the chain-jump hook applies only to plain Mario (player mode 0, not riding Yoshi), matching the
mod's rule that power-ups and rides keep Galaxy's jumps. `MarioConstTable::mWalkSpeed` (0x224, the units/frame conversion)
is the field the target's own doAirWalk loads, and getJumpHeight() returns mJumpHeight[_430], so the chain index drives
the height as in MacioGalaxy.

Porting notes (hook table, bit-mask mapping, open points, in-game test list) follow.

# Odyssey movement port notes (MG/SMG1 -> SMG2, native only)

Worktree: ~/MacioGalaxy2-wt/odyssey (branch wt/odyssey, not committed). Files changed:
src/Game/Player/Mario.cpp, MarioSMG2_A.inc, MarioWall.cpp, MarioActor.cpp. Everything is inside `#ifdef GALAXY_NATIVE`
(where an original statement had to be replaced, it is kept verbatim in the `#else` branch, as MG did for MarioWall).

## Hooks ported (MG function -> SMG2 function, anchor)
| MG | SMG2 | Anchor |
| --- | --- | --- |
| MarioJump.cpp file scope 31-208 | Mario.cpp, after the last #include | shared block (sOdyssey, odysseyOn/Start/StartDive/GroundPoundLandingSoon). Cappy parts dropped (petariOdysseyAirThrow, petariOdysseyCapBounce, MarioCappy.hpp include) |
| tryJump roll -> long jump | Mario::tryJump | just before `if (fn_803B2530(squat)) return;`, MG's full condition (floor != 0x20, mode != 4, _430 != 3, _23 clear) |
| tryJump chain jump (nextChainIndex) | Mario::tryJump | after the `ratio = 0.9f` line, before `getJumpHeight()` (reads mJumpHeight[_430]; only the cloud suit special-cases _430 0, so the new _430 is used) |
| tryJump odysseyStart | Mario::tryJump | after `if (_10._1A) _430 = 0` equivalent, before `procJump(true)` |
| tryTurnJump (sideflip) | Mario::tryTurnJump | before `procJump(true)` |
| trySquatJump (long jump) | Mario::trySquatJump | before `procJump(true)` |
| tryBackJump (backflip) | Mario::tryBackJump | before `procJump(true)` |
| trySpinJump no spin in dive/long jump | Mario::trySpinJump | right after the `_B` (ground pound) early return (the brief's anchor; MG has it before) |
| tryWallJump | Mario::tryWallJump | after `mRabbit->forceJump()` |
| procJump air step (1728-1787) | Mario::fn_8039A600 (MarioSMG2_A.inc), the `!(low>>30)` branch | SMG2 splits procJump; this is the helper that does addVelocity + gravity. The mod block runs first and `return`s when active (nothing follows in that helper), else Galaxy's step runs |
| procJump landing bookkeeping (1823) | Mario::procJump | the `{ doLanding(); fn_8039ACE0(); }` line is wrapped in `#ifdef/#else` so the bookkeeping sits before doLanding() |
| procHipDrop dive on Spin / Jump (2082-2098) | Mario::procHipDrop | top of function |
| procHipDrop ground-pound jump window, sOdysseyGroundPoundJump | Mario::procHipDrop | before the `_3CE > 5 && checkTrgA()` test (`if (odysseyOn()) {...} else <original>`), and before `tryJump()` |
| procHipDrop 45 fall limit | Mario::procHipDrop | `if (odysseyOn()) {...} else <original limit test>` |
| doLanding (2891+) | Mario::doLanding | after `fixFrontVecByGravity()` (past the hard/blend computation); new label `POST_LANDING:` before the final slope block |
| MarioWalk updateWalkSpeed roll + run | Mario::updateWalkSpeed (MarioSMG2_A.inc) | roll at function start; run right before the final `mWalkSpeed = ...` |
| MarioWall::update (463, 585) | MarioWall.cpp MarioWall::update | release test and `_20` slide step, `#ifdef/#else` copies of the originals |
| MarioActor::controlMain swing edge | MarioActor.cpp | after updateControllerSwingLeft() |
| MarioActor::updateSwingAction | MarioActor.cpp | after the `_946 = 0` reset, before `requestRush`; spin-request trace after `if (!requestRush) return;` |

Left as one-line comments `// Cappy mod (separate, not ported): <call>`: petariCappyDiveStarted, petariCappyWallJump, petariCappyInput,
petariCappySwingAction, capThrowBrake (updateWalkSpeed), petariOdysseyAirThrow / petariOdysseyCapBounce definitions.
odysseyAirVelocity / odysseyUpSpeed are kept (`[[maybe_unused]]`) because the Cappy port will need them.
No CameraDirector/OdysseyCamera hooks were touched.

## Mapping decisions
- gravity `_240` -> `mActor->mAppliedGravityVec`; `getConst().getTable()` -> `mConst->getTable()`; `mPosition` -> `mPosHolder->mPos`.
- Bit masks (all hex indices): `_B` low 0x00100000 (ground pound), `_1` low 0x40000000 (grounded), `jumping` low 0x80000000, `_A` low 0x00200000 (squat),
  `_4` low 0x08000000, `_F` low 0x00010000 (tornado), `_10` low 0x8000 (turn slip), `_23` high 0x10000000, `_26` high 0x02000000,
  `_10._1A` mFlags14.low bit 5, `_1C._A/_B` mFlags20 0x00200000/0x00100000, `_20._A` mFlags24.mLowWord 0x00200000 (mFlags24 low is a copy of the movement states, Mario.cpp:494), `mDrawStates` unused.
- Floor code: SMG2 tryJump uses `fn_803EF7C0()` (= getFloorCode, compared with 0x20 for the mud floor); used as such.
- Ground polygon: `mGroundPolygon` (pointer) instead of getGroundPolygon(); `std::acos` instead of MR::acos in the roll's slope trace-free calc.
- Dive: clearing `_B` is `mLowWord &= 0xFFEFFFFF` (the same mask SMG2's own landing code uses).

## TODO / unclear (need a human or in-game check)
- No `TODO(odyssey)` markers were needed; every hook had a clear SMG2 equivalent. Points to watch:
  - `MarioConstTable::mWalkSpeed` and `mJumpHeight[]` are marked "SMG2 offset guess" in MarioConst.hpp. The chain jump speed (`mWalkSpeed * table->mWalkSpeed`) and the run `unit` depend on mWalkSpeed being the right field.
  - `_3CE` (grounded frames for the chain, landing frames for the ground-pound jump window) is cleared in doLanding and counted elsewhere as in MG; confirm it counts per frame while standing in SMG2.
  - procJump's SMG2 gravity step in fn_8039A600 also scales by fn_8039AA40()/calcJumpGravity() (cling/bee/wall-slide etc); like MG the mod ignores those while active.
  - SMG2 has more jump types than SMG1 (Yoshi, cloud, rabbit): hooks require getPlayerMode() == 0 for roll/run/dive, but the chain-jump hook (as in MG) only excludes modes 4 and 6; Yoshi (mode 1?) / cloud jumps with the mod on were not examined.
  - SMG2's trySquatJump and tryBackJump differ slightly from MG (stick vector in the squat jump, Yoshi sounds); the hook sits at the same place (before procJump).
  - MarioWall case 2 forces `_14 = 165` as in MG, so while the stick is away from the wall the `WallKeepFrame` animation/sound conditions fire immediately; same as MG.

## Verified
1. Native compile (`audit_compile.py --root . --jobs 2` on Mario.cpp, MarioWall.cpp, MarioActor.cpp): 3/3 pass, 0 errors; warning counts identical to an untouched origin/main worktree (8 / 4 / 7). Reports: /tmp/ody-audit.json (new), /tmp/ody-audit-base.json (base).
2. Wii objects (MWCC command from `ninja -t commands`, compiled into /tmp/ody-out-base and /tmp/ody-out-new): the whole-file sha256 differs, but ONLY in the `.line` section (source line numbers; every added line shifts them, so a whole-file hash cannot match for any change that adds lines, GALAXY_NATIVE or not). All other sections (.text, .data, .rodata, .sdata/.sdata2, .sbss/.bss, .ctors, .symtab, .strtab, .debug, .comment and all .rela.*) are byte-identical for Mario.o, MarioWall.o and MarioActor.o (checked with /tmp/odyp/elfcmp.py). Path independence of the hash was confirmed by compiling the base from a second directory.
3. `clang++ -std=c++17 -I native/include native/tests/odyssey_move_tests.cpp -o /tmp/ody_tests && /tmp/ody_tests`: 62 Odyssey movement model checks passed.

## Needs in-game testing (each row of the MODS.md table; none of it was run)
Run (14 u/f after 40 frames); single/double/triple jump + chain timing (10 frames, full speed, 45 deg) and hold-for-height; long jump + landing running;
backflip; sideflip; ground pound (45 u/f fall); ground-pound jump (frames 5-30); dive via Spin and via Jump (OdysseyJumpDive) incl. landing roll / belly flop;
roll, roll boost (15 frames), long jump out of a roll; wall slide (3 frames hold, then 0.5 gravity to 35) and wall jump; plus mod-off behaviour unchanged.
Also confirm the animation names exist in SMG2's Mario (幅とび, 飛び込み失敗回転着地, 飛び込み失敗着地, 落下, 壁すべり).
