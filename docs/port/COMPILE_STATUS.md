# Native compilation status

Audited 2026-10-19 on arm64 macOS with Apple Clang 21.0.0 (clang-2100.3.34.2).
The original primary baseline ran 16:27:57–16:28:33 EDT. The current hooked
primary pass ran 16:54:49–16:57:09 EDT against
`~/MacioGalaxy2-team/scratch-port/ht3/overlay`, the final overlay announced by
the lead. It includes the transferred native hooks, `TMatrix.hpp` dependent-base
`this->get` fix, and `types.h` guards preserving native `nullptr` and `override`.
All current hooked results in this document come from this overlay. The shared
checkout's `src/`, `include/`, and `libs/` remain in flux and were not used for
this rerun; Sol B made no changes to them and no commits.

These are C++17 syntax checks with `GALAXY_NATIVE=1`, `-ferror-limit=5`,
`-Werror=return-type`, and no RTTI. Passing does not establish linking or runtime
behavior. The audit uses native compatibility headers and generated legacy
literal/UTF-16 sources under the overlay's `build/audit-sjis` directory.
Diagnostics map back to original source locations.

## Totals

| Source group | Original baseline passing/checked | With transferred hooks passing/checked | Hooked failing |
| --- | ---: | ---: | ---: |
| Game C++ | 121/1844 | 1538/1844 | 306 |
| JSystem C++ | 56/170 | 130/171 | 41 |
| nw4r C++ | 11/29 | 25/30 | 5 |
| Primary audit | 188/2043 | 1693/2045 | 352 |
| RVL_SDK C++ | 0/1 | 0/1 | 1 |
| libms C++ | 0/0 | 0/0 | 0 |
| Combined C++ | 188/2044 | 1693/2046 | 353 |

Among original units, **1506** changed from failing to passing and
**3** from passing to failing: a net gain of 1503.
The overlay adds 2 C++ units (2 passing)
and removes 0 baseline units.

New units:

- `src/JSystem/JUtility/JUTNativeResource.cpp`: passing.
- `src/nw4r/lyt/lyt_nativeResource.cpp`: passing.

Previously passing units that now fail:

- `src/Game/System/NANDManagerThread.cpp` — `src/Game/System/NANDManagerThread.cpp:4`: 'petari/editor/runtime.hpp' file not found.
- `src/Game/System/WPadButton.cpp` — `src/Game/System/WPadButton.cpp:36`: use of undeclared identifier 'petari_skip_pad'.
- `src/JSystem/JAudio2/JASDriverIF.cpp` — `libs/JSystem/include/JSystem/JAudio2/JASDSPInterface.hpp:106`: unknown type name 'JASDspAddr'.

Only `.cpp` files are discovered. The SDK's 301 `.c` files, libms's four `.c`
files, RVLFaceLib, Runtime, MSL_C, and MetroTRK are outside this syntax pass.
The zero-file libms row is not a successful validation. The separate
[hazard worklist](HAZARDS.md) is an earlier structural sweep and is not refreshed
by this compiler-only rerun.

The read-only reference reports 1605/1605 Game and 200/200 JSystem/nw4r passing
as of 2026-09-28 after its native hooks (`~/MacioGalaxy/native/COMPILE_STATUS.md`).
SMG2 Game is 1538/1844 (83.4%) with this overlay.
Inventories differ, so the reference is a readiness comparison rather than a
file-for-file regression test.

## Reproduce

Run from `/Users/roberthaisfield/MacioGalaxy2-wt/port`:

```sh
nice -n 10 python3 native/tools/audit_compile.py --root ~/MacioGalaxy2-team/scratch-port/ht3/overlay --jobs 6 --output ~/MacioGalaxy2-team/scratch-port/solb/native-audit-ht3.json src/Game src/JSystem src/nw4r
nice -n 10 python3 native/tools/audit_compile.py --root ~/MacioGalaxy2-team/scratch-port/ht3/overlay --jobs 6 --output ~/MacioGalaxy2-team/scratch-port/solb/native-audit-ht3-sdk.json src/RVL_SDK src/libms
python3 tools/port/classify_audit.py ~/MacioGalaxy2-team/scratch-port/solb/native-audit-baseline.json ~/MacioGalaxy2-team/scratch-port/solb/native-audit-rvl-sdk.json ~/MacioGalaxy2-team/scratch-port/solb/native-audit-libms.json --overlay ~/MacioGalaxy2-team/scratch-port/solb/native-audit-ht3.json ~/MacioGalaxy2-team/scratch-port/solb/native-audit-ht3-sdk.json --output ~/MacioGalaxy2-team/scratch-port/solb/native-audit-ht3-comparison.md
python3 tools/port/classify_audit.py ~/MacioGalaxy2-team/scratch-port/solb/native-audit-ht3.json ~/MacioGalaxy2-team/scratch-port/solb/native-audit-ht3-sdk.json --output ~/MacioGalaxy2-team/scratch-port/solb/native-audit-ht3-classified.md --json-output ~/MacioGalaxy2-team/scratch-port/solb/native-audit-ht3-classified.json
```

The original baseline reports are preserved in Sol B's scratch directory;
do not overwrite them using the in-flux checkout. Audit exit status 1 means
translation units failed; report generation succeeded. The scratch script
`write_ht3_status.py` regenerates this document, including diagnostic-file lane
ownership, from these raw reports.

## P1 directory worklist

Core owns Util, LiveActor, NameObj, System, Scene, Map, Camera, Animation, Gravity,
Effect, Demo, AudioLib, GameAudio, Speaker, and NWC24. Objects owns MapObj, Ride,
AreaObj, and Boss. Enemies owns Enemy, RhythmLib, Screen, and NPC. Astra owns
Player. M1 owns libraries and new units; unlisted Game directories are unassigned.
Each translation unit contributes its first error only.

| Directory | Lane | Checked | Passing | Failing | Baseline top 3 first-error classes | With transferred hooks (passing/checked) | With hooks top 3 first-error classes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| src/Game/Animation | core | 11 | 4 | 7 | unknown type / identifier: 4; inline asm / asm function: 1; template: 1 | 10/11 | other: 1 |
| src/Game/AreaObj | objects | 39 | 0 | 39 | unknown type / identifier: 39 | 37/39 | missing header: 1; other: 1 |
| src/Game/AudioLib | core | 43 | 7 | 36 | other: 23; unknown type / identifier: 8; ambiguous / overload: 5 | 18/43 | other: 22; unknown type / identifier: 3 |
| src/Game/Boss | objects | 166 | 0 | 166 | unknown type / identifier: 165; other: 1 | 112/166 | override / virtual mismatch: 27; unknown type / identifier: 19; other: 8 |
| src/Game/Camera | core | 126 | 4 | 122 | unknown type / identifier: 122 | 125/126 | other: 1 |
| src/Game/Demo | core | 39 | 8 | 31 | unknown type / identifier: 29; ambiguous / overload: 1; template: 1 | 35/39 | other: 3; unknown type / identifier: 1 |
| src/Game/Effect | core | 29 | 8 | 21 | unknown type / identifier: 16; other: 3; ambiguous / overload: 2 | 26/29 | other: 3 |
| src/Game/Enemy | enemies | 158 | 1 | 157 | unknown type / identifier: 154; other: 2; ambiguous / overload: 1 | 131/158 | unknown type / identifier: 15; other: 6; override / virtual mismatch: 4 |
| src/Game/GameAudio | core | 7 | 2 | 5 | other: 4; unknown type / identifier: 1 | 3/7 | other: 2; unknown type / identifier: 2 |
| src/Game/Gravity | core | 16 | 0 | 16 | unknown type / identifier: 16 | 15/16 | unknown type / identifier: 1 |
| src/Game/LiveActor | core | 80 | 7 | 73 | unknown type / identifier: 71; other: 2 | 66/80 | override / virtual mismatch: 10; other: 2; unknown type / identifier: 2 |
| src/Game/Map | core | 71 | 9 | 62 | unknown type / identifier: 62 | 62/71 | unknown type / identifier: 6; ambiguous / overload: 2; other: 1 |
| src/Game/MapObj | objects | 333 | 6 | 327 | unknown type / identifier: 326; other: 1 | 298/333 | unknown type / identifier: 22; override / virtual mismatch: 6; other: 6 |
| src/Game/NPC | enemies | 92 | 1 | 91 | unknown type / identifier: 91 | 77/92 | unknown type / identifier: 13; other: 2 |
| src/Game/NWC24 | core | 7 | 1 | 6 | unknown type / identifier: 5; ambiguous / overload: 1 | 6/7 | other: 1 |
| src/Game/NameObj | core | 13 | 8 | 5 | unknown type / identifier: 5 | 12/13 | unknown type / identifier: 1 |
| src/Game/Player | astra | 116 | 10 | 106 | unknown type / identifier: 105; template: 1 | 110/116 | other: 3; unknown type / identifier: 3 |
| src/Game/RhythmLib | enemies | 15 | 0 | 15 | other: 7; unknown type / identifier: 6; pointer to integer (64-bit hazard): 2 | 10/15 | other: 5 |
| src/Game/Ride | objects | 33 | 0 | 33 | unknown type / identifier: 33 | 30/33 | unknown type / identifier: 3 |
| src/Game/Scene | core | 23 | 3 | 20 | unknown type / identifier: 17; other: 3 | 14/23 | other: 5; unknown type / identifier: 2; missing header: 1 |
| src/Game/Screen | enemies | 140 | 3 | 137 | unknown type / identifier: 132; inline asm / asm function: 4; template: 1 | 125/140 | unknown type / identifier: 6; other: 4; missing header: 2 |
| src/Game/Speaker | core | 7 | 3 | 4 | other: 4 | 7/7 | — |
| src/Game/System | core | 164 | 27 | 137 | unknown type / identifier: 99; template: 12; pointer to integer (64-bit hazard): 11 | 129/164 | other: 16; missing header: 6; unknown type / identifier: 6 |
| src/Game/Util | core | 116 | 9 | 107 | unknown type / identifier: 96; other: 4; template: 4 | 80/116 | override / virtual mismatch: 12; other: 11; template: 7 |
| src/JSystem/J2DGraph | m1 | 7 | 3 | 4 | unknown type / identifier: 4 | 6/7 | unknown type / identifier: 1 |
| src/JSystem/J3DGraphAnimator | m1 | 11 | 1 | 10 | unknown type / identifier: 10 | 10/11 | unknown type / identifier: 1 |
| src/JSystem/J3DGraphBase | m1 | 13 | 3 | 10 | unknown type / identifier: 8; other: 1; ambiguous / overload: 1 | 13/13 | — |
| src/JSystem/J3DGraphLoader | m1 | 7 | 0 | 7 | unknown type / identifier: 5; pointer to integer (64-bit hazard): 1; ambiguous / overload: 1 | 6/7 | pointer to integer (64-bit hazard): 1 |
| src/JSystem/JAudio2 | m1 | 72 | 24 | 48 | other: 43; pointer to integer (64-bit hazard): 4; unknown type / identifier: 1 | 46/72 | other: 20; unknown type / identifier: 5; pointer to integer (64-bit hazard): 1 |
| src/JSystem/JGadget | m1 | 2 | 2 | 0 | — | 2/2 | — |
| src/JSystem/JKernel | m1 | 23 | 7 | 16 | ambiguous / overload: 12; pointer to integer (64-bit hazard): 3; other: 1 | 16/23 | pointer to integer (64-bit hazard): 6; other: 1 |
| src/JSystem/JMath | m1 | 3 | 2 | 1 | template: 1 | 3/3 | — |
| src/JSystem/JParticle | m1 | 16 | 3 | 13 | unknown type / identifier: 12; ambiguous / overload: 1 | 14/16 | unknown type / identifier: 1; ambiguous / overload: 1 |
| src/JSystem/JSupport | m1 | 5 | 4 | 1 | pointer to integer (64-bit hazard): 1 | 4/5 | pointer to integer (64-bit hazard): 1 |
| src/JSystem/JUtility | m1 | 11 | 7 | 4 | unknown type / identifier: 2; ambiguous / overload: 2 | 10/12 | unknown type / identifier: 1; other: 1 |
| src/RVL_SDK | m1 | 1 | 0 | 1 | MWCC-only syntax: 1 | 0/1 | MWCC-only syntax: 1 |
| src/libms | m1 | 0 | 0 | 0 | — | 0/0 | — |
| src/nw4r/db | m1 | 3 | 0 | 3 | inline asm / asm function: 2; template: 1 | 1/3 | template: 1; pointer to integer (64-bit hazard): 1 |
| src/nw4r/lyt | m1 | 14 | 3 | 11 | pointer to integer (64-bit hazard): 8; inline asm / asm function: 3 | 13/15 | unknown type / identifier: 2 |
| src/nw4r/math | m1 | 2 | 1 | 1 | inline asm / asm function: 1 | 2/2 | — |
| src/nw4r/ut | m1 | 10 | 7 | 3 | pointer to integer (64-bit hazard): 1; inline asm / asm function: 1; missing header: 1 | 9/10 | missing header: 1 |


## Current shared-header triage

The `TMatrix.hpp:793` undeclared-`get` blocker is absent from current first-error results. Removing a shared blocker exposes additional errors, so root counts must be interpreted as this audit snapshot.

Remaining errors belong to the lane owning the diagnostic file. The libc++ C-linkage errors point back to the extern C block at `libs/RVL_SDK/include/revolution/mem/list.h:5` (m1); the SDK header is the repository work item. The Sandy/SandyHand/SandyMini parser errors are at MWCC-style inline asm blocks (objects). Missing editor/mod headers and declarations are integration tasks; counts do not imply those omitted SMG1 subsystems should all be copied.

## With transferred hooks: ranked root causes (current overlay)

Counts group each file's first error by diagnostic location and normalized message. Later errors are masked; counts do not predict passing files after a fix. Ownership belongs to the diagnostic file, not the affected translation units.

| Rank | Files | Header/source:line | Owning lane | First error | Class |
| ---: | ---: | --- | --- | --- | --- |
| 1 | 76 | libs/JSystem/include/JSystem/JAudio2/JASGadget.hpp:74 | m1 | member initializer 'JASPtrTable' does not name a non-static data member or base class | other |
| 2 | 76 | include/Game/Map/CollisionParts.hpp:22 | core | unknown type name 'KCLFile' | unknown type / identifier |
| 3 | 59 | include/Game/LiveActor/ShadowVolumeDrawer.hpp:22 | core | 'loadModelDrawMtx' marked 'override' but does not override any member functions | override / virtual mismatch |
| 4 | 10 | libs/JSystem/include/JSystem/JAudio2/JASDSPInterface.hpp:106 | m1 | unknown type name 'JASDspAddr' | unknown type / identifier |
| 5 | 3 | libs/JSystem/include/JSystem/JUtility/JUTAssert.hpp:17 | m1 | unknown type name '__va_list'; did you mean 'va_list'? | unknown type / identifier |
| 6 | 3 | /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/c++/v1/__type_traits/enable_if.h:20 | m1 | templates must have C++ linkage | template |
| 7 | 2 | /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/c++/v1/__cstddef/byte.h:57 | m1 | templates must have C++ linkage | template |
| 8 | 2 | libs/libms/include/libms.h:5 | m1 | 'wchar_t.h' file not found | missing header |
| 9 | 1 | src/Game/Animation/XanimeResourceTable.cpp:351 | core | cannot jump from switch statement to this case label | other |
| 10 | 1 | src/Game/AreaObj/AreaObj.cpp:13 | objects | 'petari/editor/runtime_hooks.hpp' file not found | missing header |
| 11 | 1 | src/Game/Boss/Sandy.cpp:34 | objects | expected 'volatile', 'inline', 'goto', or '(' | other |
| 12 | 1 | src/Game/Boss/SandyHand.cpp:27 | objects | expected 'volatile', 'inline', 'goto', or '(' | other |
| 13 | 1 | src/Game/Boss/SandyMini.cpp:21 | objects | expected 'volatile', 'inline', 'goto', or '(' | other |
| 14 | 1 | src/Game/Boss/TwoLegsBulletTU03.cpp:363 | objects | cannot define or redeclare 'exeWait' here because namespace 'NrvTwoLegsReflectCtrl' does not enclose namespace 'TwoLegsReflectCtrl' | other |
| 15 | 1 | src/Game/Boss/TwoLegsBulletTU07.cpp:57 | objects | cannot initialize a parameter of type 's32' (aka 'int') with an rvalue of type 'std::nullptr_t' | other |
| 16 | 1 | src/Game/Boss/TwoLegsBulletTU08.cpp:37 | objects | cannot initialize a parameter of type 's32' (aka 'int') with an rvalue of type 'std::nullptr_t' | other |
| 17 | 1 | src/Game/Camera/CameraAnim.cpp:110 | core | cannot initialize a variable of type 'f32 *' (aka 'float *') with an rvalue of type 'CanmValue *' (aka 'BigEndianValue<float> *') | other |
| 18 | 1 | src/Game/Demo/DemoExecutor.cpp:68 | core | no member named 'mem_func' in namespace 'std' | unknown type / identifier |
| 19 | 1 | src/Game/Demo/DemoStartRequestHolder.cpp:10 | core | out-of-line declaration of 'iterator' does not match any declaration in 'MR::FixedRingBuffer<const DemoStartInfo *, 16>::iterator' | other |
| 20 | 1 | src/Game/Demo/DemoTitleAnim.cpp:17 | core | weak declaration cannot have internal linkage | other |
| 21 | 1 | src/Game/Demo/DemoTitleAnimSub.cpp:13 | core | weak declaration cannot have internal linkage | other |
| 22 | 1 | src/Game/Effect/EffectObj10x10x10SyncClipping.cpp:9 | core | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] | other |
| 23 | 1 | src/Game/Effect/EffectObj20x20x10SyncClipping.cpp:11 | core | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] | other |
| 24 | 1 | src/Game/Effect/EffectObj50x50x10SyncClipping.cpp:9 | core | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] | other |
| 25 | 1 | src/Game/Enemy/FlowerMimic.cpp:21 | enemies | weak declaration cannot have internal linkage | other |
| 26 | 1 | src/Game/Enemy/Nyoropon.cpp:22 | enemies | '__declspec' attributes are not enabled; use '-fdeclspec' or '-fms-extensions' to enable support for __declspec attributes | MWCC-only syntax |
| 27 | 1 | src/Game/Enemy/PatakuriBig.cpp:24 | enemies | weak declaration cannot have internal linkage | other |
| 28 | 1 | src/Game/Enemy/PatakuriBigTU01.cpp:36 | enemies | weak declaration cannot have internal linkage | other |
| 29 | 1 | src/Game/Enemy/Pukupuku.cpp:581 | enemies | cannot initialize a parameter of type 'const HitSensor *' with an rvalue of type 'bool' | other |
| 30 | 1 | src/Game/Enemy/TeresaAtomic.cpp:69 | enemies | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 31 | 1 | src/Game/Gravity/PlanetGravityManager.cpp:33 | core | use of undeclared identifier 'GravityHostID' | unknown type / identifier |
| 32 | 1 | src/Game/LiveActor/DisplayListMaker.cpp:26 | core | no member named 'mem_fun' in namespace 'std' | unknown type / identifier |
| 33 | 1 | src/Game/Map/AwakeControllerHolder.cpp:42 | core | no member named 'mem_func' in namespace 'std' | unknown type / identifier |
| 34 | 1 | src/Game/Map/FileSelector.cpp:905 | core | use of undeclared identifier 'FILE_SELECTOR_MILESTONE' | unknown type / identifier |
| 35 | 1 | src/Game/Map/Flag.cpp:413 | core | no matching function for call to 'GXSetArray' | ambiguous / overload |
| 36 | 1 | src/Game/Map/KCollision.cpp:49 | core | no member named 'KCL' in namespace 'PetariNative' | unknown type / identifier |
| 37 | 1 | src/Game/Map/WaterPlant.cpp:279 | core | no matching constructor for initialization of 'JUTTexture' | ambiguous / overload |
| 38 | 1 | src/Game/MapObj/DrillHole.cpp:10 | objects | functions that differ only in their return type cannot be overloaded | ambiguous / overload |
| 39 | 1 | src/Game/MapObj/ItemDice.cpp:309 | objects | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] | other |
| 40 | 1 | src/Game/MapObj/NeedlePlant.cpp:46 | objects | weak declaration cannot have internal linkage | other |
| 41 | 1 | src/Game/MapObj/PowerStar.cpp:332 | objects | no member named 'PowerStarNrvAppearDemoKoopa' in namespace 'NrvPowerStar'; did you mean 'PowerStarNrvAppearDemoMove'? | unknown type / identifier |
| 42 | 1 | src/Game/MapObj/SuperSpinDriver.cpp:291 | objects | no member named 'publishActor' in namespace 'MR::Native' | unknown type / identifier |
| 43 | 1 | src/Game/MapObj/WoodLogBridge.cpp:294 | objects | taking the address of a temporary object of type 'Color8' [-Waddress-of-temporary] | other |
| 44 | 1 | src/Game/MapObj/YoshiBlock.cpp:120 | objects | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] | other |
| 45 | 1 | src/Game/NPC/NPCActor.cpp:440 | enemies | must explicitly qualify name of member function when taking its address | other |
| 46 | 1 | src/Game/NPC/TalkMessageCtrl.cpp:188 | enemies | use of undeclared identifier 'condition'; did you mean 'std::conditional'? | unknown type / identifier |
| 47 | 1 | src/Game/NPC/TalkState.cpp:229 | enemies | use of undeclared identifier 'petari_ui_observing' | unknown type / identifier |
| 48 | 1 | src/Game/NPC/TicoStateEatCoin.cpp:49 | enemies | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] | other |
| 49 | 1 | src/Game/NWC24/NWC24SendThread.cpp:50 | core | out-of-line definition of 'requestSend' does not match any declaration in 'NWC24SendThread' | other |
| 50 | 1 | src/Game/NameObj/NameObjExecuteHolder.cpp:437 | core | no member named 'mCategoryInfo' in 'NameObjCategoryList' | unknown type / identifier |
| 51 | 1 | src/Game/Player/MarioAnimator.cpp:518 | astra | cannot jump from this goto statement to its label | other |
| 52 | 1 | src/Game/Player/MarioWarp.cpp:391 | astra | cannot jump from switch statement to this case label | other |
| 53 | 1 | src/Game/Player/YoshiLockOnManager.cpp:37 | astra | use of undeclared identifier 'sTargetMax' | unknown type / identifier |
| 54 | 1 | src/Game/Ride/Plant.cpp:426 | objects | no member named 'publishActor' in namespace 'MR::Native' | unknown type / identifier |
| 55 | 1 | src/Game/Ride/PlantSlider.cpp:500 | objects | use of undeclared identifier 'GXWGFifo' | unknown type / identifier |
| 56 | 1 | src/Game/Ride/SphereAccelSensorController.cpp:48 | objects | no member named 'Input' in namespace 'PetariNative' | unknown type / identifier |
| 57 | 1 | src/Game/Scene/IntermissionScene.cpp:38 | core | too few arguments to function call, expected 1, have 0 | other |
| 58 | 1 | src/Game/Scene/PlacementInfoOrdered.cpp:12 | core | 'petari/editor/runtime_hooks.hpp' file not found | missing header |
| 59 | 1 | src/Game/Scene/SceneNameObjListExecutor.cpp:58 | core | constant expression evaluates to -1 which cannot be narrowed to type 'u32' (aka 'unsigned int') [-Wc++11-narrowing] | other |
| 60 | 1 | src/Game/Scene/StageDataArchiveHolder.cpp:21 | core | no template named 'binary_function' in namespace 'std'; did you mean '__binary_function'? | unknown type / identifier |
| 61 | 1 | src/Game/Scene/StageDataHolder.cpp:300 | core | cast from pointer to smaller type 's32' (aka 'int') loses information | pointer to integer (64-bit hazard) |
| 62 | 1 | src/Game/Screen/BloomEffect.cpp:285 | enemies | no matching function for call to 'GXSetArray' | ambiguous / overload |
| 63 | 1 | src/Game/Screen/ButtonPaneController.cpp:146 | enemies | use of undeclared identifier '_24' | unknown type / identifier |
| 64 | 1 | libs/nw4r/include/nw4r/ut/CharStrmReader.h:36 | m1 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 65 | 1 | src/Game/Screen/InformationObserver.cpp:92 | enemies | use of undeclared identifier 'NATIVE_MILESTONE' | unknown type / identifier |
| 66 | 1 | src/Game/Screen/LayoutActor.cpp:44 | enemies | use of undeclared identifier 'petariHudHidden' | unknown type / identifier |
| 67 | 1 | src/Game/Screen/LogoFader.cpp:28 | enemies | type 'float' cannot be narrowed to 'u8' (aka 'unsigned char') in initializer list [-Wc++11-narrowing] | other |
| 68 | 1 | src/Game/Screen/ScenarioSelectLayout.cpp:728 | enemies | no member named 'publishUiPaneTarget' in namespace 'MR::Native' | unknown type / identifier |
| 69 | 1 | src/Game/Screen/StaffRoll.cpp:19 | enemies | 'functional.hpp' file not found | missing header |
| 70 | 1 | src/Game/Screen/StaffRollPart.cpp:12 | enemies | 'functional.hpp' file not found | missing header |
| 71 | 1 | src/Game/Screen/StaffRollUtil.cpp:129 | enemies | use of undeclared identifier 'wtoi'; did you mean 'atoi'? | unknown type / identifier |
| 72 | 1 | src/Game/Screen/SubMeterLayout.cpp:37 | enemies | cannot initialize a parameter of type 'u32' (aka 'unsigned int') with an rvalue of type 'std::nullptr_t' | other |
| 73 | 1 | src/Game/Screen/SysInfoWindow.cpp:92 | enemies | use of undeclared identifier 'NATIVE_TRACE_BOOT' | unknown type / identifier |
| 74 | 1 | src/Game/System/DrawBufferExecuter.cpp:7 | core | 'petari/editor/runtime_hooks.hpp' file not found | missing header |
| 75 | 1 | src/Game/System/DrawSyncManager.cpp:81 | core | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 76 | 1 | src/Game/System/FileHolder.cpp:14 | core | use of undeclared identifier 'mEntryNum' | unknown type / identifier |
| 77 | 1 | src/Game/System/FileRipper.cpp:62 | core | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 78 | 1 | src/Game/System/GameSequenceEnding.cpp:41 | core | weak declaration cannot have internal linkage | other |
| 79 | 1 | src/Game/System/GameSequenceMarioFaceShip.cpp:45 | core | weak declaration cannot have internal linkage | other |
| 80 | 1 | src/Game/System/GameSequenceMarioFaceShipEvent.cpp:88 | core | weak declaration cannot have internal linkage | other |
| 81 | 1 | src/Game/System/GameSequenceSystemStart.cpp:21 | core | weak declaration cannot have internal linkage | other |
| 82 | 1 | src/Game/System/GameSequenceTitle.cpp:29 | core | weak declaration cannot have internal linkage | other |
| 83 | 1 | src/Game/System/MarioFaceShipAttentionConversationDirector.cpp:112 | core | cast from pointer to smaller type 's32' (aka 'int') loses information | pointer to integer (64-bit hazard) |
| 84 | 1 | src/Game/System/NANDManager.cpp:8 | core | 'petari/editor/runtime.hpp' file not found | missing header |
| 85 | 1 | src/Game/System/NANDManagerThread.cpp:4 | core | 'petari/editor/runtime.hpp' file not found | missing header |
| 86 | 1 | src/Game/System/PlayerHeapHolder.cpp:35 | core | no matching member function for call to 'alloc' | ambiguous / overload |
| 87 | 1 | src/Game/System/ResourceHolder.cpp:206 | core | no member named 'KCL' in namespace 'PetariNative' | unknown type / identifier |
| 88 | 1 | src/Game/System/ResourceHolderManager.cpp:109 | core | call to non-static member function without an object argument | other |
| 89 | 1 | src/Game/System/SaveDataHandleSequence.cpp:47 | core | no member named 'SaveDataHandleSequenceSaveWindowDisappear' in the global namespace | unknown type / identifier |
| 90 | 1 | src/Game/System/UserFile.cpp:8 | core | 'petari/editor/runtime.hpp' file not found | missing header |
| 91 | 1 | src/Game/System/WPadButton.cpp:36 | core | use of undeclared identifier 'petari_skip_pad' | unknown type / identifier |
| 92 | 1 | src/Game/System/WorldMapModel.cpp:371 | core | '__declspec' attributes are not enabled; use '-fdeclspec' or '-fms-extensions' to enable support for __declspec attributes | MWCC-only syntax |
| 93 | 1 | src/Game/System/WorldMapResultTU08.cpp:155 | core | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] | other |
| 94 | 1 | src/Game/System/WorldMapResultTU20.cpp:185 | core | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] | other |
| 95 | 1 | src/Game/Util/ActorCameraUtil.cpp:230 | core | non-void function does not return a value [-Werror,-Wreturn-type] | other |
| 96 | 1 | src/Game/Util/ActorMovementUtil.cpp:1663 | core | invalid operands to binary expression ('bool' and 'std::nullptr_t') | other |
| 97 | 1 | src/Game/Util/BattanKingCollisionPress.cpp:17 | core | 'BattanKingCollisionSizeCheck' declared as an array with a negative size | other |
| 98 | 1 | src/Game/Util/CameraUtil.cpp:517 | core | functions that differ only in their return type cannot be overloaded | ambiguous / overload |
| 99 | 1 | src/Game/Util/DemoUtil.cpp:4 | core | explicit instantiation refers to member function 'JGeometry::TVec3<float>::TVec3' that is not an instantiation | template |
| 100 | 1 | src/Game/Util/DiceMover.cpp:7 | core | extraneous template parameter list in template specialization [-Wextraneous-template-head] | template |
| 101 | 1 | src/Game/Util/EventUtil.cpp:523 | core | functions that differ only in their return type cannot be overloaded | ambiguous / overload |
| 102 | 1 | src/Game/Util/FootPrint.cpp:24 | core | explicit instantiation refers to member function 'JGeometry::TVec3<float>::negate' that is not an instantiation | template |
| 103 | 1 | src/Game/Util/JMapInfo.cpp:68 | core | cannot jump from switch statement to this case label | other |
| 104 | 1 | src/Game/Util/MapPartsUtil.cpp:23 | core | explicit instantiation refers to member function 'JGeometry::TVec3<float>::operator-' that is not an instantiation | template |
| 105 | 1 | src/Game/Util/MtxUtil.cpp:768 | core | explicit specialization of 'sqrt<float>' after instantiation | template |
| 106 | 1 | src/Game/Util/NPCUtil.cpp:21 | core | explicit instantiation refers to member function 'JGeometry::TVec3<float>::operator*=' that is not an instantiation | template |
| 107 | 1 | src/Game/Util/SceneUtil.cpp:734 | core | reinterpret_cast from 'void **' to 'const void **' casts away qualifiers | other |
| 108 | 1 | src/JSystem/J2DGraph/J2DPane.cpp:213 | m1 | use of undeclared identifier 'DEG_TO_RAD' | unknown type / identifier |
| 109 | 1 | src/JSystem/J3DGraphAnimator/J3DAnimation.cpp:1266 | m1 | use of undeclared identifier 'petari_generic_world_grey_animation' | unknown type / identifier |
| 110 | 1 | src/JSystem/J3DGraphLoader/J3DAnmLoader.cpp:276 | m1 | cast from pointer to smaller type 's32' (aka 'int') loses information | pointer to integer (64-bit hazard) |
| 111 | 1 | src/JSystem/JAudio2/JAIStream.cpp:58 | m1 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 112 | 1 | src/JSystem/JAudio2/JASResArcLoader.cpp:34 | m1 | out-of-line definition of 'loadResourceAsync' does not match any declaration in namespace 'JASResArcLoader' | other |
| 113 | 1 | src/JSystem/JAudio2/JAUSeqDataBlockMgr.cpp:136 | m1 | cannot initialize a parameter of type 'void (*)(u32, u32)' (aka 'void (*)(unsigned int, unsigned int)') with an lvalue of type 'void (u32, uintptr_t)' (aka 'void (unsigned int, unsigned long)'): type mismatch at 2nd parameter ('u32' (aka 'unsigned int') vs 'uintptr_t' (aka 'unsigned long')) | other |
| 114 | 1 | libs/JSystem/include/JSystem/JAudio2/JAUSoundAnimator.hpp:94 | m1 | no viable conversion from returned value of type 'const JSUBigEndian<u32>' (aka 'const JSUBigEndian<unsigned int>') to function return type 'JAISoundID' | other |
| 115 | 1 | src/JSystem/JKernel/JKRAramArchive.cpp:266 | m1 | cast from pointer to smaller type 's32' (aka 'int') loses information | pointer to integer (64-bit hazard) |
| 116 | 1 | src/JSystem/JKernel/JKRAramPiece.cpp:89 | m1 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 117 | 1 | src/JSystem/JKernel/JKRDecomp.cpp:204 | m1 | cast from pointer to smaller type 's32' (aka 'int') loses information | pointer to integer (64-bit hazard) |
| 118 | 1 | src/JSystem/JKernel/JKRExpHeap.cpp:849 | m1 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 119 | 1 | src/JSystem/JKernel/JKRHeap.cpp:78 | m1 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 120 | 1 | src/JSystem/JKernel/JKRMemArchive.cpp:273 | m1 | cannot jump from switch statement to this case label | other |
| 121 | 1 | src/JSystem/JKernel/JKRUnitHeap.cpp:51 | m1 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 122 | 1 | src/JSystem/JParticle/JPAMath.cpp:13 | m1 | no template named 'pair' in namespace 'JMath' | unknown type / identifier |
| 123 | 1 | src/JSystem/JParticle/JPAResource.cpp:918 | m1 | no matching function for call to 'GXSetArray' | ambiguous / overload |
| 124 | 1 | src/JSystem/JSupport/JSUMemoryStream.cpp:17 | m1 | cast from pointer to smaller type 'int' loses information | pointer to integer (64-bit hazard) |
| 125 | 1 | src/JSystem/JUtility/JUTException.cpp:192 | m1 | too few arguments to function call, expected 1, have 0 | other |
| 126 | 1 | src/nw4r/db/db_assert.cpp:55 | m1 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 127 | 1 | src/nw4r/lyt/lyt_animation.cpp:221 | m1 | use of undeclared identifier 'petari_generic_world_layout_texture' | unknown type / identifier |
| 128 | 1 | src/nw4r/lyt/lyt_material.cpp:285 | m1 | use of undeclared identifier 'petari_generic_world_layout_texture' | unknown type / identifier |
| 129 | 1 | src/nw4r/ut/ut_TextWriterBase.cpp:2 | m1 | 'wchar_io.h' file not found | missing header |
| 130 | 1 | src/RVL_SDK/os/init/__ppc_eabi_init.cpp:11 | m1 | '__declspec' attributes are not enabled; use '-fdeclspec' or '-fms-extensions' to enable support for __declspec attributes | MWCC-only syntax |

## Top 20 distinct first-error messages

| Rank | Files | Normalized message |
| ---: | ---: | --- |
| 1 | 76 | member initializer 'JASPtrTable' does not name a non-static data member or base class |
| 2 | 76 | unknown type name 'KCLFile' |
| 3 | 59 | 'loadModelDrawMtx' marked 'override' but does not override any member functions |
| 4 | 11 | weak declaration cannot have internal linkage |
| 5 | 10 | unknown type name 'JASDspAddr' |
| 6 | 10 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information |
| 7 | 8 | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] |
| 8 | 5 | cast from pointer to smaller type 's32' (aka 'int') loses information |
| 9 | 5 | templates must have C++ linkage |
| 10 | 4 | cannot jump from switch statement to this case label |
| 11 | 3 | 'petari/editor/runtime_hooks.hpp' file not found |
| 12 | 3 | expected 'volatile', 'inline', 'goto', or '(' |
| 13 | 3 | '__declspec' attributes are not enabled; use '-fdeclspec' or '-fms-extensions' to enable support for __declspec attributes |
| 14 | 3 | no matching function for call to 'GXSetArray' |
| 15 | 3 | functions that differ only in their return type cannot be overloaded |
| 16 | 3 | unknown type name '__va_list'; did you mean 'va_list'? |
| 17 | 3 | 'petari/editor/runtime.hpp' file not found |
| 18 | 2 | cannot initialize a parameter of type 's32' (aka 'int') with an rvalue of type 'std::nullptr_t' |
| 19 | 2 | no member named 'mem_func' in namespace 'std' |
| 20 | 2 | no member named 'KCL' in namespace 'PetariNative' |

## Original baseline: ranked root causes

Counts group each file's first error by diagnostic location and normalized message. Later errors are masked; these counts do not establish how many files a fix will make pass.

| Rank | Files | Header/source:line | First error | Class |
| ---: | ---: | --- | --- | --- |
| 1 | 1638 | libs/JSystem/include/JSystem/JGeometry/TMatrix.hpp:793 | use of undeclared identifier 'get' | unknown type / identifier |
| 2 | 83 | libs/JSystem/include/JSystem/JAudio2/JASHeapCtrl.hpp:296 | 'operator new' takes type size_t ('unsigned long') as 1st parameter | other |
| 3 | 26 | libs/JSystem/include/JSystem/JKernel/JKRDvdRipper.hpp:53 | no matching function for call to 'loadToMainRAM' | ambiguous / overload |
| 4 | 18 | libs/RVL_SDK/include/revolution/mem/heapCommon.h:37 | cast from pointer to smaller type 'UIntPtr' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 5 | 16 | /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/c++/v1/__type_traits/enable_if.h:20 | templates must have C++ linkage | template |
| 6 | 12 | libs/nw4r/include/nw4r/math/arithmetic.h:10 | invalid output constraint '=f' in asm | inline asm / asm function |
| 7 | 12 | libs/JSystem/include/JSystem/JAudio2/JASGadget.hpp:74 | member initializer 'JASPtrTable' does not name a non-static data member or base class | other |
| 8 | 5 | libs/JSystem/include/JSystem/JAudio2/JASSeqReader.hpp:35 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 9 | 3 | /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/c++/v1/__cstddef/byte.h:57 | templates must have C++ linkage | template |
| 10 | 2 | libs/JSystem/include/JSystem/J3DGraphBase/J3DDrawBuffer.hpp:34 | non-void function does not return a value [-Werror,-Wreturn-type] | other |
| 11 | 2 | libs/JSystem/include/JSystem/JUtility/JUTAssert.hpp:17 | unknown type name '__va_list'; did you mean 'va_list'? | unknown type / identifier |
| 12 | 1 | src/Game/Animation/XanimeResourceTable.cpp:351 | cannot jump from switch statement to this case label | other |
| 13 | 1 | src/Game/AudioLib/AudEffector.cpp:34 | no matching function for call to 'operator new[]' | ambiguous / overload |
| 14 | 1 | src/Game/Demo/DemoStartRequestHolder.cpp:193 | explicit specialization of 'push_back' after instantiation | template |
| 15 | 1 | src/Game/Effect/EffectObj10x10x10SyncClipping.cpp:9 | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] | other |
| 16 | 1 | src/Game/Effect/EffectObj20x20x10SyncClipping.cpp:11 | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] | other |
| 17 | 1 | src/Game/Effect/EffectObj50x50x10SyncClipping.cpp:9 | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] | other |
| 18 | 1 | include/Game/Util/Functor.hpp:27 | no matching function for call to 'operator new' | ambiguous / overload |
| 19 | 1 | src/Game/Map/AwakeControllerHolder.cpp:42 | no member named 'mem_func' in namespace 'std' | unknown type / identifier |
| 20 | 1 | src/Game/NWC24/NWC24System.cpp:22 | no matching function for call to 'operator new[]' | ambiguous / overload |
| 21 | 1 | src/Game/System/DrawBufferExecuter.cpp:47 | no member named 'mem_func' in namespace 'std' | unknown type / identifier |
| 22 | 1 | src/Game/System/DrawSyncManager.cpp:81 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 23 | 1 | libs/libms/include/libms.h:5 | 'wchar_t.h' file not found | missing header |
| 24 | 1 | src/Game/System/OSThreadWrapper.cpp:29 | no matching function for call to 'operator new[]' | ambiguous / overload |
| 25 | 1 | src/Game/Util/DemoUtil.cpp:4 | explicit instantiation refers to member function 'JGeometry::TVec3<float>::TVec3' that is not an instantiation | template |
| 26 | 1 | src/Game/Util/JMapInfo.cpp:64 | cannot jump from switch statement to this case label | other |
| 27 | 1 | src/JSystem/J2DGraph/J2DManage.cpp:6 | use of undeclared identifier 'strcmp' | unknown type / identifier |
| 28 | 1 | src/JSystem/J2DGraph/J2DPane.cpp:213 | use of undeclared identifier 'DEG_TO_RAD' | unknown type / identifier |
| 29 | 1 | src/JSystem/J3DGraphBase/J3DShapeDraw.cpp:27 | no matching function for call to 'operator new[]' | ambiguous / overload |
| 30 | 1 | src/JSystem/J3DGraphLoader/J3DAnmLoader.cpp:230 | cast from pointer to smaller type 's32' (aka 'int') loses information | pointer to integer (64-bit hazard) |
| 31 | 1 | src/JSystem/J3DGraphLoader/J3DShapeFactory.cpp:102 | no matching function for call to 'operator new[]' | ambiguous / overload |
| 32 | 1 | src/JSystem/JAudio2/JASCalc.cpp:71 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 33 | 1 | src/JSystem/JAudio2/JAUSeqDataBlockMgr.cpp:136 | cannot initialize a parameter of type 'void (*)(u32, u32)' (aka 'void (*)(unsigned int, unsigned int)') with an lvalue of type 'void (u32, uintptr_t)' (aka 'void (unsigned int, unsigned long)'): type mismatch at 2nd parameter ('u32' (aka 'unsigned int') vs 'uintptr_t' (aka 'unsigned long')) | other |
| 34 | 1 | src/JSystem/JKernel/JKRAramBlock.cpp:30 | no matching function for call to 'operator new' | ambiguous / overload |
| 35 | 1 | src/JSystem/JKernel/JKRAramHeap.cpp:15 | no matching function for call to 'operator new' | ambiguous / overload |
| 36 | 1 | src/JSystem/JKernel/JKRAramStream.cpp:13 | no matching function for call to 'operator new' | ambiguous / overload |
| 37 | 1 | src/JSystem/JKernel/JKRArchivePub.cpp:10 | out-of-line definition of 'check_mount_already' does not match any declaration in 'JKRArchive' | other |
| 38 | 1 | src/JSystem/JKernel/JKRDecomp.cpp:12 | no matching function for call to 'operator new' | ambiguous / overload |
| 39 | 1 | src/JSystem/JKernel/JKRExpHeap.cpp:86 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 40 | 1 | src/JSystem/JKernel/JKRHeap.cpp:78 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 41 | 1 | src/JSystem/JKernel/JKRUnitHeap.cpp:41 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 42 | 1 | src/JSystem/JMath/JMATrigonometric.cpp:6 | explicit specialization of 'atan2_' after instantiation | template |
| 43 | 1 | src/JSystem/JParticle/JPAResourceLoader.cpp:31 | no matching function for call to 'operator new[]' | ambiguous / overload |
| 44 | 1 | src/JSystem/JSupport/JSUMemoryStream.cpp:16 | cast from pointer to smaller type 'int' loses information | pointer to integer (64-bit hazard) |
| 45 | 1 | src/JSystem/JUtility/JUTException.cpp:81 | no matching function for call to 'operator new' | ambiguous / overload |
| 46 | 1 | src/JSystem/JUtility/JUTTexture.cpp:9 | no matching function for call to 'operator new[]' | ambiguous / overload |
| 47 | 1 | src/nw4r/lyt/lyt_texMap.cpp:73 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 48 | 1 | src/nw4r/ut/ut_ResFont.cpp:9 | cast from pointer to smaller type 's32' (aka 'int') loses information | pointer to integer (64-bit hazard) |
| 49 | 1 | src/nw4r/ut/ut_TextWriterBase.cpp:2 | 'wchar_io.h' file not found | missing header |
| 50 | 1 | src/RVL_SDK/os/init/__ppc_eabi_init.cpp:11 | '__declspec' attributes are not enabled; use '-fdeclspec' or '-fms-extensions' to enable support for __declspec attributes | MWCC-only syntax |

## Original baseline: top 20 first-error messages

| Rank | Files | Normalized message |
| ---: | ---: | --- |
| 1 | 1638 | use of undeclared identifier 'get' |
| 2 | 83 | 'operator new' takes type size_t ('unsigned long') as 1st parameter |
| 3 | 26 | no matching function for call to 'loadToMainRAM' |
| 4 | 19 | templates must have C++ linkage |
| 5 | 18 | cast from pointer to smaller type 'UIntPtr' (aka 'unsigned int') loses information |
| 6 | 12 | invalid output constraint '=f' in asm |
| 7 | 12 | member initializer 'JASPtrTable' does not name a non-static data member or base class |
| 8 | 11 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information |
| 9 | 7 | no matching function for call to 'operator new[]' |
| 10 | 6 | no matching function for call to 'operator new' |
| 11 | 3 | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] |
| 12 | 2 | cannot jump from switch statement to this case label |
| 13 | 2 | no member named 'mem_func' in namespace 'std' |
| 14 | 2 | non-void function does not return a value [-Werror,-Wreturn-type] |
| 15 | 2 | unknown type name '__va_list'; did you mean 'va_list'? |
| 16 | 2 | cast from pointer to smaller type 's32' (aka 'int') loses information |
| 17 | 1 | explicit specialization of 'push_back' after instantiation |
| 18 | 1 | 'wchar_t.h' file not found |
| 19 | 1 | explicit instantiation refers to member function 'JGeometry::TVec3<float>::TVec3' that is not an instantiation |
| 20 | 1 | use of undeclared identifier 'strcmp' |

## Report validation

Checked totals, passing flags, unique source paths, directory sums, first-error
extraction, all ranked root counts, lane mapping, and preservation of the original
baseline are verified from raw JSON. The first-error counts sum to the failing
translation-unit count. No source fixes, Wii build, linking, or runtime validation
were performed for this rerun.
