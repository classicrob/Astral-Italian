# P1 worklist by lane

**The hook transfer is being re-landed; its first landing was reverted. All compile numbers below assume the transfer**, including the ht3 `TMatrix.hpp` and `types.h` repairs. They describe saved syntax audits, not the current checkout or a link/runtime pass. No compiler or CMake runs were made for this brief.

[ht3 compile report](COMPILE_STATUS.md): **1,693/2,045 passing, 352 failing**. With Sol B's 12 prepared patches in scratch: **1,904/2,045 passing, 141 failing** (**211 newly passing, zero regressions**); Game 1,727/1,844, JSystem 152/171, nw4r 25/30. See the [patches and validation](../../../../MacioGalaxy2-team/scratch-port/solb/patches/README.md) and [remaining diagnostics](../../../../MacioGalaxy2-team/scratch-port/solb/p1-final-classified.md). The primary audit excludes SDK/libms C sources.

| Lane | ht3 passing/checked | Patched scratch passing/checked | Manual port-clang | Manual port-native | Unsure |
| --- | ---: | ---: | ---: | ---: | ---: |
| core | 608/752 | 679/752 | 26 | 120 | 25 |
| objects | 477/571 | 555/571 | 3 | 15 | 17 |
| enemies | 343/405 | 382/405 | 20 | 55 | 78 |
| astra | 110/116 | 111/116 | 9 | 30 | 1 |
| m1 | 155/201 | 177/201 | 63 | 89 | 3 |

Compile totals use [compile ownership](COMPILE_STATUS.md#p1-directory-worklist); hook counts retain [Sol C's different lane map](HOOKS_MANUAL.md#lane-totals). Manual counts and [hazards](HAZARDS.md) are earlier overlay inventories, not counts remaining after the scratch patches. Hook locations below are resolved SMG2 anchors; diagnostic lines can differ. Use ledger group IDs when rebasing. Ranking estimates prioritize shared blockers, then local compile repairs, then native behavior/data work; first-error counts mask later errors.

## core

1. **Prepared shared fixes first:** patch 02, `include/Game/Map/CollisionParts.hpp:22` (`KCLFile`, 76 affected); patch 03, `include/Game/LiveActor/ShadowVolumeDrawer.hpp:22` (override mismatch, 59); then patch 08, `src/Game/Animation/XanimeResourceTable.cpp:351` (switch scope, one). Their affected sets have 131 newly passing units with all patches present. Coordinate CollisionParts/Xanime hooks listed under enemies rather than duplicate them.
2. **Next compile batch:** `src/Game/Gravity/PlanetGravityManager.cpp:33` (`GravityHostID`; port-clang header/signature groups together); `src/Game/Camera/CameraAnim.cpp:110` (retain `CanmValue` endian wrappers; hook anchors 108/115 are in the enemies ledger). Then `src/Game/AudioLib/AudRemixMgr.cpp:36` / `AudRemixSequencer.cpp:38` (SMG2 member mismatches), and native declarations at `src/Game/Scene/GameScene.cpp:498` / `src/Game/System/GameSystem.cpp:120` / `GameSystemSceneController.cpp:262`.
3. **Manual native hooks:** scene initialization/preemption at `src/Game/Scene/GameScene.cpp:445`; host resource/cache lifetime at `include/Game/System/HeapMemoryWatcher.hpp:48` and `src/Game/System/HeapMemoryWatcher.cpp:146,212`. Recalculate SMG2 allocation sizes. Full [core hook list](HOOKS_MANUAL.md#core).
4. **High-confidence hazards:** address truncation at `src/Game/Scene/StageDataHolder.cpp:300,533–537` (pair the `uintptr_t` field and arithmetic hooks), `src/Game/System/Overwrite.cpp:157`, and `src/Game/System/RSO.cpp:60`; full-slot pointer/value initialization at `include/Game/LiveActor/ActorParam.hpp:16`; endian word/byte alias at `include/Game/Util/Color.hpp:70`. Follow [hazard format rules](HAZARDS.md#triage-and-format-handling).

## objects

1. **Take core's collision/shadow fixes first**, then prepared patches 09–12: `src/Game/AreaObj/AreaObj.cpp:13` (unused editor include); `src/Game/Boss/Sandy.cpp:34`, `SandyHand.cpp:27`, `SandyMini.cpp:21` (paired-single replacements); `src/Game/Boss/TwoLegsBulletTU03.cpp:363` (namespace); `TwoLegsBulletTU07.cpp:57` / `TU08.cpp:37` (numeric camera argument). These local affected sets all pass: seven units.
2. **Next local blockers:** `src/Game/Boss/KoopaJrRobot.cpp:9` (missing `functional.hpp`), `src/Game/MapObj/DrillHole.cpp:10` (signature mismatch), then `src/Game/Ride/Plant.cpp:426` / `SphereAccelSensorController.cpp:48` (native observation/input declarations). Route shared `JAUSoundAnimator.hpp:94` to m1.
3. **Manual hooks:** port-clang `include/Game/Boss/TripodBossAccesser.hpp:1`, `src/Game/MapObj/CrystalCage.cpp:30`, `Sandstorm.cpp:234`; port-native `src/Game/Boss/Dodoryu.cpp:800,820` (model/matrix setup), `src/Game/MapObj/PlantGroup.cpp:230` (paired collision-ray groups), then observation/input hooks. [Objects ledger](HOOKS_MANUAL.md#objects) has 17 unresolved groups to inspect. **No high-confidence hazard rows** remain for this lane in the saved sweep; retain cast/raw-access/bitfield reviews without treating them as confirmed failures.

## enemies

1. **Consume core/m1 shared fixes first.** Then batch local portability repairs: weak linkage at `src/Game/Enemy/FlowerMimic.cpp:21`, `PatakuriBig.cpp:24` / `PatakuriBigTU01.cpp:36`; MWCC attributes at `Nyoropon.cpp:22`; sensor argument at `Pukupuku.cpp:581`; member-function qualification at `src/Game/NPC/NPCActor.cpp:440`. Each is a singleton first-error root in patched scratch.
2. **Manual port-clang/native:** rebase the callback groups together at `src/Game/NPC/TalkMessageCtrl.cpp:200` (current diagnostic `:188`, missing `condition`); `src/Game/Screen/LogoFader.cpp:23` (narrowing) and `SubMeterLayout.cpp:35` (numeric animation argument). Adapt `src/Game/Screen/CustomTagProcessor.cpp:642` native string handling and NPC/actor observation after declarations are available. Coordinate the ledger's CameraAnim/KCollision/CollisionParts work with core. [Enemies ledger](HOOKS_MANUAL.md#enemies): resolve the 78 unsure groups by SMG2 implementation, not by copying absent SMG1 features.
3. **High-confidence hazards:** `src/Game/Enemy/TeresaAtomic.cpp:69` and `src/Game/Screen/CustomTagProcessor.cpp:154–160` truncate pointers; `include/Game/NPC/TalkMessageCtrl.hpp:71` mixes pointer/value aliases. Keep addresses pointer-sized and initialize the full union slot; integrate message handling with m1's LMS path.

## astra

1. **Consume m1's `JAUSoundAnimator.hpp:94` repair**, then clear the four local roots: `src/Game/Player/MarioActor.cpp:1081` (switch scope), `MarioAnimator.cpp:518` (goto scope), `MarioWarp.cpp:391` (switch scope), `YoshiLockOnManager.cpp:37` (`sTargetMax`). MarioActor and MarioWarp's port-clang groups have anchors `:978` and `:306`; keep paired scoping groups together.
2. **Manual native data/payload hooks:** `include/Game/Player/MarioActor.hpp:812` must preserve **all seven SMG2 effect flags**, including stationary/bee-wind/special-smoke; `src/Game/Player/MarioActor.cpp:9326,9465,9513` must use pointer payload APIs. Port GhostPacket reads at `src/Game/Player/GhostPacket.cpp:17,94`, and ResTIMG host offsets at `CollisionShadow.cpp:401` / `J3DModelX.cpp:293`. See [Astra ledger](HOOKS_MANUAL.md#astra).
3. **Hazard review:** the sweep has **zero high-confidence Player findings**, with 79 review candidates. Prioritize state-word/bitfield and pointer-storage reviews alongside the hooks above; preserve SMG2 layouts and RNG order. Defer Cappy/Odyssey/editor feature work to P3.

## m1

1. **Prepared shared-header batch:** patches 01/04/05/06/07: `libs/JSystem/include/JSystem/JAudio2/JASGadget.hpp:74` (76 affected), `JASDSPInterface.hpp:106` (10), `libs/JSystem/include/JSystem/JUtility/JUTAssert.hpp:17` (3), `libs/RVL_SDK/include/revolution/mem/list.h:5` (5), `libs/libms/include/libms.h:5` (2). Their affected sets have 73 newly passing units with all patches present. Includes must stay outside C linkage on native.
2. **Largest remaining root:** `libs/JSystem/include/JSystem/JAudio2/JAUSoundAnimator.hpp:94` affects **15 files across lanes**; port-clang explicitly constructs `JAISoundID` from the endian-wrapped field (ledger anchor `:93`). Next fix pointer arithmetic at `src/JSystem/JKernel/JKRHeap.cpp:78`, `src/JSystem/JSupport/JSUMemoryStream.cpp:17,56`, and `src/JSystem/J3DGraphLoader/J3DAnmLoader.cpp:276,278,474,476`; these are high-confidence truncation hazards as well as compile work.
3. **Manual native resource hooks:** `src/JSystem/JUtility/JUTTexture.cpp:47,102` (wire offsets/host views), `src/JSystem/JKernel/JKRUnitHeap.cpp:104,148,207` (bitmap word order), and `src/JSystem/JParticle/JPAResource.cpp:903,954` (GX buffer extents). Transfer converters and entry hooks together; preserve four-byte wire offsets separately from host pointers. Full [m1 ledger](HOOKS_MANUAL.md#m1).
4. **High-confidence endian backlog:** LMS records at `libs/libms/include/libms.h:18,32,56,78,84` and raw reads at `src/libms/commonlib.c:30,34`; SMG2-only `libs/nw4r/include/nw4r/lyt/resources.h:209` (`ExtUserData`); RFL serialized bitfields/records at `libs/RVLFaceLib/include/RFLi_Types.h:262,345,352`; JPA record coverage in the [on-disc inventory](HAZARDS.md#on-disc-record-inventory). SMG1 BMG fixes do not cover MSBT/MSBF. Prioritize game/resource paths over SDK hardware casts; C sources were outside the syntax totals.

Apply native changes behind `GALAXY_NATIVE` or preserve Wii tokens for compatibility repairs; the lead owns application and Wii object-identity proof. Reconcile already prepared groups against the re-landed transfer before doing duplicate manual work. Detailed ledgers remain the source for groups omitted from this short brief.
