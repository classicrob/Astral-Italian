#include "Game/Demo/StoryBookDemo.hpp"
#include "Game/Enemy/GliderShooterBullet.hpp"
#include "Game/GameAudio/ActionSoundSystem.hpp"
#include "Game/Map/ProductMapCreator.hpp"
#include "Game/MapObj/CollectTicoHolder.hpp"
#include "Game/MapObj/ExterminationWoodBoxFlagHolder.hpp"
#include "Game/MapObj/LightFruitClipArea.hpp"
#include "Game/MapObj/MarioFaceShipNpcHolder.hpp"
#include "Game/MapObj/PlayerFollowPass.hpp"
#include "Game/MapObj/StarLightAreaHolder.hpp"
#include "Game/MapObj/StarPieceFlowerController.hpp"
#include "Game/MapObj/SwingActionWatcher.hpp"
#include "Game/NPC/GhostAttackAccessor.hpp"
#include "Game/Player/InfernoMarioManager.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultCommand.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Inline.hpp"

namespace MR {
    NO_INLINE SceneObjHolder* getSceneObjHolder();
}
#include "Game/Util.hpp"
#include "Game/System/MarioFaceShipEventDataHolder.hpp"
#include "Game/System/MarioFaceShipAttentionConversationDirector.hpp"
#include "Game/System/MarioFacePlanetSwitch.hpp"
#include "Game/Screen/TimeStopScreenEffect.hpp"
#include "Game/Screen/TimeStopScreenCaptureTexture.hpp"
#include "Game/Screen/SwitchingScreenEffect.hpp"
#include "Game/Screen/ScoreResult.hpp"
#include "Game/Screen/PostOutline.hpp"
#include "Game/Screen/PostFog.hpp"
#include "Game/Ride/GliBird.hpp"
#include "Game/NWC24/WiiMessageBoardMailSenderState.hpp"
#include "Game/NPC/SupportTico.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/NPC/PostmanEvent.hpp"
#include "Game/MapObj/SupportPlayDirector.hpp"
#include "Game/MapObj/SpinCloudBlock.hpp"
#include "Game/MapObj/SnowStep.hpp"
#include "Game/MapObj/PetitPorter.hpp"
#include "Game/MapObj/IceMeteorParts.hpp"
#include "Game/MapObj/DrillHole.hpp"
#include "Game/Map/ShadowColorSetter.hpp"
#include "Game/Map/PauseBlur.hpp"
#include "Game/LiveActor/ChangeObjHolder.hpp"
#include "Game/Enemy/Popo.hpp"
#include "Game/Boss/KoopaJrCastle.hpp"
#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/AreaObj/AreaObjContainer.hpp"
#include "Game/Boss/BossAccessor.hpp"
#include "Game/Boss/SkeletalFishBabyRailHolder.hpp"
#include "Game/Boss/SkeletalFishBossRailHolder.hpp"
#include "Game/Boss/TripodBossAccesser.hpp"
#include "Game/Camera/CameraContext.hpp"
#include "Game/Camera/CameraDirector.hpp"
#include "Game/Demo/DemoDirector.hpp"
#include "Game/Demo/PrologueDirector.hpp"
#include "Game/Effect/EffectSystem.hpp"
#include "Game/Enemy/BegomanBase.hpp"
#include "Game/Enemy/KabokuriFireHolder.hpp"
#include "Game/Enemy/KameckBeamHolder.hpp"
#include "Game/Enemy/KarikariDirector.hpp"
#include "Game/Enemy/TakoHeiInkHolder.hpp"
#include "Game/GameAudio/AudBgmConductor.hpp"
#include "Game/GameAudio/AudCameraWatcher.hpp"
#include "Game/GameAudio/AudEffectDirector.hpp"
#include "Game/Gravity/PlanetGravityManager.hpp"
#include "Game/LiveActor/AllLiveActorGroup.hpp"
#include "Game/LiveActor/ClippingDirector.hpp"
#include "Game/LiveActor/LiveActorGroupArray.hpp"
#include "Game/LiveActor/MessageSensorHolder.hpp"
#include "Game/LiveActor/MirrorCamera.hpp"
#include "Game/LiveActor/SensorHitChecker.hpp"
#include "Game/LiveActor/ShadowController.hpp"
#include "Game/LiveActor/ShadowSurfaceDrawer.hpp"
#include "Game/LiveActor/ShadowVolumeDrawer.hpp"
#include "Game/LiveActor/VolumeModelDrawer.hpp"
#include "Game/Map/Air.hpp"
#include "Game/Map/CollisionDirector.hpp"
#include "Game/Map/LightDirector.hpp"
#include "Game/Map/NamePosHolder.hpp"
#include "Game/Map/OceanHomeMapCtrl.hpp"
#include "Game/Map/PlanetMapCreator.hpp"
#include "Game/Map/QuakeEffectGenerator.hpp"
#include "Game/Map/RaceManager.hpp"
#include "Game/Map/AwakeControllerHolder.hpp"
#include "Game/Map/SleepControllerHolder.hpp"
#include "Game/Map/StageSwitch.hpp"
#include "Game/Map/SunshadeMapHolder.hpp"
#include "Game/Map/SwitchWatcherHolder.hpp"
#include "Game/Map/WaterAreaHolder.hpp"
#include "Game/Map/WaterPlant.hpp"
#include "Game/MapObj/AirBubbleHolder.hpp"
#include "Game/MapObj/ArrowSwitchMultiHolder.hpp"
#include "Game/MapObj/BigBubbleHolder.hpp"
#include "Game/MapObj/BigFanHolder.hpp"
#include "Game/MapObj/ChipHolder.hpp"
#include "Game/MapObj/ClipAreaDropHolder.hpp"
#include "Game/MapObj/ClipAreaHolder.hpp"
#include "Game/MapObj/ClipFieldFillDraw.hpp"
#include "Game/MapObj/CoinHolder.hpp"
#include "Game/MapObj/CoinRotater.hpp"
#include "Game/MapObj/EarthenPipe.hpp"
#include "Game/MapObj/ElectricRailHolder.hpp"
#include "Game/MapObj/FallOutFieldDraw.hpp"
#include "Game/MapObj/FirePressureBulletHolder.hpp"
#include "Game/MapObj/GCapture.hpp"
#include "Game/MapObj/MapPartsRailGuideHolder.hpp"
#include "Game/MapObj/MiniatureGalaxyHolder.hpp"
#include "Game/MapObj/Note.hpp"
#include "Game/MapObj/PurpleCoinHolder.hpp"
#include "Game/MapObj/SpiderThread.hpp"
#include "Game/MapObj/SpinDriverPathDrawer.hpp"
#include "Game/MapObj/StarPieceDirector.hpp"
#include "Game/MapObj/WarpPod.hpp"
#include "Game/MapObj/WaterPressureBulletHolder.hpp"
#include "Game/NPC/EventDirector.hpp"
#include "Game/NPC/MiiFaceIconHolder.hpp"
#include "Game/NPC/MiiFacePartsHolder.hpp"
#include "Game/NPC/NPCDirector.hpp"
#include "Game/NPC/TalkDirector.hpp"
#include "Game/NameObj/MovementOnOffGroupHolder.hpp"
#include "Game/NameObj/NameObjExecuteHolder.hpp"
#include "Game/NameObj/NameObjGroup.hpp"
#include "Game/Player/GroupChecker.hpp"
#include "Game/Player/MarioHolder.hpp"
#include "Game/Player/PlayerEvent.hpp"
#include "Game/Ride/FluffWind.hpp"
#include "Game/Ride/PlantLeaf.hpp"
#include "Game/Ride/PlantStalk.hpp"
#include "Game/Ride/SwingRope.hpp"
#include "Game/Ride/Trapeze.hpp"
#include "Game/Scene/PlacementStateChecker.hpp"
#include "Game/Scene/SceneDataInitializer.hpp"
#include "Game/Scene/SceneNameObjMovementController.hpp"
#include "Game/Scene/ScenePlayingResult.hpp"
#include "Game/Scene/StageDataHolder.hpp"
#include "Game/Scene/StopSceneController.hpp"
#include "Game/Screen/BloomEffect.hpp"
#include "Game/Screen/BloomEffectSimple.hpp"
#include "Game/Screen/CaptureScreenDirector.hpp"
#include "Game/Screen/CenterScreenBlur.hpp"
#include "Game/Screen/CinemaFrame.hpp"
#include "Game/Screen/CometRetryButton.hpp"
#include "Game/Screen/DepthOfFieldBlur.hpp"
#include "Game/Screen/GalaxyMapController.hpp"
#include "Game/Screen/GalaxyNamePlateDrawer.hpp"
#include "Game/Screen/GameSceneLayoutHolder.hpp"
#include "Game/Screen/HeatHazeEffect.hpp"
#include "Game/Screen/ImageEffectSystemHolder.hpp"
#include "Game/Screen/InformationObserver.hpp"
#include "Game/Screen/LensFlare.hpp"
#include "Game/Screen/MoviePlayerSimple.hpp"
#include "Game/Screen/MoviePlayingSequence.hpp"
#include "Game/Screen/OdhConverter.hpp"
#include "Game/Screen/PlayerActionGuidance.hpp"
#include "Game/Screen/SceneWipeHolder.hpp"
#include "Game/Screen/ScreenAlphaCapture.hpp"
#include "Game/Screen/ScreenBlurEffect.hpp"
#include "Game/Screen/StaffRoll.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"
#include "Game/Util/FurCtrl.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ShareUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"

SceneObjHolder::SceneObjHolder() {
    for (int i = 0; i < SceneObj_NumMax; i++) {
        mObjs[i] = nullptr;
    }
}

NameObj* SceneObjHolder::create(int id) {
    NameObj* pObj = mObjs[id];

    if (pObj != nullptr) {
        return pObj;
    }

    pObj = newEachObj(id);
    pObj->initWithoutIter();

    mObjs[id] = pObj;

    return pObj;
}

NameObj* SceneObjHolder::getObj(int id) const {
    return mObjs[id];
}

bool SceneObjHolder::isExist(int id) const {
    return mObjs[id] != nullptr;
}

// NONMATCHING: 21 cases (ScenePlayingResult, MoviePlayerSimple, OdhConverter, Kameck*, BlueChip/YellowChip, ...) have no
// SMG2 SceneObj id in the enum yet (commented out); switch layout differs.
NameObj* SceneObjHolder::newEachObj(int id) {
    switch (id) {
    case SceneObj_SensorHitChecker:
        return new SensorHitChecker("センサー当たり");
    case SceneObj_CollisionDirector:
        return new CollisionDirector();
    case SceneObj_ClippingDirector:
        return new ClippingDirector();
    case SceneObj_DemoDirector:
        return new DemoDirector("デモ指揮");
    case SceneObj_EventDirector:
        return new EventDirector();
    case SceneObj_EffectSystem:
        return new EffectSystem("エフェクトシステム", true);
    case SceneObj_LightDirector:
        return new LightDirector();
    case SceneObj_SceneDataInitializer:
        return new SceneDataInitializer();
    case SceneObj_StageDataHolder:
        return new StageDataHolder(MR::getCurrentStageName(), 0, true);
    case SceneObj_MessageSensorHolder:
        return new MessageSensorHolder("システム汎用センサー");
    case SceneObj_StageSwitchContainer:
        return new StageSwitchContainer();
    case SceneObj_SwitchWatcherHolder:
        return new SwitchWatcherHolder();
    case SceneObj_AwakeControllerHolder:
        return new AwakeControllerHolder();
    case SceneObj_AreaObjContainer:
        return new AreaObjContainer("エリアオブジェクトコンテナ管理");
    case SceneObj_LiveActorGroupArray:
        return new LiveActorGroupArray("オブジェクトグループ");
    case SceneObj_MovementOnOffGroupHolder:
        return new MovementOnOffGroupHolder("Movementグループ管理");
    case SceneObj_CaptureScreenActor:
        return new CaptureScreenActor(MR::DrawType_CaptureScreenIndirect, "Indirect");
    case SceneObj_AudCameraWatcher:
        return new AudCameraWatcher();
    case SceneObj_AudEffectDirector:
        return new AudEffectDirector();
    case SceneObj_AudBgmConductor:
        return new AudBgmConductor();
    case SceneObj_ActionSoundSystem:
        return new ActionSoundSystem("アクションサウンドシステム");
    case SceneObj_MarioHolder:
        return new MarioHolder();
    case SceneObj_MirrorCamera:
        return new MirrorCamera("鏡用カメラ");
    case SceneObj_CameraContext:
        return new CameraContext();
    case SceneObj_NameObjGroup:
        return new NameObjGroup("IgnorePauseNameObj", 16);
    case SceneObj_TalkDirector:
        return new TalkDirector("会話ディレクター");
    case SceneObj_EventSequencer:
        return new EventSequencer();
    case SceneObj_StopSceneController:
        return new StopSceneController();
    case SceneObj_SceneNameObjMovementController:
        return new SceneNameObjMovementController();
    case SceneObj_ImageEffectSystemHolder:
        return new ImageEffectSystemHolder();
    case SceneObj_BloomEffect:
        return new BloomEffect("ブルーム");
    case SceneObj_BloomEffectSimple:
        return new BloomEffectSimple();
    case SceneObj_ScreenBlurEffect:
        return new ScreenBlurEffect("画面ブラー");
    case SceneObj_DepthOfFieldBlur:
        return new DepthOfFieldBlur("被写界深度ブラー");
    case SceneObj_PostFog:
        return new PostFog("あとづけフォグ");
    case SceneObj_PostOutline:
        return new PostOutline("輪郭抽出");
    case SceneObj_SceneWipeHolder:
        return new SceneWipeHolder();
    case SceneObj_PlayerActionGuidance:
        return new PlayerActionGuidance();
    case SceneObj_LensFlareDirector:
        return new LensFlareDirector();
    case SceneObj_FurDrawManager:
        return new FurDrawManager(64);
    case SceneObj_PlacementStateChecker:
        return new PlacementStateChecker("オブジェクト配置状態の監視");
    case SceneObj_NamePosHolder:
        return new NamePosHolder();
    case SceneObj_NPCDirector:
        return new NPCDirector();
    case SceneObj_ResourceShare:
        return new ResourceShare();
    case SceneObj_InformationObserver:
        return new InformationObserver();
    case SceneObj_CenterScreenBlur:
        return new CenterScreenBlur();
    case SceneObj_SwitchingScreenEffect:
        return new SwitchingScreenEffect("スイッチ切り替えエフェクト");
    case SceneObj_TimeStopScreenCaptureTexture:
        return new TimeStopScreenCaptureTexture("時間停止画面取り込みテクスチャ");
    case SceneObj_TimeStopScreenEffect:
        return new TimeStopScreenEffect("時間停止画面エフェクト");
    case SceneObj_SwingActionWatcher:
        return new SwingActionWatcher("スイングアクション監視者");
    case SceneObj_WorldMapOwner:
        return new WorldMapOwner();
    case SceneObj_WorldMapResultCommand:
        return new WorldMapResultCommand();
    case SceneObj_ScoreAttackAccessor:
        return new ScoreAttackAccessor();
    case SceneObj_ScoreResultKeeper:
        return new ScoreResultKeeper();
    case SceneObj_Unk37:
        return new PostmanEventSystem();
    case SceneObj_SupportPlayDirector:
        return new SupportPlayDirector();
    case SceneObj_AllLiveActorGroup:
        return new AllLiveActorGroup();
    case SceneObj_CameraDirector:
        return new CameraDirector("カメラ管理");
    case SceneObj_PlanetGravityManager:
        return new PlanetGravityManager("重力");
    case SceneObj_BaseMatrixFollowTargetHolder:
        return new BaseMatrixFollowTargetHolder("行列追随先リスト", 256, 256);
    case SceneObj_GameSceneLayoutHolder:
        return new GameSceneLayoutHolder();
    case SceneObj_KabokuriFireHolder:
        return new KabokuriFireHolder();
    case SceneObj_TakoHeiInkHolder:
        return new TakoHeiInkHolder();
    case SceneObj_SwingRopeGroup:
        return new SwingRopeGroup("スイングロープ描画");
    case SceneObj_CoinHolder:
        return new CoinHolder("コイン管理");
    case SceneObj_PurpleCoinHolder:
        return new PurpleCoinHolder();
    case SceneObj_CoinRotater:
        return new CoinRotater("コイン回転管理");
    case SceneObj_AirBubbleHolder:
        return new AirBubbleHolder("空気アワ管理");
    case SceneObj_StarPieceDirector:
        return new StarPieceDirector("スターピース指揮");
    case SceneObj_BegomanAttackPermitter:
        return new BegomanAttackPermitter("ベーゴマン攻撃許可者");
    case SceneObj_BigFanHolder:
        return new BigFanHolder();
    case SceneObj_KarikariDirector:
        return new KarikariDirector("カリカリディレクター");
    case SceneObj_ShadowControllerHolder:
        return new ShadowControllerHolder();
    case SceneObj_ShadowColorSetter:
        return new ShadowColorSetter("影色セット");
    case SceneObj_ShadowVolumeDrawInit:
        return new ShadowVolumeDrawInit();
    case SceneObj_ShadowSurfaceDrawInit:
        return new ShadowSurfaceDrawInit("水面影描画初期化");
    case SceneObj_PlantStalkDrawInit:
        return new PlantStalkDrawInit("植物の茎描画初期化");
    case SceneObj_PlantLeafDrawInit:
        return new PlantLeafDrawInit("描画初期化[植物の葉]");
    case SceneObj_TrapezeRopeDrawInit:
        return new TrapezeRopeDrawInit("空中ブランコロープ描画");
    case SceneObj_SpinDriverPathDrawInit:
        return new SpinDriverPathDrawInit();
    case SceneObj_NoteGroup:
        return new NoteGroup();
    case SceneObj_ClipAreaHolder:
        return new ClipAreaHolder("クリップエリアホルダー");
    case SceneObj_SnowStepHolder:
        return new SnowStepHolder();
    case SceneObj_ClipAreaDropHolder:
        return new ClipAreaDropHolder();
    case SceneObj_FallOutFieldDraw:
        return new FallOutFieldDraw("クリップエリア描画[抜き]");
    case SceneObj_ClipFieldFillDraw:
        return new ClipFieldFillDraw("クリップエリア描画[塗りつぶし]");
    case SceneObj_LightFruitClipArea:
        return new LightFruitClipArea();
    case SceneObj_ScreenAlphaCapture:
        return new ScreenAlphaCapture("アルファテクスチャ取り込み");
    case SceneObj_MapPartsRailGuideHolder:
        return new MapPartsRailGuideHolder();
    case SceneObj_GCapture:
        return new GCapture("Gキャプチャー");
    case SceneObj_SupportTico:
        return new SupportTico("サポートチコ");
    case SceneObj_NameObjExecuteHolder:
        return new NameObjExecuteHolder(4096);
    case SceneObj_ElectricRailHolder:
        return new ElectricRailHolder("電撃レール保持");
    case SceneObj_SpiderThread:
        return new SpiderThread("クモの巣");
    case SceneObj_QuakeEffectGenerator:
        return new QuakeEffectGenerator();
    case SceneObj_KoopaBattleAccessor:
        return new KoopaBattleAccessor();
    case SceneObj_HeatHazeDirector:
        return new HeatHazeDirector("陽炎制御");
    case SceneObj_ChipHolder:
        return new ChipHolder("イエローーチップホルダー", 1);
    case SceneObj_SpinCloudBlockGroup:
        return new SpinCloudBlockGroup();
    case SceneObj_EarthenPipeMediator:
        return new EarthenPipeMediator();
    case SceneObj_PetitPorterMediator:
        return new PetitPorterMediator();
    case SceneObj_WaterAreaHolder:
        return new WaterAreaHolder();
    case SceneObj_WaterPlantDrawInit:
        return new WaterPlantDrawInit();
    case SceneObj_RaceManager:
        return new RaceManager();
    case SceneObj_GroupCheckManager:
        return new GroupCheckManager("属性グループマネージャー");
    case SceneObj_WaterPressureBulletHolder:
        return new WaterPressureBulletHolder("ウォータープレッシャー玉ホルダ−");
    case SceneObj_FirePressureBulletHolder:
        return new FirePressureBulletHolder("ファイアプレッシャー玉ホルダ−");
    case SceneObj_SunshadeMapHolder:
        return new SunshadeMapHolder();
    case SceneObj_MiiFacePartsHolder:
        return new MiiFacePartsHolder(128);
    case SceneObj_MiiFaceIconHolder:
        return new MiiFaceIconHolder(16, "Miiアイコン保持管理");
    case SceneObj_FluffWindHolder:
        return new FluffWindHolder();
    case SceneObj_GalaxyNamePlateDrawer:
        return new GalaxyNamePlateDrawer();
    case SceneObj_CinemaFrame:
        return new CinemaFrame(true);
    case SceneObj_PlayerFollowPass:
        return new PlayerFollowPass();
    case SceneObj_InfernoMarioManager:
        return new InfernoMarioManager("インフェルノマリオ管理");
    case SceneObj_PlanetMapCreator:
        return new PlanetMapCreator("惑星クリエイタ");
    case SceneObj_ProductMapCreator:
        return new ProductMapCreator("惑星クリエイタ");
    case SceneObj_WarpPodMgr:
        return new WarpPodMgr("ワープポッド管理局");
    case SceneObj_PriorDrawAirHolder:
        return new PriorDrawAirHolder();
    case SceneObj_StaffRoll:
        return new StaffRoll();
    case SceneObj_LiveActorGroup:
        return new LiveActorGroup("マップパーツ", 0x200);
    case SceneObj_StarLightAreaHolder:
        return new StarLightAreaHolder("スターライトエリアホルダー");
    case SceneObj_ChangeObjHolder:
        return new ChangeObjHolder("切り替えオブジェホルダー");
    case SceneObj_DrillHoleHolder:
        return new DrillHoleHolder("ドリルの穴保持");
    case SceneObj_CollectTicoHolder:
        return new CollectTicoHolder();
    case SceneObj_IceMeteorHolder:
        return new IceMeteorHolder("アイスメテオホルダー");
    case SceneObj_StoryBookDemo:
        return new StoryBookDemo();
    case SceneObj_PopoStickingManager:
        return new PopoStickingManager("ポポくっつき管理");
    case SceneObj_KoopaJrCastleAccessor:
        return new KoopaJrCastleAccessor();
    case SceneObj_KoopaJrCastleWindHolder:
        return new KoopaJrCastleWindHolder();
    case SceneObj_GliderShooterBulletHolder:
        return new GliderShooterBulletHolder();
    case SceneObj_GhostAttackAccessor:
        return new GhostAttackAccessor();
    case SceneObj_GliBirdTutorial:
        return new GliBirdTutorial();
    case SceneObj_PauseBlur:
        return new PauseBlur("PauseBlur");
    case SceneObj_StarPieceFlowerController:
        return new StarPieceFlowerController("スターピース花コントローラー");
    case SceneObj_ExterminationWoodBoxFlagHolder:
        return new ExterminationWoodBoxFlagHolder("木箱全滅フラグホルダー");
    case SceneObj_MarioFacePlanetSwitch:
        return new MarioFacePlanetSwitch("マリオ顔惑星スイッチ");
    case SceneObj_MarioFaceShipAttentionConversationDirector:
        return new MarioFaceShipAttentionConversationDirector("注目会話デモ指示者");
    case SceneObj_MarioFaceShipNpcHolder:
        return new MarioFaceShipNpcHolder("Npc紹介デモ参加者ホルダー");
    case SceneObj_MarioFaceShipNormalConversationDirector:
        return new MarioFaceShipNormalConversationDirector("会話デモ指示者");
    case SceneObj_MarioFaceShipEventDataHolder:
        return new MarioFaceShipEventDataHolder("マリオ顔惑星イベントデータ所持者");
    case SceneObj_WiiMessageBoardMailSender:
        return new WiiMessageBoardMailSender();
    default:
        return nullptr;
    }
}

namespace MR {
#pragma dont_inline on
    NameObj* createSceneObj(int id) {
        return getSceneObjHolder()->create(id);
    }
#pragma dont_inline reset

#pragma dont_inline on
    bool isExistSceneObj(int id) {
        GameSystemSceneController* pSceneController = SingletonHolder< GameSystem >::sInstance->mSceneController;

        if (pSceneController == nullptr) {
            return false;
        }

        if (pSceneController->isExistSceneObjHolder()) {
            return MR::getSceneObjHolder()->isExist(id);
        }

        return false;
    }
#pragma dont_inline reset

    SceneObjHolder* getSceneObjHolder() {
        return SingletonHolder< GameSystem >::get()->mSceneController->getSceneObjHolder();
    }
};  // namespace MR
