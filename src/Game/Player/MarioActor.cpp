#define XJOINT_TRANSFORM_SCALE_NOINLINE
#define COLOR8_GX_CTOR_INLINE
#define JGEOMETRY_OUTLINE_MATRIX_SET_DIRECTION  // SMG2: TRotation3 setX/Y/ZDir(const TVec3f&) out of line in this TU
#include "Game/NPC/EventDirector.hpp"
#define JGEOMETRY_VEC3_SUB_NEG_NOINLINE  // SMG2: negateInternal/sub out of line in this TU
#define MARIO_ACTOR_DEFINE_NERVES  // SMG2: NrvMarioActor instances live only in this TU
#include "Game/Player/MarioActor.hpp"
#ifdef GALAXY_NATIVE
// OdysseyMovement mod (docs/dev/ODYSSEY_MOVEMENT.md): the Spin button's moves.
#include <cstdio>
#include <cstdlib>
#include <petari/odyssey_move.hpp>
#include "Game/Player/MarioCappy.hpp"
extern "C" bool petari_mod_enabled(int mod);
#endif
#include <revolution/gd.h>
#include "Game/Util.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Animation/BvaPlayer.hpp"
#include "Game/Animation/BtpPlayer.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/Enemy/KarikariDirector.hpp"
#include "Game/Enemy/Popo.hpp"
#include "Game/Gravity/GravityInfo.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Map/WaterInfo.hpp"
#include "Game/Player/J3DModelX.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Player/MarioEffect.hpp"
#include "Game/Player/MarioHolder.hpp"
#include "Game/Player/MarioMessenger.hpp"
#include "Game/Player/MarioNullBck.hpp"
#include "Game/Player/MarioParts.hpp"
#include "Game/Player/MarioShadow.hpp"
#include "Game/Player/MarioWallShadow.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Player/MarioSwim.hpp"
#include "Game/Player/ModelHolder.hpp"
#include "Game/Player/RushEndInfo.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/GameSceneLayoutHolder.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/Util/FootPrint.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include <JSystem/JKernel/JKRHeap.hpp>
#include <JSystem/JUtility/JUTVideo.hpp>
#include "Game/MapObj/BlackHole.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Player/MarioState.hpp"
#include "Game/Player/MarioMtxPos.hpp"
#include "Game/Player/MarioYoshiBalloon.hpp"
#include "Game/Player/MarioSlider.hpp"
#include "Game/Player/YoshiTongueAction.hpp"
#include "Game/MapObj/SpinCloudBlock.hpp"
#include "Game/Player/MarioHang.hpp"
#include "JSystem/JUtility/JUTVideo.hpp"
#include <cstring>
#include "Game/LiveActor/DisplayListMaker.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/MapObj/CollectCounter.hpp"
#include "Game/MapObj/IceStep.hpp"
#include "Game/Player/DLchanger.hpp"
#include "Game/Player/DrawAdaptor.hpp"
#include "Game/Player/TornadoMario.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/SchedulerUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>
#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>
#include <JSystem/J3DGraphBase/J3DMaterial.hpp>
#include <JSystem/J3DGraphBase/J3DTexture.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <JSystem/JUtility/JUTNameTab.hpp>
#include <JSystem/JUtility/JUTTexture.hpp>
#include <revolution/gd/GDBase.h>
#include "Game/System/ResourceHolder.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include "Game/Player/MarioFpView.hpp"
#include "Game/Util/GravityUtil.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>
#include "Game/Player/MatrixControl.hpp"
#include "Game/Util/HashUtil.hpp"
#include <revolution/types.h>
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Player/FireMarioBall.hpp"
#include "Game/Player/JetTurtleShadow.hpp"
#include "Game/Util/LightUtil.hpp"
#include "Game/Player/MarioFlip.hpp"
#include "Game/Player/MarioStun.hpp"
#include "Game/Player/MarioDamage.hpp"
#include "Game/Player/MarioModule.hpp"
#include "Game/Player/MarioWait.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioFoo.hpp"
#include "Game/Player/MarioRabbit.hpp"
#include "Game/Player/MarioWall.hpp"
#include <JSystem/J3DGraphBase/J3DSys.hpp>
#include "Game/Screen/FullScreenBlur.hpp"
#include "Game/Util/DirectDraw.hpp"
#include "Game/Util/FurMulti.hpp"
#include <JSystem/JGeometry/TBox.hpp>
#include <new>
#include <revolution/gd.h>
#include <revolution/gx/GXEnum.h>
#include <revolution/gx/GXFrameBuf.h>
#include <revolution/gx/GXPixel.h>
#include <revolution/os.h>
#include "Game/Player/MarioSearchLight.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/XanimeResource.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Effect/MultiEmitter.hpp"
#include "Game/Effect/ParticleEmitter.hpp"
#include "Game/LiveActor/EffectKeeper.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include <JSystem/JParticle/JPAEmitter.hpp>
#include <JSystem/JParticle/JPAMath.hpp>
#include <cstdio>
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/gx/GXCull.h>
#include <revolution/gx/GXTev.h>
#include <revolution/mtx.h>
#include "Game/Player/MarioTeresa.hpp"
#include "Game/MapObj/BigFanHolder.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Player/YoshiTongueTarget.hpp"
#include "Game/Player/MarioAllModel.hpp"
#include "Game/Player/MarioHairAndHatModel.hpp"
#include "Game/Player/CloudMarioHat.hpp"
#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Player/MarioSkate.hpp"
#include "Game/Player/MarioFireDance.hpp"
#include "Game/MapObj/SpinCloudBlock.hpp"
#include "Game/Animation/BckCtrl.hpp"
#include "JSystem/J3DGraphBase/J3DPacket.hpp"

// ---- enum names missing from the SMG2 ActorSensorUtil.hpp ----
static const s32 ACTMES_ATTACKDROP = 0x25;  // Petari value, SMG2 unverified
static const s32 ACTMES_BILLIARDJUMP = 0x2E;  // Petari value, SMG2 unverified
static const s32 ACTMES_ENEMY_ATTACK_EXTRA_DAMAGE = 0x5F;  // Petari value, SMG2 unverified
static const s32 ACTMES_ENEMY_ATTACK_FIREBACK = 0x57;  // Petari value, SMG2 unverified
static const s32 ACTMES_ENEMY_ATTACK_FLIP_VERYWEAK = 0x5A;
static const s32 ACTMES_ENEMY_ATTACK_HEATBEAM = 0x5B;  // Petari value, SMG2 unverified
static const s32 ACTMES_FORCE_KILL = 0x71;  // Petari value, SMG2 unverified
static const s32 ACTMES_GROUP_SHOW = 0x6B;  // Petari value, SMG2 unverified
static const s32 ACTMES_IS_BROKEN = 0x41;  // Petari value, SMG2 unverified
static const s32 ACTMES_IS_PULL_ENABLE = 0x88;  // Petari value, SMG2 unverified
static const s32 ACTMES_IS_RUSH_REQUEST = 0x99;  // Petari value, SMG2 unverified
static const s32 ACTMES_PUDDLE_TOUCH_GROUND = 0x47;  // Petari value, SMG2 unverified
static const s32 ACTMES_PUSH_FORCE = 0x2A;  // Petari value, SMG2 unverified
static const s32 ACTMES_REFLECT = 0x3F;  // Petari value, SMG2 unverified
static const s32 ACTMES_REFLECT_V = 0x40;  // Petari value, SMG2 unverified
static const s32 ACTMES_REQUEST_CLAP = 0xB7;  // Petari value, SMG2 unverified
static const s32 ACTMES_RUSH_FORCE_CANCEL = 0x94;  // Petari value, SMG2 unverified
static const s32 ACTMES_RUSH_PLAYER_TOUCH = 0x4E;
static const s32 ACTMES_SHAKE_OFF_BEE = 0x27;  // Petari value, SMG2 unverified
static const s32 ACTMES_START_DEMO = 0x6F;  // Petari value, SMG2 unverified
static const s32 ACTMES_TAKE_TOUCH = 0x28;  // Petari value, SMG2 unverified
static const s32 ACTMES_TERESA_PLAYER_TOUCH = 0x44;  // Petari value, SMG2 unverified
static const s32 ACTMES_TORNADO_STORM_RANGE = 0x32;  // Petari value, SMG2 unverified
static const s32 ATYPE_COCO_NUT_BIND = 0x71;  // Petari value, SMG2 unverified
static const s32 ATYPE_COINTHROW = 0x4E;  // Petari value, SMG2 unverified
static const s32 ATYPE_G_CAPTURE_BIND = 0x68;  // Petari value, SMG2 unverified
static const s32 ATYPE_NOKONOKO = 0x22;  // Petari value, SMG2 unverified
static const s32 ATYPE_SPRING_ATTACKER_BIND = 0x69;  // Petari value, SMG2 unverified
static const s32 ATYPE_TRANSFERABLE_BINDER = 0x62;  // Petari value, SMG2 unverified

// ---- MarioActor.cpp ----

void MarioActor_FORCE_MATCH_SDATA2() {
    (void)1000.0f;
    (void)360.0f;
    (void)100.0f;
    (void)40.0f;
    (void)60.0f;
    (void)70.0f;
    (void)0.349999994f;
    (void)1.79999995f;
    (void)52.5099983f;
    (void)39.5800018f;
    (void)76.3300018f;
    (void)15.6899996f;
    (void)88.9899979f;
    (void)35.0f;
    (void)-70.0f;
    (void)-0.99000001f;
    (void)90.0f;
    (void)180.0f;
    (void)57.2957764f;
    (void)0.00999999978f;
    (void)0.785398185f;
    (void)0.00100000005f;
    (void)0.100000001f;
    (void)-0.0f;
    (void)10.0f;
    (void)20.0f;
    (void)5.0f;
    (void)-5.0f;
    (void)-40.0f;
    (void)200.0f;
    (void)6.0f;
    (void)0.75f;
    (void)500.0f;
    (void)700.0f;
    (void)140.0f;
    (void)150.0f;
    (void)0.200000003f;
    (void)0.800000012f;
    (void)15.0f;
    (void)0.0199999996f;
    (void)80.0f;
    (void)120.0f;
    (void)2607.59448f;
    (void)160.0f;
    (void)0.150000006f;
    (void)30.0f;
    (void)-10.0f;
    (void)0.899999976f;
    (void)0.52359879f;
}


template void JGeometry::SMatrix34C< f32 >::set(const f32 (*)[4]) NO_INLINE;


void MarioActor_DUMMY() {
    (void)JGeometry::TUtil< f32 >::acos(1.0f);
}


bool gIsLuigi;

static f32 BASE_ROTATION = 0.0f;


namespace MarioAccess {
    LiveActor* getPlayerOrYoshiActor() {
        return MR::getMarioHolder()->getMarioActor()->fn_803BECC0();
    }

    const JUTTexture* getShadowTexture() {
        if (!MR::isExistSceneObj(0x15)) return 0;
        if (MR::getMarioHolder()->mActor == nullptr) return 0;
        MarioActor* actor = MR::getMarioHolder()->getMarioActor();
        if (actor != nullptr && actor->mCollisionShadow580 != nullptr) {
            return actor->mCollisionShadow580->getTexture();
        }
        return 0;
    }
}

static u8 sBlinkStates[] = {0, 1, 2, 2, 2, 2, 1, 1, 0, 0};

struct MaterialEffectEntry {
    struct Options {
        u8 mSelectionOptions;
        u8 mReserved1;
        u8 mReserved2;
        u8 mReserved3;
    };

    /* 0x00 */ const char* mName;
    /* 0x04 */ Options mFlag;
    /* 0x08 */ const char* mEffects[8];
    /* 0x28 */ const char* mCurrent;
    /* 0x2C */ MultiEmitter* mEmitter;
};


struct SmokeEffectEntry {
    struct TypeCode {
        u8 mCode;
        u8 mReserved1;
        u8 mReserved2;
        u8 mReserved3;
    };

    struct Flags {
        u8 mAttachment;
        u8 mHostFollowFlags;
        u8 mPlacementFlags;
        u8 mReserved;
    };

    /* 0x00 */ const char* mName;
    /* 0x04 */ TypeCode mType;
    /* 0x08 */ f32 mScale;
    /* 0x0C */ f32 mRate;
    /* 0x10 */ Flags mFlags;
    /* 0x14 */ u16 mInterval;
    /* 0x16 */ u16 mTimer;
    /* 0x18 */ u32 mHash;
    /* 0x1C */ MultiEmitter* mEmitter;
    /* 0x20 */ u8 mMaterial;
    /* 0x21 */ u8 _21[3];
};


MaterialEffectEntry cMaterialEffectTable[] = {
    {"属性尻ドロップ",
     {0, 0, 0, 0},
     {"HipDropSmoke", "HipDropWater", "HipDropFlower", "HipDropSand", "HipDropSnow", "MudCircle", "HoneyCircle", "MarioCloudLand"},
     nullptr,
     nullptr},
    {"属性ハチ風", {3, 0, 0, 0}, {"Hovering", "HoveringWater", "Hovering", "Hovering", "Hovering", "Hovering", "Hovering", "Hovering"}, nullptr, nullptr},
    {"属性スピン", {0, 0, 0, 0}, {nullptr, "WaterSpin", "FlowerSpin", "SandSpin", "SnowSpin", "MudCircle", "HoneyCircle", nullptr}, nullptr, nullptr},
    {"属性ステージイン",
     {0, 0, 0, 0},
     {"StageStartGroundSmoke", nullptr, nullptr, "StageStartGroundSand", nullptr, nullptr, nullptr, "MarioCloudLand"},
     nullptr,
     nullptr},
    {nullptr, {0, 0, 0, 0}, {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}, nullptr, nullptr}};


SmokeEffectEntry cSmokeTable[] = {{"共通着地普通", {8, 0, 0, 0}, 1.0f, 1.0f, {0, 0, 4, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通着地大", {3, 0, 0, 0}, 1.5f, 1.0f, {0, 0, 4, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通壁ジャンプ", {3, 0, 0, 0}, 1.0f, 1.0f, {0, 0, 3, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通砂煙レベル", {7, 0, 0, 0}, 0.65f, 1.0f, {0, 0, 0, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通壁手擦り", {1, 0, 0, 0}, 0.35f, 1.0f, {2, 1, 0, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通壁上昇", {1, 0, 0, 0}, 1.0f, 1.0f, {0, 0, 2, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通スリップ坂", {2, 0, 0, 0}, 0.35f, 1.0f, {0, 1, 0, 0}, 2, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通スリップ空転", {0, 0, 0, 0}, 1.0f, 1.0f, {0, 0, 0, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通スリップ坂制動", {2, 0, 0, 0}, 0.5f, 1.0f, {0, 0, 0, 0}, 2, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通壁ヒット着地", {3, 0, 0, 0}, 1.0f, 1.0f, {0, 0, 4, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通ダメージ着地", {3, 0, 0, 0}, 1.0f, 1.0f, {0, 0, 4, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通地上スピン", {3, 0, 0, 0}, 0.9f, 1.0f, {0, 0, 4, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通ブレーキ", {2, 0, 0, 0}, 0.65f, 1.0f, {0, 1, 0, 0}, 15, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通ハイジャンプ", {2, 0, 0, 0}, 0.65f, 1.0f, {0, 1, 0, 0}, 6, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通ブラックホール", {1, 0, 0, 0}, 0.65f, 1.0f, {3, 3, 0, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通引き戻し着地", {3, 0, 0, 0}, 1.2f, 1.0f, {0, 0, 4, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通跳躍", {8, 0, 0, 0}, 1.0f, 1.0f, {0, 0, 12, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通ひこうき雲", {3, 0, 0, 0}, 0.3f, 0.2f, {0, 1, 0, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通ひこうきブースト", {0, 0, 0, 0}, 2.0f, 1.0f, {0, 0, 0, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {nullptr, {0, 0, 0, 0}, 0.0f, 0.0f, {0, 0, 0, 0}, 0, 0, 0, nullptr, 0, {0, 0, 0}}};

MarioActor::MarioActor(const char* pName) : LiveActor(pName) {
    mYoshiHolder = nullptr;
    initMember();
    mMario = new Mario(this);
    _930 = 0;
    _468 = 0;
    mHealth = GameSequenceFunction::getResultPlayerHealth();
    mMaxHealth = mHealth > 3 ? 6 : 3;
    if (MR::isGalaxyDarkCometAppearInCurrentStage()) {
        mMaxHealth = 1;
        mHealth = 1;
    }
    init2D();
    _989 = 0;
    _41C = 0;
    _37C = 0;
    mLastRushEndFrameE1C = 0;
    _92C = 0;
    _38C = 0;
    _390 = 0;
    _394 = 0;
    _398 = 0;
    _3A8 = 0;
    _3AA = 0;
    _944 = 0;
    _946 = 0;
    _BC4 = 0;
    _948 = 0;
    _94C = 0;
    _94E = 0;
    _47C = 0;
    _3DC = 0;
    _3D8 = 0;
    _3DA = 0;
    _3AC = 0;
    _B94 = 0;
    _378 = 0;
    mSpinRadius = 0.0f;
    mSuperKinokoCollected = false;
    mPowerupCollected = false;
    mTransforming = false;
    _950 = 0;
    _974 = 0;
    mCounter6C8 = 0;
    _3B0 = 1.0f;
    mPressNormal6D8.zero();
    _3C1 = false;
    _46C = nullptr;
    _470 = nullptr;
    _474 = nullptr;
    _924 = nullptr;
    _928 = nullptr;
    _480 = false;
    _481 = 0;
    _482 = false;
    _483 = false;
    mThrowProbePosition76C.zero();
    _B92 = 0;
    mPlayerMode = 0;
    _3D6 = 0;
    _3D2 = 1;
    _3DF = 0;
    _3DE = 0;
    _EEB = true;
    _9D4 = nullptr;
    _9CC = 0.0f;
    _9D0 = 0.0f;
    _9D8.zero();
    mBeeWallWalk = 0;
    _9F2 = 0;
    _EA4 = false;
    _EA5 = false;
    _FCC = false;
    _FCD = false;
    mFlag1052 = 0;
    mDashWallHit1053 = 0;
    PSMTXIdentity(mRemoteDemoMtx4E4);
    mWallShadow574 = new MarioWallShadow();
    mBlackHole = nullptr;
    mBlackHolePosition528.zero();
    mBlackHoleRotateAxis.zero();
    mPosRelativeToBlackHole.zero();
    _B48 = nullptr;
    _934 = false;
    _7E2 = 0;
    _EF6 = 0;
    _424 = nullptr;
    _4A4 = nullptr;
    mSensor794 = nullptr;
    _6D0 = false;
    _EFC = 0;
    _EF8 = 0;
    mBlendMtxTimer = 0;
    _EA0 = 1.0f;
    mRushFront6E8.zero();
    _3D0 = 0;
    _7DE = 0;
    _3C0 = false;
    _B90 = false;
    _B91 = false;
    _39D = 0;
    _B96 = 0;
    _EF0 = 0;
    _EF2 = 0;
    _EF4 = 0x78;
    _1AA = 0;
    _1AC = 0.0f;
    mSwimFogColor.set(0xFF, 0xFF, 0xFF, 0);
    mSwimFogDisabled = false;
    _988 = 0;
    mInputConfig = new MarioInputConfig();
    resetPadSwing();
    _F1C = 0x3C;
    _F1E = 0x3C;
    _F20 = false;
    _1E1 = false;
    _F24 = nullptr;
    _F28 = 0;
    PSMTXIdentity(mHandLMatrix);
    PSMTXIdentity(mHandRMatrix);
    PSMTXIdentity(mFootRMatrix);
    PSMTXIdentity(mSpine1Matrix);
    PSMTXIdentity(mCapMatrix);
    _1B4 = 0;
    _1C3 = false;
    _1E0 = false;
}


MarioActor::~MarioActor() {
    delete mMario;
}


// NONMATCHING: Verified equivalent string-pool offsets.
void MarioActor::init(const JMapInfoIter& rInfo) {
    if (!MR::isValidInfo(rInfo)) return;
    mInitializing90 = true;
    s32 initialAnimation = -1;
    fn_803BF760(rInfo, &initialAnimation);
    fn_803BF860();
    fn_803BF8A0();
    updateBaseScaleMtx();
    _A18 = mRotation;
    initDrawAndModel();
    mJointMatrices2C4 = new Mtx[MR::getJointNum(this)];
    MR::connectToScene(this, 0x24, 9, 0x15, 0x25);
    MR::initLightCtrlForPlayer(this);
    mMarioAnim = new MarioAnimator(this);
    mMarioEffect = new MarioEffect(this);
    mCollisionShadow580 = new CollisionShadow(100.0f, 40.0f);
    fn_803BF8F0();
    mMario->initAfterConst();
    fn_803BF950();
    initNerve(GET_NERVE(MarioActor, MarioActorNrvWait), 0);
    _FB4 = nullptr;
    _FB8 = 0;
    fn_803BF9D0();
    initEffect();
    MR::invalidateClipping(this);
    fn_803BFAE0();
    mLastMove.zero();
    _1FC = mPosition;
    calcCenterPos();
    if (MR::isStageEpilogueDemo() || MR::isStageMarioFaceShipAfterGrandStar() || MR::isStageMarioFaceShipAfterFinalKoopa()) {
        initSound(16, "MarioDemo", nullptr, TVec3f(0.0f));
    } else if (MR::isStageFileSelect()) {
        initSound(16, "MarioFileSelect", nullptr, TVec3f(0.0f));
    } else if (MR::isStageKoopaVs()) {
        initSound(16, "MarioKoopaStage", nullptr, TVec3f(0.0f));
    } else {
        initSound(16, "Mario", nullptr, TVec3f(0.0f));
    }
    MR::addActorToSoundObjHolder(this);
    initParts();
    fn_803BFB30();
    _33C = mPosition;
    _348.zero();
    _354.zero();
    setupSensors();
    MR::getMarioHolder()->setMarioActor(this);
    mMessenger568 = new MarioMessenger(getSensor("dummy"));
    fn_803BFBB0();
    fn_803BFC10();
    fn_803BFC90(initialAnimation);
    updateTransForCamera();
    fn_803BFCE0();
    fn_803BFE50();
    fn_803D95E0();
    _482 = true;
    appear();
    _482 = false;
    mFlagF18 = 0;
    mFlagF19 = 0;
    mFrontHistory = new MarioFrontHistory(mMario);
    mActionTable1080 = new MarioActionTable(this);
    mInitializing90 = false;
}


struct DUMMY {
    u8 _0[0x30];
};


void MarioActor::initAfterPlacement() {
    updateGravityVec(true, true);
    mMario->mAirGravityVec = mAppliedGravityVec;
    mMario->mHeadVec = -mAppliedGravityVec;
    mMario->_1FC = -mAppliedGravityVec;
    _2A0 = mMario->mHeadVec;
    _270 = _2A0;
    mVector60C = mAppliedGravityVec * -70.0f;
    calcCenterPos();
    MR::updateHitSensorsAll(this);
    _360 = getGravityVector();
    _1C0 = true;
    mGravityReferenceED0 = getGravityVector();
    updateCameraInfo();
    calcBaseFrontVec(-mAppliedGravityVec);
}


namespace MR {
    void showMaterial(J3DModel*, const char*);
};
  // namespace MR


  // namespace

namespace MR {
    void removeAllClingingKarikari();
    bool isSandyBossBGMPlaying();
    void adjustSandyBossFireMorphBGM(s32, s32);
    void endStarPointerMode(void*);
    void setPlayerFrontVec(const TVec3f&, s32);
}

namespace MR {
    void explainDrillIfAtFirst();
}

namespace MR {
    bool isCreatePlayerResourceHairAndHead();
}

namespace MR {
    bool isCreatePlayerResourceFire();
    bool isCreatePlayerResourceYoshi();
    bool isCreatePlayerResourceCloud();
    bool isCreatePlayerResourceRock();
}

namespace {
    inline void initExtraMtxBuffer(J3DModelX* pModel) {
        for (u32 i = 0; i < pModel->_DD; i++) {
            pModel->mExtraMtxBuffer[i] = new (0x20) Mtx[0xC00 / sizeof(Mtx)];
        }
    }
};

namespace {
    inline HitSensor* getDummySensor(MarioActor* pActor) {
        return pActor->getSensor("dummy");
    }
};

namespace {
    inline TVec3f subtractOffset(const TVec3f& rBase, const TVec3f& rOffset) {
        const TVec3f* pOffset = &rOffset;
        return rBase - *pOffset;
    }
};

namespace {
    inline f32 readSpinJitterBoundary(HitSensor* const& sensor) {
        // The final target jitter sample shares bytes with the following sensor pointer.
        f32 jitter;
        memcpy(&jitter, &sensor, sizeof(jitter));
        return jitter;
    }
}

namespace {

    void addMarioEffectAtJoint(LiveActor* pActor, const char* pEffectName, const char* pJointName, const char* pGroupName) NO_INLINE {
        MtxPtr mtx = pJointName != nullptr ? MR::getJointMtx(pActor, pJointName) : pActor->getBaseMtx();
        pActor->mEffectKeeper->registerEffect(pEffectName, mtx, &pActor->mScale, pGroupName, pJointName);
    }

};

namespace {
    inline J3DModelX* getModelX(LiveActor* pActor) {
        return static_cast< J3DModelX* >(MR::getJ3DModel(pActor));
    }

    inline void initExtraMtxBuffer(J3DModelX* pModel, s32 num) {
        for (u32 i = 0; i < pModel->_DD; i++) {
            pModel->mExtraMtxBuffer[i] = new (0x20) Mtx[num];
        }
    }
};

static f32 mSensorRadiusAttack = 120.0f;

static f32 mSensorRadiusTornadoAttack = 150.0f;

static f32 mSensorRadiusHipDropAttack = 80.0f;

static f32 mSensorRadiusClapCatch = 600.0f;

static f32 mSensorRadiusClapCatchSwim = 2000.0f;

static f32 mSensorRadiusTrampleAttack = 80.0f;

static f32 mSensorRadiusSpinStorm = 600.0f;

static f32 mSensorRadiusTornadoStorm = 1000.0f;

static f32 mSensorRadiusSpinPull = 450.0f;

static f32 mSensorRadiusSpinPullOnGround = 450.0f;

void MarioActor::initAfterOpeningDemo() {
    mMario->changeAnimationNonStop("ウォークイン");
    _37C = 0;
}

void MarioActor::draw() const {
    if (_B48 != nullptr) _B48->draw();
    if (!is481or482On() && (mMario->isPlayerModeYoshi() || (!mActorEffectState->mActive && !_1C3))) {
        mWallShadow574->draw(mMaskTextures[mRandomTextureIndex]);
        drawMarioModel();
        mMario->draw();
    }
}

#pragma push
#pragma global_optimizer off
void MarioActor::resetFog() {
    Color8 color(0, 0, 0, 0);
    _1A4 = 0.0f;
    _1A8 = 0;
    _1AA = 0;
}
#pragma pop


void MarioActor::exeWait() {
    if (_FB8 != 0) {
        _FB8--;
        if (_FB8 == 0) {
            setNerve(_FB4);
            _FB4 = nullptr;
        }
    }
}

void MarioActor::exeGameOver() {
    mVelocity.zero();
    calcHeadPos();
    updateRealMtx();

    if (getNerveStep() == 32) {
        _A6E = 0;
    }

    _46C = nullptr;

    PSMTXIdentity(mMarioAnim->_7C);

    damageDropThrowMemoSensor();

    if (!MR::isFirstStep(this)) {
        return;
    }

    MR::setCubeBgmChangeInvalid();
    MR::clearBgmQueue();

    if (MR::getPlayerLeft() == 0) {
        MR::startPlayerEvent("ゲームオーバー");
        changeGameOverAnimation();
    } else {
        switch (_39D) {
        case 2:
            MR::startPlayerEvent("ゴーストレース負け");
            break;

        case 1:
            MR::startPlayerEvent("レース負け");
            changeGameOverAnimation();
            break;

        case 5:
            MR::startPlayerEvent("レース負け");
            break;

        default:
            MR::startPlayerEvent("マリオダウン");
        }
    }

    MR::startGlobalEventCameraNoTarget("昇天カメラ");
    MR::startStarPointerModeDemoMarioDeath(this);
}

// NONMATCHING: verified equal string-pool base offsets.
void MarioActor::exeGameOverAbyss() {
    mVelocity = getLastMove() * 0.99f + mCameraBasisZ * 35.0f * 0.01f;
    mLastMove = mVelocity;

    if (!MR::isFirstStep(this)) {
        return;
    }

    MR::setCubeBgmChangeInvalid();
    MR::clearBgmQueue();
    MR::startGlobalEventCameraNoTarget("奈落カメラ");

    _F44 = false;

    if (MR::getPlayerLeft() == 0) {
        MR::startPlayerEvent("ゲームオーバー");
    } else {
        MR::startPlayerEvent("マリオ奈落");
    }

    MR::startStarPointerModeDemoMarioDeath(this);
    mMario->changeAnimationNonStop("奈落ダウン");

    _B90 = true;
}

void MarioActor::exeGameOverFire() {
    mVelocity.zero();
    calcHeadPos();
    updateRealMtx();

    if (getNerveStep() == 120) {
        _A6E = 0;
    }

    if (!MR::isFirstStep(this)) {
        return;
    }

    MR::setCubeBgmChangeInvalid();
    MR::clearBgmQueue();

    if (MR::getPlayerLeft() == 0) {
        MR::startPlayerEvent("ゲームオーバー");
        MR::startGlobalEventCameraNoTarget("昇天カメラ");
    } else {
        MR::startPlayerEvent("マリオ炎ダウン");
        MR::startGlobalEventCameraNoTarget("奈落カメラ");
    }

    MR::startStarPointerModeDemoMarioDeath(this);
}

void MarioActor::exeGameOverNonStop() {
    if (!MR::isFirstStep(this)) {
        return;
    }

    MR::setCubeBgmChangeInvalid();
    MR::clearBgmQueue();

    if (MR::getPlayerLeft() == 0) {
        MR::startPlayerEvent("ゲームオーバー");
        MR::startGlobalEventCameraNoTarget("昇天カメラ");
    } else {
        MR::startPlayerEvent("マリオ炎ダウン");
        MR::startGlobalEventCameraNoTarget("奈落カメラ");
    }

    MR::startStarPointerModeDemoMarioDeath(this);
}

void MarioActor::exeGameOverSink() {
    mVelocity.zero();
    calcHeadPos();
    updateRealMtx();

    if (getNerveStep() == 120) {
        _A6E = 0;
    }

    if (MR::isFirstStep(this)) {
        MR::setCubeBgmChangeInvalid();
        MR::clearBgmQueue();
        changeGameOverAnimation();

        if (MR::getPlayerLeft() == 0) {
            MR::startPlayerEvent("ゲームオーバー");
            MR::startGlobalEventCameraNoTarget("昇天カメラ");
        } else {
            MR::startPlayerEvent("マリオダウン");
            MR::startGlobalEventCameraNoTarget("奈落カメラ");
        }

        MR::startStarPointerModeDemoMarioDeath(this);

        if (mMario->_960 == 25 || mMario->_960 == 17) {
            playEffect("砂埋まり体");
            playEffect("砂埋まり手");
        }

        if (mMario->_960 == 31) {
            playEffect("泥埋まり体");
            playEffect("泥埋まり手");
        }
    }

    if (mMario->_960 == 31 || mMario->_960 == 18) {
        playSound("沼死亡", -1);

        if (getNerveStep() == 90) {
            playSound("声沼沈み死亡", -1);
        }
    } else {
        playSound("砂死亡", -1);

        if (getNerveStep() == 90) {
            playSound("声砂沈み死亡", -1);
        }
    }
}

void MarioActor::exeGameOverNoDemo() {
    if (MR::isFirstStep(this)) {
        MR::setCubeBgmChangeInvalid();
        MR::clearBgmQueue();
        if (MR::getPlayerLeft() == 0) {
            MR::startPlayerEvent("ゲームオーバー");
        } else {
            MR::startPlayerEvent("演出なし死");
            this->mHealth = 0;
        }
        MR::startStarPointerModeDemoMarioDeath(this);
    }
}

void MarioActor::exeTimeWait() {
    if (_FB8 != getNerveStep()) {
        return;
    }

    setNerve(_FB4);
    _FB4 = nullptr;
}

void MarioActor::setRasterScroll(s32 i1, s32 i2, s32 i3) {
    mScreenStrip1084->fn_803DB9D0(i1, i2, i3);
}

void MarioActor::getGravityVector(TVec3f* pVec) const {
    pVec->set(mMario->getAirGravityVec());
}

const TVec3f& MarioActor::getGravityVec() const {
    return *mMario->getGravityVec();
}

const TVec3f& MarioActor::fn_803B8520() {
    return this->mMario->getAirGravityVec();
}

void MarioActor::getGroundPos(TVec3f* pDst) const {
    *pDst = mMario->mGroundPos;
}

TVec3f* MarioActor::getShadowPos() const {
    return &mMario->mShadowPos;
}

void MarioActor::getUpVec(TVec3f* pVec) const {
    *pVec = _270;
}

void MarioActor::getSideVec(TVec3f* pVec) const {
    *pVec = mMario->mSideVec;
}

f32 MarioActor::getGravityRatio() const {
    return mGravityRatio;
}

bool MarioActor::isInPunchTimerRange() const {
    bool ret = false;
    if (_3E6) {
        ret = MR::isInRange(_945, 6.0f, 20.0f);
    }

    return ret;
}

void MarioActor::getThrowVec(TVec3f* pVec) const {
    TVec3f rotate;
    mSearchLightThrowPos->copyRotate(&rotate);

    TVec3f dirZ(0.0f, 0.0f, 1.0f);

    Mtx trMtx;
    MR::makeMtxTR(trMtx, 0.0f, 0.0f, 0.0f, rotate.x, rotate.y, rotate.z);

    PSMTXMultVec(trMtx, dirZ, pVec);
}

bool MarioActor::isTurning() const {
    return getMovementStates().mLowBits.mBit28;
}

bool MarioActor::isJumping() const {
    if (_934) {
        return _938.dot(getGravityVec()) < -10.0f;
    }

    if (mPlayerMode == 6 && mMario->mVerticalSpeed < mConst->getTable()->mTeresaDropDownHeight) {
        return false;
    }

    if (mMario->isWalling()) {
        return true;
    }

    return ((mMario->mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && !((mMario->mMovementStates.mLowWord >> 30) & 1U);
}

bool MarioActor::isJumpRising() const {
    return ((mMario->mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && mMario->isRising();
}

bool MarioActor::isPunching() const {
    if (mMario->isStatusActive(MarioStatus_Magic)) {
        return true;
    }

    if (mMario->mSwim->fn_803FE9F0()) {
        return true;
    }

    if (_3E5) {
        return true;
    }

    if (!mMario->isAnimationTerminate(nullptr) &&
        (mMario->isAnimationRun("空中ひねり") || mMario->isAnimationRun("雲スピン空中") || mMario->isAnimationRun("アイスひねり") || mMario->isAnimationRun("アイスひねり空中") ||
         mMario->isAnimationRun("アイスひねり静止") || mMario->isAnimationRun("アイスひねり移動"))) {
        return true;
    }

    return false;
}

bool MarioActor::isItemSwinging() const {
    if (mMario->isAnimationRun("テニスショット左") || mMario->isAnimationRun("テニスショット右") || mMario->isAnimationRun("テニスショット中")) {
        return true;
    } else {
        return false;
    }
}

bool MarioActor::isLongDrop() const {
    if (((mMario->getMovementStates().mLowWord >> 29) & 1U) == 0) {
        return true;
    }

    return mMario->mVerticalSpeed > 1000.0f;
}

u32 MarioActor::fn_803B89E0() {
    return ((this->mMario)->mMovementStates.mLowWord >> 0x14) & 1;
}

bool MarioActor::isFastRise() const {
    if (_934) {
        if (_938.dot(getGravityVec()) < -60.0f) {
            return true;
        }
    }

    return false;
}

bool MarioActor::isDamaging() const {
    return mMario->isDamaging() != false;
}

bool MarioActor::isFireDamageActive() {
    return this->mMario->fn_80393C40();
}

bool MarioActor::isStaggering() const {
    return mMario->isStatusActive(MarioStatus_Flip) != false;
}

bool MarioActor::isSleeping() const {
    // No way that this is a real inline but only way I could make it match
    bool out = true;
    if (!IsMarioAnimationRun("特殊ウエイト1B") && !IsMarioAnimationRun("特殊ウエイト1A")) {
        out = false;
    }

    return out;
}

bool MarioActor::isNeedDamageFog() const {
    if (!isEnableNerveChange()) {
        return false;
    }

    if (MR::isDemoActive()) {
        return false;
    }

    if (_EA4) {
        return false;
    }

    if (mMario->getDamageAfterTimer() != 0) {
        return true;
    }

    return mBlinkRequested1067;
}

// NONMATCHING: equivalent short-circuit return normalization and saved register.
bool MarioActor::isRefuseTalk() const {
    if (!isEnableNerveChange()) {
        return true;
    }

    if (mMario->isStatusActive(MarioStatus_Wait)) {
        return true;
    }

    if (mMario->isStatusActive(MarioStatus_Hang)) {
        return true;
    }

    if (mMario->isStatusActive(MarioStatus_13)) {
        return true;
    }

    if (mMario->isDamaging()) {
        return true;
    }

    if (mMario->_420 != 0) {
        return true;
    }

    bool isNormal = !mMario->isPlayerModeTeresa();
    if (isNormal) {
        u32 states;
        Mario* mario;
        if (_B94 != 0) {
            return true;
        }

        mario = mMario;
        states = mario->mMovementStates.mLowWord;
        if (((states & Mario::MovementStates::Jumping) >> 31)) {
            return true;
        }

        if (!((states >> 30) & 1U)) {
            return true;
        }

        if (mario->_3CE < 5) {
            return true;
        }

        if (((mario->mMovementStates.mHighWord >> 28) & 1U)) {
            return true;
        }

        if (((mario->mDrawStates.mWord >> 21) & 1U)) {
            return true;
        }

        if (mario->_3D2 != 0) {
            return true;
        }
    }

    if (_424 != nullptr) {
        return true;
    }

    if (_480) {
        return true;
    }
    if (mMario->isPlayerModeYoshi() && !mYoshiHolder->mTongueAction->fn_8041FA30()) return true;
    return false;
}

u32 MarioActor::getSpecialMode() const {
    u32 mode = 0;
    if (mMario->getMovementStates().mLowBits.mBit31 && mMario->getMovementStates().mLowBits.mBit16) {
        mode = 1;
    }

    if (mMario->getMovementStates().mLowBits.mBit31 && mMario->getMovementStates().mLowBits.mBit14) {
        mode = 1;
    }

    if (mode == 0) {
        return mode;
    }

    if (mMario->mVerticalSpeed < 300.0f) {
        mode = 2;
    }

    if (mMario->mVerticalSpeed > 800.0f) {
        return 3;
    }

    return mode;
}

GravityInfo* MarioActor::getGravityInfo() const {
    return mGravityInfo;
}

u8 MarioActor::getGravityLevel() const {
    return MR::isLightGravity(*mGravityInfo);
}

bool MarioActor::isDebugMode() const {
    return getMovementStates().mLowBits.mBit9;
}

MtxPtr MarioActor::getMapBaseMtx() const {
    HitSensor* pSensor = mMario->_46C->mSensor;
    if (pSensor == nullptr) {
        return nullptr;
    }

    return MR::getSensorHost(pSensor)->getBaseMtx();
}

bool MarioActor::sendMsgToSensor(HitSensor* pSensor, u32 msg) {
    return pSensor->receiveMessage(msg, getSensor("Body"));
}

// NONMATCHING: verified pooled literal addresses only.
bool MarioActor::sendMsgUpperPunch(HitSensor* pSensor) {
    if (!mActionTable1080->isActionOk("アッパーパンチ")) {
        return false;
    }
    if (pSensor && !MR::isDead(MR::getSensorHost(pSensor)) &&
        pSensor->receiveMessage(5, getSensor("Body"))) {
        if (mMario->isPlayerModeYoshi()) {
            MR::startActionSound(mYoshiHolder->mYoshi, "Hit", -1, -1, -1);
        } else {
            mMario->playSound("声蹴り", -1);
        }
        if (!mMario->isSwimming()) {
            mMario->changeAnimation("アッパーパンチ");
        }
        return true;
    }
    return false;
}

const TVec3f& MarioActor::getLastMove() const {
    return mLastMove;
}

void MarioActor::getLastMove(TVec3f* pVec) const {
    *pVec = mLastMove;
}

void MarioActor::getFrontVec(TVec3f* pVec) const {
    *pVec = mMario->mFrontVec;
}

const TVec3f& MarioActor::getTransForCamera() const {
    return mCameraTrans;
}

bool MarioActor::isCameraStateOn(SPECIAL_STATUS_FOR_CAMERA status) const {
    switch (status) {
    case CASE_0:
        if (mMario->isStatusActive(MarioStatus_FireDance)) {
            return true;
        }

        break;

    case CASE_1:
        if (isJumping() && mMario->_430 == 11) {
            return true;
        }

        break;
    }

    return false;
}

void MarioActor::decLife(u16 amt) {
    if (((getMovementStates().mLowWord >> 0) & 1U)) return;
    if (MR::isGalaxyHorrorCometAppearInCurrentStage()) {
        MR::decCometTimer(1200);
        return;
    }
    if (amt && _92C > amt) _92C = amt;
    if (_92C == 0) {
        decreaseLifeByOne();
        _92C = 0;
        fn_803D9740();
    }
}

void MarioActor::incLife(u32 amt) {
    if (isEnableNerveChange() && !_B68) {
        const u32 health = getHealth();
        if (mHealth != mMaxHealth) {
            mMario->playSound("ライフ回復");
        }

        mHealth += amt;
        if (mHealth >= mMaxHealth) {
            mHealth = mMaxHealth;
        }

        if (health == 1 && mMarioAnim->isAnimationStop()) {
            mMarioAnim->changeTrackAnim(3, "ノーマルウエイト");
            if (mMario->_970 && strcmp(mMario->_970, "DamageWait")) {
                mMario->startBas(nullptr, false, 0.0f, 0.0f);
                fn_803BBFD0(nullptr);
            }
        }
    }
}

void MarioActor::decLifeLarge() {
    decLife(0);
}

void MarioActor::decLifeMiddle() {
    decLife(0);
}

void MarioActor::fn_803B9270() {
    this->decLife(0);
}

void MarioActor::changeMaxLife(s32 max) {
    mMaxHealth = max;
    while (mHealth != max) {
        if (mHealth > max) {
            decLife(0);
        } else if (mHealth < max) {
            incLife(1);
        } else {
            break;
        }
    }
}

void MarioActor::calcBaseFrontVec(const TVec3f& rVec) {
    TVec3f cross, j(0.0f, 1.0f, 0.0f);
    f32 y = j.dot(rVec);
    if (y < -0.99f) {
        _258.set(0.0f, 0.0f, 1.0f);
    } else {
        f32 cosine;
        if (MR::makeAxisAndCosignVecToVec(&cross, &cosine, rVec, j)) {
            TVec3f k(0.0f, 0.0f, 1.0f);
            Mtx transformation;
            PSMTXRotAxisRad(transformation, &cross, -MR::acos(cosine));
            PSMTXMultVecSR(transformation, &k, &_258);
            MR::normalize(&_258);
        } else {
            _258.set(0.0f, 0.0f, 1.0f);
        }
    }
}

bool MarioActor::fn_803B93E0() {
    return MR::isNearZero(mPreviousGravityVec, 0.001f);
}

void MarioActor::setPlayerMode(u16 mode, bool a, bool b) {
    if (mode != 3) {
        (this->mMario)->mFlags14.mHighWord &= 0xFFBFFFFF;
    }
    if (!this->fn_803CB1A0(mode)) {
        if (this->mMario->isStatusActive(0x18)) {
            this->mMario->closeStatus(nullptr);
        }
        reinterpret_cast<s16&>(this->_3DC) = 0;
        this->_A6E = 0;
        this->fn_803CB2E0(mode);
        this->fn_803CB510(mode, a);
        if (mPlayerMode == 0) {
            this->fn_803CBA90(a, b);
        } else {
            this->fn_803CBB30(b);
        }
        this->updateHandAtMorph();
    }
}

void MarioActor::fn_803B94E0() {
    Mario* m = this->mMario;
    if (m->mCometHopperArea == 0) {
        if (((m)->mMovementStates.mLowWord >> 0x10) & 1) {
            m->forceStopTornado();
        }
        if (mActionTable1080->isActionOk("ダメージ解除")) {
            this->setPlayerMode(0, true, true);
        }
    }
}

void MarioActor::fn_803B9560() {
    if (mActionTable1080->isActionOk("ノーダメージ解除")) {
        this->setPlayerMode(0, true, true);
    }
}

// NONMATCHING: verified pooled literal and nerve addresses.
void MarioActor::setPress(u8 myChar, s32 myInt) {
    if (_390 != 0) {
        return;
    }

    if (myChar == 0 && mMario->calcDistToCeil(false) >= 160.0f) {
        return;
    }

    if (((getMario()->mFlags14.mLowWord >> 2) & 1U)) {
        return;
    }

    if (mCounter6C8 == 0 || static_cast<s32>(mCounter6C8) == 2) {
        // Missing instruction
        *mMario->_480 = *mMario->mGroundPolygon;
        *mMario->_484 = *mMario->_4C8;
    }

    if (mMario->isStatusActive(MarioStatus_Blown)) {
        mMario->closeStatus(nullptr);
    }

    mMario->startPadVib("マリオ[つぶれ]");

    MR::forceDeleteEffectAll(this);

    mVelocity.zero();

    if (myInt == 0) {
        myInt = 240;
    }

    _390 = myInt;

    mMario->mMovementStates.mLowWord &= 0xFFEFFFFFU;
    mMario->mMovementStates.mLowWord &= 0xFFFEFFFFU;

    mMario->stopJump();

    mMario->mMovementStates.mLowWord |= 0x40000000U;

    mMario->_13C.zero();
    mMario->_148.zero();
    mMario->_154.zero();

    mMario->mVerticalSpeed = 0.0f;

    mMario->stopWalk();

    mMarioAnim->setSpeed(0.0f);

    _B90 = true;
    mCounter6C8 = myChar;
    _F44 = false;

    mMario->playSound("プレスダメージ");
    if (mMario->isPlayerModeYoshi()) {
        MR::startActionSoundPlayer("DeathInstant", -1, -1);
    } else {
        mMario->playSound("声大ダメージ");
    }

    _FB4 = GET_NERVE(MarioActor, MarioActorNrvGameOver);
    _FB8 = 60;

    if (static_cast<s32>(mCounter6C8) != 4) {
        _390 = 1000;
    }

    for (u32 idx = 0; idx < 6; idx++) {
        decLife(0);
    }

    MR::startStarPointerModeDemoMarioDeath(this);
    mMario->mPosHolder->fn_804060C0();
}

Yoshi* MarioActor::getYoshi() {
    return mYoshiHolder->mYoshi;
}

void* MarioActor::fn_803B9800() {
    return mYoshiHolder->fn_8041B6F0();
}

YoshiTongueAction* MarioActor::fn_803B9810() {
    return mYoshiHolder->mTongueAction;
}

void* MarioActor::fn_803B9820() {
    return mYoshiHolder->_C;
}

YoshiHolder* MarioActor::fn_803B9830() {
    return mYoshiHolder;
}

bool MarioActor::fn_803B9840() {
    return mYoshiHolder != nullptr;
}

MtxPtr MarioActor::fn_803B9850() {
    return mYoshiHolder->mYoshi->getBaseMtx();
}

void MarioActor::fn_803B9870() {
    if (this->mMario->isPlayerModeYoshi()) {
        mYoshiHolder->mYoshi->fn_8040CF20();
    }
}

void MarioActor::fn_803B98C0() {
    if (static_cast<YoshiHolder* volatile&>(mYoshiHolder) != nullptr) {
        static_cast<YoshiHolder* volatile&>(mYoshiHolder)->_3C = 1;
    }
}

void MarioActor::fn_803B98E0() {
    if (static_cast<YoshiHolder* volatile&>(mYoshiHolder) != nullptr) {
        static_cast<YoshiHolder* volatile&>(mYoshiHolder)->_3C = 0;
    }
}

s32 MarioActor::fn_803B9900() {
    YoshiHolder* p = mYoshiHolder;
    if (p != nullptr) {
        return p->_3C;
    }
    return 0;
}

bool MarioActor::fn_803B9920() {
    if (!this->mMario->isPlayerModeYoshi()) {
        return false;
    }
    if (_934 != 0 || this->_38C != 0) {
        return false;
    }
    if (this->_935 != 0) {
        return false;
    }
    Mario* m = this->mMario;
    u32 r = (m)->mFlags14.mHighWord;
    if ((r >> 0x14) & 1) {
        return false;
    }
    if ((r >> 0x15) & 1) {
        return false;
    }
    if (((m)->mMovementStates.mLowWord >> 0x1F) == 1 && !m->isRising() && (((this->mMario)->mMovementStates.mLowWord >> 0xC) & 1)) {
        return true;
    }
    return this->checkButtonType(mInputConfig->getButton8(), false);
}

void MarioActor::fn_803B9A10() {
    if (static_cast<YoshiHolder* volatile&>(mYoshiHolder) != nullptr) {
        static_cast<YoshiHolder* volatile&>(mYoshiHolder)->fn_8041BAE0();
    }
}

void MarioActor::fn_803B9A30() {
    if (static_cast<YoshiHolder* volatile&>(mYoshiHolder) != nullptr) {
        static_cast<YoshiHolder* volatile&>(mYoshiHolder)->fn_8041BAF0();
    }
}

void MarioActor::fn_803B9A50() {
    if (static_cast<YoshiHolder* volatile&>(mYoshiHolder) != nullptr) {
        static_cast<YoshiHolder* volatile&>(mYoshiHolder)->fn_8041BB00();
    }
}

s32 MarioActor::fn_803B9A70() {
    YoshiHolder* p = mYoshiHolder;
    if (p != nullptr) {
        return p->fn_8041BB30();
    }
    return 0;
}

f32 MarioActor::fn_803B9A90() {
    YoshiHolder* p = mYoshiHolder;
    if (p != nullptr) {
        return p->fn_8041BB50();
    }
    return 0.0f;
}

bool MarioActor::fn_803B9AB0() const {
    YoshiHolder* p = mYoshiHolder;
    if (p == nullptr) {
        return false;
    }
    if (p->mRideModelActor == nullptr) {
        return false;
    }
    return !MR::isDead(p->mRideModelActor);
}

void MarioActor::calcHeadPos() {
    TMtx34f mtx;
    const char* pName = "CapPosition";
    getRealMtx(mtx, pName);
    MR::extractMtxTrans(mtx, &mCapPosition);
}

void MarioActor::setUpperRotateY(f32 angle) {
    mMarioAnim->setUpperRotateY(angle);
}

// NONMATCHING: Equivalent normalization of the gravity helper boolean return.
void MarioActor::updateGravityVec(bool reset, bool usePosition) {
    TVec3f gravity;
    fn_803CA270(&gravity, usePosition);
    bool resetGround = false;
    if (!reset) {
        if (_334) gravity = mPreviousGravityVec;
        else resetGround |= fn_803CA440(mPreviousGravityVec, gravity);
        if (MR::isNearZero(gravity, 0.001f)) mMario->mFlags14.mLowWord |= 0x10000000;
    } else {
        _334 = 0;
    }
    fn_803CA710(mPreviousGravityVec, gravity);
    mPreviousGravityVec = gravity;
    fn_803CA860(gravity);
    fn_803C9920(gravity);
    applyGravityVec(gravity, resetGround);
    fn_803CAA00();
    if (mMario->isPlayerModeYoshi()) mYoshiHolder->mYoshi->mGravity.set<f32>(*mMario->getGravityVec());
}

bool MarioActor::tryJetAttack(HitSensor* pReceiver) {
    return MR::sendArbitraryMsg(ACTMES_JET_TURTLE_ATTACK, pReceiver, getSensor("dummy"));
}

bool MarioActor::releaseThrowMemoSensor() {
    // FIXME: missing class at 0x988 that has HitSensor* at 0xA4
    if (_468 != 0) {
        if (mMario->isStatusActive(MarioStatus_Wall) || mMario->isStatusActive(MarioStatus_SideStep)) {
            return false;
        }

        if (mMario->isSwimming() && mSearchLight->_A4 != nullptr) {
            _470 = mSearchLight->_A4;
        }

        s32 idx = _468 - 1;
        if (_470 != nullptr) {
            TVec3f diff = _470->mPosition - _240;

            if (mMario->isSwimming()) {
                TVec3f throwVec;
                getThrowVec(&throwVec);
                if (MR::diffAngleAbs(diff, throwVec) > PI / 3.0f) {
                    _470 = nullptr;
                }
            } else if (MR::diffAngleAbs(diff, mMario->mFrontVec) > PI / 3.0f) {
                _470 = nullptr;
            }
        }

        if (_470 == nullptr || !mMario->isSwimming()) {
            if (_F24 != nullptr) {
                if (!_428[idx]->receiveMessage(ACTMES_THROW, _F24)) {
                    return false;
                }
            } else if (!_428[idx]->receiveMessage(ACTMES_THROW, getSensor("Body"))) {
                return false;
            }

            _474 = 0;
        } else {
            if (!_428[idx]->receiveMessage(ACTMES_THROW, _470)) {
                return false;
            }

            _474 = _470;
        }

        _468--;

        if (_468 == 0) {
            _470 = nullptr;
            _47C = 0;
            _428[0] = nullptr;
        }

        clearNullAnimation(0);

        _3AC = 60;

        return true;
    }

    return false;
}

void MarioActor::fn_803B9F10() {
    this->tryReleaseWithMsg(0x2F);
    this->updateSearchLight();
    if (_424 != nullptr) {
        _424->receiveMessage(0x2F, this->getSensor("Body"));
        _424 = nullptr;
    }
}

void MarioActor::damageDropThrowMemoSensor() {
    tryReleaseWithMsg(ACTMES_DAMAGEDROP);
    updateSearchLight();
    tryReleaseBombTeresa();

    if (_424 != nullptr) {
        _424->receiveMessage(ACTMES_DAMAGEDROP, getSensor("Body"));
        _424 = nullptr;
    }
}

void MarioActor::rushDropThrowMemoSensor() {
    tryReleaseWithMsg(ACTMES_RUSHDROP);
    updateSearchLight();

    if (_424 != nullptr) {
        _424->receiveMessage(ACTMES_DAMAGEDROP, getSensor("Body"));
        _424 = nullptr;
    }
}

void MarioActor::fn_803BA070() {
    if (static_cast<s32>(_468) == 0) {
        if (this->mMario->isAnimationRun("ひろいクイック")) {
            this->mMario->stopAnimationUpperForce();
            this->_B92 = 0;
            mNullAnimation->kill();
            this->_480 = 0;
        }
    } else {
        for (u32 i = 0; i < (u32)static_cast<s32>(_468); i++) {
            if (this->fn_803CEEE0(_428[i])) {
                _428[i]->receiveMessage(0x2D, this->getSensor("Body"));
                _428[i] = nullptr;
                _468 -= 1;
            } else if (_428[i] != nullptr) {
                this->fn_803D5A00(i);
            }
        }
        _474 = nullptr;
        if (static_cast<s32>(_468) == 0) {
            this->mMario->stopAnimationUpperForce();
            this->mMario->mSwim->resetJet();
        }
        if (this->_480 != 0) {
            this->mMario->stopAnimationUpperForce();
            this->_480 = 0;
        }
        this->_B92 = 0;
        mNullAnimation->kill();
        this->tryReleaseWithMsg(0x2D);
        this->updateSearchLight();
        if (_424 != nullptr) {
            _424->receiveMessage(0x2C, this->getSensor("Body"));
            _424 = nullptr;
        }
    }
}

void MarioActor::fn_803BA220() {
    this->_480 = 0;
}

// NONMATCHING: local vector stack slots, Mario receiver load scheduling, and literal offsets.
void MarioActor::tryTornadoPull(HitSensor* pSensor) {
    if (!mActionTable1080->isActionOk("コイン引っ張り")) {
        return;
    }

    u32 type = pSensor->mType;
    if (strcmp(MR::getSensorHost(pSensor)->mName, "カメックビーム用カメ") == 0) {
        type = 5555;
    }

    switch (type) {
    case 0x46: {
        if (mPlayerMode == 7 || IsMarioSwimming()) {
            TVec3f center(_240);
            TVec3f ground;
            ground = mMario->mGroundPos;
            if (IsMarioSwimming()) {
                ground = center - mBodyUpDirection * 600.0f;
            }

            center += mMario->mHeadVec * 200.0f;
            TVec3f position(*MR::getSensorPos(pSensor));
            TVec3f foot;
            f32 projection = MR::getFootPoint(center, ground, position, &foot);
            if (projection < -0.1f) {
                return;
            }

            if (projection > 2.0f) {
                return;
            }

            if (PSVECDistance(&foot, &position) > 600.0f) {
                return;
            }

            Mario* player = mMario;
            if (player->fn_803B2880(_240, *MR::getSensorPos(pSensor) - _240)) {
                return;
            }

            for (s32 i = 0; i < 128; i++) {
                if (mPulledSensors7C0[i] == pSensor) {
                    return;
                }
            }

            pSensor->receiveMessage(0x85, getSensor("Body"));
            if (pSensor->receiveMessage(0x85, getSensor("Body"))) {
                for (s32 i = 0; i < 128; i++) {
                    if (mPulledSensors7C0[i] == nullptr) {
                        mPulledSensors7C0[i] = pSensor;
                        return;
                    }
                }
            }
        }

        break;
    }

    case 0x12:
    case 0x13:
        if (mActionTable1080->isActionOk("カメ持ち") && pSensor->receiveMessage(0x84, getSensor("Body"))) {
            if (PSVECDistance(&mPosition, MR::getSensorPos(pSensor)) < 120.0f) {
                mConditionGroup->tryGetItem(pSensor);
                return;
            }

            LiveActor* host = MR::getSensorHost(pSensor);
            tryPullTrans(&host->mVelocity, *MR::getSensorPos(pSensor));
            pSensor->receiveMessage(0x85, getSensor("Body"));
            _424 = pSensor;
        }

        break;
    case 5555:
        if (mActionTable1080->isActionOk("カメ持ち") && pSensor->receiveMessage(0x84, getSensor("Body"))) {
            LiveActor* host = MR::getSensorHost(pSensor);
            tryPullTrans(&host->mVelocity, *MR::getSensorPos(pSensor));
            pSensor->receiveMessage(0x85, getSensor("Body"));
        }

        break;
    }
}

// NONMATCHING: verified equivalent pooled literal addresses.
void MarioActor::resetCondition() {
    bool inWater = false;

    if (_EA5) {
        MR::extractMtxTrans(mRemoteDemoMtx4E4.mMtx, &mPosition);
        mMario->mPosHolder->fn_80405F90(mPosition);
    }

    if (mMario->mSwim->fn_803FD5E0(1)) {
        inWater = true;
    }

    mMario->stopAnimationUpper(nullptr, nullptr);

    if (((mMario->mMovementStates.mLowWord >> 21) & 1U)) {
        mMario->cancelSquatMode();
    }

    mMarioAnim->setAnimationPaused(true);
    mMario->forceStopTornado();
    updateTornado();

    mMario->mSwim->mColdWaterDamageInterval = 0;
    mMario->mFlags20.mWord &= 0xEFFFFFFFU;

    if (mMario->isStatusActive(MarioStatus_Swim) && inWater) {
        if (!_468) {
            mMario->mSwim->resetJet();
        }
    } else if (mMario->isStatusActive(MarioStatus_Teresa) && mPlayerMode == 6) {
        if (mMario->isStatusActive(MarioStatus_Wait)) {
            mMario->closeStatus(mMario->mWait);
        }

        mMario->resetTeresaMode();
    } else {
        mMario->closeStatus(nullptr);
    }

    bool was1F = mMario->mMovementStates.mLowBits.mBit0;
    mMario->mMovementStates.mLowWord = 0;
    mMario->mMovementStates.mHighWord = 0;
    mMario->mFlags14.mLowWord = 0;
    mMario->mFlags14.mHighWord = 0;
    mMario->mMovementStates.mLowBits.mBit0 = was1F;

    resetPadSwing();

    if (mMario->isStatusActive(MarioStatus_Swim) && inWater) {
        if (_468) {
            mMario->changeAnimation("水泳ジェット", "水泳ジェット");
        } else {
            mMario->changeAnimation(nullptr, "水泳基本");
        }
    } else {
        mMario->changeAnimation(nullptr, "基本");

        if (_468) {
            mMarioAnim->waterToGround();
        }
    }

    mMario->setFrontVecKeepUp(mMario->mFrontVec);
    updateGravityVec(true, true);

    if (!MR::isNearZero(mPreviousGravityVec)) {
        mMario->setHeadVec(-mPreviousGravityVec);
    }

    mMario->_1FC = mMario->mHeadVec;
    mMario->_13C.zero();
    mMario->_148.zero();
    mMario->_154.zero();
    mMario->mVerticalSpeed = 0.0f;
    mMario->_474->mIdx = -1;
    stopEffect("いい汗");
    stopEffect("雲尻落ルイージ");
    stopEffect("雲尻落");
    stopEffect("ホッパー尻落ルイージ");
    stopEffect("ホッパー尻落");
    stopEffect("尻落ルイージ");
    stopEffect("尻落");

    if (_9C4 != nullptr) {
        _9C4->kill();
        MR::offCalcAnim(_9C4);
    }

    for (u32 i = 0; i < 11; i++) {
        mMario->_984[i] = nullptr;
    }

    if (_EA5) {
        mMario->changeAnimationInterpoleFrame(0);
        mBlendMtxTimer = 0;
    }
}

bool MarioActor::sendPunch(HitSensor* pSensor, const Triangle* triangle, bool playSound) {
    if (mMario->isDamaging()) {
        _944 = false;
        return false;
    }
    if (playSound && mPlayerMode == 3) {
        bool hit = doFreezeAttack(pSensor);
        if (hit && !mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, "スピンヒット音")) MR::startSpinHitSound(this);
        return hit;
    }
    if (fn_803CFCA0(pSensor)) return false;
    _FCC = true;
    mPunchTriangle106C = triangle;
    bool hit = MR::sendMsgPlayerPunch(pSensor, getSensor("Body"));
    _FCC = false;
    mPunchTriangle106C = nullptr;
    if (playSound && hit && !mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, "スピンヒット音")) MR::startSpinHitSound(this);
    return hit;
}

// NONMATCHING: saved-register allocation, load scheduling and verified pooled literals.
bool MarioActor::sendBodyAttack(HitSensor* pSensor) {
    bool isStatus = true;
    if (mMario->getCurrentStatus() != MarioStatus_None && mMario->getCurrentStatus() != MarioStatus_Skate) {
        isStatus = false;
    }

    if (isStatus && !((mMario->mMovementStates.mLowWord >> 23) & 1U) && !mMario->isDamaging() && !((mMario->mMovementStates.mLowWord >> 20) & 1U)) {
        HitSensor* body = getSensor("Body");
        TVec3f newPos(*MR::getSensorPos(pSensor));
        newPos -= body->mPosition;
        if (MR::diffAngleAbsHorizontal(newPos, mMario->mFrontVec, *mMario->getGravityVec()) >= HALF_PI) {
            return false;
        }

        if (MR::sendArbitraryMsg(0x35, pSensor, getSensor("Body"))) {
            if (mMario->isPlayerModeYoshi()) {
                MR::startActionSound(mYoshiHolder->mYoshi, "Hit", -1, -1, -1);
            } else {
                mMario->playSound("声ランニングキック", -1);
            }
            mMario->changeAnimation("ランニングキック", static_cast<const char*>(nullptr));
            mMario->startPadVib(2);

            if (!MR::sendArbitraryMsg(0x27, pSensor, getSensor("Body"))) {
                printHitMark(pSensor);
            }

            return true;
        }
    }

    if (mPlayerMode == 1 && MR::sendArbitraryMsg(0xE, pSensor, getSensor("Body"))) {
        mMario->startPadVib(2);
        printHitMark(pSensor);

        return true;
    }

    return false;
}

// NONMATCHING: verified pooled literal addresses only.
void MarioActor::reactionPunch(HitSensor* pSensor) {
    if (mPlayerMode == 1) {
        return;
    }

    if (isJumping()) {
        _38C = 5;
    } else {
        _38C = 5;
        if (mMario->mWalkSpeed > 0.1f) {
            mMario->mWalkSpeed = 0.1f;
            mMario->mTargetWalkSpeedIndex = 1;
        }
    }

    if (!((getMovementStates().mHighWord >> 20) & 1U)) {
        jumpHop();
    }

    if (!MR::sendArbitraryMsg(ACTMES_HITMARK_EMIT, pSensor, getSensor("Body"))) {
        printHitMark(pSensor);
    }

    mMario->startPadVib(2);

    if (pSensor->isType(ATYPE_BEGOMAN)) {
        mMario->stopWalk();

        TVec3f reactDir(_240 - *MR::getSensorPos(pSensor));
        MR::vecKillElement(reactDir, *mMario->getGravityVec(), &reactDir);
        reactDir.setLength(20.0f);
        mMario->addReaction(reactDir);
    }

    if (mMario->isSwimming()) {
        TVec3f punchDir(_240 - *MR::getSensorPos(pSensor));
        punchDir.setLength(5.0f);
        mMario->mSwim->hitPunch(punchDir);
    }
}

bool MarioActor::isEnableNerveChange() const {
    if (isNerve(GET_NERVE(MarioActor, MarioActorNrvWait))) {
        return true;
    }

    if (isNerve(GET_NERVE(MarioActor, MarioActorNrvNoRush))) {
        return true;
    }

    return false;
}

bool MarioActor::isEnableMoveMario() const {
    if (isNerve(GET_NERVE(MarioActor, MarioActorNrvWait))) {
        return true;
    }

    if (isNerve(GET_NERVE(MarioActor, MarioActorNrvNoRush))) {
        return true;
    }

    if (isNerve(GET_NERVE(MarioActor, MarioActorNrvGameOverNonStop))) {
        return true;
    }

    return false;
}

void MarioActor::forceGameOver() {
    if (!isEnableNerveChange()) {
        return;
    }

    setNerve(GET_NERVE(MarioActor, MarioActorNrvGameOver));
}

void MarioActor::forceGameOverAbyss() {
    if (!isEnableNerveChange()) {
        return;
    }

    setNerve(GET_NERVE(MarioActor, MarioActorNrvGameOverAbyss));
}

void MarioActor::forceGameOverNonStop() {
    if (!isEnableNerveChange()) {
        return;
    }

    setNerve(GET_NERVE(MarioActor, MarioActorNrvGameOverNonStop));
}

void MarioActor::fn_803BAFE0() {
    if (this->isEnableNerveChange()) {
        this->setNerve(&NrvMarioActor::MarioActorNrvGameOverSink::sInstance);
    }
}

void MarioActor::forceGameOverSink() {
    if (!isEnableNerveChange()) {
        return;
    }

    setNerve(GET_NERVE(MarioActor, MarioActorNrvGameOverNoDemo));
}

void MarioActor::forceKill(u32 type) {
    if (((mMario->getMovementStates().mLowWord >> 0) & 1U) && !MR::isGalaxyRainbowCometAppearInCurrentStage()) {
        return;
    }

    if (!isEnableNerveChange()) {
        return;
    }

    _A6E = 0;

    if (!MR::isDead(_994)) {
        _994->kill();
    }

    if (mMario->isStatusActive(MarioStatus_FpView)) {
        mMario->mFpView->forceClose();
        mMario->closeStatus(nullptr);
    }

    fn_803BE6F0();
    MR::endGalaxyCometEvent();

    if (type == 0) {
        mMario->doAbyssDamage();
        return;
    }

    if (type == 3) {
        mMario->startPadVib(3);
    }

    if (type == 4) {
        mMario->startPadVib(3);
        initDarkMask();
        mMario->doDarkDamage();

        return;
    }

    _39D = type;
    mHealth = 0;
    setNerve(GET_NERVE(MarioActor, MarioActorNrvGameOver));
}

HitSensor* MarioActor::getNearestJumpTarget() const {
    if (_930 == 0) {
        return nullptr;
    }

    if (mHealth == 0) {
        return nullptr;
    }

    f32 nearestDistance = 100000.0f;
    HitSensor* nearest = nullptr;
    for (u32 i = 0; i < _930; i++) {
        if (!isFixJumpRushSensor(_7E4[i])) {
            continue;
        }

        if (_924 == _7E4[i]) {
            continue;
        }

        TVec3f position(_7E4[i]->mPosition);
        f32 centerDistance;
        f32 distance = (position - mPosition).length();
        centerDistance = (position - _240).length();
        f32 headDistance = (position - mCapPosition).length();
        if (distance > centerDistance) {
            distance = centerDistance;
        }

        if (distance > headDistance) {
            distance = headDistance;
        }

        if ((mRushTouchingDD4[i] || !(_7E4[i]->mRadius < distance)) && distance < nearestDistance) {
            nearest = _7E4[i];
            nearestDistance = distance;
        }
    }

    return nearest;
}

HitSensor* MarioActor::getHighPriorityTarget() const {
    if (_930 == 0) {
        return nullptr;
    }

    for (s32 idx = 0; idx < _930; idx++) {
        if (_7E4[idx]->isType(ATYPE_POWER_STAR_BIND)) {
            return _7E4[idx];
        }

        if (mHealth == 0) {
            continue;
        }

        if (_7E4[idx]->isType(ATYPE_QUESTION_COIN_BIND)) {
            return _7E4[idx];
        }

        if (_7E4[idx]->isType(ATYPE_PRIOR_BINDER)) {
            return _7E4[idx];
        }

        if (_7E4[idx]->isType(ATYPE_SCENARIO_STARTER_BIND)) {
            return _7E4[idx];
        }
    }

    return nullptr;
}

HitSensor* MarioActor::getRescueTarget() const {
    if (_930 == 0) {
        return nullptr;
    }

    if (mHealth == 0) {
        return nullptr;
    }

    for (s32 idx = 0; idx < _930; idx++) {
        if (_7E4[idx]->isType(0x62)) {
            return _7E4[idx];
        }

        if (mActionTable1080->testEffect(MR::getSensorHost(_7E4[idx])->mName, "ダメージ中RUSH")) {
            return _7E4[idx];
        }
    }

    return nullptr;
}

void MarioActor::fn_803BB4F0() {
    if (mPlayerMode == 6) {
        MR::requestMovementOn(mTeresaModel->mActor);
    }
}

void MarioActor::fn_803BB510(const TVec3f& a, const TVec3f& b) {
    MR::emitEffectHit(mTeresaModel->mActor, a, b, "WallHit");
}

void MarioActor::showFreezeModel() {
    _9C4->appear();
    MR::onCalcAnim(_9C4);
    MR::startBva(_9C4, "Nomal");
}

void MarioActor::hideFreezeModel() {
    MR::startAction(_9C4, "Break");
    MR::startBva(_9C4, "Break");

    mMario->startFreezeEnd();
}

bool MarioActor::fn_803BB5F0() {
    if (MR::isActionEnd(_9C4)) {
        _9C4->kill();
        MR::offCalcAnim(_9C4);
        return false;
    }
    return true;
}

void MarioActor::syncJumpBeeStickMode() {
    if (mBeeWallWalk != 0) {
        LiveActor* host = MR::getSensorHost(mMario->_45C->mSensor);
        const char* effect = "ハチ出立";
        if (mActionTable1080->testEffect(host->mName, effect)) {
            mBeeWallWalk = 0;
            _9F2 = 30;
        }
    }

    if (!(MR::diffAngleAbs(_360, getGravityVec()) < MR::pi() / 36.0f)) {
        return;
    }

    _33C = mPosition - mMario->mFrontVec * 2.0f;
    _354 = mPosition - _33C;
    _360 = _240 - _33C;
    MR::normalize(&_360);
}

void MarioActor::entryWallWalkMode(const TVec3f& position, const TVec3f& normal) {
    if (mBeeWallWalk) {
        return;
    }

    if (!_9F2) {
        mBeeWallWalk = 5;
        TVec3f gravity(-normal);
        mAppliedGravityVec = gravity;
        mPosition = position;
        mMario->setTrans(position);
        mMario->stopJump();
        mMario->stopAnimation(nullptr);
        mMario->stopWalk();
        TVec3f front(mMario->mHeadVec);
        mMario->setGravityVec(gravity);
        mMario->setHeadVec(-gravity);
        mMario->setFrontVecKeepUp(front, 1UL);
        setBlendMtxTimer(2);
        _38C = 5;
        mMario->mMovementStates.mHighWord &= 0xFFFFFF7FU;
        mCollisionShadow580->_305 = true;
    }
}

void MarioActor::showBeeFur() {
    if (mMario->isPlayerModeBee()) {
        _9E8->appear();
        _9EC->onDraw(-1);
    }

    if (getCarrySensor() != nullptr) {
        MR::showModel(MR::getSensorHost(getCarrySensor()));
    }

    if (mMario->isPlayerModeInvincible()) {
        MR::showJoint(static_cast<J3DModelX*>(mAllModel->getJ3DModel()), "Face0");
        _A6E = 2;
        playEffect("無敵中");
    }
}

const HitSensor* MarioActor::getCarrySensor() const {
    if (_468 == 0) {
        return nullptr;
    }

    return _428[0];
}

void MarioActor::hideBeeFur() {
    if (mMario->isPlayerModeBee()) {
        _9E8->kill();
        _9EC->offDraw(-1);
    }

    if (getCarrySensor() != nullptr) {
        MR::hideModel(MR::getSensorHost(getCarrySensor()));
    }

    if (mMario->isPlayerModeInvincible()) {
        _A6E = 0;
        stopEffect("無敵中");
    }
}

void* MarioActor::fn_803BBA20() {
    return mAllModel->getJ3DModel();
}

void MarioActor::changeDisplayMode() {
    mMarioAnim->setAnimationModel(mAllModel->getJ3DModel());
    mModelManager->mDisplayListMaker->mModel = mAllModel->getJ3DModel();
    if (_494 != nullptr) {
        _494->setBaseMtx(MR::getJointMtx(this, "HandR"));
    }
    mSearchLightThrowPos->setBaseMtx(MR::getJointMtx(this, "HandR"));
}

void MarioActor::fn_803BBAD0() {
    mActorEffectState->mActive = 1;
}

void MarioActor::initBlur() {
    mBlurJointCount = mAllModel->fn_803E7B30();
    for (u32 i = 0; i < 2; ++i) {
        for (u32 j = 0; j < 8; ++j) {
            mBlurMatrices[i][j] = new (0x20) Mtx[mBlurJointCount];
            for (u32 k = 0; k < mBlurJointCount; ++k) PSMTXIdentity(mBlurMatrices[i][j][k]);
        }
    }
    _A6E = 0;
    mBlurHistoryIndex = 0;
    mBlurBufferIndex = 0;
}

void MarioActor::createIceFloor(const TVec3f& rVec) {
    TPos3f mtx;
    mtx.identity();

    TVec3f upVec;
    getUpVec(&upVec);

    MR::makeMtxUpFront(&mtx, -getAirGravityVec(), mMario->mFrontVec);

    TVec3f vec;
    mtx.getEuler(vec);

    vec *= 180.0f / PI;
    createIceFloor(rVec, vec);
}

void MarioActor::createIceFloor(const TVec3f& rVec1, const TVec3f& rVec2) {
    _B4C[_B50]->setOn(_B50, rVec1, rVec2);

    _B50 = (_B50 + 1) % 20;

    if (!MR::isDead(_B4C[_B50])) {
        _B4C[_B50]->destroy();
    }
}

void MarioActor::createIceWall(const TVec3f& rVec1, const TVec3f& rVec2) {
    TPos3f mtx;
    mtx.identity();
    TVec3f upVec;
    getUpVec(&upVec);

    MR::makeMtxFrontUp(&mtx, getGravityVector(), rVec2);

    TVec3f vec;
    mtx.getEuler(vec);
    vec *= 180.0f / PI;
    _B4C[_B50]->setOn(_B50, rVec1, vec);

    _B50 = (_B50 + 1) % 20;

    if (!MR::isDead(_B4C[_B50])) {
        _B4C[_B50]->destroy();
    }
}

void MarioActor::fn_803BBDD0(const TVec3f& a, const TVec3f& b, const TVec3f& c) {
    if (this->mMario->isPlayerModeYoshi()) {
        mYoshiHolder->fn_8041BBF0(a, b, c);
    }
}

// NONMATCHING: verified pooled literal addresses only.
void MarioActor::doFreezeAttack() {
    if (isAnimationRun("アイスひねり空中")) {
        return;
    }

    if (isJumping()) {
        if (mMario->_430 != 13) {
            mMario->changeAnimation("アイスひねり空中");
            playSound("スピンジャンプ", -1);
            playSound("声スピン", -1);

            if (mMario->_42C == 0) {
                jumpHop();
                mMario->_42C++;
            }
        }

        return;
    }

    if (mMario->isSkatableFloor()) {
        mMario->doSkate();
        return;
    }

    if (!isAnimationRun("アイスひねり")) {
        playSound("スケートスピン", -1);
        playSound("スピンジャンプ", -1);
        playSound("声パンチ", -1);
    }

    mMario->changeAnimation("アイスひねり");
}

bool MarioActor::doFreezeAttack(HitSensor* pSensor) {
    MR::getSensorHost(pSensor);
    return MR::sendArbitraryMsg(ACTMES_FREEZE_ATTACK, pSensor, getSensor("Body"));
}

void MarioActor::fn_803BBFD0(const char* name) {
    if (name == nullptr) {
        mEyeRes = nullptr;
        return;
    }
    char buf[0x80];
    snprintf(buf, 0x80, "%s%s", this->mMario->isPlayerModeYoshi() ? "Yoshi" : "", name);
    if (MR::getResourceHolder(this)->mBtpResTable->isExistRes(buf)) {
        J3DAnmTexPattern* r = static_cast<J3DAnmTexPattern*>(MR::getResourceHolder(this)->mBtpResTable->getRes(buf));
        mEyeRes = r;
        r->setFrame(0.0f);
        return;
    }
    mEyeRes = nullptr;
}

void MarioActor::changeGameOverAnimation() {
    int animation = 0;
    if (mMario->isStatusActive(MarioStatus_FpView)) {
        mMario->closeStatus(nullptr);
    }

    if (mMario->isAnimationRun("前方小ダメージ")) {
        animation = 0;
    }

    if (mMario->isAnimationRun("後方小ダメージ")) {
        animation = 0;
    }

    if (mMario->isAnimationRun("ファイアラン前兆")) {
        animation = 0;
    }

    if (mMario->isAnimationRun("炎のランナー")) {
        animation = 0;
    }

    if (mMario->isAnimationRun("電気ダメージ")) {
        animation = 1;
    }

    if (mMario->isAnimationRun("電気ダメージ終了")) {
        animation = 1;
    }

    if (mMario->isAnimationRun("炎ダメージ")) {
        animation = 2;
    }

    if (mMario->isAnimationRun("ファイアダンス")) {
        animation = 2;
    }

    if (mMario->isAnimationRun("中ダメージ")) {
        animation = 3;
    }

    if (mMario->isAnimationRun("中ダメージ空中")) {
        animation = 3;
    }

    if (mMario->isAnimationRun("中ダメージ着地")) {
        animation = 3;
    }

    if (mMario->isAnimationRun("中後ダメージ")) {
        animation = 4;
    }

    if (mMario->isAnimationRun("中後ダメージ空中")) {
        animation = 4;
    }

    if (mMario->isAnimationRun("中後ダメージ着地")) {
        animation = 4;
    }

    if (mMario->isAnimationRun("落下")) {
        animation = 5;
    }

    if (mMario->isAnimationRun("空中ふんばり")) {
        animation = 5;
    }

    if (mMario->isAnimationRun("つぶれ")) {
        animation = 6;
    }

    if (mMario->isAnimationRun("つぶれ復帰")) {
        animation = 6;
    }

    if (mMario->isAnimationRun("氷結") || mMario->isStatusActive(MarioStatus_Freeze)) {
        animation = -1;
    }

    if (mMario->isSwimming()) {
        animation = 7;
    }

    if (isNerve(GET_NERVE(MarioActor, MarioActorNrvGameOverSink))) {
        animation = 8;
    }

    if (MR::getKarikariClingNum() && ((mMario->getMovementStates().mLowWord >> 30) & 1U)) {
        animation = 0;
    }

    if (_39D == 2) {
        animation = 9;
    }

    if (_39D == 1) {
        animation = 10;
    }

    switch (animation) {
    case 0:
        mMario->changeAnimationNonStop("座りダウン");
        break;
    case 1:
        mMario->changeAnimationNonStop("感電ダウン");
        break;
    case 2:
        mMario->changeAnimationNonStop("炎ダウン");
        break;
    case 3:
        mMario->changeAnimationNonStop("仰向けダウン");
        break;
    case 4:
        mMario->changeAnimationNonStop("俯せダウン");
        break;
    case 5:
        if (((mMario->getMovementStates().mLowWord >> 30) & 1U)) {
            mMario->changeAnimationNonStop("座りダウン");
        } else {
            mMario->changeAnimationNonStop("奈落ダウン");
        }

        break;
    case 6:
        mMario->changeAnimationNonStop("つぶれダウン");
        break;
    case 7:
        mMario->changeAnimationNonStop("水泳ダウン");
        break;
    case 8:
        mMario->changeAnimationNonStop("埋まりダウン");
        break;
    case 9:
        mMario->changeAnimationNonStop("レース負け");
        break;
    case 10:
        mMario->changeAnimationNonStop("水中レース負け");
        break;
    }

    _B90 = true;
    stopEffect("無敵中");
    _A6E = 0;
}

void MarioActor::fn_803BC440(const char* name) {
    mNullAnimation->appear();
    MR::startAction(mNullAnimation, name);
    this->_B92 = -2;
}

void MarioActor::clearNullAnimation(s8 num) {
    _B92 = num;
    mNullAnimation->kill();
}

bool MarioActor::isStopNullAnimation() const {
    if (!MR::isActionEnd(mNullAnimation)) {
        return MR::isDead(mNullAnimation);
    }

    return true;
}

void MarioActor::changeSpecialModeAnimation(const char* pAnimName) {
    switch (mPlayerMode) {
    case 6:
        if (!strcmp(pAnimName, "特殊ウエイト1A")) {
            changeTeresaAnimation("SleepStart", -1);
            return;
        }

        if (strcmp(pAnimName, "特殊ウエイト1B")) {
            return;
        }

        changeTeresaAnimation("Sleep", 16);
        MR::emitEffect(mTeresaModel->mActor, "Sleep");
    }
}

void MarioActor::fn_803BC5C0() {
    Mario* m = this->mMario;
    if (!((m->mMovementStates.mLowWord >> (0x15)) & 1) && m->getCurrentStatus() == 0) {
        Mario* m2 = this->mMario;
        if (((m2->mMovementStates.mLowWord >> (0x1E)) & 1) && (m2)->_960 == 0x20 && this->mMarioAnim->isAnimationStop()) {
            this->mMarioAnim->fn_803DF0F0();
            this->_B96 = 2;
        }
    } else {
        this->_B96 = 0;
    }
    switch (mPlayerMode) {
    case 6:
        this->updateTeresaAnimation();
        break;
    case 4:
        if (this->mBeeWallWalk != 0 && !this->isJumping() && this->mMarioAnim->isAnimationStop()) {
            this->mMarioAnim->fn_803DF1B0();
        }
        break;
    default:
        reinterpret_cast<s16&>(this->mMario->_418) = 0;
        break;
    }
    u8 t = this->_B96;
    if (t != 0) {
        u8 n = t - 1;
        this->_B96 = n;
        if (n == 0 && ((this->mMario->mMovementStates.mLowWord >> (0x1E)) & 1) && this->mMarioAnim->isAnimationStop()) {
            this->mMarioAnim->fn_803DF2D0();
        }
    }
}

void MarioActor::changeTeresaAnimation(const char* pAnimation, s32 interpolation) {
    if (MR::isBckPlaying(mTeresaModel->mActor, "sleep")) {
        MR::deleteEffect(mTeresaModel->mActor, "Sleep");
    }

    if (interpolation == -1) {
        MR::startAction(mTeresaModel->mActor, pAnimation);
    } else {
        MR::startBckWithInterpole(mTeresaModel->mActor, pAnimation, interpolation);
    }

    if (MR::isEqualString(pAnimation, "wait") || MR::isEqualString(pAnimation, "run")) {
        mTeresaModel->_10 = MR::getRandom(60L, 180L);
        MR::stopBtp(mTeresaModel->mActor);
        return;
    }

    mTeresaModel->_10 = 0;
    if (MR::isExistBtp(mTeresaModel->mActor, pAnimation)) {
        MR::startBtp(mTeresaModel->mActor, pAnimation);
        return;
    }

    MR::startBtp(mTeresaModel->mActor, "blink");
}

void MarioActor::runTeresaBaseAnimation() {
    if (!mMario->isStatusActive(MarioStatus_Wait) && !mTeresaModel->mPlayer->isRun("基本")) {
        mTeresaModel->mPlayer->changeAnimationBck("基本");
        mTeresaModel->_10 = MR::getRandom(60L, 180L);
        MR::startBtp(mTeresaModel->mActor, "blink");
    }
}

void MarioActor::updateTeresaAnimation() {
    bool canChange = true;
    if (MR::isBckPlaying(mTeresaModel->mActor, "sleep")) {
        bool moving = false;
        if (mMario->fn_803AC810()) {
            moving = true;
        }

        if (mMario->getStickP() != 0.0f) {
            moving = true;
        }

        if (mMario->mJumpVec.length() > 1.0f) {
            moving = true;
        }

        if (moving) {
            runTeresaBaseAnimation();
            MR::deleteEffect(mTeresaModel->mActor, "Sleep");
        }
    }

    if (MR::isBckOneTimeAndStopped(mTeresaModel->mActor)) {
        runTeresaBaseAnimation();
    }

    if (MR::isBckPlaying(mTeresaModel->mActor, "hit")) {
        canChange = false;
    }

    if (MR::isBckPlaying(mTeresaModel->mActor, "spin") && !mMario->fn_803AC810()) {
        canChange = false;
    }

    if (canChange) {
        if (mMario->fn_803AC810()) {
            if (!MR::isBckPlaying(mTeresaModel->mActor, "fly") && !MR::isBckPlaying(mTeresaModel->mActor, "spin")) {
                changeTeresaAnimation("fly", 16);
            }
        } else if (MR::isBckPlaying(mTeresaModel->mActor, "fly") || mTeresaModel->mPlayer->isRun("基本")) {
            if (getLastMove().dot(*mMario->getGravityVec()) >= 1.0f) {
                changeTeresaAnimation("fall", 16);
            } else if (!((getDrawStates().mWord >> 3) & 1U)) {
                if (MR::abs(getLastMove().dot(*mMario->getGravityVec())) < 1.0f) {
                    runTeresaBaseAnimation();
                }
            }
        } else if (MR::isBckPlaying(mTeresaModel->mActor, "fall")) {
            if (getLastMove().dot(*mMario->getGravityVec()) < 1.0f) {
                runTeresaBaseAnimation();
            }
        }

        if (mTeresaModel->mPlayer->isRun("基本")) {
            mTeresaModel->_C = MR::clamp(mMario->mJumpVec.length() / 10.0f, 0.0f, 1.0f);
            f32 inverseWeight = mTeresaModel->_C;
            inverseWeight = 1.0f - inverseWeight;
            mTeresaModel->mPlayer->changeTrackWeight(0, inverseWeight);
            f32 weight = mTeresaModel->_C;
            mTeresaModel->mPlayer->changeTrackWeight(1, weight);
        }
    }

    if (mMario->_418) {
        if (!MR::isBckPlaying(mTeresaModel->mActor, "spin")) {
            if (mMario->_418 > getConst().getTable()->mTeresaWallThroughTime - 3) {
                changeTeresaAnimation("spin", -1);
            }
        }

        MarioConst* constants = mConst;
        MarioTeresaModel* model = mTeresaModel;
        const MarioConstTable* table = constants->getTable();
        if (model->_4 < table->mTeresaAlphaLevelMax) {
            model->_4 += table->mTeresaAlphaLevelInc;
        }

        mMario->_418--;
        if (!mMario->_418) {
            mMario->playSound("テレサ現れる", -1);
        }
    } else {
        if (mTeresaModel->_4 > 0.0f) {
            mTeresaModel->_4 -= getConst().getTable()->mTeresaAlphaLevelDec;
            if (mMario->mTeresa->_46) {
                mTeresaModel->_4 -= getConst().getTable()->mTeresaAlphaLevelDec;
            }
        }
    }

    f32 alpha = mTeresaModel->_4;
    MR::setBrkFrameAndStop(mTeresaModel->mActor, alpha);
    if (mTeresaModel->_10) {
        mTeresaModel->_10--;
        if (!mTeresaModel->_10) {
            mTeresaModel->_10 = MR::getRandom(90L, 240L);
            MR::startBtp(mTeresaModel->mActor, "blink");
        }
    }
}

void MarioActor::playSound(const char* pName, s32 duration) {
    mMario->playSound(pName, duration);
}

MultiEmitter* MarioActor::playEffect(const char* pName) {
    s32 isBlocked = 0;
    if (_482 || _483) {
        isBlocked = 1;
    }

    if (isBlocked) {
        return nullptr;
    }

    if (isCommonEffect(pName)) {
        return playCommonEffect(pName);
    }

    if (isMaterialEffect(pName)) {
        return playMaterialEffect(pName);
    }

    if (mMario->isPlayerModeYoshi()) {
        MultiEmitter* emitter = MR::tryEmitEffect(mYoshiHolder->mYoshi, pName);
        if (emitter != nullptr) {
            return emitter;
        }
    }

    return MR::emitEffect(this, pName);
}

s32 MarioActor::getFloorMaterialIndex(u32 flags) const {
    s32 materialIndex = 0;

    if (mMarioEffect->_10 == 0) {
        return 2;
    }

    if (_3A8 != 0) {
        return 1;
    }

    s32 floor = mMario->_960;
    if ((floor == 0x0D || floor == 0x1E) && !mMario->isCurrentFloorSand()) {
        floor = mMario->_962;
    }

    if (mMario->isStatusActive(MarioStatus_Wall)) {
        if (mMario->mMovementStates.mLowBits.mBit23) {
            floor = mMario->_964[0];
        } else if (mMario->mMovementStates.mLowBits.mBit6) {
            floor = mMario->_964[1];
        } else if (mMario->mMovementStates.mLowBits.mBit5) {
            floor = mMario->_964[2];
        }
    }

    if (mMario->mDrawStates.mBits.mBit13) {
        floor = 0x17;
    }

    if (mMario->mDrawStates.mBits.mBit12) {
        floor = 0x17;
    }

    if (flags & 0x2) {
        if (mMario->mDrawStates.mBits.mBit2) {
            floor = 0x17;
        }
    }

    if (mMario->_2D0 != 0.0f) {
        floor = 0x1A;
    }

    if (mMario->_A7F != 0) {
        floor = 0x20;
    }

    if (mMario->isPlayerModeBee() && MR::isWallCodeNoAction(mMario->mGroundPolygon)) {
        return -1;
    }

    switch (floor) {
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
        materialIndex = 1;
        break;
    case 0x0E:
    case 0x21:
        return -1;
    case 0x0D:
    case 0x1E:
        materialIndex = 3;
        break;
    case 0x1A:
    case 0x2B:
        materialIndex = 4;
        break;
    case 0x20:
        materialIndex = 5;
        if (strcmp(MR::getSoundCodeString(mMario->_45C), "Honey") == 0) {
            materialIndex = 6;
        }

        break;
    case 0x1B:
    case 0x1C:
        if (strcmp(MR::getSoundCodeString(mMario->_45C), "Sand") == 0) {
            materialIndex = 3;
        }

        break;
    case 0x26:
        materialIndex = 7;
        break;
    default:
        break;
    }

    if (mMario->mDrawStates.mBits.mBit13 || mMario->mDrawStates.mBits.mBit12) {
        materialIndex = 1;
    }

    if (!mMario->mMovementStates.mLowBits.mBit30) {
        if (mMario->mSwim->_1B2 != 0) {
            if (mMario->mSwim->_1B8 < 400.0f) {
                materialIndex = 1;
            }
        }
    }

    return materialIndex;
}

void MarioActor::playEffectTrans(const char* pName, const TVec3f& rTrans) {
    MultiEmitter* emitter = playEffect(pName);
    if (emitter != nullptr) {
        emitter->setGlobalTranslation(rTrans, -1);
    }
}

void MarioActor::playEffectRT(const char* pName, const TVec3f& rDir, const TVec3f& rTrans) {
    if (MR::isNearZero(rDir)) {
        return;
    }

    TVec3f rot;
    TPos3f mtx;
    MR::makeMtxUpNoSupportPos(&mtx, rDir, TVec3f(0.0f, 0.0f, 0.0f));
    MR::makeRTFromMtxPtr(nullptr, &rot, mtx.toMtxPtr(), true);

    MultiEmitter* emitter = playEffect(pName);
    if (emitter != nullptr) {
        emitter->forceFollowOff();
        emitter->setGlobalTranslation(rTrans, -1);
        emitter->setGlobalRotationDegree(rot, -1);
    }
}

void MarioActor::playEffectRTZ(const char* pName, const TVec3f& rDir, const TVec3f& rTrans) {
    if (MR::isNearZero(rDir)) {
        return;
    }

    TVec3f rot;
    TPos3f mtx;
    MR::makeMtxUpNoSupportPos(&mtx, rDir, TVec3f(0.0f, 0.0f, 0.0f));
    MR::makeRTFromMtxPtr(nullptr, &rot, mtx.toMtxPtr(), true);

    MultiEmitter* emitter = playEffect(pName);
    if (emitter != nullptr) {
        emitter->forceFollowOff();
        emitter->setGlobalTranslation(rTrans, -1);
        emitter->setGlobalRotationDegree(rot, -1);
    }
}

void MarioActor::playEffectRTW(const char* pName, const TVec3f& rDir, const TVec3f& rTrans) {
    if (MR::isNearZero(rDir)) {
        return;
    }

    TVec3f rot;
    TPos3f mtx;
    MR::makeMtxUpNoSupportPos(&mtx, rDir, TVec3f(0.0f, 0.0f, 0.0f));
    MR::makeRTFromMtxPtr(nullptr, &rot, mtx.toMtxPtr(), true);

    mEffectRotationFDC = rot;
    mEffectTranslationFD0 = rTrans;
    playEffect(pName);
}

void MarioActor::playEffectSRT(const char* pName, f32 scale, const TVec3f& rDir, const TVec3f& rTrans) {
    if (MR::isNearZero(rDir)) {
        return;
    }

    TVec3f rot;
    TPos3f mtx;
    MR::makeMtxUpNoSupportPos(&mtx, rDir, TVec3f(0.0f, 0.0f, 0.0f));
    MR::makeRTFromMtxPtr(nullptr, &rot, mtx.toMtxPtr(), true);

    MultiEmitter* emitter = playEffect(pName);
    if (emitter != nullptr) {
        emitter->setGlobalScale(scale, -1);
    }

    if (emitter != nullptr) {
        emitter->setGlobalTranslation(rTrans, -1);
    }

    if (emitter != nullptr) {
        emitter->setGlobalRotationDegree(rot, -1);
    }
}

void MarioActor::stopEffect(const char* pName) {
    if (isCommonEffect(pName)) {
        stopCommonEffect(pName);
        return;
    }

    if (isMaterialEffect(pName)) {
        stopMaterialEffect(pName);
        return;
    }

    if (mMario->isPlayerModeYoshi() && MR::tryDeleteEffect(mYoshiHolder->mYoshi, pName)) {
        return;
    }

    MR::deleteEffect(this, pName);
}

void MarioActor::stopEffectForce(const char* pName) {
    if (isCommonEffect(pName)) {
        stopCommonEffect(pName);
        return;
    }

    if (isMaterialEffect(pName)) {
        stopMaterialEffect(pName);
        return;
    }

    if (mMario->isPlayerModeYoshi() && MR::tryForceDeleteEffect(mYoshiHolder->mYoshi, pName)) {
        return;
    }

    MR::forceDeleteEffect(this, pName);
}

void MarioActor::emitEffectWaterColumn(const TVec3f& rDir, const TVec3f& rTrans) {
    if (MR::isNearZero(rDir)) {
        return;
    }

    mWaterColumn->appear();
    MR::startAction(mWaterColumn, "Splash");

    TVec3f rot;
    TPos3f mtx;
    MR::makeMtxUpNoSupportPos(&mtx, rDir, TVec3f(0.0f, 0.0f, 0.0f));
    MR::makeRTFromMtxPtr(nullptr, &rot, mtx.toMtxPtr(), true);

    mWaterColumn->mPosition = rTrans;
    mWaterColumn->mRotation = rot;
    mWaterColumn->calcAnim();

    MR::emitEffect(mWaterColumn, "Splash");
    Mario* player = mMario;
    player->fn_80395400(2);
}

bool MarioActor::checkEffectWaterColumn() {
    if (MR::isActionEnd(mWaterColumn)) {
        mWaterColumn->kill();
        return false;
    }

    return true;
}

void MarioActor::updateRealMtx() {
    updateBaseScaleMtx();
    getRealMtx(mHandLMatrix.toMtxPtr(), "HandL");
    getRealMtx(mHandRMatrix.toMtxPtr(), "HandR");
    PSMTXCopy(MR::getJointMtx(this, "HandL"), mHandLJointMatrix.toMtxPtr());
    PSMTXCopy(MR::getJointMtx(this, "HandR"), mHandRJointMatrix.toMtxPtr());
    PSMTXCopy(MR::getJointMtx(this, "Face0"), mFaceJointMatrix.toMtxPtr());
    getRealMtx(mFootRMatrix.toMtxPtr(), "FootR");
    getRealMtx(mSpine1Matrix.toMtxPtr(), "Spine1");
    getRealMtx(mCapMatrix.toMtxPtr(), "CapPosition");
    getRealMtx(mHipMatrix.toMtxPtr(), "Hip");
    getRealMtx(mSpine2Matrix.toMtxPtr(), "Spine2");
    TVec3f stack_50;
    MR::extractMtxTrans(mSpine2Matrix.toMtxPtr(), &stack_50);
    if (MR::isNan(stack_50)) {
        MR::getJointMtx(this, "Spine2");
        getBaseMtx();
    }

    bool notStatus12 = true;
    if (mMario->isStatusActive(MarioStatus_FpView)) {
        notStatus12 = false;
    }

    if (!notStatus12) {
        return;
    }

    TVec3f stack_44, stack_38;
    MR::extractMtxTrans(mCapMatrix.toMtxPtr(), &stack_44);
    MR::extractMtxTrans(mHipMatrix.toMtxPtr(), &stack_38);
    mBodyUpDirection = stack_44 - stack_38;
    mBodyDownDirection = stack_38 - stack_44;
    if (MR::normalizeOrZero(&mBodyUpDirection)) {
        mBodyUpDirection = mMario->mHeadVec;
    }

    if (MR::normalizeOrZero(&mBodyDownDirection)) {
        mBodyDownDirection = -mMario->mHeadVec;
    }

    if (((mMario->getMovementStates().mLowWord >> 21) & 1U) && ((mMario->mMovementStates.mLowWord >> 30) & 1U)) {
        mBodyDownDirection = -mMario->mHeadVec;
    }

    mCapPosition = stack_44;
}

void MarioActor::updateBaseScaleMtx() {
    if (((mMario->mMovementStates.mHighWord >> 3) & 1U)) {
        PSMTXScale(mJointTransform294.toMtxPtr(), mScale.x, mScale.y, mScale.z);
    } else {
        PSMTXIdentity(mJointTransform294.toMtxPtr());
    }
}

void MarioActor::getRealMtx(MtxPtr pMtx, const char* pName) const {
    MR::multMtx(pMtx, (MtxPtr)&mJointTransform294, MR::getJointMtx(this, pName));
    TVec3f stack_8(mMario->_13C);
    MR::addTransMtx(pMtx, stack_8);
}

void MarioActor::fn_803BDA20(MtxPtr dst, const char* joint) {
    PSMTXConcat(MR::getJointMtx(this, joint), mJointTransform294.mMtx, dst);
}

void MarioActor::getRealPos(const char* pName, TVec3f* pVec) const {
    Mtx mtx;
    getRealMtx(mtx, pName);
    MR::extractMtxTrans(mtx, pVec);
}

MtxPtr MarioActor::getGlobalJointMtx(const char* pName) {
    u16 idx = MR::getJointIndex(this, pName);
    getRealMtx(mJointMatrices2C4[idx], pName);
    return mJointMatrices2C4[idx];
}

void MarioActor::setBlendMtxTimer(u16 a1) {
    if (a1 == 0) {
        _EA0 = 1.0f;
    } else {
        _EA0 = 0.0f;
    }

    mBlendMtxTimer = a1;
}

void MarioActor::forceSetBaseMtx(MtxPtr pMtx) {
    _EA5 = true;
    _1C0 = true;
    PSMTXCopy(pMtx, mRemoteDemoMtx4E4.toMtxPtr());
    MR::extractMtxTrans(pMtx, &mCameraTrans);
    if (_482) {
        MR::extractMtxTrans(pMtx, &mPosition);
    }

    ((TRot3f*)pMtx)->getZDir(_27C);
    ((TRot3f*)pMtx)->getYDir(_270);
    ((TRot3f*)pMtx)->getXDir(_288);
    MR::updateHitSensorsAll(this);
    mMario->invalidateRelativePosition();
    mMario->_8F8.zero();
    mMario->mMovementStates.mHighWord &= 0xEFFFFFFFU;
}

void MarioActor::getStickValue(f32* pStickX, f32* pStickY) {
    *pStickX = 0.0f;
    *pStickY = 0.0f;

    if (mMario->isInputDisable()) {
        return;
    }

    if (((mMario->getDrawStates().mWord >> 24) & 1U)) {
        return;
    }

    if (pStickX != nullptr) {
        *pStickX = MR::getPlayerStickX();
    }

    if (pStickY != nullptr) {
        *pStickY = MR::getPlayerStickY();
    }
}

bool MarioActor::checkButtonType(u16 type, bool isObeyDisable) const {
    if (!isObeyDisable && mMario->isInputDisable()) {
        return false;
    }

    if (_3C0) {
        return false;
    }

    switch (type) {
    case 2:
        if (_F0C || _F00) {
            return true;
        }

        break;

    case 0:
        if (_F0C) {
            return true;
        }

        break;

    case 1:
        if (_F00) {
            return true;
        }

        break;

    case 3:
        if (MR::getPlayerTriggerA()) {
            return true;
        }

        break;

    case 4:
        if (MR::getPlayerTriggerB()) {
            return true;
        }

        break;

    case 7:
        if (MR::getPlayerTriggerZ()) {
            return true;
        }

        break;

    case 8:
        if (MR::getPlayerTriggerC()) {
            return true;
        }

        break;

    case 5:
        if (MR::getPlayerLevelA()) {
            return true;
        }

        break;

    case 6:
        if (MR::getPlayerLevelB()) {
            return true;
        }

        break;

    case 9:
        if (MR::getPlayerLevelZ()) {
            return true;
        }

        break;

    case 10:
        if (MR::getPlayerLevelC()) {
            return true;
        }

        break;
    }

    return false;
}

bool MarioActor::fn_803BDE10() {
    if (this->mMario->isPlayerModeYoshi() && YoshiParam::fn_8041BD40() == 0) {
        return false;
    }
    if (_934 != 0 || this->_38C != 0) {
        return false;
    }
    if (this->_935 != 0) {
        return false;
    }
    return checkButtonType(mInputConfig->getButton0(), false);
}

bool MarioActor::isRequestRush() const {
    if (!_EEB) {
        return false;
    }

    if (_946 != 0) {
        return false;
    }

    if (_EF6 != 0) {
        return false;
    }

    if (!mMario->isEnableRush()) {
        return false;
    }

    if (_935) {
        return false;
    }

    if (mPlayerMode == 5) {
        return false;
    }

    if (!fn_803CB0B0()) {
        return false;
    }

    bool out = checkButtonType(mInputConfig->getButton2(), false);
    if (((mMario->mFlags14.mLowWord >> 18) & 1U)) {
        if (!out) {
            mMario->mFlags14.mLowWord &= 0xFFFBFFFFU;
        }

        out = false;
    }

    return out;
}

bool MarioActor::isRequestThrow() const {
    if (!_EEB) {
        return false;
    }

    if (!mMario->isEnableRush()) {
        return false;
    }

    bool out = checkButtonType(mInputConfig->getButton2(), false);
    if (((mMario->mFlags14.mLowWord >> 18) & 1U)) {
        if (!out) {
            mMario->mFlags14.mLowWord &= 0xFFFBFFFFU;
        }

        out = false;
    }

    return out;
}

bool MarioActor::isRequestSpin() const {
    if (_935) {
        return false;
    }
    return checkButtonType(mInputConfig->getSpinButton(), false);
}

bool MarioActor::isRequestJump() const {
    if (_934 || _38C != 0) {
        return false;
    }

    if (_935) {
        return false;
    }

    return checkButtonType(mInputConfig->getButton6(), false);
}

void* MarioActor::fn_803BE0C0() const {
    return nullptr;
}

bool MarioActor::fn_803BE0D0() {
    return this->checkButtonType(5, false);
}

void* MarioActor::fn_803BE0E0() const {
    return nullptr;
}

void MarioActor::resetPadSwing() {
    _EF8 = 0;
    _EFC = 0;
    _F04 = 0;
    _F08 = 0;
    _F00 = 0;
    _F0C = false;
    _F1C = 10;
    _F1E = 10;
}

bool MarioActor::binderFilter(const Triangle* pTriangle) {
    if (_F48 != nullptr && pTriangle->mSensor == _F48) {
        return true;
    }

    return false;
}

void MarioActor::initBlackHoleOut() {
    mPosRelativeToBlackHole = mPosition - mBlackHolePosition528;

    TVec3f normalisedRelativePos(mPosRelativeToBlackHole);
    TVec3f normalisedRelativeCameraPos = mCameraPosition - mBlackHolePosition528;

    MR::normalizeOrZero(&normalisedRelativeCameraPos);
    MR::normalizeOrZero(&normalisedRelativePos);

    Mtx rotation;
    TVec3f rotateAxis;

    rotateAxis.cross(normalisedRelativeCameraPos, normalisedRelativePos);

    f32 mag = mPosRelativeToBlackHole.length();
    TVec3f killed;
    f32 flt = MR::vecKillElement(mCameraPosition - mPosition, mCameraBasisZ, &killed);
    flt *= mConst->getTable()->mBlackHoleFirstRadius;

    PSMTXRotAxisRad(rotation, &rotateAxis, atan(flt / mag));
    PSMTXMultVec(rotation, &mPosRelativeToBlackHole, &mBlackHoleRotateAxis);
    MR::normalizeOrZero(&mBlackHoleRotateAxis);

    damageDropThrowMemoSensor();
    MR::removeAllClingingKarikari();
}

void MarioActor::exeGameOverBlackHole2() {
    if (MR::isFirstStep(this)) {
        MR::setCubeBgmChangeInvalid();
        MR::clearBgmQueue();

        if (!(mBlackHole->tryStartDemoCamera()) && !((mMario->getMovementStates().mHighWord >> 8) & 1U)) {
            MR::startBlackHoleCamera("ブラックホール", mBlackHolePosition528, mPosition);
        }

        _F44 = false;

        changeAnimationNonStop("ブラックホール落下");
        playEffect("共通ブラックホール");
        playSound("ブラックホール吸い込まれ", -1);
        playEffect(changeMorphString("DieBlackHole"));
        initBlackHoleOut();

        mMario->mMovementStates.mHighWord |= 0x00000008U;

        MR::startStarPointerModeDemoMarioDeath(this);
        MR::endGalaxyCometEvent();
    }

    if (getNerveStep() == 60) {
        if (!MR::getPlayerLeft()) {
            MR::startPlayerEvent("ゲームオーバー");
        } else {
            MR::startPlayerEvent("マリオ奈落");
        }
    }

    if (getNerveStep() == mConst->getTable()->mBlackHoleHideTime) {
        _482 = true;

        MR::hidePlayerTryDeleteEffect();
        MR::emitEffect(this, "ブラックホール消滅");
    }

    f32 nervestepfloat = getNerveStep();
    f32 flt = 1.0f;

    if (nervestepfloat < 180.0f) {
        flt = MR::sqrt(nervestepfloat / 180.0f);
    }

    f32 angle = mConst->getTable()->mBlackHoleRotateSpeed;
    angle = nervestepfloat * angle;
    angle = flt * angle;

    if (angle > mConst->getTable()->mBlackHoleRotateLimit) {
        angle = mConst->getTable()->mBlackHoleRotateLimit;
    }

    Mtx rotationMatrix;
    PSMTXRotAxisRad(rotationMatrix, &mBlackHoleRotateAxis, angle);
    PSMTXMultVec(rotationMatrix, &mPosRelativeToBlackHole, &mPosRelativeToBlackHole);

    MR::vecBlendSphere(mBlackHoleRotateAxis, -mCameraBasisZ, &mBlackHoleRotateAxis, 0.01f);

    f32 distChangeFactor = 180 - getNerveStep();

    if (distChangeFactor < 0.0f) {
        distChangeFactor = 0.0f;
    }

    f32 newDistToBlackHole = mPosRelativeToBlackHole.length() * distChangeFactor / (1 + distChangeFactor);

    mPosRelativeToBlackHole.setLength(newDistToBlackHole);

    f32 scale = getNerveStep() * mConst->getTable()->mBlackHoleScaleSpeed;
    scale = 1 - scale;

    if (scale < mConst->getTable()->mBlackHoleScaleLimit) {
        scale = mConst->getTable()->mBlackHoleScaleLimit;
    }

    mScale.set(scale);

    mPosition = mBlackHolePosition528 + mPosRelativeToBlackHole;

    mVelocity.zero();
}

const char* MarioActor::changeMorphString(const char* name) const {
    const char* result = nullptr;
    if (mActionHash->fn_803DB650(&result, name, gIsLuigi != 0, mPlayerMode) == false) {
        return name;
    }
    return result;
}

XanimeResourceTable* MarioActor::getResourceTable() const {
    return mMarioAnim->mResourceTable;
}

void MarioActor::getFrontHistoryVec(TVec3f* out) {
    mFrontHistory->getFront(out);
}

void MarioActor::fn_803BE650(u32 a) {
    if (a == 0xD) {
        reinterpret_cast<s16&>(this->_948) = 0x13;
    }
}

void MarioActor::requestMovementOn() {
    MR::requestMovementOn(this);
    if (this->mMario->isPlayerModeYoshi()) {
        MR::requestMovementOn(mYoshiHolder->mYoshi);
        mYoshiHolder->mTongueAction->fn_8041FF70();
    }
}

void MarioActor::fn_803BE6D0() {
    if (_B48 != nullptr) {
        this->_B48->clearForce();
    }
}

void MarioActor::fn_803BE6F0() {
    if (_934 != 0) {
        HitSensor* s = _924;
        if (s != nullptr && s->isType(0x65)) {
            this->_B91 = 1;
            this->_B90 = 1;
        }
        if (_934 != 0) {
            if (!_924->receiveMessage(0x95, this->getSensor("Body"))) {
                _924->receiveMessage(0x96, this->getSensor("Body"));
            }
            if (_934 != 0) {
                RushEndInfo info(nullptr, 4, TVec3f(0.0f, 0.0f, 0.0f), false, 0);
                this->endRush(&info);
            }
        }
        _934 = 0;
        _7E2 = 0;
    }
}

bool MarioActor::fn_803BE800() const {
    MarioAllModel* m = mAllModel;
    J3DModel* c = m->mModels[0];
    return m->getJ3DModel() == c;
}

void MarioActor::fn_803BE840(const TVec3f& a) {
    if (this->mMario->isPlayerModeYoshi() && mYoshiHolder->mYoshi->fn_8040CF40()) {
        this->mMario->fn_803B1100();
        mYoshiHolder->mYoshi->fn_8040CD20(a);
        this->setPlayerMode(0, true, true);
    }
}

void MarioActor::fn_803BE8D0(const TVec3f& a) {
    if (this->mMario->isPlayerModeYoshi() && mYoshiHolder->mYoshi->fn_8040CF40()) {
        this->mMario->fn_803B1100();
        mYoshiHolder->mYoshi->fn_8040CDD0(a);
        this->setPlayerMode(0, true, true);
    }
}

void MarioActor::fn_803BE960() {
    if (this->mMario->isPlayerModeYoshi() && mYoshiHolder->mYoshi->fn_8040CF40()) {
        this->mMario->fn_803B1100();
        mYoshiHolder->mYoshi->fn_8040CE80();
        this->setPlayerMode(0, true, true);
    }
}

void MarioActor::fn_803BE9E0() {
    if (this->mMario->isPlayerModeYoshi() && mYoshiHolder->mYoshi->fn_8040CF40()) {
        this->mMario->fn_803B1100();
        mYoshiHolder->mYoshi->fn_8040CED0();
        this->setPlayerMode(0, true, true);
    }
}

void MarioActor::fn_803BEA60(const char* a, const char* b) {
    this->fn_803CAD10(a, b, nullptr);
}

void MarioActor::fn_803BEA70(const char* a, const BckCtrlData* b) {
    this->fn_803CAD10(a, nullptr, b);
}

void MarioActor::fn_803BEA80(const char* pName) {
    if (mHairAndHat1090 != nullptr) {
        mHairAndHat1090->fn_803EC7F0(pName);
    }
}

void MarioActor::fn_803BEAA0(const char* pName, const BckCtrlData& rData) {
    if (mHairAndHat1090 != nullptr) {
        mHairAndHat1090->fn_803EC8A0(pName, rData);
    }
}

void MarioActor::fn_803BEAC0() {
    if (mHairAndHat1090 != nullptr) {
        mHairAndHat1090->fn_803EC780();
    }
    MR::startPlayerNormalBva();
}

void MarioActor::requestHairAndHatMovementOn() {
    if (mHairAndHat1090 != nullptr) {
        MR::requestMovementOn(mHairAndHat1090);
    }
}

void MarioActor::fn_803BEB10(s32 a) {
    TVec3f from = this->_240 - *this->mMario->getGravityVec() * 150.0f;
    if (this->mMario->fn_803B2880(from, *this->mMario->getGravityVec() * 100.0f)) {
        mPressNormal6D8 = -*this->mMario->getGravityVec();
        this->setPress(4, a);
    }
}

bool MarioActor::fn_803BEBE0() const {
    if (_934 == 0) {
        return false;
    }
    if (_924 == nullptr) {
        return false;
    }
    if (MR::getSensorHost(_924) == nullptr) {
        return false;
    }
    const char* name = MR::getSensorHost(_924)->mName;
    if (MR::isEqualString(name, "スピンドライバ")) {
        return true;
    }
    if (MR::isEqualString(name, "スーパースピンドライバー")) {
        return true;
    }
    if (MR::isEqualString(name, "ピンクスーパースピンドライバー")) {
        return true;
    }
    bool isPipe = MR::isEqualString(name, "土管");
    return isPipe;
}

LiveActor* MarioActor::fn_803BECC0() {
    if (this->mMario->isPlayerModeYoshi()) {
        return mYoshiHolder->mYoshi;
    }
    return this;
}

bool MarioActor::fn_803BED10() {
    return this->isNerve(&NrvMarioActor::MarioActorNrvGameOverSink::sInstance);
}

void MarioActor::changeAnimationNonStop(const char* pName) {
    mMario->changeAnimationNonStop(pName);
}

void MarioActor::changeAnimationUpper(const char* pName) {
    if (mMario->mTargetWalkSpeedIndex == 0 && isAnimationRun("基本")) {
        mMario->changeAnimation(pName);
        return;
    }

    mMario->changeAnimationUpper(pName);
}

void MarioActor::stopAnimation(const char* pName) {
    mMario->stopAnimation(pName);
}

bool MarioActor::isAnimationRun(const char* pName) const {
    return mMario->isAnimationRun(pName);
}

// NONMATCHING: verified pooled literal addresses only.
void MarioActor::updateRotationInfo() {
    TRot3f stack_44;
    MR::multMtx(stack_44, mMainJointMatrix4A8, getBaseMtx());
    MR::makeRTFromMtxPtr(nullptr, &mRotation, stack_44, true);
    if (mRotation.z > 90.0f && mRotation.x > 90.0f) {
        f32 diff = 180.0f - mRotation.y;
        mRotation.z = 0.0f;
        mRotation.y = diff;
        mRotation.x = 0.0f;
    }

    if (MR::isSameDirection(mAppliedGravityVec, mMario->mFrontVec)) {
        _A18 = mRotation;
    } else {
        TPos3f stack_14;
        TVec3f stack_8 = -mAppliedGravityVec;
        MR::makeMtxUpFront(&stack_14, stack_8, mMario->mFrontVec);
        MR::makeRTFromMtxPtr(nullptr, &_A18, stack_14, true);
        if (_A18.z > 90.0f && _A18.x > 90.0f) {
            f32 diff = 180.0f - _A18.y;
            _A18.z = 0.0f;
            _A18.y = diff;
            _A18.x = 0.0f;
        }
    }
}

// NONMATCHING: typed count/pointer clears replace redundant vector type-pun; loop registers and scheduling differ.
void MarioActor::initMember() {
    this->mInitializing90 = 0;
    fn_803BF6F0();
    this->_1B4 = 0;
    this->mSwimFogDisabled = 0;
    _1B8 = nullptr;
    mMessenger568 = nullptr;
    this->_1C0 = 0;
    this->_A0C = 0;
    this->_1C3 = 0;
    this->_1C4 = 0;
    mActionHash = nullptr;
    mScreenBlur1074 = new MarioScreenBlur();
    mScreenStrip1084 = new MarioScreenStrip();
    mConditionGroup = new MarioActorConditionGroup(this);
    mSensorSlots108C = new MarioSensorSlots();
    mHairAndHat1090 = nullptr;
    this->_1E0 = 0;
    this->_1E1 = 0;
    mWallShadow574 = nullptr;
    mExtraWallArea578 = nullptr;
    mExtraWallAreaKind57C = 0;
    mCollisionShadow580 = nullptr;
    fn_803CA1F0();
    mMario = nullptr;
    mMarioAnim = nullptr;
    mMarioEffect = nullptr;
    mConst = nullptr;
    gIsLuigi = false;
    this->mAppliedGravityVec.x = 0.0f;
    this->mAppliedGravityVec.y = 1.0f;
    this->mAppliedGravityVec.z = 0.0f;
    this->mPreviousGravityVec.x = 0.0f;
    this->mPreviousGravityVec.y = 1.0f;
    this->mPreviousGravityVec.z = 0.0f;
    mGravityInfo = nullptr;
    mAxisRotationDisabled6A0 = 0;
    this->mGravityRatio = 0.0f;
    this->_334 = 0;
    this->_legacy2AC.zero();
    this->_258.x = 0.0f;
    this->_258.y = 0.0f;
    this->_258.z = 1.0f;
    mLastMove.zero();
    this->_1FC.zero();
    mBinderMotion5D0.zero();
    mBinderDirection5DC.zero();
    mPreviousPosition.zero();
    this->_240.zero();
    this->mCapPosition.zero();
    mVector60C.zero();
    this->_3AA = 0;
    this->_270.zero();
    this->_27C.zero();
    this->_288.zero();
    mCameraTrans.zero();
    this->_2A0.zero();
    mCounter660 = 0;
    mWaterSurfaceCameraTimer662 = 0;
    this->_3C1 = 0;
    this->mFirstTransformCameraLatch666 = 0;
    this->_338 = 0;
    this->_33C.zero();
    this->_348.zero();
    this->_354.zero();
    this->_360.zero();
    this->_378 = 0;
    this->_37C = 0;
    this->mMaxHealth = 0;
    this->mHealth = 0;
    this->_92C = 0;
    this->_B68 = 0;
    this->_39D = 0;
    this->_38C = 0;
    this->_390 = 0;
    this->_394 = 0;
    this->_398 = 0;
    mCounter6C8 = 0;
    this->_3B0 = 0.0f;
    mPressNormal6D8.zero();
    this->_3A8 = 0;
    this->_3AC = 0;
    this->_3C0 = 0;
    this->mRushFront6E8.zero();
    this->_3D0 = 0;
    this->_3D2 = 0;
    mPlayerMode = 0;
    this->_3D6 = 0;
    this->_3D8 = 0;
    this->_3DA = 0;
    this->_3DC = 0;
    this->_3DE = 0;
    this->_3DF = 0;
    _4A4 = nullptr;
    mSensor794 = nullptr;
    this->mWasInMorphLimitArea1065 = 0;
    this->_3E5 = 0;
    this->_3E6 = 0;
    this->_3E7 = 0;
    this->_3E8 = 0;
    _424 = nullptr;
    _468 = 0;
    for (s32 i = 0; i < 4; i++) {
        _428[i] = nullptr;
    }
    _46C = nullptr;
    _470 = nullptr;
    _474 = nullptr;
    this->_478 = 0.0f;
    this->_47C = 0;
    this->_480 = 0;
    mThrowProbePosition76C.zero();
    mCarryWobblePhase778 = 0.0f;
    _494 = nullptr;
    mCarryFixedPosition780 = nullptr;
    mTurnipFixedPosition784 = nullptr;
    mDrillFixedPosition788 = nullptr;
    this->mRushAnimationDelay1054 = 0;
    this->mCarryAnimationPosition1058.zero();
    this->_481 = 0;
    this->_482 = 0;
    this->_483 = 0;
    mSearchLight = nullptr;
    mSearchLightTimer = 0;
    mSearchLightThrowPos = nullptr;
    mNearestUnderSensor798 = nullptr;
    mNearestUnderAngle79C = 0.0f;
    mBodyRadius7A0 = 0.0f;
    mBodyHeight7A4 = 0.0f;
    mBodyUpDirection.zero();
    mBodyDownDirection.zero();
    for (s32 i = 0; i < 128; i++) {
        mPulledSensors7C0[i] = nullptr;
    }
    this->_6D0 = 0;
    mSpinRadius = 0.0f;
    mSpinHeight = 0.0f;
    for (s32 i = 0; i < 128; i++) {
        mSpinNoise[i] = 0.0f;
    }
    for (s32 i = 0; i < 64; i++) {
        mTornadoSensors[i] = nullptr;
    }
    this->_7DC = 0;
    this->_7DE = 0;
    this->_7E0 = 0;
    for (s32 i = 0; i < 64; i++) {
        _7E4[i] = nullptr;
    }
    for (s32 i = 0; i < 64; i++) {
        mRushTouchingDD4[i] = false;
    }
    _924 = nullptr;
    _928 = nullptr;
    mLastRushEndFrameE1C = 0;
    this->_930 = 0;
    _934 = 0;
    _7E2 = 0;
    this->mAnimationDelayE28 = 0;
    this->_935 = 0;
    this->_938.zero();
    this->_944 = 0;
    this->_945 = 0;
    this->_946 = 0;
    this->_94E = 0;
    this->_948 = 0;
    this->_94C = 0;
    this->_950 = 0;
    for (s32 i = 0; i < 8; i++) {
        _954[i] = nullptr;
    }
    _974 = 0;
    this->_978.zero();
    this->_984 = 0.0f;
    this->_988 = 0;
    this->_989 = 0;
    mTornadoMario = nullptr;
    this->_990 = 0;
    _994 = nullptr;
    mJetTurtleShadowE94 = nullptr;
    mTeresaModel = nullptr;
    mModelE9C = nullptr;
    _9C4 = nullptr;
    _9C0 = nullptr;
    this->_9CC = 0.0f;
    this->_9D0 = 0.0f;
    _9D4 = nullptr;
    this->_9D8.zero();
    _9E4 = nullptr;
    _9E8 = nullptr;
    _9EC = nullptr;
    this->_9F0 = 0;
    this->mBeeWallWalk = 0;
    this->_9F2 = 0;
    mGravityReferenceED0.zero();
    mModelEDC = nullptr;
    mModelEE0 = nullptr;
    mModelEE8 = nullptr;
    mModelEEC = nullptr;
    mModelEF4 = nullptr;
    this->mGroundReflectionAlphaEFD = 0;
    mAllModel = new MarioAllModel(this);
    mSimpleModelActorF04 = nullptr;
    mSimpleModelF08 = nullptr;
    this->_A18.zero();
    this->mFlagF18 = 0;
    this->mFlagF19 = 0;
    mModelF1C = nullptr;
    _A10 = nullptr;
    mModelF24 = nullptr;
    mModelF28 = nullptr;
    mHandModelL = nullptr;
    mHandModelR = nullptr;
    this->mShadowHideHandsF34 = 0;
    this->mHandFlagF35 = 0;
    this->mHandFlagF36 = 0;
    this->mFaceJointCountF37 = 0;
    mModelF38 = nullptr;
    this->mFaceJointIndexF3C = 0;
    mActorEffectState = new MarioActorEffectState();
    this->_A6E = 0;
    for (s32 i = 0; i < 8; i++) {
        mBlurMatrices[0][i] = nullptr;
        mBlurMatrices[1][i] = nullptr;
    }
    PSMTXIdentity(mPreviousViewMatrix.toMtxPtr());
    PSMTXIdentity(mCurrentViewMatrix.toMtxPtr());
    mBlurBufferIndex = 0;
    mBlurHistoryIndex = 0;
    mBlurJointCount = 0;
    this->mReflectionPositionF44.zero();
    this->mScreenF50.y = 0.0f;
    this->mScreenF50.x = 0.0f;
    this->mScreenF58.y = 0.0f;
    this->mScreenF58.x = 0.0f;
    this->mScreenF60.y = 0.0f;
    this->mScreenF60.x = 0.0f;
    this->mScreenF68.y = 0.0f;
    this->mScreenF68.x = 0.0f;
    mScreenBuffer = nullptr;
    _B48 = nullptr;
    _B4C = nullptr;
    this->_B50 = 0;
    for (s32 i = 0; i < 5; i++) {
        mFireBalls[i] = nullptr;
    }
    this->mFireBallDelay = 0;
    this->mBlinkF98 = 0;
    this->mEyeTextureFrameF9A = 0;
    this->mBlinkF9C = 0;
    this->mBlinkF9E = 0;
    mEyeRes = nullptr;
    _B7C = nullptr;
    mMaskTextures[0] = nullptr;
    mMaskTextures[1] = nullptr;
    mRandomTextureIndex = 0;
    mNullAnimation = nullptr;
    this->_B90 = 0;
    this->_B91 = 0;
    reinterpret_cast<u8&>(this->_B92) = 0;
    this->_B94 = 0;
    this->_B96 = 0;
    this->mCommonEffectCount = 0;
    this->mTimedCommonEffectCount = 0;
    mTimedCommonEffects = nullptr;
    mMaterialEffectsFC8 = nullptr;
    mWaterColumn = nullptr;
    mEffectTranslationFD0.zero();
    mEffectRotationFDC.zero();
    this->_BC4 = 0;
    PSMTXIdentity(mReflectionBaseMtx264.toMtxPtr());
    PSMTXIdentity(mJointTransform294.toMtxPtr());
    mJointMatrices2C4 = nullptr;
    PSMTXIdentity(mHandLMatrix.toMtxPtr());
    PSMTXIdentity(mHandRMatrix.toMtxPtr());
    PSMTXIdentity(mHandLJointMatrix.toMtxPtr());
    PSMTXIdentity(mHandRJointMatrix.toMtxPtr());
    PSMTXIdentity(mFaceJointMatrix.toMtxPtr());
    PSMTXIdentity(mFootRMatrix.toMtxPtr());
    PSMTXIdentity(mSpine1Matrix.toMtxPtr());
    PSMTXIdentity(mCapMatrix.toMtxPtr());
    PSMTXIdentity(mHipMatrix.toMtxPtr());
    PSMTXIdentity(mSpine2Matrix.toMtxPtr());
    PSMTXIdentity(mMainJointMatrix4A8.toMtxPtr());
    this->mBlendMtxTimer = 0;
    this->_EA0 = 0.0f;
    this->_EA4 = 0;
    this->_EA5 = 0;
    this->_EA6 = 0;
    PSMTXIdentity(mRemoteDemoMtx4E4.toMtxPtr());
    this->mSuperKinokoCollected = 0;
    this->mPowerupCollected = 0;
    this->mTransforming = 0;
    this->_EEB = 0;
    mActionButtonHeld = 0;
    mActionTriggered = 0;
    mJumpButtonHeld = 0;
    this->_EF0 = 0;
    this->_EF2 = 0;
    this->_EF4 = 0;
    this->_EF6 = 0;
    this->_EF8 = 0;
    this->_EFC = 0;
    this->_F00 = 0;
    this->_F04 = 0;
    this->_F08 = 0;
    this->_F0C = 0;
    this->_F20 = 0;
    mInputConfig = nullptr;
    this->_F1C = 0;
    this->_F1E = 0;
    mFlag100E = 0;
    mFlag100F = 0;
    _F24 = nullptr;
    this->_F28 = 0;
    mFrontHistory = nullptr;
    this->_F44 = 0;
    _F48 = nullptr;
    mBlackHole = nullptr;
    mBlackHolePosition528.zero();
    mBlackHoleRotateAxis.zero();
    mPosRelativeToBlackHole.zero();
    mSwimSurfaceState1010 = 0;
    mCameraPosition.zero();
    mCameraBasisX.zero();
    mCameraBasisY.zero();
    mCameraBasisZ.zero();
    mPostureReference1044.zero();
    _FB4 = nullptr;
    this->_FB8 = 0;
    mActionTable1080 = nullptr;
    this->_FCC = 0;
    this->_FCD = 0;
    mFlag1052 = 0;
    mPunchTriangle106C = nullptr;
    mLifeDecreasedThisFrame = 0;
    this->mFlag1064 = 0;
    this->mReflectionSuppressed = 0;
    this->mBlinkRequested1067 = 0;
}

void MarioActor::fn_803BF6F0() {
    mDLchanger = nullptr;
    this->mFirstPersonDrawHidden1A5 = 0;
    for (int i = 0; i < 0x40; i++) {
        mBlurDisplayLists[i] = nullptr;
    }
    mDL[0] = nullptr;
    mDL[1] = nullptr;
    this->mCurrentDLSize1A0 = 0;
    this->mCurrDL = 0;
    this->_1A4 = 0.0f;
    reinterpret_cast<s16&>(this->_1A8) = 0;
    reinterpret_cast<s16&>(this->_1AA) = 0;
    this->_1AC = 0.0f;
    mSwimFogColor.set(0, 0, 0, 0);
}

void MarioActor::fn_803BF760(const JMapInfoIter& iter, s32* anim) {
    MR::getJMapInfoTrans(iter, &this->mPosition);
    MR::getJMapInfoRotate(iter, &this->mRotation);
    this->mScale.x = 1.0f;
    this->mScale.y = 1.0f;
    this->mScale.z = 1.0f;
    this->mRotation.x = MR::repeat(this->mRotation.x, 0.0f, 360.0f);
    this->mRotation.y = MR::repeat(this->mRotation.y, 0.0f, 360.0f);
    this->mRotation.z = MR::repeat(this->mRotation.z, 0.0f, 360.0f);
    if (MR::isExistJMapInfoArg0(iter)) {
        MR::getJMapInfoArg0NoInit(iter, anim);
    }
}

void MarioActor::fn_803BF860() {
    gIsLuigi = false;
    if (MR::isPlayerLuigi()) {
        gIsLuigi = true;
    }
}

void MarioActor::fn_803BF8A0() {
    this->mMario->setHeadAndFrontVecFromRotate(this->mRotation);
    this->mMario->_290 = this->mMario->mSideVec;
}

void MarioActor::fn_803BF8F0() {
    mConst = new MarioConst();
    if (gIsLuigi) {
        mConst->changeTable(1);
    }
}

void MarioActor::fn_803BF950() {
    mGravityInfo = new GravityInfo();
    this->mGravityRatio = 1.0f;
    this->mAppliedGravityVec.x = 0.0f;
    this->mAppliedGravityVec.y = -1.0f;
    this->mAppliedGravityVec.z = 0.0f;
    mPreviousGravityVec = mAppliedGravityVec;
    reinterpret_cast<s16&>(this->_334) = 0;
}

void MarioActor::fn_803BF9D0() {
    this->initBinder(60.0f, 0.0f, 8);
    this->mVector60C.x = 0.0f;
    this->mVector60C.y = 70.0f;
    this->mVector60C.z = 0.0f;
    this->mBinder->_1EC._0 = false;
    MR::setBinderOffsetVec(this, &this->mVector60C, false);
    this->mBinder->setTriangleFilter(MR::createTriangleFilterDelegator(this, &MarioActor::binderFilter));
    this->mBinder->_1EC._3 = true;
    this->_F44 = 1;
    this->_984 = 0.0f;
    this->_978.zero();
    mBinderMotion5D0.zero();
    mBinderDirection5DC.zero();
}

void MarioActor::fn_803BFAE0() {
    this->mFirstTransformCameraLatch666 = 0;
    reinterpret_cast<s16&>(this->_338) = 0;
    mActionHash = new MarioActionHash();
}

void MarioActor::fn_803BFB30() {
    MR::declareGlobalEventCameraAbyss("奈落カメラ");
    MR::declareBlackHoleCamera("ブラックホール");
    MR::declareGlobalEventCameraDead("昇天カメラ", 0.35f, 120, 60);
    MR::declareGlobalEventCamera("水中フォロー");
    MR::declareGlobalEventCamera("水中プラネット");
    MR::declareGlobalEventCamera("水上フォロー");
    MR::declareGlobalEventCamera("引き戻し");
    MR::declareEventCameraProgrammable("変身初出カメラ");
}

void MarioActor::fn_803BFBB0() {
    this->_2A0 = (this->mMario)->mHeadVec;
    this->_270 = this->_2A0;
    mCounter660 = 0;
    mWaterSurfaceCameraTimer662 = 0;
    MR::setGameCameraTargetToPlayer();
}

void MarioActor::fn_803BFC10() {
    FootPrint* fp = new FootPrint("足跡", 0x100, -1);
    this->_B48 = fp;
    this->_B48->setTexture(MR::getTexFromArc("Footprint.bti", this));
}

void MarioActor::fn_803BFC90(s32 a) {
    switch (a) {
    case 1:
        this->mMario->changeAnimation("基本", (const char*)nullptr);
        break;
    case 2:
        this->mMario->changeAnimationNonStop("ウォークイン");
        break;
    default:
        this->mMario->changeAnimation("ステージインA", (const char*)nullptr);
        break;
    }
}

void MarioActor::fn_803BFCE0() {
    mCarryFixedPosition780 = new FixedPosition(this, "HandR", TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    mTurnipFixedPosition784 = new FixedPosition(this, "HandR", TVec3f(1.8f, 52.51f, 39.58f), TVec3f(76.33f, 15.69f, 88.99f));
    mDrillFixedPosition788 = new FixedPosition(this, "HandR", TVec3f(5.59f, 10.85f, -23.16f), TVec3f(90.45f, 19.45f, 67.7f));
    _494 = nullptr;
}

void MarioActor::fn_803BFE50() {
    mBodyRadius7A0 = 35.0f;
    mBodyHeight7A4 = 60.0f;
    mBodyUpDirection.x = 0.0f;
    mBodyUpDirection.y = 1.0f;
    mBodyUpDirection.z = 0.0f;
    mBodyDownDirection = -mBodyUpDirection;
}

void MarioActor::initDrawAndModel() {
    new DrawAdaptor(MR::Functor(this, &MarioActor::drawShadow), (MR::DrawType)0x2d);
    new DrawAdaptor(MR::Functor(this, &MarioActor::drawSilhouette), (MR::DrawType)0x2c);
    new DrawAdaptor(MR::Functor(this, &MarioActor::drawPreWipe), (MR::DrawType)0x49);
    new DrawAdaptor(MR::Functor(this, &MarioActor::drawScreenBlend), (MR::DrawType)0x34);
    new DrawAdaptor(MR::Functor(this, &MarioActor::drawIndirect), (MR::DrawType)0x27);

    if (gIsLuigi) {
        const char* modelName = "Luigi";
        const char* animationName = "MarioAnime";
        initModelManagerWithAnm(modelName, animationName, true);
    } else {
        const char* modelName = "Mario";
        const char* animationName = "MarioAnime";
        initModelManagerWithAnm(modelName, animationName, true);
    }

    mAllModel->registerModel(0, this, false, true);
    _9E4 = nullptr;
    mModelE9C = nullptr;
    _B4C = nullptr;
    _9C0 = nullptr;
    mModelEDC = nullptr;
    mModelEE0 = nullptr;
    mTornadoMario = nullptr;
    mModelEE8 = nullptr;
    mModelEEC = nullptr;
    mModelEF4 = nullptr;
    mCloudMarioHatEF0 = nullptr;
    initFireMario();

    initBeeMario();
    initIceMario();
    initInvincibleMario();
    initHopperMario();
    initTeresaMario();
    _3D2 |= 0x80;
    initBoneMario();
    initTornadoMario();
    initYoshi();
    initCloudMario();
    initRockMario();
    initBlur();

    initShadow();
    initHand();

    initFace();

    mDL[0] = new (0x20) u8[0x100];
    mDL[1] = new (0x20) u8[0x100];
    mCurrDL = 0;

    mDLchanger = new DLchanger(2, 0x100);
    _1A4 = 0.0f;

    mAllModel->fn_803E7BB0(this);
    initBlink();
    createRainbowDL();

    {
        MR::CurrentHeapRestorer restorer(static_cast< JKRHeap* >(MR::getSceneHeapGDDR3()));
        _B7C = new JUTTexture(0x80, 0x40, GX_TF_RGB565);

        for (u32 i = 0; i < 2; i++) {
            mMaskTextures[i] = new JUTTexture(8, 8, GX_TF_IA4);
            mMaskTextures[i]->mWrapS = 1;
            mMaskTextures[i]->mWrapT = 1;
            mMaskTextures[i]->mMinType = 0;
            mMaskTextures[i]->mMagType = 0;
            mMaskTextures[i]->init();
        }

        mRandomTextureIndex = 0;
        updateRandomTexture(1000.0f);
        updateRandomTexture(1000.0f);
    }

    initScreenBox();
    MR::startBtp(this, "ElementEnd");
}

void MarioActor::drawIndirect() const {
    drawModelBlur();
    drawIndirectModel();
    if (((mMario->mFlags20.mWord >> 28) & 1U)) {
        mScreenStrip1084->fn_803DBB10(_37C, mMario->mSwim->mColdWaterDamageInterval);
    }
    mScreenStrip1084->fn_803DBAA0(_37C);
    drawSpinEffect();
}

// NONMATCHING: compiler merges equivalent Face0 branches; verified pooled string addresses.
void MarioActor::drawMarioModel() const {
    if (isAllHidden()) {
        return;
    }

    if (mTornadoMario != nullptr) {
        if (((mMario->mMovementStates.mLowWord >> 16) & 1U)) {
            if (mMario->_544 > 2) {
                return;
            }
        }

        if (!MR::isDead(mTornadoMario) &&
            (MR::isBckPlaying(mTornadoMario, "MarioTornadoEnd") || MR::isBckPlaying(mTornadoMario, "MarioTornadoLoop"))) {
            return;
        }
    }

    switch (mAllModel->mCurrentModel) {
    case 3:
        return;
    default:
        break;
    }

    drawSphereMask();
    bool res = drawDarkMask();

    J3DModelX* model = static_cast<J3DModelX*>(mAllModel->getJ3DModel());

    if (res) {
        model->mDisplayListMask |= (1U << 15);
    }

    if (!mFlags.mIsNoEntryDrawBuffer) {
        model->mDisplayListMask &= ~(1U << 3);
        if (mMario->isStatusActive(MarioStatus_FpView)) {
            if (mFirstPersonDrawHidden1A5) {
                return;
            }

            mDLchanger->addDL(model);

            MR::showJoint(model, "HandR0");
            MR::showJoint(model, "HandL0");
            MR::showJoint(model, "Face0");
        } else {
            if (mAllModel->mCurrentModel == 4) {
                model->setDynamicDL(nullptr, 0);
            } else {
                model->setDynamicDL(mDL[mCurrDL], mCurrentDLSize1A0);
            }
        }

        if (mBeeWallWalk != 0) {
            GXSetAlphaUpdate(GX_TRUE);
            GXSetDstAlpha(GX_TRUE, 0);
        }

        model->setDrawView(0);
        model->directDraw(nullptr);
        model->mDisplayListMask = 0;
    }

    if (mMario->isStatusActive(MarioStatus_FpView)) {
        MR::hideJoint(model, "HandR0");
        MR::hideJoint(model, "HandL0");
        if (mAllModel->mCurrentModel >= 7) {
            MR::hideJoint(model, "Face0");
        } else {
            MR::hideJoint(model, "Face0");
        }
    }

    drawHand();

    if (!MR::isHiddenModel(mModelF38)) {
        J3DModelX* cool = static_cast< J3DModelX* >(MR::getJ3DModel(mModelF38));
        cool->setDynamicDL(mDL[mCurrDL], mCurrentDLSize1A0);
        cool->directDraw(nullptr);
    }

    if (mBeeWallWalk != 0) {
        GXSetAlphaUpdate(GX_FALSE);
        GXSetDstAlpha(GX_FALSE, 0);
    }
}

bool MarioActor::isAllHidden() const {
    bool hidden = false;
    if (_482 || _483 || _481) {
        hidden = true;
    }

    return hidden;
}

void MarioActor::drawIndirectModel() const {
    if (isAllHidden()) {
        return;
    }

    drawReflectModel();

    if (mTornadoMario != nullptr) {
        if (((mMario->mMovementStates.mLowWord >> 16) & 1U)) {
            if (mMario->_544 > 2) {
                return;
            }
        }

        if (!MR::isDead(mTornadoMario) &&
            (MR::isBckPlaying(mTornadoMario, "MarioTornadoEnd") || MR::isBckPlaying(mTornadoMario, "MarioTornadoLoop"))) {
            return;
        }
    }

    switch (mAllModel->mCurrentModel) {
    case 3:
        break;
    default:
        return;
    }

    J3DModelX* model = static_cast<J3DModelX*>(mAllModel->getJ3DModel());
    if (!mFlags.mIsNoEntryDrawBuffer) {
        model->mDisplayListMask &= ~(1U << 3);
        if (mMario->isStatusActive(MarioStatus_FpView)) {
            if (mFirstPersonDrawHidden1A5) {
                return;
            }

            mDLchanger->addDL(model);
        } else {
            model->setDynamicDL(mDL[mCurrDL], mCurrentDLSize1A0);
        }

        MR::showJoint(model, "Face0");
        model->setDrawView(0);
        model->directDraw(nullptr);
        model->mDisplayListMask = 0;
    }

    drawIndirectHand();
}

void MarioActor::drawModelBlur() const {
    if (isAllHidden()) {
        return;
    }

    J3DModelX* model = static_cast<J3DModelX*>(mAllModel->getJ3DModel());
    if (!model->_1E4) {
        model->_1E5 = true;
        return;
    }

    model->_1E4 = false;
    if (!_A6E) {
        return;
    }

    model->mDisplayListMask |= (1U << 12);

    Mtx inv;
    PSMTXInverse(mPreviousViewMatrix.mMtx, inv);
    MR::multMtx(inv, inv, MR::getCameraViewMtx());
    for (u32 i = 1; i < 8; i++) {
        const u32 idx = static_cast< u32 >(i + mBlurHistoryIndex) & 7;
        model->setDrawViewBuffer(mBlurMatrices[mBlurBufferIndex][idx][0]);

        if (!_A0C) {
            for (u32 joint = 0; joint < mAllModel->getJ3DModel()->mModelData->getDrawMtxNum(); joint++) {
                MR::multMtx(mBlurMatrices[mBlurBufferIndex][idx][joint], mBlurMatrices[1 - mBlurBufferIndex][idx][joint], inv);
            }
        }

        J3DModel* currentModel = mAllModel->getJ3DModel();
        u16 bufferIndex = mBlurBufferIndex;
        J3DModelData* modelData = currentModel->mModelData;
        DCStoreRange(mBlurMatrices[bufferIndex][idx], modelData->getDrawMtxNum() * sizeof(Mtx));

        mBlurDisplayLists[idx + ((i - 1) << 3)]->addDL(model);
        model->directDraw(nullptr);
    }

    model->mDisplayListMask &= ~(1U << 12);
}

// NONMATCHING: equivalent pooled literals and r29/r30 allocation.
void MarioActor::drawReflectModel() const {
    if (isAllHidden()) {
        return;
    }

    if (mReflectionSuppressed) return;
    if (!mWaterReflectionEF8 && !mSphereReflectionEF9 && !mGroundReflectionEFC) return;
    if (mSphereReflectionEF9 && mFlagF18 < 2) return;
    if (mWaterReflectionEF8 && mFlagF19 < 2) return;

    if (isUseScreenBox()) {
        captureScreenBox();
    }

    J3DModelX* model = static_cast<J3DModelX*>(mAllModel->getJ3DModel());

    MR::showJoint(model, "HandR0");
    MR::showJoint(model, "HandL0");
    MR::showJoint(model, "Face0");

    if (mAllModel->mCurrentModel != 1 && mAllModel->mCurrentModel != 6) {
        MR::hideMaterial(model, "EyeLid_v");
    }

    if (getCarrySensor() != nullptr) {
        if (isUseScreenBox()) {
            mDLchanger->addDL(static_cast< J3DModelX* >(MR::getJ3DModel(mJetTurtleShadowE94)));
        }

        mJetTurtleShadowE94->drawType0();
    }

    model->mDisplayListMask = 0;

    if (mGroundReflectionEFC) {
        GXSetZScaleOffset(1.0f, 0.00001f);
        model->mDisplayListMask |= (1U << 5);
        model->setDrawView(3);
        model->directDraw(nullptr);
        model->mDisplayListMask &= ~(1U << 5);
        model->mDisplayListMask |= (1U << 2);
        mDLchanger->addDL(model);
    } else {
        if (isUseScreenBox()) {
            mDLchanger->addDL(model);
        }

        model->mDisplayListMask |= (1U << 11);
    }

    GXSetZScaleOffset(1.0f, 0.0f);
    model->setDrawView(3);
    model->directDraw(nullptr);
    model->mDisplayListMask = 0;

    MR::hideJoint(model, "HandR0");
    MR::hideJoint(model, "HandL0");

    if (mAllModel->mCurrentModel != 4) {
        MR::hideJoint(model, "Face0");
    }

    if (mAllModel->mCurrentModel != 1 && mAllModel->mCurrentModel != 6) {
        MR::showMaterial(model, "EyeLid_v");
    }

    if (isUseScreenBox()) {
        writeBackScreenBox();
    }
}

// NONMATCHING: register allocation and verified pooled string address hoisting.
void MarioActor::drawShadow() const {
    bool hidden = _482 || _481;
    if (hidden) {
        return;
    }

    if (!mShadowEnabledEFA) {
        return;
    }

    if (mPlayerMode == PlayerMode_Teresa) {
        return;
    }

    if (mMario->isStatusActive(MarioStatus_FpView) && mFirstPersonDrawHidden1A5) {
        return;
    }

    if (_924 != nullptr) {
        const char* effect = "影を消す";
        if (mActionTable1080->testEffect(MR::getSensorHost(_924)->mName, effect)) return;
    }

    if (((mMario->mMovementStates.mLowWord >> 16) & 1U) && mMario->_544 > 1) {
        return;
    }

    J3DModelX* pSimple = mSimpleModelF08;
    TPos3f matrix;
    PSMTXScale(matrix.toMtxPtr(), mScale.x, mScale.y, mScale.z);
    PSMTXConcat(matrix.toMtxPtr(), mReflectionBaseMtx264.mMtx, matrix.toMtxPtr());
    PSMTXConcat(mAllModel->getJ3DModel()->getBaseTRMtx(), matrix.toMtxPtr(), pSimple->getBaseTRMtx());
    mCollisionShadow580->calcView(pSimple, 2, static_cast<J3DModelX*>(mAllModel->getJ3DModel()));
    if (mMario->mSinkTimer > 180) {
        return;
    }

    if (mMario->mSinkTimer > 128) {
        MR::hideJointAndChildren(pSimple, "Hip");
    }

    if (mMario->mSinkTimer > 150) {
        MR::hideJointAndChildren(pSimple, "ShoulderL");
        MR::hideJointAndChildren(pSimple, "ShoulderR");
    }

    if (mShadowHideHandsF34 || mMario->mSinkTimer > 120) {
        MR::hideJoint(pSimple, "HandL0");
        MR::hideJoint(pSimple, "HandR0");
    }

    mCollisionShadow580->drawAndCaptureTex(pSimple, mPosition);
    mCollisionShadow580->clearAlphaBuffer();
    if (mBeeWallWalk) {
        mCollisionShadow580->_307 = 0;
    } else {
        mCollisionShadow580->_307 = 1;
    }

    mCollisionShadow580->draw();
    MR::showJointAndChildren(pSimple, "JointRoot");
}

void MarioActor::drawSilhouette() const {
    if (this->isSilhouetteVisible()) {
        J3DModelX* m = mSimpleModelF08;
        m->setDrawView(1);
        m->mDisplayListMask = 0;
        m->mDisplayListMask |= (1U << 3);
        m->directDraw(nullptr);
        m->mDisplayListMask &= ~(1U << 3);
        GXSetAlphaUpdate(GX_TRUE);
        GXSetColorUpdate(GX_TRUE);
        GXSetDstAlpha(GX_TRUE, 0);
    }
}

// NONMATCHING: equivalent nonnegative timer shifts and argument-address scheduling.
void MarioActor::drawPreWipe() const {
    if (mMario->isPlayerModeYoshi()) {
        return;
    }
    if (!_1C3) {
        if (!mActorEffectState->mActive) {
            return;
        }
        f32 width = MR::getScreenWidth();
        f32 height = MR::getScreenHeight();
        TDDraw::setup(0, 1, 2);
        TVec3f position(0.0f, 0.0f, 0.0f);
        TVec3f size(width, height, 0.0f);
        u16 timer = mActorEffectState->mTimer;
        TDDraw::drawFillBox(position, size,
                            (timer <= 0x80 ? (timer >> 1) + timer + (static_cast<s32>(timer) >> 2) : 0xE0U) | 0x28282800U);
    }
    MR::loadViewMtx();
    MR::loadProjectionMtx();
    MR::loadActorLight(this);
    drawMarioModel();
    MR::drawInitFor2DModel();
}

void MarioActor::updateAlphaDL(u8 alpha) {
    u8 buffer[256] ATTRIBUTE_ALIGN(32);
    GDLObj displayList;
    MR::ProhibitSchedulerAndInterrupts lock(false);
    GDInitGDLObj(&displayList, buffer, sizeof(buffer));
    __GDCurrentDL = &displayList;
    GXColor color = {0, 0, 0, 255};
    color.a = alpha;
    GDSetTevColor(GX_TEVREG2, color);
    GDSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GDSetGenMode2(1, 0, 1, 0, GX_CULL_BACK);
    GDSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GDSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GDSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GDSetTevAlphaCalcAndSwap(GX_TEVSTAGE0, GX_CA_A2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV,
                             GX_TEV_SWAP0, GX_TEV_SWAP0);
    GDSetTevColorCalc(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GDSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GDPadCurr32();
    u32 size = (displayList.ptr - displayList.start + 31) & ~31;
    DLholder* holder = mDLchanger->swap();
    holder->mSize = displayList.ptr - displayList.start;
    MR::copyMemory(holder->mDL, buffer, size);
    DCStoreRange(holder->mDL, size);
}

void MarioActor::updateSimpleAlphaDL(u8 alpha) {
    u8 buffer[256] ATTRIBUTE_ALIGN(32);
    GDLObj displayList;
    MR::ProhibitSchedulerAndInterrupts lock(false);
    GDInitGDLObj(&displayList, buffer, sizeof(buffer));
    __GDCurrentDL = &displayList;
    GDSetDstAlpha(GX_TRUE, alpha);
    GDPadCurr32();
    u32 size = (displayList.ptr - displayList.start + 31) & ~31;
    DLholder* holder = mDLchanger->swap();
    holder->mSize = displayList.ptr - displayList.start;
    MR::copyMemory(holder->mDL, buffer, size);
    DCStoreRange(holder->mDL, size);
}

void MarioActor::createRainbowDL() {
    DLholder* holder;
    u8 buffer[256] ATTRIBUTE_ALIGN(32);
    GDLObj displayList;
    MR::ProhibitSchedulerAndInterrupts lock(false);

    for (u32 alpha = 0; alpha < 8; alpha++) {
        for (u32 index = 0; index < 8; index++) {
            mBlurDisplayLists[alpha * 8 + index] = new DLchanger(1, 32);
            GDInitGDLObj(&displayList, buffer, sizeof(buffer));
            __GDCurrentDL = &displayList;
            GXColor color = {0, 0, 0, 0};

            if (index & 1) {
                color.r = 255;
            }

            if (index & 2) {
                color.g = 255;
            }

            if (index & 4) {
                color.b = 255;
            }

            if (!index) {
                color.r = 64;
                color.g = 64;
                color.b = 64;
            }

            color.a = 16 + (alpha + 1) * 24;
            GDSetTevColor(GX_TEVREG0, color);
            GDPadCurr32();
            u32 length = displayList.ptr - displayList.start;
            u32 size = (length + 31) & ~31;
            DLchanger* changer = mBlurDisplayLists[alpha * 8 + index];
            holder = &changer->mBuffers[changer->mCurrentBuffer];
            holder->mSize = length;
            MR::copyMemory(holder->mDL, buffer, size);
            DCStoreRange(holder->mDL, size);
        }
    }
}

void MarioActor::updateReflectAlphaDL(u8 alpha) {
    u8 buffer[256] ATTRIBUTE_ALIGN(32);
    GDLObj displayList;
    MR::ProhibitSchedulerAndInterrupts lock(false);
    GDInitGDLObj(&displayList, buffer, sizeof(buffer));
    __GDCurrentDL = &displayList;
    GXColor color = {0, 0, 0, 255};
    color.a = alpha;
    GDSetTevColor(GX_TEVREG2, color);
    GDSetTevAlphaCalcAndSwap(GX_TEVSTAGE0, GX_CA_A2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV,
                             GX_TEV_SWAP0, GX_TEV_SWAP0);
    GDPadCurr32();
    u32 size = (displayList.ptr - displayList.start + 31) & ~31;
    DLholder* holder = mDLchanger->swap();
    holder->mSize = displayList.ptr - displayList.start;
    MR::copyMemory(holder->mDL, buffer, size);
    DCStoreRange(holder->mDL, size);
}

// NONMATCHING: register allocation and scheduling of display-list buffer bookkeeping.
void MarioActor::updateLightDL(const Color8& rAmbient, const Color8& rMaterial, const Color8& rFog, f32 strength) {
    u8 buffer[512] ATTRIBUTE_ALIGN(32);
    GDLObj displayList;
    MR::ProhibitSchedulerAndInterrupts lock(false);
    GDInitGDLObj(&displayList, buffer, sizeof(buffer));
    __GDCurrentDL = &displayList;
    GXColor ambientColor = {rAmbient.r, rAmbient.g, rAmbient.b, rAmbient.a};
    GDSetChanAmbColor(GX_COLOR0A0, ambientColor);
    GXColor materialColor = {rMaterial.r, rMaterial.g, rMaterial.b, rMaterial.a};
    GDSetChanMatColor(GX_COLOR0A0, materialColor);
    GDSetChanCtrl(GX_COLOR1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);

    if (!MR::isNearZero(strength)) {
        f32 nearZ = MR::getNearZ();
        f32 farZ = MR::getFarZ();
        GXColor color = {rFog.r, rFog.g, rFog.b, rFog.a};
        f32 start;
        f32 end;
        MR::calcFogStartEnd(mPosition, strength, &start, &end);
        GDSetFog(GX_FOG_LIN, start, end, nearZ, farZ, color);
    }

    GDPadCurr32();
    u32 length = displayList.ptr - displayList.start;
    mCurrentDLSize1A0 = length;
    mCurrDL = 1 - mCurrDL;
    u32 size = (length + 31) & ~31;
    MR::copyMemory(mDL[mCurrDL], buffer, size);
    DCStoreRange(mDL[mCurrDL], size);
}

// NONMATCHING: Matrix-address reuse, equivalent bit tests, and boolean-store scheduling.
// NONMATCHING: initial zero-register reuse, load scheduling and literal identities.
void MarioActor::movement() {
    _46C = nullptr;
    ++_378;
    _1E1 = false;
    mLifeDecreasedThisFrame = 0;
    mNearestUnderAngle79C = 0.7853981852531433f;
    mNearestUnderSensor798 = nullptr;
    PSMTXCopy(mCurrentViewMatrix.toMtxPtr(), mPreviousViewMatrix.toMtxPtr());
    PSMTXCopy(MR::getCameraViewMtx(), mCurrentViewMatrix.toMtxPtr());
    updateCameraInfo();
    mMarioAnim->fn_803DFAC0();
    mMarioAnim->fn_803DFB70();
    LiveActor::movement();
    fn_803C1940();
    if ((mMario->mDrawStates.mWord >> 5) & 1U) {
        mMario->mDrawStates.mWord &= ~0x20;
        mMarioAnim->update();
    }
    mMario->recordRelativePosition();
    updateTransForCamera();
    calcAnimInMovement();
    _935 = false;
    mMario->_2D0 = 0.0f;
    mFrontHistory->update();
    if (mMario->isPlayerModeYoshi()) {
        if (isSilhouetteVisible()) {
            mYoshiHolder->mYoshi->mRideModelVisible = true;
        } else {
            mYoshiHolder->mYoshi->mRideModelVisible = false;
        }
    }
    mSensorSlots108C->fn_803DAB90();
}

// NONMATCHING: Equivalent typed-member address reuse and saved-register allocation.
void MarioActor::fn_803C1940() {
    TVec3f a(this->mPosition);
    a -= this->mPreviousPosition;
    TVec3f b(a);
    b -= mVelocity;
    mBinderMotion5D0 = a;
    TVec3f c(mBinderDirection5DC);
    mBinderDirection5DC = b;
    this->fn_803C1AF0(&mBinderDirection5DC, &c);
    if (!(PSVECMag((Vec*)&b) <= 0.1f)) {
        if (!((this->mMario->mMovementStates.mLowWord >> (0x15)) & 1) && !MR::isNearZero(mVelocity, 0.001f)) {
            TVec3f d(this->mPreviousPosition);
            d -= this->_1FC;
            f32 m1 = PSVECMag((Vec*)&d);
            f32 m2 = PSVECMag((Vec*)&mVelocity);
            if (PSVECMag((Vec*)&b) > 2.0f * (m1 + m2)) {
                this->mMario->stopWalk();
            }
        }
        if (((this->mMario->mMovementStates.mHighWord >> (0x1C)) & 1)) {
            this->fn_803D8BD0(&a, &b);
        }
        if (getMovementStates().mLowBits.mBit31 && this->mBeeWallWalk == 0) {
            this->fn_803D8E30(&b);
            return;
        }
        if (mMario->mMovementStates.mLowBits.mBit30) {
            this->fn_803D9170(&a);
        }
    }
}

void MarioActor::fn_803C1AF0(TVec3f* a, TVec3f* b) {
    if (MR::isOppositeDirection(*a, *b, 0.01f)) {
        f32 m1 = PSVECMag((Vec*)a);
        f32 m2 = PSVECMag((Vec*)b);
        if (!MR::isNearZero(m1, 0.001f) && !MR::isNearZero(m2, 0.001f) && MR::isNearZero(m1 - m2, 1.0f)) {
            TVec3f t(*a);
            t.scale(0.5f);
            this->mPosition -= t;
        }
    }
}

void MarioActor::control() {
    if (!fn_803C1C60()) fn_803C1D30();
    Mario* mario = mMario;
    mario->mStateFlags._0 = false;
    mario->mStateFlags._1 = false;
    mario->mStateFlags._2 = false;
    if (fn_803C1E40()) fn_803C1E80();
    else control2();
    mPreviousPosition = mPosition;
}

bool MarioActor::fn_803C1C60() {
    if (this->mSuperKinokoCollected == 0) {
        return false;
    }
    if (!MR::tryStartDemoWithoutCinemaFrame(this, "マリオスーパー化")) {
        return false;
    }
    this->mSuperKinokoCollected = 0;
    this->changeMaxLife(6);
    MR::stopAnimFrame(this);
    if (this->mMario->isPlayerModeYoshi()) {
        MR::stopAnimFrame(mYoshiHolder->mYoshi);
    }
    MR::requestPowerUpHPMeter();
    this->mMario->startPadVib("マリオ[変身]");
    MR::startSystemSE("SE_SY_SUPER_KINOKO_GET", -1, -1);
    this->_3DA = 0x5A;
    return true;
}

bool MarioActor::fn_803C1D30() {
    if (this->mPowerupCollected == 0) {
        return false;
    }
    if (!MR::tryStartDemoWithoutCinemaFrame(this, "マリオ変身")) {
        return false;
    }
    this->mTransforming = 1;
    this->mPowerupCollected = 0;
    this->_3D8 = 0x40;
    if (mPlayerMode == 6) {
        this->fn_803BB4F0();
        this->changeTeresaAnimation("change", -1);
    }
    if (mPlayerMode == 0xA) {
        MR::requestMovementOn(this);
        MR::requestMovementOn(mYoshiHolder->mYoshi);
        this->_3D8 = 0x13;
        this->mMario->clearVelocity();
        (this->mMario)->mWalkSpeed = 0.0f;
    }
    if (mPlayerMode != 0xA) {
        MR::stopAnimFrame(this);
        this->playEffect("変身");
        this->mMario->startPadVib("マリオ[変身]");
    }
    return true;
}

bool MarioActor::fn_803C1E40() {
    bool r = false;
    if ((this->_3D8 != 0 && mPlayerMode != 0) || this->_3DA != 0) {
        r = true;
    }
    return r;
}

void MarioActor::fn_803C1E80() {
    (this->mMario)->_350.zero();
    (this->mMario)->_35C.zero();
    reinterpret_cast<s16&>(this->_7DC) = 0;
    this->_930 = 0;
    mVelocity.zero();
    mLastMove.zero();
    this->_1FC = this->mPosition;
    if (this->fn_803C1F30()) {
        this->mMario->mPosHolder->fn_80405F90(this->mPosition);
        this->mMario->checkEnforceMove();
        this->mPosition = this->mMario->mPosHolder->mPos;
    }
}

bool MarioActor::fn_803C1F30() const {
    MtxPtr prev;
    Mario* m = this->mMario;
    bool r = false;
    if (((m->mMovementStates.mLowWord >> (0x1E)) & 1)) {
        prev = m->_45C->getPrevBaseMtx()->mMtx;
        if (!MR::isSameMtx((this->mMario)->_45C->getBaseMtx()->mMtx, prev)) {
            r = true;
        }
    }
    return r;
}

void MarioActor::control2() {
    fn_803C2010();
    controlMain();
    fn_803C2030();
    if (mMario->isDamaging()) {
        _424 = nullptr;
    }
}

void MarioActor::fn_803C2010() {
    u16 t = this->_BC4;
    if (t != 0) {
        this->_BC4 = t - 1;
    }
}

void MarioActor::fn_803C2030() {
    if (this->_EA4 == 0) {
        if (this->_F44 == 0) {
            MR::offBind(this);
        } else {
            MR::onBind(this);
        }
    }
    this->_978 = mVelocity;
    if (((this->mMario->mDrawStates.mWord >> (0x14)) & 1)) {
        this->mBinder->_1EC._1 = false;
        return;
    }
    this->mBinder->_1EC._1 = true;
}

void MarioActor::controlMain() {
    if (_3C0) {
        _7DC = 0;
        _930 = 0;
    }

    updateActionTrigger();
    updateControllerSwing();
    updateControllerSwingLeft();
#ifdef GALAXY_NATIVE
    // OdysseyMovement: the swing's rising edge for the mod's Spin moves, every frame
    // (updateSwingAction does not run in every action, e.g. a ground pound).
    if (petari_mod_enabled(2)) {
        namespace Live = PetariNative::Odyssey::Live;
        static bool sLastCore = false, sLastSub = false;
        const bool core = MR::isCorePadSwing(0);
        const bool sub = MR::isSubPadSwing(0);
        if ((core && !sLastCore) || (sub && !sLastSub)) {
            Live::spinBuffer = Live::SpinBufferFrames;
            static const bool trace = std::getenv("PETARI_ODYSSEY_TRACE") != nullptr;
            if (trace) {
                std::fprintf(stderr, "[odyssey] swing edge (core %d sub %d): jumping %d hip drop %d ground %d\n", static_cast< int >(core),
                             static_cast< int >(sub), static_cast< int >(isJumping()),
                             static_cast< int >((mMario->mMovementStates.mLowWord >> 20) & 1U),
                             static_cast< int >((mMario->mMovementStates.mLowWord >> 30) & 1U));
            }
        } else if (Live::spinBuffer > 0) {
            --Live::spinBuffer;
        }
        sLastCore = core;
        sLastSub = sub;
        static const bool traceSwing = std::getenv("PETARI_ODYSSEY_TRACE_SWING") != nullptr;
        if (traceSwing && !((mMario->mMovementStates.mLowWord >> 30) & 1U)) {
            std::fprintf(stderr, "[odyssey] air swing: core %d sub %d F00 %d F0C %d F1C %d buffer %d hip drop %d\n", static_cast< int >(core),
                         static_cast< int >(sub), static_cast< int >(_F00), static_cast< int >(_F0C), static_cast< int >(_F1C),
                         Live::spinBuffer, static_cast< int >((mMario->mMovementStates.mLowWord >> 20) & 1U));
        }
    }
    petariCappyInput(this);
#endif
    update2D();
    if (mActorEffectState->mActive) {
        mActorEffectState->mTimer++;
    }

    if (!isEnableMoveMario()) {
        _7DC = 0;
        _930 = 0;
        careMovingObject();
    } else {
        mVelocity.zero();
        updateBehavior();
        updateRotationInfo();
        updateTakingPosition();
        updateSearchLight();
        updateThrowing();
        updateBeeWingAnimation();
        updateFairyStar();
        _930 = 0;
    }
#ifdef GALAXY_NATIVE
    petariCappyUpdate(this);
#endif
}

void MarioActor::updateBehavior() {
    fn_803C34F0();
    fn_803CBC50();
    fn_803C2320();
    fn_803C23B0();
    updateBindRatio();
    updateEffect();
    fn_803C2520();
    fn_803C2590();
    mMario->fn_803B2610();
    if (!doPressing() && !doStun() && !doRush()) {
        updateGravityVec(false, false);
        if (!tryJumpRush() && !tryThrow() && !tryStandardRush()) {
            if (((mMario->mMovementStates.mLowWord >> 30) & 1U)) {
                _988 = 0;
                _989 = 0;
            }
            updateSwingTimer();
            updateSwingAction();
            updateThrowVector();
            if (!checkClapCatchStart()) {
                _7DC = 0;
                mMario->Mario::update();
                mFlag1064 = 0;
                updateForCamera();
                updateTornado();
                tryCoinPull();
                mMario->mDrawStates.mWord |= 0x00000020U;
            }
        }
    }
}

void MarioActor::fn_803C2320() {
    this->_37C += 1;
    if (this->_EF6 != 0) {
        this->_EF6 -= 1;
    }
    if (this->_3A8 != 0) {
        this->_3A8 -= 1;
    }
    if (this->_3AC != 0) {
        this->_3AC -= 1;
    }
    if (this->mAnimationDelayE28 != 0) {
        this->mAnimationDelayE28 -= 1;
    }
    if (this->mRushAnimationDelay1054 != 0) {
        this->mRushAnimationDelay1054 -= 1;
        if (this->mRushAnimationDelay1054 == 0) {
            this->_480 = 1;
            this->mMarioAnim->fn_803DDF60();
        }
    }
}

void MarioActor::fn_803C23B0() {
    TVec3f d(this->mPosition);
    d -= this->_1FC;
    mLastMove = d;
    this->_1FC = this->mPosition;
}

void MarioActor::updateBindRatio() {
    if (!_934 && !MR::isNearZero(_978 - mLastMove)) {
        f32 mag = _978.length();
        if (mag / (_978 - mLastMove).length() < 2.0f) {
            _984 += 0.1f;
        } else {
            _984 -= 0.01f;
        }
    } else if (!MR::isNearZero(_978)) {
        _984 -= 0.01f;
    }

    _984 = MR::clamp(_984, 0.0f, 1.0f);
}

void MarioActor::fn_803C2520() {
    if (this->_B94 != 0) {
        this->_B94 -= 1;
        if (this->_B94 == 0) {
            this->mMario->stopAnimationUpper("ハンマー投げ回転中", (const char*)nullptr);
            this->mMario->stopAnimation("ハンマー投げ回転中", (const char*)nullptr);
        }
    }
}

void MarioActor::fn_803C2590() {
    u8 t;
    u8 timer = this->_944;
    if (timer != 0) {
        t = this->_945;
        this->_944 = timer - 1;
        this->_945 = t + 1;
        this->fn_803C2610();
        this->fn_803C26A0();
    }
    if (this->fn_803C2710()) {
        this->mMario->stopAnimation((const char*)nullptr, (const char*)nullptr);
    }
}

void MarioActor::fn_803C2610() {
    if ((_946 != 0 || ((this->mMario->mMovementStates.mHighWord >> (0x14)) & 1)) && !this->mMario->isSwimming() && this->_944 == 0 &&
        mActionTable1080->selectAction("スピン回復エフェクト") == 1) {
        this->playEffect("スピンリング");
    }
}

void MarioActor::fn_803C26A0() {
    if ((this->isInPunchTimerRange() || !this->mMario->isSwimming())) {
        u32 t = getMovementStates().mLowWord;
        if (((t >> 0x17) & 1) || ((t >> 5) & 1) || ((t >> 6) & 1)) {
            this->mMario->tryWallPunch();
        }
    }
}

bool MarioActor::fn_803C2710() {
    bool requested;
    bool playerGateClear;
    bool actorGateClear;
    bool releaseAnimation;
    releaseAnimation = false;
    actorGateClear = false;
    playerGateClear = false;
    requested = false;
    if (mMario->isAnimationRun("ハンマー投げリリース") &&
        ((mMario->mMovementStates.mLowWord >> 30) & 1U)) {
        releaseAnimation = true;
    }
    if (releaseAnimation && _38C == 0) {
        actorGateClear = true;
    }
    if (actorGateClear && mMario->_420 == 0) {
        playerGateClear = true;
    }
    if (playerGateClear && mMario->isStickOn()) {
        requested = true;
    }
    return requested;
}

bool MarioActor::doRush() {
    if (!_934) return false;
    tryCoinPull();
    tryRushInRush();
    if (!_934) return false;
    updateGravityVec(false, false);
    if (!checkClapCatchStart()) bodyClap();
    _7DC = 0;
    mMario->mPosHolder->fn_80405F90(mPosition);
    mMario->updateGroundInfo();
    mMario->updateCubeCode();
    if (!_934) return false;
    mMario->decDamageAfterTimer();
    fn_803C2900();
    fn_803C2AA0();
    mMarioAnim->setHoming();
    updateTornado();
    fn_803C2B50();
    updateSwingTimer();
    if (mHealth == 0 && isEnableNerveChange()) {
        setNerve(GET_NERVE(MarioActor, MarioActorNrvGameOver));
    }
    return true;
}

void MarioActor::fn_803C2900() {
    if (mActionTable1080->testEffect(MR::getSensorHost(_924)->mName, "水陸移動特殊処理")) {
        WaterInfo info;
        if (!MR::getWaterAreaObj(&info, this->mPosition)) {
            this->mMario->forceExitSwim();
        }
    } else if (!mActionTable1080->testEffect(MR::getSensorHost(_924)->mName, "水柱エフェクト")) {
        MarioSwim* sw = this->mMario->mSwim;
        s32 before = (sw)->mSwimState;
        sw->fn_803FD5E0(0);
        s32 after = (this->mMario->mSwim)->mSwimState;
        if (after != before) {
            MarioSwim* sw2 = this->mMario->mSwim;
            if ((u32)after <= 1 && (u32)(before - 2) <= 1) {
                this->playEffectRT("水面ジャンプ水柱", (sw2)->mSurfacePos, (sw2)->mSurfaceNorm);
                MarioSwim* sw3 = this->mMario->mSwim;
                this->emitEffectWaterColumn((sw3)->mSurfacePos, (sw3)->mSurfaceNorm);
            } else if ((u32)before <= 1 && (u32)(after - 2) <= 1) {
                TVec3f neg = -(this->mMario)->_328;
                this->playEffectRT("水面ジャンプ水柱", neg, (this->mMario->mSwim)->mSurfaceNorm);
                MarioSwim* sw3 = this->mMario->mSwim;
                this->emitEffectWaterColumn((sw3)->mSurfacePos, (sw3)->mSurfaceNorm);
            }
            if (after == 0) {
                MR::getGameSceneLayoutHolder()->changeLifeMeterModeGround();
                this->mMario->forceExitSwim();
                return;
            } else if (after == 2) {
                MR::getGameSceneLayoutHolder()->changeLifeMeterModeWater();
            }
        }
    }
}

void MarioActor::fn_803C2AA0() {
    if (this->mMario->isForceStopRush() && _934 != 0 && _924->receiveMessage(0x97, this->getSensor("Body"))) {
        RushEndInfo info(nullptr, 4, TVec3f(0.0f, 0.0f, 0.0f), false, 0);
        this->endRush(&info);
    }
}

void MarioActor::fn_803C2B50() {
    if (_934 != 0) {
        const char* effect = "ハチ回復";
        if (mActionTable1080->testEffect(MR::getSensorHost(_924)->mName, effect)) {
            u32 i = 0;
            do {
                this->mMario->incAirWalkTimer();
                i++;
            } while (i < 4);
        }
    }
}

void MarioActor::updateSwingTimer() {
    // NONMATCHING: GXColor conversion stack copy and pooled literal relocations.
    if (_934) {
        stopSpinTicoEffect(false);
        return;
    }
    if (_948 && --_948 == 6) {
        _94C = 5;
    }
    if (_94C && --_94C == 0) {
        mMario->startPadVib(0UL);
        mMario->playSound("スピン回復終了");
        Color8 color(80, 128, 200, 0);
        _1AA = 15;
        _1AC = 0.75f;
        mSwimFogColor.mGXColor = color.mGXColor;
        mSwimFogDisabled = true;
    }
    if (_946) {
        if (--_946 == 24) {
            stopEffectForce("スピンリング");
        }
        if (_946 == 13) {
            _948 = 19;
        }
        if (_946 == 0) {
            fn_803B98E0();
        }
    }
}

// NONMATCHING: redundant Mario receiver loads and pooled literal offsets.
void MarioActor::updateSwingAction() {
    shootFireBall();
    if (isJumping() && mPlayerMode != 6 && !mMario->isStatusActive(MarioStatus_Foo) && !fn_803B9900() &&
        (!mMario->isPlayerModeCloud() || MR::isSpinCloudBlockGroupEmpty())) {
        _946 = 0;
    }
#ifdef GALAXY_NATIVE
    petariCappySwingAction(this);
    // OdysseyMovement: the mod's Spin moves read the swing's rising edge directly,
    // buffered SpinBufferFrames frames, without Galaxy's spin cooldown (_946) and
    // swing lockouts (_F1C/_F1E). A press they use starts no Galaxy spin and no
    // cooldown; Galaxy's Star Spin otherwise keeps its own rules.
    if (petari_mod_enabled(2) && mPlayerMode == 0 && !mMario->isSwimming()) {
        namespace Live = PetariNative::Odyssey::Live;
        const bool grounded = !isJumping() && ((mMario->mMovementStates.mLowWord >> 30) & 1U);
        if (grounded && Live::roll.rolling) {
            // Rolling: a buffered press boosts once the 15-frame interval allows it.
            if (Live::spinBuffer > 0 && Live::roll.sinceBoost + 1 >= PetariNative::Odyssey::Const::SlopeRollingReStartInterval) {
                Live::rollBoost = true;
                Live::spinBuffer = 0;
            }
            return;
        }
        if (Live::spinBuffer > 0 && grounded && (((mMario->mMovementStates.mLowWord >> 21) & 1U) || mMario->checkSquat(false))) {
            Live::rollStart = true;
            Live::spinBuffer = 0;
            return;
        }
        if (isJumping() && (((mMario->mMovementStates.mLowWord >> 20) & 1U) || Live::noSpinAir)) {
            return;  // ground pound: the buffer is the dive's (Mario::procHipDrop); dive/long jump: no spin
        }
        if (Live::spinBuffer > 0 && isRequestRush()) {
            Live::spinBuffer = 0;  // Galaxy's own spin takes this press
        }
    }
#endif
    bool requestRush = isRequestRush();
    if (!requestRush) requestRush = false;
#ifdef GALAXY_NATIVE
    // Cappy's ground throw borrows the spin animation, which Galaxy reads as a
    // spin already running: it drops a Spin pressed meanwhile (and starts its
    // interval). The press is held until Cappy has left the hand, then starts
    // the game's own Spin below.
    switch (petariCappyGroundSpinOverThrow(this, requestRush)) {
    case PetariCappyGroundSpin::Hold:
        _1E1 = requestRush;
        return;
    case PetariCappyGroundSpin::Deliver:
        requestRush = true;
        break;
    case PetariCappyGroundSpin::None:
        break;
    }
#endif
    _1E1 = requestRush;
    if (_946) {
        if (requestRush) tryReleaseBombTeresa();
        return;
    }
    if (!requestRush) return;
#ifdef GALAXY_NATIVE
    if (petari_mod_enabled(2)) {
        static const bool trace = std::getenv("PETARI_ODYSSEY_TRACE") != nullptr;
        if (trace) {
            std::fprintf(stderr, "[odyssey] spin request: ground %d jumping %d squat %d/%d rolling %d cooldown %d mode %d\n",
                         static_cast< int >((mMario->mMovementStates.mLowWord >> 30) & 1U), static_cast< int >(isJumping()),
                         static_cast< int >((mMario->mMovementStates.mLowWord >> 21) & 1U), static_cast< int >(mMario->checkSquat(false)),
                         static_cast< int >(PetariNative::Odyssey::Live::roll.rolling), static_cast< int >(_946),
                         static_cast< int >(mPlayerMode));
        }
    }
#endif
    if (mMario->isPlayerModeYoshi()) {
        if (mMario->isStatusActive(0x27)) {
            static_cast<MarioYoshiBalloon*>(mMario->mYoshiBalloon)->fn_80414570();
            mMario->closeStatus(mMario->mYoshiBalloon);
        }
        if (YoshiParam::fn_8041BD70() && mYoshiHolder->mTongueAction->fn_8041F910()) {
            mMario->fn_803B0EF0();
            fn_803B98C0();
            _946 = 30;
            return;
        }
        if (!YoshiParam::fn_8041BD50()) return;
    }
#ifdef GALAXY_NATIVE
    if (std::getenv("PETARI_CAPPY_TRACE") != nullptr && isJumping()) {
        std::fprintf(stderr, "[cappy] air spin request action=%d spent=%d lock=%d\n", mMario->_430,
                     static_cast<int>((mMario->mMovementStates.mHighWord >> 20) & 1U), mMario->_428);
    }
#endif
    if (!fn_803C3260()) return;
    if (mMario->isPlayerModeYoshi() && mYoshiHolder->fn_8041BB30()) mMario->fn_803B14E0();
    if (mMario->isPlayerModeCloud() && !MR::isSpinCloudBlockGroupEmpty()) mMario->mMovementStates.mHighWord &= 0xFFEFFFFFU;
    if (mMario->isPlayerModeYoshi() && ((mMario->mMovementStates.mLowWord >> 21) & 1U)) mMario->mMovementStates.mLowWord &= 0xFFDFFFFFU;
    u8 action = mActionTable1080->selectAction("スピンアタック");
    switch (action) {
    case 1: {
        bool didSpinPunch = true;
        if (fn_803C33D0()) {
            bool tmp = false;
            if (_F0C) {
                tmp = true;
            }

            mMario->trySpinJump(tmp);
            setPunchHitTimer(0x19);
            tryReleaseBombTeresa();
        } else if (!((mMario->mMovementStates.mLowWord >> 16) & 1U) && (!mMario->isAnimationRun("地上ひねり")
#ifdef GALAXY_NATIVE
                   || petariCappyTakeGroundSpinDelivery()
#endif
                   ) && !mMario->isAnimationRun("雲スピン")) {
            const char* pLastAnimationName = mMarioAnim->getUpperAnimationName();
            if (mPlayerMode == 4) {
                if (!mMario->isAnimationRun("ハチスピン")) {
                    didSpinPunch = trySpinPunch();
                }
            } else {
                didSpinPunch = trySpinPunch();
            }

            _974 = 0;
            if (pLastAnimationName != mMarioAnim->getUpperAnimationName()) {
                mMario->playSound("パンチ風切り");
            }
        }

        if (mPlayerMode == 4) {
            if (isJumping()) {
                if (!mMario->isAnimationRun("ハチスピン空中")) {
                    mMario->playSound("スピンジャンプ");
                    if (mMario->isPlayerModeYoshi()) MR::startActionSound(mYoshiHolder->mYoshi, "HeadShake", -1, -1, -1);
                    else mMario->playSound("声スピン");
                }

                mMario->changeAnimation("ハチスピン空中");
            } else if (((getMario()->mMovementStates.mLowWord >> 21) & 1U) || mBeeWallWalk != 0) {
                mMario->changeAnimation("サマーソルト");
            } else {
                mMario->changeAnimation("ハチスピン");
            }
        }

        if (didSpinPunch) {
            _946 = mConst->getTable()->mSpinIntervalTime + 0x22;
        }

        break;
    }
    case 2:
        if (isEnableSpinPunch() && !mMario->isSwimming()) {
            updateFireSpin();
        }

        break;
    case 3:
        if (isEnableSpinPunch()) {
            doFreezeAttack();
            setPunchHitTimer(0x1e);
        }

        break;
    case 4: {
        if (mMario->_418 != 0) {
            break;
        }

        mMario->startTeresaDisappear();
        const MarioConstTable* pConstants = mConst->getTable();
        _946 = pConstants->mTeresaWallThroughTime + pConstants->mSpinIntervalTime;
        break;
    }
    case 5:
        if (!isEnableSpinPunch()) {
            break;
        }

        if (isJumping()) {
            mMario->changeAnimation("ハチスピン空中");
        } else {
            if (((getMario()->mMovementStates.mLowWord >> 21) & 1U) || mBeeWallWalk != 0) {
                mMario->changeAnimation("サマーソルト");  // Summersault
            } else {
                mMario->changeAnimation("ハチスピン");
            }
        }

        _946 = mConst->getTable()->mSpinIntervalTime + 0x22;
        break;
    }
}

void MarioActor::setPunchHitTimer(u8 punchHitTime) {
    if (_944 == 0) {
        _945 = 0;
        _974 = 0;
    }

    _944 = punchHitTime;
}

bool MarioActor::fn_803C3260() {
    if (this->mMario->isAnimationRun("壁はじき")) {
        return false;
    }
    if (this->isJumping() && this->mMario->_428 != 0) {
        return false;
    }
    if (mPlayerMode == 9) {
        return false;
    }
    if (this->mMario->getCurrentStatus() == 1) {
        return false;
    }
    if (this->mMario->isSwimming()) {
        return false;
    }
    if (this->mMario->isStatusActive(0x18)) {
        return false;
    }
    if (this->mMario->isStatusActive(0x13)) {
        return false;
    }
    if (static_cast<s32>(_468) != 0) {
        return false;
    }
    if (this->mMario->isStatusActive(2)) {
        return false;
    }
    if (this->_3C0 != 0) {
        return false;
    }
    if (this->_EA4 != 0) {
        return false;
    }
    Mario* m = this->mMario;
    if (((m->mDrawStates.mWord >> (0x13)) & 1)) {
        return false;
    }
    return m->mCometNoSpinArea == 0;
}

bool MarioActor::fn_803C33D0() {
    bool r = false;
    bool a = false;
    bool b = false;
    if (!((this->mMario->mMovementStates.mLowWord >> (0x10)) & 1) && this->isJumping()) {
        b = true;
    }
    if (b) {
        bool c = true;
        if (this->mMario->isDamaging()) {
            bool d = false;
            if (this->mMario->isPlayerModeCloud() && MR::isSpinCloudBlockGroupEmpty() == 0) {
                d = true;
            }
            if (!d) {
                c = false;
            }
        }
        if (c) {
            a = true;
        }
    }
    if (a && !((this->mMario->mMovementStates.mHighWord >> (0x14)) & 1)) {
        r = true;
    }
    return r;
}

bool MarioActor::checkClapCatchStart() {
    if (_6D0) {
        _6D0 = false;
        tryCoinPullInRush();
        return true;
    } else {
        return false;
    }
}

void MarioActor::fn_803C34F0() {
    if (this->_92C != 0) {
        this->_92C -= 1;
        if (this->_92C == 0) {
            this->decreaseLifeByOne();
            this->fn_803D9740();
        }
    }
    if (this->mHealth <= 1) {
        if (!MR::isDemoActive() && !MR::isPowerStarGetDemoActive() && !MR::isGalaxyDarkCometAppearInCurrentStage() && MR::isPermitSE() &&
            this->isEnableNerveChange()) {
            this->mMario->mStateFlags._2 = true;
        }
    }
    this->mMario->fn_8038C540();
}

bool MarioActor::doPressing() {
    if (!_390) {
        return false;
    }

    switch (mCounter6C8) {
    case 0:
        if (!mMario->checkVerticalPress(false)) {
            if (--_390 == 0x1e && !mHealth && isEnableNerveChange()) {
                setNerve(GET_NERVE(MarioActor, MarioActorNrvGameOver));
            }

            if (!_390) {
                mMario->changeAnimation("つぶれ解除");
                _F44 = true;
            }
        }

        break;
    case 2:
    case 3:
        if (--_390 == 0x1e && !mHealth && isEnableNerveChange()) {
            setNerve(GET_NERVE(MarioActor, MarioActorNrvGameOver));
        }

        if (!_390) {
            mMario->changeAnimation("つぶれ解除");
            _F44 = true;
        }

        break;
    case 4:
        if (_390 > 2) {
            _390--;
        }

        mMario->mMovementStates.mLowWord &= 0xFF7FFFFFU;
        mMario->mMovementStates.mLowWord &= 0xFFFFFFBFU;
        mMario->mMovementStates.mLowWord &= 0xFFFFFFDFU;
        break;
    }

    if (static_cast<s32>(mCounter6C8) != 1) {
        careMovingObject();
    }

    mMarioAnim->setSpeed(0.0f);
    mMario->_350.zero();
    mMario->_35C.zero();
    resetSensorCount();
    return true;
}

void MarioActor::careMovingObject() {
    if (!_1E0) {
        fn_803C38D0();
        mMario->mPosHolder->fn_80405F90(mPosition);
        mMario->mVelocity.zero();
        mMario->powerAreaMove();
        mMario->powerRailMove();
        mPosition = mMario->mPosHolder->mPos;
        mMario->checkGround();
        mMario->updateFloorCode();
    }
}

// NONMATCHING: equivalent entry guard, local stack slots, and redundant Mario receiver load.
bool MarioActor::doStun() {
    if (_38C == 0) return false;
    mMario->mDrawStates.mWord |= 0x00000020U;
    resetSensorCount();
    fn_803C38D0();

    if (!--_38C) {
        f32 stickY, stickX;
        getStickValue(&stickX, &stickY);
        bool noStick = false;
        if (MR::isNearZero(stickX) && MR::isNearZero(stickY)) noStick = true;
        if (noStick && !((mMario->mMovementStates.mLowWord >> 21) & 1U)) {
            mMario->stopWalk();
        }
    }

    mMario->inputStick();
    return true;
}

void MarioActor::fn_803C38D0() {
    if (this->fn_803C3930()) {
        this->mMario->mPosHolder->fn_80405F90(this->mPosition);
        this->mMario->checkEnforceMove();
        this->mPosition = this->mMario->mPosHolder->mPos;
    }
}

bool MarioActor::fn_803C3930() const {
    MtxPtr prev;
    Mario* m = this->mMario;
    bool r = false;
    if (((m->mMovementStates.mLowWord >> (0x1E)) & 1)) {
        prev = m->_45C->getPrevBaseMtx()->mMtx;
        if (!MR::isSameMtx((this->mMario)->_45C->getBaseMtx()->mMtx, prev)) {
            r = true;
        }
    }
    return r;
}

// NONMATCHING: native-safe matrix column access and register scheduling.
void MarioActor::scaleMtx(MtxPtr pRawMtx) {
    TPos3f* matrix = reinterpret_cast<TPos3f*>(pRawMtx);
    TVec3f i, j, k;
    f32 scalar = 0.35f * (1.0f - _3B0) + 1.0f;
    matrix->getXDir(i);
    matrix->getYDir(j);
    matrix->getZDir(k);
    f32 elementX = MR::vecKillElement(i, mPressNormal6D8, &i);
    f32 elementY = MR::vecKillElement(j, mPressNormal6D8, &j);
    f32 elementZ = MR::vecKillElement(k, mPressNormal6D8, &k);
    elementZ *= _3B0;
    elementX *= _3B0;
    elementY *= _3B0;
    i.scale(scalar);
    j.scale(scalar);
    k.scale(scalar);
    i += mPressNormal6D8 * elementX;
    j += mPressNormal6D8 * elementY;
    k += mPressNormal6D8 * elementZ;
    matrix->setXDir(i);
    matrix->setYDir(j);
    matrix->setZDir(k);
}

void MarioActor::calcAnimInMovement() {
    if (mHealth != 0 && (mMario->isDamaging() || mMario->getDamageAfterTimer() || mBlinkRequested1067)) {
        _1AA = 0;
        ++_1A8;
    } else if (_1AA != 0) {
        --_1AA;
    }
    _1C0 = true;
    updateBlink();
    calcSpinEffect();
    if (mAllModel->mCurrentModel == 4) {
        _9C0->movement();
        mHandModelL->movement();
        mHandModelR->movement();
        MR::setBrkFrame(mHandModelL, MR::getBrkFrame(_9C0));
        MR::setBrkFrame(mHandModelR, MR::getBrkFrame(_9C0));
    }
    fn_803C3C90();
    fn_803C3D10();
    fn_803C4100();
}

void MarioActor::fn_803C3C90() {
    if (this->_3DA != 0) {
        this->_3DA -= 1;
        if (this->_3DA == 0) {
            MR::endDemo(this, "マリオスーパー化");
            MR::releaseAnimFrame(this);
            if (this->mMario->isPlayerModeYoshi()) {
                MR::releaseAnimFrame(mYoshiHolder->mYoshi);
            }
            MR::explainLifeUpIfAtFirst();
        }
    }
}

void MarioActor::fn_803C3D10() {
    if (this->_3D8 != 0) {
        if (mPlayerMode == 6) {
            if (mMario->mVerticalSpeed < mConst->getTable()->mTeresaWaitHeight) {
                this->mPosition -= *this->mMario->getGravityVec();
            }
        }
        if (!this->fn_803C3DF0() && !this->mMario->fn_803B26C0()) {
            this->fn_803C3E40();
        }
        this->_3D8 -= 1;
        if (this->_3D8 == 0 && !this->mMario->fn_803B26C0()) {
            this->fn_803C4020();
        }
        this->_3DF = 1;
        return;
    }
    this->mMario->updateMorphResetTimer();
}

bool MarioActor::fn_803C3DF0() {
    switch (mPlayerMode) {
    case 4:
        return MR::isOnGameEventFlagBeeMarioAtFirst();
    case 6:
        return MR::isOnGameEventFlagTeresaMarioAtFirst();
    case 5:
        return MR::isOnGameEventFlagHopperMarioAtFirst();
    case 2:
        return MR::isOnGameEventFlagFireMarioAtFirst();
    case 3:
        return MR::isOnGameEventFlagIceMarioAtFirst();
    case 7:
        return MR::isOnGameEventFlagFlyingMarioAtFirst();
    case 12:
        return MR::isOnGameEventFlagCloudMarioAtFirst();
    case 13:
        return MR::isOnGameEventFlagRockMarioAtFirst();
    }
    return true;
}

void MarioActor::fn_803C3E40() {
    TVec3f front;
    if (_4A4 != nullptr) {
        MR::calcFrontVec(&front, MR::getSensorHost(_4A4));
    } else if (mSensor794 != nullptr) {
        MR::calcFrontVec(&front, MR::getSensorHost(mSensor794));
    } else {
        this->getFrontVec(&front);
    }
    this->mMario->setFrontVecKeepUp(front);
    if (this->mFirstTransformCameraLatch666 == 0) {
        MR::zoomInTargetGameCamera();
        CameraTargetArg arg(nullptr, nullptr, nullptr, this);
        MR::startGlobalEventCamera("変身初出カメラ", arg, 60);
        TVec3f up = -*this->mMario->getGravityVec();
        TVec3f base(this->_240);
        f32 dist = 500.0f;
        f32 height = 50.0f;
        if (mPlayerMode == 6) {
            dist = 700.0f;
            height = 150.0f;
        }
        Mario* m = this->mMario;
        u32 st = (m)->mMovementStates.mLowWord;
        if (((st >> 0x14) & 1) && !((st >> 0x1E) & 1)) {
            base += (m)->mJumpVec;
        }
        up.scale(height);
        TVec3f eye(base);
        eye += up;
        MR::setProgrammableCameraParam("変身初出カメラ", eye, base + front * dist + up, -mAppliedGravityVec, true);
    }
    this->mFirstTransformCameraLatch666 = 1;
}

void MarioActor::fn_803C4020() {
    if (this->mTransforming != 0) {
        MR::endDemo(this, "マリオ変身");
        this->mTransforming = 0;
    }
    MR::releaseAnimFrame(this);
    this->_3DE = 1;
    switch (mPlayerMode) {
    case 4:
        MR::explainBeeMarioIfAtFirst();
        break;
    case 6:
        MR::explainTeresaMarioIfAtFirst();
        break;
    case 5:
        MR::explainHopperMarioIfAtFirst();
        break;
    case 2:
        MR::explainFireMarioIfAtFirst();
        break;
    case 3:
        MR::explainIceMarioIfAtFirst();
        break;
    case 7:
        MR::explainFlyingMarioIfAtFirst();
        break;
    case 1:
        MR::explainInvincibleMarioIfAtFirst();
        break;
    case 12:
        MR::explainCloudMarioIfAtFirst();
        break;
    case 13:
        MR::explainRockMarioIfAtFirst();
        break;
    }
    if (this->mFirstTransformCameraLatch666 != 0) {
        this->_338 = 10;
        this->mFirstTransformCameraLatch666 = 0;
    }
}

void MarioActor::fn_803C4100() {
    if (this->_338 != 0) {
        this->_338 -= 1;
        if (this->_338 == 0) {
            MR::zoomOutTargetGameCamera();
            MR::endGlobalEventCamera("変身初出カメラ", 60, true);
        }
    }
}

// NONMATCHING: equivalent visibility guards, register allocation, and pooled literal offsets.
void MarioActor::calcAnim() {
    CollisionShadow* pShadow;
    if (mMario->isPlayerModeYoshi()) {
        bool hideYoshi = true;
        if (!_482 && !_481) hideYoshi = false;
        if (hideYoshi == false) hideYoshi = !_482 && _483;
        if (hideYoshi) mYoshiHolder->mYoshi->fn_8040CBF0();
        else mYoshiHolder->mYoshi->fn_8040CBD0();
    }
    bool canUpdateAnim = !_482 || _EA5;
    if (canUpdateAnim == false) {
        mBlendMtxTimer = 0;
        return;
    }
    calcAnimPlayerMode();
    calcAnimHorrorBone();
    calcAndSetBaseMtx();
    mMarioAnim->switchMirrorMode();
    if (mMario->isPlayerModeYoshi()) {
        MR::setPosition(mYoshiHolder->mYoshi, mPosition);
        mYoshiHolder->mYoshi->fn_8040C960();
    }
    MR::getBvaCtrl(this);
    mModelManager->mBvaPlayer->calc();
    mMarioAnim->resetAnimationUpdateFlags();
    mModelManager->mBvaPlayer->clearUpdateSkipFlag();
    mModelManager->mBtpPlayer->clearUpdateSkipFlag();
    mMarioAnim->calc();
    if (_3D8 == 0) {
        switch (mPlayerMode) {
        case 4:
            MR::setPosition(_9E4, mPosition);
            MR::updateModelDiffDL(_9E4);
            if (MR::isDead(_9E8)) { _9EC->onDraw(-1); _9E8->appear(); }
            break;
        case 3:
            MR::setPosition(mModelE9C, mPosition);
            MR::updateModelDiffDL(mModelE9C);
            MR::updateModelDiffDL(mModelF1C);
            MR::updateModelDiffDL(_A10);
            break;
        case 1:
            MR::setPosition(_9C0, mPosition);
            MR::updateModelDiffDL(_9C0);
            MR::updateModelDiffDL(mHandModelL);
            MR::updateModelDiffDL(mHandModelR);
            break;
        case 5: MR::setPosition(mModelEDC, mPosition); MR::updateModelDiffDL(mModelEDC); break;
        case 12: MR::setPosition(mModelEEC, mPosition); MR::updateModelDiffDL(mModelEEC); break;
        case 13: MR::setPosition(mModelEF4, mPosition); MR::updateModelDiffDL(mModelEF4); break;
        default:
            if (mAllModel->mCurrentModel != 1 && mAllModel->mCurrentModel != 6) MR::updateModelDiffDL(this);
            break;
        }
    } else {
        if (mAllModel->mCurrentModel == 0) MR::updateModelDiffDL(this);
        if (mPlayerMode == 4) {
            if (mAllModel->mCurrentModel == 2) {
                if (MR::isDead(_9E8)) { _9E8->appear(); _9EC->onDraw(-1); }
            } else if (!MR::isDead(_9E8)) { _9E8->kill(); _9EC->offDraw(-1); }
        } else if (mAllModel->mCurrentModel == 2) {
            if (MR::isDead(_9E8)) { _9E8->appear(); _9EC->onDraw(-1); }
        } else if (_3D6 == 4) {
            if (!MR::isDead(_9E8)) { _9E8->kill(); _9EC->offDraw(-1); }
        }
        if (mPlayerMode == 12) {
            if (mAllModel->mCurrentModel == 7) {
                if (MR::isDead(mCloudMarioHatEF0)) mCloudMarioHatEF0->appear();
            } else if (!MR::isDead(mCloudMarioHatEF0)) mCloudMarioHatEF0->kill();
        } else if (mAllModel->mCurrentModel == 7) {
            if (MR::isDead(mCloudMarioHatEF0)) mCloudMarioHatEF0->appear();
        } else if (_3D6 == 12) {
            if (!MR::isDead(mCloudMarioHatEF0)) mCloudMarioHatEF0->kill();
        }
    }
    if (mAllModel->mCurrentModel == 7) {
        if (mMario->isStatusActive(MarioStatus_FpView)) {
            MR::showJoint(mAllModel->getJ3DModel(), "Cap0");
            MR::hideModel(mCloudMarioHatEF0);
        } else {
            MR::hideJoint(mAllModel->getJ3DModel(), "Cap0");
            MR::showModel(mCloudMarioHatEF0);
        }
    }
    if (mAllModel->mCurrentModel == 7 && (mActorEffectState->mActive || _390)) {
        MR::showJoint(mAllModel->getJ3DModel(), "Cap0");
        MR::hideModel(mCloudMarioHatEF0);
    }
    if (MR::isHiddenModel(this) && !_482) MR::showModel(this);
    updateRealMtx();
    if (!((mMario->mMovementStates.mLowWord >> 16) & 1U) || mMario->_544 <= 1) {
        mCollisionShadow580->setMode(2);

        mCollisionShadow580->_20 = 40.0f + (mMario->mShadowPos - mPosition).dot(mAppliedGravityVec);
        mCollisionShadow580->_48 = mMario->mShadowPos;

        TVec3f leftVec;
        getRealPos("HandL", &leftVec);

        TVec3f rightVec;
        getRealPos("HandR", &rightVec);

        TVec3f stack114(rightVec - leftVec);

        f32 length = stack114.length();
        MR::normalizeOrZero(&stack114);

        TVec3f cross;
        cross.cross(stack114, mAppliedGravityVec);
        MR::normalizeOrZero(&cross);

        stack114.cross(mAppliedGravityVec, cross);

        if (MR::normalizeOrZero(&stack114)) {
            stack114 = mMario->mSideVec;
        }

        if (mMario->isSwimming()) {
            pShadow = mCollisionShadow580;
            pShadow->_3C = stack114;
            pShadow->_18 = 10.0f + length;
            pShadow->_1C = 140.0f;
        } else {
            pShadow = mCollisionShadow580;
            pShadow->_3C = stack114;
            pShadow->_18 = 5.0f + length;
            pShadow->_1C = 70.0f;
        }

        TVec3f vec;
        if (mMario->isStatusActive(MarioStatus_Hang)) {
            getRealPos("Center", &vec);
        } else {
            getRealPos("All_Root", &vec);
        }

        mCollisionShadow580->create(vec, mAppliedGravityVec, mMario->mFrontVec);
    }

    if (_A0C == 0) {
        mMario->fn_8038B9F0();
    }

    if (!mMario->isPlayerModeYoshi()) {
        updateHand();
        updateFace();
    }
    calcScreenBoxRange();

    if (_946 || mFlag100E || mFlag100F || ((mMario->mMovementStates.mHighWord >> 20) & 1U)) {
        _94E = 0;
    } else {
        _94E++;
    }

    mAllModel->fn_803E78F0();
    mScreenStrip1084->fn_803DBA40();
}

void MarioActor::calcAnimPlayerMode() {
    if (this->_3DF == 0) {
        this->_3DE = 0;
        return;
    }
    u16 mode;
    u16 t = this->_3D8;
    if ((s32)(t & 7) < ((s32)t >> 3)) {
        mode = this->_3D6;
    } else {
        mode = mPlayerMode;
    }
    this->fn_803D8A70(mode);
    switch (mode) {
    case 1:
        mAllModel->fn_803E7880(4);
        _9C0->calcAnim();
        if (this->_3DE != 0) {
            this->_A6E = 2;
        }
        break;
    case 4:
        mAllModel->fn_803E7880(2);
        break;
    case 5:
        this->_3DF = 0;
        mAllModel->fn_803E7880(5);
        break;
    case 3:
        mAllModel->fn_803E7880(3);
        break;
    case 12:
        mAllModel->fn_803E7880(7);
        break;
    case 13:
        mAllModel->fn_803E7880(8);
        break;
    default: {
        mAllModel->fn_803E7880(0);
        u32 frame = 0;
        switch (mode) {
        case 7:
            frame = 1;
            break;
        case 2:
            frame = 2;
            break;
        }
        MR::startBtp(this, "ColorChange");
        MR::setBtpFrameAndStop(this, (f32)frame);
        break;
    }
    }
    this->_3DF = 0;
    this->_3DE = 0;
}

void MarioActor::calcAnimHorrorBone() {
    if (MR::isGalaxyHorrorCometAppearInCurrentStage()) {
        Mario* m = this->mMario;
        if (m->mCometRestrictions != 0) {
            if ((mAllModel)->mCurrentModel == 6) {
                mAllModel->fn_803E7B80();
                this->playEffect("ホラーマリオ解除");
            }
        } else if (MR::isPowerStarGetDemoActive()) {
            if ((mAllModel)->mCurrentModel == 6) {
                mAllModel->fn_803E7B80();
            }
        } else {
            MarioAllModel* p = mAllModel;
            if (p->getCurrentModel() != 6) {
                p->mPreviousModel = p->mCurrentModel;
                this->playEffect("ホラーマリオ変身");
            }
            mAllModel->fn_803E7880(6);
        }
    } else {
        if (this->mMario->isStatusActive(0xB)) {
            MarioAllModel* p = mAllModel;
            if (p->getCurrentModel() != 1) {
                p->mPreviousModel = p->mCurrentModel;
            }
            if ((this->_37C & 3) && (this->isNerve(&NrvMarioActor::MarioActorNrvWait::sInstance) || this->isNerve(&NrvMarioActor::MarioActorNrvNoRush::sInstance))) {
                mAllModel->fn_803E7880(1);
                return;
            }
            mAllModel->fn_803E7B80();
            return;
        }
        if ((mAllModel)->mCurrentModel == 1 && this->mHealth != 0) {
            mAllModel->fn_803E7B80();
        }
    }
}

void MarioActor::calcAndSetBaseMtx() {
    if (!fn_803C5990()) return;
    if (fn_803C4F70()) return;
    TPos3f previous;
    if (mMario->isPlayerModeYoshi()) {
        J3DModel* previousModel = MR::getJ3DModel(mYoshiHolder->mYoshi);
        previous.set(previousModel->getBaseTRMtx());
    } else {
        J3DModel* previousModel = mAllModel->getJ3DModel();
        previous.set(previousModel->getBaseTRMtx());
    }
    bool received = fn_803C5750();
    TPos3f matrix;
    Mtx inverse, base, joint;
    TVec3f offset;
    PSMTXIdentity(matrix.toMtxPtr());
    mMario->fn_8038A560(matrix.toMtxPtr(), received);
    if (_EA6 == 1) {
        Mtx cap;
        const char* capJoint = "CapPosition";
        getRealMtx(cap, capJoint);
        MR::extractMtxTrans(cap, &mCapPosition);
        TVec3f delta(mCapPosition);
        delta -= mPosition;
        if (PSVECMag(&delta) > 500.0f) {
            PSMTXCopy(getBaseMtx(), matrix.toMtxPtr());
            MR::setMtxTrans(matrix.toMtxPtr(), 0.0f, 0.0f, 0.0f);
        }
    }
    PSMTXConcat(matrix.toMtxPtr(), MR::tmpMtxRotYRad(mMario->mYAngleOffset), matrix.toMtxPtr());
    if (mPlayerMode == 6) updateBaseMtxTeresa(matrix.toMtxPtr());
    if (!received && fn_803C4FB0()) fn_803C5090(matrix.toMtxPtr());
    else fn_803C56F0();
    MR::addTransMtx(matrix.toMtxPtr(), mPosition);
    if (received) {
        PSMTXInverse(matrix.toMtxPtr(), inverse);
        PSMTXCopy(getBaseMtx(), base);
        MtxPtr inverseMtx = inverse;
        MtxPtr baseMtx = base;
        PSMTXConcat(inverseMtx, baseMtx, mMainJointMatrix4A8.toMtxPtr());
    } else {
        mMario->fn_8038A8A0(joint, &offset);
        mMarioAnim->setMainJointOffset(1, offset);
        mMarioAnim->setYoshiJointOffset("Center", offset);
        MtxPtr jointMtx = joint;
        PSMTXCopy(jointMtx, mMainJointMatrix4A8.toMtxPtr());
    }
    mMarioAnim->setMainJointMatrix(0, mMainJointMatrix4A8.toMtxPtr());
    const char* rootJoint = "AllRoot";
    mMarioAnim->setYoshiJointMatrix(rootJoint, mMainJointMatrix4A8.toMtxPtr());
    fn_803C5AB0(matrix.toMtxPtr());
    fn_803C5ED0(matrix.toMtxPtr(), previous.toMtxPtr());
    if (_3B0 != 1.0f) scaleMtx(matrix.toMtxPtr());
    if (mMario->isPlayerModeYoshi()) mYoshiHolder->mYoshi->setTransformFromMtx(matrix.toMtxPtr());
    else {
        J3DModel* destinationModel = mAllModel->getJ3DModel();
        PSMTXCopy(matrix.toMtxPtr(), destinationModel->getBaseTRMtx());
        J3DModel* model = mAllModel->getJ3DModel();
        model->mBaseScale.x = mScale.x;
        model->mBaseScale.y = mScale.y;
        model->mBaseScale.z = mScale.z;
    }
    _EA5 = false;
}


bool MarioActor::fn_803C4F70() {
    bool r = false;
    if (this->_EA5 == 0 && this->_EA4 != 0 && this->_EA6 != 0) {
        r = true;
    }
    return r;
}

bool MarioActor::fn_803C4FB0() {
    bool a;
    bool r = false;
    a = false;
    if (mPlayerMode == 4 && !this->mMario->isStatusActive(0x16) && !this->mMario->isStatusActive(0x15) && !this->mMario->isStatusActive(0x1B)) {
        Mario* m = this->mMario;
        if (!((m->mMovementStates.mHighWord >> (0x1C)) & 1) && !((m->mMovementStates.mLowWord >> (0x15)) & 1)) {
            a = true;
        }
    }
    if (a) {
        bool b = false;
        u32 st = (this->mMario)->mMovementStates.mLowWord;
        if (((st >> 0x14) & 1) && (st >> 0x1F) != 0) {
            b = true;
        }
        if (!b) {
            r = true;
        }
    }
    return r;
}

// NONMATCHING: saved-register allocation, stack slots and equivalent scheduling.
void MarioActor::fn_803C5090(MtxPtr mtx) {
    mGravityReferenceED0 = (this->mMario)->mAirGravityVec;
    f32 k = mConst->getTable()->mBeePoseHeadToFootLength;
    TVec3f t1 = -*this->mMario->getGravityVec() * k;
    MtxPtr m1 = MR::tmpMtxTrans(-t1);
    f32 k2 = mConst->getTable()->mBeePoseHeadToFootLength;
    TVec3f d = this->_33C - (this->mPosition - mGravityReferenceED0 * k2);
    f32 angle = 0.0f;
    TVec3f axis;
    if (!MR::normalizeOrZero(&d)) {
        PSVECCrossProduct((Vec*)&(this->mMario)->mFrontVec, (Vec*)&d, (Vec*)&axis);
        if (!MR::normalizeOrZero(&axis)) {
            f32 ang = MR::acosEx(mGravityReferenceED0.dot(d));
            f32 lim = mConst->getTable()->mBeePoseDelayAngleAir;
            if (((this->mMario->mMovementStates.mLowWord >> (0x1E)) & 1)) {
                lim = mConst->getTable()->mBeePoseDelayAngleGround;
            }
            angle = MR::clamp(ang, 0.0f, lim);
        }
    }
    Mtx rot;
    PSMTXRotAxisRad(rot, (Vec*)&axis, angle);
    PSMTXConcat(m1, mtx, mtx);
    PSMTXConcat(rot, mtx, mtx);
    PSMTXConcat(MR::tmpMtxTrans(t1), mtx, mtx);
    f32 k3 = mConst->getTable()->mBeePoseDelayAccel;
    TVec3f s110(mGravityReferenceED0);
    s110.scale(k3);
    MR::vecKillElement(s110, this->_360, &s110);
    this->_354 = this->_354 + s110;
    this->_33C = this->_33C + this->_354;
    f32 k4 = mConst->getTable()->mBeePoseHeadToFootLength;
    this->_360 = this->_33C - (this->mPosition - mGravityReferenceED0 * k4);
    TVec3f s104(this->_360);
    TVec3f sF8(this->_360);
    MR::normalizeOrZero(&sF8);
    f32 ang2 = MR::acosEx(mGravityReferenceED0.dot(sF8));
    f32 lim2 = mConst->getTable()->mBeePoseLimitAngleAir;
    if (((this->mMario->mMovementStates.mLowWord >> (0x1E)) & 1)) {
        lim2 = mConst->getTable()->mBeePoseLimitAngleGround;
    }
    if (ang2 > lim2) {
        TVec3f sEC;
        PSVECCrossProduct((Vec*)&mGravityReferenceED0, (Vec*)&sF8, (Vec*)&sEC);
        if (MR::normalizeOrZero(&sEC)) {
            this->_360 = mGravityReferenceED0;
        } else {
            Mtx rot2;
            PSMTXRotAxisRad(rot2, (Vec*)&sEC, lim2);
            PSMTXMultVec(rot2, (Vec*)&mGravityReferenceED0, (Vec*)&this->_360);
        }
    }
    this->_360.setLength(mConst->getTable()->mBeePoseHeadToFootLength);
    if (PSVECMag((Vec*)&s104) > mConst->getTable()->mBeePoseHeadToFootLength) {
        this->_354 += -(s104 - this->_360);
    }
    f32 k5 = mConst->getTable()->mBeePoseHeadToFootLength;
    this->_33C = (this->mPosition - mGravityReferenceED0 * k5) + this->_360;
    MR::normalizeOrZero(&this->_360);
    if (PSVECMag((Vec*)&this->_354) > mConst->getTable()->mBeePoseDelaySpeedLimit) {
        this->_354.setLength(mConst->getTable()->mBeePoseDelaySpeedLimit);
    }
    f32 sc = mConst->getTable()->mBeePoseFrictionStop;
    if (mMario->mTargetWalkSpeedIndex == 0 && !this->isJumping()) {
        sc = mConst->getTable()->mBeePoseFrictionMove;
    }
    this->_354.scale(sc);
    if (!MR::isSameDirection(this->_360, (this->mMario)->mFrontVec, 0.01f)) {
        MR::makeMtxUpFront((TPos3f*)mtx, -this->_360, (this->mMario)->mFrontVec);
    } else if (!MR::isSameDirection((this->mMario)->_1FC, (this->mMario)->mFrontVec, 0.01f)) {
        MR::makeMtxUpFront((TPos3f*)mtx, (this->mMario)->_1FC, (this->mMario)->mFrontVec);
    } else {
        MR::makeMtxUpFront((TPos3f*)mtx, (this->mMario)->mHeadVec, (this->mMario)->mFrontVec);
    }
    if (mMario->mTargetWalkSpeedIndex > 2 || this->isJumping()) {
        f32 inv = 1.0f - mConst->getTable()->mBeePoseTransBlendingRatioMove;
        this->_348 = this->_348 * mConst->getTable()->mBeePoseTransBlendingRatioMove +
                     (this->_33C - this->mPosition) * inv;
        MR::setMtxTrans(mtx, this->_348.x, this->_348.y, this->_348.z);
        return;
    }
    this->_348.scale(mConst->getTable()->mBeePoseTransBlendingRatioStop);
    MR::setMtxTrans(mtx, this->_348.x, this->_348.y, this->_348.z);
}

void MarioActor::fn_803C56F0() {
    this->_354.zero();
    this->_348.zero();
    this->_33C = this->mPosition;
    mGravityReferenceED0 = *this->mMario->getGravityVec();
}

bool MarioActor::fn_803C5750() {
    bool r = false;
    if (_934 != 0) {
        r = _924->receiveMessage(0xA1, this->getSensor("Body"));
    }
    if (this->_EA5 != 0) {
        PSMTXCopy(mRemoteDemoMtx4E4.mMtx, this->getBaseMtx());
    }
    if (r || this->_EA5 != 0) {
        this->fn_803C58D0();
        return true;
    }
    if (_934 != 0) {
        this->fn_803C5920();
    }
    return false;
}

void MarioActor::fn_803C5820() {
    TVec3f old(this->mPosition);
    MR::extractMtxTrans(this->getBaseMtx(), &this->mPosition);
    this->mMario->invalidateRelativePosition();
    TVec3f diff(this->mPosition);
    diff -= old;
    this->_938 = diff;
    if (((this->mMario->mMovementStates.mHighWord >> (8)) & 1)) {
        (this->mMario)->_688 = this->mPosition;
    }
    this->updateGravityVec(true, true);
    this->mMario->updateGroundInfo();
}

void MarioActor::fn_803C58D0() {
    this->fn_803C59C0();
    this->fn_803C5820();
    if (this->_EA4 != 0) {
        this->_EA6 = 1;
    }
}

void MarioActor::fn_803C5920() {
    TVec3f cur;
    MR::extractMtxTrans(this->getBaseMtx(), &cur);
    TVec3f diff(this->mPosition);
    diff -= cur;
    this->_938 = diff;
}

bool MarioActor::fn_803C5990() {
    if (this->_1C0 == 0) {
        this->_A0C = 1;
        return false;
    }
    this->_A0C = 0;
    this->_1C0 = 0;
    return true;
}

void MarioActor::fn_803C59C0() {
    TVec3f front, head, side;
    TPos3f* bm = (TPos3f*)this->getBaseMtx();
    bm->getZDir(front);
    this->mMario->setFrontVec(front);
    ((TPos3f*)this->getBaseMtx())->getXDir(side);
    this->mMario->setSideVec(side);
    ((TPos3f*)this->getBaseMtx())->getYDir(head);
    this->mMario->forceSetHeadVecKeepSide(head);
    (this->mMario)->mWalkSpeed = 0.0f;
    (this->mMario)->mTargetWalkSpeedIndex = 0;
    this->mMario->stopJump();
    (this->mMario)->_334 = (this->mMario)->mFrontVec;
    if (this->mMario->isSwimming()) {
        this->mMario->mSwim->resetAndFixPose();
    }
}

void MarioActor::fn_803C5AB0(MtxPtr mtx) {
    if (this->_390 != 0) {
        this->fn_803C5B30(mtx);
    } else if (this->_394 != 0) {
        this->fn_803C5E20(mtx);
    } else if (!((this->mMario->mMovementStates.mHighWord >> (3)) & 1)) {
        this->_3B0 = 1.0f;
    }
    u32 t = this->_398;
    if (t != 0) {
        this->_398 = t - 1;
    }
}

// NONMATCHING: Typed scalar members use direct offsets instead of a saved field address.
void MarioActor::fn_803C5B30(MtxPtr mtx) {
    s32 mode = static_cast<s32>(mCounter6C8);
    reinterpret_cast<s32&>(this->_394) = 0x1E;
    switch (mode) {
    case 0:
    case 2: {
        u32 cnt = this->_390;
        if (cnt > 0xF) {
            f32 a = this->mMario->calcDistToCeil(false);
            f32 b = this->mMario->calcDistToCeilOnPress();
            f32 v1 = MR::clamp(a / 150.0f, 0.2f, 1.0f);
            if (this->_3B0 > v1) {
                this->_3B0 = v1;
            } else {
                f32 v2 = MR::clamp(b / 150.0f, 0.2f, 1.0f);
                if (this->_3B0 > v2) {
                    this->_3B0 = v2;
                }
                MR::setMtxTrans(mtx, this->mPosition.x, this->mPosition.y, this->mPosition.z);
                if ((this->mMario)->_960 != 0x1B) {
                    this->_1E0 = 1;
                }
            }
        } else {
            this->_3B0 = 0.2f + ((0.8f * (f32)(0xF - cnt)) / 15.0f);
        }
        u32 t = this->_398;
        if (t != 0) {
            this->_3B0 = this->_3B0 + 0.02f * (f32)t;
            return;
        }
        break;
    }
    case 1:
    case 3: {
        u32 cnt = this->_390;
        if (cnt > 0xF) {
            f32 w = this->mMario->calcDistWidth();
            this->mPosition = this->mMario->mPosHolder->mPos;
            f32 v = MR::clamp(w / 80.0f, 0.2f, 1.0f);
            if (this->_3B0 > v) {
                this->_3B0 = v;
            } else {
                f32 t2 = this->_3B0 - 0.01f;
                this->_3B0 = t2;
                this->_3B0 = MR::clamp(t2, 0.2f, 1.0f);
            }
        } else {
            this->_3B0 = 0.2f + ((0.8f * (f32)(0xF - cnt)) / 15.0f);
        }
        u32 t = this->_398;
        if (t != 0) {
            this->_3B0 = this->_3B0 + 0.02f * (f32)t;
            return;
        }
        break;
    }
    case 4:
        f32 angle = static_cast<f32>(this->_390) / 120.0f;
        angle = JGeometry::TUtil< f32 >::PI() * angle;
        angle = 0.5f * angle;
        f32 height = 1.0f - JMACosRadian(angle);
        height = 160.0f * height;
        height = height * 0.8f;
        this->_3B0 = 0.2f + height / 160.0f;
        break;
    }
}

void MarioActor::fn_803C5E20(MtxPtr mtx) {
    u32 t = this->_394 - 1;
    this->_394 = t;
    this->_3B0 = 1.0f + (((0.15f * (f32)this->_394) / 30.0f) * JMASinRadian((2.0f * ((f32)t * JGeometry::TUtil< f32 >::PI())) / 15.0f));
}

void MarioActor::fn_803C5ED0(MtxPtr m1, MtxPtr m2) {
    u16 t = this->mBlendMtxTimer;
    if (t != 0) {
        f32 inverseBlend = 1.0f - this->_EA0;
        inverseBlend *= (f32)(t - 1) / (f32)t;
        f32 f = 1.0f - inverseBlend;
        this->_EA0 = f;
        MR::blendMtx(m2, m1, f, m1);
        this->mBlendMtxTimer = this->mBlendMtxTimer - 1;
    }
}

void MarioActor::jumpHop() {
    // https://decomp.me/scratch/d3cJS
    if (!isJumping()) {
        return;
    }

    if (!mMario->isRising()) {
        mMario->cutGravityElementFromJumpVec(true);

        mMario->mJumpVec += -mAppliedGravityVec * getConst().getTable()->mClapJumpBonusFalling;
    } else {
        f32 val = -mMario->cutGravityElementFromJumpVec(true);
        if (val < getConst().getTable()->mHopLimit) {
            val += getConst().getTable()->mClapJumpBonusRising;
            if (val > getConst().getTable()->mHopLimit) {
                val = getConst().getTable()->mHopLimit;
            }
        }

        mMario->mJumpVec += -mAppliedGravityVec * val;
    }

    if (mMario->_430 == 5) {
        mMario->_430 = 0;
        mMario->mMovementStates.mLowWord &= 0xFFDFFFFFU;

        mMario->changeAnimation(nullptr, "落下");
    }

    if (mMario->_430 == 11) {
        mMario->_430 = 0;
    }

    mMario->_4B0 = mMario->mPosHolder->mPos;
}

// NONMATCHING: commutative floating operand/register order and verified pooled literals.
void MarioActor::calcCenterPos() {
    if (_934 || mInitializing90) {
        MR::copyJointPos(this, "Spine1", &_240);

        return;
    }

    TVec3f vec24;
    if (mMario->isStatusActive(MarioStatus_Wall)) {
        vec24 = mMario->_75C;
    } else {
        if (mMario->isStatusActive(MarioStatus_Hang)) {
            MR::copyJointPos(this, "Spine1", &_240);

            return;
        }

        if (mMario->isSwimming()) {
            vec24 = mBodyUpDirection;
        } else {
            vec24 = -*mMario->getGravityVec();
        }
    }

    f32 val;
    if (mMario->isPlayerModeYoshi() && mMario->isStatusActive(0x27) && !MR::isDead(mYoshiHolder->mRideModelActor)) {
        f32 reduction = 0.8f;
        f32 radius = mBinder->mRadius;
        val = 20.0f + reduction * radius;
        mBinder->mRadius = val;
    } else if (((mMario->getMovementStates().mLowWord >> 21) & 1U)) {
        f32 radius = mBinder->mRadius;
        f32 height = 40.0f;
        val = 0.8f * radius + 0.2f * height;
        mBinder->mRadius = val;
    } else {
        f32 radius = mBinder->mRadius;
        f32 height = 60.0f;
        val = 0.9f * radius + 0.1f * height;
        mBinder->mRadius = val;
    }

    if (mPlayerMode == 6) {
        f32 radius = mBinder->mRadius;
        f32 height = 100.0f;
        f32 val2 = 0.8f * radius + 0.2f * height;
        val = 20.0f + val2;
        mBinder->mRadius = val2;
    }

    _240 = mPosition + vec24 * val;
}

void MarioActor::updateCameraInfo() {
    mSwimSurfaceState1010 = false;

    if (MR::diffAngleAbs(mCameraBasisZ, MR::getCamZdir()) >= PI / 6.0f) {
        mSwimSurfaceState1010 = true;
    }

    if ((mCameraPosition - MR::getCamPos()).length() > 500.0f) {
        mSwimSurfaceState1010 = true;
    }

    if (mSwimSurfaceState1010) {
        mMario->changeAnimationInterpoleFrame(0);
        stopSpinTicoEffect(true);
        mBlendMtxTimer = 0;
    }

    mCameraPosition = MR::getCamPos();
    mCameraBasisX = MR::getCamXdir();
    mCameraBasisY = MR::getCamYdir();
    mPostureReference1044 = mCameraBasisZ;
    mCameraBasisZ = MR::getCamZdir();
}

void MarioActor::updateTransForCamera() {
    if (_EA5) {
        return;
    }

    if (isNerve(GET_NERVE(MarioActor, MarioActorNrvGameOverBlackHole))) {
        mCameraTrans = mBlackHolePosition528 * 0.1f + mCameraTrans * 0.9f;
        return;
    }

    if (mMario->isStatusActive(MarioStatus_Step) || mMario->isStatusActive(MarioStatus_Bump)) {
        mCameraTrans = mCameraTrans * 0.8f + mPosition * 0.2f;
        return;
    }

    mCameraTrans = mPosition + TVec3f(mMario->_13C);  // matches now. result of getter?

    if (mMario->isSwimming() && mMario->mSwim->isOnWaterSurface()) {
        if (mWaterSurfaceCameraTimer662 < 100) {
            mWaterSurfaceCameraTimer662++;
        }
    } else if (mWaterSurfaceCameraTimer662 != 0) {
        mWaterSurfaceCameraTimer662--;
    }

    mCameraTrans += mMario->mHeadVec * mWaterSurfaceCameraTimer662;
}

void MarioActor::updateForCamera() {
    if (_EA5) {
        return;
    }

    if (!(_2A0 == mMario->mHeadVec)) {
        _2A0 = mMario->mHeadVec;
        mCounter660 = mConst->getTable()->mCameraHeadRotationTimer;
    }

    if (mCounter660 != 0) {
        mCounter660--;

        if (!MR::vecBlendSphere(_270, _2A0, &_270, 1.0f - static_cast< f32 >(mCounter660) / mConst->getTable()->mCameraHeadRotationTimer)) {
            _270 = _2A0;
        }
    }

    _27C = mMario->mFrontVec;
    _288 = mMario->mSideVec;
}

void MarioActor::bodyClap() {
    const TVec3f* origin;
    HitSensor** sensorSlot;
    u32 i = 0;
    if (_7DC == 0) return;
    origin = &_240;
    for (; i < _7DC; i++) {
        sensorSlot = &mTornadoSensors[i];
        TVec3f stack_14((*sensorSlot)->mPosition);
        TVec3f stack_8(stack_14 - *origin);
        if (stack_8.length() < 100.0f) {
            (*sensorSlot)->receiveMessage(ACTMES_ITEM_GET, getSensor("Body"));
        }
    }
}

// NONMATCHING: vector stack slots, saved registers, and pooled scalar offsets.
bool MarioActor::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    Mario* player;
    const TVec3f* sensorPos;
    LiveActor* host;
    if (mPlayerMode == 6) {
        if (msg == 0x69) {
            setPlayerMode(0, true, true);
            return true;
        }
        return false;
    }
    if (mPlayerMode == 3 && msg == 0x6B) msg = 0x61;
    if (_934) {
        if (!mMario->isEnableAddDamage()) return false;
        if (cylinderHitCheck(pSender, nullptr)) return MR::sendArbitraryMsg(msg, _924, pSender);
        return false;
    }
    if (!isNerve(GET_NERVE(MarioActor, MarioActorNrvWait))) return false;
    if (msg == 0x6D) {
        if (mPlayerMode == 1) return false;
        decLife(0);
        return true;
    }
    if (!_FCC && !_FCD && !mFlag1052) {
        if (!cylinderHitCheck(pSender, nullptr)) {
            if (msg == 0x5F && _3E5) {
                TVec3f direction(*MR::getSensorPos(pReceiver) - *MR::getSensorPos(pSender));
                if (MR::isNearZero(direction)) getFrontVec(&direction);
                direction.setLength(15.0f);
                return mMario->doFlipRot(direction);
            }
            return false;
        }
        if (!mMario->isDamaging()) {
            TVec3f offset(*MR::getSensorPos(pSender) - *MR::getSensorPos(pReceiver));
            if (checkAndTryTrampleAttack(pSender, offset.length(), true)) return false;
        }
    }
    bool handled;
    bool result = tryAttackMsg(msg, pSender, &handled);
    if (handled) {
        if (result && fn_803CAF80()) mMessenger568->addRequest(pSender, 7);
        return result;
    }
    if (msg == 0x64) {
        player = mMario;
        sensorPos = MR::getSensorPos(pSender);
        TVec3f offset(_240);
        offset -= *sensorPos;
        host = MR::getSensorHost(pSender);
        if (player->fn_803B28E0(*MR::getSensorPos(pSender), offset, host)) return false;
    }
    MR::getSensorRadius(pSender);
    TVec3f direction(*MR::getSensorPos(pReceiver) - *MR::getSensorPos(pSender));
    direction.length();
    MR::getSensorRadius(pReceiver);
    if (MR::isNearZero(direction)) getFrontVec(&direction);
    direction.setLength(30.0f);
    result = tryVectorAttackMsg(msg, direction);
    if (!result) {
        MR::sendArbitraryMsg(0x28, pSender, getSensor("Body"));
    } else {
        u8 autoBind;
        if (mActionTable1080->selectAutoBind(MR::getSensorHost(pSender)->mName, &autoBind)) applyAutoBindPlayerMode(autoBind);
        if (fn_803CAF80()) mMessenger568->addRequest(pSender, 7);
        if (msg == 0x60) printHitMark(pSender);
    }
    return result;
}

// NONMATCHING: equivalent message switch lowering and string address scheduling.
bool MarioActor::tryAttackMsg(u32 msg, const HitSensor* pSensor, bool* pHandled) {
    if (pHandled != nullptr) {
        *pHandled = true;
    }

    switch (msg) {
    case ACTMES_ENEMY_ATTACK_FLIP_VERYWEAK:
        if (((getMovementStates().mLowWord >> 30) & 1U) && !isAnimationRun(nullptr)) {
            mMario->changeAnimation("ノーダメージ");
        }

        return true;

    case 0x65:
    case 0x66:
    case 0x67: {
        const char* effect = "青い炎エフェクト";
        if (mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, effect)) {
            _1B4 = true;
        } else {
            _1B4 = false;
        }

        mMario->fn_803949E0();

        return true;
    }

    case 0x68:
    case 0x69:
        return mMario->doParalyze();

    case 0x6B:
        return mMario->doFreeze();

    case 0x6C:
        return mMario->requestCrush();

    default:
        if (pHandled != nullptr) {
            *pHandled = false;
        }

        return false;
    }
}

bool MarioActor::tryVectorAttackMsg(u32 msg, const TVec3f& rVec) {
    TVec3f newVec;
    bool out = false;
    switch (msg) {
    case ACTMES_ENEMY_ATTACK_MAXIMUM:
        out = mMario->fn_80394550(&rVec);
        break;

    case ACTMES_ENEMY_ATTACK_EXPLOSION:
        out = mMario->damage(rVec);
        break;

    case ACTMES_ENEMY_ATTACK_FLIP_MAXIMUM:
        newVec = rVec;
        newVec.setLength(15.0f);
        out = mMario->doFlipLarge(newVec);
        break;

    case ACTMES_ENEMY_ATTACK_FLIP:
        newVec = rVec;
        newVec.setLength(10.0f);
        out = mMario->doFlipLarge(newVec);
        break;

    case ACTMES_ENEMY_ATTACK_FLIP_ROT:
        if (!isJumping()) {
            out = mMario->doFlipRot(rVec);
        }

        break;

    case ACTMES_ENEMY_ATTACK_FLIP_WEAK:
        if (!isJumping()) {
            out = mMario->doFlipWeak(rVec);
        }

        break;

    case ACTMES_ENEMY_ATTACK_FLIP_WEAK_JUMP:
        out = mMario->doFlipJump(rVec);
        break;

    case ACTMES_ENEMY_ATTACK_FLIP_JUMP:
        newVec = rVec;
        newVec.setLength(15.0f);
        out = mMario->doFlipBackRoll(newVec);
        break;

    case ACTMES_ENEMY_ATTACK_COUNTER_SPIN:
        if (!isJumping()) {
            out = mMario->doFlipRot(rVec);
        } else {
            mMario->stopWalk();
            newVec = rVec;
            newVec.setLength(20.0f);
            mMario->addReaction(newVec);
        }

        break;

    case ACTMES_ENEMY_ATTACK_COUNTER_HIPDROP:
        _988 = 2;
        trampleJump(mConst->getTable()->mTrampleNormal, mConst->getTable()->mTrampleLong);
        break;

    default:
        if (isJumping() || msg == ACTMES_ENEMY_ATTACK_STRONG) {
            out = mMario->damage(rVec);
        } else {
            out = mMario->faint(rVec);
        }

        break;
    }

    return out;
}

bool MarioActor::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    mMario->resetSleepTimer();
    if (pSender == _424) {
        return false;
    }

    if (pSender == getCarrySensor()) {
        return false;
    }

    mMario->setNotSafetyTimer();
    if (mMario->isStatusActive(MarioStatus_Stick)) {
        return false;
    }

    if (mMario->isStatusActive(MarioStatus_Crush)) {
        return false;
    }

    if (!isEnableMoveMario()) {
        return false;
    }

    if (_934) {
        return MR::sendMsgPush(_924, pSender);
    }

    TVec3f push;
    if (!cylinderHitCheck(pSender, &push)) {
        return false;
    }

    if (!MR::isNearZero(push)) {
        if (!mMario->isDamaging()) {
            if (checkAndTryTrampleAttack(pSender, (*MR::getSensorPos(pSender) - *MR::getSensorPos(pReceiver)).length(), true)) {
                return false;
            }

            if (((mMario->mMovementStates.mLowWord >> 20) & 1U) && ((mMario->mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && tryHipDropAttack(pSender)) {
                return false;
            }
        }

        if (mMario->isSwimming() && getCarrySensor() != nullptr && (pSender->isType(ATYPE_JET_TURTLE) || pSender->isType(ATYPE_JET_TURTLE_SLOW))) {
            mMario->startPadVib(2);
            pSender->receiveMessage(0x2E, getSensor("Body"));
            return false;
        }

        if (strstr(MR::getSensorHost(pSender)->mName, "ゴーストマリオ") != nullptr && (isJumping() || mMario->isStateHanging())) {
            return false;
        }

        if (mMario->isActiveTaskID(0x200)) {
            return false;
        }

        mMario->push(push * 1.1f);
        if (mMario->isStatusActive(MarioStatus_Foo)) {
            TVec3f direction = *MR::getSensorPos(pReceiver) - *MR::getSensorPos(pSender);
            if (!MR::normalizeOrZero(&direction)) {
                mMario->mFoo->hitWall(direction, nullptr);
            }
        }

        if (isJumping()) {
            if (mMario->_3BC > 1) {
                if (push.dot(mMario->mJumpVec) < 0.0f) {
                    MR::normalizeOrZero(&push);
                    mMario->cutVecElementFromJumpVec(push);
                }

                if (!mMario->isRising() && !MR::sendMsgPush(pSender, pReceiver)) {
                    if (((mMario->mMovementStates.mLowWord >> 20) & 1U)) {
                        mMario->startHipDropSlide(pSender);
                    } else {
                        mMario->startJumpDropSlide(pSender);
                    }
                }
            }
        }

        return true;
    }

    return false;
}

bool MarioActor::receiveMsgTaken(HitSensor* pSender, HitSensor* pReceiver) {
    if (_934) {
        return false;
    }

    if (mMario->isSwimming() && !mMario->isStatusActive(MarioStatus_Foo)) {
        if (!mMario->mSwim->startJet(pSender)) {
            return false;
        }
    } else if (pSender != _424) {
        return false;
    } else if (_B92 == 0) {
        mMarioAnim->updateTakingAnimation(pSender);
    }

    memorizeSensorThrow(pSender);

    return true;
}

namespace {
inline bool tryWaterBounce(Mario* mario, TVec3f impulse) {
    return mario->fn_803972D0(&impulse);
}
}

bool MarioActor::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgTaken(msg)) return receiveMsgTaken(pSender, pReceiver);
    if (_934) {
        if (_924->isType(0x68)) return _924->receiveMessage(msg, pSender);
        if (msg == ACTMES_INHALE_BLACK_HOLE && fn_803CAEE0()) {
            if (!_924->receiveMessage(ACTMES_RUSH_CANCEL, getSensor("Body")) || _934) {
                RushEndInfo info(nullptr, 4, TVec3f(0.0f, 0.0f, 0.0f), false, 0);
                endRush(&info);
            }
        } else {
            return false;
        }
    }

    switch (msg) {
    case 0x34:
        if (mMario->isStatusActive(MarioStatus_Hang)) {
            mMario->mHang->forceDrop();
        }

        return receiveMsgPush(pSender, pReceiver);
    case 0x7F:
        mMario->invalidateRelativePosition();
        break;
    case 0x81:
        if (((mMario->mFlags20.mWord >> 23) & 1U)) {
            return false;
        }

        mBlackHolePosition528 = *MR::getSensorPos(pSender);
        mBlackHole = static_cast< BlackHole* >(MR::getSensorHost(pSender));
        forceGameOverBlackHole();
        return true;
    case 0x49:
        if (mMario->getCurrentStatus() != MarioStatus_Damage && MR::isSensorPlayer(pReceiver)) {
            mMario->mMovementStates.mLowBits.mBit26 = 0;
            mMario->mMovementStates.mHighBits.mBit20 = 1;
            mMario->_402 = 0;
            mMario->cutGravityElementFromJumpVec(true);
            return true;
        }

        break;
    case 0x36:
        if (mMario->getCurrentStatus() == MarioStatus_Damage) {
            break;
        }

        if (MR::isSensorPlayer(pReceiver)) {
            TVec3f velocity(*MR::getSensorPos(pReceiver) - *MR::getSensorPos(pSender));
            if (!isJumping()) {
                MR::vecKillElement(velocity, *mMario->getGravityVec(), &velocity);
                velocity.setLength(mConst->getTable()->mMsgJumpGroundPowerXZ);
                velocity.add(-*mMario->getGravityVec() * mConst->getTable()->mMsgJumpGroundPowerY);
            } else {
                velocity.setLength(mConst->getTable()->mMsgJumpAirPower);
            }

            mMario->tryForceFreeJump(velocity);
            mMario->mMovementStates.mLowWord &= 0xFFBFFFFFU;
            mMario->mMovementStates.mLowWord &= 0xFDFFFFFFU;
            return true;
        }

    case 0x37:
        if (mMario->getCurrentStatus() != MarioStatus_Damage && MR::isSensorPlayer(pReceiver)) {
            if (!cylinderHitCheck(pSender, nullptr)) {
                return false;
            }

            TVec3f velocity(*MR::getSensorPos(pReceiver) - *MR::getSensorPos(pSender));
            if (!isJumping()) {
                MR::vecKillElement(velocity, *mMario->getGravityVec(), &velocity);
                velocity.setLength(mConst->getTable()->mMsgJumpGroundPowerXZ);
                velocity.add(-*mMario->getGravityVec() * mConst->getTable()->mMsgJumpGroundPowerY);
            } else {
                velocity.setLength(mConst->getTable()->mMsgJumpAirPower);
            }

            mMario->tryForcePowerJump(velocity, false);
            mMario->stopAnimation(nullptr);
            mMario->mMovementStates.mLowWord &= 0xFFBFFFFFU;
            mMario->mMovementStates.mLowWord &= 0xFDFFFFFFU;
            mMario->playSound("声壁反射", -1);
            return true;
        }

        break;
    case 0x38: {
        if (!isJumping()) {
            return receiveMsgPush(pSender, pReceiver);
        }

        if (!cylinderHitCheck(pSender, nullptr)) {
            return false;
        }

        TVec3f direction(*MR::getSensorPos(pReceiver) - *MR::getSensorPos(pSender));
        MR::normalizeOrZero(&direction);
        f32 speed = mMario->cutVecElementFromJumpVec(direction);
        if (speed >= 0.0f) {
            return false;
        }

        f32 impulse = speed - mConst->getTable()->mBilliardJumpExtra;
        mMario->mJumpVec.add(-direction * impulse);
        mMario->_430 = 3;
        return true;
    }

    case 0x39:
        doTrampleJump(pSender);
        mMario->startPadVib(3);
        mMario->mJumpVec -= mMario->mFrontVec * 10.0f;
        mMario->changeAnimation("飛びすさりジャンプ");
        break;
    case 0x48: {
        TVec3f normal;
        MR::calcDistance(pSender, pReceiver, &normal);
        mMario->reflectWallOnSpinning(normal, 5);
        if (mMario->mWalkSpeed > 0.5f) {
            mMario->mWalkSpeed = 0.5f;
        }

        return true;
    }

    case 0x58:
        if (mMario->isStatusActive(MarioStatus_Slider)) return mMario->mSlider->pushSensor(pSender, pReceiver);
        return false;
    case 0x35:
        if (mMario->isPlayerModeYoshi()) MR::startActionSound(mYoshiHolder->mYoshi, "Hit", -1, -1, -1);
        else mMario->playSound("声ランニングキック", -1);
        mMario->changeAnimation("ランニングキック");
        return true;
    case 0x4B:
        if (mMario->_97C == nullptr) {
            return false;
        }

        return mMario->_97C->passRing(pSender);
    case 0x4C: {
        mMario->touchWater();
        TVec3f horizontal;
        MR::vecKillElement(*MR::getSensorPos(pReceiver) - *MR::getSensorPos(pSender), *mMario->getGravityVec(), &horizontal);
        if (horizontal.length() >= MR::getSensorRadius(pSender)) {
            return receiveMsgPush(pSender, pReceiver);
        }
        if (MR::normalizeOrZero(&horizontal)) {
            getFrontVec(&horizontal);
            horizontal.negate();
        }
        bool flowing = tryWaterBounce(mMario, (-*mMario->getGravityVec() + horizontal) * 10.0f);
        if (flowing) {
            mMario->playSound("水弾かれ", -1);
            mMario->playSound("声噴水ジャンプ", -1);
        }

        return flowing;
    }

    case 0x2D:
        tryReleaseDirect(pSender);
        break;
    case 0x31:
        _9F2 = 60;
        break;
    case 0x50:
        _3A8 = 3;
        break;
    case 0x4F:
        if (_468 != 0) {
            return false;
        }

        if (mMario->isDamaging()) {
            return false;
        }

        if (!mPlayerMode || mPlayerMode == PlayerMode_Invincible) {
            _B94 = 2;
            changeAnimationUpper("ハンマー投げ回転中");
            return true;
        }

        return false;
    case 0x51:
        return mMario->doObjWarp(MR::getSensorHost(pSender));
    case 0x57:
        mFlag1064 = 1;
        return true;
    }

    return false;
}

bool MarioActor::cylinderHitCheck(const HitSensor* pSensor, TVec3f* pPush) {
    Mario* pMario = mMario;
    f32 radius = pSensor->mRadius;
    TVec3f delta = pSensor->mPosition - (_240 + pMario->_350 + pMario->_35C + mVelocity);
    TVec3f radial;
    f32 axial = MR::vecKillElement(delta, mBodyDownDirection, &radial);
    f32 spherePenetration = 100.0f + radius - delta.length();
    if (spherePenetration < 0.0f) {
        return false;
    }

    if (axial > -radius) {
        f32 radialPenetration = radius + mBodyRadius7A0 - radial.length();
        f32 axialPenetration = radius + mBodyHeight7A4 - axial;
        if (radialPenetration > 0.0f && axialPenetration > 0.0f) {
            if (pPush != nullptr) {
                MR::normalizeOrZero(&radial);
                pPush->zero();
                if (spherePenetration < radialPenetration && spherePenetration < axialPenetration) {
                    MR::normalizeOrZero(&delta);
                    pPush->add(-delta * spherePenetration);
                } else if (radialPenetration < axialPenetration) {
                    pPush->add(-radial * radialPenetration);
                } else {
                    pPush->add(-mBodyDownDirection * axialPenetration);
                    pPush->add(-radial);
                }
            }

            return true;
        }
    }

    if (((mMario->getMovementStates().mLowWord >> 21) & 1U) && ((mMario->mMovementStates.mLowWord >> 30) & 1U)) {
        return false;
    }

    axial = MR::vecKillElement(delta, mBodyUpDirection, &radial);
    if (axial < -radius) {
        return false;
    }

    f32 radialPenetration = radius + mBodyRadius7A0 - radial.length();
    f32 axialPenetration = radius + mBodyHeight7A4 - axial;
    if (radialPenetration > 0.0f && axialPenetration > 0.0f) {
        if (pPush != nullptr) {
            MR::normalizeOrZero(&radial);
            pPush->zero();
            if (spherePenetration < radialPenetration && spherePenetration < axialPenetration) {
                MR::normalizeOrZero(&delta);
                pPush->add(-delta * spherePenetration);
            } else if (radialPenetration < axialPenetration) {
                pPush->add(-radial * radialPenetration);
            } else {
                pPush->add(-mBodyUpDirection * axialPenetration);
                pPush->add(-radial);
            }
        }

        return true;
    }

    return false;
}

void MarioActor::initFireMario() {
    if (MR::isCreatePlayerResourceFire()) {
        this->initFireBall();
        this->_3D2 |= 4;
    }
}

// NONMATCHING: verified pooled literal addresses only.
void MarioActor::initBeeMario() {
    const char* modelName;
    const char* archiveName;
    if (gIsLuigi) {
        modelName = "BeeLuigi";
        archiveName = "BeeLuigi.arc";
    } else {
        modelName = "BeeMario";
        archiveName = "BeeMario.arc";
    }

    if (!MR::isCreatePlayerResourceBee()) {
        return;
    }

    MR::createAndAddResourceHolder(archiveName);

    _9E4 = new ModelHolder(modelName, true);
    _9E4->initWithoutIter();

    mAllModel->registerModel(2, _9E4, false, true);
    _9E4->kill();
    MR::hideModel(_9E4);
    _9EC = MR::initFurPlayer(_9E4);
    MR::hideJoint(_9E4, "Face0");

    _9E8 = new MarioParts(this, "B-WING", "BeeMarioWing", false, nullptr, nullptr);
    _9E8->initWithoutIter();

    _9E8->initFixedPosition(mSpine2Matrix.toMtxPtr(), TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    _9E8->makeActorDead();

    _9F0 = 0;
    _3D2 |= 0x10;
}

// NONMATCHING: verified pooled literal addresses only.
void MarioActor::initHopperMario() {
    if (!MR::isCreatePlayerResourceHopper()) {
        return;
    }
    const char* modelName;
    const char* archiveName;
    if (gIsLuigi) {
        modelName = "HopperLuigi";
        archiveName = "HopperLuigi.arc";
    } else {
        modelName = "HopperMario";
        archiveName = "HopperMario.arc";
    }
    MR::createAndAddResourceHolder(archiveName);
    mModelEDC = new ModelHolder(modelName, true);
    mModelEDC->initWithoutIter();
    mAllModel->registerModel(5, mModelEDC, false, true);
    mModelEDC->kill();
    MR::hideModel(mModelEDC);
    MR::hideJoint(mModelEDC, "Face0");
    _3D2 |= 0x20;
}

// NONMATCHING: verified pooled literal addresses only.
void MarioActor::initIceMario() {
    const char* modelName;
    const char* archiveName;
    if (gIsLuigi) {
        modelName = "IceLuigi";
        archiveName = "IceLuigi.arc";
    } else {
        modelName = "IceMario";
        archiveName = "IceMario.arc";
    }

    if (!MR::isCreatePlayerResourceIce()) {
        return;
    }

    MR::createAndAddResourceHolder(archiveName);

    mModelE9C = new ModelHolder(modelName, true);
    mModelE9C->initWithoutIter();

    mAllModel->registerModel(3, mModelE9C, true, false);
    mModelE9C->kill();
    MR::hideModel(mModelE9C);

    _B4C = new IceStep*[20];
    for (u32 i = 0; i < 20; i++) {
        _B4C[i] = new IceStep("アイス床");
        _B4C[i]->initWithoutIter();
        _B4C[i]->kill();
    }

    _B50 = 0;
    _3D2 |= 0x8;
}

// NONMATCHING: verified pooled literal addresses only.
void MarioActor::initInvincibleMario() {
    const char* modelName;
    const char* archiveName;
    if (gIsLuigi) {
        modelName = "InvincibleLuigi";
        archiveName = "InvincibleLuigi.arc";
    } else {
        modelName = "InvincibleMario";
        archiveName = "InvincibleMario.arc";
    }

    if (!MR::isCreatePlayerResourceInvincible() && !MR::isGalaxyRainbowCometAppearInCurrentStage()) {
        return;
    }

    MR::createAndAddResourceHolder(archiveName);

    _9C0 = new ModelHolder(modelName, true);
    _9C0->initWithoutIter();

    mAllModel->registerModel(4, _9C0, true, false);
    _9C0->kill();
    MR::hideModel(_9C0);
    MR::startBrk(_9C0, "InvincibleMario");
    _3D2 |= 0x2;
}

void MarioActor::initTeresaMario() {
    mTeresaModel = MarioTeresaModel::create(this, gIsLuigi);
    _3D2 |= 0x40;
}

void MarioActor::initTornadoMario() {
    const char* modelName = gIsLuigi ? "LuigiTornado" : "MarioTornado";
    if (!MR::isCreatePlayerResourceTornado()) {
        return;
    }

    mTornadoMario = new TornadoMario(modelName);
    mTornadoMario->initWithoutIter();
    _990 = 0;
    _3D2 |= 0x200;
}

void MarioActor::initBoneMario() {
    const char* modelName = gIsLuigi ? "BoneLuigi" : "BoneMario";
    mModelEE0 = new ModelHolder(modelName, true);
    mModelEE0->initWithoutIter();
    mAllModel->registerModel(1, mModelEE0, false, false);
    mModelEE0->kill();
    MR::hideModel(mModelEE0);
    _3D2 |= 0x100;
}

void MarioActor::initYoshi() {
    if (MR::isCreatePlayerResourceYoshi()) {
        mYoshiHolder = new YoshiHolder(this->mMario);
        mYoshiHolder->fn_8041B700(this);
        this->_3D2 |= 0x400;
    }
}

void MarioActor::initCloudMario() {
    if (MR::isCreatePlayerResourceCloud()) {
        const char* name;
        if (gIsLuigi) {
            name = "CloudLuigi";
        } else {
            name = "CloudMario";
        }
        ModelHolder* mh = new ModelHolder(name, true);
        this->mModelEEC = mh;
        mh->initWithoutIter();
        mAllModel->registerModel(7, mModelEEC, true, true);
        mModelEEC->kill();
        MR::hideModel(mModelEEC);
        MR::hideJoint(mModelEEC, "Cap0");
        MR::hideJoint(mModelEEC, "Face0");
        CloudMarioHat* hat = new CloudMarioHat("雲マリオの帽子");
        mCloudMarioHatEF0 = hat;
        hat->initWithoutIter();
        this->_3D2 |= 0x1000;
    }
}

void MarioActor::initRockMario() {
    if (MR::isCreatePlayerResourceRock()) {
        const char* name;
        if (gIsLuigi) {
            name = "RockLuigi";
        } else {
            name = "RockMario";
        }
        ModelHolder* mh = new ModelHolder(name, true);
        this->mModelEF4 = mh;
        mh->initWithoutIter();
        mAllModel->registerModel(8, mModelEF4, true, true);
        mModelEF4->kill();
        MR::hideModel(mModelEF4);
        MR::hideJoint(mModelEF4, "Face0");
        this->_3D2 |= 0x2000;
    }
}

void MarioActor::fn_803C8AA0() {
    J3DModelX* m = (J3DModelX*)mAllModel->getJ3DModel();
    if (this->mFlags.mIsNoCalcView == 0) {
        m->viewCalc3(0, nullptr);
        Mtx inv1, inv2;
        PSMTXInverse(mJointTransform294.mMtx, inv1);
        PSMTXInverse(mAllModel->getJ3DModel()->getBaseTRMtx(), inv2);
        PSMTXConcat(inv1, inv2, mReflectionBaseMtx264.mMtx);
    }
}

void MarioActor::calcViewAndEntry() {
    fn_803D26F0();
    J3DModelX* model = static_cast<J3DModelX*>(mAllModel->getJ3DModel());
    model->mDisplayListMask &= ~(1U << 3);
    model->swapDrawBuffer(4);
    mSimpleModelF08->swapDrawBuffer(4);
    fn_803C8AA0();
    calcViewSilhouetteModel();
    fn_803D32F0();
    model->setDrawView(0);
    calcViewBlurModel();
    if (_B48) calcViewFootPrint();
    if (mMario->isStatusActive(MarioStatus_FpView)) calc1stPersonView();
    else calcFogLighting();
    mAllModel->fn_803E7BF0(mEyeTextureFrameF9A);
    if (!mReflectionSuppressed && (mGroundReflectionEFC || mWaterReflectionEF8 || mSphereReflectionEF9)) calcViewReflectionModel();
    else { mFlagF18 = 0; mFlagF19 = 0; }
    calcViewSearchLight();
    if (mModelF1C) mModelF1C->updateMtx(mHandLJointMatrix.toMtxPtr());
    if (_A10) _A10->updateMtx(mHandRJointMatrix.toMtxPtr());
    mModelF24->updateMtx(mHandLJointMatrix.toMtxPtr());
    mModelF28->updateMtx(mHandRJointMatrix.toMtxPtr());
    if (mHandModelL) mHandModelL->updateMtx(mHandLJointMatrix.toMtxPtr());
    if (mHandModelR) mHandModelR->updateMtx(mHandRJointMatrix.toMtxPtr());
    mModelF38->updateMtx(mFaceJointMatrix.toMtxPtr());
    updateDarkMask(0x96);
    updateDarkMask(0x96);
    if (MR::isDemoActive()) _1B8->kill();
}

// NONMATCHING: verified pooled literal offsets only.
void MarioActor::initFace() {
    mFaceJointCountF37 = 8;
    MR::hideJoint(mAllModel->mModels[0], "Face0");
    mFaceJointIndexF3C = 0;

    if (gIsLuigi) {
        mModelF38 = new ModelHolder("LuigiFace", true);
    } else {
        mModelF38 = new ModelHolder("MarioFace", true);
    }

    MR::initDLMakerFog(mModelF38, true);
    MR::newDifferedDLBuffer(mModelF38);

    MR::getJ3DModelData(mModelF38)->mMaterialTable.mTexture = MR::getJ3DModelData(this)->mMaterialTable.mTexture;
    MR::getJ3DModelData(mModelF38)->mMaterialTable.mTextureName = MR::getJ3DModelData(this)->mMaterialTable.mTextureName;

    for (u32 i = 0; i < mFaceJointCountF37; i++) {
        copyMaterial(MR::getJ3DModel(mModelF38), MR::getJointIndex(this, "Face0"), i);
    }
}

void MarioActor::copyMaterial(J3DModel* pModel, u16 materialNo, s32 packetIndex) {
    J3DModelData* modelData = mAllModel->getJ3DModel()->getModelData();
    J3DMaterial* material = modelData->getJointNodePointer(materialNo)->getMesh();
    if (material == nullptr) {
        return;
    }

    u16 materialIndex = material->mIndex;

    u16 start;
    u16 end;

    if (packetIndex == -1) {
        start = 0;
        end = 7;
    } else {
        start = static_cast< u16 >(packetIndex);
        end = static_cast< u16 >(packetIndex);
    }

    for (u16 i = start; i <= end; i++) {
        J3DMatPacket* matPacket = pModel->getMatPacket(i);
        matPacket->mpMaterial = modelData->getMaterialNodePointer(materialIndex);
        J3DShapePacket* shapePacket = &pModel->mShapePacket[i];
        matPacket->mpInitShapePacket = shapePacket;
        matPacket->addShapePacket(shapePacket);
        matPacket->mpTexture = modelData->mMaterialTable.mTexture;
        matPacket->mpDisplayListObj = modelData->getMaterialNodePointer(materialIndex)->mSharedDLObj;
    }
}

// NONMATCHING: equivalent model switch lowering, Animator receiver load scheduling, and verified literal.
void MarioActor::updateFace() {
    s32 model = mAllModel->mCurrentModel;
    if ((3 <= model && model <= 4) || model == 1 || model == 6) {
        MR::hideModel(mModelF38);
        return;
    }
    if (mMario->isStatusActive(MarioStatus_FpView)) {
        MR::hideModel(mModelF38);
        return;
    }
    if (MR::isHiddenModel(this) || is481or482On() || ((mMario->mMovementStates.mLowWord >> 16) & 1U) || _1C3) {
        MR::hideModel(mModelF38);
        return;
    }
    MR::showModel(mModelF38);
    TVec3f partsControl;
    MarioAnimator* animator = mMarioAnim;
    animator->fn_803DEFD0(MR::getJointIndex(this, "PartsControl"), &partsControl);
    mFaceJointIndexF3C = MR::clamp(static_cast<s32>(0.49f + partsControl.x / 10.0f), 0, mFaceJointCountF37 - 1);
    for (u16 i = 0; i < mFaceJointCountF37; ++i) {
        J3DJoint* eyeJoint = MR::getJoint(mModelF38, i + 1);
        if (mFaceJointIndexF3C == i) {
            MR::showJoint(eyeJoint);
        } else {
            MR::hideJoint(eyeJoint);
        }
    }
}

void MarioActor::initBlink() {
    mBlinkF9C = 0;
    mBlinkF9E = 0;
    mBlinkF98 = 0;
    mEyeRes = nullptr;
}

// NONMATCHING: equivalent early model guards and verified pooled literal/table addresses.
void MarioActor::updateBlink() {
    if (mAllModel->mCurrentModel == 1) return;
    if (mAllModel->mCurrentModel == 6) return;

    J3DAnmTexPattern* pPattern = mEyeRes;
    if (pPattern != nullptr) {
        f32 frame;

        if (pPattern->mAttribute == 2) {
            const f32 currentFrame = pPattern->mFrame;
            frame = currentFrame;
            frame += 1.0f;

            if (frame >= pPattern->mFrameMax - 1.0f) {
                frame = 0.0f;
            }
        } else {
            frame = mMarioAnim->getFrame();
        }

        mEyeRes->mFrame = frame;

        u16 eyeLidMaterial = MR::getMaterialNo(MR::getJ3DModelData(this), "EyeLid_v");

        for (u16 i = 0; i < mEyeRes->mUpdateMaterialNum; i++) {
            u16 texNo;
            mEyeRes->getTexNo(i, &texNo);

            u16 materialID = mEyeRes->mUpdateMaterialID[i];
            if (materialID == eyeLidMaterial) {
                mEyeTextureFrameF9A = texNo - mAllModel->mEyeLidTextureIndex;
                mBlinkF98 = 1;
                return;
            }
        }
    }

    if (isNerve(GET_NERVE(MarioActor, MarioActorNrvGameOver)) && mEyeRes == nullptr) {
        if (mMario->isAnimationTerminate(nullptr)) {
            mEyeTextureFrameF9A = 2;
        } else {
            mEyeTextureFrameF9A = 1;
        }

        return;
    }

    if (mBlinkF98) {
        mBlinkF98 = 0;
        mBlinkF9C = 0;
        mBlinkF9E = MR::getRandom(60L, 360L);
        mEyeTextureFrameF9A = 0;
    }

    if (mBlinkF9C) {
        mBlinkF9C--;
        mEyeTextureFrameF9A = ::sBlinkStates[9 - mBlinkF9C];
        mBlinkF9E = MR::getRandom(60L, 360L);
        return;
    }

    if (mBlinkF9E != 0) {
        mBlinkF9E--;
    }

    if (mBlinkF9E == 0) {
        mBlinkF9C = 10;
    }
}

// NONMATCHING: scalar literal symbol identities; instruction words match after relocation masking.
bool MarioActor::fn_803C9300(TVec3f&) {
    bool c = this->mMario->isCeiling();
    if ((c || ((this->mMario->mDrawStates.mWord >> (0xA)) & 1)) && this->mBeeWallWalk == 0 && this->_9F2 == 0) {
        Mario* m = this->mMario;
        bool r = false;
        if (((m->mDrawStates.mWord >> (0xA)) & 1)) {
            r = true;
        } else {
            const char* wallCode = MR::getWallCodeString(m->_4C8);
            if (wallCode != nullptr && strcmp(wallCode, "Fur") == 0) {
                r = true;
            }
        }
        if (r) {
            Triangle tri;
            TVec3f hit;
            Mario* m2 = this->mMario;
            TVec3f dir(TVec3f(-mAppliedGravityVec));
            dir.scale(200.0f);
            if (m2->fn_803B2770(&hit, &tri, this->mPosition, dir)) {
                this->entryWallWalkMode(hit, *tri.getNormal(0));
            }
        }
    }
    return false;
}

// NONMATCHING: equivalent Mario pointer reload and verified Fur string pool.
bool MarioActor::checkBeeWallStick(TVec3f& rVec) {
    if (((mMario->mMovementStates.mLowWord >> 23) & 1U) && mMario->checkWallCode("Fur", false) && mBeeWallWalk == 0 && !_9F2) {
        TVec3f head = mMario->mHeadVec;

        mBeeWallWalk = 5;

        rVec = -mMario->getWallNorm();

        mPosition = mMario->getWallPos();

        mMario->stopJump();
        mMario->stopAnimation(nullptr);
        mMario->stopWalk();

        mAppliedGravityVec = rVec;
        mMario->setGravityVec(rVec);
        mMario->setHeadVec(-rVec);
        mMario->setFrontVecKeepUp(head, 1UL);
        setBlendMtxTimer(2);

        _38C = 5;
        mMario->mMovementStates.mHighWord &= 0xFFFFFF7FU;
        mCollisionShadow580->_305 = true;

        return true;
    }

    return false;
}

bool MarioActor::checkBeeFloorStick(TVec3f& rVec) {
    if (((mMario->getMovementStates().mLowWord >> 30) & 1U) && strcmp("Fur", MR::getWallCodeString(mMario->getGroundPolygon())) == 0 && mBeeWallWalk == 0) {
        mBeeWallWalk = 5;
        rVec = -mMario->_368;

        return true;
    }

    return false;
}

// NONMATCHING: Equivalent Mario pointer reload and pooled string addresses.
void MarioActor::updateBeeStickMode(TVec3f& rGravity) {
    if (mBeeWallWalk != 0) {
        bool detach = false;
        Mario* mario = mMario;
        bool fur = mario->mDrawStates.mBits.mBit6;
        if (!fur) {
            const char* pWallCode = MR::getWallCodeString(mario->_45C);
            if (pWallCode != nullptr && strcmp(pWallCode, "Fur") == 0) {
                fur = true;
            }
        }

        if (fur && _9F2 == 0) {
            f32 radius = mConst->getTable()->mBeeWallWalkCancelRadius;
            if (MR::getAreaObj("BeeWallShortDistArea", mPosition) != nullptr) {
                radius = mConst->getTable()->mBeeWallWalkCancelRadiusShort;
            }

            if (mMario->mVerticalSpeed > radius) {
                detach = true;
            } else if (isJumping() && mMario->checkWallCode("Normal", false)) {
                detach = true;
            } else {
                mBeeWallWalk = 5;
            }

            if (isJumping() && isRequestRush()) {
                mBeeWallWalk = 0;
            }
        } else if (isJumping()) {
            if (mMario->mVerticalSpeed < 100.0f) {
                mBeeWallWalk--;
            }

            if (mMario->checkWallCode("Normal", false)) {
                detach = true;
            }

            f32 radius = mConst->getTable()->mBeeWallWalkCancelRadius;
            if (MR::getAreaObj("BeeWallShortDistArea", mPosition) != nullptr) {
                radius = mConst->getTable()->mBeeWallWalkCancelRadiusShort;
            }

            if (mMario->mVerticalSpeed > radius) {
                detach = true;
            }
        } else {
            if (((mMario->mMovementStates.mLowWord >> 20) & 1U)) {
                mMario->mMovementStates.mLowWord &= 0xFFEFFFFFU;
            }

            detach = true;
        }

        if (detach) {
            if (mBeeWallWalk != 0) {
                mBeeWallWalk--;
            }
        }

        if (mBeeWallWalk == 0) {
            mMario->stopWalk();
            mMario->tryJump();
            mMario->_408 = mConst->getTable()->mBeeGravityPowerTime;
            mMario->_3BC = mConst->getTable()->mBeeAirWalkInhibitTime - 5;
            mMario->push(*mMario->_45C->getNormal(0) * 100.0f);
            mMario->cutVecElementFromJumpVec(mPreviousGravityVec);
            return;
        }

        rGravity = -*mMario->_45C->getNormal(0);
    }
}

void MarioActor::fn_803C9920(TVec3f& vec) {
    if (mPlayerMode != 4) {
        this->mBeeWallWalk = 0;
        this->_9F2 = 0;
        return;
    }
    u16 timer = this->_9F2;
    u8 prev = this->mBeeWallWalk;
    if (timer != 0) {
        this->_9F2 = timer - 1;
    }
    if (this->_9F2 != 0 || (!this->fn_803C9300(vec) && !this->checkBeeWallStick(vec) && !this->checkBeeFloorStick(vec))) {
        this->updateBeeStickMode(vec);
    }
    u8 cur = this->mBeeWallWalk;
    if (cur == 5) {
        MR::vecBlendSphere(&vec, mAppliedGravityVec, vec, 0.1f);
        return;
    }
    if (cur != 0) {
        MR::vecBlendSphere(&vec, mAppliedGravityVec, vec, 0.3f);
        return;
    }
    if (prev != 0) {
        this->_9F2 = 0x3C;
    }
}

// NONMATCHING: verified equivalent pooled string addresses.
void MarioActor::initHand() {
    MR::hideJoint(mAllModel->mModels[0], "HandR0");
    MR::hideJoint(mAllModel->mModels[0], "HandL0");

    if (mModelEE0 != nullptr) {
        MR::hideJoint(mAllModel->mModels[1], "HandR0");
        MR::hideJoint(mAllModel->mModels[1], "HandL0");
    }

    if (_9E4 != nullptr) {
        MR::hideJoint(mAllModel->mModels[2], "HandR0");
        MR::hideJoint(mAllModel->mModels[2], "HandL0");
    }

    if (mModelE9C != nullptr) {
        MR::hideJoint(mAllModel->mModels[3], "HandR0");
        MR::hideJoint(mAllModel->mModels[3], "HandL0");
    }

    if (_9C0 != nullptr) {
        MR::hideJoint(mAllModel->mModels[4], "HandR0");
        MR::hideJoint(mAllModel->mModels[4], "HandL0");
    }

    if (mModelEEC != nullptr) {
        MR::hideJoint(mAllModel->mModels[7], "HandR0");
        MR::hideJoint(mAllModel->mModels[7], "HandL0");
    }
    if (mModelEF4 != nullptr) {
        MR::hideJoint(mAllModel->mModels[8], "HandR0");
        MR::hideJoint(mAllModel->mModels[8], "HandL0");
    }
    mHandFlagF35 = 0;
    mHandFlagF36 = 0;
    mModelF1C = nullptr;
    _A10 = nullptr;

    if (mModelE9C != nullptr) {
        mModelF1C = new ModelHolder("IceMarioHandL", true);
        MR::initDLMakerFog(mModelF1C, true);
        MR::newDifferedDLBuffer(mModelF1C);

        _A10 = new ModelHolder("IceMarioHandR", true);
        MR::initDLMakerFog(_A10, true);
        MR::newDifferedDLBuffer(_A10);
    }

    mModelF24 = new ModelHolder("MarioHandL", true);
    MR::initDLMakerFog(mModelF24, true);
    MR::newDifferedDLBuffer(mModelF24);

    mModelF28 = new ModelHolder("MarioHandR", true);
    MR::initDLMakerFog(mModelF28, true);
    MR::newDifferedDLBuffer(mModelF28);

    mHandModelL = nullptr;
    mHandModelR = nullptr;

    if (_9C0 != nullptr) {
        mHandModelL = new ModelHolder("InvincibleMarioHandL", true);
        MR::initDLMakerFog(mHandModelL, true);
        MR::newDifferedDLBuffer(mHandModelL);

        mHandModelR = new ModelHolder("InvincibleMarioHandR", true);
        MR::initDLMakerFog(mHandModelR, true);
        MR::newDifferedDLBuffer(mHandModelR);
    }

    for (u32 idx = 0; idx < MR::getJointNum(mModelF24) - 1; idx++) {
        copyMaterial(MR::getJ3DModel(mModelF24), MR::getJointIndex(this, "HandL0"), idx);
        copyMaterial(MR::getJ3DModel(mModelF28), MR::getJointIndex(this, "HandR0"), idx);
    }

    updateHandAtMorph();
}

void MarioActor::updateHand() {
    bool hideHands = false;
    switch (mPlayerMode) {
    case PlayerMode_Hopper:
        hideHands = true;
        break;
    }

    if (mAllModel->mCurrentModel == 1) hideHands = true;
    if (mAllModel->mCurrentModel == 6) hideHands = true;
    if (getMovementStates().mLowBits.mBit16) {
        hideHands = true;
    }
    if (mMario->isStatusActive(MarioStatus_FpView)) {
        hideHands = true;
    }
    if (_1C3) {
        hideHands = true;
    }

    if (MR::isHiddenModel(this) || is481or482On() || hideHands) {
        mShadowHideHandsF34 = true;
        return;
    }
    mShadowHideHandsF34 = false;

    TVec3f control;
    MarioAnimator* animator = mMarioAnim;
    animator->fn_803DEFD0(MR::getJointIndex(this, "PartsControl"), &control);
    f32 leftControl = control.y;
    f32 rightControl = control.z;
    s32 leftPose = 0.49f + leftControl / 10.0f;
    s32 rightPose = 0.49f + rightControl / 10.0f;
    s32 left = static_cast< u8 >(MR::clamp(leftPose, 0, 7));
    s32 right = static_cast< u8 >(MR::clamp(rightPose, 0, 7));

    ModelHolder* leftHand = mModelF24;
    ModelHolder* rightHand = mModelF28;
    if (mPlayerMode == PlayerMode_Ice) {
        leftHand = mModelF1C;
        rightHand = _A10;
    } else if (mPlayerMode == PlayerMode_Invincible) {
        leftHand = mHandModelL;
        rightHand = mHandModelR;
    }

    if (left != mHandFlagF35) {
        if (mHandFlagF35 != 0xFF) {
            MR::hideJoint(MR::getJoint(leftHand, mHandFlagF35 + 1));
        }
        MR::showJoint(MR::getJoint(leftHand, left + 1));
    }
    if (right != mHandFlagF36) {
        if (mHandFlagF36 != 0xFF) {
            MR::hideJoint(MR::getJoint(rightHand, mHandFlagF36 + 1));
        }
        MR::showJoint(MR::getJoint(rightHand, right + 1));
    }

    mHandFlagF35 = left;
    mHandFlagF36 = right;
}

void MarioActor::updateHandAtMorph() {
    ModelHolder* holder1 = mModelF24;
    ModelHolder* holder2 = mModelF28;

    if (mPlayerMode == 3) {
        holder1 = mModelF1C;
        holder2 = _A10;
    } else if (mPlayerMode == 1) {
        holder1 = mHandModelL;
        holder2 = mHandModelR;
    }

    for (u32 idx = 0; idx <= 7; idx++) {
        MR::hideJoint(MR::getJoint(holder1, idx + 1));
        MR::hideJoint(MR::getJoint(holder2, idx + 1));
    }

    mHandFlagF35 = -1;
    mHandFlagF36 = -1;
}

void MarioActor::drawHand() const {
    if (mShadowHideHandsF34) {
        return;
    }

    if (mPlayerMode == 1) {
        reinterpret_cast< J3DModelX* >(MR::getJ3DModel(mHandModelL))->directDraw(nullptr);
        reinterpret_cast< J3DModelX* >(MR::getJ3DModel(mHandModelR))->directDraw(nullptr);

        return;
    }

    if (MR::isHiddenModel(mModelF24)) {
        return;
    }

    J3DModelX* model = reinterpret_cast< J3DModelX* >(MR::getJ3DModel(mModelF24));
    model->setDynamicDL(mDL[mCurrDL], mCurrentDLSize1A0);
    model->directDraw(nullptr);

    model = reinterpret_cast< J3DModelX* >(MR::getJ3DModel(mModelF28));
    model->setDynamicDL(mDL[mCurrDL], mCurrentDLSize1A0);
    model->directDraw(nullptr);
}

void MarioActor::drawIndirectHand() const {
    if (mShadowHideHandsF34) {
        return;
    }

    if (MR::isHiddenModel(mModelF1C)) {
        return;
    }

    J3DModelX* model = reinterpret_cast< J3DModelX* >(MR::getJ3DModel(mModelF1C));

    if (mPlayerMode != 3) {
        return;
    }

    model->setDynamicDL(mDL[mCurrDL], mCurrentDLSize1A0);
    model->directDraw(nullptr);

    model = reinterpret_cast< J3DModelX* >(MR::getJ3DModel(_A10));
    model->setDynamicDL(mDL[mCurrDL], mCurrentDLSize1A0);
    model->directDraw(nullptr);
}

void MarioActor::fn_803CA1F0() {
    this->mWaterReflectionEF8 = 0;
    this->mSphereReflectionEF9 = 0;
    this->mGroundReflectionEFC = 0;
    this->mShadowEnabledEFA = 0;
    this->mShadowModeEFB = 0;
}

void MarioActor::decreaseLifeByOne() {
    if (mMario->isPlayerModeYoshi()) fn_803B9870();
    if (mHealth) --mHealth;
    this->mLifeDecreasedThisFrame = 1;
}

void MarioActor::fn_803CA270(TVec3f* out, s32 isStatic) {
    TVec3f g;
    MR::calcGravityVectorOrZero(this, this->mPosition, &g, mGravityInfo, 0);
    if (isStatic == 0) {
        MR::calcGravityVectorOrZero(this, this->_240, out, mGravityInfo, 0);
    } else {
        out->set(g);
    }
    TVec3f g2;
    MR::calcGravityVectorOrZero(this, this->mPosition + this->mMario->mFrontVec * 100.0f, &g2, nullptr, 0);
    mAxisRotationDisabled6A0 = MR::isNearZero(g - g2, 0.001f);
    TVec3f hit;
    if (this->mMario->fn_803AA340(&this->mPosition, &hit)) {
        out->set(hit);
        g = hit;
        mAxisRotationDisabled6A0 = 0;
    }
    MR::normalizeOrZero(out);
    MR::normalizeOrZero(&g);
    if (out->dot(g) < 0.9f) {
        f32 a = this->mMario->getAirGravityVec().dot(*out);
        f32 b = this->mMario->getAirGravityVec().dot(g);
        if (b > a) {
            out->set(g);
        }
    }
    if (((this->mMario->mMovementStates.mLowWord >> (0x14)) & 1) && MR::isNearZero(*out, 0.001f)) {
        out->set(this->mMario->getAirGravityVec());
    }
}

bool MarioActor::fn_803CA440(TVec3f& a, TVec3f& b) {
    bool r = false;
    f32 ang = MR::diffAngleAbs(a, b);
    if (MR::isNearZero(a, 0.001f) || MR::isNearZero(b, 0.001f)) {
        ang = 0.0f;
    }
    if (this->mMario->isPlayerModeBee()) {
        if (ang >= 0.5235988f) {
            this->setBlendMtxTimer(8);
        }
    }
    if (ang < 1.2566371f) {
        return false;
    }
    if (this->mMario->isStatusActive(0x27)) {
        return false;
    }
    if (!this->isJumping()) {
        (this->mMario)->mFlags14.mLowWord |= 0x20;
    } else {
        if ((this->mMario)->_430 == 0xB) {
            (this->mMario)->_430 = 0;
        }
        if (_934 == 0) {
            if (((this->mMario->mMovementStates.mLowWord >> (0x14)) & 1)) {
                const char* fall = "落下";
                const char* jump = "ショートジャンプ";
                this->mMario->changeAnimation(jump, fall);
            } else {
                this->mMario->changeAnimation("ショートジャンプ", (const char*)nullptr);
            }
        }
        r = true;
    }
    (this->mMario)->mFlags14.mLowWord &= 0xFFDFFFFF;
    (this->mMario)->mMovementStates.mLowWord &= 0xFFEFFFFF;
    if (mSwimSurfaceState1010 != 0) {
        r = true;
    }
    this->_334 = 0xF;
    this->_legacy2AC = (this->mMario)->mHeadVec;
    if (!MR::isNearZero(b, 0.001f)) {
        if (ang >= 2.0943952f || !((this->mMario->mMovementStates.mHighWord >> (8)) & 1)) {
            this->mMario->cutGravityElementFromJumpVec(false);
            (this->mMario)->mJumpVec.scale(0.5f);
        } else {
            const TVec3f& gravity = *this->mMario->getGravityVec();
            Mario* player = this->mMario;
            f32 k = MR::vecKillElement(player->mJumpVec, gravity, &player->mJumpVec);
            this->mMario->mJumpVec.scale(0.75f);
            this->mMario->mJumpVec += (*this->mMario->getGravityVec() * k) * 0.75f;
        }
    }
    this->mMario->clear2DStick();
    (this->mMario)->mFlags14.mLowWord |= 0x10000000;
    this->_3AA = 0xF;
    if (!((this->mMario->mMovementStates.mHighWord >> (8)) & 1)) {
        (this->mMario)->_2E0 = -(this->mMario)->_2E0;
    }
    return r;
}

// NONMATCHING: vector register allocation, local stack slots, and verified literal addresses.
void MarioActor::fn_803CA710(TVec3f& a, TVec3f& b) {
    TVec3f axis, cr;
    if (MR::isNearZero(b, 0.001f) || MR::isNearZero(a, 0.001f)) {
        if (!MR::isNearZero(b, 0.001f)) {
            TVec3f n = -b;
            this->calcBaseFrontVec(n);
            return;
        }
        if (MR::isNearZero(b, 0.001f) && MR::isNearZero(a, 0.001f)) {
            this->calcBaseFrontVec((this->mMario)->_1FC);
        }
    } else {
        f32 cosv;
        if (MR::makeAxisAndCosignVecToVec(&axis, &cosv, a, b)) {
            const f32 angle = JMAAcosRadian(cosv);
            Mtx m;
            PSMTXRotAxisRad(m, (Vec*)&axis, angle);
            PSMTXMultVecSR(m, (Vec*)&this->_258, (Vec*)&this->_258);
            PSVECCrossProduct((Vec*)&this->_258, (Vec*)&b, (Vec*)&cr);
            if (MR::normalizeOrZero(&cr)) {
            } else {
                PSVECCrossProduct((Vec*)&b, (Vec*)&cr, (Vec*)&this->_258);
            }
            MR::normalize(&this->_258);
        }
    }
}

void MarioActor::fn_803CA860(TVec3f& v) {
    if (!MR::isNearZero(v, 0.001f)) {
        MR::normalize(&v);
    } else {
        Mario* m = this->mMario;
        if (!((m->mMovementStates.mLowWord >> (0x1E)) & 1)) {
            v.set(mAppliedGravityVec);
        } else {
            TVec3f n = -(m)->_368;
            v.set(n);
        }
    }
    if (this->mMario->isPlayerModeYoshi() && (mYoshiHolder->mYoshi)->mDashTimer > 0 && mDashWallHit1053 == 0) {
        WaterInfo info;
        MR::getWaterAreaObj(&info, this->mPosition);
        if (((this->mMario->mMovementStates.mLowWord >> (0x1E)) & 1) && !info.isInWater()) {
            TVec3f n = -(this->mMario)->_368;
            v.set(n);
        }
    }
    mDashWallHit1053 = 0;
}

void MarioActor::applyGravityVec(const TVec3f& gravity, bool resetGround) {
    mMario->setGravityVec(gravity);
    mAppliedGravityVec = gravity;
    if (resetGround) {
        TVec3f ground;
        JGeometry::negateInternal(&gravity.x, &ground.x);
        mMario->setGroundNorm(ground);
    }
    calcCenterPos();
}

// NONMATCHING: saved registers, stack slots and equivalent reuse of the 70.0f scalar.
void MarioActor::fn_803CAA00() {
    f32 len = 70.0f;
    Mario* m = this->mMario;
    TVec3f blend;
#ifdef GALAXY_NATIVE
    // The Wii helper also leaves its output untouched for opposite directions.
    // Retain the source direction in that case, rather than reading host stack
    // contents and copying a possible NaN into the binder offset.
    blend.set(m->mHeadVec);
#endif
    if (((m->mDrawStates.mWord >> (0x10)) & 1)) {
        MR::vecBlendSphere(&blend, (m)->mHeadVec, (m)->_380, 0.5f * this->_984);
    } else {
        MR::vecBlendSphere(&blend, (m)->mHeadVec, (m)->_368, this->_984);
    }
    TVec3f old(this->mVector60C);
    m = this->mMario;
    if (m->mMovementStates.mLowBits.mBit21) {
        len = 50.0f;
    }
    if (m->mMovementStates.mLowBits.mBit30) {
        this->mVector60C = blend * len;
    } else {
        len = 70.0f;
        if (((m->mFlags14.mHighWord >> (0x1C)) & 1) || _934 != 0) {
            TVec3f t = -mAppliedGravityVec * len;
            if (this->_334 != 0 || mMario->calcPolygonAngleD(mMario->mGroundPolygon) >= 59.0f) {
                MR::vecBlend(&this->mVector60C, this->mVector60C, t, 0.1f);
            } else {
                MR::vecBlend(&this->mVector60C, this->mVector60C, t, 0.5f);
            }
        } else {
            TVec3f t = -mAppliedGravityVec * len;
            MR::vecBlend(&this->mVector60C, this->mVector60C, t, 0.1f);
        }
    }
    u16 c = this->_3AA;
    if (c != 0) {
        this->_3AA = c - 1;
        f32 r = (f32)(u16)(c - 1) / 15.0f;
        this->mVector60C = this->mVector60C * (1.0f - r) + old * r;
        (this->mMario)->mFlags14.mLowWord |= 0x10000000;
    }
    if (this->_334 != 0) {
        Mario* m2 = this->mMario;
        if ((m2)->mVerticalSpeed > 160.0f) {
            TVec3f a((m2)->mHeadVec);
            a.scale(80.0f);
            TVec3f b(this->_legacy2AC);
            b.scale(80.0f);
            TVec3f c2(b);
            c2 -= a;
            f32 k = MR::vecKillElement(c2, mAppliedGravityVec, &c2);
            this->mMario->push2(mAppliedGravityVec * k);
            this->_legacy2AC = (this->mMario)->mHeadVec;
        }
        this->_334 = this->_334 - 1;
    }
}

void MarioActor::fn_803CAD10(const char* name, const char* b, const BckCtrlData* d) {
    const char* bck;
    if (this->_B91 == 0) {
        bck = b;
        if (static_cast<s32>(_468) == 0) {
            this->mMario->stopAnimationUpperForce();
        }
        char buf[0x80];
        LiveActor* actor = this;
        if (this->mMario->isPlayerModeYoshi()) {
            actor = mYoshiHolder->mYoshi;
            snprintf(buf, 0x80, "Yoshi%s", name);
        } else if (this->mMario->isPlayerModeTeresa()) {
            actor = mTeresaModel->mActor;
            snprintf(buf, 0x80, "%s", name);
        } else {
            snprintf(buf, 0x80, "%s", name);
        }
        if (MR::getResourceHolder(actor)->mMotionResTable->isExistRes(buf)) {
            if (d == nullptr) {
                MR::startBck(actor, buf, bck);
            } else {
                MR::startBck(actor, buf, nullptr);
                MR::reflectBckCtrlData(actor, *d);
            }
            if (this->mMario->isPlayerModeYoshi()) {
                this->mMarioAnim->fn_803DFBF0()->changeSimpleBck(buf);
                if (d == nullptr) {
                    BckCtrl* c = this->mModelManager->getResourceHolder()->mBckCtrl;
                    if (c != nullptr) {
                        if (bck == nullptr) {
                            bck = buf;
                        }
                        c->changeBckSetting(bck, this->mMarioAnim->fn_803DFBF0());
                    }
                } else {
                    BckCtrlFunction::reflectBckCtrlData(*d, this->mMarioAnim->fn_803DFBF0());
                }
            }
            this->fn_803BBFD0(name);
            this->mMarioAnim->closeCallback();
            this->mMarioAnim->entryCallback(name);
        }
    }
}

bool MarioActor::fn_803CAEE0() const {
    if (!_924) return false;
    if (_924->isType(0x65)) return true;
    if (MR::isEqualString(MR::getSensorHost(_924)->mName, "グライバード")) return true;
    bool isDandelion = MR::isEqualString(MR::getSensorHost(_924)->mName, "わたげ");
    return isDandelion;
}

bool MarioActor::fn_803CAF80() const {
    bool result;
    const HitSensor* sensor = getCarrySensor();
    if (!sensor) {
        result = false;
    } else {
        result = false;
        if (sensor->isType(0x13) || sensor->isType(0x12) || sensor->isType(0x26)) result = true;
    }
    return result;
}

bool MarioActor::fn_803CB010() const {
    if (!mMario->fn_803EEA90("ファイアスピン")) {
        return mMarioAnim->getUpperFrame() < 25.0f;
    }
    if (mMario->isAnimationRun("ファイアスピン") && mMarioAnim->getFrame() < 25.0f) {
        return true;
    }
    return false;
}

bool MarioActor::fn_803CB0B0() const {
    bool allowed = false;
    if (!mMario->isPlayerModeYoshi() || mMario->isStatusActive(0x27) || mYoshiHolder->mTongueAction->fn_8041FAA0()) allowed = true;
    return allowed;
}

void MarioActor::applyAutoBindPlayerMode(u32 policy) {
    if (policy == 2) setPlayerMode(0, true, true);
    if (policy == 4) setPlayerMode(0, false, true);
}

bool MarioActor::fn_803CB1A0(u16 mode) {
    if (mPlayerMode != mode) {
        return false;
    }
    if (mode == 1) {
        this->_3DC = mConst->getTable()->mMetalHoldTime;
        MR::startSubBGM("BGM_MUTEKI_A", false);
    } else if (mode == 9) {
        this->_3DC = mConst->getTable()->mTornadoHoldTime;
    } else if (mode == 3) {
        if (((this->mMario->mFlags14.mHighWord >> (0x16)) & 1)) {
            this->_3DC = 0;
        } else {
            this->_3DC = mConst->getTable()->mIceModeTime;
        }
        MR::startSubBGM("BGM_ICE_A", false);
    } else if (mode == 2) {
        this->_3DC = mConst->getTable()->mFireModeTime;
        MR::startPlayFireMorphBGM();
    } else if (mode == 7) {
        this->_3DC = 0xE10;
        MR::startStageBGM("BGM_FLYING_A", false);
    } else if (mode != 0) {
        this->playEffect("アイテム再ゲット");
    }
    return true;
}

void MarioActor::fn_803CB2E0(u16 newMode) {
    switch (mPlayerMode) {
    case 5:
        this->mMario->endRabbitMode();
        MR::startBtp(this, "ColorChange");
        MR::setBtpFrameAndStop(this, 0.0f);
        break;
    case 6:
        MR::deleteEffect(mTeresaModel->mActor, "Glow");
        if (newMode == 0) {
            MR::offCalcAnim(mTeresaModel->mActor);
        } else {
            (mTeresaModel->mActor)->kill();
        }
        this->_483 = 0;
        this->mMario->resetTeresaMode();
        break;
    case 2:
        MR::startBtp(this, "ColorChange");
        MR::setBtpFrameAndStop(this, 0.0f);
        MR::fn_8001C280();
        break;
    case 4:
        this->mMario->stopAnimation("ハチ飛行中", (const char*)nullptr);
        if (!this->mMario->isSwimming()) {
            MR::getGameSceneLayoutHolder()->changeLifeMeterModeGround();
        }
        _9E4->kill();
        _9E8->kill();
        break;
    case 3:
        this->stopEffect("アイス中");
        if (this->mMario->isStatusActive(0x1F)) {
            this->mMario->closeStatus(nullptr);
        }
        MR::stopSubBGM(30);
        break;
    case 1:
        this->stopEffect("無敵中");
        this->_A6E = 0;
        MR::stopSubBGM(30);
        break;
    case 7:
        MR::startCurrentStageBGM(false);
        break;
    case 10:
        if (this->mMario->isStatusActive(0x12)) {
            this->mMario->closeStatus(nullptr);
        }
        if (gIsLuigi) {
            mConst->changeTable(1);
        } else {
            mConst->changeTable(0);
        }
        MR::endStarPointerMode(this);
        if (!mYoshiHolder->mYoshi->fn_8040F0E0()) {
            this->mMario->fn_803B1180();
        }
        break;
    case 12:
        MR::resetSpinCloudBlocks();
        this->stopEffect("雲マリオ頭煙");
        break;
    }
    this->_3D6 = mPlayerMode;
}

void MarioActor::fn_803CB510(u16 mode, bool a) {
    mPlayerMode = mode;
    this->fn_803BA070();
    HitSensor* s = _4A4;
    if (s != nullptr) {
        s->receiveMessage(0x8F, this->getSensor("Body"));
        _4A4 = nullptr;
    }
    mSensor794 = nullptr;
    switch (mode) {
    case 9:
        this->_3DC = mConst->getTable()->mTornadoHoldTime;
        MR::startBtp(this, "TornadoElement");
        if (this->isJumping()) {
            this->mMario->changeAnimation("エレメントゲット", (const char*)nullptr);
        } else {
            this->mMario->changeAnimation("エレメントゲット接地中", (const char*)nullptr);
        }
        break;
    case 1:
        this->_3DC = mConst->getTable()->mMetalHoldTime;
        this->playEffect("無敵中");
        MR::startSubBGM("BGM_MUTEKI_A", false);
        MR::stopBrk(_9C0);
        MR::stopBrk(mHandModelL);
        MR::stopBrk(mHandModelR);
        MR::startBrk(_9C0, "InvincibleMario");
        MR::startBrk(mHandModelL, "InvincibleMarioHandL");
        MR::startBrk(mHandModelR, "InvincibleMarioHandR");
        MR::removeAllClingingKarikari();
        break;
    case 0:
        MR::startBtp(this, "ElementEnd");
        if (a) {
            switch (static_cast<s32>(_3D6)) {
            case 4:
            case 5:
            case 6:
                mMario->playSound("呪い解除", -1);
                break;
            case 10:
                break;
            case 2:
                if (MR::isSandyBossBGMPlaying()) {
                    mMario->playSound("変身解除ＢＧＭ維持", -1);
                } else {
                    mMario->playSound("変身解除", -1);
                }
                break;
            default:
                mMario->playSound("変身解除", -1);
                break;
            }
        }
        this->_3DC = 0;
        (this->mMario)->_76C = 0;
        if (this->mMario->isSwimming()) {
            if (static_cast<s32>(_468) == 0) {
                this->mMario->changeAnimation((const char*)nullptr, "水泳基本");
            }
        } else if (this->isJumping()) {
            this->mMario->changeAnimation((const char*)nullptr, "落下");
        } else {
            this->mMario->changeAnimation((const char*)nullptr, "基本");
        }
        break;
    case 5:
        this->mMario->startRabbitMode();
        this->rushDropThrowMemoSensor();
        break;
    case 6:
        (mTeresaModel->mActor)->appear();
        MR::onCalcAnim(mTeresaModel->mActor);
        this->runTeresaBaseAnimation();
        MR::startBrk(mTeresaModel->mActor, "Erase");
        MR::setBrkFrameAndStop(mTeresaModel->mActor, 0.0f);
        mTeresaModel->_4 = 0.0f;
        mTeresaModel->_8 = 0.0f;
        this->_483 = 1;
        this->rushDropThrowMemoSensor();
        this->mMario->startTeresaMode();
        (mTeresaModel->mActor)->calcAnim();
        MR::emitEffect(mTeresaModel->mActor, "Glow");
        MR::emitEffect(mTeresaModel->mActor, "Appear");
        break;
    case 2:
        MR::startPlayFireMorphBGM();
        this->_3DC = mConst->getTable()->mFireModeTime;
        break;
    case 3:
        this->playEffect("アイス中");
        MR::startSubBGM("BGM_ICE_A", false);
        if (((this->mMario->mFlags14.mHighWord >> (0x16)) & 1)) {
            this->_3DC = 0;
        } else {
            this->_3DC = mConst->getTable()->mIceModeTime;
        }
        break;
    case 4:
        MR::getGameSceneLayoutHolder()->changeLifeMeterModeBee();
        this->rushDropThrowMemoSensor();
        _9E4->appear();
        MR::showModel(_9E4);
        (this->mMario)->_402 = mConst->getTable()->mAirWalkTime;
        (this->mMario)->_42A = 0;
        _9E8->appear();
        this->_9F0 = 0;
        MR::startAction(_9E8, "Wait");
        MR::startBva(_9E8, "Wait");
        MR::stopBtk(_9E8);
        break;
    case 7:
        MR::startStageBGM("BGM_FLYING_A", false);
        this->_3DC = 3600;
        break;
    case 10: {
        mConst->changeTable(2);
        LiveActor* ya = mYoshiHolder->mYoshi;
        MR::setPlayerPos(ya->mPosition);
        MtxPtr bm = mYoshiHolder->mYoshi->getBaseMtx();
        TVec3f front(bm[0][2], bm[1][2], bm[2][2]);
        MR::setPlayerFrontVec(front, 1);
        mYoshiHolder->mTongueAction->fn_8041FCD0();
        mYoshiHolder->_10->mSensor = this->getSensor("Body");
        this->mMarioAnim->fn_803DF8E0();
        this->rushDropThrowMemoSensor();
        if (mYoshiHolder->mYoshi->mMountedBySpin != 0) {
            const char* lowerAnimation = "基本";
            this->mMario->changeAnimation("ヨッシースピン乗車", lowerAnimation);
        } else {
            const char* lowerAnimation = "基本";
            this->mMario->changeAnimation("ヨッシー乗車", lowerAnimation);
        }
        this->mMario->stopWalk();
        this->_3DC = 0;
        this->resetCondition();
        this->setPunchHitTimer(0);
        MR::startStarPointerModeCommandStream(this);
        break;
    }
    case 12:
        if (!MR::isOnGameEventFlagCloudMarioAtFirst()) {
            MR::startSystemSE("SE_SY_POWER_UP_2", -1, -1);
        } else {
            MR::startSystemSE("SE_SY_POWER_UP", -1, -1);
        }
        this->playEffect("雲マリオ頭煙");
        break;
    }
}

void MarioActor::fn_803CBA90(bool a, bool b) {
    if (this->mTransforming != 0) {
        MR::endDemo(this, "マリオ変身");
        this->mTransforming = 0;
    }
    if (a) {
        this->_3D8 = 0x1E;
        this->mMario->startPadVib("マリオ[変身解除]");
    } else {
        this->_3D8 = 2;
    }
    if (b) {
        this->playEffect("変身解除");
    }
}

// NONMATCHING: Typed flag stores use direct member addresses instead of a cached address.
void MarioActor::fn_803CBB30(bool b) {
    if (this->_3D8 == 0 || this->_3D6 == 0) {
        this->mPowerupCollected = 1;
    } else {
        if (mPlayerMode == 0xA) {
            this->_3D8 = 0x13;
            this->mMario->clearVelocity();
            (this->mMario)->mWalkSpeed = 0.0f;
        } else {
            this->_3D8 = 0x40;
            MR::stopAnimFrame(this);
            if (b) {
                this->playEffect("変身");
                this->mMario->startPadVib("マリオ[変身]");
            }
        }
        this->mPowerupCollected = 0;
    }
    if (!b) {
        this->_3D8 = 2;
        this->mPowerupCollected = 0;
    }
    this->_945 = 0;
    reinterpret_cast<s16&>(this->_946) = 0;
    this->mSpinRadius = 0.0f;
    this->mSpinHeight = 0.0f;
    for (int i = 0; i < 0x80; i++) {
        mSpinNoise[i] = 0.0f;
    }
    reinterpret_cast<s16&>(this->_948) = 0;
    reinterpret_cast<s16&>(this->_94C) = 0;
    this->updateFairyStar();
}

void MarioActor::fn_803CBC50() {
    if (mPlayerMode == 2) {
        MR::adjustSandyBossFireMorphBGM(this->_3DC, mConst->getTable()->mFireModeTime);
    }
    u16 t = this->_3DC;
    if (t != 0) {
        this->_3DC = t - 1;
        if (this->_3DC == 0) {
            this->setPlayerMode(0, true, true);
        }
    }
    if (MR::isInAreaObj("PlayerMorphLimitArea", this->mPosition)) {
        this->mWasInMorphLimitArea1065 = 1;
        return;
    }
    if (this->mWasInMorphLimitArea1065 != 0) {
        this->mWasInMorphLimitArea1065 = 0;
        this->setPlayerMode(0, true, true);
    }
}

// NONMATCHING: saved Mario receiver, stack frame size, and register allocation.
void MarioActor::touchSensor(HitSensor* pSensor) {
    if (pSensor->mType == ATYPE_WATER_PRESSURE_BULLET_BIND) {
        Mario* mario = mMario;
        const TVec3f collisionOffset = *MR::getSensorPos(pSensor) - _240;
        if (mario->fn_803B2880(_240, collisionOffset)) {
            return;
        }
        mMario->touchWater();
    }
    if (MR::isSensorEnemy(pSensor) && mMario->isStatusActive(MarioStatus_Bury)) {
        mMario->closeStatus(nullptr);
    }
}

// NONMATCHING: commutative float-add operands, string-pool register scheduling, and literal offsets.
void MarioActor::attackOrPushSensor(HitSensor* pSensor, f32 distance) {
    f32 radius = MR::getSensorRadius(pSensor);
    mMario->checkOnimasu(pSensor);
    bool pull = false;
    if (((getMovementStates().mLowWord >> 16) & 1U)) {
        if (distance < mSensorRadiusTornadoAttack + radius && tryTornadoAttack(pSensor)) {
            return;
        }

        if (distance < mSensorRadiusTornadoStorm + radius) {
            pSensor->receiveMessage(0x3B, getSensor("Body"));
            if (mMario->isSwimming()) {
                if (distance < mSensorRadiusSpinPull) {
                    pull = true;
                }
            } else if (distance < mSensorRadiusSpinPullOnGround) {
                pull = true;
            }
        }
    } else {
        if (_3E5 && _945 < 15) {
            if (distance < mSensorRadiusSpinStorm + radius) {
                if (mActionTable1080->selectAction("スピンアタック") < 4) {
                    pSensor->receiveMessage(ACTMES_SPIN_STORM_RANGE, getSensor("Body"));
                }

                if (mMario->isSwimming()) {
                    if (distance < mSensorRadiusSpinPull) {
                        pull = true;
                    }
                } else if (distance < mSensorRadiusSpinPullOnGround) {
                    pull = true;
                }
            }
        } else if (mMario->isAnimationRun("ファイアスピン") && distance < mSensorRadiusSpinStorm + radius) {
            pSensor->receiveMessage(ACTMES_SPIN_STORM_RANGE, getSensor("Body"));
        }
    }

    if (pull && _424 == nullptr) {
        tryTornadoPull(pSensor);
    }

    if (((getMovementStates().mLowWord >> 20) & 1U) && !((getMovementStates().mLowWord >> 30) & 1U) &&
        distance < mSensorRadiusHipDropAttack + radius + mMario->mJumpVec.dot(*mMario->getGravityVec())) {
        if (tryHipDropAttack(pSensor)) {
            return;
        }

        if (mConditionGroup->tryGetItem(pSensor)) {
            return;
        }

        mMario->mFlags14.mHighWord &= 0xFEFFFFFFU;
    }

    if (distance < mSensorRadiusAttack + radius) {
        if (_944 && tryPunchAirAfter(pSensor)) {
            return;
        }

        if (checkAndTryTrampleAttack(pSensor, distance, false)) {
            return;
        }

        bool touching = false;
        if (mPlayerMode == PlayerMode_Teresa) {
            const TVec3f* sensorPos = MR::getSensorPos(pSensor);
            TVec3f offset(_240);
            offset -= *sensorPos;
            f32 separation = offset.length();
            if (separation < MR::getSensorRadius(pSensor) + mConst->getTable()->mTeresaBodyRadius) {
                const char* ignoredEffect = "テレサ時は無視";
                if (mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, ignoredEffect)) {
                    return;
                }

                if (mConditionGroup->tryGetItem(pSensor)) {
                    return;
                }

                offset.setLength(MR::getSensorRadius(pSensor) + mConst->getTable()->mTeresaBodyRadius - separation);
                if (!MR::isSensorAutoRush(pSensor)) {
                    mMario->doTeresaReflection(offset, false);
                    MR::sendArbitraryMsg(0x4D, pSensor, getSensor("Body"));
                    touchSensor(pSensor);
                }

                if (MR::isSensorEnemy(pSensor)) {
                    TVec3f impulse(offset);
                    impulse.setLength(30.0f);
                    mMario->damage(impulse);
                }
            }
        } else {
            f32 width = 20.0f + mBodyRadius7A0;
            f32 height = mBodyHeight7A4;
            if (!mMario->isActiveTaskID(0x200)) {
                if (isJumping()) {
                    height += 10.0f;
                }

                if (mMario->mTargetWalkSpeedIndex >= 5) {
                    width += 15.0f;
                }
            } else {
                width -= 20.0f;
            }

            if (pSensor->mType == 0x61) {
                width = mBodyRadius7A0;
                height = mBodyHeight7A4;
            }
            if (fn_803CD4B0(pSensor, width, height)) {
                if (sendBodyAttack(pSensor)) {
                    return;
                }

                if (mConditionGroup->tryGetItem(pSensor)) {
                    return;
                }

                touchSensor(pSensor);
                bool pushed = MR::sendMsgPush(pSensor, getSensor("dummy"));
                touching = true;
                if (!pushed && !pSensor->isType(ATYPE_POWER_STAR_BIND)) {
                    mMario->mFlags14.mHighWord &= 0xFEFFFFFFU;
                }
            }
        }

        addRushSensor(pSensor, touching);
    } else if (((mMario->mFlags14.mLowWord >> 25) & 1U)) {
        f32 width = 20.0f + mBodyRadius7A0;
        f32 height = mBodyHeight7A4;
        if (isJumping()) {
            height += 10.0f;
        }

        if (mMario->mTargetWalkSpeedIndex >= 5) {
            width += 15.0f;
        }

        if (pSensor->mType == 0x61) {
            width = mBodyRadius7A0;
            height = mBodyHeight7A4;
        }
        f32 sensorRadius = MR::getSensorRadius(pSensor);
        TVec3f offset(*MR::getSensorPos(pSensor));
        offset -= _240;
        f32 depth = MR::vecKillElement(offset, mCameraBasisZ, &offset);
        if (depth < 250.0f && depth > -250.0f && cylinderPushCheck(offset, sensorRadius, width, height) && mConditionGroup->tryGetItem(pSensor)) {
            return;
        }
    }

    f32 catchRadius = mSensorRadiusClapCatch;
    if (mMario->isSwimming()) {
        catchRadius = mSensorRadiusClapCatchSwim;
    }

    if (distance < radius + catchRadius) {
        tryAddClapCoin(pSensor);
    }

    trySetLockOnTarget(pSensor);
    if (mPlayerMode == PlayerMode_Bee && distance < 100.0f + radius) {
        mMario->tryBeeStick(pSensor);
    }

    if (isUnderTarget(pSensor) && !pSensor->receiveMessage(6, getSensor("Body"))) {
        const char* homingEffect = "SHDホーミング許可";
        if (!mActionTable1080->testEffectDisabled(MR::getSensorHost(pSensor)->mName, homingEffect)) {
        TVec3f offset(*MR::getSensorPos(pSensor));
        offset -= _240;
        f32 angle = MR::diffAngleAbs(offset, *mMario->getGravityVec());
        if (mNearestUnderAngle79C > angle) {
            mNearestUnderAngle79C = angle;
            mNearestUnderSensor798 = pSensor;
        }
        }
    }
}

// NONMATCHING: verified equivalent pooled literal addresses.
void MarioActor::attackOrPushSensorInDamage(HitSensor* pReceiver, f32 radius) {
    f32 initialRadius = MR::getSensorRadius(pReceiver);

    if (!isEnableMoveMario()) {
        return;
    }

    if (radius < initialRadius + 120.0f) {
        bool touching = false;
        f32 sensorRadius;

        f32 width = 20.0f + mBodyRadius7A0;
        f32 height = mBodyHeight7A4;

        if (isJumping()) {
            height += 10.0f;
        }

        if (mMario->mTargetWalkSpeedIndex >= 5) {
            width += 15.0f;
        }

        if (fn_803CD4B0(pReceiver, width, height)) {
            if (mConditionGroup->tryGetItem(pReceiver)) {
                return;
            }

            touchSensor(pReceiver);
            MR::sendMsgPush(pReceiver, getSensor("dummy"));
            touching = true;
        }

        addRushSensor(pReceiver, touching);
        return;
    }

    if (!((mMario->mFlags14.mLowWord >> 25) & 1U)) {
        return;
    }

    f32 width = 20.0f + mBodyRadius7A0;
    f32 height = mBodyHeight7A4;

    if (isJumping()) {
        height += 10.0f;
    }

    if (mMario->mTargetWalkSpeedIndex >= 5) {
        width += 15.0f;
    }

    f32 sensorRadius = MR::getSensorRadius(pReceiver);
    TVec3f offset = *MR::getSensorPos(pReceiver) - _240;
    f32 depth = MR::vecKillElement(offset, mCameraBasisZ, &offset);
    if (depth < 250.0f && depth > -250.0f && cylinderPushCheck(offset, sensorRadius, width, height) && mConditionGroup->tryGetItem(pReceiver)) {
        return;
    }
}

// NONMATCHING: equivalent fadds operand order and verified equal scalar pools.
void MarioActor::attackOrPushSensorInRush(HitSensor* pSensor, f32 radius) {
    f32 sensorRadius = MR::getSensorRadius(pSensor);
    if (radius < mSensorRadiusAttack + sensorRadius) {
        f32 width = 20.0f + mBodyRadius7A0;
        f32 height = mBodyHeight7A4;
        if (fn_803CD4B0(pSensor, width, height)) {
            if (mConditionGroup->tryGetItem(pSensor)) {
                return;
            }

            pSensor->receiveMessage(ACTMES_RUSH_PLAYER_TOUCH, getSensor("Body"));
        }

        addRushSensor(pSensor, false);
    }

    if (radius < mSensorRadiusClapCatch + sensorRadius) {
        tryAddClapCoin(pSensor);
    }

    trySetLockOnTarget(pSensor);
}

void MarioActor::tryAddClapCoin(HitSensor* pSensor) {
    if (_7DC != 64 && pSensor->isType(ATYPE_STAR_PIECE)) {
        mTornadoSensors[_7DC] = pSensor;
        _7DC++;
    }
}

bool MarioActor::tryTornadoAttack(HitSensor* pSensor) {
    bool isMessageReceived = pSensor->receiveMessage(ACTMES_TORNADO_ATTACK, getSensor("Body"));
    if (isMessageReceived) {
        mMario->startPadVib(1);
    }

    return isMessageReceived;
}

bool MarioActor::isUnderTarget(HitSensor* pSensor) {
    TVec3f down;
    if (mBeeWallWalk != 0) {
        down = mAppliedGravityVec;
    } else {
        MR::calcGravityVectorOrZero(this, *MR::getSensorPos(pSensor), &down, mGravityInfo, 0);
    }

    TVec3f delta(mPosition - *MR::getSensorPos(pSensor));
    bool result = false;
    if (down.dot(delta) <= 0.0f && mMario->_424 == 0) {
        result = true;
    }

    return result;
}

bool MarioActor::tryHipDropAttack(HitSensor* pSensor) {
    if (isUnderTarget(pSensor) && cylinderHorizontalCheck(pSensor)) {
        if (pSensor->isType(ATYPE_PLAYER_AUTO_JUMP)) {
            return tryTrampleAttack(pSensor);
        }

        return pSensor->receiveMessage(ACTMES_PLAYER_HIP_DROP, getSensor("Body"));
    }

    return false;
}

// NONMATCHING: floating-point register allocation and trample-margin literal identity.
bool MarioActor::checkAndTryTrampleAttack(HitSensor* pSensor, f32 distance, bool force) {
    f32 radius = MR::getSensorRadius(pSensor);
    bool trample = ((getMovementStates().mLowWord >> 20) & 1U) != true;
    bool falling = false;
    if (isJumping() && !mMario->isRising()) {
        falling = true;
    }

    bool near = distance < (mSensorRadiusTrampleAttack + radius) + 1.5f * mMario->mJumpVec.dot(getGravityVector());
    if (force) {
        falling = isJumping();
    }

    if (trample && falling && near) {
        bool attacked = false;
        if (tryTrampleAttack(pSensor)) {
            attacked = true;
        }

        if (mPlayerMode == PlayerMode_Invincible && MR::sendArbitraryMsg(ACTMES_INVINCIBLE_ATTACK, pSensor, getSensor("Body"))) {
            mMario->startPadVib(2);
            printHitMark(pSensor);
        }

        return attacked;
    }

    return false;
}

bool MarioActor::tryTrampleAttack(HitSensor* pSensor) {
    if (_FCD) {
        return false;
    }

    if (mMario->isStatusActive(MarioStatus_Blown)) {
        return false;
    }

    bool isUnder = isUnderTarget(pSensor);
    bool isHorizontalCheck = cylinderHorizontalCheck(pSensor);
    if (isUnder && isHorizontalCheck) {
        _FCD = true;

        bool isReceiveMessage = pSensor->receiveMessage(ACTMES_PLAYER_TRAMPLE, getSensor("Body"));

        _FCD = false;

        if (isReceiveMessage) {
            doTrampleJump(pSensor);
            mConditionGroup->tryGetItem(pSensor);
        }

        return isReceiveMessage;
    }

    return false;
}

bool MarioActor::cylinderHorizontalCheck(HitSensor* pSensor) {
    f32 radius = pSensor->mRadius;
    TVec3f diff = pSensor->mPosition - (_240 + mMario->_350 + mMario->_35C + mVelocity);
    TVec3f vec;
    MR::vecKillElement(diff, getGravityVector(), &vec);

    if (radius + mBodyRadius7A0 - vec.length() > 0.0f) {
        return true;
    }

    MR::vecKillElement(diff, mBodyDownDirection, &vec);

    return radius + mBodyRadius7A0 - vec.length() > 0.0f;
}

void MarioActor::fn_803CCE60(HitSensor* a, s16 b) {
    this->mMarioAnim->fn_803DDD80(a);
    if (!this->mMario->isSwimming()) {
        if (((this->mMario->mMovementStates.mLowWord >> (0x1E)) & 1) && this->_B92 != -3) {
            reinterpret_cast<s16&>(this->_38C) = b;
        }
        mVelocity.zero();
        (this->mMario)->mWalkSpeed = 0.0f;
        if (((this->mMario->mMovementStates.mLowWord >> (0x1E)) & 1)) {
            this->mMario->stopJump();
            (this->mMario)->mVerticalSpeed = 0.0f;
        }
    }
    _424 = a;
    this->_480 = 1;
}

void MarioActor::fn_803CCF20(HitSensor* s) {
    HitSensor* old = _4A4;
    if (old != nullptr) {
        old->receiveMessage(0x8F, this->getSensor("Body"));
    }
    _4A4 = s;
    mSensor794 = nullptr;
}

void MarioActor::fn_803CCF90(HitSensor* sensor) {
    HitSensor* old = _4A4;
    if (old != nullptr) {
        old->receiveMessage(0x8F, this->getSensor("Body"));
    }
    _4A4 = nullptr;
    mSensor794 = sensor;
}

bool MarioActor::fn_803CD000() const {
    if (_934 != 0) {
        return false;
    }
    if (this->mHealth == 0) {
        return false;
    }
    if (this->mMario->isDamaging()) {
        return false;
    }
    if (!mActionTable1080->isActionOk("カメ持ち")) {
        return false;
    }
    if (_468 != 0) {
        return false;
    }
    return true;
}

bool MarioActor::fn_803CD0A0() {
    if (_934 != 0) {
        return false;
    }
    if (this->mHealth == 0) {
        return false;
    }
    if (this->_3AC != 0) {
        return false;
    }
    if (this->mMario->isDamaging()) {
        return false;
    }
    if (!mActionTable1080->isActionOk("カメ持ち")) {
        return false;
    }
    if (static_cast<s32>(_468) != 0) {
        return false;
    }
    if (((this->mMario->mMovementStates.mLowWord >> (0x14)) & 1) && this->fn_803CD180()) {
        return false;
    }
    return true;
}

bool MarioActor::fn_803CD180() {
    bool r = false;
    if (!this->mMario->isSwimming() && !((this->mMario->mMovementStates.mLowWord >> (0x1E)) & 1) && _424 == nullptr) {
        r = true;
    }
    return r;
}

bool MarioActor::fn_803CD1F0(HitSensor* s) {
    if (this->fn_803CD250(s) > 100.0f) {
        return false;
    }
    return MR::isNearZero(MR::getSensorHost(s)->mVelocity, 0.001f);
}

f32 MarioActor::fn_803CD250(HitSensor* s) const {
    TVec3f b;
    TVec3f a((this->mMario)->mShadowPos);
    a -= (s)->mPosition;
    return MR::vecKillElement(a, *this->mMario->getGravityVec(), &b);
}

bool MarioActor::fn_803CD2C0() {
    if (this->_3AC != 0) {
        return false;
    }
    if (this->mMario->isDamaging()) {
        return false;
    }
    if (!mActionTable1080->isActionOk("カメ持ち")) {
        return false;
    }
    if (static_cast<s32>(_468) != 0) {
        return false;
    }
    if (((this->mMario->mMovementStates.mLowWord >> (0x14)) & 1) && this->fn_803CD180()) {
        return false;
    }
    return true;
}

bool MarioActor::fn_803CD380() {
    if (this->mHealth == 0) {
        return false;
    }
    if (this->_3D8 != 0) {
        return false;
    }
    return (this->mMario)->mMorphResetTimer == 0;
}

bool MarioActor::fn_803CD3C0() const {
    if (_934 != 0) {
        return false;
    }
    if (this->mHealth == 0) {
        return false;
    }
    if (this->_3AC != 0) {
        return false;
    }
    if (this->mMario->isDamaging()) {
        return false;
    }
    return static_cast<s32>(_468) == 0;
}

void MarioActor::fn_803CD450(HitSensor* s) {
    if (this->fn_803CD1F0(s) && this->fn_803CD180()) {
        this->doTrampleJump(s);
    }
}

bool MarioActor::fn_803CD4B0(HitSensor* s, f32 f0, f32 f1) {
    f32 r = (s)->mRadius;
    TVec3f d((s)->mPosition);
    d -= this->_240;
    LiveActor* h = s->mHost;
    if (h != nullptr && MR::isEqualString(h->mName, "ヨッシー")) {
        return this->fn_803CD8A0(&d, r, 40.0f, 10.0f);
    }
    return this->cylinderPushCheck(d, r, f0, f1);
}

// NONMATCHING: equivalent flag test instruction and verified pooled literal addresses.
inline bool MarioActor::cylinderPushCheck(HitSensor* pSensor, f32 radius, f32 width, f32 height) {
    TVec3f offset(pSensor->mPosition - _2A0);
    return cylinderPushCheck(offset, radius, width, height);
}

// NONMATCHING: equivalent flag test instruction and verified pooled literal addresses.
bool MarioActor::cylinderPushCheck(const TVec3f& rOffset, f32 radius, f32 width, f32 height) {
    TVec3f offset(rOffset);
    if (mMario->mFlags14.mLowBits.mBit25) {
        f32 depth = MR::vecKillElement(offset, mCameraBasisZ, &offset);
        if (depth >= 250.0f || depth <= -250.0f) return false;
    }
    TVec3f radial;
    f32 axial = MR::vecKillElement(offset, mBodyDownDirection, &radial);
    if (axial > -radius) {
        f32 radialPenetration = radius + width - radial.length();
        f32 axialPenetration = radius + height - axial;
        if (radialPenetration > 0.0f && axialPenetration > 0.0f) {
            if (radialPenetration > 0.0f) {
                getDummySensor(this)->mPosition.set(_240 + mBodyDownDirection * axial);
                getSensor("dummy")->mRadius = width;
            } else {
                HitSensor* dummy = getSensor("dummy");
                dummy->mPosition.set(_240 + radial);
                getSensor("dummy")->mRadius = height;
            }

            return true;
        }
    }

    if (((mMario->mMovementStates.mLowWord >> 21) & 1U) && ((mMario->getMovementStates().mLowWord >> 30) & 1U)) {
        return false;
    }

    axial = MR::vecKillElement(offset, mBodyUpDirection, &radial);
    if (axial < -radius) {
        return false;
    }

    f32 radialPenetration = radius + width - radial.length();
    f32 axialPenetration = radius + height - axial;
    if (radialPenetration > 0.0f && axialPenetration > 0.0f) {
        if (radialPenetration > 0.0f) {
            getDummySensor(this)->mPosition.set(_240 + mBodyUpDirection * axial);
            getSensor("dummy")->mRadius = width;
        } else {
            HitSensor* dummy = getSensor("dummy");
            dummy->mPosition.set(_240 + radial);
            getSensor("dummy")->mRadius = height;
        }

        return true;
    }

    return false;
}

bool MarioActor::fn_803CD8A0(TVec3f* v, f32 f0, f32 f1, f32 f2) {
    TVec3f p(*v);
    if (((this->mMario->mFlags14.mLowWord >> (0x19)) & 1)) {
        f32 k = MR::vecKillElement(p, mCameraBasisZ, &p);
        if (k >= 250.0f || k <= -250.0f) {
            return false;
        }
    }
    TVec3f perp;
    f32 d = MR::vecKillElement(p, mBodyDownDirection, &perp);
    if (d > -f0) {
        if (d < f2 && PSVECMag((Vec*)&perp) < (f1 + f0)) {
            HitSensor* hs = this->getSensor("dummy");
            (hs)->mPosition.set(this->_240 + mBodyDownDirection * d);
            (this->getSensor("dummy"))->mRadius = f1;
            return true;
        }
        if (d >= f2) {
            if ((p - mBodyDownDirection * f2).length() < (f0 + f1)) {
                HitSensor* hs = this->getSensor("dummy");
                (hs)->mPosition.set(this->_240 + mBodyDownDirection * f2);
                (this->getSensor("dummy"))->mRadius = f1;
                return true;
            }
        }
    }
    Mario* m = this->mMario;
    if (((m->getMovementStates().mLowWord >> 21) & 1U) && ((m->mMovementStates.mLowWord >> 30) & 1U)) {
        return false;
    }
    f32 d2 = MR::vecKillElement(p, mBodyUpDirection, &perp);
    if (d2 > -f0) {
        if (d2 < f2 && PSVECMag((Vec*)&perp) < (f1 + f0)) {
            HitSensor* hs = this->getSensor("dummy");
            (hs)->mPosition.set(this->_240 + mBodyUpDirection * d2);
            (this->getSensor("dummy"))->mRadius = f1;
            return true;
        }
        if (d2 >= f2) {
            if ((p - mBodyUpDirection * f2).length() < (f0 + f1)) {
                HitSensor* hs = this->getSensor("dummy");
                (hs)->mPosition.set(this->_240 + mBodyUpDirection * f2);
                (this->getSensor("dummy"))->mRadius = f1;
                return true;
            }
        }
    }
    return false;
}

void MarioActor::attackOrPushPolygons() {
    HitSensor* bodySensor = getSensor("Body");

    _FCC = true;

    if (((mMario->getMovementStates().mLowWord >> 30) & 1U) && _390 == 0) {
        HitSensor* groundSensor = mMario->mGroundPolygon->mSensor;

        if (groundSensor != nullptr && !MR::isDead(MR::getSensorHost(groundSensor))) {
            groundSensor->receiveMessage(ACTMES_FLOOR_TOUCH, bodySensor);
        }
    }

    if (((mMario->getMovementStates().mLowWord >> 23) & 1U)) {
        sendWallTouch(173, mMario->mFrontWallTriangle->mSensor, bodySensor);
    }

    if (((mMario->getMovementStates().mLowWord >> 6) & 1U)) {
        sendWallTouch(175, mMario->mBackWallTriangle->mSensor, bodySensor);
    }

    if (((mMario->getMovementStates().mLowWord >> 5) & 1U)) {
        sendWallTouch(174, mMario->mSideWallTriangle->mSensor, bodySensor);
    }

    if (((mMario->getMovementStates().mLowWord & Mario::MovementStates::Jumping) >> 31) && !((mMario->getMovementStates().mLowWord >> 30) & 1U) && ((mMario->getMovementStates().mLowWord >> 20) & 1U)) {
        _3E8 = true;
    } else {
        if (_3E8) {
            if (((getMovementStates().mLowWord >> 30) & 1U)) {
                if (mMario->mGroundPolygon->isValid()) {
                    HitSensor* groundSensor = mMario->mGroundPolygon->mSensor;

                    if (groundSensor != nullptr && !MR::isDead(MR::getSensorHost(groundSensor))) {
                        HitSensor* hitSensor = mMario->mGroundPolygon->mSensor;
                        if (!hitSensor->receiveMessage(ACTMES_PLAYER_HIP_DROP_FLOOR, getSensor("Body"))) {
                            _3E8 = false;
                        }
                    }
                }
            } else {
                _3E8 = false;
            }
        }

        if (!((mMario->getMovementStates().mLowWord & Mario::MovementStates::Jumping) >> 31)) {
            _3E8 = false;
        }
    }

    _FCC = false;
}

void MarioActor::sendWallTouch(u32 msg, HitSensor* a, HitSensor* b) {
    if (a != nullptr) {
        if (!MR::isDead(MR::getSensorHost(a))) {
            a->receiveMessage(msg, b);
            if (((this->mMario->mMovementStates.mLowWord >> (0x10)) & 1)) {
                a->receiveMessage(0x3A, b);
            }
        }
    }
}

void MarioActor::updateActionTrigger() {
    mActionTriggered = false;
    mJumpButtonHeld = false;

    if (checkButtonType(3, false) || _F00 != 0) {
        if (mActionButtonHeld) {
            return;
        }

        mActionButtonHeld = true;
        mActionTriggered = true;
    } else {
        mActionButtonHeld = false;
    }

    if (mMario->checkLvlA()) {
        mJumpButtonHeld = true;
    }
}

void MarioActor::updateControllerSwing() {
    if (checkButtonType(3, false) || checkButtonType(4, false)) {
        _F1C = 10;
        _EF8 = 0;
    }

    const u8 previousSwing = mFlag100E;
    if (mMario->isSwimming()) {
        mFlag100E = MR::isCorePadSwingTrigger(0);
    } else {
        mFlag100E = MR::isCorePadSwing(0);
    }

    if (_F1C != 0) {
        _F1C--;
        _F00 = false;
    } else if (mFlag100E && previousSwing == 0) {
        _F00 = true;
    } else {
        _F00 = false;
    }

    if (mFlag100E && _946 > 10 && _F1C < 8) {
        _F1C = 8;
    }
}

void MarioActor::updateControllerSwingLeft() {
    if (checkButtonType(7, false) || checkButtonType(8, false)) {
        _F1E = 10;
        _F04 = 0;
    }

    const u8 previousSwing = mFlag100F;
    mFlag100F = MR::isSubPadSwing(0);

    if (_F1E != 0) {
        _F1E--;
        _F0C = false;
    } else if (mFlag100F && previousSwing == 0) {
        _F0C = true;
    } else {
        _F0C = false;
    }

    if (mFlag100F && _946 > 10 && _F1E < 8) {
        _F1E = 8;
    }
}

void MarioActor::init2D() {
    MR::getGameSceneLayoutHolder()->initLifeCount(mMaxHealth);

    _1B8 = new CollectCounter("マリオ連続踏み", 0.0f);

    _1B8->initWithoutIter();
}

void MarioActor::update2D() {
    GameSceneLayoutHolder* layoutHolder = MR::getGameSceneLayoutHolder();
    layoutHolder->setLifeCount(mHealth);

    if (mMario->getPlayerMode() == 4) {
        layoutHolder->setBeePowerRatio((f32)mMario->_402 / mConst->getTable()->mAirWalkTime);
    }

    if (mMario->isSwimming()) {
        layoutHolder->setOxygenRatio((f32)mMario->mSwim->mOxygen / mConst->getTable()->mOxygenMax);
    }

    if (mMario->isStatusActive(MarioStatus_YoshiBalloon)) {
        layoutHolder->setOxygenRatio(mMario->mYoshiBalloon->mAir);
    }

    if (_989 == 0) {
        _1B8->kill();
    }
}

void MarioActor::initParts() {
    mNullAnimation = new MarioNullBck("NULLアニメ");
    mNullAnimation->initWithoutIter();

    mSearchLight = 0;
    mSearchLightThrowPos = nullptr;

    initSearchLight();
    initThrowing();

    if (MR::isCreatePlayerResourceFreezeIce()) {
        _9C4 = new MarioParts(this, "氷結モデル", "MarioFreezeIce", true, nullptr, nullptr);
        _9C4->initWithoutIter();
        _9C4->makeActorDead();
        _9C4->initFixedPosition(TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f), nullptr);
    }

    mJetTurtleShadowE94 = new JetTurtleShadow("カメシャドウモデル");
    mJetTurtleShadowE94->initWithoutIter();

    _994 = new MarioParts(this, "スピンチコ", "SpinTico", false, getBaseMtx(), nullptr);
    _994->initWithoutIter();
    _994->kill();
#ifdef GALAXY_NATIVE
    petariCappyInit(this);
#endif
}

void MarioActor::updateBeeWingAnimation() {
    if (mPlayerMode != 4) {
        mMarioAnim->getJointCtrl("HandR")->setLocalScale(1.0f);
        mMarioAnim->getJointCtrl("HandL")->setLocalScale(1.0f);

        return;
    }

    mMarioAnim->getJointCtrl("HandR")->setLocalScale(0.9f);
    mMarioAnim->getJointCtrl("HandL")->setLocalScale(0.9f);

    if (mMario->checkLvlA() && mMario->_402 && ((mMario->mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        if (_9F0 != 1) {
            MR::startAction(_9E8, "Fly");
            MR::startBva(_9E8, "Fly");
            MR::startBtk(_9E8, "Fly");
        }

        _9F0 = 1;
        return;
    }

    s32 val;
    if (((mMario->mMovementStates.mLowWord >> 30) & 1U) || mMario->isStatusActive(MarioStatus_Stick)) {
        val = 0;
    } else {
        val = mMario->_402 != 0 ? 2 : 3;
    }

    if (_9F0 == val) {
        return;
    }

    switch (val) {
    case 0:
        MR::startAction(_9E8, "Wait");
        MR::startBva(_9E8, "Wait");
        break;

    case 2:
        MR::startAction(_9E8, "FlyWait");
        MR::startBva(_9E8, "FlyWait");
        break;

    case 3:
        MR::startAction(_9E8, "FlyFall");
        MR::startBva(_9E8, "FlyFall");
        break;
    }

    MR::stopBtk(_9E8);
    _9F0 = val;
}

void MarioActor::updateTornado() {
    if (mTornadoMario == nullptr) {
        return;
    }

    if (((mMario->getMovementStates().mLowWord >> 16) & 1U) && mMario->_544 > 1) {
        mTornadoMario->show();
    } else if ((!(mMario->isAnimationRun("空中ひねり") || mMario->isAnimationRun("雲スピン空中")) || mMario->_430 != 8) && !mMario->isStatusActive(MarioStatus_Magic)) {
        if (((mMario->getDrawStates().mWord >> 23) & 1U) || _990 != 0) {
            mTornadoMario->hideForce();
        } else {
            mTornadoMario->hide();
        }

        _990 = 0;
    }

    mTornadoMario->setTrHeight(mPosition, mMario->mFrontVec, mMario->_54C, mAppliedGravityVec);
}

void MarioActor::updateTakingPosition() {
    HitSensor* sensor = _424;
    if (!sensor) sensor = const_cast<HitSensor*>(getCarrySensor());
    fn_803CE8A0(sensor);
    switch (_B92) {
    case -3:
        fn_803CE9D0(sensor);
        return;
    case -2: {
        f32 frame;
        if (mMarioAnim->isAnimationTerminate(nullptr)) frame = 0.0f;
        else frame = mMarioAnim->getFrame();
        fn_803CEA80(sensor, frame);
        return;
    }
    case -1: {
        f32 frame;
        if (mMarioAnim->isUpperAnimationTerminated(nullptr)) frame = 0.0f;
        else frame = mMarioAnim->getUpperFrame();
        fn_803CEA80(sensor, frame);
        return;
    }
    default:
        fn_803CEBC0();
        return;
    }
}

void MarioActor::fn_803CE8A0(HitSensor* s) {
    if (this->_480 != 0) {
        if (s == nullptr) {
            this->_480 = 0;
            return;
        }
        bool drill = false;
        if (MR::getSensorHost(s) != nullptr && MR::isEqualString(MR::getSensorHost(s)->mName, "アイテムドリル")) {
            drill = true;
        }
        if (drill) {
            if (this->mMario->isAnimationTerminate(nullptr)) {
                this->_480 = 0;
                MR::explainDrillIfAtFirst();
            }
        } else {
            u32 t = (s)->mType;
            if (t - 0x12 > 1) {
                if (t == 0x1D && this->mMario->isAnimationTerminate("カブ抜き")) {
                    this->mMario->stopAnimation((const char*)nullptr, (const char*)nullptr);
                    this->_480 = 0;
                    this->mMario->changeAnimationUpper("カブウエイト", (const char*)nullptr);
                }
            } else if (this->mMario->isAnimationTerminate(nullptr)) {
                this->_480 = 0;
            }
        }
    }
}

void MarioActor::fn_803CE9D0(HitSensor* s) {
    if (this->mRushAnimationDelay1054 == 0) {
        _494->calc();
        TVec3f t, r;
        _494->mMtx.getTrans(t);
        _494->copyRotate(&r);
        if (s != nullptr) {
            if (this->fn_803CEE50()) {
                mMarioAnim->updateTakingAnimation(s);
            }
            MR::getSensorHost(s)->mPosition = t;
            MR::getSensorHost(s)->mRotation = r;
        }
    }
}

void MarioActor::fn_803CEA80(HitSensor* s, f32 t) {
    if (this->mRushAnimationDelay1054 == 0) {
        Mtx tmp;
        PSMTXConcat(this->getBaseMtx(), mMainJointMatrix4A8.mMtx, tmp);
        PSMTXCopy(tmp, MR::getJ3DModel(mNullAnimation)->getBaseTRMtx());
        TVec3f pos, rot;
        if (mNullAnimation->getFramePos(t, &pos, &rot)) {
            this->_B92 = -3;
            mNullAnimation->kill();
            if (s == nullptr) {
                this->_480 = 0;
                this->_B92 = 0;
                mNullAnimation->kill();
                return;
            }
            MR::sendArbitraryMsg(50, s, this->getSensor("Body"));
        }
        if (s != nullptr) {
            MR::getSensorHost(s)->mPosition = pos;
            MR::getSensorHost(s)->mRotation = rot;
        }
    }
}

// NONMATCHING: vector temporary stack slots and literal identities.
void MarioActor::fn_803CEBC0() {
    if (static_cast<s32>(_468) != 0) {
        HitSensor* cs = (HitSensor*)this->getCarrySensor();
        bool drill = false;
        if (MR::getSensorHost(cs) != nullptr && MR::isEqualString(MR::getSensorHost(cs)->mName, "アイテムドリル")) {
            drill = true;
        }
        if (drill) {
            if (_934 == 0 && this->mRushAnimationDelay1054 == 0) {
                _494->calc();
                TVec3f t;
                _494->mMtx.getTrans(t);
                LiveActor* host = MR::getSensorHost(cs);
                host->mPosition = t + mVelocity;
                _494->copyRotate(&MR::getSensorHost(cs)->mRotation);
            }
        } else {
            if (cs->isType(0x48) || cs->isType(0x12) || cs->isType(0x13) || cs->isType(0x1D)) {
                _494->calc();
                TVec3f t;
                _494->mMtx.getTrans(t);
                LiveActor* host = MR::getSensorHost(cs);
                host->mPosition = t + mVelocity;
                _494->copyRotate(&MR::getSensorHost(cs)->mRotation);
                return;
            }
            TVec3f lp;
            mNullAnimation->getLastPos(&lp);
            PSMTXMultVec(mMainJointMatrix4A8.mMtx, (Vec*)&lp, (Vec*)&lp);
            PSMTXMultVec(this->getBaseMtx(), (Vec*)&lp, (Vec*)&lp);
            f32 sp = (this->mMario)->mWalkSpeed;
            f32 amp = 10.0f * (sp * sp);
            amp *= JMASinRadian(mCarryWobblePhase778);
            mCarryWobblePhase778 += MR::getRandom(0.1f, 1.0f);
            lp.y += amp;
            MR::getSensorHost(cs)->mPosition = lp;
            LiveActor* host = MR::getSensorHost(cs);
            host->mRotation = mThrowMemoRotations[0] + mRotation;
        }
    }
}

bool MarioActor::fn_803CEE50() {
    bool r = true;
    bool a = true;
    if (!this->mMario->isAnimationTerminate(nullptr) && this->mMario->isAnimationRun(nullptr)) {
        a = false;
    }
    if (!a && !this->mMario->fn_8038CCF0()) {
        r = false;
    }
    return r;
}

s32 MarioActor::fn_803CEEE0(HitSensor* s) {
    bool r = false;
    if (MR::getSensorHost(s) != nullptr && MR::isEqualString(MR::getSensorHost(s)->mName, "アイテムドリル")) {
        r = true;
    }
    return r;
}

void MarioActor::fn_803CEF50(LiveActor* a) {
    mSensorSlots108C->fn_803DAB10(a);
}

// NONMATCHING: verified equivalent scalar literal addresses.
void MarioActor::updateThrowVector() {
    HitSensor* sensor = _46C;
    if (sensor && !MR::isExistInAttributeGroupSearchTurtle(MR::getSensorHost(sensor))) sensor = nullptr;
    if (((getDrawStates().mWord >> 26) & 1U)) return;
    if (sensor && _468) {
        if (_470 && _46C != _470) _470 = nullptr;
        if (!_47C) mThrowProbePosition76C = _240;
        ++_47C;
        TVec3f direction = _46C->mPosition - mThrowProbePosition76C;
        f32 radius = _46C->mRadius;
        if (direction.length() <= 40.0f + radius) {
            mThrowProbePosition76C = _46C->mPosition;
            _470 = _46C;
            if (MR::isSensorEnemy(_46C)) {
                _47C = 60;
                return;
            }
            _47C = 2;
            return;
        }
        MR::normalize(&direction);
        mThrowProbePosition76C += direction * 40.0f;
        if (mMario->fn_803B2AF0(mThrowProbePosition76C, nullptr)) {
            _47C = 0;
            _470 = nullptr;
        }
    } else {
        if (_47C) --_47C;
        if (!_47C) _470 = nullptr;
    }
}

void MarioActor::updateBaseMtxTeresa(MtxPtr mtx) {
    TVec3f tmp;
    f32 k = MR::vecKillElement(mVelocity, *this->mMario->getGravityVec(), &tmp);
    f32 v = MR::clamp(k / 10.0f, -1.0f, 1.0f);
    if (!MR::isNearZero(this->mMario->getWorldPadDir(), 0.001f)) {
        const TVec3f* g = this->mMario->getGravityVec();
        Mario* m = this->mMario;
        if (MR::diffAngleAbsHorizontal(m->getWorldPadDir(), m->mFrontVec, *g) > 0.3926991f) {
            v = 0.0f;
        }
    }
    f32 nextTilt;
    f32 oldTilt = mTeresaModel->_8;
    nextTilt = 0.02f * v;
    f32 previousTilt = 0.98f * oldTilt;
    mTeresaModel->_8 = nextTilt + previousTilt;
    f32 var = mConst->getTable()->mTeresaAngleDown;
    if (mTeresaModel->_8 > 0.0f) {
        var = mConst->getTable()->mTeresaAngleUp;
    }
    PSMTXConcat(mtx, MR::tmpMtxRotXRad(var * mTeresaModel->_8), mtx);
}

void MarioActor::initFireBall() {
    for (u32 idx = 0; idx < ARRAY_SIZE(mFireBalls); idx++) {
        mFireBalls[idx] = new FireMarioBall("マリオ炎球");
        mFireBalls[idx]->initWithoutIter();
    }
}

void MarioActor::updateFireSpin() {
    if (!this->mMario->isAnimationRun("ファイア投げ") && !this->mMario->isAnimationRun("ファイアスピン空中") &&
        (!this->mMario->isAnimationRun("ファイアスピン") || !this->fn_803CB010())) {
        if (((this->mMario->mMovementStates.mLowWord >> (0x17)) & 1)) {
            const Triangle* tri = this->mMario->getWallPolygon();
            LiveActor* h = MR::getSensorHost(tri->getSensor());
            if (!MR::isEqualString(h->mName, "木箱") && !MR::isEqualString(h->mName, "タイムアタック用木箱")) {
                HitSensor* ws = this->mMario->getWallPolygon()->getSensor();
                ws->receiveMessage(0xB, this->getSensor("Body"));
                this->mMario->changeAnimation("ファイアスピン", (const char*)nullptr);
                return;
            }
        }
        this->mFireBallDelay = 7;
        this->mMario->playSound("声投げ", -1);
        if (!this->isJumping()) {
            this->mMario->stopAnimation("ファイアスピン", (const char*)nullptr);
            this->mMario->stopAnimationUpper("ファイアスピン", (const char*)nullptr);
            this->mMario->changeAnimation("ファイアスピン", (const char*)nullptr);
            (this->mMario)->mWalkSpeed = 0.0f;
            return;
        }
        this->mMario->setFrontVecKeepUp((this->mMario)->mWorldPadDir);
        Mario* m = this->mMario;
        u16 c = (m)->_42C;
        if (c >= 3) {
            m->changeAnimation("ファイア投げ", (const char*)nullptr);
            return;
        }
        if (c == 0) {
            m->changeAnimation("ファイアスピン空中", (const char*)nullptr);
        } else {
            m->changeAnimation("ファイア投げ", (const char*)nullptr);
        }
        this->jumpHop();
        (this->mMario)->_42C += 1;
        f32 k = this->mMario->cutGravityElementFromJumpVec(true);
        (this->mMario)->mJumpVec.scale(0.5f);
        TVec3f g(mAppliedGravityVec);
        g.scale(k);
        (this->mMario)->mJumpVec += g;
    }
}

void MarioActor::killAllFireBall() {
    int i = 0;
    do {
        if (!MR::isDead(mFireBalls[i])) {
            mFireBalls[i]->kill();
        }
        i++;
    } while (i < 5);
}

// NONMATCHING: getRealMtx argument setup instruction order.
void MarioActor::shootFireBall() {
    if (mMario->isAnimationRun("ファイアスピン") && mMarioAnim->getFrame() < 20.0f) mMario->mWalkSpeed = 0.0f;
    if (mFireBallDelay != 0) {
        --mFireBallDelay;
        if (mFireBallDelay <= 0) {
            u32 index;
            for (index = 0; index < 5; ++index) if (MR::isDead(mFireBalls[index])) break;
            if (index != 5) {
                FireMarioBall* ball = mFireBalls[index];
                TVec3f direction;
                direction = mMario->mFrontVec;
                direction.add(mMario->mHeadVec);
                MR::normalize(&direction);
                Mtx hand;
                const char* handJoint = "HandR";
                getRealMtx(hand, handJoint);
                TVec3f position;
                MR::extractMtxTrans(hand, &position);
                TVec3f offset(mMario->mFrontVec);
                offset.scale(30.0f);
                position.add(offset);
                ball->appearAndThrow(position, direction);
            }
        }
    }
}

void MarioActor::updateFairyStar() {
    if (_482)
        return;
    if (!_EEB)
        return;
    if (!isEnableNerveChange())
        return;
    bool enabled = true;
    if (mActionTable1080->selectAction("スピン回復エフェクト") != 1)
        enabled = false;
    if (_948 && enabled) {
        if (MR::isDead(_994)) {
            _994->appear();
            MR::setRotation(_994, 0.0f);
            MR::startAction(_994, "SpinTimer");
            playSound("スピン許可", -1);
        }
        TVec3f position;
        MR::copyJointPos(_994, "Center", &position);
        Color8 color(255, 225, 225, 255);
        f32 power = 0.001f;
        MR::requestPointLightPlayer(_994, position, color, power, 0, -1.0f);
        return;
    }
    if (!MR::isDead(_994))
        _994->kill();
}

bool MarioActor::isEnableSpinPunch() {
    if (((mMario->mMovementStates.mHighWord >> 28) & 1U)) {
        return false;
    }

    if (mMario->getCurrentStatus() == MarioStatus_Slider) {
        return false;
    }

    if (mMario->getCurrentStatus() == MarioStatus_Swim) {
        return false;
    }

    if (mMario->getCurrentStatus() == MarioStatus_Flip) {
        return mMario->mFlip->_14 >= 2;  // FIXME
    }

    if (mMario->getCurrentStatus() == MarioStatus_Stun) {
        bool out = false;
        if (mMario->mStun->_12 || mMario->mStun->_14 < 15) {
            out = true;
        }

        return out;
    }

    if (((mMario->mMovementStates.mLowWord >> 21) & 1U) && mMario->calcDistToCeil(false) < 160.0f) {
        return false;
    }

    if (mMario->isPlayerModeYoshi() && !YoshiParam::fn_8041BD50()) return false;
    return true;
}

// NONMATCHING: Mario receiver reloads and verified pooled literal addresses.
bool MarioActor::trySpinPunch() {
    bool out = true;
    if (((mMario->mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        mMario->changeAnimation("空パンチ");
        setPunchHitTimer(15);
        mMario->mWalkSpeed = 0.0f;
    } else if (isEnableSpinPunch()) {
        if (mMario->isStatusActive(MarioStatus_Bury)) {
            playSound("スケキヨ終了スピン", -1);
        }

        if (((mMario->mMovementStates.mLowWord >> 21) & 1U)) {
            if (mMario->calcDistToCeil(false) > 160.0f) {
                mMario->changeAnimation("サマーソルト");
                playSound("スピンジャンプ", -1);
                playSound("声スピン", -1);
            } else {
                return false;
            }
        } else {
            mMario->startMagic();
        }

        setPunchHitTimer(25);
    } else {
        out = false;
    }

    tryReleaseBombTeresa();
    _950 = false;

    tryShootStickingPopo();
    return out;
}

void MarioActor::printHitMark(HitSensor* pSensor) {
    TVec3f vec(*MR::getSensorPos(pSensor) - _240);
    vec.setLength(200.0f);
    playEffectTrans("パンチヒット", _240 + vec);
}

bool MarioActor::tryPunchAirAfter(HitSensor* pSensor) {
    if (pSensor->isType(0x6C)) {
        return false;
    }

    for (u32 idx = 0; idx < _974; idx++) {
        if (_954[idx] == pSensor) {
            return false;
        }
    }

    if (_974 >= 8) {
        return false;
    }

    if (!_3E5 && mPlayerMode != 1) {
        HitSensor* body = getSensor("Body");
        TVec3f vec(*MR::getSensorPos(pSensor));
        vec -= body->mPosition;
        MR::vecKillElement(vec, getGravityVector(), &vec);

        if (MR::diffAngleAbs(vec, mMario->mFrontVec) >= MR::pi() / 4.0f) {
            return false;
        }
    }

    if (sendPunch(pSensor, nullptr, true)) {
        _954[_974] = pSensor;
        _924 = pSensor;
        _974++;

        reactionPunch(pSensor);

        return true;
    }

    return false;
}

bool MarioActor::fn_803CFCA0(HitSensor* s) {
    if (!this->mMario->isPlayerModeYoshi()) {
        return false;
    }
    TVec3f d(*MR::getSensorPos(s));
    d -= this->mPosition;
    const TVec3f* g = &this->mMario->getAirGravityVec();
    TVec3f dd;
    JMAVECScaleAdd((Vec*)g, (Vec*)&d, (Vec*)&d, -g->dot(d));
    if (MR::normalizeOrZero(&d)) {
        return false;
    }
    f32 c = JMath::sSinCosTable.cosDegree(YoshiParam::fn_8041BDC0());
    if (d.dot((this->mMario)->mFrontVec) > c) {
        return false;
    }
    return true;
}

void MarioActor::settingRush() {
    rushDropThrowMemoSensor();

    switch (_924->mType) {
    case ATYPE_WATER_PRESSURE_BULLET_BIND:
        mMario->touchWater();
        break;
    }

    _934 = 1;
    resetCondition();
    mMarioAnim->setAnimationPaused(false);
}

bool MarioActor::isFixJumpRushSensor(const HitSensor* sensor) const {
    const u32 type = sensor->mType;
    if (type - 0x62 <= 3 || type - 0x5E <= 1 || type == 0x5C) {
        return true;
    }
    const char* effect = "乗換センサ";
    return mActionTable1080->testEffect(MR::getSensorHost(sensor)->mName, effect);
}

bool MarioActor::isLandEffectRushSensor(const HitSensor* pSensor) const {
    switch (pSensor->mType) {
    case ATYPE_SPIN_CANNON_BIND:
    case ATYPE_MARIO_LAUNCHER_BIND:
        return true;
    default:
        return false;
    }
}

// NONMATCHING: saved registers and string-pool address hoisting.
void MarioActor::beginRush(HitSensor* target) {
    _924 = target;
    mRushAnimationDelay1054 = 0;
    mMarioAnim->clearAllJointTransform();
    MR::invalidateHitSensors(this);
    stopEffect("共通壁手擦り");
    stopEffect("スピンライト");
    const char* spinCatchEffect = "スピンキャッチ許可";
    bool spinCatch = mActionTable1080->testEffect(MR::getSensorHost(_924)->mName, spinCatchEffect);
    if (mPlayerMode == PlayerMode_Bee) {
        const char* meterEffect = "ハチメータ消去";
        if (mActionTable1080->testEffect(MR::getSensorHost(_924)->mName, meterEffect)) {
            MR::getGameSceneLayoutHolder()->changeLifeMeterModeGround();
        }
    }

    if (isFixJumpRushSensor(_924) || spinCatch) {
        settingRush();
        getSensor("eye")->validate();
        MR::validateHitSensor(this, "Body");
        return;
    }

    MR::validateHitSensor(this, "Body");
    if (_924->isType(ATYPE_POWER_STAR_BIND)) {
        MR::forceDeleteEffectAll(this);
        _1B8->kill();
    }

    const char* landEffect = "着地エフェクト";
    if (mActionTable1080->testEffect(MR::getSensorHost(_924)->mName, landEffect)) {
        playEffect("特殊着地");
    }

    switch (_924->mType) {
    case ATYPE_POWER_STAR_BIND:
        if (mMario->isPlayerModeYoshi()) mYoshiHolder->mYoshi->fn_8040F3C0();
        resetFog();
        break;
    }

    settingRush();
    if (!_924->isType(ATYPE_POWER_STAR_BIND)) {
        setBlendMtxTimer(getConst().getTable()->mRushInBlendTimer);
    }
}

// NONMATCHING: register scheduling, local matrix/vector stack slots, and pooled literal offsets.
void MarioActor::endRush(const RushEndInfo* pInfo) {
    bool launched = false;
    if (_924 && (!pInfo->_1C || MR::getSensorHost(_924) == pInfo->_1C)) {
        _934 = false;
        _7E2 = 0;
        _935 = true;
        mMario->stopJump();
        mMario->mMovementStates.mHighBits.mBit29 = 0;
        s32 mode = pInfo->_4;
        if (mMario->isStatusActive(MarioStatus_Swim)) {
            switch (mode) {
            case 1:
            case 2:
            case 3:
                mode = 4;
                break;
            }
        }

        switch (mode) {
        case 2:
            launched = true;
            mMario->initJumpParam();
            mMario->mMovementStates.mHighBits.mBit30 = 1;
            if (mPlayerMode == PlayerMode_Bee) {
                mMario->_774 = 60;
            }

            mMario->tryForceFreeJump(pInfo->_8);
            mMario->mMovementStates.mLowBits.mBit22 = 1;
            mMario->mFlags14.mLowBits.mBit18 = 1;
            if ((pInfo->_20 >> 30) & 1) {
                mMario->mMovementStates.mHighBits.mBit16 = 1;
            } else {
                mMario->mMovementStates.mHighBits.mBit16 = 0;
            }

            if (pInfo->_20 >> 31) {
                mMario->mMovementStates.mHighBits.mBit29 = 1;
            }

            break;
        case 3: {
            launched = true;
            if (mPlayerMode == PlayerMode_Bee) {
                mMario->_774 = 60;
            }

            mMario->tryForcePowerJump(pInfo->_8, false);
            if ((pInfo->_20 >> 30) & 1) {
                mMario->mMovementStates.mHighBits.mBit16 = 1;
            } else {
                mMario->mMovementStates.mHighBits.mBit16 = 0;
            }

            if ((pInfo->_20 >> 23) & 1) {
                mMario->mMovementStates.mHighBits.mBit31 = 0;
            }

            if (pInfo->_20 >> 31) {
                mMario->mMovementStates.mHighBits.mBit29 = 1;
            }

            if ((pInfo->_20 >> 24) & 15) {
                switch ((pInfo->_20 >> 24) & 15) {
                case 1:
                    mMario->mMovementStates.mLowBits.mBit4 = 1;
                    mMario->mDamage->setVec(pInfo->_8);
                    break;
                case 4:
                    mMario->mFlags14.mLowBits.mBit31 = 1;
                    break;
                case 2:
                    mMario->mFlags14.mLowBits.mBit31 = 1;
                    break;
                case 5:
                    mMario->mFlags14.mLowBits.mBit11 = 1;
                    break;
                case 6:
                    mMario->doFlipLarge(pInfo->_8);
                    break;
                case 3:
                    mMario->doFreeze();
                    break;
                }
            }

            const char* rebindEffect = "リバインド制限";
            mMario->mFlags14.mLowBits.mBit18 = 1;
            if (mActionTable1080->testEffect(MR::getSensorHost(_924)->mName, rebindEffect)) {
                mLastRushEndFrameE1C = _37C;
            }

            break;
        }
        case 1:
            mMario->mFlags14.mLowBits.mBit18 = 1;
            mMario->mMovementStates.mHighBits.mBit30 = 1;
            break;
        case 0:
            mMario->stopWalk();
        case 4:
            mMario->mMovementStates.mLowBits.mBit20 = 0;
            mMario->mMovementStates.mLowBits.mBit31 = 0;
            mMario->_3C0 = 0;
            break;
        }

        mMario->mFlags14.mLowBits.mBit23 = pInfo->_20 >> 22;
        if (!_3D0) {
            TPos3f base;
            TVec3f front;
            PSMTXConcat(getBaseMtx(), mMainJointMatrix4A8, base);
            front.set< f32 >(base[0][2], base[1][2], base[2][2]);
            mMario->setFrontVecKeepUp(front);
        } else {
            TPos3f base;
            TVec3f front;
            TVec3f side;
            TVec3f up;
            PSMTXConcat(getBaseMtx(), mMainJointMatrix4A8, base);
            front.set< f32 >(base[0][2], base[1][2], base[2][2]);
            up.set< f32 >(base[0][1], base[1][1], base[2][1]);
            side.set< f32 >(base[0][0], base[1][0], base[2][0]);
            MR::normalize(&front);
            MR::normalize(&side);
            MR::normalize(&up);
            if (!MR::isNearZero(up)) {
                mMario->mHeadVec = up;
            }

            if (!MR::isNearZero(side)) {
                mMario->mSideVec = side;
            }

            if (!MR::isNearZero(front)) {
                mMario->setFrontVec(front);
            }

            if (up.dot(-*mMario->getGravityVec()) < 0.0f) {
                mMario->setFrontVecKeepSide(mRushFront6E8);
            } else if (_3D0 > 1) {
                mMario->setFrontVecKeepUp(mRushFront6E8, static_cast< u32 >(_3D0));
            } else {
                mMario->setFrontVecKeepUp(mRushFront6E8);
            }
        }

        mMario->_1FC = mMario->mHeadVec;
        if (!pInfo->_14) {
            mMario->stopAnimation(nullptr);
        }

        if (pInfo->_18) {
            setBlendMtxTimer(pInfo->_18);
        }

        MR::validateHitSensors(this);
        getSensor("dummy")->invalidate();
        mMarioAnim->setAnimationPaused(true);
        if (_F1C < 15) {
            _F1C = 15;
        }

        if (_F1E < 15) {
            _F1E = 15;
        }

        _EF8 = 0;
        _F04 = 0;
        if (!((pInfo->_20 >> 24) & 15) && isLandEffectRushSensor(_924) && ((pInfo->_20 >> 30) & 1)) {
            mMario->mMovementStates.mHighPairBits.mLowPair = 1;
            mMario->mMovementStates.mLowBits.mBit31 = 1;
            mMario->mMovementStates.mLowBits.mBit30 = 0;
        } else if (_924->isType(ATYPE_MARIO_LAUNCHER_BIND)) {
            if (!MR::isNearZero(_938)) {
                mMario->mMovementStates.mHighPairBits.mLowPair = 1;
                mMario->mJumpVec = _938;
            }
        } else if (_924->isType(ATYPE_SCENARIO_STARTER_BIND)) {
            if (pInfo->_4) {
                mMario->mMovementStates.mHighPairBits.mLowPair = 2;
                mMario->mMovementStates.mLowBits.mBit31 = 1;
                mMario->mMovementStates.mLowBits.mBit30 = 0;
            } else {
                mMario->changeAnimation("基本");
            }

            updateGravityVec(true, true);
            mVector60C = mPreviousGravityVec * -70.0f;
            mAppliedGravityVec = mPreviousGravityVec;
            if (!MR::isNearZero(mAppliedGravityVec)) mMario->setGravityVec(mAppliedGravityVec);
        }

        _928 = _924;
        _924 = nullptr;
        mMario->mFlags14.mLowWord |= Mario::MovementStates::Turning;
        updateGravityVec(true, false);
        if (mPlayerMode == PlayerMode_Hopper) {
            mMario->startRabbitMode();
        }

        if (mPlayerMode == PlayerMode_Bee) {
            MR::getGameSceneLayoutHolder()->changeLifeMeterModeBee();
        }

        if (launched) {
            mLastMove = mMario->mVelocity;
        } else {
            mMario->mVelocity = mLastMove;
        }

        const char* waterEffect = "水陸移動特殊処理";
        mMarioAnim->_15 = 0xFF;
        if (mActionTable1080->testEffect(MR::getSensorHost(_928)->mName, waterEffect) && !mMario->forceStartSwimAndShoot(pInfo->_8)) {
            mMario->forceExitSwim();
        }

        if (_468 && fn_803CEEE0(_428[0])) {
            mRushAnimationDelay1054 = mConst->getTable()->mDrillCarryAnimationDelay794;
            mMario->stopAnimationUpper(nullptr);
            MarioMtxPos carryMatrix;
            carryMatrix.fn_80409700(MR::getJointMtx(MR::getSensorHost(_428[0]), "DrillPod"));
            carryMatrix.getTranslation(&mCarryAnimationPosition1058);
        }

        mMario->_350.zero();
        mMario->_35C.zero();
        if (_938.length() > 200.0f) {
            mMario->mFlags20.mBits.mBit23 = 1;
        }
    }
}

void MarioActor::addRushSensor(HitSensor* pSensor, bool myBool) {
    if (!MR::isSensorAutoRush(pSensor) && !MR::isSensorRush(pSensor)) {
        return;
    }

    _7E4[_930] = pSensor;
    mRushTouchingDD4[_930] = myBool;
    _930++;
}

bool MarioActor::tryStandardRush() {
    if (((mMario->mMovementStates.mLowWord & Mario::MovementStates::DebugMode) >> 9)) return false;
    if (mMario->isStatusActive(MarioStatus_Bury)) return false;
    if (mMario->isStatusActive(MarioStatus_Sukekiyo)) return false;
    if (!isEnableNerveChange()) return false;
    bool automatic = !isRequestRush();
    if (mMario->isEnableRush()) return tryStartRush(fn_803D0CE0(automatic), automatic);
    return tryStartRush(fn_803D0E40(automatic), automatic);
}

// NONMATCHING: saved registers, string-pool address hoisting, and stack frame size.
bool MarioActor::tryStartRush(HitSensor* target, bool automatic) {
    if (!target) return false;
    if (((mMario->mMovementStates.mLowWord >> 16) & 1U) && mMario->isRising() && target == _928) return false;
    if (!fn_803CB0B0()) return false;
    if (automatic) {
        if (_468) {
            const char* effect = "カメを捨てない";
            if (mActionTable1080->testEffect(MR::getSensorHost(target)->mName, effect)) return false;
        }
        u8 autoBind;
        if (mActionTable1080->selectAutoBind(MR::getSensorHost(target)->mName, &autoBind)) {
            if (!autoBind) return false;
            applyAutoBindPlayerMode(autoBind);
        }
    }
    if (target == _928) {
        const char* effect = "リバインド制限";
        if (mActionTable1080->testEffect(MR::getSensorHost(target)->mName, effect) && static_cast<u32>(_37C - mLastRushEndFrameE1C) < 120) return false;
    }
    u32 message = 0x92;
    if (automatic) message = 0x93;
    if (!target->receiveMessage(message, getSensor("Body"))) return false;
    resetSensorCount();
    mVelocity.zero();
    beginRush(target);
    return true;
}

// NONMATCHING: sensor-loop register allocation and verified constant-pool addresses.
HitSensor* MarioActor::fn_803D0CE0(s32 a) {
    if (this->mHealth == 0) return nullptr;
    if (this->_930 == 0) return nullptr;
    f32 best = 100000.0f;
    HitSensor** currentSlot;
    HitSensor* res = nullptr;
    for (u32 i = 0; i < _930; ++i) {
        currentSlot = &_7E4[i];
        if (isFixJumpRushSensor(*currentSlot) || (a != 0 && !MR::isSensorAutoRush(*currentSlot))) {
            continue;
        }
        if (getCarrySensor() != nullptr) {
            LiveActor* host = MR::getSensorHost(*currentSlot);
            if (MR::getSensorHost(getCarrySensor()) == host) {
                continue;
            }
        }
        const f32 distance = fn_803D1470(*currentSlot);
        if ((mRushTouchingDD4[i] || a == 0 || !(20.0f + (*currentSlot)->mRadius < distance)) && distance < best) {
            res = *currentSlot;
            best = distance;
        }
    }
    return res;
}

HitSensor* MarioActor::fn_803D0E40(s32 a) {
    HitSensor* t = (HitSensor*)this->getHighPriorityTarget();
    if (t == nullptr) {
        t = this->getRescueTarget();
    }
    if (t != nullptr) {
        if (a != 0) {
            if (MR::getSensorRadius(t) < this->fn_803D1470(t)) {
                return nullptr;
            }
        }
        return t;
    }
    if (!this->mMario->isStateHanging()) {
        if (!this->mMario->isDamaging()) {
            const char* effect = "HD中ラッシュ";
            HitSensor** currentSlot;
            u32 i = 0;
            while (i < this->_930) {
                currentSlot = &_7E4[i];
                if (mActionTable1080->testEffect(MR::getSensorHost(*currentSlot)->mName, effect)) {
                    return *currentSlot;
                }
                i++;
            }
        }
    }
    return nullptr;
}

bool MarioActor::fn_803D0F60(HitSensor* s) {
    Mario* m = this->mMario;
    if (((m->mMovementStates.mLowWord >> (9)) & 1)) {
        return false;
    }
    if (s == nullptr) {
        return false;
    }
    if (this->mHealth == 0) {
        return false;
    }
    if (m->isStatusActive(0x13)) {
        return false;
    }
    if (this->mMario->isStatusActive(0x1B)) {
        return false;
    }
    if (!s->receiveMessage(0x94, this->getSensor("Body"))) {
        return false;
    }
    mVelocity.zero();
    this->beginRush(s);
    return true;
}

// NONMATCHING: equivalent early guards, saved registers, and verified pooled literals.
bool MarioActor::tryJumpRush() {
    const char* effect;
    HitSensor* jumpTarget = getNearestJumpTarget();
    if (jumpTarget == nullptr) return false;
    if (fn_803D11C0()) return false;
    u8 autoBind;
    const bool hasAutoBind = mActionTable1080->selectAutoBind(MR::getSensorHost(jumpTarget)->mName, &autoBind);
    if (!hasAutoBind && !mActionTable1080->isActionOk("ジャンプ系バインド")) {
        return false;
    }
    if (_468 != 0) {
        effect = "カメを捨てない";
        if (mActionTable1080->testEffect(MR::getSensorHost(jumpTarget)->mName, effect)) return false;
    }
    if (hasAutoBind && autoBind == 0) {
        return false;
    }
    if (jumpTarget->receiveMessage(ACTMES_AUTORUSH_BEGIN, getSensor("Body"))) {
        resetSensorCount();
        mVelocity.zero();
        beginRush(jumpTarget);
        if (hasAutoBind) {
            applyAutoBindPlayerMode(autoBind);
        }
        return true;
    }
    return false;
}

bool MarioActor::fn_803D11C0() {
    Mario* m = this->mMario;
    if (((m->mMovementStates.mLowWord >> (9)) & 1)) {
        return true;
    }
    if (this->mHealth == 0) {
        return true;
    }
    if (m->fn_8038CCF0()) {
        return true;
    }
    if (this->mMario->isStatusActive(0x13)) {
        return true;
    }
    if (this->mMario->isStatusActive(0x1B)) {
        return true;
    }
    return !this->fn_803CB0B0();
}

// NONMATCHING: equivalent guards, saved sensor reload, and pooled literal address scheduling.
void MarioActor::tryRushInRush() {
    HitSensor* target = getHighPriorityTarget();
    if (!target) target = getNearestJumpTarget();
    _930 = 0;
    if (!target || target == _924) return;
    if (!fn_803D8B30(target)) return;
    if (!target->receiveMessage(0x9A, getSensor("Body"))) return;
    HitSensor* previous = _924;
    if (!_924->receiveMessage(0x95, getSensor("Body"))) return;
    u8 policy;
    if (mActionTable1080->selectAutoBind(MR::getSensorHost(target)->mName, &policy)) {
        if (!policy) return;
        applyAutoBindPlayerMode(policy);
    }
    if (!target->receiveMessage(0x93, getSensor("Body"))) {
        if (_934) {
            RushEndInfo info(nullptr, 4, TVec3f(0.0f, 0.0f, 0.0f), false, 0);
            endRush(&info);
        }
        return;
    }
    if (isFixJumpRushSensor(target)) {
        bool special;
        if (previous->mType == 0x5F || previous->mType == 0x65) special = true;
        else special = false;
        if (special) {
            mMario->mMovementStates.mHighWord &= 0xFFFFFFFCU;
            playEffect("スーパースピンドライバ終了");
        }
    }
    beginRush(target);
}

f32 MarioActor::fn_803D1470(HitSensor* s) {
    f32 db;
    TVec3f p(*MR::getSensorPos(s));
    TVec3f a(p);
    a -= this->mPosition;
    f32 da = PSVECMag((Vec*)&a);
    TVec3f b(p);
    b -= this->_240;
    db = PSVECMag((Vec*)&b);
    TVec3f c(p);
    c -= this->mCapPosition;
    f32 dc = PSVECMag((Vec*)&c);
    if (da > db) {
        da = db;
    }
    if (da > dc) {
        da = dc;
    }
    return da;
}

void MarioActor::setupSensors() {
    initHitSensor(4);

    MR::addHitSensorCallback(this, "Body", ATYPE_PLAYER, 32, 100.0f);
    MR::addHitSensorCallback(this, "ex-eye", ATYPE_PUSH, 32, 100.0f);
    MR::addHitSensorCallback(this, "eye", ATYPE_EYE, 64, 2000.0f);

    MR::addHitSensor(this, "dummy", ATYPE_PLAYER, 1, 0.0f, TVec3f(0.0f, 0.0f, 0.0f));

    getSensor("dummy")->invalidate();

    _468 = 0;
    _46C = nullptr;

    _7DC = 0;

    for (s32 idx = 0; idx < ARRAY_SIZE(mPulledSensors7C0); idx++) {
        mPulledSensors7C0[idx] = nullptr;
    }

    for (s32 idx = 0; idx < ARRAY_SIZE(_428); idx++) {
        _428[idx] = nullptr;
    }

    initScouter();

    _3E5 = false;
    _3E8 = false;

    MR::initStarPointerTarget(this, 80.0f, TVec3f(0.0f, 80.0f, 0.0f));
}

void MarioActor::updateHitSensor(HitSensor* pSensor) {
    u32 type = pSensor->mType;
    if (type == 1) { fn_803D1740(pSensor); return; }
    if (type == 0x7B) { pSensor->setType(0x7A); return; }
    bool scouter = false;
    if (type == 0x7A && pSensor == getSensor("ex-eye")) scouter = true;
    if (scouter) { updateScouter(); return; }
    if (pSensor->mType == 0x7A) fn_803D1830(pSensor);
}

// NONMATCHING: Typed sensor position uses direct member addresses; saved-register and frame scheduling differ.
void MarioActor::fn_803D1740(HitSensor* s) {
    if (this->mMario->isStatusActive(5)) {
        Mtx m;
        const char* spineJoint = "Spine1";
        this->getRealMtx(m, spineJoint);
        MR::extractMtxTrans(m, &(s)->mPosition);
        (s)->mRadius = 60.0f;
        return;
    }
    if (this->mMario->isStatusActive(6)) {
        Mtx m;
        const char* spineJoint = "Spine1";
        this->getRealMtx(m, spineJoint);
        MR::extractMtxTrans(m, &(s)->mPosition);
        (s)->mRadius = 70.0f;
        return;
    }
    (s)->mPosition.set(this->_240);
    Mario* m = this->mMario;
    u32 st = (m)->mMovementStates.mLowWord;
    if (((st >> 0x14) & 1) && !((st >> 0x1E) & 1)) {
        (s)->mPosition += (m)->mJumpVec;
    }
    (s)->mRadius = 100.0f;
}

void MarioActor::fn_803D1830(HitSensor* s) {
    (s)->mPosition.set(this->_240);
    (s)->mRadius = this->fn_803D18D0();
    this->fn_803D1940();
    bool r = false;
    if (s->mValidByHost != 0 && s->mValidBySystem != 0) {
        r = true;
    }
    if (r) {
        this->attackOrPushPolygons();
    }
    HitSensor* t = _424;
    if (t != nullptr) {
        this->tryTornadoPull(t);
    }
}

f32 MarioActor::fn_803D18D0() {
    if (this->mMario->isSwimming()) {
        return 2000.0f;
    }
    if (static_cast<s32>(_468) != 0) {
        return 2000.0f;
    }
    if (((this->mMario->mMovementStates.mLowWord >> (0x10)) & 1)) {
        return 1000.0f;
    }
    return 600.0f;
}

void MarioActor::fn_803D1940() {
    this->_3E5 = 0;
    this->_3E6 = 0;
    const char* cur = this->mMarioAnim->fn_803DF0D0();
    if (strcmp(cur, "spin2nd") == 0) {
        this->_3E5 = 1;
        this->_3E6 = 1;
        if (this->_944 <= 2) {
            this->setPunchHitTimer(2);
        }
    } else if (!this->mMario->isAnimationTerminate(nullptr) &&
               (((this->mMario)->_430 == 8 && this->isJumping()) || this->mMario->isAnimationRun("地上ひねり") ||
                this->mMario->isAnimationRun("雲スピン") || this->mMario->isAnimationRun("サマーソルト") ||
                this->mMario->isAnimationRun("水泳スピン") || this->mMario->isAnimationRun("水上スピン") ||
                this->mMario->isAnimationRun("しゃがみスピン") || this->mMario->isAnimationRun("フーファイタースピン") ||
                this->mMario->isAnimationRun("ハチスピン") || this->mMario->isAnimationRun("ハチスピン空中") ||
                this->mMario->isAnimationRun("アイスひねり空中") || this->mMario->isAnimationRun("ファイアスピン空中") ||
                this->mMario->isAnimationRun("ファイアスピン") || this->mMario->isAnimationRun("アイスひねり") ||
                this->mMario->isAnimationRun("アイスひねり移動") || this->mMario->isAnimationRun("アイスひねり静止") ||
                this->mMario->isAnimationRun("ハンマー投げリリース") ||
                (this->mMario->isSwimming() && this->mMario->mSwim->fn_803FE9F0()) ||
                (this->mMario->isStatusActive(0x18) && this->mMario->mFoo->_4C != 0))) {
        this->_3E5 = 1;
        this->_3E6 = 1;
    }
}

// NONMATCHING: equivalent conditional branch layout and verified pooled literals.
void MarioActor::attackSensor(HitSensor* pOwn, HitSensor* pOther) {
    if (MR::isSensorType(pOther, 0x3D)) return;
    if (!isEnableNerveChange()) return;

    if (pOwn->mType == ATYPE_PLAYER) {
        if (_934 && !getSensor("eye")->isValid()) {
            addRushSensor(pOther, false);
        }
    } else {
        if (pOwn == getSensor("eye")) {
            if (MR::isDead(MR::getSensorHost(pOther))) {
                return;
            }

            if (_934) {
                attackOrPushSensorInRush(pOther, (*MR::getSensorPos(pOther) - *MR::getSensorPos(pOwn)).length());
            } else if (mMario->isDamaging()) {
                attackOrPushSensorInDamage(pOther, (*MR::getSensorPos(pOther) - *MR::getSensorPos(pOwn)).length());
            } else {
                attackOrPushSensor(pOther, (*MR::getSensorPos(pOther) - *MR::getSensorPos(pOwn)).length());
            }
        }

        if (pOwn == getSensor("ex-eye")) {
            recordScoutingObject(pOther);
        }
    }
}

void MarioActor::resetSensorCount() {
    _930 = 0;
    _7DC = 0;
    _46C = 0;
}

void MarioActor::recordScoutingObject(HitSensor* pSensor) {
    if (pSensor == _424) {
        return;
    }

    if (!MR::isSensorEnemy(pSensor) && !MR::isSensorMapObj(pSensor) && !MR::isSensorRide(pSensor)) {
        return;
    }

    HitSensor* eye = getSensor("ex-eye");
    TVec3f direction(*MR::getSensorPos(pSensor));
    direction -= eye->mPosition;
    if (MR::diffAngleAbsHorizontal(direction, mMario->mFrontVec, mAppliedGravityVec) >= 1.5707964f) {
        return;
    }

    _9D4 = pSensor;
    _9D8 = getSensor("ex-eye")->mPosition;
    _9CC = _9D0;
    _9D0 = 60.0f;
}

// NONMATCHING: independent timer/sensor-store scheduling and verified string pools.
void MarioActor::updateScouter() {
    const u32 count = _468;
    HitSensor* previous = _F24;
    _F24 = nullptr;
    if (count) {
        if (_9D4 != nullptr && MR::isExistInAttributeGroupSearchTurtle(MR::getSensorHost(_9D4))) {
            if (MR::isSensorEnemy(_9D4)) {
                _F28 = 16;
            } else {
                _F28 = 2;
            }

            _F24 = _9D4;
        } else if (_F28) {
            _F28--;
            _F24 = previous;
        }
    }

    _9D0 += 80.0f;
    if (_9D0 < 100.0f) {
        _9D0 = 100.0f;
    }

    if (_9D0 > _9CC) {
        _9D4 = nullptr;
    }

    f32 maximum = 1000.0f;
    if (mMario->isStatusActive(MarioStatus_Foo)) {
        maximum = 2400.0f;
    }

    if (_468) {
        maximum = 3000.0f;
    }

    if (_9D0 > maximum) {
        _9D0 = 60.0f;
    }

    TVec3f position;
    if (mMario->isSwimming()) {
        position = _240 + _270 * _9D0;
    } else {
        position = _240 + mMario->mFrontVec * _9D0;
    }

    getSensor("ex-eye")->mPosition = position;
    f32 radius = 100.0f;
    if (_9D0 >= 300.0f) {
        radius = 100.0f + (_9D0 - 300.0f) * MR::tan(5.0f * MR::pi() / 180.0f);
    }

    if (_9D0 < 200.0f) {
        radius = 40.0f;
    }

    getSensor("ex-eye")->mRadius = radius;
}

void MarioActor::initScouter() {
    _9CC = 0.0f;
    _9D0 = 60.0f;
    _9D4 = 0;
    _9D8.zero();

    HitSensor* sensor = getSensor("ex-eye");
    sensor->mRadius = 100.0f;
    getSensor("ex-eye")->validate();
}

void MarioActor::doTrampleJump(HitSensor* pSensor) {
    if (((mMario->mFlags20.mWord >> 25) & 1U)) return;
    if (mMario->isStatusActive(MarioStatus_Wall)) mMario->closeStatus(mMario->mWall);
    mMario->_402 = getConst().getTable()->mAirWalkTimeTornado;
    mMario->mMovementStates.mHighWord &= 0xFFEFFFFFU;
    if (pSensor->mType == 0x23) {
        _988 = 0;
        trampleJump(getConst().getTable()->mTrampleBegoma, getConst().getTable()->mTrampleLong);
        mMario->changeAnimationNonStop("ヘリコプタージャンプ");
        mMario->startPadVib(2);
        mMario->playSound("ヘリコプタージャンプ", -1);
        mMario->startRotationTask(4);
        mMario->_430 = 11;
        return;
    }
    if (strcmp(MR::getSensorHost(pSensor)->mName, "巨大リクノコ") == 0) {
        mMario->playSoundTrampleCombo(_989);
        ++_989;
        if (_989 >= 8) {
            _1B8->kill();
            _989 = 7;
            MR::addPlayerLeft(1);
        }
    }
    trampleJump(getConst().getTable()->mTrampleNormal, getConst().getTable()->mTrampleLong);
}

// NONMATCHING: verified string and member-function-pointer pool addresses only.
void MarioActor::trampleJump(f32 normal, f32 extra) {
    if (mMario->isStatusActive(MarioStatus_Foo)) {
        return;
    }

    TVec3f velocity(mMario->mJumpVec);
    MR::vecKillElement(velocity, getGravityVec(), &velocity);
    velocity.add(-getGravityVec() * normal);
    switch (_988) {
    case 2:
        if (mMario->checkLvlA()) {
            velocity *= 1.2f;
        } else {
            velocity *= 1.5f;
        }

        break;
    }

    if (mMario->checkLvlA()) {
        velocity.add(-getGravityVec() * extra);
    }

    mMario->tryForceFreeJump(velocity);
    mMario->popTask(&Mario::taskOnHipDropSlide);
    if (mPlayerMode != PlayerMode_Hopper) {
        if (!((mMario->mMovementStates.mLowWord >> 21) & 1U)) {
            _988++;
            if (_988 == 1) {
                mMario->changeAnimation("ジャンプふみ1");
            } else if (_988 == 2) {
                mMario->changeAnimation("ジャンプふみ2");
            } else if (_988 >= 3) {
                mMario->changeAnimation("ジャンプふみ3");
                _988 = 0;
            }
        }
    } else {
        stopAnimation(nullptr);
        switch (mMario->mRabbit->_68) {
        case 0:
            mMario->changeAnimation("ホッパーふみジャンプA");
            break;
        case 1:
            mMario->changeAnimation("ホッパーふみジャンプB");
            break;
        }
    }

    if (mMario->isPlayerModeYoshi()) {
        MR::startActionSound(mYoshiHolder->mYoshi, "Stomp", -1, -1, -1);
    } else {
        mMario->playSound("声踏み", -1);
    }
    playEffect("ふみつぶし");
    mMario->startPadVib(0UL);
    mMario->mMovementStates.mHighWord &= 0xFFFEFFFFU;
    mMario->mMovementStates.mHighWord &= 0xDFFFFFFFU;
    mMario->mMovementStates.mHighWord &= 0xFFFFFFFCU;
}

void MarioActor::initShadow() {
    mShadowEnabledEFA = true;
    mWaterReflectionEF8 = false;
    mSphereReflectionEF9 = false;
    mGroundReflectionEFC = false;
    mShadowModeEFB = 0;
    fn_803CA1F0();
    mShadowEnabledEFA = true;

    if (gIsLuigi) {
        mSimpleModelActorF04 = new ModelHolder("LuigiShadow", true);
    } else {
        mSimpleModelActorF04 = new ModelHolder("MarioShadow", true);
    }

    mSimpleModelActorF04->initWithoutIter();

    mSimpleModelF08 = reinterpret_cast< J3DModelX* >(MR::getJ3DModel(mSimpleModelActorF04));
    u32 idx;
    J3DModelX* pSource = mAllModel->mModels[0];
    J3DModelX* pShadow = mSimpleModelF08;
    pShadow->_DD = pSource->_DD;

    for (idx = 0; idx < pShadow->_DD; idx++) {
        pShadow->mExtraMtxBuffer[idx] = pSource->mExtraMtxBuffer[idx];
    }
}

// NONMATCHING: boolean materialization, constant conversion and stack allocation.
void MarioActor::fn_803D26F0() {
    Mario* m = this->mMario;
    bool reflect = false;
    if (((m->mMovementStates.mLowWord >> 0x1D) & 1) && !m->fn_803AA2C0() && !mMario->isStatusActive(0x12) &&
        (mMario->_962 == 0xE || mMario->_962 == 0x21)) {
        const TVec3f* gravity = mMario->getGravityVec();
        TVec3f camera = MR::getCamZdir();
        reflect = gravity->dot(camera) >= 0.0f;
    }
    if (reflect) {
        this->fn_803CA1F0();
        this->mGroundReflectionEFC = 1;
        f32 v = (this->mMario)->mVerticalSpeed / 500.0f;
        if (v >= 0.75f) {
            v = 0.75f;
        }
        s32 target = (s32)((f32)0x40U * (1.0f - v));
        for (u32 i = 0; i < 4; ++i) {
            u8 c = this->mGroundReflectionAlphaEFD;
            if (c < (u8)target) {
                this->mGroundReflectionAlphaEFD = c + 1;
            } else if (c > (u8)target) {
                this->mGroundReflectionAlphaEFD = c - 1;
            }
        }
    } else {
        u8 c = this->mGroundReflectionAlphaEFD;
        if (c != 0) {
            if (c > 4) {
                this->mGroundReflectionAlphaEFD = c - 4;
            } else {
                this->mGroundReflectionAlphaEFD = 0;
            }
            if (this->mGroundReflectionAlphaEFD == 0) {
                this->fn_803CA1F0();
                this->mShadowEnabledEFA = 1;
            } else {
                this->fn_803CA1F0();
                this->mGroundReflectionEFC = 1;
            }
        } else {
            this->fn_803CA1F0();
            this->mShadowEnabledEFA = 1;
        }
    }
    if (this->mMario->isSwimming() && MR::isInWater(mCameraPosition)) {
        if (this->mFlagF19 != 0 || mCameraBasisZ.dot(-*this->mMario->getGravityVec()) > 0.0f) {
            this->fn_803CA1F0();
            this->mWaterReflectionEF8 = 1;
        }
        if (this->mFlagF18 != 0 ||
            (this->mMario->mSwim->getWaterEdgeDist() > 0.0f && this->mMario->mSwim->getWaterEdgeDist() < 1000.0f)) {
            this->fn_803CA1F0();
            this->mWaterReflectionEF8 = 1;
            this->mSphereReflectionEF9 = 1;
            this->mShadowEnabledEFA = 1;
        }
    }
    mCollisionShadow580->setUpdateFlag();
}

// NONMATCHING: literal-pool symbol offsets only; resolved calls, scalars, strings, and instructions match.
void MarioActor::calcViewReflectionModel() {
    J3DModelX* pModel = static_cast<J3DModelX*>(mAllModel->getJ3DModel());
    TPos3f matrix;
    PSMTXCopy(mAllModel->getJ3DModel()->getBaseTRMtx(), matrix.toMtxPtr());
    TPos3f offset;
    PSMTXIdentity(offset.toMtxPtr());
    f32 factor;
    f32 distance = 10.0f;
    if (mMario->isAnimationRun("ターンジャンプ")) {
        distance = 25.0f;
    }

    if (!mSphereReflectionEF9) mFlagF18 = 0;
    if (!mWaterReflectionEF8) mFlagF19 = 0;
    TVec3f position;
    if (mGroundReflectionEFC) {
        matrix.mMtx[0][1] = -matrix.mMtx[0][1];
        matrix.mMtx[1][1] = -matrix.mMtx[1][1];
        matrix.mMtx[2][1] = -matrix.mMtx[2][1];
        position = mMario->mShadowPos - *mMario->_45C->getNormal(0) * distance;
        TVec3f root;
        getRealPos("All_Root", &root);
        TVec3f translation(mPosition - root);
        PSMTXTrans(offset.toMtxPtr(), translation.x, translation.y, translation.z);
        updateReflectAlphaDL(mGroundReflectionAlphaEFD);
    } else if (mWaterReflectionEF8) {
        TVec3f x;
        x.set< f32 >(matrix.mMtx[0][0], matrix.mMtx[1][0], matrix.mMtx[2][0]);
        TVec3f y;
        y.set< f32 >(matrix.mMtx[0][1], matrix.mMtx[1][1], matrix.mMtx[2][1]);
        TVec3f z;
        z.set< f32 >(matrix.mMtx[0][2], matrix.mMtx[1][2], matrix.mMtx[2][2]);
        const TVec3f& rNormal = mMario->mSwim->mSurfacePos;
        f32 dx = MR::vecKillElement(x, rNormal, &x);
        f32 dy = MR::vecKillElement(y, rNormal, &y);
        f32 dz = MR::vecKillElement(z, rNormal, &z);
        x -= rNormal * dx;
        y -= rNormal * dy;
        z -= rNormal * dz;
        matrix.mMtx[0][0] = x.x;
        matrix.mMtx[1][0] = x.y;
        matrix.mMtx[2][0] = x.z;
        matrix.mMtx[0][1] = y.x;
        matrix.mMtx[1][1] = y.y;
        matrix.mMtx[2][1] = y.z;
        matrix.mMtx[0][2] = z.x;
        matrix.mMtx[1][2] = z.y;
        matrix.mMtx[2][2] = z.z;
        position = mMario->mSwim->mSurfaceNorm - rNormal * distance;
        f32 depth = -mMario->mSwim->mWaterDepth;
        if (depth < 40.0f) {
            factor = 0.0f;
        } else if (depth < 200.0f) {
            factor = (depth - 40.0f) / 160.0f;
        } else {
            factor = 1.0f;
        }

        f32 alpha = mCameraBasisZ.dot(rNormal);
        alpha = MR::clamp(alpha, 0.0f, 1.0f);
        alpha *= 1.414f;
        alpha = MR::clamp(alpha, 0.0f, 1.0f);
        alpha *= 64.0f;
        f32 target = factor * alpha;
        if (mFlagF19 > static_cast< u8 >(target)) {
            mFlagF19--;
        } else if (mFlagF19 < static_cast< u8 >(target)) {
            mFlagF19++;
        }

        updateSimpleAlphaDL(mFlagF19);
        mReflectionPositionF44 = position;
    } else if (mSphereReflectionEF9) {
        TVec3f x;
        x.set< f32 >(matrix.mMtx[0][0], matrix.mMtx[1][0], matrix.mMtx[2][0]);
        TVec3f y;
        y.set< f32 >(matrix.mMtx[0][1], matrix.mMtx[1][1], matrix.mMtx[2][1]);
        TVec3f z;
        z.set< f32 >(matrix.mMtx[0][2], matrix.mMtx[1][2], matrix.mMtx[2][2]);
        TVec3f normal(mMario->mSwim->_178 - mPosition);
        MR::normalize(&normal);
        f32 dx = MR::vecKillElement(x, normal, &x);
        f32 dy = MR::vecKillElement(y, normal, &y);
        f32 dz = MR::vecKillElement(z, normal, &z);
        x -= normal * dx;
        y -= normal * dy;
        z -= normal * dz;
        matrix.mMtx[0][0] = x.x;
        matrix.mMtx[1][0] = x.y;
        matrix.mMtx[2][0] = x.z;
        matrix.mMtx[0][1] = y.x;
        matrix.mMtx[1][1] = y.y;
        matrix.mMtx[2][1] = y.z;
        matrix.mMtx[0][2] = z.x;
        matrix.mMtx[1][2] = z.y;
        matrix.mMtx[2][2] = z.z;
        position = mMario->mSwim->_178 - normal * distance;
        f32 edgeDistance = mMario->mSwim->getWaterEdgeDist();
        if (edgeDistance < 0.0f) {
            factor = 0.0f;
        } else if (edgeDistance < 1000.0f) {
            factor = 0.5f + 0.5f * (edgeDistance / 1000.0f);
        } else {
            factor = 0.0f;
        }

        f32 alpha = mCameraBasisZ.dot(normal);
        alpha = MR::clamp(alpha, 0.0f, 1.0f);
        alpha *= 1.414f;
        alpha = MR::clamp(alpha, 0.0f, 1.0f);
        alpha *= 127.0f;
        f32 target = factor * alpha;
        if (mFlagF18 > static_cast< u8 >(target)) {
            mFlagF18--;
        } else if (mFlagF18 < static_cast< u8 >(target)) {
            mFlagF18++;
        }

        updateSimpleAlphaDL(mFlagF18);
        mReflectionPositionF44 = position;
    }

    matrix.setTrans(position);
    TPos3f reflected;
    PSMTXCopy(matrix.toMtxPtr(), reflected.toMtxPtr());
    if (getCarrySensor() != nullptr) {
        TPos3f carry;
        PSMTXConcat(mReflectionBaseMtx264.mMtx, MR::getJointMtx(MR::getSensorHost(getCarrySensor()), (s32)0), carry.toMtxPtr());
        PSMTXConcat(reflected.toMtxPtr(), carry.toMtxPtr(), reflected.toMtxPtr());
        mJetTurtleShadowE94->calcType0(reflected.toMtxPtr());
    }

    PSMTXConcat(mReflectionBaseMtx264.mMtx, offset.toMtxPtr(), offset.toMtxPtr());
    PSMTXConcat(matrix.toMtxPtr(), offset.toMtxPtr(), matrix.toMtxPtr());
    PSMTXConcat(j3dSys.mViewMtx, matrix.toMtxPtr(), j3dSys.mViewMtx);
    pModel->viewCalc3(3, nullptr);
    MR::loadViewMtx();
}

// NONMATCHING: pooled string literal label only.
void MarioActor::calcViewSilhouetteModel() {
    J3DModelX* pModel = static_cast<J3DModelX*>(mAllModel->getJ3DModel());
    J3DModelX* pSimple = mSimpleModelF08;
    TVec3f direction(mPosition - mCameraPosition);
    MR::normalizeOrZero(&direction);
    PSMTXCopy(pModel->getBaseTRMtx(), pSimple->getBaseTRMtx());
    TPos3f& matrix = pSimple->mBaseTransformMtx;
    TVec3f x;
    matrix.getXDir(x);
    MR::vecKillElement(x, direction, &x);
    TVec3f y;
    matrix.getYDir(y);
    MR::vecKillElement(y, direction, &y);
    TVec3f z;
    matrix.getZDir(z);
    MR::vecKillElement(z, direction, &z);
    matrix.setXDir(x);
    matrix.setYDir(y);
    matrix.setZDir(z);
    TVec3f root;
    TVec3f translation;
    if (_EA4) {
        const char* rootJoint = "All_Root";
        TPos3f rootMatrix;
        getRealMtx(rootMatrix.toMtxPtr(), rootJoint);
        MR::extractMtxTrans(rootMatrix.toMtxPtr(), &root);
        translation = root - direction * getConst().getTable()->mSilhouetteZoffset;
    } else if (mMario->isPlayerModeYoshi()) {
        MR::copyJointPos(mYoshiHolder->mYoshi, "MarioPos", &translation);
    } else {
        translation = mPosition;
    }

    MR::setMtxTrans(matrix.toMtxPtr(), translation.x, translation.y, translation.z);
    PSMTXConcat(matrix.toMtxPtr(), mReflectionBaseMtx264.mMtx, matrix.toMtxPtr());
    pSimple->viewCalcRef(1, pModel);
}

void MarioActor::fn_803D32F0() {
    mWallShadow574->mActive = 0;
    if (this->mGroundReflectionEFC || this->mWaterReflectionEF8 || this->mSphereReflectionEF9) {
        return;
    }
    if (this->mBeeWallWalk) {
        return;
    }

    J3DModel* mdl;
    J3DModelX* mx;
    const TVec3f* gravity;
    TVec3f axis;
    s32 len = 0;
    AreaObj* area;
    area = MR::getAreaObj("ExtraWallCheckArea", this->_240);
    if (area != nullptr) {
        MR::calcCubeAxisZ(area, &axis);
        len = MR::getAreaObjArg(area, 0);
        mExtraWallArea578 = area;
        mExtraWallAreaKind57C = 0;
    } else {
        area = MR::getAreaObj("ExtraWallCheckCylinder", this->_240);
        if (area != nullptr) {
            len = this->calcCylinderToCenter(area, &axis);
            mExtraWallArea578 = area;
            mExtraWallAreaKind57C = 1;
        }
    }

    bool fading = false;
    if (area == nullptr || (f32)len < 0.0f) {
        f32 f = mWallShadow574->mDistance;
        if (f >= 1000.0f) {
            return;
        }
        mWallShadow574->mDistance = 10.0f + f;
        if (mWallShadow574->mDistance >= 1000.0f) {
            mWallShadow574->mDistance = 1000.0f;
        }
        switch (mExtraWallAreaKind57C) {
        case 0:
            MR::calcCubeAxisZ(mExtraWallArea578, &axis);
            len = MR::getAreaObjArg(mExtraWallArea578, 0);
            break;
        case 1:
            len = this->calcCylinderToCenter(mExtraWallArea578, &axis);
            break;
        }
        fading = true;
    }

    Triangle tri;
    TVec3f hit;
    if (!this->mMario->fn_803B2770(&hit, &tri, this->_240, axis * (f32)len)) {
        return;
    }

    mWallShadow574->mPosition.set(hit);
    mWallShadow574->mNormal.set(*tri.getNormal(0));
    if (!fading) {
        f32 distanceToHit = (hit - this->_240).length();
        f32 newContribution = 0.1f * distanceToHit;
        f32 oldDistance = mWallShadow574->mDistance;
        f32 oldContribution = 0.9f * oldDistance;
        mWallShadow574->mDistance = newContribution + oldContribution;
    }
    this->updateRandomTexture(mWallShadow574->mDistance);
    TVec3f nrm(axis);
    MR::normalizeOrZero(&nrm);
    mdl = mAllModel->getJ3DModel();
    mx = mSimpleModelF08;
    PSMTXCopy(mdl->getBaseTRMtx(), mx->getBaseTRMtx());
    TPos3f& rot = mx->mBaseTransformMtx;
    TVec3f x;
    rot.getXDir(x);
    MR::vecKillElement(x, nrm, &x);
    TVec3f y;
    rot.getYDir(y);
    MR::vecKillElement(y, nrm, &y);
    TVec3f z;
    rot.getZDir(z);
    MR::vecKillElement(z, nrm, &z);
    rot.setXDir(x);
    rot.setYDir(y);
    rot.setZDir(z);
    gravity = this->mMario->getGravityVec();
    TVec3f perp;
    MR::vecKillElement(this->mPosition - hit, *gravity, &perp);
    f32 dist = perp.length();
    TVec3f trans = this->mPosition + nrm * (10.0f + dist);
    MR::setMtxTrans(rot.toMtxPtr(), trans.x, trans.y, trans.z);
    PSMTXConcat(rot.toMtxPtr(), mReflectionBaseMtx264.mMtx, rot.toMtxPtr());
    mx->viewCalcRef(3, mdl);
    mWallShadow574->mActive = 1;
}

// NONMATCHING: equivalent fadds operand order.
f32 MarioActor::calcCylinderToCenter(const AreaObj* pAreaObj, TVec3f* pVec) {
    TVec3f upVec;
    MR::calcCylinderUpVec(&upVec, pAreaObj);

    TVec3f centerPos;
    MR::calcCylinderCenterPos(&centerPos, pAreaObj);

    MR::vecKillElement(centerPos - _240, upVec, pVec);

    MR::normalizeOrZero(pVec);
    if (MR::normalizeOrZero(pVec)) {
        if (mWallShadow574->mDistance >= 1000.0f) {
            return -1.0f;
        }

        mWallShadow574->mDistance = 10.0f + mWallShadow574->mDistance;

        if (mWallShadow574->mDistance >= 1000.0f) {
            mWallShadow574->mDistance = 1000.0f;
        }

        return -1.0f;
    }

    return MR::getAreaObjArg(pAreaObj, 0);
}

void MarioActor::calcViewBlurModel() {
    J3DModelX* pModel = static_cast<J3DModelX*>(mAllModel->getJ3DModel());
    if (pModel->_1E5) {
        if (_A6E == 1) {
            _A6E = 2;
        }

        pModel->_1E5 = 0;
    }

    if (MR::isDemoActive()) {
        if (_A6E) {
            _A6E = 5;
        }

        return;
    }

    if (_A0C) {
        return;
    }

    if (!_A6E) {
        return;
    }

    if (_A6E >= 3) {
        _A6E--;
        return;
    }

    if (_A6E == 2) {
        for (u32 i = 0; i < 8; i++) {
            for (u32 j = 0; j < mAllModel->getJ3DModel()->mModelData->getDrawMtxNum(); j++) {
                PSMTXCopy(pModel->getDrawMtx(j), mBlurMatrices[0][i][j]);
                PSMTXCopy(pModel->getDrawMtx(j), mBlurMatrices[1][i][j]);
            }
        }

        mBlurHistoryIndex = 0;
        _A6E = 1;
    } else if (_37C % 3 == 0) {
        mBlurHistoryIndex = (mBlurHistoryIndex + 1) & 7;
        for (u32 j = 0; j < mAllModel->getJ3DModel()->mModelData->getDrawMtxNum(); j++) {
            PSMTXCopy(pModel->getDrawMtx(j), mBlurMatrices[1 - mBlurBufferIndex][mBlurHistoryIndex][j]);
        }
    }

    mBlurBufferIndex = 1 - mBlurBufferIndex;
    pModel->_1E4 = 1;
}

void MarioActor::calcViewFootPrint() {
    if (!_934 && ((mMario->mMovementStates.mLowWord >> 30) & 1U) &&
        (mMario->mTargetWalkSpeedIndex || ((getMario()->mFlags20.mWord & Mario::DrawStates::Underwater) >> 13) || mMario->mWalkSpeed > 0.1f || mMario->isPlayerModeHopper())) {
        if (!mMario->mSinkTimer && !((mMario->mDrawStates.mWord >> 22) & 1U) && !((mMario->mDrawStates.mWord >> 21) & 1U) && ((mMario->mFlags20.mWord >> 12) & 1U) &&
            (mMario->checkCurrentFloorCodeSevere(13) || mMario->checkCurrentFloorCodeSevere(26) ||
                mMario->checkCurrentFloorCodeSevere(43) || mMario->checkCurrentFloorCodeSevere(42))) {
            _B48->addPrint(mPosition, mMario->mFrontVec, mMario->_368, false);
        }
    }

    if (_B48->isValid(_37C) && MR::isInWater(*_B48->getPrintPos(_37C))) {
        _B48->invalidate(_37C);
    }
}

void MarioActor::initScreenBox() {
    mScreenBuffer = new (32) u8[0x20000];
    mScreenF50.y = 0.0f;
    mScreenF50.x = 0.0f;
    mScreenF58.y = 0.0f;
    mScreenF58.x = 0.0f;
    mScreenF60.y = 0.0f;
    mScreenF60.x = 0.0f;
    mScreenF68.y = 0.0f;
    mScreenF68.x = 0.0f;
}

bool MarioActor::isUseScreenBox() const {
    if (mWaterReflectionEF8 || mSphereReflectionEF9) {
        if (!mShadowEnabledEFA) {
            return true;
        }
    }
    return false;
}

// NONMATCHING: integer-to-float conversion scheduling and local stack/register allocation.
void MarioActor::calcScreenBoxRange() {
    if (!isUseScreenBox()) {
        return;
    }

    TVec3f position;
    position = mReflectionPositionF44;
    TVec2f center;
    TVec2f x;
    TVec2f y;
    TVec2f z;
    TVec2f minimum;
    TVec2f maximum;
    TVec2f frameMin;
    TVec2f frameMax;
    MR::calcScreenPosition(&center, position);
    MR::calcScreenPosition(&x, position + TVec3f(80.0f, 0.0f, 0.0f));
    MR::calcScreenPosition(&y, position + TVec3f(0.0f, 80.0f, 0.0f));
    MR::calcScreenPosition(&z, position + TVec3f(0.0f, 0.0f, 80.0f));
    TVec3f radius((x - center).length(), (y - center).length(), (z - center).length());
    f32 range = radius.length();
    minimum = center - TVec2f(range, range);
    maximum = center + TVec2f(range, range);
    mScreenF50.x = minimum.x < maximum.x ? minimum.x : maximum.x;
    mScreenF58.x = minimum.x > maximum.x ? minimum.x : maximum.x;
    mScreenF50.y = minimum.y < maximum.y ? minimum.y : maximum.y;
    mScreenF58.y = minimum.y > maximum.y ? minimum.y : maximum.y;
    MR::convertScreenPosToFrameBufferPos(&frameMin, mScreenF50);
    MR::convertScreenPosToFrameBufferPos(&frameMax, mScreenF58);
    s16 left = static_cast< s32 >(frameMin.x) & ~1;
    s16 top = static_cast< s32 >(frameMin.y) & ~1;
    s16 width = static_cast< s32 >(frameMax.x) - left;
    s16 height = static_cast< s32 >(frameMax.y) - top;
    width = (width + 1) & ~1;
    height = (height + 1) & ~1;

    if (width > 256) {
        width = 256;
    }

    if (height > 256) {
        height = 256;
    }

    if (!width) {
        width = 2;
    }

    if (!height) {
        height = 2;
    }

    mScreenF60.x = left;
    mScreenF60.y = top;
    mScreenF68.x = width;
    mScreenF68.y = height;
    TBox2f capture(mScreenF60.x, mScreenF60.y, mScreenF60.x + mScreenF68.x, mScreenF60.y + mScreenF68.y);
    s32 screenHeight = MR::getScreenHeight();
    TBox2f screen(0.0f, 0.0f, static_cast<s32>(JUTVideo::getManager()->getFbWidth()), screenHeight);
    TBox2f clipped(capture);

    if (!clipped.intersect(screen)) {
        clipped.i.zero();
        clipped.f.zero();
    }

    mScreenF60.x = clipped.i.x;
    mScreenF60.y = clipped.i.y;
    mScreenF68.x = clipped.getWidth();
    mScreenF68.y = clipped.getHeight();
}

void MarioActor::captureScreenBox() const {
    if (!isUseScreenBox()) {
        return;
    }

    if (mScreenF68.x < 1.0f || mScreenF68.y < 1.0f) {
        return;
    }

    GXSetTexCopyDst(mScreenF68.x, mScreenF68.y, GX_TF_RGB565, GX_FALSE);
    GXSetTexCopySrc(mScreenF60.x, mScreenF60.y, mScreenF68.x, mScreenF68.y);
    GXSetColorUpdate(GX_FALSE);
    GXSetAlphaUpdate(GX_TRUE);

    GXColor clear = {};

    GXSetCopyClear(clear, 0xFFFFFF);
    GXCopyTex(mScreenBuffer, GX_TRUE);
    GXSetColorUpdate(GX_ENABLE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetDstAlpha(GX_FALSE, 0);
}

void MarioActor::writeBackScreenBox() const {
    if (!isUseScreenBox()) {
        return;
    }

    if (mScreenF68.x < 1.0f || mScreenF68.y < 1.0f) {
        return;
    }

    GXTexObj texture;
    TVec2f minimum;
    TVec2f maximum;
    TDDraw::setup(1, 0, 2);
    GXInitTexObj(&texture, mScreenBuffer, mScreenF68.x, mScreenF68.y, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GXLoadTexObj(&texture, GX_TEXMAP0);
    MR::convertFrameBufferPosToScreenPos(&minimum, mScreenF60);
    MR::convertFrameBufferPosToScreenPos(&maximum, mScreenF60 + mScreenF68 + TVec2f(0.99f, 0.99f));
    GXSetBlendMode(GX_BM_BLEND, GX_BL_INVDSTALPHA, GX_BL_DSTALPHA, GX_LO_NOOP);
    GXSetDstAlpha(GX_TRUE, 0);
    GXSetAlphaUpdate(GX_TRUE);
    GXPixModeSync();
    GXTexModeSync();
    maximum.x = static_cast< s16 >(static_cast< s16 >(static_cast< s32 >(maximum.x) + 1) & ~1);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(minimum.x, minimum.y, 0.9999f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(maximum.x, minimum.y, 0.9999f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(maximum.x, maximum.y, 0.9999f);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(minimum.x, maximum.y, 0.9999f);
    GXTexCoord2f32(0.0f, 1.0f);
    GXEnd();
    GXSetDstAlpha(GX_FALSE, 0);
    GXSetAlphaUpdate(GX_FALSE);
    TDDraw::close();
}

void MarioActor::calc1stPersonView() {
    f32 length = (mCameraPosition - mPosition).length();
    f32 minVal = 300.0f;
    f32 maxVal = 1000.0f;
    f32 val;

    if (length > maxVal) {
        val = 1.0f;
    } else if (length < minVal) {
        val = 0.0f;
    } else {
        val = 1.0f - (maxVal - length) / (maxVal - minVal);
    }

    u8 compareVal = 255.0f * val;

    if (compareVal == 0) {
        mFirstPersonDrawHidden1A5 = true;
        hideBeeFur();
    } else {
        mFirstPersonDrawHidden1A5 = false;
        updateAlphaDL(compareVal);
    }
}

void MarioActor::calcFogLighting() {
    Color8 fog(240, 16, 80, 0);

    if (!mMario->isStatusActive(MarioStatus_Paralyze) && isNeedDamageFog()) {
        f32 phase = ((_1A8 + 31) & 63) / 63.0f;

        if (phase > 0.5f) {
            phase = 1.0f - phase;
        }

        f32 wave = 2.0f * phase;
        f32 low = getConst().getTable()->mDamageFogLow;
        _1A4 = (low + wave * (getConst().getTable()->mDamageFogHigh - getConst().getTable()->mDamageFogLow)) / 255.0f;
    } else if (_1AA) {
        fog.mGXColor = mSwimFogColor.mGXColor;

        if (mSwimFogDisabled) {
            fog.g = 255.0f * MR::sin(JGeometry::TUtil< f32 >::PI() * (_1AA / 20.0f));
        }

        _1A4 = _1AC * MR::sin((_1AA / static_cast< f32 >(getConst().getTable()->mStarPieceFogTime)) * JGeometry::TUtil< f32 >::PI());
    } else if (mMario->_434) {
        fog.set(255, 255, 0, 0);
        _1A4 = 0.8f * MR::sin(((mMario->_434 / static_cast< f32 >(getConst().getTable()->mItemDashTimer)) * JGeometry::TUtil< f32 >::PI()) / 2.0f);
    } else {
        resetFog();
    }

    Color8 ambient = *MR::getLightAmbientColor(this);
    updateLightDL(ambient, Color8(255, 255, 255, 255), fog, _1A4);
}

void MarioActor::drawScreenBlend() const {
    if (mMario->_97C != nullptr && mMario->_97C->getBlurOffset() != 0.0f) {
        MR::drawFullScreenBlur(mMario->_97C->getBlurOffset());
    }

    mScreenBlur1074->draw();

    if (mBeeWallWalk) {
        TVec2f position;
        TVec2f opposite;
        TVec2f minimum;
        TVec2f maximum;
        TDDraw::project2D(&position, mMario->mShadowPos);
        opposite = position + TVec2f(100.0f, 100.0f);
        position = position + TVec2f(-100.0f, -100.0f);

        if (position.x >= opposite.x) {
            minimum.x = opposite.x;
            maximum.x = position.x;
        } else {
            minimum.x = position.x;
            maximum.x = opposite.x;
        }

        if (position.y >= opposite.y) {
            minimum.y = opposite.y;
            maximum.y = position.y;
        } else {
            minimum.y = position.y;
            maximum.y = opposite.y;
        }

        position = minimum + TVec2f(-10.0f, -10.0f);
        opposite = maximum + TVec2f(10.0f, 10.0f);
        TDDraw::setup(0, 0, 2);
        GXSetCullMode(GX_CULL_NONE);
        GXSetAlphaUpdate(GX_FALSE);
        GXSetColorUpdate(GX_TRUE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_INVDSTALPHA, GX_LO_NOOP);
        GXSetDstAlpha(GX_TRUE, 0);
        TDDraw::drawFillBox(position, opposite, 0);
        GXSetAlphaUpdate(GX_FALSE);
        MR::loadProjectionMtx();
        MR::loadViewMtx();
    }

    if (_BC4) {
        u32 color = 0xFF000080;

        if (((mMario->mFlags20.mWord >> 28) & 1U)) {
            color = 0x00004020;
        }

        TDDraw::setup(0, 1, 2);
        TDDraw::drawFillBox(TVec3f(0.0f, 0.0f, 0.0f),
                            TVec3f(static_cast< f32 >(MR::getScreenWidth()), static_cast< f32 >(MR::getScreenHeight()), 0.0f), color);
        TDDraw::close();
    }
}

void MarioActor::updateRandomTexture(f32 distance) {
    mRandomTextureIndex = 1 - mRandomTextureIndex;
    u8* pixel;
    u8* image = mMaskTextures[mRandomTextureIndex]->mImage;
    distance = 1.0f - distance / 1000.0f;
    f32 probability = MR::clamp(distance, 0.0f, 1.0f);

    for (u32 y = 0; y < 8; y++) {
        pixel = image + y * 8;

        for (u32 x = 0; x < 8; x++) {
            s32 alpha = *pixel >> 4;

            if (MR::getRandom() < probability) {
                alpha += 4;
            } else {
                alpha--;
            }

            s32 clamped = MR::clamp(alpha, 0L, 15L);
            *pixel = (clamped * 16) & 0xF0;
            pixel++;
        }
    }

    DCStoreRange(image, 64);
}

void MarioActor::calcSpinEffect() {
    f32 baseRadius = 30.0f;
    f32 radiusScale = 5.8f;
    mSpinRadius = 0.0f;
    mSpinHeight = 0.0f;
    for (s32 i = 0; i < static_cast<s32>(ARRAY_SIZE(mSpinNoise)); ++i) {
        mSpinNoise[i] = MR::getRandom();
    }

    if (mActionTable1080->selectAction("スピン回復エフェクト") != 1) {
        return;
    }

    if (!_945) {
        return;
    }

    if (_944) {
        return;
    }

    if (!_946) {
        return;
    }

    s32 frame = _946 - 9;

    if (frame <= 0 || frame > 15) {
        mSpinRadius = 0.0f;
        return;
    }

    if (frame < 10) {
        mSpinRadius = baseRadius + (frame / 10.0f) * (_945 * radiusScale);
        mSpinHeight = 120.0f;
        return;
    }

    mSpinRadius = baseRadius + _945 * radiusScale;
    mSpinHeight = 120.0f;
}

void MarioActor::drawSpinEffect() const {
    if (mSpinRadius == 0.0f) {
        return;
    }

    if (!isEnableNerveChange()) {
        return;
    }

    TDDraw::setup(0, 1, 0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    Mtx rotation;
    TVec3f center = _240 + mMario->mHeadVec * mSpinHeight;
    TVec3f direction(mMario->mFrontVec);
    TVec3f previous;
    PSMTXRotAxisRad(rotation, &mMario->mHeadVec, 0.09817477f);
    GXSetLineWidth(36, GX_TO_ZERO);

    for (u32 i = 0; i <= 64; i++) {
        TVec3f point(direction);
        point.setLength(mSpinRadius);
        point += mMario->mHeadVec * mSpinNoise[i] * 10.0f;

        if (i) {
            TDDraw::drawLine(center + previous, center + point, 0xFFFFFF20);
        }

        previous = point;
        PSMTXMultVecSR(rotation, &direction, &direction);
    }

    GXSetLineWidth(18, GX_TO_ZERO);

    for (u32 i = 0; i <= 64; i++) {
        TVec3f point(direction);
        point.setLength(mSpinRadius);
#ifdef GALAXY_NATIVE
        point += mMario->mHeadVec * (i == 64 ? readSpinJitterBoundary(mTornadoSensors[0]) : mSpinNoise[64 + i]) * 10.0f;
#else
        // The original reads one past mSpinNoise at i == 64 (the first mTornadoSensors word).
        point += mMario->mHeadVec * mSpinNoise[64 + i] * 10.0f;
#endif
        f32 wave = MR::sin((((_37C + i) & 31) / 32.0f) * JGeometry::TUtil< f32 >::PI());
        f32 square = wave * wave;
        u32 alpha = 160.0f * (square * square);

        if (i) {
            TDDraw::drawLine(center + previous, center + point, alpha | 0x40FF00 | (alpha << 24));
        }

        previous = point;
        PSMTXMultVecSR(rotation, &direction, &direction);
    }

    TDDraw::close();
}

void MarioActor::drawSphereMask() const {
    if (!mMario->isVisibleRecoveryWarpBubble()) {
        return;
    }

    TDDraw::setup(0, 1, 0);
    GXSetZScaleOffset(1.0f, 0.00001f);
    GXSetCullMode(GX_CULL_FRONT);
    GXSetZMode(GX_TRUE, GX_GREATER, GX_TRUE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    TVec3f position;
    getRealPos("Spine1", &position);
    TDDraw::drawSphere3D(position, 180.0f, 0xFFFFFF30, 16);
    GXSetZScaleOffset(1.0f, 0.0f);
}

void MarioActor::initDarkMask() {
    for (u32 texture = 0; texture < 2; texture++) {
        u8* image = mMaskTextures[texture]->mImage;

        for (u32 y = 0; y < 8; y++) {
            u8* pixel = image + y * 8;

            for (u32 x = 0; x < 8; x++) {
                pixel[x] = 0;
            }
        }
    }

    _1C3 = true;
    _1C4 = 0;
}

void MarioActor::updateDarkMask(u16 unused) {
    if (!_1C3) {
        return;
    }

    u8* previous = mMaskTextures[mRandomTextureIndex]->mImage;
    mRandomTextureIndex = 1 - mRandomTextureIndex;
    u8* image = mMaskTextures[mRandomTextureIndex]->mImage;
    u8 x = MR::getRandom(0L, 8L);
    u8 y = MR::getRandom(0L, 8L);
    s32 index = x + y * 8;
    image[index] = 0xF0;
    previous[index] = 0xF0;
    DCStoreRange(image, 64);
    _1C4++;
}

bool MarioActor::drawDarkMask() const {
    if (!_1C3) {
        return false;
    }

    TDDraw::setup(1, 0, 2);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetColorUpdate(GX_FALSE);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    mMaskTextures[mRandomTextureIndex]->load(GX_TEXMAP0);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 1, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetDstAlpha(GX_TRUE, 255);
    TVec3f spine;
    TVec2f center;
    TVec2f screenX;
    TVec2f screenY;
    TVec2f origin;
    TVec2f frameOrigin;
    TVec2f minimum;
    TVec2f maximum;
    getRealPos("Spine1", &spine);
    f32 radius = _1C4;

    if (radius >= 240.0f) {
        radius = 240.0f;
    }

    TVec3f position(mPosition);
    TDDraw::project2D(&center, position);
    TDDraw::project2D(&screenX, position + mCameraBasisX * radius);
    TDDraw::project2D(&screenY, position + mCameraBasisY * radius);
    u16 width = ((static_cast< u32 >((screenX - center).length()) + 3) & ~3) + 4;
    u16 height = ((static_cast< u32 >((screenY - center).length()) + 3) & ~3) + 4;
    TDDraw::project2D(&origin, position);
    origin.x -= width / 2;
    origin.y -= width / 2;
    MR::convertScreenPosToFrameBufferPos(&frameOrigin, origin);
    u32 left = static_cast< u32 >(frameOrigin.x);
    u32 top = static_cast< u32 >(frameOrigin.y);
    top &= ~1;
    left &= ~1;
    MR::convertFrameBufferPosToScreenPos(&minimum, TVec2f(left, top));
    MR::convertFrameBufferPosToScreenPos(&maximum, TVec2f(left + width, top + height));
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(minimum.x, minimum.y, 0.0f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(maximum.x, minimum.y, 0.0f);
    GXTexCoord2f32(4.0f, 0.0f);
    GXPosition3f32(maximum.x, maximum.y, 0.0f);
    GXTexCoord2f32(4.0f, 4.0f);
    GXPosition3f32(minimum.x, maximum.y, 0.0f);
    GXTexCoord2f32(0.0f, 4.0f);
    GXEnd();
    GXSetDstAlpha(GX_FALSE, 0);
    GXSetColorUpdate(GX_TRUE);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    TDDraw::close();
    return true;
}

// NONMATCHING: register moves and independent count/sensor-store scheduling.
void MarioActor::memorizeSensorThrow(HitSensor* pSensor) {
    _428[_468] = pSensor;
    LiveActor* host = MR::getSensorHost(pSensor);
    mThrowMemoRotations[_468] = host->mRotation - mRotation;

    _468++;
    Mario* player = mMario;
    _424 = nullptr;
    mCarryWobblePhase778 = 0.0f;
    _EF6 = 30;

    player->startHandy();

    if (pSensor == _470) {
        _470 = nullptr;
    }

    if (pSensor == _46C) {
        _46C = nullptr;
    }
}

// NONMATCHING: Reloads the unchanged Mario receiver instead of reusing its cached value.
bool MarioActor::tryThrow() {
    Mario* mario = mMario;
    if (((mario->getMovementStates().mLowWord & Mario::MovementStates::DebugMode) >> 9)) {
        return false;
    }

    if (_468 == 0) {
        return false;
    }

    if (_480) {
        return false;
    }

    if (mario->isSwimming()) {
        return false;
    }

    if (mMario->isStatusActive(MarioStatus_Bury)) {
        return false;
    }

    if (!isRequestThrow()) {
        return false;
    }

    HitSensor* pSensor = _428[_468 - 1];

    if (fn_803D0F60(pSensor)) {
        return true;
    }

    if (releaseThrowMemoSensor()) {
        mMario->mSwim->resetJet();
        mMarioAnim->changeThrowAnimation(pSensor);

        mVelocity.zero();
        _F1C = 30;

        if (isJumping()) {
            _38C = 1;
            jumpHop();
        } else {
            _38C = 15;
        }

        mMario->playSound("声投げ", -1);
        offTakingFlag();

        return true;
    }

    return false;
}

void MarioActor::tryReleaseWithMsg(u32 msg) {
    u32 maxIdx = _468;
    if (maxIdx == 0) {
        if (isAnimationRun("ひろいクイック")) {
            mMario->stopAnimationUpperForce();
            clearNullAnimation(0);
            offTakingFlag();
        }

        return;
    }

    for (s32 idx = 0; idx < maxIdx; idx++) {
        if (!_428[idx]->receiveMessage(msg, getSensor("Body"))) {
            fn_803D5A00(idx);
        } else {
            _428[idx] = nullptr;
            _468--;
        }
    }

    _474 = nullptr;

    if (_468 == 0) {
        mMario->stopAnimationUpperForce();
        mMario->mSwim->resetJet();
    }

    if (_480) {
        mMario->stopAnimationUpperForce();
        offTakingFlag();
    }

    clearNullAnimation(0);
}

// NONMATCHING: equivalent counted-loop lowering uses a general register instead of CTR.
void MarioActor::fn_803D5A00(u32 idx) {
    if (idx == 0) return;
    for (u32 i = 0; i < idx; ++i) {
        if (_428[i] == nullptr) {
            _428[i] = _428[idx];
            _428[idx] = nullptr;
            return;
        }
    }
}

void MarioActor::tryReleaseDirect(const HitSensor* pSensor) {
    const u32 count = _468;
    for (u32 idx = 0; idx < count; ++idx) {
        if (_428[idx] == pSensor) {
            _428[idx] = nullptr;
            --_468;
        } else {
            fn_803D5A00(idx);
        }
    }
    if (_474 == pSensor) {
        _474 = nullptr;
    }
    if (_468 == 0) {
        mMario->stopAnimationUpper(nullptr);
        mMario->mSwim->resetJet();
        offTakingFlag();
        clearNullAnimation(0);
    }
}

void MarioActor::trySetLockOnTarget(HitSensor* pSensor) {
    if (getCarrySensor() == pSensor) return;
    if (getCarrySensor() != nullptr && MR::getSensorHost(getCarrySensor()) != nullptr &&
        MR::getSensorHost(pSensor) != nullptr &&
        MR::getSensorHost(getCarrySensor()) == MR::getSensorHost(pSensor)) return;

    if (isJumping() && mMario->_430 == 5) {
        return;
    }

    TVec3f vec = *MR::getSensorPos(pSensor) - mPosition;
    if (vec.length() > 500.0f) {
        return;
    }

    MR::vecKillElement(vec, *mMario->getGravityVec(), &vec);

    if (MR::normalizeOrZero(&vec)) {
        return;
    }

    TVec3f front;
    getFrontVec(&front);
    MR::vecKillElement(front, *mMario->getGravityVec(), &front);

    if (MR::normalizeOrZero(&front)) {
        return;
    }

    f32 diffAngle = MR::diffAngleAbsFast(vec, front) * _180_PI;
    if (_46C == nullptr || diffAngle < _478) {
        _46C = pSensor;
        _478 = diffAngle;
    }
}

void MarioActor::tryCoinPull() {
    for (u32 idx = 0; idx < ARRAY_SIZE(mPulledSensors7C0); idx++) {
        if (mPulledSensors7C0[idx] == nullptr) {
            continue;
        }

        if (MR::isDead(MR::getSensorHost(mPulledSensors7C0[idx]))) {
            mPulledSensors7C0[idx] = nullptr;
        } else {
            tryCoinPullOne(mPulledSensors7C0[idx]);
        }
    }
}

void MarioActor::tryCoinPullInRush() {
    for (u32 idx = 0; idx < _7DC; idx++) {
        if (mTornadoSensors[idx] == nullptr) {
            continue;
        }

        if (MR::isDead(MR::getSensorHost(mTornadoSensors[idx]))) {
            mTornadoSensors[idx] = nullptr;
        } else {
            tryTornadoPull(mTornadoSensors[idx]);
        }
    }
}

void MarioActor::flushCoinPull() {
    for (u32 idx = 0; idx < ARRAY_SIZE(mPulledSensors7C0); idx++) {
        if (mPulledSensors7C0[idx] == nullptr) {
            continue;
        }

        if (MR::isDead(MR::getSensorHost(mPulledSensors7C0[idx]))) {
            mPulledSensors7C0[idx] = nullptr;
        } else {
            mConditionGroup->tryGetItem(mPulledSensors7C0[idx]);
            mPulledSensors7C0[idx] = nullptr;
        }
    }
}

void MarioActor::tryPullTrans(TVec3f* pVelocity, const TVec3f& rPosition) {
    TVec3f center(_240);
    TVec3f ground;
    getGroundPos(&ground);
    if (IsMarioSwimming()) {
        ground = center - mBodyUpDirection * 600.0f;
    } else if (((mMario->mMovementStates.mLowWord >> 30) & 1U)) {
        TVec3f offset = ground - center;
        f32 height = MR::vecKillElement(offset, mMario->_368, &offset);
        ground = center + mMario->_368 * height;
    }

    TVec3f foot;
    f32 projection = MR::getFootPoint(center, ground, rPosition, &foot);
    TVec3f inward = foot - rPosition;
    f32 radius = inward.length();
    if (radius > 600.0f) {
        radius = 600.0f;
    }

    TVec3f speed;
    speed.y = 400000.0f / (10000.0f + radius * radius);
    f32 height = MR::abs(projection * (ground - center).length());
    if (height < speed.y) {
        speed.y = height;
    }

    if (projection < 0.0f) {
        speed.y = -speed.y;
    }

    if (!IsMarioSwimming()) {
        speed.y = 0.4f * speed.y;
    }

    TVec3f axis = ground - center;
    MR::normalizeOrZero(&axis);
    MR::normalizeOrZero(&inward);
    TVec3f tangent;
    tangent.cross(axis, inward);
    speed.z = (0.01f + 0.04f * (600.0f - radius) / 600.0f) * (2.0f * (MR::pi() * radius)) * getConst().getTable()->mCoinPullAngleSpeedRatio;
    TVec3f nextPosition = rPosition + tangent * speed.z;
    TVec3f pull = foot - nextPosition;
    f32 nextRadius = pull.length();
    MR::normalizeOrZero(&pull);
    f32 pullSpeed = radius / 25.0f;
    if (_934) {
        pullSpeed += 50.0f;
    }

    pullSpeed *= getConst().getTable()->mCoinPullDistSpeedRatio;
    if (radius < pullSpeed) {
        pullSpeed = radius;
    }

    speed.x = pullSpeed + (nextRadius - radius);
    *pVelocity = pull * speed.x - axis * speed.y + tangent * speed.z;
}

// NONMATCHING: redundant Mario receiver reload and verified pooled literal addresses.
void MarioActor::tryReleaseBombTeresa() {
    if (_B94 == 0) {
        return;
    }

    _B94 = 0;

    mMario->stopAnimationUpper(nullptr);

    if (!((mMario->getMovementStates().mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        mMario->stopWalk();
        _38C = 5;
        mMario->_420 = 25;
    }

    mMario->changeAnimation("ハンマー投げリリース");
}

void MarioActor::initMaterialEffect() {
    MaterialEffectEntry* entry = cMaterialEffectTable;
    u32 entryCount = 0;
    while (entry->mName != nullptr) {
        for (u32 i = 0; i < 8; i++) {
            if (entry->mEffects[i] == nullptr) {
                continue;
            }

            if (!MR::isRegisteredEffect(this, entry->mEffects[i])) {
                mEffectKeeper->registerEffectWithoutSRT(entry->mEffects[i], nullptr);
            } else {
                MR::setEffectHostSRT(this, entry->mEffects[i], nullptr, nullptr, nullptr);
            }
        }

        entry->mCurrent = nullptr;
        entryCount++;
        entry->mEmitter = nullptr;
        entry++;
    }

    mMaterialEffectsFC8 = new HashSortTable(entryCount);
    entry = cMaterialEffectTable;
    for (int i = 0; i < entryCount; i++) {
        mMaterialEffectsFC8->addPtr(cMaterialEffectTable[i].mName, &cMaterialEffectTable[i], false);
    }

    mMaterialEffectsFC8->sort();
}

MultiEmitter* MarioActor::playMaterialEffect(const char* pName) {
    uintptr_t payload = 0;
    mMaterialEffectsFC8->search(pName, &payload);

    const s32 materialIndex = getFloorMaterialIndex(reinterpret_cast< MaterialEffectEntry* >(payload)->mFlag.mSelectionOptions);
    if (materialIndex == -1) {
        return nullptr;
    }

    const u32 materialOffset = static_cast< u32 >(materialIndex);
    const char* effectName = reinterpret_cast< MaterialEffectEntry* >(payload)->mEffects[materialOffset];
    if (effectName == nullptr) {
        return nullptr;
    }

    if (reinterpret_cast< MaterialEffectEntry* >(payload)->mFlag.mSelectionOptions == 1) {
        const char* current = reinterpret_cast< MaterialEffectEntry* >(payload)->mCurrent;
        if (current != nullptr && current == effectName) {
            return reinterpret_cast< MaterialEffectEntry* >(payload)->mEmitter;
        }
    }

    MultiEmitter* emitter = MR::emitEffect(this, effectName);

    const char* current = reinterpret_cast< MaterialEffectEntry* >(payload)->mCurrent;
    if (current != nullptr && current != reinterpret_cast< MaterialEffectEntry* >(payload)->mEffects[materialOffset]) {
        MR::deleteEffect(this, current);
    }

    reinterpret_cast< MaterialEffectEntry* >(payload)->mCurrent = reinterpret_cast< MaterialEffectEntry* >(payload)->mEffects[materialOffset];
    reinterpret_cast< MaterialEffectEntry* >(payload)->mEmitter = emitter;

    if (emitter != nullptr) {
        MarioEffect* marioEffect = mMarioEffect;
        if (marioEffect->_18) {
            emitter->setGlobalPrmColor(marioEffect->_1C.r, marioEffect->_1C.g, marioEffect->_1C.b, -1);
            emitter->setGlobalEnvColor(marioEffect->_20.r, marioEffect->_20.g, marioEffect->_20.b, -1);
        } else {
            emitter->setGlobalPrmColor(0xFF, 0xFF, 0xFF, -1);
            emitter->setGlobalEnvColor(0xFF, 0xFF, 0xFF, -1);
        }
    }

    return emitter;
}

void MarioActor::stopMaterialEffect(const char* pName) {
    uintptr_t payload = 0;
    mMaterialEffectsFC8->search(pName, &payload);

    if (reinterpret_cast< MaterialEffectEntry* >(payload)->mCurrent != nullptr) {
        MR::deleteEffect(this, reinterpret_cast< MaterialEffectEntry* >(payload)->mCurrent);
        reinterpret_cast< MaterialEffectEntry* >(payload)->mCurrent = nullptr;
        reinterpret_cast< MaterialEffectEntry* >(payload)->mEmitter = nullptr;
    }
}

// NONMATCHING: pooled literal offsets differ; instructions and material tables verified.
void MarioActor::initCommonEffect() {
    mCommonEffectCount = 0;
    mTimedCommonEffectCount = 0;
    mTimedCommonEffects = new SmokeEffectEntry*[8];

    SmokeEffectEntry* entry = cSmokeTable;
    while (entry->mName != nullptr) {
        const char* smokeEffects[] = {"SmokeSphere", "SmokeSphereLoop", "SmokeCircle", "SmokeCircleLoop"};
        const char* waterEffects[] = {"WaterSphere", "WaterSphereLoop", "WaterCircle", "WaterCircleLoop"};
        const char* flowerEffects[] = {"FlowerCircle", "FlowerSphereLoop", "FlowerCircle", "FlowerSphereLoop"};
        const char* sandEffects[] = {"SandCircle", "SandSphereLoop", "SandCircle", "SandSphereLoop"};
        const char* snowEffects[] = {"SnowSphere", "SnowSphereLoop", "SnowCircle", "SnowCircleLoop"};
        const char* smokeFollowEffects[] = {"SmokeSphereFollow", "SmokeSphereFollow1Time", "SmokeCircleFollow", "SmokeCircleLoopFollow"};
        const char* mudEffects[] = {"MudSphere", "MudSphereLoop", "MudCircle", "MudCircleLoop"};
        const char* honeyEffects[] = {"HoneySphere", "HoneySphereLoop", "HoneyCircle", "HoneyCircleLoop"};
        const char* cloudEffects[] = {"SmokeSphere", "SmokeSphereLoop", "MarioCloudLand", "SmokeCircleLoop"};

        s32 variant = 0;
        s32 useFollow = 0;

        switch (entry->mType.mCode) {
        case 6:
            useFollow = 1;
        case 0:
            variant = 0;
            break;
        case 7:
            useFollow = 1;
        case 1:
        case 2:
            variant = 1;
            break;
        case 8:
            useFollow = 1;
        case 3:
            variant = 2;
            break;
        case 9:
            useFollow = 1;
        case 4:
        case 5:
            variant = 3;
            break;
        }

        char name[0x80];
        for (u32 materialIndex = 0; materialIndex < 8; materialIndex++) {
            sprintf(name, "%s", entry->mName);

            const u8 flag2 = entry->mFlags.mPlacementFlags;
            if ((flag2 & 0x8) && materialIndex != 1) {
                continue;
            }

            const char* const* effectList;
            switch (materialIndex) {
            case 0:
                effectList = useFollow ? smokeFollowEffects : smokeEffects;
                break;
            case 1:
                effectList = waterEffects;
                break;
            case 2:
                effectList = flowerEffects;
                break;
            case 3:
                effectList = sandEffects;
                break;
            case 4:
                effectList = snowEffects;
                break;
            case 5:
                effectList = mudEffects;
                break;
            case 6:
                effectList = honeyEffects;
                break;
            case 7:
                effectList = cloudEffects;
                break;
            }

            if (flag2 != 0) {
                const char* effectName = effectList[variant];
                mEffectKeeper->registerEffectWithoutSRT(effectName, name + materialIndex);
            } else {
                const u8 flag0 = entry->mFlags.mAttachment;
                switch (flag0) {
                case 0: {
                    const char* effectName = effectList[variant];
                    const char* hostName = name + materialIndex;
                    mEffectKeeper->registerEffectWithoutSRT(effectName, hostName);
                    const TVec3f* rot = &mRotation;
                    const TVec3f* pos = &mPosition;
                    MR::setEffectHostSRT(this, hostName != nullptr ? hostName : effectName, pos, rot, nullptr);
                    break;
                }

                case 1:
                    ::addMarioEffectAtJoint(this, effectList[variant], "FootR", name + materialIndex);
                    break;
                case 2:
                    ::addMarioEffectAtJoint(this, effectList[variant], "HandR", name + materialIndex);
                    break;
                case 3:
                    ::addMarioEffectAtJoint(this, effectList[variant], "Spine1", name + materialIndex);
                    break;
                default:
                    break;
                }
            }

            if ((entry->mFlags.mHostFollowFlags & 0x1) != 0) {
                MR::getEffect(this, name + materialIndex)->forceFollowOn();
            }

            if ((entry->mFlags.mHostFollowFlags & 0x2) != 0) {
                MR::getEffect(this, name + materialIndex)->forceScaleOn();
            }
        }

        entry->mTimer = 0;
        switch (entry->mType.mCode) {
        case 5:
        case 2: {
            mTimedCommonEffects[mTimedCommonEffectCount] = entry;
            mTimedCommonEffectCount++;
            break;
        }

        default:
            entry->mInterval = 0;
            break;
        }

        entry->mHash = MR::getHashCode(entry->mName);
        entry->mEmitter = nullptr;
        entry->mMaterial = 0;
        entry++;
        mCommonEffectCount++;
    }
}

MultiEmitter* MarioActor::playCommonEffect(const char* pName) {
    if (mPlayerMode == 3) {
        return nullptr;
    }

    const u32 hash = MR::getHashCode(pName);
    for (u32 i = 0; i < mCommonEffectCount; i++) {
        SmokeEffectEntry* entry = &cSmokeTable[i];
        if (entry->mHash == hash) {
            MultiEmitter* emitter = nullptr;
            MarioEffect* marioEffect;
            s32 materialIndex = getFloorMaterialIndex(0);

            if (materialIndex == -1) {
                return nullptr;
            }

            if ((entry->mFlags.mPlacementFlags & 0x8) && materialIndex != 1) {
                return nullptr;
            }

            switch (entry->mType.mCode) {
            case 1:
            case 2:
            case 4:
            case 5:
                emitter = entry->mEmitter;
                break;
            case 7:
            case 9:
                if (materialIndex != 0) {
                    emitter = entry->mEmitter;
                }

                break;
            default:
                break;
            }

            if (emitter != nullptr && materialIndex != entry->mMaterial) {
                emitter->deleteEmitter();
                emitter = nullptr;
            }

            if (emitter == nullptr) {
                s32 needsFollow = 0;
                s32 needsSRT = 0;

                if (materialIndex == 0) {
                    if (entry->mType.mCode >= 6) {
                        const Triangle* tri = mMario->getGroundPolygon();
                        MtxPtr prevMtx = tri->getPrevBaseMtx()->toMtxPtr();
                        MtxPtr baseMtx = mMario->getGroundPolygon()->getBaseMtx()->toMtxPtr();

                        if (!MR::isSameMtx(baseMtx, prevMtx)) {
                            needsFollow = 1;
                        }

                        needsSRT = 1;
                    }
                }

                if (needsFollow) {
                    const Triangle* tri = mMario->getGroundPolygon();
                    JPABaseEmitter* baseEmitter = mMarioEffect->addRequest(pName + materialIndex, tri->getBaseMtx()->toMtxPtr());

                    if (baseEmitter != nullptr) {
                        baseEmitter->setGlobalScale(TVec3f(entry->mScale, entry->mScale, entry->mScale));
                    }
                } else {
                    if (needsSRT) {
                        const u8 hostFlag = entry->mFlags.mHostFollowFlags;
                        const TVec3f* hostPos = nullptr;
                        const TVec3f* hostRot = nullptr;
                        const TVec3f* hostScale = nullptr;

                        if (hostFlag & 0x1) {
                            hostPos = &mPosition;
                        }

                        if (hostFlag & 0x4) {
                            hostRot = &mRotation;
                        }

                        if (hostFlag & 0x2) {
                            hostScale = &mScale;
                        }

                        MR::setEffectHostSRT(this, pName + materialIndex, hostPos, hostRot, hostScale);
                    }

                    emitter = MR::emitEffect(this, pName + materialIndex);
                }
            }

            if (!(entry->mFlags.mHostFollowFlags & 0x2)) {
                if (emitter != nullptr) {
                    emitter->setGlobalScale(entry->mScale, -1);
                }
            }

            if (emitter != nullptr) {
                emitter->setRate(entry->mRate, -1);
            }

            if (entry->mFlags.mPlacementFlags & 0x1) {
                TVec3f dir(mMario->getWallNorm());
                TPos3f mtx;
                TVec3f rot;
                MR::makeMtxUpNoSupportPos(&mtx, dir, TVec3f(0.0f, 0.0f, 0.0f));
                MR::makeRTFromMtxPtr(nullptr, &rot, mtx.toMtxPtr(), true);

                if (emitter != nullptr) {
                    emitter->setGlobalRotationDegree(rot, -1);
                }

                if (emitter != nullptr) {
                    emitter->setGlobalTranslation(mPosition, -1);
                }
            } else {
                TVec3f dir(mMario->_368);
                TPos3f mtx;
                TVec3f rot;
                MR::makeMtxUpNoSupportPos(&mtx, dir, TVec3f(0.0f, 0.0f, 0.0f));
                MR::makeRTFromMtxPtr(nullptr, &rot, mtx.toMtxPtr(), true);
                PSMTXCopy(mtx.toMtxPtr(), mMarioEffect->_24.toMtxPtr());

                if (emitter != nullptr) {
                    emitter->setGlobalRotationDegree(rot, -1);
                }

                if (emitter != nullptr) {
                    emitter->setGlobalTranslation(mPosition, -1);
                }
            }

            if (entry->mFlags.mPlacementFlags & 0x2) {
                TVec3f trans(mMario->_4E8);
                if (emitter != nullptr) {
                    emitter->setGlobalTranslation(trans, -1);
                }
            }

            if (entry->mInterval != 0) {
                entry->mTimer = entry->mInterval;
            }

            switch (entry->mType.mCode) {
            case 1:
            case 2:
            case 4:
            case 5:
                entry->mEmitter = emitter;
                entry->mMaterial = static_cast< u8 >(materialIndex);
                break;
            case 7:
            case 9:
                if (materialIndex != 0) {
                    entry->mEmitter = emitter;
                }

                entry->mMaterial = static_cast< u8 >(materialIndex);
                break;
            default:
                break;
            }

            if (emitter != nullptr) {
                marioEffect = mMarioEffect;
                if (marioEffect->_18) {
                    emitter->setGlobalPrmColor(marioEffect->_1C.r, marioEffect->_1C.g, marioEffect->_1C.b, -1);
                    emitter->setGlobalEnvColor(marioEffect->_20.r, marioEffect->_20.g, marioEffect->_20.b, -1);
                } else {
                    emitter->setGlobalPrmColor(0xFF, 0xFF, 0xFF, -1);
                    emitter->setGlobalEnvColor(0xFF, 0xFF, 0xFF, -1);
                }

                if (materialIndex == 1) {
                    emitter->setDrawOrder(3);
                }
            }

            return emitter;
        }
    }

    return nullptr;
}

// NONMATCHING: pooled literal offsets differ; all registration calls and strings verified.
void MarioActor::initEffect() {
    mEffectFlagsWord = 0;
    initEffectKeeper(0xB8, "Mario", true);
    initCommonEffect();
    initMaterialEffect();

    MR::setEffectName(this, "SlipUp", "スリップアップ");
    MR::setEffectName(this, "WallSpark", "壁スパーク");
    MR::setEffectName(this, "Damage", "ダメージ");
    MR::setEffectName(this, "StepOn", "ふみつぶし");
    MR::setEffectName(this, "SuperSpinDriverEnd", "スーパースピンドライバ終了");
    MR::setEffectName(this, "BringBubble", "引き戻し泡");
    MR::setEffectName(this, "BringBubbleBreak", "引き戻し泡破裂");
    MR::setEffectName(this, "WaterSplashRight", "水はね右");
    MR::setEffectName(this, "WaterSplashLeft", "水はね左");
    MR::setEffectName(this, "WaterSplashSRight", "水はね右弱");
    MR::setEffectName(this, "WaterSplashSLeft", "水はね左弱");
    MR::setEffectName(this, "RailMoveWaterAreaSplash", "水エリア飛び込み飛沫");
    MR::setEffectName(this, "SpinLightBurst", "スピンライト消去");
    MR::setEffectName(this, "ElectricDamage", "ビリビリ");
    MR::setEffectName(this, "FireDamageSmoke", "炎ダメージ煙");
    MR::setEffectName(this, "BlueFireDamageSmoke", "炎ダメージ青煙");
    MR::setEffectName(this, "SpinLight", "スピンライト");
    MR::setEffectName(this, "IceMarioSpinLight", "アイススピン");
    MR::setEffectName(this, "FireMarioSpinLight", "ファイアスピン");
    MR::setEffectName(this, "BeeMarioSpinLight", "ハチスピン");
    MR::setEffectName(this, "BeeLuigiSpinLight", "ハチルイージスピン");
    MR::setEffectName(this, "Sweat", "いい汗");
    MR::setEffectName(this, "CarryStartShort", "ひろいクイック");
    MR::setEffectName(this, "AirRecover", "酸素回復");
    MR::setEffectName(this, "Invincible", "無敵中");
    MR::setEffectHostSRT(this, "無敵中", &mPosition, nullptr, nullptr);
    MR::setEffectName(this, "Ice", "アイス中");
    MR::setEffectHostSRT(this, "アイス中", &mPosition, nullptr, nullptr);
    MR::setEffectName(this, "FireBallThrow", "ファイアボール投げ");
    MR::setEffectName(this, "Throw", "こうら投げ");
    MR::setEffectName(this, "DieBuryBody", "砂埋まり体");
    MR::setEffectName(this, "DieBuryHand", "砂埋まり手");
    MR::setEffectName(this, "DieDeathMudBody", "泥埋まり体");
    MR::setEffectName(this, "DieDeathMudHand", "泥埋まり手");
    MR::setEffectName(this, "SpinLightConcentrate", "スピン許可");
    MR::setEffectName(this, "FooMarioHandGlowL", "フーマリオグロー左");
    MR::setEffectName(this, "FooMarioHandGlowR", "フーマリオグロー右");
    MR::setEffectName(this, "FooMarioBrakeLampL", "フーマリオブレーキ左");
    MR::setEffectName(this, "FooMarioBrakeLampR", "フーマリオブレーキ右");
    MR::setEffectName(this, "FooMarioEndL", "フーマリオ解除左");
    MR::setEffectName(this, "FooMarioEndR", "フーマリオ解除右");
    MR::setEffectName(this, "SpinScattarGrass", "芝生スピン");
    MR::setEffectName(this, "HorrorStart", "ホラーマリオ変身");
    MR::setEffectName(this, "HorrorEnd", "ホラーマリオ解除");
    MR::setEffectName(this, "YoshiDashJump", "ダッシュジャンプ煙");
    MR::setEffectName(this, "CloudMarioSmoke", "雲マリオ頭煙");
    MR::setEffectName(this, "SandSliderSmoke", "砂スライダー煙");
    MR::setEffectName(this, "SandStreamSmoke", "流砂足元砂煙");
    MR::setEffectName(this, "PetitPorterGlow", "プチポーター");
    MR::setEffectName(this, "WallHit", "壁ヒット");
    MR::setEffectName(this, "BarrierBound", "結界ヒット");
    MR::setEffectName(this, "WaterBound", "水壁ヒット");
    MR::setEffectName(this, "IceWallJump", "氷壁ジャンプ");
    MR::setEffectHostSRT(this, "氷壁ジャンプ", nullptr, nullptr, nullptr);
    MR::setEffectName(this, "CeilingCrash", "天井ヒット");
    MR::setEffectName(this, "Coin", "コイン");
    MR::setEffectName(this, "PunchHit", "パンチヒット");
    MR::setEffectName(this, "WaterRipple", "水波紋");
    MR::setEffectName(this, "SwimWaitRipple", "水面ウエイト波紋");
    MR::setEffectHostSRT(this, "水面ウエイト波紋", &mEffectTranslationFD0, &mEffectRotationFDC, nullptr);
    MR::setEffectName(this, "SwimSpinLight", "水泳スピン");
    MR::setEffectName(this, "CloudMarioSpinLight", "雲スピン");
    MR::setEffectName(this, "DiveBubble", "水面Ｚ沈降");
    MR::addEffect(this, "WaterColumnS");
    MR::setEffectName(this, "WaterColumnS", "水面ジャンプ水柱");
    MR::setEffectName(this, "SurfaceWave", "水面泳ぎ");
    MR::setEffectHostSRT(this, "水面泳ぎ", &mEffectTranslationFD0, &mEffectRotationFDC, nullptr);
    MR::setEffectName(this, "BodyBubble", "水中身体泡");
    MR::setEffectName(this, "MouthBubble", "水中口泡");
    MR::setEffectName(this, "SeaSmoke", "水底接触");
    MR::setEffectName(this, "Metamorphosis", "変身");
    MR::setEffectName(this, "MetamorphosisEnd", "変身解除");
    MR::setEffectName(this, "BeeFlowerPollen", "ハチ花くっつき");
    MR::setEffectHostSRT(this, "ハチ花くっつき", nullptr, nullptr, nullptr);
    MR::setEffectName(this, "SkateL", "スケート左");
    MR::setEffectName(this, "SkateR", "スケート右");
    MR::setEffectName(this, "IceJumpLand", "スケート着地");

    MR::setEffectHostSRT(this, "天井ヒット", nullptr, nullptr, nullptr);
    MR::setEffectHostSRT(this, "壁ヒット", nullptr, nullptr, nullptr);
    MR::setEffectHostSRT(this, "パンチヒット", nullptr, nullptr, nullptr);
    MR::setEffectHostSRT(this, "FlowerSpin", nullptr, nullptr, nullptr);
    MR::setEffectHostSRT(this, "水波紋", nullptr, nullptr, nullptr);
    MR::setEffectHostSRT(this, "結界ヒット", nullptr, nullptr, nullptr);
    MR::setEffectHostSRT(this, "水壁ヒット", nullptr, nullptr, nullptr);

    MR::setEffectName(this, "Blur", "ワープポッドブラー");
    MR::setEffectName(this, "SpinRing", "スピンリング");
    MR::setEffectName(this, "Vanish", "ブラックホール消滅");

    {
        const char* groupName = "雪煙レベル";
        mEffectKeeper->registerEffect("CrestGetFlyC", mFootRMatrix.toMtxPtr(), groupName, nullptr);
    }

    MR::setEffectName(this, "RunDust", "埃煙レベル");

    const TVec3f* scale = &mScale;
    const TVec3f* rot = &mRotation;
    const TVec3f* pos = &mPosition;

    {
        const char* groupName = "スイングフライ";
        mEffectKeeper->registerEffect("CommonPhotonDustCircle", pos, rot, scale, groupName);
    }

    {
        const char* groupName = "宇宙遊泳";
        mEffectKeeper->registerEffect("MarioSpaceDust", pos, rot, scale, groupName);
    }

    {
        const char* groupName = "ジャンプフェアリー";
        mEffectKeeper->registerEffect("CrestGetFlyC", pos, rot, scale, groupName);
    }

    {
        const char* groupName = "Bダッシュ";
        mEffectKeeper->registerEffect("MiniPandaSpinLoopLeg", pos, rot, scale, groupName);
    }

    {
        const char* groupName = "インダイレクトテスト";
        mEffectKeeper->registerEffect("IndTest", pos, rot, scale, groupName);
    }

    {
        const char* groupName = "ラケット消去";
        mEffectKeeper->registerEffect("TestMarioBaobabooPowerDown", mHipMatrix.toMtxPtr(), groupName, nullptr);
    }

    {
        const char* groupName = "チャージ完了";
        mEffectKeeper->registerEffect("TornadoChargeEnd", pos, rot, scale, groupName);
    }

    {
        const char* groupName = "チャージ中";
        mEffectKeeper->registerEffect("TornadoCharge", pos, rot, scale, groupName);
    }

    {
        const char* groupName = "チャージ切れ";
        mEffectKeeper->registerEffect("AppleHit", pos, rot, scale, groupName);
    }

    MR::setEffectName(this, "LandRubber", "特殊着地");
    MR::setEffectName(this, "DieDarkMatter", "ダークマター死亡");
    MR::setEffectName(this, "CloudMarioHipDropBlur", "雲尻落");
    MR::setEffectName(this, "CloudLuigiHipDropBlur", "雲尻落ルイージ");
    MR::setEffectName(this, "HopperMarioHipDropBlur", "ホッパー尻落");
    MR::setEffectName(this, "HopperLuigiHipDropBlur", "ホッパー尻落ルイージ");
    MR::setEffectName(this, "HipDropBlur", "尻落");
    MR::setEffectName(this, "HipDropBlurLuigi", "尻落ルイージ");
    MR::setEffectName(this, "GetAgain", "アイテム再ゲット");

    {
        const char* groupName = "パンチブラー左";
        mEffectKeeper->registerEffect("MarioPunchLBlur", pos, rot, scale, groupName);
    }

    MR::getEffect(this, "パンチブラー左")->forceFollowOn();
    {
        const char* groupName = "パンチブラー右";
        mEffectKeeper->registerEffect("MarioPunchRBlur", pos, rot, scale, groupName);
    }

    MR::getEffect(this, "パンチブラー右")->forceFollowOn();

    mEffectKeeper->finalizeSort();

    ModelObj* obj = new ModelObj("WaterColumn", "WaterColumn", nullptr, -2, -2, -2, false);
    mWaterColumn = obj;
    obj->kill();
}

bool MarioActor::isCommonEffect(const char* pName) const {
    if (static_cast< u8 >(pName[0]) == 0x8B) {
        if (static_cast< u8 >(pName[1]) == 0xA4) {
            return true;
        }
    }

    return false;
}

bool MarioActor::isMaterialEffect(const char* pName) const {
    if (static_cast< u8 >(pName[0]) == 0x91) {
        if (static_cast< u8 >(pName[1]) == 0xAE) {
            return true;
        }
    }

    return false;
}

void MarioActor::stopCommonEffect(const char* pName) {
    const u32 hash = MR::getHashCode(pName);
    for (u32 i = 0; i < mCommonEffectCount; i++) {
        SmokeEffectEntry* entry = &cSmokeTable[i];
        if (entry->mHash == hash) {
            if (entry->mEmitter != nullptr) {
                entry->mEmitter->deleteEmitter();
                entry->mEmitter = nullptr;
            }
        }
    }
}

// NONMATCHING: redundant Mario pointer reload before checkLvlA and pooled literal offsets.
void MarioActor::updateEffect() {
    s32 effectA = 0;
    s32 effectB = 0;
    s32 effectC = 0;

    if (mMario->mMovementStates.mLowBits.mBit27) {
        effectA = 1;
    }

    if (mMario->mMovementStates.mLowBits.mBit16 && mMario->mMovementStates.mLowBits.mBit30) {
        effectA = 1;
    }

    if (mMario->mMovementStates.mLowBits.mBit30 && mMario->_3F4 > 0.1f) {
        effectA = 1;
    }

    if (mMario->isStatusActive(MarioStatus_Flip)) {
        effectA = 1;
    }

    if (getMovementStates().mHighBits.mBit11 || getMovementStates().mHighBits.mBit10) {
        if (mMario->mWalkSpeed > 0.1f) {
            effectA = 1;
        }

        if (effectA != 0) {
            effectB = 1;
            effectA = 0;
        }
    }

    s32 specialSmoke = 0;
    if (mMario->_960 == 42) {
        if (mMario->mWalkSpeed > 0.1f) {
            specialSmoke = 1;
        }
        if (effectB != 0) {
            effectB = 0;
        }
        if (effectA != 0) {
            effectA = 0;
        }
    }

    if (_3C0 || mPlayerMode == 1 || mMario->isStatusActive(MarioStatus_Swim) || mMario->isStatusActive(MarioStatus_FpView) || _934) {
        effectA = 0;
        effectB = 0;
        effectC = 0;
        specialSmoke = 0;
    }

    if (effectA) {
        playEffect("共通砂煙レベル");
    } else if (!effectA) {
        if (mEffectFlags.mSmoke != 0) {
            stopEffect("共通砂煙レベル");
        }
    }

    if (effectB && mEffectFlags.mSnow == 0) {
        playEffect("雪煙レベル");
    } else if (!effectB && mEffectFlags.mSnow != 0) {
        stopEffect("雪煙レベル");
    }

    if (specialSmoke && mEffectFlags.mSpecialSmoke == 0) {
        playEffect("埃煙レベル");
    } else if (!specialSmoke && mEffectFlags.mSpecialSmoke == 1) {
        stopEffect("埃煙レベル");
    }

    mEffectFlags.mSmoke = effectA;
    mEffectFlags.mSnow = effectB;
    mEffectFlags._2 = effectC;
    mEffectFlags.mSpecialSmoke = specialSmoke;

    effectA = 0;
    if (getMovementStates().mHighBits.mBit28 && !getMovementStates().mLowBits.mBit7 && getMovementStates().mLowBits.mBit30) {
        effectA = 1;
    }

    if (effectA) {
        playEffect("共通スリップ坂");
    } else if (!effectA) {
        if (mEffectFlags.mSlopeSlip != 0) {
            stopEffect("共通スリップ坂");
        }
    }

    mEffectFlags.mSlopeSlip = effectA;

    s32 nearZero = 0;
    if (MR::isNearZero(mAppliedGravityVec)) {
        nearZero = 1;
    }

    if (!nearZero && mEffectFlags.mIsStationary) {
        playEffect("宇宙遊泳");
    } else if (nearZero && !mEffectFlags.mIsStationary) {
        stopEffect("宇宙遊泳");
    }

    mEffectFlags.mIsStationary = nearZero;

    if (!isJumping() && _934 == 0) {
        MR::deleteEffect(this, "ジャンプフェアリー");
    }

    mMarioEffect->doCubeEffect();

    effectC = 0;
    if (mMario->mMovementStates.mLowBits.mBit31) {
        if (mPlayerMode == 4 && mMario->checkLvlA() && mMario->_402 != 0 && ((_37C & 3) == 0)) {
            if (mMario->mVerticalSpeed < 300.0f) {
                effectC = 1;
            } else if (mMario->mSwim->_1B2 != 0) {
                f32 swimSpeed = mMario->mSwim->_1B8;
                if (swimSpeed < 300.0f) {
                    effectC = 1;
                }
            }
        }
    }

    if (effectC) {
        playEffectRT("属性ハチ風", mMario->_368, mMario->mShadowPos);
    } else {
        if (mEffectFlags.mBeeWind) {
            stopEffect("属性ハチ風");
        }
    }

    mEffectFlags.mBeeWind = effectC;

    u32 i = 0;
    while (i < mTimedCommonEffectCount) {
        SmokeEffectEntry* entry = mTimedCommonEffects[i];
        if (entry->mTimer != 0) {
            entry->mTimer--;
            if (entry->mTimer == 0) {
                stopEffect(entry->mName);
            }
        }

        i++;
    }
}

void MarioActor::stopSpinTicoEffect(bool force) {
    _948 = 0;
    _94C = 0;
    _946 = 0;

    if (force) {
        stopEffectForce("スピンリング");
    } else {
        stopEffect("スピンリング");
    }
}

void MarioActor::initSearchLight() {
    mSearchLight = new MarioSearchLight(this);
    mSearchLight->initWithoutIter();
    new DrawAdaptor(MR::Functor(this, &MarioActor::drawSearchLight), (MR::DrawType)0x38);
}

void MarioActor::initThrowing() {
    mSearchLightThrowPos = new FixedPosition(this, "HandR", TVec3f(13.19f, 41.57f, 19.02f), TVec3f(-3.63f, -41.69f, 15.76f));
}

void MarioActor::updateSearchLight() {
    if (mSearchLight == nullptr) {
        return;
    }

    if (mMario->isStatusActive(MarioStatus_Swim) && mMario->mSwim->fn_80400910()) {
        if (MR::isDead(mSearchLight)) {
            mSearchLight->appear();
            MR::hideModel(mSearchLight);
            MR::emitEffect(mSearchLight, "Glow");
        }
    } else {
        mSearchLight->kill();
    }
}

void MarioActor::updateThrowing() {
    if (mSearchLightThrowPos == nullptr) {
        return;
    }

    mSearchLightThrowPos->calc();

    if (mSearchLight == nullptr) {
        return;
    }

    TVec3f trans;
    mSearchLightThrowPos->mMtx.getTrans(trans);

    TVec3f& position = mSearchLight->mPosition;
    position = trans + mVelocity;

    mSearchLightThrowPos->copyRotate(&mSearchLight->mRotation);

    MR::calcAnimDirect(mSearchLight);
}

void MarioActor::calcViewSearchLight() {
    if (mSearchLight == nullptr) {
        return;
    }

    if (!MR::isDead(mSearchLight)) {
        J3DModelX* model = ::getModelX(mSearchLight);

        // The search light is has three scaled cone "shells" to fake a blur effect.
        Mtx modelMdx;
        MR::multMtx(modelMdx, model->getBaseTRMtx(), j3dSys.getViewMtx());

        Mtx invBaseMtx;
        PSMTXInverse(model->getBaseTRMtx(), invBaseMtx);

        model->viewCalc3(0, nullptr);

        for (u32 i = 0; i < ARRAY_SIZE(mConst->getTable()->mSearchLightBlurScale); i++) {
            Mtx scaledMtx;
            f32 scale = mConst->getTable()->mSearchLightBlurScale[i];
            PSMTXScale(scaledMtx, scale, scale, scale);

            MR::multMtx(scaledMtx, scaledMtx, modelMdx);
            MR::multMtx(j3dSys.getViewMtx(), invBaseMtx, scaledMtx);

            model->viewCalc3(i + 1, nullptr);
        }

        MR::loadViewMtx();
        mSearchLightTimer++;
    } else {
        mSearchLightTimer = 0;
    }
}

void MarioActor::drawSearchLight() const {
    TVec2f a, b;
    if (mSearchLight == nullptr || !this->mMario->isStatusActive(6) || !this->mMario->mSwim->fn_80400910()) {
        return;
    }
    if (!MR::isSystemTalking() && !this->mMario->mSwim->fn_80400930()) {
        this->fn_803D82C0(&a, &b);
        this->fn_803D8400(&a, &b);
        if (this->fn_803D8490()) {
            this->fn_803D86B0(&a, &b);
        }
    }
}

// NONMATCHING: vector temporary stack slots and literal identities.
void MarioActor::fn_803D82C0(TVec2f* a, TVec2f* b) const {
    TVec3f thr;
    this->getThrowVec(&thr);
    TDDraw::project2D(a, this->mPosition);
    TDDraw::project2D(b, mPosition + thr * 10000.0f);
    TVec2f lo;
    TVec2f hi;
    if (a->x >= b->x) {
        lo.x = b->x;
        hi.x = a->x;
    } else {
        lo.x = a->x;
        hi.x = b->x;
    }
    if (a->y >= b->y) {
        lo.y = b->y;
        hi.y = a->y;
    } else {
        lo.y = a->y;
        hi.y = b->y;
    }
    *a = lo + TVec2f(-10.0f, -10.0f);
    *b = hi + TVec2f(10.0f, 10.0f);
}

void MarioActor::fn_803D8400(TVec2f* a, TVec2f* b) const {
    TDDraw::setup(0, 0, 2);
    GXSetCullMode(GX_CULL_NONE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetColorUpdate(GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_DSTALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetDstAlpha(GX_TRUE, 0);
    TDDraw::drawFillBox(*a, *b, 0);
}

// NONMATCHING: Equivalent model/material saved-register allocation.
s32 MarioActor::fn_803D8490() const {
    TDDraw::setup(0, 1, 1);
    J3DModelX* m = static_cast<J3DModelX*>(MR::getJ3DModel(mSearchLight));
    J3DMaterial* mat = m->getModelData()->getMaterialNodePointer(0);
    if (!m->simpleDrawSetup(mat)) {
        return 0;
    }
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetColorUpdate(GX_FALSE);
    GXSetDstAlpha(GX_FALSE, 0);
    u32 t = mSearchLightTimer;
    f32 f = 1.0f;
    if (t < 0x1E) {
        f = 0.0f;
    } else if (t < 0xB4) {
        f = (f32)(t - 0x1E) / 150.0f;
    }
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    static const GXColorS10 white = {255, 255, 255, 0};
    GXColorS10 c = white;
    c.a = (u8)(s32)(f * (f32)mConst->getTable()->mSearchLightColor[3]);
    GXSetTevColorS10(GX_TEVREG0, c);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_NOOP);
    GXSetCullMode(GX_CULL_BACK);
    m->setDrawView(0);
    m->simpleDrawShape(mat);
    GXSetCullMode(GX_CULL_FRONT);
    GXSetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ONE, GX_LO_NOOP);
    m->simpleDrawShape(mat);
    return 1;
}

// NONMATCHING: registers, pooled color/bias addressing, equivalent color packing and redundant target branches.
void MarioActor::fn_803D86B0(TVec2f* a, TVec2f* b) const {
    J3DMaterial* mat;
    J3DModelX* m = static_cast<J3DModelX*>(MR::getJ3DModel(mSearchLight));
    u32 t = mSearchLightTimer;
    f32 f = 1.0f;
    mat = m->getModelData()->getMaterialNodePointer(0);
    if (t < 0x1E) {
        f = 0.0f;
    } else if (t < 0xB4) {
        f = (f32)(t - 0x1E) / 150.0f;
    }
    TDDraw::setup(0, 1, 2);
    GXSetCullMode(GX_CULL_NONE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetColorUpdate(GX_TRUE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_DSTALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetDstAlpha(GX_TRUE, 0);
    const MarioConstTable* tbl = mConst->getTable();
    u32 red = tbl->mSearchLightColor[0];
    u32 blue = tbl->mSearchLightColor[2];
    u32 green = tbl->mSearchLightColor[1];
    u32 col = (red << 24) | (green << 16) | (blue << 8);
    TDDraw::drawFillBox(*a, *b, col);
    GXSetDstAlpha(GX_FALSE, 0);
    GXSetAlphaUpdate(GX_FALSE);
    TDDraw::setup(0, 1, 1);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 4, GX_AOP_AND, GX_ALWAYS, 0);
    bool ok = m->simpleDrawSetup(mat);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXColorS10 c = {255, 255, 255, 0};
    c.a = (u8)(s32)(f * (f32)(mConst->getTable())->mSearchLightAlphaLevel);
    GXSetTevColorS10(GX_TEVREG0, c);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_A0, GX_CA_RASA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    if (ok) {
        m->simpleDrawShape(mat);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A1, GX_CA_A0, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
        u32 i = 0;
        do {
            const GXColorS10 c1 = {255, 255, 255, 0};
            const GXColorS10 c2 = {255, 255, 255, 4};
            m->setDrawView(i + 1);
            GXColorS10 d = c1;
            d.a = (u8)(s32)(f * (f32)mConst->getTable()->mSearchLightBlurAlpha[i]);
            GXSetTevColorS10(GX_TEVREG0, d);
            GXSetTevColorS10(GX_TEVREG1, c2);
            m->simpleDrawShape(mat);
            i++;
        } while (i < 3);
    }
}

void MarioActor::fn_803D8A30(s32 a, TVec3f& v) {
    MarioSearchLight* p = mSearchLight;
    if (p == nullptr) {
        return;
    }
    if (a != 0) {
        p->_A8 = 1;
        p->_AC = v;
        return;
    }
    p->_A8 = 0;
}

void MarioActor::fn_803D8A70(s32 mode) {
    u16 t = this->_3D8;
    if (t != 0 && mPlayerMode == 0) {
        if (mode != mPlayerMode && this->_3D6 == 6) {
            if (t > 0x1A) {
                (mTeresaModel->mActor)->appear();
            }
            this->_483 = 1;
            this->updateHand();
            this->updateFace();
            return;
        }
        if (this->_3D6 == 6) {
            (mTeresaModel->mActor)->kill();
            this->_483 = 0;
        }
    }
}

bool MarioActor::fn_803D8B30(HitSensor* s) const {
    HitSensor* hs = _924;
    if (hs == nullptr) {
        return true;
    }
    LiveActor* h = MR::getSensorHost(hs);
    if (h == nullptr) {
        return true;
    }
    if (MR::getSensorHost(s) == nullptr) {
        return true;
    }
    if (MR::isEqualString(h->mName, "たまころ")) {
        return s->isType(0x61);
    }
    return true;
}

void MarioActor::fn_803D8BD0(TVec3f* a, TVec3f* b) {
    f32 ma = PSVECMag((Vec*)a);
    if (PSVECMag((Vec*)&mVelocity) < ma) {
        if (a->dot(*this->mMario->getGravityVec()) < -0.0f) {
            TVec3f k;
            MR::vecKillElement(mVelocity, *mMario->getGravityVec(), &k);
            if (MR::isNearZero(k, 0.001f)) {
                MR::vecKillElement(*a, *this->mMario->getGravityVec(), &k);
            }
            k.setLength(PSVECMag((Vec*)a));
            this->mMario->push(k);
            if ((this->mMario)->_3BC <= 2) {
                f32 m2 = PSVECMag((Vec*)b);
                f32 v = m2;
                if (m2 > 10.0f) {
                    v = 10.0f;
                }
                (this->mMario)->mJumpVec += -*this->mMario->getGravityVec() * v;
            }
        }
    } else {
        TVec3f n((this->mMario)->_8F8);
        MR::normalizeOrZero(&n);
        TVec3f tmp;
        f32 f1 = MR::vecKillElement(*a, n, &tmp);
        f32 f2 = MR::vecKillElement(mVelocity, n, &tmp);
        if (PSVECMag((Vec*)&mVelocity) > 20.0f && f1 < 0.5f * f2) {
            if (this->mMario->isAnimationRun("坂すべり下向きあおむけ")) {
                this->mMario->push((this->mMario)->mFrontVec * 5.0f);
            } else if (this->mMario->isAnimationRun("坂すべり上向きうつぶせ")) {
                this->mMario->push((this->mMario)->mFrontVec * -5.0f);
            }
            (this->mMario)->mFlags14.mHighWord |= 0x20000000;
        }
    }
}

void MarioActor::fn_803D8E30(TVec3f* v) {
    if (v->dot(*this->mMario->getGravityVec()) < -40.0f) {
        TVec3f start = this->mPosition - *this->mMario->getGravityVec() * 100.0f;
        TVec3f dir(*this->mMario->getGravityVec());
        dir.scale(200.0f);
        Triangle* tri = this->mMario->fn_803EFA20();
        TVec3f hit;
        if (this->mMario->fn_803B2770(&hit, tri, start, dir)) {
            TVec3f d(hit);
            d -= this->mPosition;
            if (!(this->mMario->getGravityVec()->dot(d) >= -5.0f)) {
                CollisionParts* h = tri->mParts;
                if (h != nullptr && h->mStationaryFrames == 0) {
                    if (((this->mMario)->mMovementStates.mHighWord & 3) == 1) {
                        return;
                    }
                    this->mPosition = hit;
                    this->mMario->mPosHolder->fn_80405F90(this->mPosition);
                    this->mMario->stopJump();
                    this->mMario->changeAnimation("基本", "落下");
                    this->mMario->updateGroundInfo();
                }
            }
        }
    } else if (v->dot(*this->mMario->getGravityVec()) > 40.0f) {
        if (this->mMario->_4C8->isValid()) {
            if (MR::isSensorPressObj(this->mMario->_4C8->getSensor()) || MR::getFloorCodeIndex((this->mMario)->_4C8) == 0x1D ||
                MR::getFloorCodeIndex((this->mMario)->_4C8) == 0x27) {
                (this->mMario)->mDrawStates.mWord |= 2;
            }
        } else {
            u32 n = (this->mBinder)->mPlaneNum;
            u32 i = 0;
            while (i < n) {
                const HitInfo* p = this->mBinder->getPlane(i);
                if (MR::isSensorPressObj(p->mParentTriangle.getSensor())) {
                    (this->mMario)->mDrawStates.mWord |= 2;
                }
                i++;
            }
        }
        TVec3f d((this->mMario)->mShadowPos);
        d -= this->mPosition;
        if (d.dot(*this->mMario->getGravityVec()) < 0.0f && !this->fn_803D9370(this->mMario->_45C->mParts, v)) {
            this->mPosition = (this->mMario)->mShadowPos;
            (this->mMario)->mJumpVec.zero();
            (this->mMario)->_148.zero();
            Mario* m = this->mMario;
            if (m->_5FC == nullptr && (((m->mDrawStates.mWord >> (1)) & 1) || ((m->mPrevDrawStates.mWord >> (1)) & 1))) {
                TVec3f nv(*v);
                MR::normalizeOrZero(&nv);
                mPressNormal6D8 = nv;
                this->setPress(0, 0);
                this->_3B0 = 0.1f;
            }
        }
    }
}

void MarioActor::fn_803D9170(TVec3f* v) {
    Triangle* tri;
    u32 i;
    s32 hit;
    u32 n = (this->mBinder)->mPlaneNum;
    hit = 0;
    i = 0;
    while (i < n) {
        tri = (Triangle*)this->mBinder->getPlane(i);
        if (MR::isSensorPressObj(tri->getSensor())) {
            if ((this->mMario)->_368.dot(*tri->getNormal(0)) > 0.0f) {
                if ((this->mMario)->_72C < 200.0f) {
                    hit = 1;
                } else if (tri->getNormal(0)->dot(*v) < 0.0f) {
                    hit = 1;
                } else {
                    hit = 0;
                }
            } else {
                f32 d = tri->getNormal(0)->dot(*v);
                if (d < 0.0f) {
                    hit = 1;
                } else if (this->mMario->_5FC != nullptr) {
                    if (!MR::isWallCodeNoAction(tri) && !this->mMario->isOnimasuBinderPressSkip()) {
                        mPressNormal6D8 = (this->mMario)->_368;
                        (this->mMario)->mFlags14.mLowWord &= 0xFFFFFFFB;
                        *(this->mMario)->_4C8 = *tri;
                        this->setPress(2, 0);
                        this->_3B0 = 0.1f;
                    }
                    hit = 1;
                }
            }
        }
        i++;
    }
    if (hit != 0) {
        TVec3f tmp;
        f32 k = MR::vecKillElement(*v, (this->mMario)->_368, &tmp);
        TVec3f s((this->mMario)->_368);
        s.scale(k);
        this->mPosition -= s;
    }
}


const TVec3f& MarioActor::getGravityVector() const {
    return *mMario->getGravityVec();
}


void MarioActor::updatePunching() {
    if (_944) {
        bool canSpinring = true;
        _944--;
        _945++;
        if (!_946 && !((getMovementStates().mHighWord >> 20) & 1U)) {
            canSpinring = false;
        }

        if (canSpinring && !mMario->isSwimming() && !_944 && mActionTable1080->selectAction("スピン回復エフェクト") == true) {
            playEffect("スピンリング");
        }

        bool canPunch = isInPunchTimerRange();
        if (!mMario->isSwimming()) {
            canPunch = true;
        }

        if (canPunch) {
            const Mario::MovementStates& moveStates = getMovementStates();
            if (((moveStates.mLowWord >> 23) & 1U) || ((moveStates.mLowWord >> 5) & 1U) || ((moveStates.mLowWord >> 6) & 1U)) {
                mMario->tryWallPunch();
            }
        }
    }

    if (mMario->isAnimationRun("ハンマー投げリリース") && ((mMario->getMovementStates().mLowWord >> 30) & 1U) && !_38C && !mMario->_420 && mMario->Mario::isStickOn()) {
        mMario->stopAnimation(nullptr);
    }
}


void MarioActor::decLifeSmall() {
    decLife(0);
}


void MarioActor::forceGameOverBlackHole() {
    if (!isEnableNerveChange()) {
        return;
    }

    setNerve(GET_NERVE(MarioActor, MarioActorNrvGameOverBlackHole));
}


// native: defined in DrawUtil.cpp (DOL split owner); misplaced copy
#ifndef GALAXY_NATIVE
namespace MR {
    s32 getFrameBufferWidth() {
        return JUTGetVideoManager()->getRenderMode()->fbWidth;
    }
};
#endif
  // namespace MR


// ---- MarioActorBlackHole.cpp ----


// ---- MarioActorCamera.cpp ----


bool MarioActor::isFastDrop() const {
    return ((mMario->getMovementStates().mLowWord >> 20) & 1U);
}


// ---- MarioActorClap.cpp ----


// ---- MarioActorDefensiveMsg.cpp ----

void MarioActorDefensiveMsg_FORCE_MATCH_SDATA2() {
    (void)0.001f;
    (void)15.0f;
    (void)30.0f;
    (void)10.0f;
    (void)20.0f;
    (void)1.1f;
    (void)100.0f;
}


// ---- MarioActorDraw.cpp ----


void MarioActorDraw_FORCE_MATCH_SDATA2_2() {
    (void)0.00001f;
}


// ---- MarioActorEye.cpp ----


// ---- MarioActorGameOver.cpp ----


// ---- MarioActorGravity.cpp ----

void MarioActorGravity_FORCE_MATCH(TVec3f& rVec, f32 a, f32 b, f32 c) {
    rVec *= a;
    rVec *= b;
    rVec *= c;
}


void MarioActorGravity_FORCE_MATCH_SDATA2() {
    (void)1.0f;
    (void)0.0f;
    (void)0.5f;
    (void)2.0f;
    (void)100.0f;
    (void)0.001f;
    (void)0.9f;
    (void)(MR::pi() / 6.0f);
    (void)(MR::pi() * 0.4f);
    (void)(MR::pi() * (2.0f / 3.0f));
    (void)0.75f;
    (void)70.0f;
    (void)50.0f;
    (void)59.0f;
    (void)0.1f;
    (void)15.0f;
    (void)160.0f;
    (void)80.0f;
    (void)200.0f;
    (void)(MR::pi() / 36.0f);
    (void)0.3f;
}


const TVec3f& MarioActor::getAirGravityVec() const {
    return mMario->getAirGravityVec();
}


void MarioActor::updateBeeModeGravity(TVec3f& rVec) {
    u8 alpha = mBeeWallWalk;

    if (_9F2 != 0) {
        _9F2--;
    }

    if (_9F2 != 0 || (!checkBeeCeilStick(rVec) && !checkBeeWallStick(rVec) && !checkBeeFloorStick(rVec))) {
        updateBeeStickMode(rVec);
    }

    if (mBeeWallWalk == 5) {
        MR::vecBlendSphere(_240, rVec, &rVec, 0.1f);
        return;
    }

    if (mBeeWallWalk != 0) {
        MR::vecBlendSphere(_240, rVec, &rVec, 0.3f);
        return;
    }

    if (alpha != 0) {
        _9F2 = 60;
    }
}


bool MarioActor::checkBeeCeilStick(TVec3f& rVec) {
    if ((mMario->isCeiling() || ((getDrawStates().mWord >> 10) & 1U)) && mBeeWallWalk == 0 && _9F2 == 0) {
        bool out = false;
        if (((getDrawStates().mWord >> 10) & 1U)) {
            out = true;
        } else {
            const char* pWallCodeString = MR::getWallCodeString(mMario->_4C8);
            if (pWallCodeString != nullptr && strcmp(pWallCodeString, "Fur") == 0) {
                out = true;
            }
        }

        if (out) {
            Triangle triangle = Triangle();
            TVec3f vec;
            if (MR::getFirstPolyOnLineToMap(&vec, &triangle, mPosition, (-_240 * 200.0f))) {
                entryWallWalkMode(vec, *(triangle.getNormal(0)));
            }
        }
    }

    return false;
}


// ---- MarioActorHand.cpp ----


void MarioActor::changeHandMaterial() {
}


// ---- MarioActorInit.cpp ----


// ---- MarioActorMatrix.cpp ----

static MatrixMap morph_bitmap[] = {{"ジャンプ系バインド", 0x1111100F},
                                   {"スピンアタック", 0x11231041},
                                   {"コイン引っ張り", 0x11110001},
                                   {"カメ持ち", 0x1111000F},
                                   {"水解除", 0x00001110},
                                   {"ダメージ解除", 0x00001100},
                                   {"ノーダメージ解除", 0x00000100},
                                   {"アッパーパンチ", 0x11110101},
                                   {"スピン回復エフェクト", 0x10002000},
                                   {""}};


static MatrixSelectList select_list[] = {{8, "ノーマル", "メタル", "ファイア", "アイス", "ハチ", "ホッパー", "テレサ", "フー"},
                                         {2, "×NG", "OK○"},
                                         {6, "×NG", "OK○", "FIRE", "ICE", "消える", "回転のみ"},
                                         {2, "×NG", "OK○"},
                                         {2, "×NG", "OK○"},
                                         {2, "×NG", "OK○"},
                                         {2, "×NG", "OK○"},
                                         {2, "×NG", "OK○"},
                                         {3, "×NG", "OK○", "頭突き"},
                                         {3, "×NG", "エフェクト＋音", "音のみ"}};


static MatrixMap auto_bind_bitmap[] = {{"ウォータープレッシャーの弾", 0x10112222},
                                       {"ポール", 0x11111000},
                                       {"グリーンスーパースピンドライバー", 0x11111222},
                                       {"スーパースピンドライバー", 0x11111222},
                                       {"スピンドライバ", 0x11111222},
                                       {"Gキャプチャー", 0x11111222},
                                       {"移動用砲台", 0x11111222},
                                       {"ウォーターロード", 0x11112222},
                                       {"土管", 0x33333103},
                                       {"噴水（大）", 0x00002000},
                                       {"オオアワ[共有]", 0x11112222},
                                       {"スイングロープ", 0x11111000},
                                       {"空中ブランコ", 0x11111000},
                                       {""}};


static MatrixSelectList auto_bind_selecterlist[] = {{8, "ノーマル", "メタル", "ファイア", "アイス", "ハチ", "ホッパー", "テレサ", "フー"},
                                                    {4, "×スルー", "OK○", "変身解除", "移動中不可"}};


static MatrixMap auto_efx_bitmap[] = {
    {"わたげ発生源", 0x20000000},
    {"杭スイッチ", 0x10000000},
    {"ダミー敵"},
    {"ザコカメムシ", 0x18000400},
    {"子連れカメムシ", 0x18000400},
    {"細野用マップパーツ75", 0x04000000},
    {"幽霊船たいまつ", 0x01000000},
    {"燭台", 0x01000000},
    {"燭台(テレサマンション)", 0x01000000},
    {"燭台(アイスボルケーノ)"},
    {"バブル", 0x01000000},
    {"カボクリ", 0x11000000},
    {"エイ", 0x10800000},
    {"ポール", 0x025A0000},
    {"ポール（鉄骨）", 0x025A0000},
    {"オニマス(ピボット)", 0x00200000},
    {"オニマス", 0x00200000},
    {"移動用砲台", 0x00040000},
    {"スーパースピンドライバー", 0x00040000},
    {"スピンドライバ", 0x00040000},
    {"伸び植物", 0x00090000},
    {"空中ブランコ", 0x00080000},
    {"つる花", 0x00080000},
    {"土管", 0x0040A800},
    {"タイマーピースブロック", 0x00004000},
    {"ゴム星", 0x00001000},
    {"電撃レール"},
    {"電撃レール点"},
    {"ビリキュー"},
    {"宝箱"},
    {"宝箱(空っぽ)"},
    {"宝箱ゴールド(空っぽ)"},
    {"宝箱(ライフＵＰキノコ)"},
    {"ひび割れ宝箱"},
    {"アイス床"},
    {"草"},
    {"花"},
    {"青い花"},
    {"トルネード小石"},
    {"サークルシェル"},
    {"サークルストロベリー"},
    {"タマコロチュートリアル"},
    {"コイン花"},
    {"隠れアイテム"},
    {"スターピーススター"},
    {"レバースイッチ"},
    {"クリスタルケージ[小]"},
    {"クリスタルケージ[中]"},
    {"クリスタルケージ[大]"},
    {"ファイアバー"},
    {"電撃ビリビリレール"},
    {"移動電撃ビリビリレール"},
    {"スターピース"},
    {"アイテムバブルピース"},
    {"スターピースディレクターピース"},
    {"フォロースターピース"},
    {"グループスターピース"},
    {"スターピースマザーピース"},
    {"トゲ植物"},
    {"トゲ植物(空中)"},
    {"陸ウニゾー"},
    {"ビリビリボール"},
    {"飾り足"},
    {"モグ"},
    {"蝶"},
    {"タコヘイ墨"},
    {"マグナムキラー", 0x10000000},
    {"雪だるま", 0x10000000},
    {"よわブロック", 0x10000000},
    {"ジャンプ台", 0x10000000},
    {"スピニングボックス", 0x10000000},
    {"杭スイッチビッグ", 0x10000000},
    {"ヒビ石", 0x10000000},
    {"メカクッパパーツローラーA", 0x00004000},
    {""},
};


static MatrixSelectList auto_efx_selecterlist[] = {{19,
                                                    "スピンヒット振動",
                                                    "スピーカー音",
                                                    "スピンキャッチ許可",
                                                    "SHDホーミング許可(地形)",
                                                    "地形移動無視",
                                                    "ハチ出立",
                                                    "乗換センサ",
                                                    "青い炎エフェクト",
                                                    "水柱エフェクト",
                                                    "ダメージ中RUSH",
                                                    "ナナメつぶし許可",
                                                    "カメを捨てない",
                                                    "ハチ回復",
                                                    "ハチメータ消去",
                                                    "テレサ時は無視",
                                                    "リバインド制限",
                                                    "HD中ラッシュ",
                                                    "食い込み除外",
                                                    "水陸移動特殊処理",
                                                    "着地エフェクト",
                                                    "影を消す",
                                                    "ダメージ中跳ねる"},
                                                   {2, "□マリオ側で立てる", "[v]オブジェ側で立てる"}};


static MatrixValueTable talking_head_height[] = {
    {"クッパJｒ", 300.0f}, {"ルイージ", 200.0f}, {"ピーチ姫", 180.0f},       {"ペンギンコーチ", 500.0f},       {"ペンギン仙人", 500.0f},
    {"チコ", 100.0f},      {"デブチコ", 200.0f}, {"キノピオ", 100.0f},       {"天文台用キノピオ", 100.0f},     {"ペンギン", 160.0f},
    {"ロゼッタ", 240.0f},  {"ウサギ", 100.0f},   {"いたずらウサギ", 100.0f}, {"シャッチー（会話用）", 220.0f}, {""}};


void MarioActor::initActionMatrix() {
    mActionTable1080 = new MarioActionTable(this);
}

bool MarioActor::isActionOk(const char* name) const {
    return mActionTable1080->isActionOk(name);
}

u8 MarioActor::selectAction(const char* name) const {
    return mActionTable1080->selectAction(name);
}

bool MarioActor::selectAutoBind(const char* name, u8* value) const {
    return mActionTable1080->selectAutoBind(name, value);
}


bool MarioActor::selectCustomEffectSpinHitSound(const char* pName) const {
    return mActionTable1080->testEffect(pName, "スピンヒット音");
}


bool MarioActor::selectSpinCatchInRush(const char* pName) const {
    return mActionTable1080->testEffect(pName, "スピンキャッチ許可");
}


bool MarioActor::selectNotHomingSensor(const HitSensor* pSensor) const {
    return mActionTable1080->testEffectDisabled(MR::getSensorHost(pSensor)->mName, "SHDホーミング許可");
}


bool MarioActor::selectInvalidMovingCollision(const char* pName) const {
    return mActionTable1080->testEffect(pName, "地形移動無視");
}


bool MarioActor::selectQuickResetBeeWallGravity(const char* pName) const {
    return mActionTable1080->testEffect(pName, "ハチ出立");
}


bool MarioActor::selectJumpRushSensor(const char* pName) const {
    return mActionTable1080->testEffect(pName, "乗換センサ");
}


bool MarioActor::selectDamageFireColor(const char* pName) const {
    return mActionTable1080->testEffect(pName, "青い炎エフェクト");
}


bool MarioActor::selectWaterInOutEffect(const char* pName) const {
    return mActionTable1080->testEffect(pName, "水柱エフェクト");
}


bool MarioActor::selectOnDamageRush(const char* pName) const {
    return mActionTable1080->testEffect(pName, "ダメージ中RUSH");
}


bool MarioActor::selectTiltPress(const HitSensor* pSensor) const {
    return mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, "ナナメつぶし許可");
}


bool MarioActor::selectHandyRush(const HitSensor* pSensor) const {
    return mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, "カメを捨てない");
}


bool MarioActor::selectRecoverFlyMeter(const HitSensor* pSensor) const {
    return mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, "ハチ回復");
}


bool MarioActor::selectHideFlyMeter(const HitSensor* pSensor) const {
    return mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, "ハチメータ消去");
}


bool MarioActor::selectTeresaThru(const HitSensor* pSensor) const {
    return mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, "テレサ時は無視");
}


bool MarioActor::selectRebindTimer(const HitSensor* pSensor) const {
    return mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, "リバインド制限");
}


bool MarioActor::selectHipDropRush(const HitSensor* pSensor) const {
    return mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, "HD中ラッシュ");
}


bool MarioActor::selectPushOff(const HitSensor* pSensor) const {
    return mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, "食い込み除外");
}


bool MarioActor::selectWaterInOutRush(const HitSensor* pSensor) const {
    return mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, "水陸移動特殊処理");
}


bool MarioActor::selectLandEffect(const HitSensor* pSensor) const {
    return mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, "着地エフェクト");
}


bool MarioActor::selectNoShadow(const HitSensor* pSensor) const {
    return mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, "影を消す");
}


bool MarioActor::selectDamagePop(const HitSensor* pSensor) const {
    return mActionTable1080->testEffect(MR::getSensorHost(pSensor)->mName, "ダメージ中跳ねる");
}


f32 MarioActor::getFaceLookHeight(const char* pName) const {
    return mActionTable1080->getFaceLookHeight(pName);
}

// ---- MarioActorMorph.cpp ----


void MarioActor::updatePlayerMode() {
    if (_3DC == 0) {
        return;
    }

    _3DC--;

    if (_3DC != 0) {
        return;
    }

    setPlayerMode(0, true, true);
}


struct myStruct {
    const char* _0[9];
    u32 _24;
};


static myStruct cMorphStringTable[] = {{"DieBlackHole", 0, 0, 0, "DieBlackHoleBee", 0, 0, 0, "DieBlackHoleLuigi", 0}, {}};


void MarioActor::initMorphStringTable() {
    for (myStruct* item = cMorphStringTable;; item++) {
        if (item->_0[0] == nullptr) {
            break;
        }

        item->_24 = MR::getHashCode(item->_0[0]);
    }
}


// ---- MarioActorOffensiveMsg.cpp ----


  // namespace


// ---- MarioActorPad.cpp ----


bool MarioActor::isRequestJump2P() const {
    if (mMario->isInputDisable()) {
        return false;
    }

    if (_3C0) {
        return false;
    }

    if (_934 || _38C != 0) {
        return false;
    }

    if (_935) {
        return false;
    }

    if (MR::isStarPointerPointing2P(this, "弱", true, false) && MR::testCorePadTriggerA(1)) {
        return true;
    }

    return false;
}


bool MarioActor::isKeepJump() const {
    return checkButtonType(5, false);
}


bool MarioActor::isKeepJump2P() const {
    if (mMario->isInputDisable()) {
        return false;
    }

    if (_3C0) {
        return false;
    }

    return MR::testCorePadButtonA(1);
}


void MarioActor::lockOnDPD() {
}


bool MarioActor::isRequestSpinJump2P() const {
    if (mMario->isInputDisable()) {
        return false;
    }

    if (_3C0) {
        return false;
    }

    if (!_EEB) {
        return false;
    }

    if (_946) {
        return false;
    }

    if (_EF6) {
        return false;
    }

    if (!mMario->isEnableRush()) {
        return false;
    }

    if (_935) {
        return false;
    }

    if (mPlayerMode == 5) {
        return false;
    }

    if (((mMario->getMovementStates().mLowWord & Mario::MovementStates::Jumping) >> 31) && mMario->_430 == 2 && ((mMario->mFlags14.mLowWord >> 0) & 1U)) {
        return false;
    }

    if (isJumping() && mMario->_3BC > 6 && MR::isStarPointerPointing2P(this, "弱", true, false) && MR::testCorePadTriggerA(1)) {
        return true;
    }

    return false;
}

// ---- MarioActorParts.cpp ----


void MarioActor::updateSpecialModeAnimation() {
    if (!((mMario->mMovementStates.mLowWord >> 21) & 1U) && mMario->getCurrentStatus() == MarioStatus_None) {
        if (((mMario->mMovementStates.mLowWord >> 30) & 1U) && mMario->_960 == 0x20 && mMarioAnim->isAnimationStop()) {
            mMarioAnim->mXanimePlayer->changeTrackAnimation(0, "泥低速歩行");
            mMarioAnim->mXanimePlayer->changeTrackAnimation(1, "泥高速歩行");
            _B96 = 2;
        }
    } else {
        _B96 = 0;
    }
    switch (mPlayerMode) {
    case PlayerMode_Teresa:
        updateTeresaAnimation();
        break;
    case PlayerMode_Bee:
        if (mBeeWallWalk && !isJumping() && mMarioAnim->isAnimationStop()) {
            mMarioAnim->mXanimePlayer->changeTrackAnimation(0, "ハチ匍匐前進");
            mMarioAnim->mXanimePlayer->changeTrackAnimation(1, "ハチ匍匐前進");
            mMarioAnim->mXanimePlayer->changeTrackAnimation(2, "ハチ匍匐前進");
            mMarioAnim->mXanimePlayer->changeTrackAnimation(3, "ハチ匍匐ウエイト");
        }
        break;
    default:
        mMario->_418 = 0;
        break;
    }
    if (_B96) {
        _B96--;
        if (!_B96 && ((mMario->mMovementStates.mLowWord >> 30) & 1U) && mMarioAnim->isAnimationStop()) {
            mMarioAnim->mXanimePlayer->changeTrackAnimation(0, "鈍行");
            mMarioAnim->mXanimePlayer->changeTrackAnimation(1, "歩行");
        }
    }
}


bool MarioActor::finalizeFreezeModel() {
    if (MR::isActionEnd(_9C4)) {
        _9C4->kill();
        MR::offCalcAnim(_9C4);

        return false;
    }

    return true;
}


void MarioActor::offTakingFlag() {
    _480 = false;
}

// ---- MarioActorPunch.cpp ----


// ---- MarioActorRush.cpp ----

void MarioActorRush_FORCE_MATCH_SDATA2() {
    (void)0.001f;
}


// ---- MarioActorRushMsg.cpp ----


void MarioActor::checkPriorRushTarget() {
    s32 maxVal = 0;
    s32 val = 0;
    for (s32 idx = 0; idx < _930; idx++) {
        if (isFixJumpRushSensor(_7E4[idx])) {
            continue;
        }

        u32 v = 0;
        switch (_7E4[idx]->mType) {
        case ATYPE_COCO_NUT_BIND:
            v = 10;
            break;
        }

        if (maxVal >= v) {
            continue;
        }

        maxVal = v;
        val = idx;
    }

    if (maxVal != 0) {
        _7E4[0] = _7E4[val];
        _930 = 1;
    }
}


// ---- MarioActorSensor.cpp ----

void MarioActorSensor_FORCE_MATCH_SDATA2() {
    (void)100.0f;
    (void)2000.0f;
    (void)80.0f;
    (void)60.0f;
    (void)600.0f;
    (void)1000.0f;
    (void)2400.0f;
    (void)3000.0f;
    (void)300.0f;
    (void)0.0872664675f;
    (void)2607.59448f;
    (void)200.0f;
    (void)40.0f;
    (void)1.20000005f;
    (void)1.5f;
}


void MarioActor::initForJump() {
    _988 = 0;
    _989 = 0;
}

// ---- MarioActorShadow.cpp ----

void MarioActorShadow_FORCE_MATCH_SDATA2() {
    (void)1.0f;
    (void)0.0f;
    (void)0.5f;
    (void)-1.0f;
    (void)500.0f;
    (void)0.75f;
    (void)1000.0f;
    (void)4503599627370496.0;
    (void)4503601774854144.0;
}


// ---- MarioActorSpecialDraw.cpp ----

void MarioActorSpecialDraw_FORCE_MATCH_SDATA2() {
    (void)80.0f;
    (void)0.99000001f;
    (void)0.999899983f;
    (void)300.0f;
    (void)1000.0f;
    (void)255.0f;
    (void)63.0f;
    (void)20.0f;
    (void)0.800000012f;
    (void)2607.59448f;
}


  // namespace


void MarioActor::drawMosaic() const {
}


void MarioActor::drawLifeUp() const {
}


// ---- MarioActorTakeMsg.cpp ----

void MarioActorTakeMsg_FORCE_MATCH_SDATA2() {
    (void)1.04719758f;
    (void)500.0f;
    (void)57.2957802f;
    (void)600.0f;
    (void)200.0f;
    (void)-0.100000001f;
    (void)120.0f;
    (void)400000.0f;
    (void)10000.0f;
    (void)0.400000006f;
    (void)0.00999999978f;
    (void)0.0399999991f;
    (void)25.0f;
    (void)50.0f;
}


bool MarioActor::tryCoinPullOne(HitSensor* pSensor) NO_INLINE {
    LiveActor* host = MR::getSensorHost(pSensor);
    tryPullTrans(&host->mVelocity, *MR::getSensorPos(pSensor));

    return pSensor->receiveMessage(ACTMES_ITEM_PULL, getSensor("Body"));
}


// ---- MarioActorWipe.cpp ----


// ---- MarioAnimator.cpp ----


void MarioAnimator_FORCE_MATCH_SDATA2() {
    (void)79.0f;
    (void)59.0f;
    (void)0.00100000005f;
    (void)0.899999976f;
    (void)0.100000001f;
    (void)0.949999988f;
    (void)0.0500000007f;
    (void)0.25f;
    (void)0.970000029f;
    (void)0.0299999993f;
    (void)10.0f;
    (void)1.41421294f;
    (void)6.28318548f;
    (void)30.0f;
    (void)-2607.59448f;
    (void)2607.59448f;
    (void)0.699999988f;
    (void)60.0f;
    (void)5.0f;
    (void)-0.707000017f;
    (void)0.707000017f;
    (void)-0.5f;
}


extern const char* jname_chest;


// native: defined in MarioAnimator.cpp (DOL split owner); misplaced copy
#ifndef GALAXY_NATIVE
const char* jname_chest = "Spine1";
#endif
#pragma push
#pragma opt_propagation off
#pragma pop


// native: defined in MarioAnimator.cpp / Mario.cpp (DOL split owner); misplaced copy
#ifndef GALAXY_NATIVE
HitSensor* MarioActor::getLookTargetSensor() const {
    if (_934) {
        return nullptr;
    }

    if (isSleeping()) {
        return nullptr;
    }

    return _46C;
}
#endif

// ---- MarioBee.cpp ----


// ---- MarioEffect.cpp ----


// ---- MarioSearchLight.cpp ----


  // namespace


// ---- MarioTeresa.cpp ----

// native: defined in MarioAnimator.cpp / Mario.cpp (DOL split owner); misplaced copy
#ifndef GALAXY_NATIVE
void MarioTeresa_FORCE_MATCH_SDATA2() {
    (void)100.0f;
    (void)10.0f;
    (void)0.00100000005f;
    (void)0.00999999978f;
    (void)120.0f;
    (void)110.0f;
    (void)-0.100000001f;
    (void)0.100000001f;
    (void)50.0f;
    (void)0.939999998f;
    (void)1.5f;
    (void)0.200000003f;
    (void)200.0f;
    (void)0.99000001f;
    (void)60.0f;
    (void)-2.0f;
    (void)-0.400000006f;
    (void)-0.25f;
    (void)0.25f;
    (void)0.75f;
    (void)-0.5f;
    (void)0.0399999991f;
    (void)0.300000012f;
    (void)20.0f;
    (void)-2607.59448f;
    (void)2607.59448f;
    (void)0.707000017f;
    (void)-10.0f;
}
#endif


#include "../src/Game/Player/MarioActorSMG2_A.inc"
#include "../src/Game/Player/MarioActorSMG2_B.inc"


namespace {
    TVec3f sInitialVector0(0.0f);
    TVec3f sInitialVector1(0.0f);
    TVec3f sInitialVector2(100.0f);
}


void MarioActor::changeNullAnimation(const char* pName, s8 num) {
    mNullAnimation->appear();
    MR::startAction(mNullAnimation, pName);
    _B92 = num;
}
