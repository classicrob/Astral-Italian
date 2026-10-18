#pragma once

// SMG2 SceneObj IDs, derived from SceneObjHolder::newEachObj (jump table)
enum SceneObj {
    SceneObj_SensorHitChecker = 0x00,
    SceneObj_CollisionDirector = 0x01,
    SceneObj_ClippingDirector = 0x02,
    SceneObj_DemoDirector = 0x03,
    SceneObj_EventDirector = 0x04,
    SceneObj_EffectSystem = 0x05,
    SceneObj_LightDirector = 0x06,
    SceneObj_SceneDataInitializer = 0x07,
    SceneObj_StageDataHolder = 0x08,
    SceneObj_MessageSensorHolder = 0x09,
    SceneObj_StageSwitchContainer = 0x0A,
    SceneObj_SwitchWatcherHolder = 0x0B,
    SceneObj_AwakeControllerHolder = 0x0C,
    SceneObj_AreaObjContainer = 0x0D,
    SceneObj_LiveActorGroupArray = 0x0E,
    SceneObj_MovementOnOffGroupHolder = 0x0F,
    SceneObj_CaptureScreenActor = 0x10,
    SceneObj_AudCameraWatcher = 0x11,
    SceneObj_AudEffectDirector = 0x12,
    SceneObj_AudBgmConductor = 0x13,
    SceneObj_ActionSoundSystem = 0x14,
    SceneObj_MarioHolder = 0x15,
    SceneObj_Unk16 = 0x16,
    SceneObj_MirrorCamera = 0x17,
    SceneObj_CameraContext = 0x18,
    SceneObj_NameObjGroup = 0x19,
    SceneObj_TalkDirector = 0x1A,
    SceneObj_EventSequencer = 0x1B,
    SceneObj_StopSceneController = 0x1C,
    SceneObj_SceneNameObjMovementController = 0x1D,
    SceneObj_ImageEffectSystemHolder = 0x1E,
    SceneObj_BloomEffect = 0x1F,
    SceneObj_BloomEffectSimple = 0x20,
    SceneObj_ScreenBlurEffect = 0x21,
    SceneObj_DepthOfFieldBlur = 0x22,
    SceneObj_PostFog = 0x23,
    SceneObj_PostOutline = 0x24,
    SceneObj_SceneWipeHolder = 0x25,
    SceneObj_PlayerActionGuidance = 0x26,
    SceneObj_LensFlareDirector = 0x27,
    SceneObj_FurDrawManager = 0x28,
    SceneObj_PlacementStateChecker = 0x29,
    SceneObj_NamePosHolder = 0x2A,
    SceneObj_NPCDirector = 0x2B,
    SceneObj_ResourceShare = 0x2C,
    SceneObj_WarpPodMgr = 0x2D,
    SceneObj_CenterScreenBlur = 0x2E,
    SceneObj_SwitchingScreenEffect = 0x2F,
    SceneObj_TimeStopScreenCaptureTexture = 0x30,
    SceneObj_TimeStopScreenEffect = 0x31,
    SceneObj_SwingActionWatcher = 0x32,
    SceneObj_Unk33 = 0x33,
    SceneObj_WorldMapResultCommand = 0x34,
    SceneObj_ScoreAttackAccessor = 0x35,
    SceneObj_ScoreResultKeeper = 0x36,
    SceneObj_Unk37 = 0x37,
    SceneObj_SupportPlayDirector = 0x38,
    SceneObj_AllLiveActorGroup = 0x39,
    SceneObj_CameraDirector = 0x3A,
    SceneObj_PlanetGravityManager = 0x3B,
    SceneObj_BaseMatrixFollowTargetHolder = 0x3C,
    SceneObj_GameSceneLayoutHolder = 0x3D,
    SceneObj_Unk3E = 0x3E,
    SceneObj_CoinHolder = 0x3F,
    SceneObj_PurpleCoinHolder = 0x40,
    SceneObj_CoinRotater = 0x41,
    SceneObj_AirBubbleHolder = 0x42,
    SceneObj_KarikariDirector = 0x43,
    SceneObj_StarPieceDirector = 0x44,
    SceneObj_BegomanAttackPermitter = 0x45,
    SceneObj_BigFanHolder = 0x46,
    SceneObj_Unk47 = 0x47,
    SceneObj_Unk48 = 0x48,
    SceneObj_Unk49 = 0x49,
    SceneObj_KabokuriFireHolder = 0x4A,
    SceneObj_TakoHeiInkHolder = 0x4B,
    SceneObj_ShadowControllerHolder = 0x4C,
    SceneObj_ShadowColorSetter = 0x4D,
    SceneObj_ShadowVolumeDrawInit = 0x4E,
    SceneObj_ShadowSurfaceDrawInit = 0x4F,
    SceneObj_SwingRopeGroup = 0x50,
    SceneObj_PlantStalkDrawInit = 0x51,
    SceneObj_PlantLeafDrawInit = 0x52,
    SceneObj_TrapezeRopeDrawInit = 0x53,
    SceneObj_Unk54 = 0x54,
    SceneObj_SpinDriverPathDrawInit = 0x55,
    SceneObj_NoteGroup = 0x56,
    SceneObj_ClipAreaDropHolder = 0x57,
    SceneObj_FallOutFieldDraw = 0x58,
    SceneObj_ClipFieldFillDraw = 0x59,
    SceneObj_Unk5A = 0x5A,
    SceneObj_ClipAreaHolder = 0x5B,
    SceneObj_SnowStepHolder = 0x5C,
    SceneObj_LightFruitClipArea = 0x5D,
    SceneObj_ScreenAlphaCapture = 0x5E,
    SceneObj_MapPartsRailGuideHolder = 0x5F,
    SceneObj_SpinCloudBlockGroup = 0x60,
    SceneObj_GCapture = 0x61,
    SceneObj_SupportTico = 0x62,
    SceneObj_NameObjExecuteHolder = 0x63,
    SceneObj_ElectricRailHolder = 0x64,
    SceneObj_SpiderThread = 0x65,
    SceneObj_QuakeEffectGenerator = 0x66,
    SceneObj_KoopaBattleAccessor = 0x67,
    SceneObj_Unk68 = 0x68,
    SceneObj_Unk69 = 0x69,
    SceneObj_HeatHazeDirector = 0x6A,
    SceneObj_ChipHolder = 0x6B,
    SceneObj_EarthenPipeMediator = 0x6C,
    SceneObj_PetitPorterMediator = 0x6D,
    SceneObj_Unk6E = 0x6E,
    SceneObj_WaterAreaHolder = 0x6F,
    SceneObj_WaterPlantDrawInit = 0x70,
    SceneObj_RaceManager = 0x71,
    SceneObj_GroupCheckManager = 0x72,
    SceneObj_WaterPressureBulletHolder = 0x73,
    SceneObj_FirePressureBulletHolder = 0x74,
    SceneObj_SunshadeMapHolder = 0x75,
    SceneObj_MiiFacePartsHolder = 0x76,
    SceneObj_MiiFaceIconHolder = 0x77,
    SceneObj_FluffWindHolder = 0x78,
    SceneObj_GalaxyNamePlateDrawer = 0x79,
    SceneObj_CinemaFrame = 0x7A,
    SceneObj_Unk7B = 0x7B,
    SceneObj_PlayerFollowPass = 0x7C,
    SceneObj_InfernoMarioManager = 0x7D,
    SceneObj_PlanetMapCreator = 0x7E,
    SceneObj_ProductMapCreator = 0x7F,
    SceneObj_PriorDrawAirHolder = 0x80,
    SceneObj_InformationObserver = 0x81,
    SceneObj_StaffRoll = 0x82,
    SceneObj_LiveActorGroup = 0x83,
    SceneObj_StarLightAreaHolder = 0x84,
    SceneObj_ChangeObjHolder = 0x85,
    SceneObj_DrillHoleHolder = 0x86,
    SceneObj_CollectTicoHolder = 0x87,
    SceneObj_IceMeteorHolder = 0x88,
    SceneObj_StoryBookDemo = 0x89,
    SceneObj_PopoStickingManager = 0x8A,
    SceneObj_KoopaJrCastleAccessor = 0x8B,
    SceneObj_KoopaJrCastleWindHolder = 0x8C,
    SceneObj_GliderShooterBulletHolder = 0x8D,
    SceneObj_GhostAttackAccessor = 0x8E,
    SceneObj_GliBirdTutorial = 0x8F,
    SceneObj_PauseBlur = 0x90,
    SceneObj_StarPieceFlowerController = 0x91,
    SceneObj_ExterminationWoodBoxFlagHolder = 0x92,
    SceneObj_MarioFacePlanetSwitch = 0x93,
    SceneObj_MarioFaceShipNpcHolder = 0x94,
    SceneObj_MarioFaceShipAttentionConversationDirector = 0x95,
    SceneObj_Unk96 = 0x96,
    SceneObj_MarioFaceShipNormalConversationDirector = 0x96,  // alias of Unk96 (core4)
    SceneObj_MarioFaceShipEventDataHolder = 0x97,
    SceneObj_WiiMessageBoardMailSender = 0x98,
    SceneObj_SleepControllerHolder = 0x0C,  // Petari name for AwakeControllerHolder
    SceneObj_NumMax = 0x99,
};

class NameObj;

class SceneObjHolder {
public:
    SceneObjHolder();
    NameObj* create(int);
    NameObj* getObj(int) const;
    bool isExist(int) const;
    NameObj* newEachObj(int);

    NameObj* mObjs[0x99];  // 0x00
};

namespace MR {
    NameObj* createSceneObj(int);
    SceneObjHolder* getSceneObjHolder();
    bool isExistSceneObj(int);

    // getSceneObj< Foo* >(id) (Garigari style) and getSceneObj< Foo >(id) (Petari style) both return Foo*.
    template < class T >
    struct SceneObjCast {
        typedef T* Ptr;
        static Ptr cast(NameObj* pObj) { return static_cast< T* >(pObj); }
    };

    template < class T >
    struct SceneObjCast< T* > {
        typedef T* Ptr;
        static Ptr cast(NameObj* pObj) { return reinterpret_cast< T* >(pObj); }
    };

    template < class T >
    inline typename SceneObjCast< T >::Ptr getSceneObj(int objID) {
        return SceneObjCast< T >::cast(MR::getSceneObjHolder()->getObj(objID));
    }
};  // namespace MR
