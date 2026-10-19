# Hook transfer report

MacioGalaxy e5bc761c0..HEAD onto .. Macro mapping: PETARI_NATIVE -> GALAXY_NATIVE.

Hook groups: clean 379, adapted 508, relocated 8, excluded 46, manual 284, unconditional 191, feature 149, missing 255, comment 14, applied-guarded 496, applied-neutral 399

Files: applied 293, excluded 9, missing 65, partial 56, relocated 6, unchanged 109

- clean/adapted: applied (Wii side verified unchanged with unifdef -UGALAXY_NATIVE -D__MWERKS__).
  `applied-guarded` hooks change nothing on the Wii side; `applied-neutral` ones rewrite Wii code into a form that expands to the same tokens (JSU_BE(T) -> T, `this->`, u32 -> uintptr_t, ...); the DOL hash check at landing is the final proof.
- excluded: would apply, but the file failed the MWCC compile gate (tools/team/hookgate.py); port by hand.
- relocated: hook from a Petari file that SMG2 merged into another TU, applied by unique context match.
- manual: guarded hook that conflicts with SMG2 changes; port by hand (manual/<path>.diff).
- unconditional: MacioGalaxy changed Wii-visible code (bug fix or clang syntax fix); not applied; port natively behind GALAXY_NATIVE where clang needs it.
- feature: hook for a deferred MacioGalaxy feature (editor, test server, mods); P3.
- missing: no SMG2 file at that path or with that unique basename.

| dir | clean | adapted | relocated | excluded | manual | unconditional | feature | missing | comment |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| include/Game/Animation | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 |
| include/Game/AudioLib | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| include/Game/Boss | 1 | 1 | 0 | 0 | 0 | 1 | 0 | 0 | 0 |
| include/Game/Camera | 1 | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| include/Game/Effect | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| include/Game/Gravity | 0 | 1 | 0 | 0 | 1 | 1 | 0 | 0 | 0 |
| include/Game/LiveActor | 0 | 3 | 0 | 0 | 0 | 1 | 0 | 0 | 0 |
| include/Game/Map | 0 | 3 | 0 | 0 | 0 | 1 | 0 | 0 | 0 |
| include/Game/MapObj | 0 | 1 | 0 | 0 | 3 | 0 | 0 | 0 | 0 |
| include/Game/NWC24 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| include/Game/NameObj | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| include/Game/Player | 0 | 7 | 0 | 0 | 5 | 0 | 0 | 0 | 0 |
| include/Game/RhythmLib | 2 | 5 | 0 | 0 | 1 | 0 | 0 | 0 | 1 |
| include/Game/Scene | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 |
| include/Game/Screen | 0 | 4 | 0 | 0 | 1 | 0 | 0 | 0 | 1 |
| include/Game/Speaker | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| include/Game/System | 1 | 9 | 0 | 0 | 4 | 1 | 3 | 0 | 0 |
| include/Game/Util | 3 | 11 | 0 | 0 | 7 | 0 | 0 | 0 | 0 |
| include/Inline.hpp | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| include/math_types.hpp | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| libs/JSystem/include/Inline.hpp | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| libs/JSystem/include/JSystem | 143 | 56 | 0 | 1 | 18 | 13 | 0 | 0 | 8 |
| libs/RVLFaceLib/include/RFL_DataUtility.h | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| libs/RVLFaceLib/include/RFL_MiddleDatabase.h | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| libs/RVLFaceLib/include/RFL_Model.h | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| libs/RVLFaceLib/include/RFL_Types.h | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| libs/RVLFaceLib/include/RFLi_Model.h | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 |
| libs/RVLFaceLib/include/RFLi_Texture.h | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| libs/RVL_SDK/include/revolution | 48 | 12 | 0 | 0 | 5 | 10 | 0 | 0 | 0 |
| libs/nw4r/include/nw4r | 6 | 4 | 0 | 0 | 1 | 1 | 0 | 0 | 0 |
| src/Game/Animation | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 4 | 0 |
| src/Game/AreaObj | 0 | 1 | 0 | 0 | 0 | 0 | 2 | 3 | 0 |
| src/Game/AudioLib | 0 | 5 | 0 | 1 | 4 | 5 | 4 | 1 | 0 |
| src/Game/Boss | 0 | 5 | 0 | 0 | 5 | 1 | 0 | 3 | 0 |
| src/Game/Camera | 2 | 8 | 0 | 0 | 3 | 2 | 5 | 0 | 0 |
| src/Game/Demo | 0 | 2 | 0 | 0 | 2 | 0 | 2 | 5 | 0 |
| src/Game/Effect | 0 | 5 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| src/Game/Enemy | 0 | 10 | 0 | 0 | 5 | 0 | 0 | 4 | 0 |
| src/Game/GameAudio | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 27 | 0 |
| src/Game/Gravity | 0 | 0 | 0 | 2 | 1 | 0 | 0 | 0 | 0 |
| src/Game/LiveActor | 0 | 5 | 0 | 0 | 5 | 0 | 0 | 0 | 0 |
| src/Game/Map | 0 | 12 | 0 | 5 | 13 | 7 | 0 | 8 | 0 |
| src/Game/MapObj | 0 | 17 | 0 | 0 | 11 | 0 | 8 | 10 | 0 |
| src/Game/NPC | 0 | 8 | 1 | 0 | 8 | 3 | 2 | 3 | 0 |
| src/Game/NWC24 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| src/Game/NameObj | 0 | 3 | 0 | 0 | 0 | 0 | 2 | 0 | 0 |
| src/Game/Player | 0 | 2 | 4 | 0 | 16 | 14 | 16 | 58 | 3 |
| src/Game/RhythmLib | 0 | 5 | 0 | 0 | 1 | 0 | 0 | 0 | 0 |
| src/Game/Ride | 0 | 4 | 0 | 0 | 2 | 0 | 0 | 6 | 0 |
| src/Game/Scene | 0 | 4 | 0 | 0 | 14 | 5 | 24 | 0 | 0 |
| src/Game/Screen | 0 | 37 | 1 | 25 | 11 | 10 | 6 | 81 | 0 |
| src/Game/Speaker | 2 | 3 | 0 | 0 | 2 | 0 | 0 | 0 | 0 |
| src/Game/System | 1 | 56 | 1 | 9 | 20 | 10 | 47 | 34 | 0 |
| src/Game/Util | 5 | 51 | 1 | 0 | 98 | 4 | 5 | 5 | 1 |
| src/JSystem/J2DGraph | 1 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 |
| src/JSystem/J3DGraphAnimator | 1 | 0 | 0 | 0 | 1 | 9 | 3 | 0 | 0 |
| src/JSystem/J3DGraphBase | 20 | 1 | 0 | 0 | 0 | 4 | 0 | 0 | 0 |
| src/JSystem/J3DGraphLoader | 7 | 1 | 0 | 0 | 0 | 4 | 0 | 0 | 0 |
| src/JSystem/JAudio2 | 13 | 33 | 0 | 3 | 0 | 35 | 0 | 0 | 0 |
| src/JSystem/JKernel | 74 | 22 | 0 | 0 | 2 | 20 | 0 | 0 | 0 |
| src/JSystem/JMath | 1 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 0 |
| src/JSystem/JParticle | 5 | 9 | 0 | 0 | 0 | 2 | 0 | 0 | 0 |
| src/JSystem/JSupport | 1 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 |
| src/JSystem/JUtility | 0 | 3 | 0 | 0 | 0 | 7 | 0 | 0 | 0 |
| src/RVLFaceLib/RFL_Controller.c | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| src/RVLFaceLib/RFL_DataUtility.c | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| src/RVLFaceLib/RFL_Database.c | 0 | 19 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| src/RVLFaceLib/RFL_DefaultDatabase.c | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| src/RVLFaceLib/RFL_Format.c | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| src/RVLFaceLib/RFL_HiddenDatabase.c | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| src/RVLFaceLib/RFL_MakeRandomFace.c | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| src/RVLFaceLib/RFL_MakeTex.c | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| src/RVLFaceLib/RFL_MiddleDatabase.c | 0 | 8 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| src/RVLFaceLib/RFL_Model.c | 0 | 6 | 0 | 0 | 1 | 15 | 0 | 0 | 0 |
| src/RVLFaceLib/RFL_NANDAccess.c | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| src/RVLFaceLib/RFL_NANDLoader.c | 0 | 5 | 0 | 0 | 1 | 0 | 0 | 0 | 0 |
| src/RVLFaceLib/RFL_System.c | 0 | 9 | 0 | 0 | 1 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/arc | 8 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/mem | 1 | 5 | 0 | 0 | 2 | 2 | 0 | 0 | 0 |
| src/RVL_SDK/mtx | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/os | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| src/nw4r/db | 0 | 3 | 0 | 0 | 2 | 0 | 0 | 0 | 0 |
| src/nw4r/lyt | 2 | 2 | 0 | 0 | 0 | 0 | 12 | 1 | 0 |
| src/nw4r/ut | 0 | 5 | 0 | 0 | 0 | 0 | 7 | 0 | 0 |

## Added files

| file | action |
|---|---|
| include/Game/Camera/EditorAreas.hpp | deferred-feature |
| include/Game/Camera/EditorCamera.hpp | deferred-feature |
| include/Game/Camera/EditorCameraPreview.hpp | deferred-feature |
| include/Game/Camera/EditorDraw.hpp | deferred-feature |
| include/Game/Camera/EditorGizmo.hpp | deferred-feature |
| include/Game/Camera/EditorGravity.hpp | deferred-feature |
| include/Game/Camera/EditorPaths.hpp | deferred-feature |
| include/Game/Camera/EditorPicking.hpp | deferred-feature |
| include/Game/Camera/EditorPlaytest.hpp | deferred-feature |
| include/Game/Camera/EditorThumbs.hpp | deferred-feature |
| include/Game/Camera/PhotoCamera.hpp | deferred-feature |
| include/Game/Editor/EditorGalaxyTest.hpp | deferred-feature |
| include/Game/Editor/EditorScene.hpp | deferred-feature |
| include/Game/Editor/EditorScratch.hpp | deferred-feature |
| include/Game/Editor/EditorTerrain.hpp | deferred-feature |
| include/Game/Player/GenericFigure.hpp | deferred-feature |
| include/Game/Player/MarioCappy.hpp | deferred-feature |
| include/Game/Scene/ClassSweep.hpp | deferred-feature |
| include/Game/Scene/TestServer.hpp | deferred-feature |
| include/Game/Scene/TestServerEditor.hpp | deferred-feature |
| include/Game/Scene/TestServerInput.hpp | deferred-feature |
| include/Game/Scene/TestServerReplay.hpp | deferred-feature |
| include/Game/System/CutsceneSkip.hpp | deferred-feature |
| include/Game/System/GenericWorld.hpp | deferred-feature |
| include/Game/System/NativeBootTrace.hpp | copy |
| include/Game/System/NativeUnlockedSave.hpp | deferred-feature |
| include/Game/System/PointMove.hpp | deferred-feature |
| include/Game/Util/BigEndian.hpp | copy |
| include/Game/Util/GenericShapes.hpp | deferred-feature |
| include/Game/Util/NativeActorObserve.hpp | copy |
| include/Game/Util/NativeOverload.hpp | copy |
| include/Game/Util/NativeUiObserve.hpp | copy |
| libs/JSystem/include/JSystem/JSupport/JSUBigEndian.hpp | copy |
| src/Game/Camera/EditorAreas.cpp | deferred-feature |
| src/Game/Camera/EditorCamera.cpp | deferred-feature |
| src/Game/Camera/EditorCameraPreview.cpp | deferred-feature |
| src/Game/Camera/EditorDebugDraw.cpp | deferred-feature |
| src/Game/Camera/EditorDraw.cpp | deferred-feature |
| src/Game/Camera/EditorGizmo.cpp | deferred-feature |
| src/Game/Camera/EditorGravity.cpp | deferred-feature |
| src/Game/Camera/EditorGravityTest.cpp | deferred-feature |
| src/Game/Camera/EditorPaths.cpp | deferred-feature |
| src/Game/Camera/EditorPicking.cpp | deferred-feature |
| src/Game/Camera/EditorPlaytest.cpp | deferred-feature |
| src/Game/Camera/EditorPlaytestTest.cpp | deferred-feature |
| src/Game/Camera/EditorThumbs.cpp | deferred-feature |
| src/Game/Camera/OdysseyCamera.cpp | deferred-feature |
| src/Game/Camera/PhotoCamera.cpp | deferred-feature |
| src/Game/Editor/EditorGalaxyTest.cpp | deferred-feature |
| src/Game/Editor/EditorHooksTest.cpp | deferred-feature |
| src/Game/Editor/EditorReload.cpp | deferred-feature |
| src/Game/Editor/EditorScene.cpp | deferred-feature |
| src/Game/Editor/EditorScratch.cpp | deferred-feature |
| src/Game/Editor/EditorTerrain.cpp | deferred-feature |
| src/Game/Editor/EditorTerrainTest.cpp | deferred-feature |
| src/Game/Player/GenericFigure.cpp | deferred-feature |
| src/Game/Player/MarioCappy.cpp | deferred-feature |
| src/Game/Scene/ClassSweep.cpp | deferred-feature |
| src/Game/Scene/TestServer.cpp | deferred-feature |
| src/Game/Scene/TestServerEditor.cpp | deferred-feature |
| src/Game/Scene/TestServerInput.cpp | deferred-feature |
| src/Game/Scene/TestServerReplay.cpp | deferred-feature |
| src/Game/Screen/MessageEditorMessageTagNative.cpp | deferred-feature |
| src/Game/System/BCSVInclude_native.S | copy |
| src/Game/System/CutsceneSkip.cpp | deferred-feature |
| src/Game/System/GenericWorld.cpp | deferred-feature |
| src/Game/System/NativeUnlockedSave.cpp | deferred-feature |
| src/Game/System/PointMove.cpp | deferred-feature |
| src/Game/Util/GenericShapes.cpp | deferred-feature |
| src/JSystem/JUtility/JUTNativeResource.cpp | copy |
| src/nw4r/lyt/lyt_nativeResource.cpp | copy |

## Files with manual work (manual + unconditional + missing groups)

- include/Game/Animation/XanimeCore.hpp: manual 1, unconditional 0
- include/Game/Boss/TripodBossAccesser.hpp: manual 0, unconditional 1
- include/Game/Gravity/PlanetGravityManager.hpp: manual 1, unconditional 1
- include/Game/LiveActor/ShadowVolumeDrawer.hpp: manual 0, unconditional 1
- include/Game/Map/CollisionParts.hpp: manual 0, unconditional 1
- include/Game/MapObj/MorphItemObjNeo.hpp: manual 3, unconditional 0
- include/Game/Player/J3DModelX.hpp: manual 1, unconditional 0
- include/Game/Player/Mario.hpp: manual 3, unconditional 0
- include/Game/Player/PlayerEvent.hpp: manual 1, unconditional 0
- include/Game/RhythmLib/AudMePlayer.hpp: manual 1, unconditional 0
- include/Game/Scene/StageDataHolder.hpp: manual 1, unconditional 0
- include/Game/Screen/THPSimplePlayerWrapper.hpp: manual 1, unconditional 0
- include/Game/System/GameDataFunction.hpp: manual 1, unconditional 1
- include/Game/System/MessageHolder.hpp: manual 2, unconditional 0
- include/Game/System/ScenarioDataParser.hpp: manual 1, unconditional 0
- include/Game/Util/HashUtil.hpp: manual 4, unconditional 0
- include/Game/Util/MathUtil.hpp: manual 1, unconditional 0
- include/Game/Util/PlayerUtil.hpp: manual 2, unconditional 0
- libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp: manual 0, unconditional 1
- libs/JSystem/include/JSystem/JAudio2/JASDspInterface.hpp: manual 0, unconditional 1
- libs/JSystem/include/JSystem/JAudio2/JASGadget.hpp: manual 0, unconditional 1
- libs/JSystem/include/JSystem/JAudio2/JASResArcLoader.hpp: manual 0, unconditional 1
- libs/JSystem/include/JSystem/JAudio2/JAUSoundAnimator.hpp: manual 0, unconditional 1
- libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp: manual 0, unconditional 2
- libs/JSystem/include/JSystem/JAudio2/osdsp_task.hpp: manual 0, unconditional 2
- libs/JSystem/include/JSystem/JGeometry/TUtil.hpp: manual 1, unconditional 0
- libs/JSystem/include/JSystem/JGeometry/TVec.hpp: manual 10, unconditional 1
- libs/JSystem/include/JSystem/JKernel/JKRCompArchive.hpp: manual 0, unconditional 1
- libs/JSystem/include/JSystem/JMath/JMATrigonometric.hpp: manual 1, unconditional 0
- libs/JSystem/include/JSystem/JMath/JMath.hpp: manual 6, unconditional 0
- libs/JSystem/include/JSystem/JParticle/JPAEmitter.hpp: manual 0, unconditional 1
- libs/JSystem/include/JSystem/JUtility/JUTAssert.hpp: manual 0, unconditional 1
- libs/RVLFaceLib/include/RFLi_Model.h: manual 2, unconditional 0
- libs/RVL_SDK/include/revolution/ai.h: manual 0, unconditional 1
- libs/RVL_SDK/include/revolution/arc.h: manual 0, unconditional 1
- libs/RVL_SDK/include/revolution/dvd.h: manual 0, unconditional 1
- libs/RVL_SDK/include/revolution/gx/GXGet.h: manual 0, unconditional 1
- libs/RVL_SDK/include/revolution/gx/GXVert.h: manual 0, unconditional 1
- libs/RVL_SDK/include/revolution/mem/list.h: manual 0, unconditional 2
- libs/RVL_SDK/include/revolution/mtx.h: manual 0, unconditional 1
- libs/RVL_SDK/include/revolution/nand.h: manual 0, unconditional 1
- libs/RVL_SDK/include/revolution/os.h: manual 4, unconditional 0
- libs/RVL_SDK/include/revolution/os/OSTime.h: manual 1, unconditional 0
- libs/RVL_SDK/include/revolution/wpad.h: manual 0, unconditional 1
- libs/nw4r/include/nw4r/ut/Color.h: manual 0, unconditional 1
- libs/nw4r/include/nw4r/ut/LinkList.h: manual 1, unconditional 0
- src/Game/Animation/XanimeResource.cpp: missing in SMG2 (4 groups)
- src/Game/AreaObj/ChangeBgmCube.cpp: missing in SMG2 (3 groups)
- src/Game/AudioLib/AudBgmSetting.cpp: manual 0, unconditional 1
- src/Game/AudioLib/AudParams.cpp: missing in SMG2 (1 groups)
- src/Game/AudioLib/AudSoundNameConverter.cpp: manual 3, unconditional 0
- src/Game/AudioLib/AudSoundObject.cpp: manual 0, unconditional 2
- src/Game/AudioLib/AudSystem.cpp: manual 1, unconditional 2
- src/Game/Boss/BossKameckStateBattle.cpp: missing in SMG2 (1 groups)
- src/Game/Boss/DinoPackunBattleEggVs2.cpp: manual 0, unconditional 1
- src/Game/Boss/DinoPackunVs1.cpp: manual 1, unconditional 0
- src/Game/Boss/Dodoryu.cpp: manual 4, unconditional 0
- src/Game/Boss/TripodBossAccesser.cpp: missing in SMG2 (1 groups)
- src/Game/Boss/TripodBossKillerGeneraterCircle.cpp: missing in SMG2 (1 groups)
- src/Game/Camera/CamTranslatorAnim.cpp: manual 1, unconditional 0
- src/Game/Camera/CamTranslatorSpiral.cpp: manual 1, unconditional 0
- src/Game/Camera/CameraAnim.cpp: manual 0, unconditional 2
- src/Game/Camera/CameraManGame.cpp: manual 1, unconditional 0
- src/Game/Demo/DemoDirector.cpp: manual 1, unconditional 0
- src/Game/Demo/DemoExecutor.cpp: manual 1, unconditional 0
- src/Game/Demo/PrologueDirector.cpp: missing in SMG2 (5 groups)
- src/Game/Effect/EffectObjGravityDust.cpp: missing in SMG2 (1 groups)
- src/Game/Enemy/BallBeamer.cpp: missing in SMG2 (2 groups)
- src/Game/Enemy/CannonShellBase.cpp: missing in SMG2 (1 groups)
- src/Game/Enemy/Karikari.cpp: manual 1, unconditional 0
- src/Game/Enemy/KoopaJrShip.cpp: missing in SMG2 (1 groups)
- src/Game/Enemy/OtaRock.cpp: manual 1, unconditional 0
- src/Game/Enemy/PackunPetit.cpp: manual 1, unconditional 0
- src/Game/Enemy/Takobo.cpp: manual 2, unconditional 0
- src/Game/GameAudio/AudStageBgmTable.cpp: missing in SMG2 (21 groups)
- src/Game/GameAudio/AudStageBgmWrap.cpp: missing in SMG2 (6 groups)
- src/Game/Gravity/PlanetGravityManager.cpp: manual 1, unconditional 0
- src/Game/LiveActor/DisplayListMaker.cpp: manual 1, unconditional 0
- src/Game/LiveActor/EffectKeeper.cpp: manual 3, unconditional 0
- src/Game/LiveActor/LiveActorGroupArray.cpp: manual 1, unconditional 0
- src/Game/Map/FileSelectItem.cpp: manual 1, unconditional 1
- src/Game/Map/FileSelector.cpp: manual 8, unconditional 2
- src/Game/Map/Flag.cpp: manual 0, unconditional 2
- src/Game/Map/KCollision.cpp: manual 1, unconditional 1
- src/Game/Map/OceanBowl.cpp: missing in SMG2 (2 groups)
- src/Game/Map/OceanRingPipeInside.cpp: missing in SMG2 (1 groups)
- src/Game/Map/OceanRingPipeOutside.cpp: missing in SMG2 (1 groups)
- src/Game/Map/RaceManager.cpp: manual 2, unconditional 0
- src/Game/Map/SleepControllerHolder.cpp: missing in SMG2 (1 groups)
- src/Game/Map/SwitchWatcherHolder.cpp: missing in SMG2 (1 groups)
- src/Game/Map/WaterPlant.cpp: manual 1, unconditional 1
- src/Game/Map/WaterRoad.cpp: missing in SMG2 (2 groups)
- src/Game/MapObj/AstroDomeBlueStar.cpp: missing in SMG2 (2 groups)
- src/Game/MapObj/ChipBase.cpp: manual 2, unconditional 0
- src/Game/MapObj/CrystalCage.cpp: manual 3, unconditional 0
- src/Game/MapObj/ElectricBall.cpp: missing in SMG2 (1 groups)
- src/Game/MapObj/LargeChain.cpp: missing in SMG2 (1 groups)
- src/Game/MapObj/LavaFloater.cpp: manual 1, unconditional 0
- src/Game/MapObj/LavaGalaxyParts.cpp: missing in SMG2 (1 groups)
- src/Game/MapObj/MarioLauncherAttractor.cpp: missing in SMG2 (1 groups)
- src/Game/MapObj/MiniatureGalaxy.cpp: missing in SMG2 (3 groups)
- src/Game/MapObj/OceanWaveFloater.cpp: missing in SMG2 (1 groups)
- src/Game/MapObj/PlantGroup.cpp: manual 2, unconditional 0
- src/Game/MapObj/Sandstorm.cpp: manual 1, unconditional 0
- src/Game/MapObj/SpinDriver.cpp: manual 1, unconditional 0
- src/Game/MapObj/SuperSpinDriver.cpp: manual 1, unconditional 0
- src/Game/NPC/KinopioAstro.cpp: missing in SMG2 (1 groups)
- src/Game/NPC/NPCActor.cpp: manual 0, unconditional 1
- src/Game/NPC/Syati.cpp: missing in SMG2 (1 groups)
- src/Game/NPC/TalkDirector.cpp: manual 1, unconditional 0
- src/Game/NPC/TalkMessageCtrl.cpp: manual 3, unconditional 1
- src/Game/NPC/TalkState.cpp: manual 3, unconditional 0
- src/Game/NPC/Tico.cpp: manual 1, unconditional 1
- src/Game/Player/GhostPacket.cpp: manual 2, unconditional 0
- src/Game/Player/J3DModelX.cpp: manual 0, unconditional 1
- src/Game/Player/MarineSnow.cpp: manual 0, unconditional 1
- src/Game/Player/Mario.cpp: manual 5, unconditional 0
- src/Game/Player/MarioActor.cpp: manual 3, unconditional 10
- src/Game/Player/MarioActorBlackHole.cpp: missing in SMG2 (2 groups)
- src/Game/Player/MarioActorDraw.cpp: missing in SMG2 (6 groups)
- src/Game/Player/MarioActorGameOver.cpp: missing in SMG2 (7 groups)
- src/Game/Player/MarioActorMorph.cpp: missing in SMG2 (1 groups)
- src/Game/Player/MarioActorParts.cpp: missing in SMG2 (3 groups)
- src/Game/Player/MarioActorRushMsg.cpp: missing in SMG2 (1 groups)
- src/Game/Player/MarioActorShadow.cpp: missing in SMG2 (2 groups)
- src/Game/Player/MarioEffect.cpp: manual 5, unconditional 0
- src/Game/Player/MarioShadow.cpp: missing in SMG2 (1 groups)
- src/Game/Player/MarioSound.cpp: missing in SMG2 (4 groups)
- src/Game/Player/MarioWalk.cpp: missing in SMG2 (4 groups)
- src/Game/Player/MarioWarp.cpp: manual 1, unconditional 2
- src/Game/Player/PlayerEvent.cpp: missing in SMG2 (1 groups)
- src/Game/RhythmLib/AudMePlayer.cpp: manual 1, unconditional 0
- src/Game/Ride/Plant.cpp: manual 1, unconditional 0
- src/Game/Ride/SphereAccelSensorController.cpp: manual 1, unconditional 0
- src/Game/Ride/SurfRay.cpp: missing in SMG2 (6 groups)
- src/Game/Scene/GameScene.cpp: manual 2, unconditional 0
- src/Game/Scene/IntermissionScene.cpp: manual 0, unconditional 1
- src/Game/Scene/LogoScene.cpp: manual 2, unconditional 0
- src/Game/Scene/PlacementInfoOrdered.cpp: manual 1, unconditional 0
- src/Game/Scene/SceneNameObjListExecutor.cpp: manual 4, unconditional 4
- src/Game/Scene/StageDataHolder.cpp: manual 5, unconditional 0
- src/Game/Screen/BloomEffect.cpp: manual 0, unconditional 1
- src/Game/Screen/BrosButton.cpp: missing in SMG2 (1 groups)
- src/Game/Screen/CustomTagProcessor.cpp: manual 1, unconditional 0
- src/Game/Screen/GalaxyConfirmLayout.cpp: manual 1, unconditional 0
- src/Game/Screen/GalaxyMap.cpp: missing in SMG2 (3 groups)
- src/Game/Screen/GalaxyMapController.cpp: missing in SMG2 (3 groups)
- src/Game/Screen/GalaxyMapSelectButton.cpp: missing in SMG2 (1 groups)
- src/Game/Screen/GalaxyNamePlate.cpp: manual 0, unconditional 1
- src/Game/Screen/InformationObserver.cpp: manual 1, unconditional 0
- src/Game/Screen/IsbnManager.cpp: missing in SMG2 (4 groups)
- src/Game/Screen/LayoutManager.cpp: manual 1, unconditional 0
- src/Game/Screen/LogoFader.cpp: manual 0, unconditional 1
- src/Game/Screen/LuigiLetter.cpp: missing in SMG2 (2 groups)
- src/Game/Screen/MiiSelect.cpp: manual 3, unconditional 0
- src/Game/Screen/MiiSelectIcon.cpp: manual 1, unconditional 0
- src/Game/Screen/MoviePlayerSimple.cpp: missing in SMG2 (2 groups)
- src/Game/Screen/MoviePlayingSequence.cpp: missing in SMG2 (3 groups)
- src/Game/Screen/MovieStarter.cpp: missing in SMG2 (3 groups)
- src/Game/Screen/OdhConverter.cpp: missing in SMG2 (1 groups)
- src/Game/Screen/PauseMenu.cpp: manual 0, unconditional 1
- src/Game/Screen/PeachLetter.cpp: missing in SMG2 (2 groups)
- src/Game/Screen/PictureBookLayout.cpp: missing in SMG2 (4 groups)
- src/Game/Screen/PowerStarList.cpp: missing in SMG2 (4 groups)
- src/Game/Screen/ProloguePictureBook.cpp: missing in SMG2 (3 groups)
- src/Game/Screen/ScenarioSelectLayout.cpp: manual 0, unconditional 1
- src/Game/Screen/StageResultInformer.cpp: manual 0, unconditional 5
- src/Game/Screen/SubMeterLayout.cpp: manual 1, unconditional 0
- src/Game/Screen/SysInfoWindow.cpp: manual 1, unconditional 0
- src/Game/Screen/THPSimplePlayerWrapper.cpp: missing in SMG2 (40 groups)
- src/Game/Screen/TitleSequenceProduct.cpp: manual 1, unconditional 0
- src/Game/Screen/odh.cpp: missing in SMG2 (2 groups)
- src/Game/Speaker/SpkTable.cpp: manual 1, unconditional 0
- src/Game/Speaker/SpkWave.cpp: manual 1, unconditional 0
- src/Game/System/DrawBufferExecuter.cpp: manual 0, unconditional 1
- src/Game/System/DrawSyncManager.cpp: manual 0, unconditional 2
- src/Game/System/FileHolder.cpp: manual 1, unconditional 0
- src/Game/System/FileRipper.cpp: manual 1, unconditional 0
- src/Game/System/GalaxyCometScheduler.cpp: missing in SMG2 (1 groups)
- src/Game/System/GameDataFunction.cpp: manual 1, unconditional 0
- src/Game/System/GameDataGalaxyStorage.cpp: missing in SMG2 (4 groups)
- src/Game/System/GameEventFlagChecker.cpp: missing in SMG2 (10 groups)
- src/Game/System/GameSequenceProgress.cpp: missing in SMG2 (4 groups)
- src/Game/System/GameSystem.cpp: manual 6, unconditional 0
- src/Game/System/GameSystemSceneController.cpp: manual 1, unconditional 0
- src/Game/System/GameSystemStationedArchiveLoader.cpp: manual 1, unconditional 3
- src/Game/System/HeapMemoryWatcher.cpp: manual 5, unconditional 0
- src/Game/System/Language.cpp: manual 1, unconditional 0
- src/Game/System/MainLoopFramework.cpp: manual 1, unconditional 0
- src/Game/System/MessageHolder.cpp: missing in SMG2 (7 groups)
- src/Game/System/ResourceHolder.cpp: manual 1, unconditional 0
- src/Game/System/ResourceHolderManager.cpp: manual 0, unconditional 2
- src/Game/System/SaveDataBannerCreator.cpp: manual 0, unconditional 2
- src/Game/System/SpinDriverPathStorage.cpp: missing in SMG2 (1 groups)
- src/Game/System/StorySequenceExecutor.cpp: missing in SMG2 (6 groups)
- src/Game/System/WPadButton.cpp: manual 1, unconditional 0
- src/Game/Util/CameraUtil.cpp: manual 1, unconditional 0
- src/Game/Util/GravityUtil.cpp: manual 1, unconditional 0
- src/Game/Util/HashUtil.cpp: manual 6, unconditional 0
- src/Game/Util/JMapInfo.cpp: manual 0, unconditional 3
- src/Game/Util/LayoutUtil.cpp: manual 84, unconditional 0
- src/Game/Util/LiveActorUtil.cpp: manual 1, unconditional 0
- src/Game/Util/MathUtil.cpp: manual 1, unconditional 0
- src/Game/Util/MemoryUtil.cpp: manual 1, unconditional 0
- src/Game/Util/StarPointerUtil.cpp: manual 2, unconditional 0
- src/Game/Util/StringUtil.cpp: manual 0, unconditional 1
- src/Game/Util/SystemUtil.cpp: manual 1, unconditional 0
- src/JSystem/J2DGraph/J2DPane.cpp: manual 0, unconditional 1
- src/JSystem/J3DGraphAnimator/J3DAnimation.cpp: manual 1, unconditional 8
- src/JSystem/J3DGraphAnimator/J3DMtxBuffer.cpp: manual 0, unconditional 1
- src/JSystem/J3DGraphBase/J3DDrawBuffer.cpp: manual 0, unconditional 1
- src/JSystem/J3DGraphBase/J3DTevs.cpp: manual 0, unconditional 2
- src/JSystem/J3DGraphBase/J3DTransform.cpp: manual 0, unconditional 1
- src/JSystem/J3DGraphLoader/J3DAnmLoader.cpp: manual 0, unconditional 4
- src/JSystem/JAudio2/JAISoundHandles.cpp: manual 0, unconditional 1
- src/JSystem/JAudio2/JAIStream.cpp: manual 0, unconditional 1
- src/JSystem/JAudio2/JASAramStream.cpp: manual 0, unconditional 6
- src/JSystem/JAudio2/JASBNKParser.cpp: manual 0, unconditional 6
- src/JSystem/JAudio2/JASTrack.cpp: manual 0, unconditional 2
- src/JSystem/JAudio2/JASWaveArcLoader.cpp: manual 0, unconditional 1
- src/JSystem/JAudio2/dspproc.cpp: manual 0, unconditional 10
- src/JSystem/JAudio2/osdsp_task.cpp: manual 0, unconditional 8
- src/JSystem/JKernel/JKRAram.cpp: manual 0, unconditional 1
- src/JSystem/JKernel/JKRAramArchive.cpp: manual 0, unconditional 2
- src/JSystem/JKernel/JKRArchivePri.cpp: manual 2, unconditional 0
- src/JSystem/JKernel/JKRDecomp.cpp: manual 0, unconditional 2
- src/JSystem/JKernel/JKRDvdAramRipper.cpp: manual 0, unconditional 1
- src/JSystem/JKernel/JKRDvdRipper.cpp: manual 0, unconditional 1
- src/JSystem/JKernel/JKRExpHeap.cpp: manual 0, unconditional 4
- src/JSystem/JKernel/JKRHeap.cpp: manual 0, unconditional 1
- src/JSystem/JKernel/JKRMemArchive.cpp: manual 0, unconditional 2
- src/JSystem/JKernel/JKRThread.cpp: manual 0, unconditional 1
- src/JSystem/JKernel/JKRUnitHeap.cpp: manual 0, unconditional 5
- src/JSystem/JMath/JMath.cpp: manual 3, unconditional 0
- src/JSystem/JParticle/JPAResource.cpp: manual 0, unconditional 2
- src/JSystem/JSupport/JSUMemoryStream.cpp: manual 0, unconditional 2
- src/JSystem/JUtility/JUTException.cpp: manual 0, unconditional 1
- src/JSystem/JUtility/JUTTexture.cpp: manual 0, unconditional 6
- src/RVLFaceLib/RFL_MakeRandomFace.c: missing in SMG2 (1 groups)
- src/RVLFaceLib/RFL_Model.c: manual 1, unconditional 15
- src/RVLFaceLib/RFL_NANDLoader.c: manual 1, unconditional 0
- src/RVLFaceLib/RFL_System.c: manual 1, unconditional 0
- src/RVL_SDK/mem/mem_allocator.c: manual 0, unconditional 2
- src/RVL_SDK/mem/mem_expHeap.c: manual 2, unconditional 0
- src/nw4r/db/db_assert.cpp: manual 2, unconditional 0
- src/nw4r/lyt/lyt_arcResourceAccessor.cpp: missing in SMG2 (1 groups)

## Landing note (redo)

Compile-gate exclusions (tools/team/hookgate.py; their hooks need hand porting): see `excluded` rows above.
Also left for P1: files with native-semantics divergences (MessageTagSkipTagProcessor.hpp, AudRemixMgr.cpp/.hpp, RingBeam.cpp, FileSelector.cpp, HeapMemoryWatcher.cpp, MarioActor.hpp; corrections in scratch-port/solb/NATIVE-SEMANTICS.md), files that changed on main during the redo (ResourceHolder.cpp, RFL_DataUtility.c) and enemies-deferred files (AudSystem.cpp, MogucchiShooter.cpp, ParticleDrawExecutor.cpp, Popo.cpp, LayoutManager.cpp, SuddenDeathMeter.cpp).
Proof: tools/team/hookproof.sh, two clean Wii object builds, identical code/data/relocations/symbols for all 2,423 objects.
