# M1A list 2: scalar ownership proposal (read-only, quiet M1)

No split, symbol, build configuration or target objects were changed. This proposal is evidence only; no code/data gain is claimed. All instruction references below come from the original DOL, not source relocations. A global scan of all seven DOL text sections found only the named owner for each proposed range. Ranges are half-open.

| Unit | Current owned range | Proposed range/change | Added bytes | Direct DOL references |
|---|---|---|---:|---:|
| ImageEffectLocalUtil | .sdata2 807E1470..807E1490 | Extend start to 807E1468 | 8 | 36 |
| Jugem | .sdata2 807DA8B8..807DA96C | Extend to 807DA8B0..807DA97C | 24 | 50 |
| BellyDragonTU02 | .sdata2 807D7700..807D7818 | Extend end to 807D781C | 4 | 1 |
| BellyDragonTU02 | No .sbss2 assignment | Add 807E2FD8..807E2FE0 | 8 | 3 |
| DiceMover | No .sdata2 assignment | Add 807D7128..807D7158 | 48 | 45 |
| Battan | No .sdata2 assignment | Add 807D9C10..807D9CC8 | 184 | 91 |

Total: 276 bytes of supported ownership. Existing auto/unowned sections must be shortened by the lander. Scalar emission order is still different in the current sources, so ownership changes alone are not a claim of exact matched data. These are not evidence of additional text TUs.

## ImageEffect prefix 0x807e1468..0x807e1470
- 0x807e1468 = 3f800000 (1.0), 9 references; 0x80476f40 instruction 0xc00227c8 in `sendTextureVertex__20ImageEffectLocalUtilFll+0x130`
- 0x807e146c = 00000000 (0.0), 27 references; 0x80476a2c instruction 0xc02227cc in `setupDrawTexture__20ImageEffectLocalUtilFv+0x4C`
## Jugem prefix 0x807da8b0..0x807da8b8
- 0x807da8b0 = 3f800000 (1.0), 12 references; 0x8019ad40 instruction 0xc062bc10 in `moveJugemKeepingHeight__FP9LiveActorRCQ29JGeometry8TVec3<f>bfffff+0xD0`
- 0x807da8b4 = 00000000 (0.0), 34 references; 0x8019abf4 instruction 0xc022bc14 in `turnJugemFrontToTarget__FPQ29JGeometry8TVec3<f>P9LiveActorRCQ29JGeometry8TVec3<f>f+0x24`
## Jugem suffix 0x807da96c..0x807da97c
- 0x807da96c = 43340000 (180.0), 1 references; 0x8019f2e0 instruction 0xc042bccc in `__sinit_\Jugem_cpp+0xF0`
- 0x807da970 = 42dc0000 (110.0), 1 references; 0x8019f2e4 instruction 0xc062bcd0 in `__sinit_\Jugem_cpp+0xF4`
- 0x807da974 = 428c0000 (70.0), 1 references; 0x8019f314 instruction 0xc022bcd4 in `__sinit_\Jugem_cpp+0x124`
- 0x807da978 = c28c0000 (-70.0), 1 references; 0x8019f31c instruction 0xc042bcd8 in `__sinit_\Jugem_cpp+0x12C`
## BellyDragon suffix 0x807d7818..0x807d781c
- 0x807d7818 = 42b40000 (90.0), 1 references; 0x8009148c instruction 0xc0228b78 in `calcJoint__19BellyDragonBodyPartFPQ29JGeometry64TPosition3<Q29JGeometry38TMatrix34<Q29JGeometry13SMatrix34C<f>>>RC19JointControllerInfo+0x22C`
## BellyDragon sbss2 0x807e2fd8..0x807e2fe0
- 0x807e2fd8 = None (BSS zero), 1 references; 0x8009037c instruction 0xc0224338 in `exeBindB__19BellyDragonBodyPartFv+0x3C`
- 0x807e2fdc = None (BSS zero), 2 references; 0x80090310 instruction 0xc022433c in `exeExitA__19BellyDragonBodyPartFv+0x120`
## DiceMover sdata2 0x807d7128..0x807d7158
- 0x807d7128 = 3f800000 (1.0), 11 references; 0x800546a4 instruction 0xc0428488 in `normalizeComponents__13DiceMoverUtilFPfPfPf+0x44`
- 0x807d712c = 00000000 (0.0), 16 references; 0x80054930 instruction 0xc002848c in `setMtx__9DiceMoverFPA4_f+0x50`
- 0x807d7130 = 3f000000 (0.5), 1 references; 0x80054794 instruction 0xc0628490 in `averageNormalComponents__13DiceMoverUtilFPfPfPfPfPfPf+0x64`
- 0x807d7134 = 36800000 (3.814697265625e-06), 2 references; 0x800559a8 instruction 0xc0028494 in `applyCollision__9DiceMoverFv+0x118`
- 0x807d7138 = bf800000 (-1.0), 2 references; 0x80054d8c instruction 0xc3c28498 in `resolveCollision__9DiceMoverFPCQ29JGeometry8TVec3<f>PCQ29JGeometry8TVec3<f>f+0xAC`
- 0x807d713c = 40000000 (2.0), 2 references; 0x80054fcc instruction 0xc002849c in `init__9DiceMoverFv+0x4C`
- 0x807d7140 = 3f7fbe77 (0.9990000128746033), 6 references; 0x80054dc0 instruction 0xc3a284a0 in `resolveCollision__9DiceMoverFPCQ29JGeometry8TVec3<f>PCQ29JGeometry8TVec3<f>f+0xE0`
- 0x807d7144 = 3cf5c28f (0.029999999329447746), 1 references; 0x80054dbc instruction 0xc00284a4 in `resolveCollision__9DiceMoverFPCQ29JGeometry8TVec3<f>PCQ29JGeometry8TVec3<f>f+0xDC`
- 0x807d7148 = be4ccccd (-0.20000000298023224), 1 references; 0x80054fc8 instruction 0xc02284a8 in `init__9DiceMoverFv+0x48`
- 0x807d714c = 403fef9e (2.999000072479248), 1 references; 0x800552e4 instruction 0xc3e284ac in `integrate__9DiceMoverFv+0xE4`
- 0x807d7150 = 3e800000 (0.25), 1 references; 0x80055850 instruction 0xc02284b0 in `addGravity__9DiceMoverFv+0x20`
- 0x807d7154 = 3c23d70a (0.009999999776482582), 1 references; 0x80055fa8 instruction 0xc3a284b4 in `addCollision__20DiceMoverAccumulatorFPCQ29JGeometry8TVec3<f>PCQ29JGeometry8TVec3<f>PCQ29JGeometry8TVec3<f>f+0xF8`
## Battan sdata2 0x807d9c10..0x807d9cc8
- 0x807d9c10 = 3f800000 (1.0), 4 references; 0x80159248 instruction 0xc062af70 in `__ct__6BattanFPCc+0x58`
- 0x807d9c14 = 00000000 (0.0), 23 references; 0x8015922c instruction 0xc082af74 in `__ct__6BattanFPCc+0x3C`
- 0x807d9c18 = 36800000 (3.814697265625e-06), 2 references; 0x8015adec instruction 0xc002af78 in `moveBattanOnRail__6BattanFf+0x10C`
- 0x807d9c1c = b6800000 (-3.814697265625e-06), 1 references; 0x8015af4c instruction 0xc342af7c in `appearBattanBreakParts__6BattanFv+0x6C`
- 0x807d9c20 = 3f000000 (0.5), 4 references; 0x8015b344 instruction 0xc042af80 in `calcBattanPlayerPushDirection__6BattanFPQ29JGeometry8TVec3<f>PCQ29JGeometry8TVec3<f>+0x74`
- 0x807d9c24 = bf800000 (-1.0), 1 references; 0x801595f4 instruction 0xc002af84 in `init__6BattanFRC12JMapInfoIter+0x324`
- 0x807d9c28 = 3fc90fdb (1.5707963705062866), 1 references; 0x8015af5c instruction 0xc3a2af88 in `appearBattanBreakParts__6BattanFv+0x7C`
- 0x807d9c2c = bfc90fdb (-1.5707963705062866), 1 references; 0x8015af64 instruction 0xc362af8c in `appearBattanBreakParts__6BattanFv+0x84`
- 0x807d9c30 = 3f000000 (0.5), 1 references; 0x8015ae1c instruction 0xc022af90 in `moveBattanOnRail__6BattanFf+0x13C`
- 0x807d9c34 = 44480000 (800.0), 1 references; 0x8015945c instruction 0xc022af94 in `init__6BattanFRC12JMapInfoIter+0x18C`
- 0x807d9c38 = 43c80000 (400.0), 1 references; 0x8015aafc instruction 0xc042af98 in `receiveMsgPlayerAttack__6BattanFUlP9HitSensorP9HitSensor+0x3C`
- 0x807d9c3c = 43480000 (200.0), 1 references; 0x8015aaf8 instruction 0xc022af9c in `receiveMsgPlayerAttack__6BattanFUlP9HitSensorP9HitSensor+0x38`
- 0x807d9c40 = 43fa0000 (500.0), 1 references; 0x8015b3a0 instruction 0xc022afa0 in `calcBattanPlayerPushDirection__6BattanFPQ29JGeometry8TVec3<f>PCQ29JGeometry8TVec3<f>+0xD0`
- 0x807d9c44 = 3f800000 (1.0), 2 references; 0x8015a9f0 instruction 0xc022afa4 in `attackSensor__6BattanFP9HitSensorP9HitSensor+0x140`
- 0x807d9c48 = 42c80000 (100.0), 1 references; 0x8015b604 instruction 0xc022afa8 in `updateHitSensor__6BattanFP9HitSensor+0x154`
- 0x807d9c4c = 3c23d70a (0.009999999776482582), 3 references; 0x801598b4 instruction 0xc002afac in `exeBattanWalk__6BattanFv+0xD4`
- 0x807d9c50 = 3dcccccd (0.10000000149011612), 1 references; 0x80159a80 instruction 0xc022afb0 in `exeBattanHalfTurn__6BattanFv+0x80`
- 0x807d9c54 = 43c80000 (400.0), 2 references; 0x8015a1a8 instruction 0xc022afb4 in `exeBattanDown__6BattanFv+0xB8`
- 0x807d9c58 = 41200000 (10.0), 1 references; 0x8015a1cc instruction 0xc022afb8 in `exeBattanDown__6BattanFv+0xDC`
- 0x807d9c5c = 42200000 (40.0), 1 references; 0x8015a1d4 instruction 0xc042afbc in `exeBattanDown__6BattanFv+0xE4`
- 0x807d9c60 = 43160000 (150.0), 2 references; 0x8015ad70 instruction 0xc002afc0 in `moveBattanOnRail__6BattanFf+0x90`
- 0x807d9c64 = 41f00000 (30.0), 1 references; 0x8015add4 instruction 0xc3e2afc4 in `moveBattanOnRail__6BattanFf+0xF4`
- 0x807d9c68 = 42652ee1 (57.295780181884766), 1 references; 0x8015af68 instruction 0xc3e2afc8 in `appearBattanBreakParts__6BattanFv+0x88`
- 0x807d9c6c = 43960000 (300.0), 2 references; 0x8015b238 instruction 0xc022afcc in `calcBattanBreakPartPosition__6BattanFPQ29JGeometry8TVec3<f>l+0x108`
- 0x807d9c70 = 42480000 (50.0), 2 references; 0x8015b5c4 instruction 0xc002afd0 in `updateHitSensor__6BattanFP9HitSensor+0x114`
- 0x807d9c74 = 44480000 (800.0), 1 references; 0x8015b750 instruction 0xc022afd4 in `__sinit_\Battan_cpp+0x80`
- 0x807d9c78 = 44a28000 (1300.0), 3 references; 0x8015b758 instruction 0xc042afd8 in `__sinit_\Battan_cpp+0x88`
- 0x807d9c7c = 44c80000 (1600.0), 2 references; 0x8015b75c instruction 0xc062afdc in `__sinit_\Battan_cpp+0x8C`
- 0x807d9c80 = 44160000 (600.0), 1 references; 0x8015b764 instruction 0xc022afe0 in `__sinit_\Battan_cpp+0x94`
- 0x807d9c84 = 43e10000 (450.0), 1 references; 0x8015b778 instruction 0xc022afe4 in `__sinit_\Battan_cpp+0xA8`
- 0x807d9c88 = 442f0000 (700.0), 3 references; 0x8015b784 instruction 0xc062afe8 in `__sinit_\Battan_cpp+0xB4`
- 0x807d9c8c = c3c80000 (-400.0), 1 references; 0x8015b794 instruction 0xc042afec in `__sinit_\Battan_cpp+0xC4`
- 0x807d9c90 = 44034000 (525.0), 1 references; 0x8015b7a8 instruction 0xc042aff0 in `__sinit_\Battan_cpp+0xD8`
- 0x807d9c94 = c2c80000 (-100.0), 1 references; 0x8015b7ac instruction 0xc062aff4 in `__sinit_\Battan_cpp+0xDC`
- 0x807d9c98 = 44548000 (850.0), 2 references; 0x8015b7bc instruction 0xc042aff8 in `__sinit_\Battan_cpp+0xEC`
- 0x807d9c9c = 43480000 (200.0), 1 references; 0x8015b7c0 instruction 0xc062affc in `__sinit_\Battan_cpp+0xF0`
- 0x807d9ca0 = c2480000 (-50.0), 1 references; 0x8015b7f8 instruction 0xc022b000 in `__sinit_\Battan_cpp+0x128`
- 0x807d9ca4 = c3147021 (-148.43800354003906), 1 references; 0x8015b804 instruction 0xc022b004 in `__sinit_\Battan_cpp+0x134`
- 0x807d9ca8 = 42c80000 (100.0), 4 references; 0x8015b80c instruction 0xc042b008 in `__sinit_\Battan_cpp+0x13C`
- 0x807d9cac = 4453ab85 (846.6799926757812), 1 references; 0x8015b810 instruction 0xc062b00c in `__sinit_\Battan_cpp+0x140`
- 0x807d9cb0 = 43379810 (183.593994140625), 1 references; 0x8015b818 instruction 0xc022b010 in `__sinit_\Battan_cpp+0x148`
- 0x807d9cb4 = 443526fa (724.6090087890625), 1 references; 0x8015b824 instruction 0xc062b014 in `__sinit_\Battan_cpp+0x154`
- 0x807d9cb8 = c36b0000 (-235.0), 1 references; 0x8015b82c instruction 0xc022b018 in `__sinit_\Battan_cpp+0x15C`
- 0x807d9cbc = 440dd883 (567.3829956054688), 1 references; 0x8015b838 instruction 0xc062b01c in `__sinit_\Battan_cpp+0x168`
- 0x807d9cc0 = 422c0000 (43.0), 1 references; 0x8015b840 instruction 0xc022b020 in `__sinit_\Battan_cpp+0x170`
- 0x807d9cc4 = 43c40000 (392.0), 1 references; 0x8015b84c instruction 0xc062b024 in `__sinit_\Battan_cpp+0x17C`

## Baseline data and boundary review

- All nine owned .data payloads have zero non-relocation byte differences over the common range. In particular, every BattanKing/Battan/ItemDice string occurs at the target offset. ItemDice target has only four additional trailing zero bytes at 806A3324..806A3328. No missing string should be inserted into those pools.
- ItemDice duplicate-string scanner hit at 806A3318 is the inline BenefitItemOneUp constructor string KinokoOneUp; the source already emits it there. All class and nerve vtables also share the expected string layout. No supported split based on this hit.
- BattanKing init target loads collision descriptor base independently: init+44 lis r4,lbl_8066C178@ha and +54 addi r26,r4,lbl_8066C178@l. Ours uses the existing pooled base+E0. Both source and DOL table bytes occupy 8066C178..8066C1B4; no absent pool prefix causes this difference. Read-only descriptor constness/linkage is a source hypothesis for later testing.
- BattanKing, BellyDragonTU02, Jugem, ItemDice and Battan have only one initializer each, near the end followed by weak/helper tails. Addresses: 8008D310/972B, 80092D10/92B, 8019F1F0/352B, 802B5630/356B, 8015B6D0/428B. DepthOfFieldBlur, ImageEffectLocalUtil, J3DModelLoader and DiceMover have no initializer. No separate interior sinit/ctors boundary was found.
- Exact DOL data-reference sites, owner counts and raw words are in global-ownership.json; external-sda-ownership.json contains every unit-local site; data-compare.json records byte comparisons. These scripts do not compile or mutate source/configuration.
