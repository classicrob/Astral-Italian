#pragma once

#include <revolution/types.h>

namespace MR {
    /* thanks to https://github.com/magcius/noclip.website/blob/master/src/SuperMarioGalaxy/NameObj.ts */
    enum MovementType {
        MovementType_None = -1,
        /* 0x00 */ MovementType_UNK_0x00,
        /* 0x01 */ MovementType_StopSceneDelayRequest,
        /* 0x02 */ MovementType_Camera,
        /* 0x03 */ MovementType_ScreenEffect,
        /* 0x04 */ MovementType_ClippingDirector,
        /* 0x05 */ MovementType_SensorHitChecker,
        /* 0x06 */ MovementType_MsgSharedGroup,
        /* 0x07 */ MovementType_UNK_0x07,
        /* 0x08 */ MovementType_UNK_0x08,
        /* 0x09 */ MovementType_AudCameraWatcher,
        /* 0x0A */ MovementType_TalkDirector,
        /* 0x0B */ MovementType_DemoDirector,
        /* 0x0C */ MovementType_UNK_0x0C,
        /* 0x0D */ MovementType_AreaObj,
        /* 0x0E */ MovementType_Layout,
        /* 0x0F */ MovementType_LayoutDecoration,
        /* 0x10 */ MovementType_WipeLayout,
        /* 0x11 */ MovementType_LayoutOnPause,
        /* 0x12 */ MovementType_UNK_0x14,  // SMG2 id 0x12 (Particle check-update); name kept from Petari
        /* 0x13 */ MovementType_UNK_0x15,
        /* 0x14 */ MovementType_MirrorCamera,
        /* 0x15 */ MovementType_ImageEffect,
        /* 0x16 */ MovementType_AudEffectDirector,
        /* 0x17 */ MovementType_AudBgmConductor,
        /* 0x18 */ MovementType_CameraCover,
        /* 0x19 */ MovementType_SwitchWatcherHolder,
        /* 0x1A */ MovementType_PauseBlur,
        /* 0x1B */ MovementType_ClippedMapParts,
        /* 0x1C */ MovementType_Planet,
        /* 0x1D */ MovementType_CollisionMapObj,
        /* 0x1E */ MovementType_CollisionEnemy,
        /* 0x1F */ MovementType_CollisionDirector,
        /* 0x20 */ MovementType_Environment,
        /* 0x21 */ MovementType_MapObj,
        /* 0x22 */ MovementType_MapObjDecoration,
        /* 0x23 */ MovementType_Sky,
        /* 0x24 */ MovementType_Player,
        /* 0x25 */ MovementType_PlayerDecoration,
        /* 0x26 */ MovementType_PlayerMessenger,
        /* 0x27 */ MovementType_NPC,
        /* 0x28 */ MovementType_Ride,
        /* 0x29 */ MovementType_Enemy,
        /* 0x2A */ MovementType_EnemyDecoration,
        /* 0x2B */ MovementType_Item,
        /* 0x2C */ MovementType_ShadowControllerHolder,
    };

    enum CalcAnimType {
        CalcAnimType_None = -1,
        /* 0x00 */ CalcAnimType_ClippedMapParts,
        /* 0x01 */ CalcAnimType_Planet,
        /* 0x02 */ CalcAnimType_CollisionMapObj,
        /* 0x03 */ CalcAnimType_CollisionEnemy,
        /* 0x04 */ CalcAnimType_Environment,
        /* 0x05 */ CalcAnimType_MapObj,
        /* 0x06 */ CalcAnimType_NPC,
        /* 0x07 */ CalcAnimType_Ride,
        /* 0x08 */ CalcAnimType_Enemy,
        /* 0x09 */ CalcAnimType_Player,
        /* 0x0A */ CalcAnimType_PlayerDecoration,
        /* 0x0B */ CalcAnimType_MapObjDecoration,
        /* 0x0C */ CalcAnimType_MirrorMapObj,
        /* 0x0D */ CalcAnimType_Layout,
        /* 0x0E */ CalcAnimType_LayoutDecoration,
        /* 0x0F */ CalcAnimType_Item,
        /* 0x10 */ CalcAnimType_UNK_0x10,
        /* 0x11 */ CalcAnimType_UNK_0x11,
        /* 0x12 */ CalcAnimType_AnimParticle,
        /* 0x13 */ CalcAnimType_AnimParticleIgnorePause,
        /* 0x14 */ CalcAnimType_WorldMapMini,
        /* 0x15 */ CalcAnimType_WorldMapIcon,
        /* 0x16 */ CalcAnimType_UNK_0x16,
    };

    enum DrawBufferType {
        DrawBufferType_None = -1,
        /* 0x00 */ DrawBufferType_ClippedMapParts,
        /* 0x01 */ DrawBufferType_UNK_0x01,
        /* 0x02 */ DrawBufferType_Sky,
        /* 0x03 */ DrawBufferType_Air,
        /* 0x04 */ DrawBufferType_Sun,
        /* 0x05 */ DrawBufferType_Planet,
        /* 0x06 */ DrawBufferType_PlanetLow,
        /* 0x07 */ DrawBufferType_Environment,
        /* 0x08 */ DrawBufferType_EnvironmentStrongLight,
        /* 0x09 */ DrawBufferType_MapObj,
        /* 0x0A */ DrawBufferType_MapObjWeakLight,
        /* 0x0B */ DrawBufferType_MapObjStrongLight,
        /* 0x0C */ DrawBufferType_NoShadowedMapObj,
        /* 0x0D */ DrawBufferType_NoShadowedMapObjStrongLight,
        /* 0x0E */ DrawBufferType_NoSilhouettedMapObj,
        /* 0x0F */ DrawBufferType_NoSilhouettedMapObjWeakLight,
        /* 0x10 */ DrawBufferType_NoSilhouettedMapObjStrongLight,
        /* 0x11 */ DrawBufferType_NPC,
        /* 0x12 */ DrawBufferType_Ride,
        /* 0x13 */ DrawBufferType_Enemy,
        /* 0x14 */ DrawBufferType_EnemyDecoration,
        /* 0x15 */ DrawBufferType_Player,
        /* 0x16 */ DrawBufferType_PlayerDecoration,
        /* 0x17 */ DrawBufferType_CrystalBox,
        /* 0x18 */ DrawBufferType_UNK_0x17,
        /* 0x19 */ DrawBufferType_UNK_0x18,
        /* 0x1A */ DrawBufferType_IndirectMapObj,
        /* 0x1B */ DrawBufferType_IndirectMapObjStrongLight,
        /* 0x1C */ DrawBufferType_IndirectNpc,
        /* 0x1D */ DrawBufferType_IndirectEnemy,
        /* 0x1E */ DrawBufferType_IndirectPlanet,
        /* 0x1F */ DrawBufferType_BloomModel,
        /* 0x20 */ DrawBufferType_TripodBoss,
        /* 0x21 */ DrawBufferType_Crystal,
        /* 0x22 */ DrawBufferType_CrystalItem,
        /* 0x23 */ DrawBufferType_GlaringLight,
        /* 0x24 */ DrawBufferType_AstroDomeSky,
        /* 0x25 */ DrawBufferType_UNK_0x25,
        /* 0x26 */ DrawBufferType_UNK_0x26,
        /* 0x27 */ DrawBufferType_WorldMapMini,
        /* 0x28 */ DrawBufferType_WorldMapIcon,
        /* 0x29 */ DrawBufferType_Model3DFor2D,
        /* 0x2A */ DrawBufferType_UNK_0x2A,
        /* 0x2B */ DrawBufferType_SwitchingScreenEffect,
        /* 0x2C */ DrawBufferType_MirrorMapObj,
        /* 0x2D */ DrawBufferType_UNK_0x2D,
    };

    // TODO: Tweaked from Petari, might not be correct
    enum DrawType {
        DrawType_None = -1,
        /* 0x00 */ DrawType_SwingRope,
        /* 0x01 */ DrawType_0x1,
        /* 0x02 */ DrawType_Creeper,
        /* 0x03 */ DrawType_0x3,
        /* 0x04 */ DrawType_PlantStalk,
        /* 0x05 */ DrawType_Plant,
        /* 0x06 */ DrawType_Trapeze,
        /* 0x07 */ DrawType_OceanBowl,
        /* 0x08 */ DrawType_OceanRing,
        /* 0x09 */ DrawType_OceanRingPipeInside,
        /* 0x0A */ DrawType_OceanRingPipeOutside,
        /* 0x0B */ DrawType_OceanSphere,
        /* 0x0C */ DrawType_WaterRoad,
        /* 0x0D */ DrawType_WhirlPoolAccelerator,
        /* 0x0E */ DrawType_ElectricRailHolder,
        /* 0x0F */ DrawType_BigBubble,
        /* 0x10 */ DrawType_BigBubbleGoal,
        /* 0x11 */ DrawType_0x11,
        /* 0x12 */ DrawType_SpinDriverPathDrawer,
        /* 0x13 */ DrawType_GCapture,
        /* 0x14 */ DrawType_ClipAreaDropLaser,
        /* 0x15 */ DrawType_SpiderThread,
        /* 0x16 */ DrawType_SpaceCocoon,
        /* 0x17 */ DrawType_0x17,
        /* 0x18 */ DrawType_WarpPodPath,     // TODO: DrawType_NormalMap?
        /* 0x19 */ DrawType_FlexibleSphere,  // TODO: DrawType_DynamicCollisionObj?
        /* 0x1A */ DrawType_KirairaChain,
        /* 0x1B */ DrawType_WaterPlant,
        /* 0x1C */ DrawType_VolumeModel,
        /* 0x1D */ DrawType_UNK_0x1D,  // SMG2: Flag is 0x1E (Flag.cpp), Player is 0x25; exact placement of the dropped id unverified
        /* 0x1E */ DrawType_Flag,
        /* 0x1F */ DrawType_AstroDomeSkyClear,
        /* 0x20 */ DrawType_AstroDomeOrbit,
        /* 0x21 */ DrawType_WoodLogBridge,
        /* 0x22 */ DrawType_HipDropStarMove,
        /* 0x23 */ DrawType_DynamicMapObj,
        /* 0x24 */ DrawType_OceanBowlBloomDrawer,
        /* 0x25 */ DrawType_Player,
        /* 0x26 */ DrawType_0x26,
        /* 0x27 */ DrawType_0x27,
        /* 0x28 */ DrawType_MiiFacePartsHolder,
        /* 0x29 */ DrawType_0x29,
        /* 0x2A */ DrawType_ShadowSurface,
        /* 0x2B */ DrawType_ShadowVolume,
        /* 0x2C */ DrawType_0x2C,
        /* 0x2D */ DrawType_AlphaShadow,
        /* 0x2E */ DrawType_ClipArea,
        /* 0x2F */ DrawType_FallOutFieldDraw,
        /* 0x30 */ DrawType_ClipFieldFillDraw,
        /* 0x31 */ DrawType_CaptureScreenIndirect,
        /* 0x32 */ DrawType_CaptureScreenCamera,
        /* 0x33 */ DrawType_0x33,
        /* 0x34 */ DrawType_CenterScreenBlur,
        /* 0x35 */ DrawType_0x35,
        /* 0x36 */ DrawType_Fur,
        /* 0x37 */ DrawType_0x37,
        /* 0x38 */ DrawType_0x38,
        /* 0x39 */ DrawType_BloomEffectPreDraw,
        /* 0x3A */ DrawType_BloomEffectPostDraw,
        /* 0x3B */ DrawType_BloomModel,
        /* 0x3C */ DrawType_ImageEffect,
        /* 0x3D */ DrawType_DashRing,
        /* 0x3E */ DrawType_BrightSun,
        /* 0x3F */ DrawType_WaterCameraFilter,
        /* 0x40 */ DrawType_0x40,
        /* 0x41 */ DrawType_TimeStopScreenEffect,
        /* 0x42 */ DrawType_PauseBlur,
        /* 0x43 */ DrawType_0x43,
        /* 0x44 */ DrawType_0x44,
        /* 0x45 */ DrawType_0x45,
        /* 0x46 */ DrawType_Layout,
        /* 0x47 */ DrawType_LayoutDecoration,
        /* 0x48 */ DrawType_LayoutOnPause,
        /* 0x49 */ DrawType_CometScreenFilter,
        /* 0x4A */ DrawType_WipeLayout,
        /* 0x4B */ DrawType_TalkLayout,
        /* 0x4C */ DrawType_0x4C,
        /* 0x4D */ DrawType_CinemaFrame,
        /* 0x4E */ DrawType_GalaxyNamePlate,
        /* 0x4F */ DrawType_WorldMapInfo,
        /* 0x50 */ DrawType_0x50,
        /* 0x51 */ DrawType_FileSelect,
        /* 0x52 */ DrawType_EffectDraw3D,
        /* 0x53 */ DrawType_EffectDrawIndirect,
        /* 0x54 */ DrawType_EffectDrawAfterIndirect,
        /* 0x55 */ DrawType_EffectDrawWorldMapMiniIcon,
        /* 0x56 */ DrawType_EffectDraw2D,
        /* 0x57 */ DrawType_EffectDrawFor2DModel,
        /* 0x58 */ DrawType_EffectDrawForBloomEffect,
        /* 0x59 */ DrawType_EffectDrawAfterImageEffect,
        /* 0x5A */ DrawType_MiiFaceIcon,
        /* 0x5B */ DrawType_MiiFaceNew,
        /* 0x5C */ DrawType_CameraCover,
    };

    enum LightType {
        LightType_None = -1,  // TODO: better name?
        /* 0x00 */ LightType_Player,
        /* 0x01 */ LightType_Strong,
        /* 0x02 */ LightType_Weak,
        /* 0x03 */ LightType_Planet,
        /* 0x04 */ LightType_Coin,
    };

    enum CameraType {
        /* 0x00 */ CameraType_3D,
        /* 0x01 */ CameraType_2D,
        /* 0x02 */ CameraType_Mirror,
        /* 0x03 */ CameraType_WorldMapBig,
        /* 0x04 */ CameraType_WorldMapSmall,
    };
};  // namespace MR

class Scene;

class SceneFunction {
public:
    static void movementStopSceneController();
    static void executeMovementList();
    static void executeMovementListOnPlayingMovie();
    static void executeCalcAnimList();
    static void executeCalcAnimListWithWorldMap();
    static void executeDrawAfterImageEffect();
    static void executeCalcAnimListOnPlayingMovie();
    static void executeCalcViewAndEntryList();
    static void executeCalcViewAndEntryList2D();
    static void executeDrawBufferListNormalOpaBeforeVolumeShadow();
    static void executeDrawBufferListNormalOpaBeforeSilhouette();
    static void executeDrawBufferListNormalOpa();
    static void executeDrawBufferListNormalXlu();
    static void executeDrawListOpa();
    static void executeDrawListXlu();
    static void executeDrawSilhouetteAndFillShadow();
    static void executeDrawAlphaShadow();
    static void executeDrawAfterIndirect();
    static void executeDrawImageEffect();
    static void executeDrawList2DNormal();
    static void executeDrawList2DMovie();
    static void startStageFileLoad();
    static void waitDoneStageFileLoad();
    static void startActorFileLoadCommon();
    static void startActorFileLoadScenario();
    static void startActorFileLoadPostCreation();
    static void startActorPlacement();
    static void initAfterScenarioSelected();
    static void initForNameObj();
    static void initForLiveActor();
    static void initEffectSystem(u32, u32);
    static void allocateDrawBufferActorList();
    static void createHioBasicNode(Scene*);
};

class CategoryList {
public:
    static void execute(MR::MovementType) NO_INLINE;
    static void execute(MR::CalcAnimType) NO_INLINE;
    static void execute(MR::DrawType) NO_INLINE;
    static void entryDrawBuffer2D() NO_INLINE;
    static void entryDrawBuffer3D() NO_INLINE;
    static void entryDrawBufferMirror() NO_INLINE;
    static void entryDrawBufferWorldMapBig() NO_INLINE;
    static void entryDrawBufferWorldMapSmall() NO_INLINE;
    static void drawOpa(MR::DrawBufferType) NO_INLINE;
    static void drawXlu(MR::DrawBufferType) NO_INLINE;
    static void requestMovementOn(MR::MovementType) NO_INLINE;
    static void requestMovementOff(MR::MovementType) NO_INLINE;
};

namespace MR {
    void movementWorldMapBig();
    void calcAnimWorldMapBig();
    void entryWorldMapBig();
    void drawWorldMapBig();
    void drawWorldMapMiniEffects();
    void drawWorldMapInfo();
    void movementWorldMapMini();
    void calcAnimWorldMapMini();
    void entryWorldMapMini();
    void drawWorldMapMini();
}
