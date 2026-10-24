#define JGEOMETRY_VEC3_SUB_NEG_NOINLINE  // SMG2: negateInternal/sub out of line in this TU
#include "Game/Player/Mario.hpp"
#include "Game/NPC/EventDirector.hpp"
#include "Game/Player/MarioMoveInfoA84.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Enemy/KarikariDirector.hpp"
#include "Game/Enemy/Popo.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Player/MarioAbyssDamage.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/MarioBlown.hpp"
#include "Game/Player/MarioBump.hpp"
#include "Game/Player/MarioClimb.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Player/MarioCrush.hpp"
#include "Game/Player/MarioDamage.hpp"
#include "Game/Player/MarioDarkDamage.hpp"
#include "Game/Player/MarioFaint.hpp"
#include "Game/Player/MarioFireDamage.hpp"
#include "Game/Player/MarioFireDance.hpp"
#include "Game/Player/MarioFireRun.hpp"
#include "Game/Player/MarioFlip.hpp"
#include "Game/Player/MarioFlow.hpp"
#include "Game/Player/MarioFoo.hpp"
#include "Game/Player/MarioFpView.hpp"
#include "Game/Player/MarioFreeze.hpp"
#include "Game/Player/MarioFrontStep.hpp"
#include "Game/Player/MarioHang.hpp"
#include "Game/Player/MarioMagic.hpp"
#include "Game/Player/MarioMapCode.hpp"
#include "Game/Player/MarioMove.hpp"
#include "Game/Player/MarioParalyze.hpp"
#include "Game/Player/MarioRabbit.hpp"
#include "Game/Player/MarioRecovery.hpp"
#include "Game/Player/MarioSideStep.hpp"
#include "Game/Player/MarioSkate.hpp"
#include "Game/Player/MarioSlider.hpp"
#include "Game/Player/MarioStep.hpp"
#include "Game/Player/MarioStick.hpp"
#include "Game/Player/MarioStun.hpp"
#include "Game/Player/MarioSukekiyo.hpp"
#include "Game/Player/MarioSwim.hpp"
#include "Game/Player/MarioTalk.hpp"
#include "Game/Player/MarioTeresa.hpp"
#include "Game/Player/MarioWait.hpp"
#include "Game/Player/MarioWall.hpp"
#include "Game/Player/MarioWarp.hpp"
#include "Game/Util.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "math_types.hpp"
#include "Game/Player/MarioShadow.hpp"
#include "Game/Player/MarioState.hpp"
#include "Game/Player/FireMarioBall.hpp"
#include <revolution/types.h>
#include "Game/Util/MapUtil.hpp"
#include "Game/AreaObj/CubeCamera.hpp"
#include "Game/Map/CollisionCode.hpp"
#include "Game/Map/SunshadeMapHolder.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include <cstring>
#include <revolution/mtx.h>
#include "Game/Player/MarioAccess.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Player/MarioEffect.hpp"
#include "Game/Effect/MultiEmitter.hpp"
#include "Game/Effect/ParticleEmitter.hpp"
#include "Game/LiveActor/EffectKeeper.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <JSystem/JParticle/JPAEmitter.hpp>
#include <JSystem/JParticle/JPAMath.hpp>
#include <cstdio>
#include <cstddef>
#include "Game/Player/MarioModule.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/MapObj/DashRing.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DirectDraw.hpp"
#include "Game/Util/DirectDrawUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>
#include "Game/Util/GamePadUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>
#include "Game/AudioLib/AudAnmSoundObject.hpp"
#include "Game/AudioLib/AudSoundId.hpp"
#include "Game/System/ResourceHolder.hpp"
#include <JSystem/JAudio2/JAISound.hpp>
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/AreaObj/WaterArea.hpp"
#include "Game/Player/MarineSnow.hpp"
#include "Game/Screen/GameSceneLayoutHolder.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/XanimeResource.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/MapObj/BigFanHolder.hpp"
#include "Game/MapObj/SpinCloudBlock.hpp"
#include "Game/Player/MarioParts.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JMath/JMath.hpp>
#include "Game/AreaObj/WarpCube.hpp"
#include "Game/MapObj/WarpPod.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Player/YoshiTongueAction.hpp"
#include "Game/Player/YoshiHolder.hpp"
#include "Game/Player/MarioMtxPos.hpp"
#include "Game/Player/MarioYoshiPull.hpp"
#include "Game/Player/MarioYoshiBalloon.hpp"
#include "Game/Player/MarioYoshiShot.hpp"
#include "Game/Player/MarioYoshiDash.hpp"
#include "Game/Player/MarioAllModel.hpp"
#include "Game/AudioLib/AudSeLifeTable.hpp"

#ifdef GALAXY_NATIVE
// OdysseyMovement mod (MacioGalaxy docs/dev/ODYSSEY_MOVEMENT.md): with the mod on,
// the chain jumps (single, double, triple) and the other jump types follow Super
// Mario Odyssey's model (petari/odyssey_move.hpp). With it off none of this runs.
// The hooks are in tryJump, tryTurnJump, trySquatJump, tryBackJump, trySpinJump,
// tryWallJump, procHipDrop, doLanding and fn_8039A600 (procJump's velocity step),
// plus updateWalkSpeed in MarioSMG2_A.inc.
#include <algorithm>
#include <cstdlib>
#include <petari/odyssey_move.hpp>
#include "Game/Player/MarioCappy.hpp"
extern "C" bool petari_mod_enabled(int mod);
namespace {
    constexpr int kModOdysseyMovement = 2;  // PetariNative::Mods::Mod::OdysseyMovement
    struct OdysseyAir {
        PetariNative::Odyssey::AirState air;
        TVec3f front = TVec3f(0.0f, 0.0f, 1.0f);  // reference direction (across gravity)
        bool active = false;                       // the current jump is the mod's
        s32 jumpKind = -1;                         // its _430 value
        s32 lastChain = -1;                        // chain index of the last mod jump
        TVec3f lastDir = TVec3f(0.0f, 0.0f, 1.0f);
    };
    OdysseyAir sOdyssey;
    bool sOdysseyGroundPoundJump = false;  // the next tryJump is a ground-pound jump
    PetariNative::Odyssey::Air sOdysseyLanded = PetariNative::Odyssey::Air::None;  // the mod's move that just landed (doLanding)
    constexpr f32 kOdysseyDiveLeapRate = 2.2f;  // the long-jump leap (49 frames) reaches its end ~22 frames into a dive
    // "Jump during a ground pound dives" (Mods::Mod::OdysseyJumpDive, on by
    // default): Jump pressed during the mod's ground pound dives, as Spin does.
    constexpr int kModOdysseyJumpDive = 4;
    s32 sOdysseyAirFrames = 0;             // frames in procJump's air branch since the last landing (or wall jump)
    int sCappyPostBounceDives = 0;
    bool sCappyDiveReady = false;
    bool sOdysseyAirDived = false;         // one jump-press dive per airtime (a cap bounce gives it back)

    bool odysseyOn() { return petari_mod_enabled(kModOdysseyMovement); }

    // Starts the mod's model for a jump the game has just set up; dir is the
    // take-off direction (any length), up the current -gravity.
    void odysseyStart(PetariNative::Odyssey::Air kind, TVec3f dir, const TVec3f& up, f32 speed, s32 jumpKind) {
        dir -= up * up.dot(dir);
        MR::normalizeOrZero(&dir);
        sOdyssey.air = PetariNative::Odyssey::startAir(kind, speed);
        sOdyssey.front = dir;
        sOdyssey.jumpKind = jumpKind;
        sOdyssey.active = true;
    }

    // The current horizontal (across gravity) velocity of an airborne Mario: the
    // mod's own when its model runs, else Galaxy's jump vector. (Used by the Cappy
    // mod, which is a separate port; kept so that port can share this block.)
    [[maybe_unused]] TVec3f odysseyAirVelocity(const Mario* pMario, const TVec3f& up) {
        if (sOdyssey.active) {
            TVec3f front(sOdyssey.front - up * up.dot(sOdyssey.front));
            MR::normalizeOrZero(&front);
            TVec3f side;
            side.cross(up, front);
            return front * sOdyssey.air.front + side * sOdyssey.air.side;
        }
        return pMario->mJumpVec - up * up.dot(pMario->mJumpVec);
    }

    [[maybe_unused]] f32 odysseyUpSpeed(const Mario* pMario, const TVec3f& up) {
        return sOdyssey.active ? sOdyssey.air.up : up.dot(pMario->mJumpVec);
    }


    // OdysseyMovement: SMO's dive (20 forward, 28 up, gravity 2.0) from a ground
    // pound in the air, on Spin or (OdysseyJumpDive) Jump (procHipDrop).
    // Galaxy has no dive; it uses _430 5 (the long jump's type, which also keeps a
    // new ground pound from starting, as in SMO) with Galaxy's long-jump leap,
    // played fast enough to reach its end (both arms swept back, the pose a long
    // jump settles into when it flies long) and held there (procJump); it lands
    // running or in a belly flop (doLanding).
    void odysseyStartDive(Mario& m, const char* pFrom) {
        m.mMovementStates.mLowWord &= 0xFFEFFFFFU;  // _B: ground pound
        m._424 = 0;
        m._430 = 5;
        m.stopAnimation(nullptr);
        m.changeAnimationNonStop("幅とび");
        m.changeAnimation(nullptr, "落下");
        m.playSound("声高飛び込み");
        m.playSound("ジャンプ踏切");
        TVec3f up(-m.mActor->mAppliedGravityVec);
        MR::normalizeOrZero(&up);
        odysseyStart(PetariNative::Odyssey::Air::Dive, m.getAirFrontVec(), up, 0.0f, m._430);
        sOdyssey.lastChain = -1;
        sOdysseyAirDived = true;
        if (sCappyDiveReady) {
            ++sCappyPostBounceDives;
            sCappyDiveReady = false;
        }
        petariCappyDiveStarted();
        std::fprintf(stderr, "[odyssey] dive (%s) from (%.3f, %.3f, %.3f)\n", pFrom, m.mPosHolder->mPos.x, m.mPosHolder->mPos.y,
                     m.mPosHolder->mPos.z);
    }

    // Whether a ground pound lands within ContinuousJumpPreInputFrame frames: a
    // Jump press then belongs to the ground-pound jump on landing, not to a dive.
    // In the pound's wind-up (its stall in the air) Mario does not fall; in the
    // fall the speed grows by Galaxy's hip-drop gravity up to the mod's 45.
    bool odysseyGroundPoundLandingSoon(Mario& m) {
        if (m.isAnimationRun(m._720) && !m.isAnimationTerminate(nullptr)) {
            return false;
        }
        f32 speed = std::max(m.getGravityVec()->dot(m.mJumpVec), 0.0f);
        f32 height = m.mVerticalSpeed;  // distance to the ground below, along gravity
        const f32 gravity = m.mActor->mConst->getTable()->mGravityHipDrop;
        for (int i = 0; i < PetariNative::Odyssey::Const::ContinuousJumpPreInputFrame; ++i) {
            speed = std::min(speed + gravity, PetariNative::Odyssey::groundPoundFallSpeed());
            height -= speed;
            if (height <= 0.0f) {
                return true;
            }
        }
        return false;
    }
}  // namespace

bool petariOdysseyAirThrow(Mario* pMario, bool hop) {
    Mario& m = *pMario;
    if (!((m.mMovementStates.mLowWord & Mario::MovementStates::Jumping) != 0) || ((m.mMovementStates.mLowWord >> 20) & 1U) || ((m.mMovementStates.mLowWord >> 30) & 1U)) {
        return false;
    }
    TVec3f up(-m.mActor->mAppliedGravityVec);
    MR::normalizeOrZero(&up);
    const TVec3f velocity = odysseyAirVelocity(pMario, up);
    const f32 upSpeed = odysseyUpSpeed(pMario, up);
    TVec3f dir(velocity);
    MR::normalizeOrZero(&dir);
    if (MR::isNearZero(dir)) {
        dir = m.getAirFrontVec();
    }
    odysseyStart(PetariNative::Odyssey::Air::CapThrow, dir, up, velocity.length(), m._430);
    sOdyssey.air = PetariNative::Odyssey::startCapThrow(velocity.length(), upSpeed, hop);
    sOdyssey.lastChain = -1;
    m.stopAnimationUpper(nullptr);
    m.changeAnimation("空中ひねり", "落下");
    return true;
}

// Cappy mod: bounce off the cap. From a dive: up 22, gravity 1.0, horizontal up
// to 16; from a ground pound: up 35, gravity 1.5; otherwise up 25, gravity 1.0
// (a jump's horizontal rules). Galaxy sees a normal jump (_430 0) with its
// mid-air somersault.
void petariOdysseyCapBounce(Mario* pMario) {
    namespace Odyssey = PetariNative::Odyssey;
    Mario& m = *pMario;
    TVec3f up(-m.mActor->mAppliedGravityVec);
    MR::normalizeOrZero(&up);
    const bool groundPound = ((m.mMovementStates.mLowWord >> 20) & 1U);
    const bool dive = sOdyssey.active && sOdyssey.air.kind == Odyssey::Air::Dive;
    const Odyssey::Air kind = groundPound ? Odyssey::Air::GroundPoundCapBounce : dive ? Odyssey::Air::DiveCapBounce : Odyssey::Air::CapBounce;
    TVec3f velocity = groundPound ? TVec3f(0.0f, 0.0f, 0.0f) : odysseyAirVelocity(pMario, up);
    TVec3f dir(velocity);
    MR::normalizeOrZero(&dir);
    if (MR::isNearZero(dir)) {
        dir = m.getAirFrontVec();
    }
    if (groundPound) {
        m.mMovementStates.mLowWord &= ~(1U << 20);
        m._424 = 0;
    }
    m._430 = 0;
    // A cap bounce starts a fresh airborne phase. Ground pound (including the
    // one before a dive) calls cancelTornadoJump, which spends Galaxy's air
    // Spin separately from _430. Restore that move as landing does (stopJump).
    m.mMovementStates.mHighWord &= ~(1U << 20);
    // The dive holds the leap's last frame at rate 0 (procJump): play on.
    if (XanimeFrameCtrl* ctrl = m.getAnimator()->getXanimePlayer()->getFrameCtrl(); ctrl != nullptr) {
        ctrl->setRate(1.0f);
    }
    m.stopAnimationUpper(nullptr);
    m.stopAnimation(nullptr);
    m.changeAnimation("空中一回転", "落下");
    m.playSound("中ジャンプ");
    m.playSound("声中ジャンプ");
    m.playSound("ジャンプ踏切");
    odysseyStart(kind, dir, up, velocity.length(), m._430);
    sOdyssey.lastChain = -1;
    // A cap bounce is a new air action: the jump-press dive is available again.
    sCappyDiveReady = true;
    sOdysseyAirDived = false;
    sOdysseyAirFrames = 0;
    std::fprintf(stderr, "[odyssey] cap bounce (kind %d) from (%.3f, %.3f, %.3f)\n", static_cast< int >(kind), m.mPosHolder->mPos.x,
                 m.mPosHolder->mPos.y, m.mPosHolder->mPos.z);
}
extern "C" int petari_cappy_post_bounce_dives() { return sCappyPostBounceDives; }

#endif

namespace {
    static const char* sPreviousInertiaName = "デフォルト";
    static const char* sInertiaName = "デフォルト";
}

struct SoundList {
    struct SoundFlags {
        u8 mPolicy;
        u8 _pad[3];
    };

    const char* name;
    u32 _4;

    SoundFlags _8;
    const char* _C;

    u32 _10;
    u32 _14;
};


SoundList soundlist[] = {
    {
        "声小ジャンプ",  // name
        SE_PV_JUMP_S,    // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声中ジャンプ",  // name
        SE_PV_JUMP_M,    // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声大ジャンプ",  // name
        SE_PV_JUMP_L,    // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声後ジャンプ",   // name
        SE_PV_JUMP_TURN,  // 0x4
        {0x00, {0, 0, 0}},                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "声幅ジャンプ",   // name
        SE_PV_JUMP_LONG,  // 0x4
        {0x00, {0, 0, 0}},                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "声物ジャンプ",  // name
        SE_PV_JUMP_JOY,  // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声高飛び込み",   // name
        SE_PV_HIGH_DIVE,  // 0x4
        {0x00, {0, 0, 0}},                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "声ミスジャンプ",
        0x10059,
        {0x00, {0, 0, 0}},
        0,
        0,
        0,
    },
    {
        "声尻ドロップ",  // name
        SE_PV_HIP_DROP,  // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声尻ドロップ着地",   // name
        SE_PV_HIP_DROP_LAND,  // 0x4
        {0x00, {0, 0, 0}},                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "声着地停止",  // name
        SE_PV_LAND,    // 0x4
        {0x00, {0, 0, 0}},             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "声ステージイン着地",  // name
        SE_PV_LAND_COOL,       // 0x4
        {0x00, {0, 0, 0}},                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "声小ダメージ",  // name
        SE_PV_DAMAGE_S,  // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声大ダメージ",  // name
        SE_PV_DAMAGE_L,  // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声最終ダメージ",   // name
        SE_PV_LAST_DAMAGE,  // 0x4
        {0x00, {0, 0, 0}},                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "声炎ダメージ",  // name
        SE_PV_BURN,      // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声電気ダメージ",   // name
        SE_PV_ELEC_DAMAGE,  // 0x4
        {0x00, {0, 0, 0}},                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "声電気ダメージ終了",       // name
        SE_PV_ELEC_DAMAGE_RECOVER,  // 0x4
        {0x00, {0, 0, 0}},                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "声倒れ",    // name
        SE_PV_DOWN,  // 0x4
        {0x00, {0, 0, 0}},           // 0x8
        0,           // 0xC
        0,           // 0x10
        0,           // 0x14
    },
    {
        "声落下死亡",    // name
        SE_PV_FALL_DIE,  // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声しびれ",      // name
        0x1005F,  // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声氷ダメージ",  // name
        SE_PV_FREEZE,    // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声つぶれダメージ",    // name
        SE_PV_DIE_ROCK_CRASH,  // 0x4
        {0x00, {0, 0, 0}},                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "声スピン",  // name
        SE_PV_SPIN,  // 0x4
        {0x00, {0, 0, 0}},           // 0x8
        0,           // 0xC
        0,           // 0x10
        0,           // 0x14
    },
    {
        "声スピンキャンセル",  // name
        SE_PV_SPIN_CANCEL,     // 0x4
        {0x00, {0, 0, 0}},                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "声パンチ",   // name
        SE_PV_PUNCH,  // 0x4
        {0x00, {0, 0, 0}},            // 0x8
        0,            // 0xC
        0,            // 0x10
        0,            // 0x14
    },
    {
        "声踏み",     // name
        SE_PV_STOMP,  // 0x4
        {0x00, {0, 0, 0}},            // 0x8
        0,            // 0xC
        0,            // 0x10
        0,            // 0x14
    },
    {
        "声蹴り",    // name
        SE_PV_KICK,  // 0x4
        {0x00, {0, 0, 0}},           // 0x8
        0,           // 0xC
        0,           // 0x10
        0,           // 0x14
    },
    {
        "声トルネード",     // name
        SE_PV_TWIST_START,  // 0x4
        {0x00, {0, 0, 0}},                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "声壁反射",   // name
        SE_PV_GUARD,  // 0x4
        {0x00, {0, 0, 0}},            // 0x8
        0,            // 0xC
        0,            // 0x10
        0,            // 0x14
    },
    {
        "声投げ",     // name
        SE_PV_THROW,  // 0x4
        {0x00, {0, 0, 0}},            // 0x8
        0,            // 0xC
        0,            // 0x10
        0,            // 0x14
    },
    {
        "声壁押し",     // name
        SE_PV_LIFT_UP,  // 0x4
        {0x00, {0, 0, 0}},              // 0x8
        0,              // 0xC
        0,              // 0x10
        0,              // 0x14
    },
    {
        "声壁体当たり",       // name
        SE_PV_WALL_HIT_BODY,  // 0x4
        {0x00, {0, 0, 0}},                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "声スピン尻ドロップ",  // name
        SE_PV_PUNCH,           // 0x4
        {0x00, {0, 0, 0}},                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "声軽い力み",  // name
        SE_PV_CATCH,   // 0x4
        {0x00, {0, 0, 0}},             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "声崖つかまり",         // name
        SE_PV_CLIFF_FALL_HANG,  // 0x4
        {0x00, {0, 0, 0}},                      // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "声崖つかまり終了",  // name
        SE_PV_CLIFF_CLIMB,   // 0x4
        {0x00, {0, 0, 0}},                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "声炎ダメージ中",  // name
        SE_PV_BURN_RUN,    // 0x4
        {0x00, {0, 0, 0}},                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "声炎ダメージ終了",  // name
        SE_PV_BURN_RECOVER,  // 0x4
        {0x00, {0, 0, 0}},                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "声氷ダメージ終了",    // name
        SE_PV_FREEZE_RECOVER,  // 0x4
        {0x00, {0, 0, 0}},                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "声針ダメージ",       // name
        SE_PV_NEEDLE_DAMAGE,  // 0x4
        {0x00, {0, 0, 0}},                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "声針ダメージ中",         // name
        SE_PV_NEEDLE_DAMAGE_RUN,  // 0x4
        {0x00, {0, 0, 0}},                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "声針ダメージ終了",           // name
        SE_PV_NEEDLE_DAMAGE_RECOVER,  // 0x4
        {0x00, {0, 0, 0}},                            // 0x8
        0,                            // 0xC
        0,                            // 0x10
        0,                            // 0x14
    },
    {
        "声水中ダメージ",      // name
        SE_PV_DAMAGE_S_WATER,  // 0x4
        {0x00, {0, 0, 0}},                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "声水中最終ダメージ",     // name
        SE_PV_LAST_DAMAGE_WATER,  // 0x4
        {0x00, {0, 0, 0}},                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "声冷水ダメージ",         // name
        SE_PV_COLD_WATER_DAMAGE,  // 0x4
        {0x00, {0, 0, 0}},                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "声スケキヨ開始",  // name
        SE_PV_BURY_HEAD,   // 0x4
        {0x00, {0, 0, 0}},                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "声スケキヨ終了",         // name
        SE_PV_BURY_HEAD_RECOVER,  // 0x4
        {0x00, {0, 0, 0}},                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "声足埋まり開始",  // name
        SE_PV_BURY_FOOT,   // 0x4
        {0x00, {0, 0, 0}},                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "声足埋まり終了",         // name
        SE_PV_BURY_FOOT_RECOVER,  // 0x4
        {0x00, {0, 0, 0}},                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "声あくび",  // name
        SE_PV_YAWN,  // 0x4
        {0x00, {0, 0, 0}},           // 0x8
        0,           // 0xC
        0,           // 0x10
        0,           // 0x14
    },
    {
        "声いびき１",   // name
        SE_PV_SLEEP_1,  // 0x4
        {0x00, {0, 0, 0}},              // 0x8
        0,              // 0xC
        0,              // 0x10
        0,              // 0x14
    },
    {
        "声いびき２",   // name
        SE_PV_SLEEP_2,  // 0x4
        {0x00, {0, 0, 0}},              // 0x8
        0,              // 0xC
        0,              // 0x10
        0,              // 0x14
    },
    {
        "声噴水ジャンプ",  // name
        SE_PV_UPSET,       // 0x4
        {0x00, {0, 0, 0}},                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "声ランニングキック",  // name
        SE_PV_KICK,            // 0x4
        {0x00, {0, 0, 0}},                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "声砂脱出",     // name
        SE_PV_LIFT_UP,  // 0x4
        {0x00, {0, 0, 0}},              // 0x8
        0,              // 0xC
        0,              // 0x10
        0,              // 0x14
    },
    {
        "声投げられ",  // name
        SE_PV_THROWN,  // 0x4
        {0x00, {0, 0, 0}},             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "声慌て",     // name
        SE_PV_UPSET,  // 0x4
        {0x00, {0, 0, 0}},            // 0x8
        0,            // 0xC
        0,            // 0x10
        0,            // 0x14
    },
    {
        "声しゃがむ",  // name
        SE_PV_SQUAT,   // 0x4
        {0x00, {0, 0, 0}},             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "声砂沈み",     // name
        SE_PV_DIE_MUD,  // 0x4
        {0x00, {0, 0, 0}},              // 0x8
        0,              // 0xC
        0,              // 0x10
        0,              // 0x14
    },
    {
        "声砂沈み死亡",  // name
        SE_PV_MUD_SINK,  // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声沼沈み",     // name
        SE_PV_DIE_MUD,  // 0x4
        {0x00, {0, 0, 0}},              // 0x8
        0,              // 0xC
        0,              // 0x10
        0,              // 0x14
    },
    {
        "声沼沈み死亡",  // name
        SE_PV_MUD_SINK,  // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "足音左",          // name
        SE_PM_FOOTNOTE_L,  // 0x4
        {0x00, {0, 0, 0}},                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "足音右",          // name
        SE_PM_FOOTNOTE_R,  // 0x4
        {0x00, {0, 0, 0}},                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "ジャンプ踏切",    // name
        SE_PM_JUMP,        // 0x4
        {0x04, {0, 0, 0}},         // 0x8
        "水跳ねジャンプ",  // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "着地",        // name
        SE_PM_LAND,    // 0x4
        {0x04, {0, 0, 0}},     // 0x8
        "水跳ね着地",  // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "重い着地",        // name
        SE_PM_LAND_HEAVY,  // 0x4
        {0x00, {0, 0, 0}},                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "掴み",          // name
        SE_PM_GRAB_OBJ,  // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "ぶらさがり",  // name
        SE_PM_HAND,    // 0x4
        {0x00, {0, 0, 0}},             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "尻ドロップ回転",   // name
        SE_PM_PRE_HIPDROP,  // 0x4
        {0x00, {0, 0, 0}},                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "尻ドロップ着地",  // name
        SE_PM_HIPDROP,     // 0x4
        {0x04, {0, 0, 0}},         // 0x8
        "水跳ね着地",      // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "スピン尻ドロップ回転",    // name
        SE_PM_SPIN_HIP_DROP_TURN,  // 0x4
        {0x00, {0, 0, 0}},                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "スピン尻ドロップ落下",       // name
        SE_PM_LV_SPIN_HIP_DROP_FALL,  // 0x4
        {0x01, {0, 0, 0}},                    // 0x8
        0,                            // 0xC
        0,                            // 0x10
        0,                            // 0x14
    },
    {
        "中ジャンプ",  // name
        SE_PM_JUMP_M,  // 0x4
        {0x00, {0, 0, 0}},             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "大ジャンプ",  // name
        SE_PM_JUMP_L,  // 0x4
        {0x00, {0, 0, 0}},             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "後ジャンプ",     // name
        SE_PM_JUMP_TURN,  // 0x4
        {0x00, {0, 0, 0}},                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "幅ジャンプ",     // name
        SE_PM_JUMP_LONG,  // 0x4
        {0x00, {0, 0, 0}},                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "トルネードジャンプ",  // name
        SE_PM_TWIST_JUMP,      // 0x4
        {0x00, {0, 0, 0}},                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "スピンジャンプ",   // name
        SE_PM_SPIN_ATTACK,  // 0x4
        {0x04, {0, 0, 0}},          // 0x8
        "水跳ねジャンプ",   // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "トランポリンジャンプ小",  // name
        SE_OJ_TRAMPOLINE_BOUND_S,  // 0x4
        {0x00, {0, 0, 0}},                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "トランポリンジャンプ大",  // name
        SE_OJ_TRAMPOLINE_BOUND_L,  // 0x4
        {0x00, {0, 0, 0}},                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "スリップアップ",  // name
        SE_PM_SLIP_UP,     // 0x4
        {0x00, {0, 0, 0}},                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "ダメージ",      // name
        SE_PM_DAMAGE_S,  // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "炎ダメージ",  // name
        SE_PM_BURN,    // 0x4
        {0x00, {0, 0, 0}},             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "針ダメージ",         // name
        SE_PM_NEEDLE_DAMAGE,  // 0x4
        {0x00, {0, 0, 0}},                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "電気ダメージ",     // name
        SE_PM_ELEC_DAMAGE,  // 0x4
        {0x00, {0, 0, 0}},                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "壁反射",             // name
        SE_PM_SPIN_HIT_WALL,  // 0x4
        {0x00, {0, 0, 0}},                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "プレスダメージ",      // name
        SE_PM_DAMAGE_STOMPED,  // 0x4
        {0x00, {0, 0, 0}},                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "氷ダメージ",      // name
        SE_PM_ICE_DAMAGE,  // 0x4
        {0x00, {0, 0, 0}},                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "倒れ",            // name
        SE_PM_FALLDOWN_S,  // 0x4
        {0x04, {0, 0, 0}},         // 0x8
        "水跳ね左足",      // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "吹っ飛び倒れ",    // name
        SE_PM_FALLDOWN_M,  // 0x4
        {0x04, {0, 0, 0}},         // 0x8
        "水跳ね着地",      // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "坂滑り",                 // name
        SE_PM_LV_SLIP_SLIP_CODE,  // 0x4
        {0x09, {0, 0, 0}},                // 0x8
        "水面滑り",               // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "ルイージ滑り",            // name
        SE_PM_LV_LUIGI_WALK_SLIP,  // 0x4
        {0x09, {0, 0, 0}},                 // 0x8
        "水面滑り",                // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "パンチ風切り",     // name
        SE_PM_PUNCH_SHOOT,  // 0x4
        {0x00, {0, 0, 0}},                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "炎ダメージ復帰バウンド",  // name
        SE_PM_BURN_JUMP,           // 0x4
        {0x00, {0, 0, 0}},                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "炎ダメージ炎上中",  // name
        SE_PM_LV_BURNING,    // 0x4
        {0x01, {0, 0, 0}},           // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "氷ダメージ終了",          // name
        SE_PM_ICE_DAMAGE_RECOVER,  // 0x4
        {0x00, {0, 0, 0}},                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "引き戻し基本",           // name
        SE_PM_LV_PULL_BACK_BASE,  // 0x4
        {0x01, {0, 0, 0}},                // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "引き戻し浮遊",          // name
        SE_PM_LV_PULL_BACK_FLY,  // 0x4
        {0x01, {0, 0, 0}},               // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "引き戻し泡破裂",        // name
        SE_OJ_GCAPTURE_RELEASE,  // 0x4
        {0x00, {0, 0, 0}},                       // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "はねとばされ",             // name
        SE_PM_LV_FLIP_DAMAGE_TURN,  // 0x4
        {0x01, {0, 0, 0}},                  // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "結界ヒット",                // name
        SE_PM_KAMECK_BARRIER_BOUND,  // 0x4
        {0x00, {0, 0, 0}},                           // 0x8
        0,                           // 0xC
        0,                           // 0x10
        0,                           // 0x14
    },
    {
        "壁衝突",             // name
        SE_PM_WALL_HIT_BODY,  // 0x4
        {0x00, {0, 0, 0}},                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "投げられ",       // name
        SE_PM_FLIP_AWAY,  // 0x4
        {0x00, {0, 0, 0}},                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "ヘリコプタージャンプ",  // name
        SE_PM_HELI_JUMP,         // 0x4
        {0x00, {0, 0, 0}},                       // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "滞空開始",                 // name
        SE_PM_HELI_JUMP_AIR_START,  // 0x4
        {0x00, {0, 0, 0}},                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "滞空中",                // name
        SE_PM_LV_HELI_JUMP_AIR,  // 0x4
        {0x01, {0, 0, 0}},               // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "ホッパー跳ね返り",  // name
        SE_PM_HOPPER_BOUND,  // 0x4
        {0x00, {0, 0, 0}},                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "ホッパージャンプ溜め",    // name
        SE_PM_LV_HOPPER_PRE_JUMP,  // 0x4
        {0x01, {0, 0, 0}},                 // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "ホッパージャンプ",  // name
        SE_PM_HOPPER_JUMP,   // 0x4
        {0x00, {0, 0, 0}},                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "テレサ浮遊",               // name
        SE_PM_LV_TERESA_MARIO_FLY,  // 0x4
        {0x01, {0, 0, 0}},                  // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "テレサ踏ん張り",   // name
        SE_PM_LV_AIR_WALK,  // 0x4
        {0x01, {0, 0, 0}},          // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "テレサ壁反射",            // name
        SE_PM_TERESA_MARIO_BOUND,  // 0x4
        {0x00, {0, 0, 0}},                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "テレサ消える",             // name
        SE_PM_TERESA_MARIO_VANISH,  // 0x4
        {0x00, {0, 0, 0}},                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "テレサ現れる",             // name
        SE_PM_TERESA_MARIO_APPEAR,  // 0x4
        {0x00, {0, 0, 0}},                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "テレサ風に乗る",            // name
        SE_PM_LV_T_MARIO_RIDE_WIND,  // 0x4
        {0x01, {0, 0, 0}},                   // 0x8
        0,                           // 0xC
        0,                           // 0x10
        0,                           // 0x14
    },
    {
        "ハチ飛行中",            // name
        SE_PM_LV_BEE_MARIO_FLY,  // 0x4
        {0x01, {0, 0, 0}},               // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "スケキヨ開始",    // name
        SE_PM_BURY_START,  // 0x4
        {0x00, {0, 0, 0}},                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "スケキヨ終了",  // name
        SE_PM_BURY_END,  // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "スケキヨ終了スピン",  // name
        SE_PM_BURY_END_SPIN,   // 0x4
        {0x00, {0, 0, 0}},                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "スケート足",  // name
        SE_PM_SKATE,   // 0x4
        {0x00, {0, 0, 0}},             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "スケート滑り",       // name
        SE_PM_LV_SKATE_SLIP,  // 0x4
        {0x01, {0, 0, 0}},            // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "スケートジャンプ",  // name
        SE_PM_SKATE_JUMP,    // 0x4
        {0x00, {0, 0, 0}},                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "スケート着地",    // name
        SE_PM_SKATE_LAND,  // 0x4
        {0x00, {0, 0, 0}},                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "スケートスピン",  // name
        SE_PM_SKATE_SPIN,  // 0x4
        {0x00, {0, 0, 0}},                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "ハチ壁くっつき",     // name
        SE_PM_BEE_WALL_LAND,  // 0x4
        {0x00, {0, 0, 0}},                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "ハチ壁歩き左",         // name
        SE_PM_BEE_WALL_WALK_L,  // 0x4
        {0x00, {0, 0, 0}},                      // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "ハチ壁歩き右",         // name
        SE_PM_BEE_WALL_WALK_R,  // 0x4
        {0x00, {0, 0, 0}},                      // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "ハチ壁ジャンプ",     // name
        SE_PM_BEE_WALL_JUMP,  // 0x4
        {0x00, {0, 0, 0}},                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "砂沈み",            // name
        SE_PM_LV_SAND_SINK,  // 0x4
        {0x01, {0, 0, 0}},           // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "砂脱出",        // name
        SE_PM_SAND_OUT,  // 0x4
        {0x00, {0, 0, 0}},               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "砂強制沈み",              // name
        SE_PM_LV_SAND_SINK_FORCE,  // 0x4
        {0x01, {0, 0, 0}},                 // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "砂死亡",           // name
        SE_PM_LV_SAND_DIE,  // 0x4
        {0x01, {0, 0, 0}},          // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "沼強制沈み",             // name
        SE_PM_LV_MUD_SINK_FORCE,  // 0x4
        {0x01, {0, 0, 0}},                // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "沼死亡",          // name
        SE_PM_LV_MUD_DIE,  // 0x4
        {0x01, {0, 0, 0}},         // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "毒沼脱出",            // name
        SE_PM_POISON_MUD_OUT,  // 0x4
        {0x00, {0, 0, 0}},                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "毒沼ダメージ",           // name
        SE_PM_POISON_MUD_DAMAGE,  // 0x4
        {0x00, {0, 0, 0}},                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "ダークマター沈み",       // name
        SE_PM_LV_DARK_MATTER_IN,  // 0x4
        {0x01, {0, 0, 0}},                // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "惑星貫通中",            // name
        SE_PM_LV_WARP_STRAIGHT,  // 0x4
        {0x01, {0, 0, 0}},               // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "惑星貫通終了",           // name
        SE_PM_WARP_STRAIGHT_END,  // 0x4
        {0x00, {0, 0, 0}},                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "フー滞空中",           // name
        SE_PM_LV_FOO_FLY_WAIT,  // 0x4
        {0x01, {0, 0, 0}},              // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "フー加速",       // name
        SE_PM_FOO_ACCEL,  // 0x4
        {0x00, {0, 0, 0}},                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "フー飛行中",         // name
        SE_PM_LV_FOO_FLYING,  // 0x4
        {0x01, {0, 0, 0}},            // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "フーブレーキ",   // name
        SE_PM_FOO_BRAKE,  // 0x4
        {0x00, {0, 0, 0}},                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "ワープポッド入り",  // name
        SE_PM_WARP_POD_IN,   // 0x4
        {0x00, {0, 0, 0}},                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "ワープポッド出",    // name
        SE_PM_WARP_POD_OUT,  // 0x4
        {0x00, {0, 0, 0}},                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "ワープポッド移動",      // name
        SE_PM_LV_WARP_POD_MOVE,  // 0x4
        {0x01, {0, 0, 0}},               // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "スペシャルダッシュ強",   // name
        SE_PM_RACE_START_DASH_L,  // 0x4
        {0x00, {0, 0, 0}},                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "スペシャルダッシュ弱",   // name
        SE_PM_RACE_START_DASH_S,  // 0x4
        {0x00, {0, 0, 0}},                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "ダッシュ加速強成功",  // name
        SE_SY_GET_DASH_RING,   // 0x4
        {0x02, {0, 0, 0}},             // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "ブラックホール吸い込まれ",  // name
        SE_PM_BLACK_HOLE_IN,         // 0x4
        {0x00, {0, 0, 0}},                           // 0x8
        0,                           // 0xC
        0,                           // 0x10
        0,                           // 0x14
    },
    {
        "水歩行突入",        // name
        SE_PM_WALK_TO_SWIM,  // 0x4
        {0x00, {0, 0, 0}},                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "水歩行脱出",        // name
        SE_PM_SWIM_TO_WALK,  // 0x4
        {0x00, {0, 0, 0}},                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "水面ウエイト",       // name
        SE_PM_WAIT_ON_WATER,  // 0x4
        {0x00, {0, 0, 0}},                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "水中ウエイト",          // name
        SE_PM_WAIT_UNDER_WATER,  // 0x4
        {0x00, {0, 0, 0}},                       // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "水落下突入",         // name
        SE_PM_DIVE_TO_WATER,  // 0x4
        {0x00, {0, 0, 0}},                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "水落下突入高",
        0x200C5,
        {0x00, {0, 0, 0}},
        0,
        0,
        0,
    },
    {
        "水ジャンプ脱出",       // name
        SE_PM_JUMP_FROM_WATER,  // 0x4
        {0x00, {0, 0, 0}},                      // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "水面一掻き",               // name
        SE_PM_SWIM_ACCEL_ON_WATER,  // 0x4
        {0x00, {0, 0, 0}},                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "水中一掻き",               // name
        SE_PM_SWIM_ACCEL_IN_WATER,  // 0x4
        {0x00, {0, 0, 0}},                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "水面バタ足",            // name
        SE_PM_LV_SWIM_ON_WATER,  // 0x4
        {0x01, {0, 0, 0}},               // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "水中バタ足",            // name
        SE_PM_LV_SWIM_IN_WATER,  // 0x4
        {0x01, {0, 0, 0}},               // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "水面潜り",                // name
        SE_PM_DIVE_WATER_ROLLING,  // 0x4
        {0x00, {0, 0, 0}},                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "水中潜り",                // name
        SE_PM_DIVE_FAST_IN_WATER,  // 0x4
        {0x00, {0, 0, 0}},                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "水面スピン開始",           // name
        SE_PM_TORNADE_ON_WATER_ST,  // 0x4
        {0x00, {0, 0, 0}},                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "水面スピン",               // name
        SE_PM_LV_TORNADE_ON_WATER,  // 0x4
        {0x01, {0, 0, 0}},                  // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "水中スピン開始",           // name
        SE_PM_TORNADE_IN_WATER_ST,  // 0x4
        {0x00, {0, 0, 0}},                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "水中スピン",               // name
        SE_PM_LV_TORNADE_IN_WATER,  // 0x4
        {0x01, {0, 0, 0}},                  // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "水面ダメージ",         // name
        SE_PM_DAMAGE_ON_WATER,  // 0x4
        {0x00, {0, 0, 0}},                      // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "水中ダメージ",         // name
        SE_PM_DAMAGE_IN_WATER,  // 0x4
        {0x00, {0, 0, 0}},                      // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "亀ジェット泳ぎ",          // name
        SE_OJ_LV_TURTLE_JET_SWIM,  // 0x4
        {0x01, {0, 0, 0}},                 // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "水底接触",                 // name
        SE_PM_LV_TURTLE_SEA_SMOKE,  // 0x4
        {0x01, {0, 0, 0}},                  // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "亀壁ヒット",              // name
        SE_OJ_TURTLE_JET_BOUND_W,  // 0x4
        {0x00, {0, 0, 0}},                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "亀ブレーキ",        // name
        SE_PM_TURTLE_BRAKE,  // 0x4
        {0x00, {0, 0, 0}},                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "亀加速",            // name
        SE_PM_TURTLE_ACCEL,  // 0x4
        {0x00, {0, 0, 0}},                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "水跳ね左足",        // name
        SE_PM_FOOTNOTE_L_W,  // 0x4
        {0x00, {0, 0, 0}},                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "水跳ね右足",        // name
        SE_PM_FOOTNOTE_R_W,  // 0x4
        {0x00, {0, 0, 0}},                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "水跳ね左足小",          // name
        SE_PM_FOOTNOTE_SUB_L_W,  // 0x4
        {0x00, {0, 0, 0}},                       // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "水跳ね右足小",          // name
        SE_PM_FOOTNOTE_SUB_R_W,  // 0x4
        {0x00, {0, 0, 0}},                       // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "水跳ねジャンプ",  // name
        SE_PM_JUMP_W,      // 0x4
        {0x00, {0, 0, 0}},                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "水跳ね着地",  // name
        SE_PM_LAND_W,  // 0x4
        {0x00, {0, 0, 0}},             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "水跳ね手",    // name
        SE_PM_HAND_W,  // 0x4
        {0x00, {0, 0, 0}},             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "水弾かれ",         // name
        SE_PM_WATER_BOUND,  // 0x4
        {0x00, {0, 0, 0}},                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "水面滑り",           // name
        SE_PM_LV_SLIP_WATER,  // 0x4
        {0x01, {0, 0, 0}},            // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "トルネード風",       // name
        SE_PM_LV_TWIST_WIND,  // 0x4
        {0x01, {0, 0, 0}},            // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "スリップ",     // name
        SE_PM_LV_SLIP,  // 0x4
        {0x09, {0, 0, 0}},      // 0x8
        "水面滑り",     // 0xC
        0,              // 0x10
        0,              // 0x14
    },
    {
        "最後の一撃",       // name
        SE_PM_LAST_DAMAGE,  // 0x4
        {0x00, {0, 0, 0}},                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "ターンブレーキ",
        0x200A1,
        {0x00, {0, 0, 0}},
        0,
        0,
        0,
    },
    {
        "空中ふんばり",     // name
        SE_PM_LV_AIR_WALK,  // 0x4
        {0x01, {0, 0, 0}},          // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "スピン許可",       // name
        SE_PM_SPIN_ENABLE,  // 0x4
        {0x00, {0, 0, 0}},                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "スピン回復終了",        // name
        SE_PM_SPIN_RECOVER_END,  // 0x4
        {0x00, {0, 0, 0}},                       // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "トルネード飛行",         // name
        SE_PM_LV_TORNADE_FLYING,  // 0x4
        {0x01, {0, 0, 0}},                // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "ライフ警告",         // name
        SE_SY_LV_LIFE_ALERT,  // 0x4
        {0x03, {0, 0, 0}},            // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "ライフ回復",        // name
        SE_SY_LIFE_RECOVER,  // 0x4
        {0x02, {0, 0, 0}},           // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "水中ライフ減少",      // name
        SE_SY_WATER_LIFE_DEC,  // 0x4
        {0x02, {0, 0, 0}},             // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "酸素減少警告",      // name
        SE_SY_OXYGEN_ALERT,  // 0x4
        {0x02, {0, 0, 0}},           // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "無酸素警告",             // name
        SE_SY_OXYGEN_ZERO_ALERT,  // 0x4
        {0x02, {0, 0, 0}},                // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "酸素回復",            // name
        SE_SY_INC_OXYGEN_ONE,  // 0x4
        {0x02, {0, 0, 0}},             // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "酸素完全回復",         // name
        SE_SY_INC_OXYGEN_FULL,  // 0x4
        {0x02, {0, 0, 0}},              // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "水面酸素回復",           // name
        SE_SY_LV_RECOVER_OXYGEN,  // 0x4
        {0x03, {0, 0, 0}},                // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "水面酸素完全回復",         // name
        SE_SY_RECOVER_OXYGEN_FULL,  // 0x4
        {0x02, {0, 0, 0}},                  // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "無酸素ダメージ",  // name
        SE_SY_NO_OXYGEN,   // 0x4
        {0x02, {0, 0, 0}},         // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "ハチ体力回復",               // name
        SE_SY_LV_INC_AIR_WALK_TIMER,  // 0x4
        {0x03, {0, 0, 0}},                    // 0x8
        0,                            // 0xC
        0,                            // 0x10
        0,                            // 0x14
    },
    {
        "ハチ体力完全回復",          // name
        SE_SY_INC_AIR_WALK_TIMER_F,  // 0x4
        {0x02, {0, 0, 0}},                   // 0x8
        0,                           // 0xC
        0,                           // 0x10
        0,                           // 0x14
    },
    {
        "ハチ体力切れ",           // name
        SE_SY_NO_AIR_WALK_TIMER,  // 0x4
        {0x02, {0, 0, 0}},                // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "変身解除",       // name
        SE_SY_MORPH_END,  // 0x4
        {0x02, {0, 0, 0}},        // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "変身解除ＢＧＭ維持",
        0xEB,
        {0x02, {0, 0, 0}},
        0,
        0,
        0,
    },
    {
        "呪い解除",             // name
        SE_SY_MORPH_TO_NORMAL,  // 0x4
        {0x02, {0, 0, 0}},              // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "",  // name
        0,   // 0x4
        {0x00, {0, 0, 0}},   // 0x8
        0,   // 0xC
        0,   // 0x10
        0,   // 0x14
    },
};


struct SoundSwapList {
    const char* name;

    u32 soundIDs[3];
};


SoundSwapList soundswaplist[] = {{"", {0, 0, 0}}};


namespace {
    static f32 sSpeedTableA[] = {0.15f, 0.3f, 0.45f, 0.6f, 0.7f, 0.85f, 0.99f};
    static f32 sSpeedTableB[] = {0.02f, 0.2f, 0.4f, 0.5f, 0.65f, 0.75f, 0.98f};
    static f32 sWalkTargetTable[] = {0.0f, 0.15f, 0.25f, 0.4f, 0.5f, 0.6f, 0.8f, 1.0f};
    static f32 sWeightTable[8][4] = {{0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 0.0f},   {0.75f, 0.25f, 0.0f, 0.0f}, {0.25f, 0.75f, 0.0f, 0.0f},
                                     {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.75f, 0.25f, 0.0f}, {0.0f, 0.25f, 0.75f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}};
    static f32 sFootStep[] = {1.2f, 1.5f, 1.3f, 0.0f};
    static f32 sFootStepYoshi[] = {0.8f, 1.4f, 1.7f, 0.0f};
    static f32 sFootStepBeeWallWalk[] = {0.5f, 0.5f, 0.5f, 0.0f};
};
  // namespace

/*
void Mario::setFrontVecKeepUp(const TVec3f &rV)
{
    TVec3f stack_38(mFrontVec);
    if (!MR::isNearZero(rV)) {
        setFrontVec(rV);
    }

    TVec3f stack_2C = mHeadVec.cross(mFrontVec);

    if (MR::isNearZero(stack_2C)) {
        TVec3f stack_20;
        MR::vecBlendSphere(stack_38, rV, &stack_20, 0.5f);
        stack_2C.cross(mHeadVec, stack_20);
        setFrontVec(stack_20);
        if (MR::isNearZero(stack_2C)) {
            setFrontVecKeepSide(rV);
            return;
        }
    }

    MR::normalize(&stack_2C);
    mSideVec = stack_2C;
    MR::normalize(&mSideVec);
    setFrontVec(mSideVec.cross(mHeadVec));
    _22C = mFrontVec;
    _328 = mFrontVec % _328.length();
    _344 = mSideVec;
}

*/

#ifdef GALAXY_NATIVE
namespace {
    const TVec3f sNativeZeroVec(0.0f, 0.0f, 0.0f);
}
#endif

namespace {
    const char sDamageAirAnimation[] = "中ダメージ空中";
    const char sDamageLandAnimation[] = "中ダメージ着地";
    const char sBackDamageAirAnimation[] = "中後ダメージ空中";
    const char sBackDamageLandAnimation[] = "中後ダメージ着地";
};

namespace {

    void addMarioEffectAtJoint(LiveActor* pActor, const char* pEffectName, const char* pJointName, const char* pGroupName) NO_INLINE {
        MtxPtr mtx = pJointName != nullptr ? MR::getJointMtx(pActor, pJointName) : pActor->getBaseMtx();
        pActor->mEffectKeeper->registerEffect(pEffectName, mtx, &pActor->mScale, pGroupName, pJointName);
    }

};

namespace {
    f32 cTurnMotionSpeed = 5.0f;
    f32 cWidth = 70.0f;
    f32 cLimitAngleSink = JGeometry::TUtil< f32 >::PI() / 1.0001f;
    f32 cNeutralAngleWait = JGeometry::TUtil< f32 >::PI() / 2.0f - JGeometry::TUtil< f32 >::PI() / 6.0f;
    f32 cLimitAngleWait = JGeometry::TUtil< f32 >::PI() / 1.0001f;
    f32 cUpperAngleWait = JGeometry::TUtil< f32 >::PI() / 100.0f;
};

namespace {
    static bool sIsDisabled;
};

namespace {
    f32 cDropFrontSpeed = 2.0f;
};

#if 0  // SMG2: uses LiveActor::mSoundObject (AudAnmSoundObject), gone in SMG2
bool Mario::isRunningBas(const char* pAnimName) const {
    if (_970 == nullptr) {
        return false;
    }

    if (strcmp(_970, pAnimName) == 0) {
        return true;
    }

    return false;
}


void Mario::skipBas(f32 frame) {
    mActor->mSoundObject->skip(frame);
}
#endif

static const f32 cAnimSpeeds[] = {1.0f, 1.0f, 1.0f, 1.0f};

namespace {
    static f32 cFrontAcc[] = {-0.2f, -0.2f, -0.2f, -0.1f, -0.1f, -0.1f, -0.1f, -0.1f, -0.0f, -0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                              0.0f,  0.0f,  0.0f,  0.2f,  0.2f,  0.2f,  0.3f,  0.3f,  0.4f,  0.5f,  0.5f, 0.5f, 0.5f, 0.5f,
                              0.4f,  0.4f,  0.3f,  0.3f,  0.2f,  0.2f,  0.1f,  0.1f,  0.1f,  0.1f,  0.1f, 0.0f};

    static f32 cFrontAccSpin[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 8.0f,  6.0f,  4.0f,  2.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
                                  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};

    static f32 cFrontAccSpinSurface[] = {0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  8.0f,  6.0f,  4.0f,  2.0f,  1.0f,
                                         -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};
    static f32 cWeightTable[] = {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    static f32 cSwimWeightTable[] = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    static f32 cWeightTableSP[] = {0.0f, 0.0f, 0.75f, 0.25f};
    static f32 cWaterSurfaceRange = 160.0f;
};
  // namespace

f32 getSwimValue(f32 stick, u32 index, const MarioConstTable* pTable) {
    if (index == 2) {
        return pTable->mSwimRotXIne;
    }

    if (index == 3) {
        return pTable->mSwimRotZIne;
    }

    return 0.0f;
}

static const f32 sOne = 1.0f;

static const f32 sZero = 0.0f;

static const f32 sSameDirEpsilon = 0.01f;

static const f32 sNearZeroEpsilon = 0.001f;

static const f32 sHipDropSlideAngle = 0.7853982f;

static const f32 sHipDropSlideRot = 0.10471976f;

static const f32 sHipDropSlideLen = 10.0f;

static const f32 sJumpDropSlideDotMin = 0.1f;

static const f32 sHopperJumpSlideDotMin = 0.707f;



namespace JGeometry {
};

// SMG2-only Mario:: functions (fn_ range A), written from the asm. Included at the end of Mario.cpp (unity TU).
// Named Yoshi member views retain canonical compiler-derived offsets.
template <typename T, typename P>
inline T& FLD(P* p, u32 off) { return *reinterpret_cast<T*>(reinterpret_cast<u8*>(const_cast<void*>(static_cast<const void*>(p))) + off); }

static inline MarioConstTable* marioCurTableEntry(Mario* self) {
    return self->mActor->mConst->getTable();
}

namespace {
    const f32 cSquatWalkStoppedWeights[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    const f32 cSquatWalkMovingWeights[4] = {0.0f, 1.0f, 0.0f, 0.0f};
}

// Mario definitions in target executable order.

f32 fn_80386D30(f32 first, f32 second, f32 third) {
    f32 largest = first >= second ? first : second;
    return largest >= third ? largest : third;
}

Mario::Mario(MarioActor* pActor)
    : MarioModule(pActor), mMovementStates(), mFlags14(), mDrawStates(), mFlags20(), mFlags24(), mPrevDrawStates() {
    initMember();
    mVelocity.zero();
    _13C.zero();
    _148.zero();
    _154.zero();
    mFrontVec.set(1.0f, 0.0f, 0.0f);
    _220 = mFrontVec;
    _43C = mFrontVec;
    _214.set(1.0f, 0.0f, 0.0f);
    _22C.set(1.0f, 0.0f, 0.0f);
    _3E4.set(1.0f, 0.0f, 0.0f);
    _2B4 = 0.0f;
    mSideVec.set(0.0f, 0.0f, 1.0f);
    _290 = mSideVec;
    mAirGravityVec.set(0.0f, -1.0f, 0.0f);
    _1E4.zero();
    bool nearZero = MR::isNearZero(mAirGravityVec);
    mHeadVec = -mAirGravityVec;
    _1FC = mHeadVec;
    _938 = mHeadVec;
    _944 = mFrontVec;
    _950 = mSideVec;
    _60D = false;
    mTargetWalkSpeedIndex = 0;
    mWalkSpeed = 0.0f;
    _2D0 = 0.0f;
    mJumpVec.zero();
    _2E0.zero();
    _2EC.zero();
    _42C = 0;
    _2F8.zero();
    _328.zero();
    _334.zero();
    mVerticalSpeed = 0.0f;
    _430 = 0;
    _3BC = 0;
    _3BE = 0;
    _3C0 = 0;
    _3CE = 0;
    _3CC = 0;
    _3CA = 0;
    _3D0 = 0;
    _3D4 = 0;
    _424 = 0;
    _434 = 0;
    _436 = 0;
    _428 = 0;
    _3D2 = 0;
    _404 = 0;
    _408 = 0;
    _40A = 0;
    _426 = 0;
    _418 = 0;
    _41A = 0;
    _774 = 0;
    _3C2 = 0;
    _3C4 = 0;
    _3C6 = 0;
    _3C8 = 0;
    _7D0 = 0x78;
    _524 = 0.0f;
    _528 = 0.0f;
    _52C = 0.0f;
    _530 = 0.0f;
    _534 = 0;
    _538 = 0.0f;
    _53C = 0.0f;
    _544 = 0;
    mYAngleOffset = 0.0f;
    _3F8 = 0;
    _3FA = 0;
    _3FC = 0;
    _3FE = 0;
    _400 = 0;
    _402 = 0;
    mPrevAnimFrame = 0.0f;
    mIceAnimFoot = 0;
    mSinkTimer = 0;
    mPoisonTimer = 0;
    _71E = 0;
    _414 = 0;
    mMorphResetTimer = 0;
    _41C = 0;
    _41E = 0;
#ifdef GALAXY_NATIVE
    // Preserve native compatibility initialization; the Wii constructor leaves these fields untouched.
    _410 = 0;
#endif
    _420 = 0;
    _43A = 0;
    _3F4 = 0.0f;
    _3F0 = 1.0f;
    _58.zero();
    _12C = 0;
    _76C = 0;
    _770 = 0.0f;
    _738 = 0.0f;
    _73C.set(0.0f, 1.0f, 0.0f);
    _368.set(0.0f, 1.0f, 0.0f);
    _374 = -_368;
    _380 = _368;
    _38C = -_368;
    _398.set(0.0f, 1.0f, 0.0f);
    _344.set(1.0f, 0.0f, 0.0f);
    _74C = 0.0f;
    _750 = 0;
    _754 = 0;
    _40C = 0;
    _40E = 0;
    _406 = 0;
#ifdef GALAXY_NATIVE
    _412 = 0;
#endif
    _558 = 0;
    _2A8 = -_368;
    _29C = _368;
    _548 = _128 = _124 = 0.0f;
    _54C = mHeadVec;
    PSMTXIdentity(_C4);
    PSMTXIdentity(_64);
    PSMTXIdentity(_94);
    clearSlope();
    _910.zero();
    _91C.zero();
    _928.zero();
    _934 = 0.8f;
    _3C = -0.515f;
    mMovementStates.mLowWord = 0;
    mMovementStates.mHighWord = 0;
    mFlags14.mLowWord = 0;
    mFlags14.mHighWord = 0;
    mDrawStates.mWord = 0;
    mFlags20.mWord = 0;
    mFlags24.mLowWord = 0;
    mFlags24.mHighWord = 0;
    _28 = 0;

    _458 = MR::createTriangleFilterDelegator(this, &Mario::isIgnoreTriangle);

    _45C = new Triangle();
    _460 = new Triangle();
    mGroundPolygon = new Triangle();
    _468 = new Triangle();
    _46C = new Triangle();

    _4C8 = new Triangle();
    mFrontWallTriangle = new Triangle();
    mBackWallTriangle = new Triangle();
    mSideWallTriangle = new Triangle();
    _4D8 = new Triangle();
    _4DC = new Triangle();

    _470 = new Triangle();
    _474 = new Triangle();
    mTmpPolygon = new Triangle();

    _7E0 = new Triangle();

    _820 = new Triangle();

    _47C = new Triangle();
    _480 = new Triangle();
    _484 = new Triangle();

    for (u32 i = 0; i < ARRAY_SIZE(_57C); i++) {
        _57C[i] = new Triangle();
    }

    _578 = 0;

    _4E8.zero();
    _4F4.zero();

    _7D4.zero();
    _8A4.zero();

    _8C8 = new Triangle();

    _8E8 = new Triangle();

    _8CC[0] = new Triangle();
    _8CC[1] = new Triangle();

    _564 = -1;
    _56C = 0;
    _570 = 0;

    _4A4.zero();
    _4B0.zero();

    _25C.zero();
    _268.zero();

    _350.zero();
    _35C.zero();

    _184.zero();

    _1A8.zero();

    _190.zero();
    _19C.zero();

    _4E4 = 0.0f;
    _4E0 = 0.0f;

    _72C = 200.0f;

    _730 = nullptr;

    _8D8 = nullptr;
    _8D4 = nullptr;

    mTornadoCenterSensor = nullptr;
    _A68 = 0;

    _5FC = 0;
    mPreviousCometRestrictions = 0;
    mCometRestrictions = 0;

    mWorldPadDir.zero();

    _38 = 0.0f;
    mStickPos.z = 0.0f;
    mStickPos.y = 0.0f;
    mStickPos.x = 0.0f;

    _3D8.zero();

    _244.zero();
    _238.zero();
    _250.zero();

    clear2DStick();

    mAxisMode6A4 = 0;
    mCandidateAxisMode6A5 = 0;
    _6B0.zero();
    _6BC.zero();

    mSide2D.set(1.0f, 0.0f, 0.0f);
    mUp2D.set(0.0f, 1.0f, 0.0f);
    mNormal2D.set(0.0f, 0.0f, 1.0f);
    mSwimSideVec694 = mNormal2D;

    initSound();
    mAirWalkRecoverySoundArmed = 0;
    mJumpFallTimer = 0;

    _97C = 0;

    mFlow = new MarioFlow(mActor);
    mWall = new MarioWall(mActor);
    mDamage = new MarioDamage(mActor);
    mFaint = new MarioFaint(mActor);
    mBlown = new MarioBlown(mActor);
    mHang = new MarioHang(mActor);
    mSwim = new MarioSwim(mActor);
    mSlider = new MarioSlider(mActor);
    mFireDamage = new MarioFireDamage(mActor);
    mFireRun = new MarioFireRun(mActor);
    mFireDance = new MarioFireDance(mActor);
    mAbyssDamage = new MarioAbyssDamage(mActor);
    mDarkDamage = new MarioDarkDamage(mActor);
    mStep = new MarioStep(mActor);
    mBump = new MarioBump(mActor);
    mParalyze = new MarioParalyze(mActor);
    mStun = new MarioStun(mActor);
    mCrush = new MarioCrush(mActor);
    mFreeze = new MarioFreeze(mActor);
    mMagic = new MarioMagic(mActor);
    mFpView = new MarioFpView(mActor);
    mRecovery = new MarioRecovery(mActor);
    mFlip = new MarioFlip(mActor);
    mSideStep = new MarioSideStep(mActor);
    mFrontStep = new MarioFrontStep(mActor);
    mStick = new MarioStick(mActor);
    mRabbit = new MarioRabbit(mActor);
    mSukekiyo = new MarioSukekiyo(mActor);
    mBury = new MarioBury(mActor);
    mWait = new MarioWait(mActor);
    mClimb = new MarioClimb(mActor);
    mSkate = new MarioSkate(mActor);
    mFoo = new MarioFoo(mActor);
    mWarp = new MarioWarp(mActor);
    mTeresa = new MarioTeresa(mActor);
    mTalk = new MarioTalk(mActor);
    mYoshiPull = new MarioYoshiPull(mActor);
    mYoshiBalloon = new MarioYoshiBalloon(mActor);
    mYoshiShot = new MarioYoshiShot(mActor);
    mYoshiDash = new MarioYoshiDash(mActor);
    mMove = new MarioMove(mActor);
    mLifeAlertCounter = nullptr;
    _A98 = 0;

    initTask();
    _95C = new FloorCode();
    _960 = 0;
    _962 = 0;
    for (int i = 0; i < ARRAY_SIZE(_964); i++) {
        _964[i] = _960;
    }

    _96A = 0;
    _574 = 0;
    _898 = false;
}

void Mario::initAfterConst() {
    MarioConst* pConst = mActor->mConst;
    _402 = pConst->getTable()->mAirWalkTime;
    mPosHolder->fn_80405F90(mActor->mPosition);
    mShadowPos = mPosHolder->mPos;
    mGroundPos = mPosHolder->mPos;
    mMove->initAfter();
    mFoo->init();
    mSwim->init();
    _7D4 = mPosHolder->mPos;
    _814 = mPosHolder->mPos;
    PSMTXIdentity(_7E4);
    PSMTXIdentity(_824);
    _720 = getAnimationStringPointer("ヒップドロップ開始");
    _724 = getAnimationStringPointer("ヒップドロップ");
    _728 = getAnimationStringPointer("ヒップドロップ着地");
}

void Mario::updateAndClearStrideParameter() {
    _43C = mFrontVec;
    _16C = mVelocity;
    mPosHolder->fn_80405F90(mActor->mPosition);
    mVelocityAfter.zero();
    mVelocity.zero();
    mFlags24.mLowWord = mMovementStates.mLowWord;
    mFlags24.mHighWord = mMovementStates.mHighWord;
    _28 = mFlags14.mLowWord;
    _2C = mFlags14.mHighWord;
    mPrevDrawStates = mDrawStates;
    _34 = mFlags20.mWord;
    mDrawStates.mWord = 0;
    mFlags20.mWord = 0;
    mMovementStates.mHighBits.mBit23 = 0;
    if (((mMovementStates.mLowWord >> 30) & 1U)) {
        mWall->_1C = 0;
    }

    if (((mFlags14.mHighWord >> 26) & 1U)) {
        mFlags20.mBits.mBit30 = 1;
    }

    if (((mFlags14.mHighWord >> 25) & 1U)) {
        mFlags20.mBits.mBit29 = 1;
    }

    if (((mFlags14.mHighWord >> 27) & 1U)) {
        mFlags20.mBits.mBit31 = 1;
    }

    mFlags14.mHighBits.mBit26 = 0;
    mFlags14.mHighBits.mBit25 = 0;
}

void Mario::checkKeyLock() {
    if (!((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        mMovementStates.mHighBits.mBit29 = 0;
    }

    if (mActor->_37C < 0x4b) {
        mMovementStates.mHighBits.mBit29 = 1;
    }

    if (mActor->isEnableNerveChange() == false) {
        mMovementStates.mHighBits.mBit29 = 1;
    }

    if (isSwimming()) {
        mMovementStates.mHighBits.mBit29 = 0;
    }

    if (MR::testSubPadButtonC(WPAD_CHAN0)) {
        mDrawStates.mBits.mBit24 = 1;
    }
}

// Regswap
void Mario::updateTimers() {
    if (((mMovementStates.mLowWord >> 30) & 1U)) {
        _3CE++;
    }

    if (_414 != 0) {
        _414--;
    }

    if (_434 != 0) {
        _434--;
    }

    if (mHang->mHangTimer != 0) {
        mHang->mHangTimer--;
    }
}

void Mario::updateMorphResetTimer() {
    u16 timer = mMorphResetTimer;
    if (timer == 0) {
        return;
    }

    timer--;
    mMorphResetTimer = timer;
    if (timer != 0) {
        return;
    }

    mActor->setPlayerMode(0, true, true);
    return;
}

void Mario::doExtraServices() {
    TVec3f stack_2c;
    TVec3f stack_20;
    if (MR::diffAngleAbs(_43C, mFrontVec) < mActor->mConst->getTable()->mFrontAngleFixMargin) {
        setFrontVec(_43C);
    }

    bool notBee = !isPlayerModeBee();
    if (notBee) {
        bool notTeresa = !isPlayerModeTeresa();
        if (notTeresa && (static_cast<u32>(mMovementStates.mLowBits.mBit31)) && _3BC > 600 && !MR::isNearZero(*getGravityVec())) {
            TVec3f vec(*getGravityVec() * MR::vecKillElement(mJumpVec, *getGravityVec(), &stack_2c));
            mJumpVec = vec;
        }
    }

    bool notFoo = !isPlayerModeFoo();
    if (notFoo) {
        bool notTeresa = !isPlayerModeTeresa();
        if (notTeresa && (static_cast<u32>(mMovementStates.mLowBits.mBit31)) && _3BC > 300) {
            TVec3f stack_8(mPosHolder->mPos - *getLastSafetyTrans(nullptr));

            if (MR::isNearZero(mAirGravityVec)) {
            }

            if (MR::vecKillElement(stack_8, mAirGravityVec, &stack_20) > 28000.0f) {
                if (mVerticalSpeed > 2499.0f || mActor->fn_803B93E0()) {
                    mActor->forceKill(0);
                }
            }
        }
    }

    if (getCurrentStatus() != 0 || mActor->_3C0 != 0 || mActor->_EA4) {
        mFlags14.mLowBits.mBit19 = 0;
    }
}

void Mario::writeBackPhyisicalVector() {
    fn_803880D0();
    fn_80388150();
    if ((mFlags14.mLowWord >> 12) & 1) {
        TVec3f pos;
        MR::calcPlayerScaledVec(&pos, mPosHolder->mPos, _6F4, _700, _718);
        const TVec3f& oldPos = mPosHolder->mPos;
        TVec3f delta(pos);
        delta.sub(oldPos);
        mPosHolder->fn_80406010(delta);
    }
    if (((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        fn_803882A0();
    } else if (isSwimming()) {
        fn_803885F0();
    } else {
        fn_803886B0();
    }
    mPosHolder->fn_80405EF0(getCurrentStatus());
    mActor->mVelocity = mVelocity;
    mActor->mPosition = mPosHolder->mPos;
}

void Mario::fn_803880D0() {
    if (static_cast<u32>(this->mFlags20.mBits.mBit15)) {
        this->mPosHolder->fn_80406040();
        this->addTrans(this->mVelocity);
        if (this->calcDistToCeil(false) < 200.0f) {
            this->mVelocity.zero();
            this->stopWalk();
        }
        this->mPosHolder->fn_80406080();
    }
}

void Mario::fn_80388150() {
    if ((static_cast<u32>(this->mMovementStates.mHighBits.mBit8)) || (static_cast<u32>(this->mFlags14.mLowBits.mBit10))) {
        TVec3f sp38(this->mPosHolder->mPos);
        TVec3f sp2C;
        sp38 -= this->_688;
        MR::vecKillElement(sp38, this->mSwimSideVec694, &sp38);
        TVec3f* temp_r31 = &this->mPosHolder->mPos;
        TVec3f sp20(TVec3f(this->_688 + sp38));
        sp20 -= *temp_r31;
        this->mPosHolder->fn_80406010(sp20);
        MR::vecKillElement(this->mVelocity, this->mSwimSideVec694, &this->mVelocity);
        f32 temp_f1 = MR::vecKillElement(this->_694 - this->_688, this->mSwimSideVec694, &sp2C);
        f32 var_f31 = temp_f1;
        if (temp_f1 > 1.0f) {
            var_f31 = 1.0f;
        } else if (temp_f1 < -1.0f) {
            var_f31 = -1.0f;
        }
        sp2C = this->mSwimSideVec694;
        sp2C.setLength(var_f31);
        this->_688 += sp2C;
    }
}

// NONMATCHING: saved registers and temporary allocation; recovered full collision response.
namespace {
    inline bool queryMarioWallPolygon(Mario* mario, TVec3f* hitPos, Triangle* triangle, const TVec3f& origin, const TVec3f& ray) {
        if (mario->fn_803B2950()) {
            return MR::getFirstPolyOnLineToDashYoshiStepAndMap(hitPos, triangle, origin, ray);
        }
        return MR::getFirstPolyOnLineToMap(hitPos, triangle, origin, ray);
    }
}

void Mario::fn_803882A0() {
    s32 rising = 0;
    if (isRising()) {
        rising = 1;
    }
    if (mActor->_334 != 0) {
        this->_578 = 0;
    }
    if (!(static_cast<u32>(this->mMovementStates.mHighBits.mBit29))) {
        for (u32 i = 0; i < this->_578; i++) {
            TVec3f clipped;
            if (MR::vecKillElement(this->mVelocity, *this->_57C[i]->getNormal(0), &clipped) < 0.0f) {
                this->mVelocity = clipped;
                if (!(static_cast<u32>(this->mFlags20.mBits.mBit18))) {
                    TVec3f projected;
                    f32 depth = MR::vecKillElement(this->mJumpVec, *this->_57C[i]->getNormal(0), &projected);
                    if (depth <= 0.0f) {
                        this->mJumpVec = projected + (*this->_57C[i]->getNormal(0) * depth) * 0.5f;
                    }
                } else if (MR::diffAngleAbs(*getGravityVec(), this->mJumpVec) < 0.10471976f) {
                    TVec3f tangent;
                    MR::vecKillElement(*this->_57C[i]->getNormal(0), *getGravityVec(), &tangent);
                    f32 length = PSVECMag(&this->mJumpVec);
                    MR::normalizeOrZero(&tangent);
                    this->mJumpVec += tangent;
                    this->mJumpVec.setLength(length);
                }
                if (rising) {
                    if (this->_57C[i]->getNormal(0)->dot(*getGravityVec()) > 0.707f) {
                        mActor->sendMsgUpperPunch(this->_57C[i]->mSensor);
                    }
                }
                if (isPlayerModeBee() && mActor->mBeeWallWalk == 0) {
                    const char* code = MR::getWallCodeString(this->_57C[i]);
                    if (code && strcmp(code, "Fur") == 0) {
                        Triangle hitTriangle;
                        TVec3f hitPos;
                        bool hit = queryMarioWallPolygon(this, &hitPos, &hitTriangle, mActor->_240,
                                                         -*this->_57C[i]->getNormal(0) * 300.0f);
                        if (hit) {
                            const char* hitCode = MR::getWallCodeString(&hitTriangle);
                            if (hitCode && strcmp(hitCode, "Fur") == 0) {
                                MarioActor* actor = getActor();
                                actor->entryWallWalkMode(hitPos, *hitTriangle.getNormal(0));
                            }
                        }
                    }
                }
            }
        }
    }
}

void Mario::fn_803885F0() {
    if (!(static_cast<u32>(this->mMovementStates.mHighBits.mBit29))) {
        f32 temp_f31 = 0.0f;
        u32 var_r29 = 0U;
        while (var_r29 < this->_578) {
            Triangle* tri = this->_57C[var_r29];
            TVec3f sp8;
            if (MR::vecKillElement(this->mVelocity, *tri->getNormal(0), &sp8) < temp_f31) {
                this->mVelocity = sp8;
                this->mSwim->fn_803F7C20(tri->getNormal(0));
            }
            var_r29 += 1;
        }
    }
}

// NONMATCHING: 0x274 bookkeeping approximated
// NONMATCHING: equivalent early-exit branch orientation and stack allocation.
void Mario::fn_803886B0() {
    if (static_cast<u32>(mFlags20.mBits.mBit19) || mActor->mBinderDirection5DC.dot(_368) >= 0.707f) {
        return;
    }
    if (MR::isNearZero(mActor->mBinderDirection5DC, 1.0f)) {
        return;
    }
    TVec3f direction(mActor->mBinderDirection5DC);
    MR::normalizeOrZero(&direction);
    TVec3f horizontal;
    f32 component = MR::vecKillElement(mVelocity, direction, &horizontal);
    if (component >= 0.0f) {
        return;
    }
    mVelocity = horizontal;
    MR::normalizeOrZero(&direction);
    f32 dot = direction.dot(mFrontVec);
    f32 weight = 1.0f - MR::clamp(1.4285715f * (-dot - 0.3f), 0.0f, 1.0f);
    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit23)) && weight < 0.1f) {
        if ((static_cast<u32>(mMovementStates.mLowBits.mBit5))) {
            mVelocity += getSideWallNorm() * -component;
        } else {
            s32 count = mActor->mBinder->mPlaneNum;
            if (count != 0) {
                f32 bestDot = 0.0f;
                u32 bestIndex = 0;
                for (u32 i = 0; i < count; i++) {
                    f32 facing = mActor->mBinder->getPlane(i)->mParentTriangle.getNormal(0)->dot(mFrontVec);
                    if (facing < bestDot) {
                        bestIndex = i;
                        bestDot = facing;
                    }
                }
                const TVec3f& contact = mActor->mBinder->getPlane(bestIndex)->mHitPos;
                TVec3f correction;
                MR::vecKillElement(mPosHolder->mPos - contact, *getGravityVec(), &correction);
                MR::vecKillElement(correction, direction, &correction);
                if (!MR::isNearZero(correction)) {
                    correction.setLength(-component);
                    mVelocity += correction;
                }
            }
        }
    }
    mTargetWalkSpeedIndex = 0;
    mWalkSpeed *= weight;
}

// NONMATCHING: register/load scheduling.
void Mario::update() {
    if (MR::isInAreaObj("DemoPlayerForbidUpdateArea", mPosHolder->mPos) && MR::isDemoActive()) return;
    OSGetTime();
    mPosHolder->fn_80405EB0();
    updateAndClearStrideParameter();
    checkKeyLock();
    u32 flags = mMovementStates.mLowWord;
    if (!(flags >> 31) || !((flags >> 20) & 1)) {
        if (_728 != nullptr && isAnimationRun(_728)) mMovementStates.mLowWord |= 0x80100000;
    }
    updateCubeCode();
    updateCometCode();
    { const u32 flagValue = static_cast<u32>(updateBinderInfo()); mMovementStates.mLowBits.mBit24 = flagValue; }
    checkEnforceMoveInner();
    updateOnimasu();
    if (!isStatusActive(22) && !isStatusActive(5)) {
        checkBaseTransPoint();
        checkHeadPoint();
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit16)) && _544 > 1) createAtField(true, 150.0f);
    else createAtField(false, 40.0f);
    _72C = calcDistToCeil(true);
    if (isPlayerModeYoshi() && fn_803B1850()) { const u32 flagValue = static_cast<u32>(_72C < 200.0f); mFlags20.mBits.mBit16 = flagValue; }
    else { const u32 flagValue = static_cast<u32>(_72C < 160.0f); mFlags20.mBits.mBit16 = flagValue; }
    if (static_cast<u32>(mFlags20.mBits.mBit16)) {
        if (_4C8->isValid()) damagePolygonCheck(_4C8);
    }
    if (MR::getKarikariClingNum() != 0 && mActor->isEnableNerveChange()) mFlags20.mBits.mBit26 = 1;
    OSGetTime();
    checkAllWall(mActor->_240);
    OSGetTime();
    updateGroundInfo();
    OSGetTime();
    if ((static_cast<u32>(getMovementStates().mLowBits.mBit30)) && (!(static_cast<u32>(mMovementStates.mLowBits.mBit31)) || (static_cast<u32>(mMovementStates.mLowBits.mBit20)))) {
        if (damageFloorCheck()) {
            writeBackPhyisicalVector();
            return;
        }
        saveLastSafetyTrans();
        if (isCurrentShadowFloorDangerAction()) {
            TVec3f projected;
            TVec3f groundToShadow(mShadowPos);
            groundToShadow -= mGroundPos;
            MR::isNearZero(mAirGravityVec, 0.001f);
            if (MR::vecKillElement(groundToShadow, mAirGravityVec, &projected) >= 5.0f && groundToShadow.dot(mFrontVec) > 0.0f) {
                mDrawStates.mBits.mBit21 = 1;
                groundToShadow.setLength(5.0f);
                addVelocity(groundToShadow);
            }
        }
    }
    mDrawStates.mBits.mBit15 = 1;
    if (damageWallCheck()) return;
    mDrawStates.mBits.mBit15 = 0;
    OSGetTime();
    if (checkPressDamage()) return;
    OSGetTime();
    if (checkSliderMode()) startSlider();
    checkAndTryForceJump();
    if (static_cast<u32>(mMovementStates.mHighBits.mBit17)) return;
    if (checkStartSwim()) {
        writeBackPhyisicalVector();
        return;
    }
    if (!(static_cast<u32>(mMovementStates.mHighBits.mBit29))) check2DMode();
    fn_803896E0();
    inputStick();
    checkLockOnHoming();
    fn_80388DB0();
    fn_8038C620();
    fn_803929A0();
    fn_803897A0();
    tryPushToVelocity();
    powerAreaMove();
    powerRailMove();
    addVelocity(mVelocityAfter);
    checkForceGrounding();
    checkStep();
    checkBump();
    doCubeWarp();
    if (isSwimming()) checkBaseTransBall();
    doSpinPunchAroundPolygons();
    writeBackPhyisicalVector();
    updateTimers();
    doExtraServices();
}

void Mario::fn_80388DB0() {
    if (!(static_cast<u32>(this->mMovementStates.mLowBits.mBit9))) {
        if (this->checkDamage() == false) {
            if (this->isPlayerModeYoshi()) {
                this->mActor->fn_803B9810()->fn_8041DF40();
            }
            if (!this->isPlayerModeYoshi() || this->mActor->fn_803B9810()->fn_8041F8A0() == 0) {
                if (_97C != nullptr) {
                    this->sendStateMsg(2);
                } else if ((static_cast<u32>(this->mMovementStates.mLowBits.mBit31)) != 0) {
                    this->procJump(false);
                    this->checkTornado();
                    this->checkHang();
                    this->checkWallStick();
                    this->fn_803B0E60();
                } else {
                    this->fn_803A0BE0();
                    this->updateWalkSpeed();
                    this->fn_803B0E60();
                }
            }
            this->fn_803B3CE0();
            if (static_cast<u32>(this->mMovementStates.mLowBits.mBit30)) {
                this->mMovementStates.mHighWord |= 0xC000;
                this->doFrontStep();
            }
            if (this->isStatusActive(0x28)) {
                this->mActor->fn_803B9A30();
                mFlags14.mHighBits.mBit21 = 0;
                this->mActor->fn_803B9830()->fn_8041BB90();
                return;
            }
            u32 t = this->mMovementStates.mLowWord;
            if (((t >> 30) & 1) || !((t >> 12) & 1)) {
                mFlags14.mHighBits.mBit20 = 0;
            }
        }
        this->fn_803A7540();
        if (this->getPlayerMode() != 6 && (static_cast<u32>(this->mMovementStates.mLowBits.mBit23)) && this->mFrontWallTriangle->isValid() &&
            this->mFrontWallTriangle->getNormal(0)->dot(this->mVelocity) < 0.0f) {
            MR::vecKillElement(this->mVelocity, *this->mFrontWallTriangle->getNormal(0), &this->mVelocity);
        }
        if (this->mActor->fn_803B9840() != 0) {
            this->mActor->fn_803B9830()->fn_8041BBB0();
        }
        if (this->isStickOn()) {
            if (this->isAnimationRun("水泳陸うちあげ")) {
                this->stopAnimation((const char*)nullptr, (const char*)nullptr);
            }
        }
        if (!(static_cast<u32>(this->mDrawStates.mBits.mBit27))) {
            this->_3FE = 0;
        }
        if (MR::isGalaxyHorrorCometAppearInCurrentStage()) {
            bool restrictionsAdded = false;
            if (mCometRestrictions != 0 && mPreviousCometRestrictions == 0) {
                restrictionsAdded = true;
            }
            if (restrictionsAdded) {
                MR::addCometTimer(3600);
                MR::pauseCometTimer();
            }
            bool restrictionsRemoved = false;
            if (mCometRestrictions == 0 && mPreviousCometRestrictions != 0) {
                restrictionsRemoved = true;
            }
            if (restrictionsRemoved) {
                MR::resumeCometTimer();
            }
        }
        mPreviousCometRestrictions = mCometRestrictions;
        this->callExtraTasks(0);
        this->beeMarioOnGround();
        this->checkSpecialWaitAnimation();
        this->fn_803B4280();
        if (!(static_cast<u32>(this->mDrawStates.mBits.mBit9))) {
            this->retainMoveDir(mStickPos.x, mStickPos.y);
        }
    }
}

bool Mario::fn_803890B0() {
    if (this->isStatusActive(6)) {
        return false;
    }
    return !this->isStatusActive(0x27);
}

void Mario::setGroundNorm(const TVec3f& rVec) {
    TVec3f opposite(-rVec);
    if (MR::isNearZero(mAirGravityVec)) {
    }

    if (MR::diffAngleAbs(opposite, mAirGravityVec) > 1.4959966f) {
    } else {
        _368 = rVec;
        _374 = -rVec;
    }
}

void Mario::updateGroundInfo() {
    checkMap();
    if (fn_803890B0()) {
        bool b1 = !(static_cast<u32>(mMovementStates.mLowBits.mBit31)) && !(static_cast<u32>(mMovementStates.mLowBits.mBit30));

        { const u32 flagValue = static_cast<u32>(checkGround()); mMovementStates.mLowBits.mBit30 = flagValue; }

        if ((static_cast<u32>(mDrawStates.mBits.mBit7))) {
            _3C6++;
        } else {
            _3C6 = 0;
        }

        if (b1 && (static_cast<u32>(mMovementStates.mLowBits.mBit30)) && isDefaultAnimationRun("落下")) {
            changeAnimation(nullptr, "基本");
        }

        if (!isStatusActive(MarioStatus_13)) {
            if (!isStatusActive(MarioStatus_13) && !(static_cast<u32>(mMovementStates.mLowBits.mBit30)) && ((mMovementStates.mHighWord >> 0) & 3U) == 1 && mJumpVec.dot(*getGravityVec()) > 0.0f &&
                mJumpVec.dot(*_45C->getNormal(0)) < 0.0f && _1FC.dot(*getGravityVec()) > 0.0f && mVerticalSpeed < 170.0f) {
                mMovementStates.mLowBits.mBit30 = 1;
                setGroundNorm(*_45C->getNormal(0));
                setTrans(mShadowPos);
            }
        }
    }

    updateCameraPolygon();

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        mJumpFallTimer = 0;
        mMovementStates.mHighBits.mBit22 = 0;
        if (_43A == 0 && isSlipPolygon(mGroundPolygon)) {
            mDrawStates.mBits.mBit19 = 1;
        }
    }

    if (_43A != 0) {
        _43A--;
    }

    updateFloorCode();
    updateSoundCode();
    updateOnSand();
    updateOnWater();
    updateOnPoison();

    if (mHang != nullptr && (static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        mHang->mHangTimer = 0;
    }
}

void Mario::checkForceGrounding() {
    if (((static_cast<u32>(mMovementStates.mHighBits.mBit8)) || (static_cast<u32>(mFlags14.mLowBits.mBit10))) && (static_cast<u32>(mMovementStates.mLowBits.mBit30)) && MR::diffAngleAbs(_368, *_45C->getNormal(0)) < MR::toRadian(45.0f)) {
        mDrawStates.mBits.mBit31 = 1;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mLowBits.mBit6)) || (static_cast<u32>(mMovementStates.mLowBits.mBit5))) {
        return;
    }

    // does not match if this if statement is an early return
    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit10)) && !(static_cast<u32>(mMovementStates.mHighBits.mBit28)) && !((mMovementStates.mLowWord & Mario::MovementStates::DebugMode) >> 9) && (static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        if (!(static_cast<u32>(mDrawStates.mBits.mBit31))) {
            TVec3f vec;

            f32 dot = getGravityVec()->dot(-_368);

            if (dot > 0.99f) {
                vec = *getGravityVec();
            } else {
                vec = -_368;
            }

            f32 dot2 = MR::vecKillElement(mVelocity, vec, &mVelocity);

            dot *= (mShadowPos - mPosHolder->mPos).dot(vec);

            if (MR::isNearZero(dot)) {
                return;
            }

            if (!(MR::abs(dot) < 30.0f)) {
                return;
            }

            if (dot2 > dot) {
                dot2 = dot;
            }

            mVelocity += vec * dot2;

            return;
        }

        TVec3f vec2;
        f32 dot3 = MR::vecKillElement(mVelocity, *getGravityVec(), &vec2);
        f32 f4 = vec2.length();

        if (mVerticalSpeed && dot3 > mVerticalSpeed) {
            dot3 = mVerticalSpeed;
        }

        if (!(static_cast<u32>(mDrawStates.mBits.mBit27))) {
            f4 = MR::abs(dot3) > f4 ? 0.0f : MR::sqrt(f4 * f4 - dot3 * dot3);
        }

        MR::normalizeOrZero(&vec2);

        mVelocity = (vec2 * f4) + *getGravityVec() * dot3;
    }
}

void Mario::fn_803896E0() {
    this->_898 = 0;
    if (!(static_cast<u32>(this->mMovementStates.mLowBits.mBit9)) && MR::testFpViewStartTrigger()) {
        if (MR::isPossibleToShiftToFirstPersonCamera() == false) {
            if (!MR::isDemoActive()) {
                if (!MR::isStageStoryBook() && !MR::isStageFileSelect() && !MR::isStageEpilogueDemo() && !MR::isStageStaffRoll()) {
                    MR::startSystemSE("SE_SY_CAMERA_NG", -1, -1);
                }
                this->_898 = 1;
            }
        } else {
            this->tryFpViewMode();
        }
    }
}

void Mario::fn_803897A0() {
    u32 temp_r4 = this->mMovementStates.mHighWord;
    if (!((temp_r4 >> 28) & 1) || !(static_cast<u32>(this->mMovementStates.mLowBits.mBit30))) {
        return;
    }
    if (((temp_r4 >> 27) & 1) && this->isSlipPolygon(this->_460)) {
        const TVec3f* nrm = this->_460->getNormal(0);
        f32 temp_f31 = MR::vecKillElement(this->mVelocity, *nrm, &this->mVelocity);
        nrm = this->_460->getNormal(0);
        TVec3f sp14(this->_368);
        sp14 -= *nrm;
        TVec3f sp20(sp14);
        sp20 *= 0.5f;
        f32 temp_f31_2 = MR::abs(temp_f31);
        TVec3f sp8(sp20);
        sp8 *= temp_f31_2;
        this->mVelocity += sp8;
    }
}

// NONMATCHING: independent load scheduling and temporary/register allocation.
void Mario::fixHeadFrontVecByGravity() {
    TVec3f groundNormal(_368);
    MR::diffAngleAbs(mActor->mCameraBasisZ, mActor->mPostureReference1044);
    if (MR::isCameraInterpolationStateReady() && isStickOn()) {
        if (_60D != 1) _60D = 1;
        mDrawStates.mBits.mBit18 = 1;
        mFlags14.mLowBits.mBit21 = 1;
        _40E = 0;
    }
    if (_3D0 != 0 || _3D2 != 0) mDrawStates.mBits.mBit18 = 0;
    if (!_60D && mActor->mAxisRotationDisabled6A0 != 0) mDrawStates.mBits.mBit18 = 0;
    if (MR::isInAreaObj("PlayerSideRetainOffArea", getTrans())) mDrawStates.mBits.mBit18 = 0;
    if ((isStatusActive(30) || isStatusActive(29)) && _60D) _60D = 0;
    TVec3f previousHead(mHeadVec);
    TVec3f up(-*getGravityVec());
    MR::normalize(&up);
    if (mHeadVec.dot(up) < -0.9f && (static_cast<u32>(mMovementStates.mLowBits.mBit31))) this->_A71 = 1;
    else if (static_cast<u32>(mMovementStates.mLowBits.mBit30)) this->_A71 = 0;
    f32 rate = fn_803B2CC0(&up);
    if (_60D && mTargetWalkSpeedIndex != 0 && ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) || !(static_cast<u32>(mMovementStates.mLowBits.mBit31)))) {
        if (!(static_cast<u32>(mDrawStates.mBits.mBit18)) || !fn_803B2BD0()) {
            fn_803B23E0(&up, &mSideVec, rate);
            fixFrontVecFromUpSide();
            _29C = groundNormal;
            _290 = mSideVec;
        }
    } else {
        if (this->_A71 != 0) {
            fn_803B1EF0(&up, rate);
            setFrontVecKeepUp(mFrontVec);
        } else if (fn_803B1DE0(&up)) {
            fn_803B23E0(&up, &mFrontVec, rate);
            setFrontVecKeepUp(mFrontVec);
        } else {
            fn_803B23E0(&up, &mSideVec, rate);
            fixFrontVecFromUpSide();
        }
        MarioConst* constants = mActor->mConst;
        MarioConstTable* table = constants->mTable[constants->mCurrentTable];
        if (MR::diffAngleAbs(_43C, mFrontVec) < table->mFrontAngleFixMargin) setFrontVec(_43C);
        constants = mActor->mConst;
        table = constants->mTable[constants->mCurrentTable];
        if (MR::diffAngleAbs(mHeadVec, previousHead) < table->mHeadAngleFixMargin) {
            mHeadVec = previousHead;
            MR::normalize(&mHeadVec);
        }
        if (fn_803B2470()) {
            mSideVec = _290;
            MR::normalize(&mSideVec);
            f32 angle = marioAcos(groundNormal.dot(_29C));
            TVec3f axis;
            PSVECCrossProduct(&_29C, &groundNormal, &axis);
            MR::normalizeOrZero(&axis);
            if (MR::isNearZero(axis, 0.001f) || MR::isNearZero(_29C + groundNormal, 0.001f) || MR::isNearZero(_29C - groundNormal, 0.001f)) {
                if (_60D != 1) _60D = 1;
            } else {
                if (!MR::isNearZero(axis, 0.001f)) {
                    TMtx34f rotation;
                    PSMTXRotAxisRad(rotation, axis, angle);
                    PSMTXMultVec(rotation, mSideVec, mSideVec);
                    _29C = groundNormal;
                    _290 = mSideVec;
                }
                if (_60D != 0) _60D = 0;
            }
            fixFrontVecFromUpSide();
            return;
        }
        TVec3f axis;
        PSVECCrossProduct(&groundNormal, &_22C, &axis);
        MR::normalizeOrZero(&axis);
        if (!MR::isNearZero(axis, 0.001f)) _290 = axis;
        else _290 = mSideVec;
        if (_60D != 0) _60D = 0;
    }
}

void Mario::fn_80389D90(f32 (*m)[4], const TVec3f* col2, const TVec3f* col1, const TVec3f* col0) {
    TVec3f a;
    TVec3f c;
    TVec3f b;
    a = *col0;
    c = *col2;
    b = *col1;
    m[0][0] = a.x;
    m[1][0] = a.y;
    m[2][0] = a.z;
    m[0][1] = b.x;
    m[1][1] = b.y;
    m[2][1] = b.z;
    m[0][2] = c.x;
    m[1][2] = c.y;
    m[2][2] = c.z;
}

bool Mario::postureCtrl(MtxPtr) {
    // NONMATCHING: literal-pool placement.
    TVec3f vec1(_1FC);
    TVec3f vec2(mHeadVec);
    f32 blend;
    f32 f1 = mActor->getConst().getTable()->mHeadRotateRatio;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        bool noSlip = !isSlipFloorCode(_960);
        if (noSlip) {
            if (mTargetWalkSpeedIndex < 3) {
                if (calcAngleD(*mGroundPolygon->getNormal(0)) < 45.0f) {
                    MR::isNearZero(mAirGravityVec);
                    vec2 = -mAirGravityVec;
                } else {
                    f32 f2 = calcAngleD(*mGroundPolygon->getNormal(0)) - 45.0f;
                    MR::isNearZero(mAirGravityVec);
                    MR::vecBlendSphere(-mAirGravityVec, vec2, &vec2, f2 / 60.0f);
                }
            } else {
                MR::isNearZero(mAirGravityVec);
                vec2 = -*getGravityVec() - mAirGravityVec;
                MR::normalizeOrZero(&vec2);
            }
        }
    }

    if (isAnimationRun("坂すべり上向きうつぶせ", 2)) {
        vec2 = _368;
    }

    if (isStatusActive(MarioStatus_Wait)) {
        if (isAnimationRun("特殊ウエイト1A")) {
            if (getFrame() > 290.0f) {
                blend = 1.0f;
            } else if (getFrame() > 260.0f) {
                blend = 0.5f + 0.5f * ((getFrame() - 260.0f) / 30.0f);
            } else {
                blend = 0.5f * (getFrame() / 260.0f);
            }

            MR::vecBlendSphere(vec2, -_374, &vec2, blend);
        } else if ((static_cast<u32>(mFlags20.mBits.mBit12)) && (static_cast<u32>(mFlags20.mBits.mBit11))) {
            vec2 = -_374;
        }
    }

    if (!isSwimming() && (isAnimationRun("水泳ジェット") || isAnimationRun("水泳スピン移動") || isAnimationRun("リングダッシュ") ||
                          isAnimationRun("水泳スピンジャンプ"))) {
        MR::vecBlendSphere(mHeadVec, mJumpVec, &vec2, 0.1f);

        if (MR::normalizeOrZero(&vec2)) {
            vec2 = mHeadVec;
        }

        mDrawStates.mBits.mBit14 = 1;
    }

    calcTornadoTilt();

    slopeTiltHead(&vec2);

    isStatusActive(MarioStatus_Climb);

    if (MR::diffAngleAbs(vec1, vec2) > mActor->getConst().getTable()->mHeadAngleFixMargin && !MR::vecBlendSphere(vec1, vec2, &vec1, f1)) {
        TMtx34f mtx;
        PSMTXRotAxisRad(mtx, &mSideVec, 0.1f);
        PSMTXMultVecSR(mtx, vec1, &vec1);
    }

    if (isPlayerModeYoshi() && isNonFixHeadVec()) {
        vec1 = mHeadVec;
    }

    if (!MR::normalizeOrZero(&vec1)) {
        _1FC = vec1;
    }

    return false;
}

bool Mario::isNonFixHeadVec() const {
    if (isStatusActive(MarioStatus_Swim)) {
        return true;
    }

    if (isAnimationRun("水泳ジェット")) {
        return true;
    }

    if (isStatusActive(MarioStatus_Wall)) {
        return true;
    }

    if (isStatusActive(MarioStatus_SideStep)) {
        return true;
    }

    if (isStatusActive(MarioStatus_Foo)) {
        return true;
    }

    if (isStatusActive(MarioStatus_Stick)) {
        return true;
    }

    if (isStatusActive(MarioStatus_YoshiBalloon)) {
        return true;
    }

    if (((mMovementStates.mHighWord >> 0) & 3U)) {
        return true;
    }

    if (mActor->_EA5) {
        return true;
    }

    if (isPlayerModeYoshi() && mActor->fn_803B9810()->fn_8041F8B0()) {
        return true;
    }

    if (isStatusActive(MarioStatus_Talk)) {
        return false;
    }

    return mActor->_EA4;
}

void Mario::createDirectionMtx(MtxPtr pMtx) {
    TVec3f side;
    TVec3f up;
    TVec3f front;
    up = _1FC;
    side = mSideVec;
    front = mFrontVec;

    if ((static_cast<u32>(mDrawStates.mBits.mBit14))) {
        MR::normalize(&up);
        front.cross(side, up);
        MR::normalizeOrZero(&front);

        if (MR::isNearZero(front)) {
            front = mFrontVec;
        }

        side.cross(up, front);
        MR::normalizeOrZero(&side);

        if (MR::isNearZero(side)) {
            side = mSideVec;
        }
    } else {
        MR::normalizeOrZero(&up);
        side.cross(up, mFrontVec);
        MR::normalizeOrZero(&side);
        if (MR::isNearZero(side)) {
            side = mSideVec;
        }

        front.cross(side, up);
        MR::normalizeOrZero(&front);
        if (MR::isNearZero(front)) {
            front = mFrontVec;
        }
    }

#ifdef GALAXY_NATIVE
    createMtxDir(pMtx, front, up, side);
#else
    fn_80389D90(pMtx, &front, &up, &side);
#endif
    MR::setMtxTrans(pMtx, 0.0f, 0.0f, 0.0f);
}

void Mario::fn_8038A560(f32 (*arg1)[4], s32 arg2) {
    if (!this->isNonFixHeadVec() && arg2 == 0 && !((this->mMovementStates.mLowWord >> 9) & 1)) {
        this->fixHeadFrontVecByGravity();
    }
    if (!this->updatePosture(arg1)) {
        this->createDirectionMtx(arg1);
        PSMTXConcat(_C4, arg1, arg1);
        this->fn_8038A600();
    }
}

// NONMATCHING: temporary/register allocation and discarded comparison bookkeeping.
void Mario::fn_8038A600() {
    HitSensor* sensor = mActor->_470;
    if (isAnimationRun("投げ")) {
        sensor = mActor->_474;
    }
    if (mActor->isPunching()) {
        sensor = mActor->_924;
    }
    if (sensor == nullptr) {
        fn_8038A840(nullptr, 0.8f * this->_58.x, 0.0f, 0.8f * this->_58.z);
        if (this->_12C != 0) {
            this->_12C--;
        }
    } else {
        TVec3f previous(this->mSideVec);
        TVec3f up(this->_1FC);
        TVec3f front(this->mFrontVec);
        const TVec3f& origin = mPosHolder->mPos;
        TVec3f direction(*MR::getSensorPos(sensor));
        direction -= origin;
        MR::normalizeOrZero(&direction);
        if (!MR::isNearZero(direction, 0.001f)) {
            TVec3f projected;
            MR::vecKillElement(direction, up, &projected);
            if (!MR::normalizeOrZero(&projected)) {
                f32 angle = JMAAcosRadian(projected.dot(front));
                TVec3f cross;
                PSVECCrossProduct(&front, &projected, &cross);
                if (cross.dot(up) < 0.0f) {
                    angle = -angle;
                }
                JMAAcosRadian(projected.dot(direction));
                bool positive = direction.dot(up) >= 0.0f;
                bool narrow = MR::abs(angle) < 1.5707964f;
                bool differs = positive != narrow;
                (void)differs;
                f32 zero = 0.0f;
                f32 limit = 0.1f * this->_406;
                if (limit > 1.0471976f) {
                    limit = 1.0471976f;
                }
                f32 clamped = MR::clamp(angle, -limit, limit);
                fn_8038A840(nullptr, clamped, 0.0f, zero);
                this->_12C = 0x10;
            }
        }
    }
}

void Mario::fn_8038A840(void*, f32 x, f32 y, f32 z) {
    this->_58.set(x, y, z);
    MR::orderRotateMtx(0, this->_58, this->_64);
    MR::orderRotateMtx(0, -this->_58, this->_94);
}

s32 Mario::fn_8038A8A0(f32 (*matrix)[4], TVec3f* offset) {
    offset->zero();
    if (this->isNonFixHeadVec()) {
        PSMTXIdentity(matrix);
        return 0;
    }
    if (MR::isNearZero(this->_74C, 0.001f) && this->_754 == 0) {
        PSMTXIdentity(matrix);
        this->_750 = 0;
        this->_74C = 0.0f;
        return 1;
    }
    bool apply = false;
    if (this->_750 == 0 && this->_754 == 0) {
        apply = true;
    }
    if (apply) {
        this->fn_8038A9D0();
    }
    TVec3f up(0.0f, 1.0f, 0.0f);
    if (this->fn_8038AA30()) {
        PSMTXRotAxisRad(matrix, &up, -this->_74C);
    } else {
        PSMTXRotAxisRad(matrix, &up, this->_74C);
    }
    this->fn_8038AB10();
    this->fn_8038AB70();
    return 1;
}

void Mario::fn_8038A9D0() {
    TMtx34f sp18;
    PSMTXRotAxisRad(sp18, &this->mHeadVec, this->_74C);
    TVec3f sp8;
    PSMTXMultVec(sp18, &this->mFrontVec, &sp8);
    this->setFrontVecKeepUp(sp8);
    this->_74C = 0.0f;
}

bool Mario::fn_8038AA30() {
    if (!(static_cast<u32>(this->mMovementStates.mHighBits.mBit8)) || this->isStatusActive(0x22)) {
        return false;
    }
    f32 alignment = this->mFrontVec.dot(this->getCamDirX());
    if (this->_1FC.dot(this->getCamDirY()) < 0.0f) {
        alignment = -alignment;
    }
    return (alignment < 0.0f && this->_74C < 0.0f) || (alignment > 0.0f && this->_74C > 0.0f);
}

void Mario::fn_8038AB10() {
    u32 count = this->_754;
    if (count != 0) {
        f32 phase = this->_74C;
        this->_754 = count - 1;
        this->_750 = 0;
        this->_74C = phase + ((3.1415927f - phase) / (f32)count);
    }
}

void Mario::fn_8038AB70() {
    u32 remaining;
    u32 count = this->_750;
    if (count != 0) {
        remaining = count - 1;
        this->_750 = remaining;
        this->_74C = (this->_74C * (f32)remaining) / (f32)count;
        if (remaining == 0) {
            this->setFrontVecKeepUp(this->mFrontVec);
        }
    }
}

void Mario::slopeTiltHead(TVec3f* pVec) {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        if (_8F0 > 0.0f && !(static_cast<u32>(mMovementStates.mHighBits.mBit28))) {
            f32 blendFactor = _8F0 / 5.0f;
            if (blendFactor > 1.0f) {
                blendFactor = 1.0f;
            }

            MR::vecBlend(_368, *pVec, pVec, blendFactor);
            MR::normalize(pVec);
        } else if ((static_cast<u32>(mDrawStates.mBits.mBit27))) {
            MR::vecBlend(_368, *pVec, pVec, 0.2);
            MR::normalize(pVec);
        }
    }
}

void Mario::fixFrontVecByGravity() {
    if (isNonFixHeadVec()) {
        return;
    }

    if (MR::isNearZero(mAirGravityVec)) {
    }

    TVec3f up = -mAirGravityVec;
    MR::normalize(&up);
    if ((static_cast<u32>(mFlags14.mLowBits.mBit12))) {
        TVec3f side;
        MR::calcPlayerKillDiffVec(&side, mPosHolder->mPos, _6F4, _700);
        if (_344.dot(side) >= 0.0f) {
            _344 = side;
        } else {
            _344 = -side;
        }

        if (mSideVec.dot(_344) >= 0.0f) {
            mSideVec = _344;
            MR::normalize(&mSideVec);
        } else {
            // TVec3f stack_14 = -_344;
            mSideVec = -_344;
            MR::normalize(&mSideVec);
        }
    } else if ((static_cast<u32>(mFlags20.mBits.mBit29))) {
        TVec3f side2 = up.cross(mFrontVec);
        MR::normalizeOrZero(&side2);
        if (MR::isNearZero(side2) == false) {
            _344 = side2;
        }
    }

    TVec3f front = _344.cross(up);
    if (MR::normalizeOrZero(&front) == false) {
        setFrontVec(front);
        _22C = mFrontVec;
        f32 _328mag = _328.length();
        _328 = mFrontVec * _328mag;
    }
}

void Mario::fixFrontVecFromUpSide() {
    TVec3f frontVec = mSideVec.cross(mHeadVec);
    MR::normalizeOrZero(&frontVec);
    if (MR::isNearZero(frontVec) == false) {
        setFrontVec(frontVec);
        _22C = mFrontVec;
        f32 scaleFactor = _328.length();
        _328 = mFrontVec * scaleFactor;
    }
}

void Mario::fixSideVecFromFrontUp() {
    TVec3f side = mHeadVec.cross(mFrontVec);
    MR::normalizeOrZero(&side);
    if (MR::isNearZero(side) == false) {
        mSideVec = side;
    }
}

void Mario::setSideVec(const TVec3f& rSide) {
    mSideVec = rSide;
    MR::normalize(&mSideVec);
}

void Mario::setHeadVec(const TVec3f& rHead) {
    mHeadVec = rHead;
    MR::normalize(&mHeadVec);
}

void Mario::setFrontVec(const TVec3f& rFront) {
    TVec3f plusMinus6A0;
    TVec3f frontWithout6A0;
    if ((static_cast<u32>(mMovementStates.mHighBits.mBit8)) && !isStatusActive(MarioStatus_Talk) && !isSwimming() &&
        !(static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit31))) {
        if (getCamDirZ().dot(mSwimSideVec694) > 0.0f) {
            plusMinus6A0 = mSwimSideVec694;
        } else {
            TVec3f neg6A0 = -mSwimSideVec694;
            plusMinus6A0 = neg6A0;
        }

        if (plusMinus6A0.dot(rFront) > 0.0f) {
            // maybe this reflects rFront around mSwimSideVec694?? (very unsure)
            f32 f1 = MR::vecKillElement(rFront, plusMinus6A0, &frontWithout6A0);
            frontWithout6A0 -= plusMinus6A0 * f1;
            MR::normalizeOrZero(&frontWithout6A0);
            mFrontVec = frontWithout6A0;
        } else {
            mFrontVec = rFront;
        }
    } else {
        mFrontVec = rFront;
    }

    MR::normalize(&mFrontVec);
}

void Mario::setFrontVecKeepUp(const TVec3f& rFront, f32 blendFactor) {
    if (MR::isNearZero(rFront) == false) {
        TVec3f blendedFront(rFront);
        TVec3f normalizedFront(rFront);
        MR::normalize(&normalizedFront);
        s32 blend = 0;
        f32 angleDiff = MR::diffAngleAbs(mFrontVec, normalizedFront);
        if (angleDiff < HALF_PI) {
            blend = MR::vecBlendSphere(mFrontVec, normalizedFront, &blendedFront, blendFactor);
        }

        if (blend == 0) {
            MR::vecRotAxis(mFrontVec, normalizedFront, mHeadVec, &blendedFront, angleDiff * blendFactor);
        }

        setFrontVecKeepUp(blendedFront);
    }
}

void Mario::setFrontVecKeepUpAngle(const TVec3f& rFrontVec, f32 angle) {
    if (MR::isNearZero(rFrontVec) == false) {
        f32 angleDiff = MR::diffAngleAbs(mFrontVec, rFrontVec);
        if (angleDiff != 0.0f) {
            if (angleDiff < angle) {
                setFrontVecKeepUp(rFrontVec);
            } else {
                setFrontVecKeepUp(rFrontVec, angle / angleDiff);
            }
        }
    }
}

void Mario::setFrontVecKeepUp(const TVec3f& rFront, u32 _750_set) {
    Mtx rotMtx;
    TVec3f rotFrontVec;
    TVec3f rotVec2;
    PSMTXRotAxisRad(rotMtx, &mHeadVec, _74C);
    PSMTXMultVec(rotMtx, &mFrontVec, &rotFrontVec);
    f32 angleDiff = MR::diffAngleAbs(rFront, rotFrontVec);
    if (angleDiff > 0.024543693f) {
        PSMTXRotAxisRad(rotMtx, &mHeadVec, PI_180);
        PSMTXMultVec(rotMtx, &rotFrontVec, &rotVec2);
        f32 angleDiff2 = MR::diffAngleAbs(rFront, rotVec2);
        if (angleDiff2 < angleDiff) {
            _74C = -angleDiff;
        } else {
            _74C = angleDiff;
        }
    }

    setFrontVecKeepUp(rFront);
    if (_750_set != 0) {
        _750 = _750_set;
    }
}

void Mario::setFrontVecKeepUp(const TVec3f& rFront) {
    TVec3f front = mFrontVec;
    TVec3f side;
    TVec3f blendedFront;
    if (MR::isNearZero(rFront) == false) {
        setFrontVec(rFront);
    }

    side.cross(mHeadVec, mFrontVec);
    if (MR::isNearZero(side)) {
        MR::vecBlendSphere(front, rFront, &blendedFront, 0.5f);
        side.cross(mHeadVec, blendedFront);
        setFrontVec(blendedFront);
        if (MR::isNearZero(side)) {
            setFrontVecKeepSide(rFront);
            return;
        }
    }

    MR::normalize(&side);
    mSideVec = side;
    MR::normalize(&mSideVec);
    TVec3f stack_14 = mSideVec.cross(mHeadVec);
    setFrontVec(stack_14);
    _22C = mFrontVec;
    f32 _328mag = _328.length();
    _328 = mFrontVec * _328mag;
    _344 = mSideVec;
}

void Mario::setFrontVecKeepSide(const TVec3f& rFront) {
    TVec3f headVec;
    TVec3f front;
    if (MR::isNearZero(rFront) == false) {
        setFrontVec(rFront);
    }

    headVec.cross(mFrontVec, mSideVec);
    if (MR::normalizeOrZero(&headVec) != false) {
        const TVec3f* gravity = getGravityVec();
        TVec3f up = -(*gravity);
        mHeadVec = up;
        MR::normalize(&mHeadVec);
    } else {
        mHeadVec = headVec;
        MR::normalize(&mHeadVec);
    }

    front.cross(mSideVec, mHeadVec);
    setFrontVec(front);
    _22C = mFrontVec;
    f32 _328mag = _328.length();
    _328 = mFrontVec * _328mag;
}

void Mario::setHeadAndFrontVecFromRotate(const TVec3f& rRotate) {
    TVec3f frontVec = TVec3f(0.0f, 0.0f, 1.0f);
    TVec3f headVec = TVec3f(0.0f, 1.0f, 0.0f);
    Mtx mtx;
    MR::makeMtxTR(mtx, 0.0f, 0.0f, 0.0f, rRotate.x, rRotate.y, rRotate.z);
    PSMTXMultVec(mtx, (Vec*)&frontVec, (Vec*)&frontVec);
    PSMTXMultVecSR(mtx, (Vec*)&headVec, (Vec*)&headVec);
    mHeadVec = headVec;
    MR::normalize(&mHeadVec);
    setFrontVecKeepUp(frontVec);
}

void Mario::forceSetHeadVecKeepSide(const TVec3f& rVec) {
    mHeadVec = rVec;
    _1FC = rVec;
    if (MR::isSameDirection(rVec, mSideVec)) {
        fixSideVecFromFrontUp();
    }

    fixFrontVecFromUpSide();
    _43C = mFrontVec;
    _750 = 0;
}

// float regswap + bad stack ordering
void Mario::draw() const {
    if ((static_cast<u32>(mDrawStates.mBits.mBit30))) {
        TDDraw::cameraInit3D();
        TDDraw::setup(0, 1, 0);
        const TVec3f& position = mPosHolder->mPos;
        const TVec3f& actorPosition = mActor->_240;
        TDDraw::drawCylinder(position, mShadowPos - actorPosition, 32.0f, 0xffff0010, 0xff000004, 8);
        TVec3f diff2A0_31C(mActor->_240 - mShadowPos);
        f32 diffMag = diff2A0_31C.length();
        MR::normalize(&diff2A0_31C);
        TVec3f _31CPlusStuff(mShadowPos);
        _31CPlusStuff += diff2A0_31C * diffMag * 0.125f * (mActor->_37C % 0xa) * 0.1f;
        f32 one_eighth = 0.125f;
        for (u32 i = 0; i < 8; i++) {
            TDDraw::drawCircle(_31CPlusStuff, diff2A0_31C, 100.0f, 0xffffff70, 0x10);
            _31CPlusStuff += diff2A0_31C * diffMag * one_eighth;
        }
    }

    if (_97C != nullptr) {
        _97C->draw3D();
    }

    drawTask();
}

void Mario::fn_8038B9F0() {
    TVec3f offset;
    offset.zero();
    if (isStatusActive(MarioStatus_Blown)) {
        return;
    }
    if (mActor->_934) {
        MR::vecBlend(&_13C, _13C, offset, 0.3f);
    }
    MarioState* state = _97C;
    if (state == nullptr) {
        if (isAnimationRun("壁押し")) {
            const TVec3f& position = mPosHolder->mPos;
            TVec3f horizontal;
            MR::vecKillElement(_4E8 - position, mHeadVec, &horizontal);
            f32 length = PSVECMag(&horizontal);
            offset = *mFrontWallTriangle->getNormal(0) * (length - 35.0f);
        }
    } else if (state->mStatusId == MarioStatus_Swim) {
        f32 surfaceOffset = mSwim->mSurfaceOffset;
        offset += -*getGravityVec() * surfaceOffset;
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) && !(static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        f32 verticalSpeed = mVerticalSpeed;
        if (verticalSpeed > 5.0f && verticalSpeed < 65.0f && !isCurrentShadowFloorDangerAction()) {
            TVec3f direction(mGroundPos);
            direction -= mShadowPos;
            MR::normalizeOrZero(&direction);
            if (direction.dot(_368) < 0.1f) {
                f32 speed = mVerticalSpeed;
                offset += getAirGravityVec() * speed;
            } else {
                const TVec3f& position = mPosHolder->mPos;
                TVec3f horizontal;
                f32 distance = MR::vecKillElement(mGroundPos - position, *getGravityVec(), &horizontal);
                offset += *getGravityVec() * distance;
            }
        }
    }
    TVec3f sinkOffset;
    sinkOffset.zero();
    if (mSinkTimer != 0) {
        f32 sinkTimer = static_cast<f32>(mSinkTimer);
        sinkOffset = *getGravityVec() * sinkTimer * 0.00390625f * 120.0f;
        f32 sink = static_cast<f32>(mSinkTimer);
        sinkOffset = -_1FC * sink * 0.00390625f * 120.0f;
    }
    f32 blendRate = 0.2f;
    if (!isUseSimpleGroundCheck()) {
        if (fn_8038CCF0()) {
            blendRate = 0.01f;
        } else if (mTargetWalkSpeedIndex == 0 && mWalkSpeed < 0.2f && isBlendWaitGround()) {
            f32 length = PSVECMag(&offset);
            if (PSVECMag(&_148) > length) {
                blendRate = 0.5f;
            } else {
                blendRate = 0.02f * static_cast<f32>(_3CE);
                if (blendRate >= 0.2f) {
                    blendRate = 0.2f;
                }
            }
        }
    }
    MR::vecBlend(&_148, _148, offset, blendRate);
    MR::vecBlend(&_154, _154, sinkOffset, 0.2f);
    _13C = _148 + _154;
}

bool Mario::isEnableRush() const {
    if (isStatusActive(MarioStatus_FireDance) && isPlayerModeCloud() && !MR::isSpinCloudBlockGroupEmpty()) {
        return true;
    }

    if (isHanging()) {
        return false;
    }

    if (isDamaging()) {
        return false;
    }

    if (isStatusActive(MarioStatus_Flow)) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 20) & 1U) && ((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        return false;
    }

    return true;
}

bool Mario::isInvincible() const {
    if (isPlayerModeInvincible()) {
        return true;
    }

    if (((mMovementStates.mLowWord >> 0) & 1U)) {
        return true;
    } else {
        return mActor->_3D8 != false;
    }
}

void Mario::inputStick() {
    mActor->getStickValue(&mStickPos.x, &mStickPos.y);
    if (static_cast<u32>(mFlags14.mHighBits.mBit23)) {
        mFlags14.mHighBits.mBit23 = 0;
        mStickPos.x = 0.0f;
        mStickPos.y = 0.0f;
    }

    mStickPos.x = 1.5f * mStickPos.x;
    mStickPos.y = 1.5f * mStickPos.y;

    mStickPos.x = MR::clamp(mStickPos.x, -1.0f, 1.0f);
    mStickPos.y = MR::clamp(mStickPos.y, -1.0f, 1.0f);

    _38 = mStickPos.z;

    mStickPos.z = MR::sqrt(mStickPos.x * mStickPos.x + mStickPos.y * mStickPos.y);
    mStickPos.z = MR::clamp(mStickPos.z, 0.0f, 1.0f);

    if (MR::isNearZero(mStickPos.z, 0.01f)) {
        mStickPos.z = 0.0f;
    }

    f32 angle = MR::normalizeAngleAbs(MR::atan2(mStickPos.y, mStickPos.x));
    f32 quarterTurn = HALF_PI;
    f32 stickAngleMargin = mActor->getConst().getTable()->mStickAngleMargin;
    u32 quotient = 0;
    while (angle > quarterTurn) {
        angle -= quarterTurn;
        quotient++;
    }

    f32 adjustedAngle;

    if (angle >= quarterTurn - stickAngleMargin) {
        adjustedAngle = quarterTurn;
    } else if (angle <= stickAngleMargin) {
        adjustedAngle = 0.0f;
    } else {
        adjustedAngle = (quarterTurn / (quarterTurn - 2.0f * stickAngleMargin)) * (angle - stickAngleMargin);
    }

    f32 resultAngle = adjustedAngle + quarterTurn * static_cast< f32 >(quotient);

    if (!(static_cast<u32>(getPlayer()->mFlags14.mLowBits.mBit14))) {
        mStickPos.x = mStickPos.z * MR::cos(resultAngle);
        mStickPos.y = mStickPos.z * MR::sin(resultAngle);
    }

    _524 = _528;
    _528 = resultAngle;

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit8)) && !isSwimming()) {
        stick2DadjustGround(mStickPos.x, mStickPos.y);
    } else if (static_cast<u32>(mFlags14.mLowBits.mBit10)) {
        stick2Dadjust(mStickPos.x, mStickPos.y);
    } else if (static_cast<u32>(mMovementStates.mHighBits.mBit5)) {
        update25Dmode();
        updateAxisFromMode(mAxisMode6A4);
    } else if (static_cast<u32>(mFlags14.mLowBits.mBit12)) {
        fn_803A56C0();
    }
    if (mStickPos.z <= 0.01f) {
        mMovementStates.mLowBits.mBit2 = 0;
    }

    calcWorldPadDir(&mWorldPadDir, mStickPos.x, mStickPos.y, false);
    MR::normalizeOrZero(&mWorldPadDir);
}

bool Mario::isCeiling() const {
    if (_4C8->isValid() == false) {
        return false;
    } else {
        return !MR::isNearZero(_3A4);
    }
}

void Mario::setGravityVec(const TVec3f& rGravity) {
    if (MR::isSameDirection(mAirGravityVec, rGravity)) {
        if (MR::isOppositeDirection(mAirGravityVec, rGravity)) {
            mFlags14.mHighBits.mBit27 = 1;
            mFlags14.mHighBits.mBit26 = 1;
        }

        _1E4.zero();
    } else {
        _1E4.cross(mAirGravityVec, rGravity);
        MR::normalizeOrZero(&_1E4);
        f32 frontDot = MR::abs(mFrontVec.dot(_1E4));
        f32 sideDot = MR::abs(mSideVec.dot(_1E4));
        f32 headDot = MR::abs(mHeadVec.dot(_1E4));
        if (frontDot > sideDot && frontDot > headDot) {
            mFlags14.mLowBits.mBit21 = 0;
            mFlags14.mHighBits.mBit25 = 1;
        }

        mFlags14.mHighBits.mBit27 = 1;
    }

    TVec3f stack_8 = -_368;
    _374 = stack_8;
    mAirGravityVec = rGravity;
}

void Mario::updateSoundCode() {
    s32 soundCode = -1;
    Mario* pPlayer = getPlayer();
    if (((pPlayer->mMovementStates.mLowWord >> 30) & 1U)) {
        if (mGroundPolygon->isValid()) {
            soundCode = MR::getSoundCodeIndex(mGroundPolygon->getAttributes());
        }
    } else if (_45C->isValid()) {
        soundCode = MR::getSoundCodeIndex(_45C->getAttributes());
    }

    if (isStatusActive(MarioStatus_Wall)) {
        if (((mMovementStates.mLowWord >> 23) & 1U)) {
            soundCode = MR::getSoundCodeIndex(mFrontWallTriangle->getAttributes());
        } else if (((mMovementStates.mLowWord >> 6) & 1U)) {
            soundCode = MR::getSoundCodeIndex(mBackWallTriangle->getAttributes());
        } else if (((mMovementStates.mLowWord >> 5) & 1U)) {
            soundCode = MR::getSoundCodeIndex(mSideWallTriangle->getAttributes());
        }
    }

    MR::setMapSondCodeGravity(mActor, soundCode);
}

void Mario::fn_8038C540() {
    if (this->mStateFlags._2 != 0) {
        AudSeLifeEntry* p = this->mLifeAlertCounter;
        if (p == nullptr || p->mIsUsed == 0) {
            this->mLifeAlertCounter = MR::fn_8005BC60(0x258);
        }
        this->mLifeAlertCounter->add(1);
        s32 count = this->mLifeAlertCounter->mFrame;
        s32 v = MR::clamp(0x1A4 - count, 0, 0x1A4);
        if (count % 74 == 5) {
            MR::startActionSound(this->mActor, "LifeAlertTrg", v, -1, -1);
        }
    } else {
        AudSeLifeEntry* p = this->mLifeAlertCounter;
        if (p != nullptr && p->mIsUsed != 0) {
            p->reset();
        }
    }
}

// NONMATCHING: the two-stage neighbourhood test is approximated (returns the matched-triangle flag)

// NONMATCHING: independent loads and register scheduling.
void Mario::fn_8038C620() {
    if (this->mStateFlags._1 != 0) {
        MR::startLevelSoundPlayer("JumpFromYoshi", -1, -1);
    } else if (this->mStateFlags._0 != 0) {
        if (this->isPlayerModeYoshi()) {
            if (this->mActor->getYoshi()->mDashTimer > 0) {
                MR::startActionSound(this->mActor->getYoshi(), "DashJump", -1, -1, -1);
            } else {
                MR::startActionSound(this->mActor->getYoshi(), "JumpS", -1, -1, -1);
            }
        } else {
            this->playSound("声小ジャンプ", -1);
        }
        this->playSound("ジャンプ踏切", -1);
    }
    if (this->isPlayerModeYoshi()) {
        if (MR::isPlayerElementModeYoshiDash() && !MR::fn_804225C0()) {
            s32 temp_r29;
            s32 v28 = MR::clamp((s32)(500.0f * MarioAccess::fn_803B57A0()), 0, 1000);
            s32 v3 = MR::clamp((s32)(400.0f * MarioAccess::fn_803B57A0()) + 200, 400, 1000);
            s32 t5 = this->_A98;
            s32 d = v3 - t5;
            s32 var_r31;
            if ((d < 0 ? -d : d) > 150) {
                var_r31 = MR::clamp(v3, t5 - 66, t5 + 66);
            } else {
                var_r31 = MR::clamp(v3, t5 - 22, t5 + 22);
            }
            temp_r29 = this->mActor->getYoshi()->mDashTimer;
            MR::startActionSoundPlayer("DashLV", v28, var_r31);
            MR::startActionSoundPlayer("DashLVWind", v28, -1);
            MR::startActionSoundPlayer("DashLVFire", temp_r29, -1);
            this->fn_803B2950();
            this->_A98 = var_r31;
        } else {
            this->_A98 = 0x226;
        }
        if (PSVECMag(&this->mVelocity) >= 2.0f && this->isAnimationRun("口に含む") && this->isAnimationRun("基本")) {
            if (this->getAnimator()->fn_803DF0E0() >= 2.5f) {
                MR::startActionSoundPlayer("RunMouthFull", -1, -1);
            }
        }
    }
    this->mStateFlags.fn_8038C890();
}

void MarioFlags3::fn_8038C890() {
    _0 = 0;
    _1 = 0;
    _2 = 0;
}

bool Mario::isForceStopRush() const {
    if (mVerticalSpeed == 0.0f) {
        switch (_960) {
        case 1:
        case 4:
        case 10:
        case 15:
        case 16:
        case 24:
        case 41:
        case 129:
            return true;
        }
    }

    return false;
}

const TVec3f& Mario::getShadowNorm() const {
    return *_45C->getNormal(0);
}

// conditionals won't behave
const TVec3f* Mario::getGravityVec() const {
    if (isStatusActive(MarioStatus_Bump) || isStatusActive(MarioStatus_Climb)) {
        return &_790;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        bool noSlip = !isSlipFloorCode(_960);
        if (noSlip) {
            if (mActor->mBeeWallWalk != 0) {
                return &mAirGravityVec;
            }

            if (isAnimationRun("ハード着地")) {
                return &mAirGravityVec;
            }

            if (_3CE < 0x10 && _3BC > 3 && !(static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
                return &mAirGravityVec;
            }

            if (MR::isNearZero(_374) == false && (static_cast<u32>(mFlags20.mBits.mBit12)) && (static_cast<u32>(mFlags20.mBits.mBit11))) {
                return &_374;
            }
        }
    }

    if (isSwimming()) {
        if (mSwim->mWaterDepth >= -150.0f && (static_cast<u32>(mDrawStates.mBits.mBit16)) && MR::isNearZero(_38C) == false) {
            return &_38C;
        }

        return &mAirGravityVec;
    }

    if (MR::isNearZero(mAirGravityVec)) {
    }

    bool b1 = true;
    if (_430 == 0xc) {
        b1 = false;
    } else if (_430 == 0xd) {
        b1 = false;
    }

    bool noSlip = !isSlipFloorCode(_960);
    if (noSlip && b1) {
        bool noHopper = !isPlayerModeHopper();
        if (noHopper) {
            bool noTeresa = !isPlayerModeTeresa();
            if (noTeresa && !isDamaging() && !isStatusActive(MarioStatus_Swim) && !isStatusActive(MarioStatus_Blown) &&
                !isStatusActive(MarioStatus_13) && _430 != 5 && mActor->_334 == 0 && (static_cast<u32>(mMovementStates.mLowBits.mBit31)) && !(static_cast<u32>(mMovementStates.mHighBits.mBit29)) && _3BC < 8) {
                return &_374;
            }
        }
    }

    return &mAirGravityVec;
}

const TVec3f& Mario::getAirGravityVec() const {
    if (MR::isNearZero(this->mAirGravityVec)) {
    }

    return mAirGravityVec;
}

const TVec3f Mario::getAirFrontVec() const {
    TVec3f stack_8;
    MR::vecKillElement(mFrontVec, mAirGravityVec, &stack_8);
    if (MR::normalizeOrZero(&stack_8)) {
        return mFrontVec;
    } else {
        return stack_8;
    }
}

void Mario::touchWater() {
    if (mActor->mActionTable1080->isActionOk("水解除")) {
        if (isPlayerModeHopper() && mCometHopperArea) {
            return;
        }
        if (mMorphResetTimer == 0) {
            mMorphResetTimer = 10;
        }
    }
}

bool Mario::fn_8038CCF0() const {
    if (this->isPlayerModeTeresa()) {
        return false;
    }
    if (this->isAnimationRun("着地") || this->isAnimationRun("着地B") || this->isAnimationRun("着地C") ||
        this->isAnimationRun("着地ターン") || this->isAnimationRun("着地幅とび")) {
        return true;
    }
    if (this->isAnimationRun("すべり着地")) {
        return true;
    }
    if (this->isAnimationRun(this->_728)) {
        return true;
    }
    if (this->isAnimationRun("ハード着地")) {
        return true;
    }
    if (this->isAnimationRun("逆着地")) {
        return true;
    }
    if (this->isAnimationRun("ショート着地")) {
        return true;
    }
    bool isPullOut = MR::isBckPlaying(this->mActor, "pulloutgraspend");
    return isPullOut;
}

void Mario::fn_8038CE40() {
    if (fn_8038CCF0()) stopAnimation(nullptr, static_cast<const char*>(nullptr));
}

// NONMATCHING: equivalent status boolean materialization and branch lowering.
void Mario::stick2DadjustGround(f32& rX, f32& rY) {
    bool cameraRotate = false;
    if (MR::isEqualStageName("HellProminenceGalaxy") && MR::getCurrentScenarioNo() == 3) {
        cameraRotate = true;
    }
    if (cameraRotate) {
        return;
    }

    if (((mFlags14.mLowWord & Mario::MovementStates::Turning) >> 28) && (static_cast<u32>(mMovementStates.mLowBits.mBit30)) && !(static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        mFlags14.mLowWord &= ~Mario::MovementStates::Turning;
        clear2DStick();
        if (_66C) { _66C = false; }
    }

    mSide2D.set(1.0f, 0.0f, 0.0f);
    mUp2D.set(0.0f, 1.0f, 0.0f);
    mNormal2D.set(0.0f, 0.0f, -1.0f);

    PSMTXMultVecSR(_F4, mSide2D, mSide2D);
    PSMTXMultVecSR(_F4, mNormal2D, mNormal2D);
    PSMTXMultVecSR(_F4, mUp2D, mUp2D);

    mSwimSideVec694 = mNormal2D;

    MR::vecKillElement(_368, mSwimSideVec694, &_660);

    if (MR::normalizeOrZero(&_660)) {
        _660 = _368;
    }

    f32 dot = _660.dot(mUp2D);

    if (!MR::isInRange(dot, -0.707f, 0.707f)) {
        set2Dmode(dot >= 0.0f);
    }

    TVec3f stickDir2D(rX, rY, 0.0f);
    MR::normalizeOrZero(&stickDir2D);

    if (_66C) {
        if (MR::isNearZero(mStickPos.z)) {
            if (_66C) { _66C = false; }
            return;
        }

        if (MR::diffAngleAbs(stickDir2D, _670) > 0.3f) {
            if (_66C) { _66C = false; }
            return;
        }

        f32 angle = MR::diffAngleSignedHorizontal(_67C, _660, mSwimSideVec694);
        TVec3f cameraZ;
        MR::getCameraInvViewMtx().getZDir(cameraZ);
        if (cameraZ.dot(mSwimSideVec694) > 0.0f) {
            angle = -angle;
        }
        PSMTXMultVecSR(MR::tmpMtxRotZRad(-angle), stickDir2D, stickDir2D);
        rX = stickDir2D.x;
        rY = stickDir2D.y;
        return;
    }

    if (MR::isNearZero(mStickPos.z)) {
        return;
    }

    TVec3f vec2;
    vec2.x = _660.dot(getCamDirX());
    vec2.y = _660.dot(getCamDirY());
    vec2.z = 0.0f;

    MR::normalizeOrZero(&vec2);
    f32 diff = MR::diffAngleAbs(stickDir2D, vec2);

    bool hanging = getCurrentStatus() == 39;
    if (!hanging && !isSwimming() && (diff < 0.5235988f || diff > 2.617994f)) {
        rX = 0.0f;
        rY = 0.0f;
        mStickPos.z = 0.0f;
        return;
    }

    _670 = stickDir2D;
    _67C = _660;
    if (_66C != true) { _66C = true; }
}

void Mario::calcDir2D(f32 x, f32 y, TVec3f* pOut) {
    TVec3f xDir;
    TVec3f yDir;
    MR::vecKillElement(getCamDirX(), mSwimSideVec694, &xDir);
    MR::vecKillElement(getCamDirY(), mSwimSideVec694, &yDir);

    MR::normalizeOrZero(&xDir);
    MR::normalizeOrZero(&yDir);

    pOut->set(xDir * x);
    pOut->add(yDir * y);
}

void Mario::beeMarioOnGround() {
    if (getPlayerMode() == PlayerMode_Bee && !(static_cast<u32>(mDrawStates.mBits.mBit19)) && (static_cast<u32>(mMovementStates.mLowBits.mBit30)) && !(static_cast<u32>(mMovementStates.mHighBits.mBit28)) && !isStatusActive(MarioStatus_Slider)) {
        getPlayer()->incAirWalkTimer();
        getPlayer()->incAirWalkTimer();
        getPlayer()->incAirWalkTimer();
        getPlayer()->incAirWalkTimer();
    }
}

bool Mario::beeMarioOnAir() {
    if (_774) {
        _774--;
        if (!_774) {
            mMovementStates.mHighBits.mBit16 = 0;
        }
    }

    if (getPlayerMode() == PlayerMode_Bee) {
        if (_3BC == 1 && _402) {
            _402--;
        }

        if (isAnimationRun("ハチ壁ジャンプ") && isAnimationTerminate(nullptr)) {
            if (checkLvlA()) {
                changeAnimation("ハチ飛行中", "落下");
            } else {
                changeAnimation("ハチ飛行中無入力", "落下");
            }

            changeAnimationInterpoleFrame(30);
        }
    }

    if (getPlayerMode() == PlayerMode_Bee) {
        TVec3f horizontal;
        if (MR::vecKillElement(mJumpVec, getAirGravityVec(), &horizontal) > -5.0f || _76C) {
            if (!_774) {
                f32 verticalSpeed = cutGravityElementFromJumpVec(true);
                f32 speed = mJumpVec.length();
                MR::normalizeOrZero(&mJumpVec);
                if (!MR::vecBlendSphere(mJumpVec, mFrontVec, &mJumpVec, mActor->getConst().getTable()->mBeeSpeedRotateRatio)) {
                    Mtx rotation;
                    PSMTXRotAxisRad(rotation, &mHeadVec, 0.1f);
                    PSMTXMultVecSR(rotation, &mJumpVec, &mJumpVec);
                }

                mJumpVec.setLength(speed);
                mJumpVec += getAirGravityVec() * verticalSpeed;
            }

            if (!(static_cast<u32>(mMovementStates.mLowBits.mBit14)) && _402) {
                if (_76C < 30) {
                    _76C = 30;
                    _770 = 0.0f;
                }

                mMovementStates.mLowBits.mBit13 = 1;
                _4B0 = mPosHolder->mPos;
                mMovementStates.mLowBits.mBit14 = 1;
            }

            s16 inhibitTime = mActor->getConst().getTable()->mBeeAirWalkInhibitTime;
            s16 powerTime = mActor->getConst().getTable()->mBeeGravityPowerTime;
            TVec3f horizontalVelocity;
            MR::vecKillElement(mJumpVec, getAirGravityVec(), &horizontalVelocity);
            if (horizontalVelocity.length() < 5.0f) {
                inhibitTime = mActor->getConst().getTable()->mBeeAirWalkInhibitTimeV;
                powerTime = mActor->getConst().getTable()->mBeeGravityPowerTimeV;
            }

            if (checkLvlA() && _402 && _3BC > inhibitTime) {
                playSound("ハチ飛行中");
                if (!MR::isNearZero(mStickPos.z)) {
                    setFrontVecKeepUp(getWorldPadDir(), mActor->getConst().getTable()->mBeeAirWalkTurnSpd);
                }

                u16 previousTime = _402;
                if (_402) {
                    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit16)) && _402 > mActor->getConst().getTable()->mAirWalkTime) {
                        _402 = mActor->getConst().getTable()->mAirWalkTime;
                    }

                    _402--;
                }

                if (!_402) {
                    if (previousTime) {
                        playSound("ハチ体力切れ");
                    }

                    mMovementStates.mLowBits.mBit14 = 0;
                    stopAnimation("ハチ飛行中");
                } else {
                    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit16))) {
                        if (!isAnimationRun("ハチ壁ジャンプ") && !isAnimationRun("ハチスピン空中")) {
                            changeAnimation("ハチ飛行中", "落下");
                        }

                        cancelSquatMode();
                        playSound("空中ふんばり");
                        if (_402 < mActor->getConst().getTable()->mAirWalkTime / 2) {
                            getAnimator()->setSpeed(1.5f);
                        }

                        if (_430 == 4) {
                            setFrontVecKeepUp(-_220);
                            _430 = 0;
                        }

                        if (_430 == 5) {
                            _430 = 0;
                        }
                    }

                    s16 time = static_cast< s16 >(_408);
                    if (time > powerTime) {
                        time = powerTime;
                    }

                    f32 ratio = static_cast< f32 >(time) / powerTime;
                    ratio = 0.9f * (ratio * ratio);
                    f32 acceleration = 15.0f * (MR::getRandom() - ratio);
                    acceleration =
                        acceleration * mActor->getConst().getTable()->mBeeFlyRandomFactor - mActor->getConst().getTable()->mBeeFlyConstantFactor;
                    f32 verticalSpeed = cutGravityElementFromJumpVec(true);
                    TVec3f velocity(mJumpVec);
                    f32 factor = 1.0f;
                    if (!MR::isNearZero(mStickPos.z)) {
                        velocity.dot(getWorldPadDir());
                    }

                    if (factor < 0.0f) {
                        factor *= mActor->getConst().getTable()->mBeeUpAccelRatio;
                    }

                    _770 += factor * (acceleration * mActor->getConst().getTable()->mBeeAccelRatio);
                    if (_770 > 0.0f) {
                        _770 *= mActor->getConst().getTable()->mBeeUpDownKiller;
                    }

                    if (!_774) {
                        if (!(static_cast<u32>(getPlayer()->mFlags20.mBits.mBit26))) {
                            if (_770 < -mActor->getConst().getTable()->mBeeUpSpeedMax) {
                                _770 = -mActor->getConst().getTable()->mBeeUpSpeedMax;
                            }
                        } else if (_770 < -0.5f) {
                            _770 = -0.5f;
                        }
                    }

                    addVelocity(*getGravityVec(), _770);
                    if (!(static_cast<u32>(getPlayer()->mFlags20.mBits.mBit26)) && verticalSpeed > 0.0f) {
                        verticalSpeed *= mActor->getConst().getTable()->mBeePushRiseGravityEraser;
                    }

                    mJumpVec += getAirGravityVec() * verticalSpeed;
                    _408++;
                    if (_408 > 120) {
                        _408 = 120;
                    }

                    _4B0 = mPosHolder->mPos;
                    return true;
                }
            } else {
                if (!isAnimationRun("ハチジャンプ") && !isAnimationRun("ハチ壁ジャンプ")) {
                    stopAnimation("ハチ飛行中");
                    if (!isAnimationRun(nullptr) || isAnimationTerminate(nullptr)) {
                        changeAnimation("ハチ飛行中無入力");
                    }
                }

                if (_408) {
                    _408--;
                }

                if (_770 < mActor->getConst().getTable()->mBeeFreeDropMaxSpd) {
                    _770 += mActor->getConst().getTable()->mBeeFreeDropAcc;
                }

                f32 factor = 1.0f;
                addVelocity(*getGravityVec(), _770 * factor);
                if (!MR::isNearZero(mStickPos.z)) {
                    setFrontVecKeepUp(getWorldPadDir(), mActor->getConst().getTable()->mBeeAirWalkTurnSpd);
                }
            }
        }
    }

    return false;
}

bool Mario::blown(const TVec3f& rVec) {
    if (getCurrentStatus() == MarioStatus_Blown) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_Damage) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_13) {
        return false;
    }

    stopWalk();

    mBlown->vecKillActor240(rVec);

    mMovementStates.mHighBits.mBit19 = 1;
    mMovementStates.mLowBits.mBit21 = 0;
    _430 = 3;

    return true;
}

void Mario::checkBump() {
    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit31)) && !(static_cast<u32>(mMovementStates.mLowBits.mBit21)) && (static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        if (!isStatusActive(MarioStatus_Hang) && !isStatusActive(MarioStatus_Wall) && !isStatusActive(MarioStatus_Swim) &&
            !isStatusActive(MarioStatus_Climb)) {
            if (!isDamaging() && getPlayerMode() != 5 && !isSwimming()) {
                if (isStickOn()) {
                    if (!_750 && ((static_cast<u32>(mMovementStates.mHighBits.mBit28)) == 0)) {
                        if (_4DC->isValid()) {
                            const char* floor = MR::getFloorCodeString(_4DC);

                            if (!isSlipPolygon(_4DC) || !strcmp(floor, "NoSlip") || !strcmp(floor, "PressAndNoSlip") ||
                                !strcmp(floor, "SnowAndNoSlip") || !strcmp(floor, "Sand")) {
                                startBump(_518);
                            }
                        }
                    }
                }
            }
        }
    }
}

void Mario::startBump(const TVec3f& rVec) {
    if ((getTrans() - rVec).length() >= 100.0f) {
        return;
    }

    TVec3f v21;
    const TVec3f* grav = &getAirGravityVec();
    const TVec3f* norm = _4DC->getNormal(0);
    MR::vecKillElement(*norm, *grav, &v21);

    if (!MR::normalizeOrZero(&v21)) {
        const TVec3f& opposite = -v21;
        if (MR::diffAngleAbsHorizontal(mFrontVec, opposite, getAirGravityVec()) > 0.52359879f) {
            TVec3f v20;
            if (MR::vecKillElement(getTrans() - rVec, getAirGravityVec(), &v20) > 0.0f) {
                return;
            }
        }
    }

    if (getCurrentStatus() == 30) {
        closeStatus(mBump);
    }

    setTrans(rVec);
    changeStatus(mBump);
    mVelocity.zero();
    _3D0 = 0;
    mMovementStates.mLowBits.mBit15 = 0;
    _790 = -(*_4DC->getNormal(0));
    TVec3f v19;
    MR::vecKillElement(-(*_4DC->getNormal(0)), mSideVec, &v19);

    if (!MR::normalizeOrZero(&v19)) {
        _790 = v19;
    }
}

void Mario::connectToClimb() {
    _790 = -getShadowNorm();
    changeStatus(mClimb);
}

bool Mario::isIgnoreTriangle(const Triangle* pTriangle) {
    return MR::isNearZero(pTriangle->getNormal(0)->dot(*getGravityVec()));
}

void Mario::checkBaseTransPoint() {
    s32 inside = fn_803B2AF0(mPosHolder->mPos - *getGravityVec() * 30.0f, nullptr);
    const TVec3f& position=mPosHolder->mPos;
    u32 count;
    if(fn_803B2950()) count=Collision::checkStrikeBallToDashYoshiStepAndMap(position,1.0f,nullptr,nullptr);
    else count=Collision::checkStrikeBallToMap(position,1.0f,nullptr,nullptr);
    for(u32 i=0;i<count;i++) fn_8038E1A0(fn_803B2AA0(i),inside);
}

// NONMATCHING: the wall-contact push-out is approximated
void Mario::fn_8038E1A0(const HitInfo* hit, s32 inside) {
    const Triangle* tri = &hit->mParentTriangle;
    if (MR::isThroughPolygon(tri) || this->isThroughWall(tri)) {
        return;
    }
    if (hit->isCollisionAtEdge() || hit->isCollisionAtCorner()) {
        this->mDrawStates.mBits.mBit22 = 1;
        return;
    }
    const TVec3f& normal = *tri->getNormal(0);
    f32 alignment = normal.dot(this->_368);
    if (!MR::isNearZero(alignment, 0.3f) || inside != 0) {
        TVec3f movement(this->_16C);
        MR::normalizeOrZero(&movement);
        TVec3f inward = -normal;
        if (!(movement.dot(inward) < 0.707f)) {
            f32 approach = this->_16C.dot(normal);
            if (!(MR::abs(approach) < hit->mDistance) && !MR::isNearZero(hit->mDistance, 1.01f)) {
                const TVec3f& position = this->mPosHolder->mPos;
                TVec3f offset(hit->mHitPos);
                offset -= position;
                if (!(offset.dot(this->mHeadVec) < 0.0f)) {
                    f32 pushDistance = hit->mDistance - 1.0f;
                    TVec3f push(normal);
                    push *= pushDistance;
                    this->addTrans(push);
                    u32 flags = this->mMovementStates.mLowWord;
                    if (!((flags >> 30) & 1) && (flags >> 31) != 0 && !this->isRising() && normal.dot(*this->getGravityVec()) < -0.99f) {
                        this->addVelocity(this->mFrontVec, 5.0f);
                    }
                }
            }
        }
    }
}

void Mario::checkHeadPoint() {
    if (getPlayerMode() == 4 && isStatusActive(0x16)) return;
    TVec3f center;
    f32 radius;
    fn_803B3EE0(&center, &radius);
    MarioStrikeEntry entries[32];
    TVec3f push(0.0f);
    u32 count = fn_803B4300(&push, entries, 32, center, radius);
    if (isSwimming()) {
        for (u32 i = 0; i < count; i++) {
            mSwim->hitHead(&entries[i].mTriangle, entries[i].mHitPos);
            TVec3f delta(entries[i].mHitPos);
            delta.sub(center);
            f32 distance = PSVECMag(&delta);
            addVelocity(*entries[i].mTriangle.getNormal(0), radius - distance);
        }
    }
    if (isPlayerModeBee()) {
        for (u32 i = 0; i < count; i++) {
            const char* code = MR::getWallCodeString(&entries[i].mTriangle);
            if (code && strcmp(code, "Fur") == 0) mDrawStates.mBits.mBit10 = 1;
        }
    }
    if (isStatusActive(2)) {
        for (u32 i = 0; i < count; i++) mDamage->stopHead(*entries[i].mTriangle.getNormal(0));
    }
    if (((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && isRising()) || (isSwimming() && mSwim->fn_803FE9F0()) || isStatusActive(0x27)) {
        for (u32 i = 0; i < count; i++) {
            if (entries[i].mTriangle.getNormal(0)->dot(*getGravityVec()) > 0.707f)
                mActor->sendMsgUpperPunch(entries[i].mTriangle.mSensor);
        }
    }
    addVelocity(push);
    if (!MR::isNearZero(push, 0.001f) && (mMovementStates.mHighWord & 3)) fn_803B44F0(&push);
}

void Mario::checkBaseTransBall() {
    TVec3f center = mPosHolder->mPos + mVelocity + mHeadVec * 50.0f;
    u32 count;
    if (fn_803B2950()) {
        count = Collision::checkStrikeBallToDashYoshiStepAndMap(center, 50.0f, nullptr, nullptr);
    } else {
        count = Collision::checkStrikeBallToMap(center, 50.0f, nullptr, nullptr);
    }
    for (u32 i = 0; i < count; i++) {
        doSwimmingHitCheck(fn_803B2AA0(i), 0);
    }

    center = mPosHolder->mPos + mVelocity - mHeadVec * 40.0f;
    if (fn_803B2950()) {
        count = Collision::checkStrikeBallToDashYoshiStepAndMap(center, 50.0f, nullptr, nullptr);
    } else {
        count = Collision::checkStrikeBallToMap(center, 50.0f, nullptr, nullptr);
    }
    for (u32 i = 0; i < count; i++) {
        doSwimmingHitCheck(fn_803B2AA0(i), 1);
    }

    center = mPosHolder->mPos + mVelocity;
    if (fn_803B2950()) {
        count = Collision::checkStrikeBallToDashYoshiStepAndMap(center, 40.0f, nullptr, nullptr);
    } else {
        count = Collision::checkStrikeBallToMap(center, 40.0f, nullptr, nullptr);
    }
    for (u32 i = 0; i < count; i++) {
        doSwimmingHitCheck(fn_803B2AA0(i), 2);
    }
}

void Mario::createAtField(bool force, f32 radius) {
    u32 hits;
    _578 = 0;
    f32 checkRadius = 50.0f;
    f32 offset = 50.0f;
    u32 count = 2;

    if (isSwimming()) {
        checkRadius = 40.0f;
        offset = 0.0f;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit21))) {
        checkRadius = 40.0f;
        count = 1;
        offset = 40.0f;
    }

    if (force) {
        checkRadius = radius;

        if (radius < 40.0f) {
            checkRadius = 40.0f;
        }

        offset = 0.0f;
    }

    f32 minAngle = 60.0f;
    f32 maxAngle = 120.0f;

    for (u32 i = 0; i < count; i++) {
        TVec3f center = mPosHolder->mPos - getAirGravityVec() * offset * (i + 1);

        if (force && (static_cast<u32>(mMovementStates.mLowBits.mBit16)) && _544 > 1) {
            hits = fn_803B29B0(center, checkRadius);
        } else {
            hits = fn_803B2A20(center, checkRadius, checkRadius);
        }

        for (u32 j = 0; j < hits; j++) {
            const HitInfo* hit = fn_803B2AA0(j);
            const Triangle* triangle = &hit->mParentTriangle;

            if (force && (static_cast<u32>(mMovementStates.mLowBits.mBit16)) && _544 > 1) {
                if (calcAngleD(*triangle->getNormal(0)) < minAngle) {
                    continue;
                }

                if (calcAngleD(*triangle->getNormal(0)) > maxAngle) {
                    continue;
                }

                TVec3f horizontal;

                if (MR::abs(MR::vecKillElement(hit->mHitPos - center, getAirGravityVec(), &horizontal)) > 80.0f) {
                    continue;
                }

                if (hit->isCollisionAtEdge()) {
                    const TVec3f& opposite = -*triangle->getNormal(0);
                    if (MR::diffAngleAbsHorizontal(mJumpVec, opposite, getAirGravityVec()) < 0.7853982f) {
                        continue;
                    }
                }
            }

            if (!force && !isSwimming()) {
                if (triangle->mSensor != nullptr && fn_803EFA30(triangle->mSensor, "食い込み除外")) {
                    continue;
                }

                if (isThroughWall(triangle)) {
                    continue;
                }

                const TVec3f* normal = triangle->getNormal(0);
                s32 code = _95C->getCode(triangle);
                // 0x27 "PressAndNoSlip", 0x2B "SnowAndNoSlip" (SMG2 floor codes, CollisionCode.cpp table; no enumerators yet)
                f32 angle;
                if (code == CollisionFloorCode_NoSlip || code == 0x27 || code == 0x2B) {
                    angle = 60.0f;
                } else {
                    angle = 45.0f;
                }

                if ((static_cast<u32>(mMovementStates.mHighBits.mBit8))) {
                    if (!MR::isSameMtx(*triangle->getBaseMtx(), *triangle->getPrevBaseMtx()) && mGroundPolygon->mSensor != triangle->mSensor) {
                        angle = -1.0f;
                        push(*normal * hit->mDistance);
                    }
                }

                if (calcAngleD(*normal) < angle) {
                    continue;
                }
            }

            *_57C[_578] = *triangle;
            _578++;

            if (_578 == 32) {
                break;
            }
        }

        if (_578 == 32) {
            break;
        }
    }
}

void Mario::doSwimmingHitCheck(const HitInfo* pHit, u32 type) {
    const Triangle* triangle = &pHit->mParentTriangle;

    if (MR::isThroughPolygon(triangle)) {
        return;
    }

    if (isThroughWall(triangle)) {
        return;
    }

    if (damagePolygonCheck(triangle)) {
        return;
    }

    TVec3f velocity(mVelocity);
    const TVec3f* normal = triangle->getNormal(0);

    if (velocity.dot(-*normal) < 0.0f) {
        return;
    }

    mSwim->addVelocity(*normal, pHit->mDistance);

    if (type == 0) {
        _97C->hitWall(*normal, triangle->mSensor);
    } else {
        _97C->hitPoly(*normal, triangle->mSensor);
    }
}

void Mario::doSpinPunchAroundPolygons() {
    if (!mActor->isPunching()) {
        return;
    }

    if (!mActor->isInPunchTimerRange()) {
        return;
    }

    TVec3f center = mPosHolder->mPos + mVelocity + mHeadVec * 80.0f;
    Triangle triangles[256];
    u32 count = MR::createAreaPolygonList(triangles, 256, center + mFrontVec * 120.0f + mSideVec * 120.0f + mHeadVec * 120.0f,
                                          center - mFrontVec * 120.0f - mSideVec * 120.0f - mHeadVec * 120.0f);

    for (u32 i = 0; i < count; i++) {
        sendPunch(triangles[i].mSensor, &triangles[i], true);
    }
}

void Mario::calcShadowPos() {
    f32 offset = 100.0f;
    f32 alignment = _1FC.dot(-*getGravityVec());
    if (alignment < 0.0f) {
        alignment = 0.0f;
    }
    offset *= alignment;
    TVec3f start = mPosHolder->mPos - *getGravityVec() * offset;

    if ((getCurrentStatus() == MarioStatus_Hang && mHang->_12 < 2) || mActor->_EA4) {
        mActor->getRealPos("Spine1", &start);
    }

    f32 previousDistance = (mShadowPos - mPosHolder->mPos).length();
    TVec3f point(start);
    TVec3f direction(*getGravityVec());
    direction.scale(400.0f);
    u32 count = 6;

    if (isStatusActive(MarioStatus_Recovery)) {
        count = 30;
    }

    for (u32 i = 0; i < count; i++) {
        { const u32 flagValue = static_cast<u32>(fn_803B27F0(&mShadowPos, _45C, point, direction, nullptr, _458)); mMovementStates.mLowBits.mBit29 = flagValue; }

        if ((static_cast<u32>(mMovementStates.mLowBits.mBit29))) {
            break;
        }

        point += direction;
    }

    f32 distance = 10.0f + (mShadowPos - mPosHolder->mPos).length();
    bool changed = false;

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit29)) && MR::abs(previousDistance - distance) > 100.0f) {
        changed = true;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit29)) || changed) {
        direction.setLength(100.0f + (50.0f + previousDistance));
        point = start + (mSideVec + mFrontVec) * 0.1f;
        { const u32 flagValue = static_cast<u32>(fn_803B27F0(&mShadowPos, _45C, point, direction, nullptr, _458)); mMovementStates.mLowBits.mBit29 = flagValue; }

        if (changed) {
            mMovementStates.mLowBits.mBit29 = 1;
        }
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit29))) {
        mShadowPos = mPosHolder->mPos + *getGravityVec() * 2500.0f;
    }
}

// NONMATCHING: register allocation and masked flag insertion lowering.
void Mario::checkMap() {
    calcShadowPos();

    if (isStatusActive(MarioStatus_Swim)) {
        mGroundPos = mShadowPos;
    }

    TVec3f gravity(*getGravityVec());

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) && isSlipPolygon(mGroundPolygon)) {
        gravity = _374;
    }

    if (isUseSimpleGroundCheck()) {
        if (!(static_cast<u32>(mMovementStates.mLowBits.mBit11))) {
            mVerticalSpeed = (mGroundPos - mPosHolder->mPos).dot(gravity);
        } else {
            mVerticalSpeed = (mShadowPos - mPosHolder->mPos).dot(gravity);
        }

    } else {
        mVerticalSpeed = (mShadowPos - mPosHolder->mPos).dot(gravity);
    }

    if (mVerticalSpeed < 0.0f) {
        mVerticalSpeed = 0.0f;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit28)) && calcAngleD(_368) >= 30.0f) {
        TVec3f start = mPosHolder->mPos + _368 * 100.0f;
        { const u32 flagValue = static_cast<u32>(fn_803B2770(&_498, _460, start, -_904 * 150.0f)); mMovementStates.mHighBits.mBit27 = flagValue; }
    } else {
        mMovementStates.mHighBits.mBit27 = 0;
    }
}

// NONMATCHING: cached actor-vector pointers.
bool Mario::updateBinderInfo() {
    s32 count;
    bool firstCeiling = true;
    bool firstGround = true;
    _3A4.zero();
    _4C8->mIdx = -1;
    Binder* binder = mActor->mBinder;

    if (binder == nullptr) {
        return false;
    }

    count = binder->mPlaneNum;

    if (!count) {
        return false;
    }

    for (u32 i = 0; i < count; i++) {
        const HitInfo* hit = binder->getPlane(i);
        const Triangle* triangle = &hit->mParentTriangle;

        if (MR::isThroughPolygon(triangle) || isThroughWall(triangle)) {
            continue;
        }

        TVec3f normal(*triangle->getNormal(0));
        f32 dot = normal.dot(*getGravityVec());
        f32 ceilingThreshold = 0.1f;

        if (_430 == 10) {
            ceilingThreshold = 0.707f;
        }

        if (getPlayerMode() == PlayerMode_Teresa) {
            mTeresa->fn_80405B10(*triangle->getNormal(0));
        }

        if (dot < _3C) {
            mDrawStates.mBits.mBit25 = 1;

            if (firstGround && (static_cast<u32>(mMovementStates.mLowBits.mBit31)) && mVerticalSpeed > 30.0f) {
                bool oblique = false;
                TVec3f fromHit = mActor->_240 - hit->mHitPos;

                if (MR::diffAngleAbs(fromHit, *triangle->getNormal(0)) > 0.10471976f) {
                    oblique = true;
                }

                if (hit->isCollisionAtCorner() || hit->isCollisionAtEdge() || oblique) {
                    TVec3f pushDirection;

                    if (hit->isCollisionAtEdge()) {
                        pushDirection = *triangle->getNormal(hit->mCollisionType - 1);
                    } else if (!hit->isCollisionAtCorner()) {
                        pushDirection = mPosHolder->mPos - hit->mHitPos;
                    } else {
                        pushDirection = mPosHolder->mPos - hit->mHitPos;
                    }

                    TVec3f direction(fromHit);
                    MR::normalizeOrZero(&direction);
                    f32 facing = direction.dot(*triangle->getNormal(0));
                    f32 scale = facing;

                    if (facing >= 0.707f) {
                        if (hit->isCollisionAtEdge()) {
                            MR::vecKillElement(mVelocity, *triangle->getNormal(0), &mVelocity);
                            scale = 1.0f;
                        }

                    } else if (facing <= -0.707f) {
                        scale = 0.0f;
                    } else {
                        scale = (0.707f - MR::abs(facing)) / 0.707f;
                    }

                    MR::vecKillElement(pushDirection, getAirGravityVec(), &pushDirection);

                    if (MR::normalizeOrZero(&pushDirection)) {
                        MR::vecKillElement(fromHit, getAirGravityVec(), &pushDirection);
                        MR::normalizeOrZero(&pushDirection);
                    }

                    if ((static_cast<u32>(mMovementStates.mLowBits.mBit11))) {
                        if (MR::diffAngleAbsHorizontal(mActor->mBinderDirection5DC, pushDirection, getAirGravityVec()) >= 2.3561945f) {
                            if (_45C->isValid()) {
                                if (triangle->getNormal(0)->dot(*_45C->getNormal(0)) <= 0.17f) {
                                    f32 component = MR::vecKillElement(mJumpVec, *triangle->getNormal(0), &mJumpVec);
                                    mJumpVec += *triangle->getNormal(0) * component * 0.5f;
                                    mJumpVec += *_45C->getNormal(0);
                                } else {
                                    if ((static_cast<u32>(mMovementStates.mLowBits.mBit5)) && (MR::diffAngleAbs(pushDirection, getAirGravityVec()) >= 1.4959966f ||
                                                                MR::diffAngleAbsHorizontal(mJumpVec, pushDirection, getAirGravityVec()) < 0.0f)) {
                                        pushDirection = -pushDirection;
                                    }

                                    mMovementStates.mLowBits.mBit11 = 0;
                                }

                            } else {
                                if ((static_cast<u32>(mMovementStates.mLowBits.mBit5)) && (MR::diffAngleAbs(pushDirection, getAirGravityVec()) >= 1.4959966f ||
                                                            MR::diffAngleAbsHorizontal(mJumpVec, pushDirection, getAirGravityVec()) < 0.0f)) {
                                    pushDirection = -pushDirection;
                                }

                                mMovementStates.mLowBits.mBit11 = 0;
                            }
                        }

                        if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && !isRising()) {
                            TVec3f horizontal;
                            if (MR::isNearZero(MR::vecKillElement(mActor->mBinderMotion5D0, getAirGravityVec(), &horizontal)) &&
                                ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mLowBits.mBit6)) || (static_cast<u32>(mMovementStates.mLowBits.mBit5)))) {
                                cutGravityElementFromJumpVec(true);
                                mJumpVec += -getAirGravityVec() * 5.0f + getWallNorm() * 2.0f;

                                if (!isCeiling()) {
                                    addTrans(getAirGravityVec() * -30.0f);
                                }

                                pushDirection.zero();
                            }
                        }
                    }

                    push(pushDirection * hit->mDistance * scale);
                    firstGround = false;

                    mFlags20.mBits.mBit18 = 1;
                }
            }

            if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && (static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
                TVec3f lastMove;
                mActor->getLastMove(&lastMove);

                if (MR::isNearZero(lastMove.dot(*getGravityVec()))) {
                    mActor->sendMsgToSensor(triangle->mSensor, 0xAC);

                    if (!mActor->sendMsgToSensor(triangle->mSensor, 3)) {
                        TVec3f direction = mPosHolder->mPos - hit->mHitPos;
                        TVec3f originalDirection(direction);
                        TVec3f velocity(_1A8);
                        MR::normalizeOrZero(&velocity);
                        f32 blend = 1.0f;

                        if (!MR::isNearZero(velocity)) {
                            blend = velocity.dot(*triangle->getNormal(0));
                        }

                        blend = MR::clamp(blend, 0.0f, 1.0f);

                        MR::vecBlendSphere(_1A8, direction, &direction, blend);
                        MR::vecKillElement(direction, *getGravityVec(), &direction);
                        MR::normalizeOrZero(&direction);

                        if (MR::isNearZero(direction)) {
                            direction = mFrontVec;
                        }

                        f32 distance = hit->mDistance;

                        if (distance == 0.0f) {
                            distance = 1.0f;
                        }

                        push(direction * distance);
                        firstGround = false;
                    }
                }
            }

        } else if (!(dot < ceilingThreshold)) {
            TVec3f horizontal;

            if (firstCeiling && (MR::vecKillElement(mJumpVec, normal, &horizontal) < 0.0f || (static_cast<u32>(mPrevDrawStates.mBits.mBit1))) && calcAngleD(normal) > 100.0f) {
                if (strcmp(triangle->mSensor->mHost->mName, "マンホールのふた（クッパ船）")) {
                    mJumpVec = horizontal;
                }

                startPadVib(0UL);
                firstCeiling = false;
                *_4C8 = *triangle;
            }

            _3A4 += normal;
        }
    }

    MR::normalizeOrZero(&_3A4);
    return true;
}

f32 Mario::calcDistToCeil(bool saveSensor) {
    f32 maxDistance = 200.0f;
    f32 offset = 30.0f;
    if (isPlayerModeYoshi()) {
        getCurrentStatus();
    }
    if (saveSensor) {
        _730 = nullptr;
    }
    if (isStatusActive(MarioStatus_Hang)) {
        offset = 0.0f;
    }
    MR::isNearZero(mAirGravityVec, 0.001f);
    TVec3f start = mPosHolder->mPos + mAirGravityVec * offset;
    MR::isNearZero(mAirGravityVec, 0.001f);
    TVec3f ray = -mAirGravityVec * (maxDistance + offset);
    Triangle triangle;
    TVec3f hitPos;
    bool hit;
    if (fn_803B2950()) {
        hit = MR::getFirstPolyOnLineToDashYoshiStepAndMap(&hitPos, &triangle, start, ray);
    } else {
        hit = MR::getFirstPolyOnLineToMap(&hitPos, &triangle, start, ray);
    }
    if (!hit) {
        return maxDistance;
    }
    if (isThroughWall(&triangle)) {
        return maxDistance;
    }
    if (fn_80390210(&triangle)) {
        if ((hitPos - mPosHolder->mPos).dot(*getGravityVec()) >= 0.0f) {
            return maxDistance;
        }
    }
    _3B0 = *triangle.getNormal(0);
    if (saveSensor) {
        _730 = triangle.mSensor;
    }
    if (!_4C8->isValid()) {
        *_4C8 = triangle;
    }
    return (hitPos - mPosHolder->mPos).length();
}

bool Mario::fn_80390210(const Triangle* triangle) {
    bool result = false;
    if (MR::isSensorPressObj(triangle->mSensor) || this->_95C->getCode(triangle) == 0x1D || this->_95C->getCode(triangle) == 0x27
        || this->fn_803902B0() || this->_960 == 0x1D || this->_960 == 0x27) {
        result = true;
    }
    return result;
}

bool Mario::fn_803902B0() const {
    const HitSensor* sensor = this->mGroundPolygon->mSensor;
    return sensor != nullptr && MR::isSensorPressObj(sensor);
}

// NONMATCHING: independent loads and temporary/register allocation.
f32 Mario::calcDistToCeilOnPress() {
    if (!_480->isValid() || !_484->isValid()) return 0.0f;
    TVec3f projected;
    if (_480->getNormal(0)->dot(mAirGravityVec) < 0.0f) {
        const TVec3f& position = mPosHolder->mPos;
        TVec3f delta(*_480->calcAndGetPos(0));
        delta -= position;
        f32 distance = MR::vecKillElement(delta, *_480->calcAndGetNormal(0), &projected);
        TVec3f displacement(*_480->getNormal(0));
        displacement *= distance;
        mPosHolder->fn_80406010(displacement);
    } else {
        const TVec3f& position = mPosHolder->mPos;
        TVec3f delta(*_484->calcAndGetPos(0));
        delta -= position;
        f32 distance = MR::vecKillElement(delta, *_484->calcAndGetNormal(0), &projected);
        TVec3f displacement(*_484->getNormal(0));
        displacement *= distance;
        mPosHolder->fn_80406010(displacement);
    }
    MR::setPosition(mActor, mPosHolder->mPos);
    const TVec3f* firstPosition = _480->calcAndGetPos(0);
    TVec3f first(mPosHolder->mPos);
    first -= *firstPosition;
    f32 firstDistance = MR::vecKillElement(first, *_480->calcAndGetNormal(0), &projected);
    const TVec3f* secondPosition = _484->calcAndGetPos(0);
    TVec3f second(mPosHolder->mPos);
    second -= *secondPosition;
    f32 secondDistance = MR::vecKillElement(second, *_484->calcAndGetNormal(0), &projected);
    if (firstDistance > secondDistance) return secondDistance;
    return firstDistance;
}

f32 Mario::calcDistToCeilHead() {
    Triangle triangle;
    TVec3f hitPos;
    TVec3f start = mActor->mCapPosition - mFrontVec * 40.0f;

    if (fn_803B2770(&hitPos, &triangle, start, -getAirGravityVec() * 80.0f)) {
        *fn_803EFA20() = triangle;
        return (hitPos - mPosHolder->mPos).length();
    }

    return 80.0f;
}

void Mario::fixTransBetweenWall(const TVec3f& rFirst, const TVec3f& rSecond) {
    TVec3f center = (rFirst + rSecond) * 0.5f;
    MR::vecKillElement(center - mPosHolder->mPos, *getGravityVec(), &center);
    setTrans(mPosHolder->mPos + center);
}

// NONMATCHING: stack-slot allocation.
f32 Mario::calcDistWidth() {
    Triangle triangle;
    TVec3f hitPos;

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        if (fn_803B2770(&hitPos, &triangle, _4E8, *mFrontWallTriangle->getNormal(0) * 100.0f)) {
            fixTransBetweenWall(hitPos, _4E8);
            return (hitPos - _4E8).length();
        }
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        if (fn_803B2770(&hitPos, &triangle, _4F4, *mBackWallTriangle->getNormal(0) * 100.0f)) {
            fixTransBetweenWall(hitPos, _4F4);
            return (hitPos - _4F4).length();
        }
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit5))) {
        if (fn_803B2770(&hitPos, &triangle, _500, *mSideWallTriangle->getNormal(0) * 100.0f)) {
            fixTransBetweenWall(hitPos, _500);
            return (hitPos - _500).length();
        }
    }

    TVec3f center = mPosHolder->mPos + mHeadVec * 80.0f;
    TVec3f firstPos;
    TVec3f secondPos;
    u32 count = fn_803B29B0(center, 30.0f);

    if (count < 2) {
        return 100.0f;
    }

    f32 width = 100.0f;

    for (u32 i = 0; i < count; i++) {
        const HitInfo* first = fn_803B2AA0(i);
        TVec3f firstNormal(*first->mParentTriangle.getNormal(0));

        for (u32 j = i + 1; j < count; j++) {
            const HitInfo* second = fn_803B2AA0(j);
            TVec3f secondNormal(*second->mParentTriangle.getNormal(0));

            if (firstNormal.dot(secondNormal) >= -0.707f) {
                continue;
            }

            if ((first->mHitPos - center).dot(second->mHitPos - center) > 0.0f) {
                continue;
            }

            f32 distance = (first->mHitPos - second->mHitPos).length();

            if (distance < width) {
                width = distance;
                firstPos = first->mHitPos;
                secondPos = second->mHitPos;
            }
        }
    }

    if (width < 100.0f) {
        fixTransBetweenWall(firstPos, secondPos);
    }

    return width;
}

void Mario::updateCameraPolygon() {
    // NONMATCHING: scalar pool relocations; instructions and referenced values match.
    HitSensor* sensor;
    const Triangle* triangle;

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        triangle = mGroundPolygon;
    } else {
        triangle = _45C;
    }

    sensor = triangle->mSensor;

    if (triangle->isValid()) {
        if (sensor == nullptr) {
            *_468 = *triangle;
            *_46C = *triangle;
            return;
        }

        *_468 = *triangle;
        *_46C = *triangle;
        return;
    }

    TVec3f start = mPosHolder->mPos - *getGravityVec() * 100.0f;
    const Triangle* cameraTriangle = MR::getCameraPolyFast(start, *getGravityVec() * 5000.0f, nullptr);

    if (cameraTriangle != nullptr) {
        *_468 = *cameraTriangle;
        *_46C = *cameraTriangle;
    }
}

void Mario::checkAllWall(const TVec3f& rPosition) {
    TVec3f center(rPosition);
    TVec3f front(mFrontVec);
    _4D8->mIdx = -1;
    _4DC->mIdx = -1;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit16))) {
        TVec3f direction;
        mActor->getLastMove(&direction);
        MR::vecKillElement(direction, *getGravityVec(), &direction);
        if (!MR::isNearZero(direction, 0.001f)) {
            MR::normalize(&direction);
            front = direction;
        }
    }
    s32 count;
    f32 radius;
    bool forwardCheck = false;
    if (getPlayerMode() == 6) {
        radius = 110.0f;
        count = fn_803B2A20(center, 110.0f, 110.0f);
        if (!count) {
            forwardCheck = true;
            center.add(mFrontVec * 120.0f);
            count = fn_803B2950() ? Collision::checkStrikeBallToDashYoshiStepAndMapWithThickness(center, 25.0f, 25.0f, nullptr, nullptr)
                                 : Collision::checkStrikeBallToMapWithThickness(center, 25.0f, 25.0f, nullptr, nullptr);
        }
    } else if (isPlayerModeYoshi()) {
        radius = 110.0f;
        count = fn_803B2A20(center, 110.0f, 110.0f);
    } else {
        if (isSwimming()) radius = 90.0f;
        else radius = 80.0f;
        count = fn_803B29B0(center, radius);
    }
    const HitInfo* walls[3];
    for (s32 i = 0; i < 3; i++) walls[i] = nullptr;
    bool through = false;
    for (u32 i = 0; i < count; i++) {
        const HitInfo* hit = fn_803B2AA0(i);
        const Triangle* triangle = &hit->mParentTriangle;
        if (fn_80391260(triangle)) continue;
        TVec3f normal(*triangle->getNormal(0));
        TVec3f direction(hit->mHitPos);
        direction.sub(center);
        MR::normalizeOrZero(&direction);
        fn_803912D0(hit, normal, direction);
        s32 flat = MR::abs(normal.dot(mHeadVec)) <= 0.5f;
        if (!flat) {
            if (fn_803B2E80(&direction)) fn_803B2F20(hit);
        } else {
            f32 facing = normal.dot(front);
            s32 side;
            if (facing >= mActor->getConst().getTable()->mWallBackAngleRange) side = 2;
            else side = !(facing <= -mActor->getConst().getTable()->mWallFrontAngleRange);
            if (walls[side] && walls[side]->mDistance > hit->mDistance) continue;
            if (isThroughWall(triangle)) {
                through = true;
                mActor->_F44 = false;
            } else walls[side] = hit;
        }
    }
    if (!through && getPlayerMode() == 6 && !isStatusActive(0x13)) mActor->_F44 = true;
    fn_803B35D0(center, count);
    HitInfo pointHit;
    if (fn_803B2AF0(center, &pointHit)) {
        TVec3f normal(*pointHit.mParentTriangle.getNormal(0));
        if (MR::abs(normal.dot(mHeadVec)) <= 0.5f) {
            f32 facing = normal.dot(front);
            s32 side;
            if (facing >= 0.707f) side = 2;
            else side = !(facing <= -0.707f);
            walls[side] = &pointHit;
        }
    }
    mMovementStates.mLowBits.mBit23 = 0;
    mMovementStates.mLowBits.mBit6 = 0;
    mMovementStates.mLowBits.mBit5 = 0;
    mMovementStates.mHighBits.mBit13 = 0;
    mMovementStates.mHighBits.mBit12 = 0;
    if (walls[0]) fn_803B38B0(walls[0], &center);
    if (!forwardCheck) {
        if (walls[1]) fn_803B3AE0(walls[1], &center);
        if (walls[2]) fn_803B3BB0(walls[2], &center);
    }
    updateWallFloorCode();
    _38C = -_380;
    if (_400) {
        _400--;
        mMovementStates.mLowBits.mBit23 = 1;
    }
    if (isPlayerModeYoshi() && (static_cast<u32>(mMovementStates.mLowBits.mBit23)) && mFrontWallTriangle->isValid() && getCurrentStatus() == 0) {
        TVec3f delta(_4E8);
        delta.sub(center);
        f32 distance = PSVECMag(&delta);
        if (!MR::isNearZero(distance, 0.001f)) {
            delta.setLength(radius - distance);
            f32 penetration = -delta.dot(*mFrontWallTriangle->getNormal(0));
            f32 amount;
            if (penetration > 2.0f) amount = 2.0f;
            else amount = 0.0f;
            TVec3f pushVec(*mFrontWallTriangle->getNormal(0));
            pushVec.scale(amount);
            push(pushVec);
        }
    }
}

bool Mario::fn_80391260(const Triangle* triangle) {
    if (static_cast<u32>(mMovementStates.mHighBits.mBit8)) {
        if (MR::abs(triangle->getNormal(0)->dot(mSwimSideVec694)) > 0.707f) return true;
    }
    return false;
}

void Mario::fn_803912D0(const HitInfo* hit, const TVec3f& normal, const TVec3f& direction) {
    if (fn_80391330(hit, normal, direction)) {
        *_4DC = hit->mParentTriangle;
        _518 = hit->mHitPos;
    }
}

// NONMATCHING: register allocation and position-reference load scheduling.
bool Mario::fn_80391330(const HitInfo* hit, const TVec3f& normal, const TVec3f& direction) {
    f32 angle = 180.0f * (marioAcos(-getGravityVec()->dot(normal)) / 3.14159f);
    const TVec3f& position = mPosHolder->mPos;
    TVec3f fromBase(hit->mHitPos);
    fromBase.sub(position);
    MR::normalizeOrZero(&fromBase);
    bool result = false;
    bool slip = false;
    bool inFront = false;
    bool flat = false;
    if (angle >= mActor->getConst().getTable()->mFlatAngle && direction.dot(mFrontVec) > 0.0f) flat = true;
    if (flat && fromBase.dot(mFrontVec) > 0.0f) inFront = true;
    if (inFront && calcAngleD(normal) <= mActor->getConst().getTable()->mSlipAngle) slip = true;
    if (slip && MR::diffAngleAbs(_368, normal) > 30.0f) result = true;
    return result;
}

bool Mario::isThroughWall(const Triangle* pTriangle) const {
    if (getPlayerMode() == PlayerMode_Teresa) {
        if (!_418) {
            return false;
        }

        const char* code = MR::getWallCodeString(pTriangle);

        if (code != nullptr && !strcmp(code, "GhostThroughCode")) {
            return true;
        }
    }

    return false;
}

// NONMATCHING: boolean temporaries and register allocation.
bool Mario::checkGround() {
    if (isStatusActive(MarioStatus_Recovery) || isStatusActive(MarioStatus_Recovery)) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit11))) {
        if ((static_cast<u32>(mMovementStates.mHighBits.mBit9))) {
            bool ground = checkGroundOnSlope();

            if (ground) {
                mFlags20.mBits.mBit12 = 1;
                mFlags20.mBits.mBit11 = 1;
                return true;
            }

            return false;
        }

    } else {
        if (isUseSimpleGroundCheck()) {
            if (!(static_cast<u32>(mMovementStates.mHighBits.mBit9))) {
                if (MR::isNearZero(getGravityVec()->y - 1.0f)) {
                    mActor->setBlendMtxTimer(4);
                }

                mMovementStates.mHighBits.mBit9 = 1;
            }

            bool ground = checkGroundOnSlope();

            if (ground) {
                mFlags20.mBits.mBit12 = 1;
                mFlags20.mBits.mBit11 = 1;
                return true;
            }

            return false;
        }

        if ((static_cast<u32>(mMovementStates.mHighBits.mBit9))) {
            if (MR::isNearZero(getGravityVec()->y - 1.0f)) {
                mActor->setBlendMtxTimer(4);
            }

            mMovementStates.mHighBits.mBit9 = 0;
        }
    }

    TVec3f normal;

    if (isAnimationRun("崖ふんばり")) {
        normal = -*getGravityVec();
    } else {
        normal = *_45C->getNormal(0);
    }

    TVec3f offset;
    MR::vecKillElement(mFrontVec, normal, &offset);

    if (MR::isNearZero(offset)) {
        return false;
    }

    offset.setLength(50.0f);
    TVec3f axis;
    axis = normal;

    if (MR::isNearZero(axis)) {
        return false;
    }

    TMtx34f rotation;
    PSMTXRotAxisRad(rotation, &axis, 2.0943952f);
    u8 hit[4];
    Triangle triangles[4];
    TVec3f positions[4];
    bool noGround = false;
    mMovementStates.mLowBits.mBit11 = 0;
    f32 maxDistance;

    maxDistance = 30.0f;
    if (getCurrentStatus() == MarioStatus_Freeze) {
        maxDistance = 5.0f;
    } else if (getCurrentStatus() == MarioStatus_Slider) {
        maxDistance = 100.0f;
    } else if (((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && isRising()) {
        maxDistance = 10.0f;
    }

    TVec3f sum;
    TVec3f selectedPos;
    u32 count = 3;

    if (!mTargetWalkSpeedIndex && (static_cast<u32>(mMovementStates.mLowBits.mBit30)) && !((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && !(static_cast<u32>(mMovementStates.mHighBits.mBit28))) {
        count = 4;
    }

    if (isAnimationRun("壁押し", 0)) {
        count = 4;
    }

    s32 groundCount = 0;
    sum.zero();
    bool select = true;

    for (u32 i = 0; i < count; i++) {
        TVec3f start = mPosHolder->mPos + offset - *getGravityVec() * 30.0f;

        if (i == 3) {
            start = mPosHolder->mPos - *getGravityVec() * 30.0f;
        }

        hit[i] = fn_803B2B50(start, *getGravityVec() * 100.0f, &positions[i], &triangles[i]);

        if (i == 3) {
            break;
        }

        if (hit[i]) {
            if (_414 && MR::abs((mPosHolder->mPos - positions[i]).dot(*getGravityVec())) > 0.0f) {
                hit[i] = 0;
            }

            if (MR::abs((mPosHolder->mPos - positions[i]).dot(*getGravityVec())) > maxDistance) {
                hit[i] = 0;
            }

            if (calcAngleD(*triangles[i].getNormal(0)) >= 80.0f) {
                hit[i] = 0;
            }

            f32 dot = getGravityVec()->dot(*triangles[i].getNormal(0));

            if (getCurrentStatus() != MarioStatus_Slider && dot > _3C) {
                hit[i] = 0;
            }

            if (isStatusActive(MarioStatus_Recovery) && MR::isThroughPolygon(&triangles[i])) {
                hit[i] = 0;
            }

            if (hit[i]) {
                sum += positions[i];
                groundCount++;

                if ((static_cast<u32>(mMovementStates.mHighBits.mBit28)) && !select) {
                    TVec3f direction(_16C);
                    MR::normalizeOrZero(&direction);

                    if ((positions[i] - mPosHolder->mPos).dot(direction) > (selectedPos - mPosHolder->mPos).dot(direction)) {
                        select = true;
                    }
                }

                if (select) {
                    setGroundNorm(*triangles[i].getNormal(0));
                    *mGroundPolygon = triangles[i];
                    select = false;
                    selectedPos = positions[i];
                    mGroundPos = selectedPos;
                    recordLastGround();
                }

                const char* wallCode = MR::getWallCodeString(&triangles[i]);

                if (wallCode != nullptr && !strcmp(wallCode, "Fur")) {
                    mDrawStates.mBits.mBit6 = 1;
                }
            }
        }

        PSMTXMultVec(rotation, &offset, &offset);
    }

    u32 i;

    for (i = 0; i < count; i++) {
        if (!hit[i]) {
            break;
        }
    }

    if (i == count) {
        mFlags20.mBits.mBit14 = 1;
    }

    if (!isNoWalkFallOnDossun() && !isStatusActive(MarioStatus_Faint) && !isStatusActive(MarioStatus_Damage) && !isStatusActive(MarioStatus_Talk) &&
        !isStatusActive(MarioStatus_Hang) && (!(static_cast<u32>(mMovementStates.mLowBits.mBit7)) || (static_cast<u32>(mFlags14.mLowBits.mBit15)))) {
        while ((count == 4 && !hit[3]) || _960 == CollisionFloorCode_Slide) {
            u8 anyHit;
            const u8 backHit = hit[2];
            const u32 frontHit = hit[0];
            const u8 sideHit = hit[1];
            anyHit = backHit;
            anyHit |= frontHit;
            anyHit |= sideHit;
            if (!anyHit) {
                return false;
            }

            if (!frontHit) {
                bool moved = false;

                if (!(static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
                    addTrans(mFrontVec * 6.0f);
                    moved = true;
                }

                if (sideHit && backHit) {
                    if (!moved) {
                        break;
                    }

                } else if (!sideHit) {
                    if (((static_cast<u32>(mMovementStates.mLowBits.mBit5)) || (static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mLowBits.mBit6))) && mSideVec.dot(getWallNorm()) < 0.0f) {
                        if (moved) {
                            addTrans(-mSideVec * 3.0f - mFrontVec * 6.0f);
                        }

                        break;
                    } else {
                        addTrans(mSideVec * 3.0f);
                        moved = true;
                    }

                } else {
                    if (((static_cast<u32>(mMovementStates.mLowBits.mBit5)) || (static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mLowBits.mBit6))) && -mSideVec.dot(getWallNorm()) < 0.0f) {
                        if (moved) {
                            addTrans(mSideVec * 3.0f - mFrontVec * 6.0f);
                        }

                        break;
                    } else {
                        addTrans(-mSideVec * 3.0f);
                        moved = true;
                    }
                }

                if (moved) {
                    mDrawStates.mBits.mBit21 = 1;
                }

                return true;
            } else if (!sideHit && !backHit) {
                stopWalk();

                if (_3CE < 24 && mJumpVec.dot(mFrontVec) >= 0.0f && _960 != CollisionFloorCode_Slide) {
                    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit23)) && !(static_cast<u32>(mMovementStates.mHighBits.mBit13))) {
                        addTrans(mFrontVec * 3.0f);
                        mDrawStates.mBits.mBit21 = 1;
                    }

                } else {
                    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
                        mDrawStates.mBits.mBit7 = 1;
                        addTrans(mFrontVec * -6.0f);
                        mDrawStates.mBits.mBit21 = 1;
                    }

                    if (_3C6 > 8) {
                        addTrans(mSideVec * 5.0f);
                    }

                    return true;
                }

            } else if (_960 == CollisionFloorCode_Slide) {
                if (!backHit) {
                    addTrans(-mSideVec * 5.0f);
                } else if (!sideHit) {
                    addTrans(mSideVec * 5.0f);
                } else {
                    addTrans(mFrontVec * 5.0f);
                }

                mDrawStates.mBits.mBit21 = 1;
            }

            break;
        }
    }

    if (!groundCount) {
        noGround = true;
    }

    s32 sameSensor = 0;
    s32 sameSlope = 0;
    TVec3f groundHorizontal;
    MR::vecKillElement(*mGroundPolygon->getNormal(0), getAirGravityVec(), &groundHorizontal);

    for (u32 i = 0; i < groundCount; i++) {
        if (hit[i]) {
            if (triangles[i].mSensor == mGroundPolygon->mSensor) {
                sameSensor++;
            } else {
                sameSensor--;
            }

            TVec3f horizontal;
            MR::vecKillElement(*triangles[i].getNormal(0), getAirGravityVec(), &horizontal);

            if (horizontal.dot(groundHorizontal) >= 0.0f) {
                sameSlope++;
            }
        }
    }

    if (sameSensor > 0) {
        mFlags20.mBits.mBit12 = 1;
    }

    if (sameSlope >= 3) {
        mFlags20.mBits.mBit11 = 1;
    }

    _8EC = sameSensor;

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit18))) {
        mMovementStates.mLowBits.mBit18 = 0;
        TVec3f direction = mGroundPos - mPosHolder->mPos;
        f32 distance = MR::vecKillElement(direction, *getGravityVec(), &direction);
        addTrans(*getGravityVec() * distance);
        return true;
    }

    if (groundCount) {
        f32 distance = 0.0f;
        TVec3f horizontal;

        if (calcAngleD(*_45C->getNormal(0)) < 55.0f) {
            distance = MR::vecKillElement(mShadowPos - mPosHolder->mPos, *getGravityVec(), &horizontal);
        }

        if (MR::abs(distance) > 30.0f) {
            distance = MR::vecKillElement(mGroundPos - mPosHolder->mPos, *getGravityVec(), &horizontal);
        }

        if (MR::abs(distance) < 30.0f && MR::abs(distance) > 1.0f && (static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
            f32 dot = getGravityVec()->dot(-_368);

            if (dot > 0.99f) {
                if (!(static_cast<u32>(mDrawStates.mBits.mBit22)) && !_4D8->isValid()) {
                    if (!isStatusActive(MarioStatus_Talk)) {
                        addTrans(*getGravityVec() * distance);
                    }

                    return true;
                }

            } else if (dot > 0.0f && !(static_cast<u32>(mDrawStates.mBits.mBit22)) && (static_cast<u32>(mMovementStates.mHighBits.mBit28))) {
                addTrans(-_368 * distance * dot);
                return true;
            }
        }
    }

    if (((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && isRising()) {
        return false;
    }

    if (!isStatusActive(MarioStatus_Bury) && count == 4 && noGround) {
        if (hit[3] == 1 && MR::abs((mPosHolder->mPos - positions[3]).dot(*getGravityVec())) < maxDistance) {
            noGround = false;
        }

        bool moveFront = true;
        bool moveBack = true;

        if ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
            moveFront = false;
        }

        if ((static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
            moveBack = false;
        }

        if (mActor->mBinderDirection5DC.dot(mFrontVec) < 0.0f) {
            if (moveBack) {
                addTrans(-mFrontVec * 6.0f);
            }

        } else if (moveFront) {
            addTrans(mFrontVec * 6.0f);
        }
    }

    if (noGround) {
        mMovementStates.mLowBits.mBit11 = 1;
        return false;
    }

    if (!mActor->_EA4 && getCurrentStatus() != MarioStatus_Hang) {
        if ((((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && groundCount) || ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) && mVerticalSpeed >= 5.0f)) {
            TVec3f direction;
            f32 distance = MR::vecKillElement(mGroundPos - mShadowPos, _368, &direction);
            f32 maxMove = 5.0f;
            direction = mShadowPos - mPosHolder->mPos;

            if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) && direction.dot(getFrontWallNorm()) < -0.01f) {
                maxMove = 0.0f;
            }

            if ((static_cast<u32>(mMovementStates.mLowBits.mBit5)) && direction.dot(getSideWallNorm()) < -0.01f) {
                maxMove = 0.0f;
            }

            if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) && direction.dot(getBackWallNorm()) < -0.01f) {
                maxMove = 0.0f;
            }

            if (maxMove != 0.0f) {
                if (direction.length() > maxMove) {
                    mShadowPos = mShadowPos - mPosHolder->mPos;
                    mShadowPos.setLength(maxMove);
                    mShadowPos += mPosHolder->mPos;
                }

                if (distance < maxDistance) {
                    setTrans(mShadowPos);
                }
            }
        }

        if (!groundCount && MR::abs((mShadowPos - mPosHolder->mPos).dot(*getGravityVec())) < maxDistance) {
            TVec3f direction = mShadowPos - mPosHolder->mPos;
            f32 distance = MR::vecKillElement(direction, *getGravityVec(), &direction);

            if (!MR::isNearZero(distance, 1.0f) || !(static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
                mVelocity += *getGravityVec() * distance;
                mDrawStates.mBits.mBit31 = 1;
            }

            return true;
        }
    }

    if (!groundCount && MR::abs((mGroundPos - mPosHolder->mPos).dot(*getGravityVec())) < maxDistance) {
        TVec3f direction = mGroundPos - mPosHolder->mPos;
        f32 distance = MR::vecKillElement(direction, *getGravityVec(), &direction);

        if (!MR::isNearZero(distance, 1.0f)) {
            mVelocity += *getGravityVec() * distance;
            mDrawStates.mBits.mBit31 = 1;
        }

        return true;
    }

    if (mVerticalSpeed < 5.0f) {
        return true;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit25))) {
        return true;
    }

    if (groundCount) {
        return true;
    }

    return (static_cast<u32>(mDrawStates.mBits.mBit31));
}

void Mario::fn_803929A0() {
    this->fn_803B3010();
    Triangle sp18;
    TVec3f sp8;
    if (this->fn_803B3040(&sp8, &sp18)) {
        this->fn_803B3280(sp8, &sp18);
    }
}

const TVec3f& Mario::getWallNorm() const {
    if (((mMovementStates.mLowWord >> 23) & 1U)) {
        return *mFrontWallTriangle->getNormal(0);
    }

    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        return *mBackWallTriangle->getNormal(0);
    }

    if (((mMovementStates.mLowWord >> 5) & 1U)) {
        return *mSideWallTriangle->getNormal(0);
    }

    if (mFrontWallTriangle->isValid()) {
        return *mFrontWallTriangle->getNormal(0);
    }

    return mFrontVec;
}

const TVec3f& Mario::getSideWallNorm() const {
    if (((mMovementStates.mLowWord >> 5) & 1U)) {
        return *mSideWallTriangle->getNormal(0);
    }

#ifdef GALAXY_NATIVE
    return sNativeZeroVec;  // the Wii build returns a reference to a temporary copy of gZeroVec
#else
    return TVec3f(gZeroVec);
#endif
}

const TVec3f& Mario::getFrontWallNorm() const {
    if (((mMovementStates.mLowWord >> 23) & 1U)) {
        return *mFrontWallTriangle->getNormal(0);
    }

#ifdef GALAXY_NATIVE
    return sNativeZeroVec;  // the Wii build returns a reference to a temporary copy of gZeroVec
#else
    return TVec3f(gZeroVec);
#endif
}

const TVec3f& Mario::getBackWallNorm() const {
    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        return *mBackWallTriangle->getNormal(0);
    }

#ifdef GALAXY_NATIVE
    return sNativeZeroVec;  // the Wii build returns a reference to a temporary copy of gZeroVec
#else
    return TVec3f(gZeroVec);
#endif
}

const TVec3f& Mario::getWallPos() const {
    if (((mMovementStates.mLowWord >> 23) & 1U)) {
        return _4E8;
    }

    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        return _4F4;
    }

    if (((mMovementStates.mLowWord >> 5) & 1U)) {
        return _500;
    }

    return mActor->_240;
}

const Triangle* Mario::getWallPolygon() const {
    if (((mMovementStates.mLowWord >> 23) & 1U)) {
        return mFrontWallTriangle;
    }

    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        return mBackWallTriangle;
    }

    if (((mMovementStates.mLowWord >> 5) & 1U)) {
        return mSideWallTriangle;
    }

    if (mFrontWallTriangle->isValid()) {
        return mFrontWallTriangle;
    }

    return nullptr;
}

const Triangle* Mario::getGroundPolygon() const {
    return mGroundPolygon;
}

// NONMATCHING: literal pool offsets; all string arguments verified.
void Mario::updateCubeCode() {
    AreaObj* raster;
    s32 first;
    CubeCameraArea* camera;

    if ((isSwimming() && mSwim->isOnWaterSurface()) || ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && isRising())) {
        camera = static_cast< CubeCameraArea* >(MR::getAreaObj("CubeCamera", mPosHolder->mPos - *getGravityVec() * 100.0f));
    } else {
        camera = static_cast< CubeCameraArea* >(MR::getAreaObj("CubeCamera", mPosHolder->mPos));
    }

    _570 = 0;

    if (camera != nullptr) {
        _564 = MR::getAreaObjArg(camera, 0);
        switch (MR::getAreaObjArg(camera, 1)) {
        case 1:
            mDrawStates.mBits.mBit29 = 1;
            break;
        case 2:
            mDrawStates.mBits.mBit28 = 1;
            break;
        }

    } else {
        _564 = -1;
    }

    MR::tryToUpdatePlayerRestartIdInfo(mPosHolder->mPos);
    AreaObj* pullBack = MR::getAreaObj("PullBack", mPosHolder->mPos);
    if (pullBack != nullptr && MR::isAreaFormCylinder(pullBack) && MR::getAreaObjArg(pullBack, 0) != 1) {
        TVec3f position;
        TVec3f up;
        MR::calcCylinderPos(&position, pullBack);
        MR::calcCylinderUpVec(&up, pullBack);
        MarioRecovery* recovery = mRecovery;
        recovery->_4C = position;
        recovery->_58 = up;
        recovery->_12 = true;
    }

    if (pullBack != nullptr) {
        doRecovery();
    }

    mFlags14.mLowBits.mBit25 = 0;

    if (MR::getAreaObj("PlaneCollisionCube", mPosHolder->mPos) != nullptr) {
        mFlags14.mLowBits.mBit25 = 1;
    }

    { const u32 flagValue = static_cast<u32>(MR::isInTowerModeCylinder(mPosHolder->mPos, &_6F4, &_700, &_718)); mFlags14.mLowBits.mBit12 = flagValue; }

    if (MR::getAreaObj("ForbidTriangleJump", mPosHolder->mPos) != nullptr) {
        mDrawStates.mBits.mBit28 = 1;
    }

    if (getPlayerMode() == PlayerMode_Teresa) {
        if (MR::getAreaObj("GlaringLightArea", mPosHolder->mPos) != nullptr) {
            mActor->setPlayerMode(PlayerMode_Normal, true, true);
        }

        if (!MR::isInShadeFromTheSun(mPosHolder->mPos, 2000.0f)) {
            mActor->setPlayerMode(PlayerMode_Normal, true, true);
        }
    }

    if (MR::getAreaObj("FallsCube", mActor->mCapPosition) != nullptr) {
        touchWater();
        playEffectRTZ("水壁ヒット", mHeadVec, mActor->mCapPosition);
    }

    if (MR::getAreaObj("HeavySteeringCube", mPosHolder->mPos) != nullptr) {
        mFlags14.mLowBits.mBit14 = 1;
        mFlags14.mLowBits.mBit13 = 1;
    } else {
        mFlags14.mLowBits.mBit14 = 0;
        mFlags14.mLowBits.mBit13 = 0;
    }

    if (MR::getAreaObj("DashChargeArea", mPosHolder->mPos) != nullptr) {
        _434 = mActor->getConst().getTable()->mItemDashTimer;
    }

    raster = MR::getAreaObj("RasterScrollArea", mPosHolder->mPos);

    if (raster != nullptr) {
        first = MR::getAreaObjArg(raster, 0);
        s32 second = MR::getAreaObjArg(raster, 1);
        mActor->setRasterScroll(first, second, MR::getAreaObjArg(raster, 2));
    }

    if (MR::getAreaObj("ForbidJumpCube", mPosHolder->mPos) != nullptr) {
        mFlags20.mBits.mBit24 = 1;
    }

    if (!isStatusActive(MarioStatus_DarkDamage)) {
        if (MR::getAreaObj("DarkMatterArea", mPosHolder->mPos) != nullptr || _960 == 41 || checkWallFloorCode(41)) {
            mActor->forceKill(4);
        }

        mVelocity.zero();
    }
}

void Mario::updateCometCode() {
    if (MR::isGalaxyHorrorCometAppearInCurrentStage()) {
        if (MR::getAreaObj("GlaringLightArea", mPosHolder->mPos)) {
            mCometRestrictions |= 2;
        } else {
            mCometRestrictions &= ~2u;
        }
        if (MR::getAreaObj("SunLightArea", mPosHolder->mPos)) {
            mCometRestrictions |= 4;
        } else {
            mCometRestrictions &= ~4u;
        }
    }
    if (mCometHopperArea) {
        if (!isPlayerModeHopper()) {
            mActor->setPlayerMode(PlayerMode_Hopper, false, false);
            mCometHopperApplied = true;
        }
    } else if (mCometHopperApplied && isPlayerModeHopper()) {
        mActor->setPlayerMode(PlayerMode_Normal, false, false);
        mCometHopperApplied = false;
    }
    if (mCometRainbowArea) {
        if (!isPlayerModeInvincible()) {
            mActor->setPlayerMode(PlayerMode_Invincible, false, false);
            mActor->_3DC = 0;
            mCometRainbowApplied = true;
        }
    } else if (mCometRainbowApplied && isPlayerModeInvincible()) {
        mActor->setPlayerMode(PlayerMode_Normal, false, false);
        mCometRainbowApplied = false;
    }
}

void Mario::updateFloorCode() {
    s32 code = _95C->getCode(mGroundPolygon);

    if (code != -1) {
        _960 = code;
    }

    code = _95C->getCode(_45C);

    if (code != -1) {
        _962 = code;
    }

    if (mActor->mBeeWallWalk) {
        _41C = 15;
        return;
    }

    f32 angle = calcPolygonAngleD(mGroundPolygon);

    if (angle > mActor->getConst().getTable()->mSlipAngle) {
        if (angle > mActor->getConst().getTable()->mForceWallAngle) {
            _41C = 0;
        }

        if (_41C) {
            _41C--;
        }

        if (!_41C) {
            _960 = 0x80;
        }

    } else {
        bool noSlip = !isSlipFloorCode(_960);

        if (noSlip) {
            _41C = 15;
        }
    }
    _A80 = _A7F;
    _A7F = 0;
    if (isPlayerModeYoshi() && _960 == 32) {
        _960 = 0;
        _A7F = 1;
    }

}

void Mario::updateWallFloorCode() {
    if (((mMovementStates.mLowWord >> 23) & 1U) || ((mMovementStates.mHighWord >> 13) & 1U)) {
        s32 code = _95C->getCode(mFrontWallTriangle);

        if (code != -1) {
            _964[0] = code;
        }
    }

    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        s32 code = _95C->getCode(mBackWallTriangle);

        if (code != -1) {
            _964[1] = code;
        }
    }

    if (((mMovementStates.mLowWord >> 5) & 1U)) {
        s32 code = _95C->getCode(mSideWallTriangle);

        if (code != -1) {
            _964[2] = code;
        }
    }
}

void Mario::saveLastSafetyTrans() {
    if (isStatusActive(MarioStatus_FireDamage)) {
        return;
    }

    if (isStatusActive(MarioStatus_Recovery)) {
        return;
    }

    if (!(static_cast<u32>(mFlags20.mBits.mBit14))) {
        return;
    }

    if (_96A) {
        _96A--;
        return;
    }

    switch (_960) {
    case CollisionFloorCode_Ice:
    case CollisionFloorCode_GlassIce:
        if (!MR::isInAreaObj("RecoveryFloorControlArea", getTrans())) {
            return;
        }
        break;
    case CollisionFloorCode_Normal:
    case CollisionFloorCode_NoSlip:
    case CollisionFloorCode_Sand:
    case CollisionFloorCode_Glass:
    case CollisionFloorCode_WaterBottomH:
    case CollisionFloorCode_WaterBottomM:
    case CollisionFloorCode_WaterBottomL:
    case CollisionFloorCode_Wet:
    case CollisionFloorCode_NoStampSand:
    case 0x25:  // "Lawn" (SMG2 floor code, CollisionCode.cpp table; no CollisionFloorCode enumerator yet)
        break;
    default:
        return;
    }

    if (mGroundPolygon->mSensor->isType(ATYPE_MAP_OBJ_MOVE_COLLISION)) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit13))) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit5))) {
        return;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit19))) {
        return;
    }

    if ((static_cast<u32>(mFlags20.mBits.mBit16))) {
        return;
    }

    if (!(mVerticalSpeed < 10.0f)) {
        return;
    }

    if (!(calcAngleD(*mGroundPolygon->getNormal(0)) < 15.0f)) {
        return;
    }

    if (!MR::isSameMtx(*mGroundPolygon->getBaseMtx(), *mGroundPolygon->getPrevBaseMtx())) {
        return;
    }

    if (_7E0->isValid() && MR::isSameMtx(*_7E0->getBaseMtx(), _7E4) && MR::isSameMtx(*_7E0->getBaseMtx(), *_7E0->getPrevBaseMtx())) {
        _814 = _7D4;
        *_820 = *_7E0;
        PSMTXCopy(_7E4, _824);
    }

    *_7E0 = *mGroundPolygon;
    PSMTXCopy(*mGroundPolygon->getBaseMtx(), _7E4);
    _7D4 = (mPosHolder->mPos * 5.0f + *mGroundPolygon->getPos(0) + *mGroundPolygon->getPos(1) + *mGroundPolygon->getPos(2)) * 0.125f;

    if ((_7D4 - mPosHolder->mPos).length() > 50.0f) {
        TVec3f direction = _7D4 - mPosHolder->mPos;
        direction.setLength(50.0f);
        _7D4 = mPosHolder->mPos + direction;
    }

    mFlags20.mBits.mBit17 = 1;
}

void Mario::setNotSafetyTimer() {
    _96A = 2;

    if (((mFlags20.mWord >> 17) & 1U)) {
        _7E0->mIdx = -1;
    }
}

const TVec3f* Mario::getLastSafetyTrans(TVec3f* pNormal) const {
    if (pNormal != nullptr) {
        pNormal->set(-getAirGravityVec());
    }

    if (_7E0->isValid() && MR::isSameMtx(*_7E0->getBaseMtx(), const_cast< TMtx34f& >(_7E4))) {
        if (pNormal != nullptr) {
            pNormal->set(*_7E0->calcAndGetNormal(0));
        }

        return &_7D4;
    }

    if (_820->isValid() && pNormal) {
        pNormal->set(*_820->calcAndGetNormal(0));
    }

    return &_814;
}

bool Mario::checkCurrentFloorCodeSevere(u32 code) const {
    if (_960 != code) {
        return false;
    }

    TVec3f horizontal;

    TVec3f delta(mShadowPos - mGroundPos);
    if (MR::vecKillElement(delta, getAirGravityVec(), &horizontal) > 30.0f) {
        return code == _95C->getCode(mGroundPolygon);
    }

    u32 shadowCode = _95C->getCode(_45C);

    if (shadowCode != code) {
        return false;
    }

    return shadowCode == _95C->getCode(mGroundPolygon);
}

bool Mario::isCurrentFloorSink() const {
    bool sink = false;
#ifdef GALAXY_NATIVE
    u16 shifted = _960 + 0xFFEF;
    if (shifted <= 14 && ((1 << shifted) & 0x4103)) {
#else
    // The original shifts by the untruncated sum (slw uses its low six bits).
    u32 shifted = _960 + 0xFFEF;
    if (static_cast<u16>(shifted) <= 14 && ((1 << shifted) & 0x4103)) {
#endif
        sink = true;
    }
    return sink;
}

bool Mario::isCurrentFloorSand() const {
    if (((getPlayer()->mDrawStates.mWord & Mario::DrawStates::Underwater) >> 13)) {
        return false;
    }

    if (((getPlayer()->mDrawStates.mWord >> 12) & 1U)) {
        return false;
    }

    return checkCurrentFloorCodeSevere(CollisionFloorCode_Sand) || checkCurrentFloorCodeSevere(CollisionFloorCode_NoStampSand);
}

bool Mario::isCurrentShadowFloorDangerAction() const {
    s32 code = _95C->getCode(_45C);

    if (code == _95C->getCode(mGroundPolygon)) {
        return false;
    }

    if (isPlayerModeTeresa()) {
        return false;
    }

    switch (code) {
    case CollisionFloorCode_DamageFire:
        if (isPlayerModeIce()) {
            return false;
        }

    case CollisionFloorCode_DamageNormal:
    case CollisionFloorCode_JumpLow:
    case CollisionFloorCode_JumpMiddle:
    case CollisionFloorCode_JumpHigh:
    case CollisionFloorCode_JumpNormal:
    case CollisionFloorCode_DamageElectric:
    case CollisionFloorCode_Sink:
    case CollisionFloorCode_SinkPoison:
    case CollisionFloorCode_Needle:
    case CollisionFloorCode_SinkDeath:
    case CollisionFloorCode_RailMove:
    case CollisionFloorCode_AreaMove:
    case CollisionFloorCode_SinkDeathMud:
    case CollisionFloorCode_JumpParasol:
        return true;
    default:
        return false;
    }
}

bool Mario::isDamaging() const {
    if (isAnimationRun("水上ダメージ中")) {
        return true;
    }

    if (_41E) {
        return true;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit4))) {
        return true;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit24))) {
        return true;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit19))) {
        return true;
    }

    if ((static_cast<u32>(mFlags14.mLowBits.mBit31))) {
        return true;
    }

    if ((static_cast<u32>(mFlags14.mLowBits.mBit11))) {
        return true;
    }

    if ((static_cast<u32>(mFlags14.mLowBits.mBit7))) {
        return true;
    }

    switch (getCurrentStatus()) {
    case MarioStatus_FireDamage:
    case MarioStatus_FireDance:
    case MarioStatus_FireRun:
    case MarioStatus_Paralyze:
    case MarioStatus_AbyssDamage:
    case MarioStatus_Freeze:
    case MarioStatus_Crush:
        return true;
    case MarioStatus_Damage:
        return mDamage->_12;
    case MarioStatus_Faint:
        return mFaint->mTookDamage;
    default:
        return false;
    }
}

bool Mario::fn_80393C40() {
    if (((u32)this->mFlags14.mLowWord >> 31) != 0) {
        return true;
    }
    return (u32)(this->getCurrentStatus() - 8) <= 2U;
}

void Mario::decDamageAfterTimer() {
    MarioDamage* damage = mDamage;
    if (damage->_16) {
        damage->_16--;
    }

    MarioFaint* faint = mFaint;
    if (faint->_14) {
        faint->_14--;
    }

    MarioParalyze* paralyze = mParalyze;
    if (paralyze->_16) {
        paralyze->_16--;
    }

    MarioFreeze* freeze = mFreeze;
    if (freeze->_1C) {
        freeze->_1C--;
    }

    mFireDamage->decAfterTimer();
    if (_41E) {
        _41E--;
    }
}

bool Mario::checkDamage() {
    decDamageAfterTimer();
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit0))) {
        return false;
    }

    if (mActor->_EA4) {
        return false;
    }

    if (MR::isDemoActive()) {
        return false;
    }

    if (isStatusActive(MarioStatus_Talk)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Recovery) || isInvincible()) {
        mMovementStates.mLowBits.mBit4 = 0;
        mMovementStates.mHighBits.mBit24 = 0;
        mMovementStates.mHighBits.mBit19 = 0;
        mFlags14.mLowBits.mBit31 = 0;
        mFlags14.mLowBits.mBit11 = 0;
        mFlags14.mLowBits.mBit7 = 0;
        mFaint->mNoDamage = false;
        return false;
    }

    if (isStatusActive(MarioStatus_Swim)) {
        checkWaterDamage();
        mMovementStates.mHighBits.mBit19 = 0;
        mFlags14.mLowBits.mBit31 = 0;
        mFlags14.mLowBits.mBit11 = 0;
        mFlags14.mLowBits.mBit7 = 0;
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit4)) ||
        ((static_cast<u32>(mMovementStates.mHighBits.mBit24)) && MR::isGalaxyHorrorCometAppearInCurrentStage())) {
        mMovementStates.mLowBits.mBit4 = 0;
        changeStatus(mDamage);
        mMovementStates.mHighBits.mBit24 = 0;
        mFaint->mNoDamage = false;
        mMovementStates.mHighBits.mBit19 = 0;
        return true;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit24))) {
        mMovementStates.mHighBits.mBit24 = 0;
        mMovementStates.mHighBits.mBit19 = 0;
        if (isPlayerModeYoshi()) {
            mDamage->_30 = true;
            changeStatus(mDamage);
        } else {
            changeStatus(mFaint);
        }
        return true;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit19))) {
        mMovementStates.mHighBits.mBit19 = 0;
        changeStatus(mBlown);
        return true;
    }

    if ((static_cast<u32>(mFlags14.mLowBits.mBit31))) {
        mFlags14.mLowBits.mBit31 = 0;
        doFireDanceWithInitialDamage(1);
        return true;
    }

    if ((static_cast<u32>(mFlags14.mLowBits.mBit11))) {
        mFlags14.mLowBits.mBit11 = 0;
        doParalyze();
        return true;
    }

    if ((static_cast<u32>(mFlags14.mLowBits.mBit7))) {
        mFlags14.mLowBits.mBit7 = 0;
        if (tryCrush()) {
            return true;
        }
    }

    checkKarikariDamage();
    return false;
}

u16 Mario::getDamageAfterTimer() const {
    u16 timer = mDamage->_16;
    if (timer < mFaint->_14) {
        timer = mFaint->_14;
    }

    if (timer < mParalyze->_16) {
        timer = mParalyze->_16;
    }

    if (timer < mFreeze->_1C) {
        timer = mFreeze->_1C;
    }

    if (timer < mFireDamage->_12) {
        timer = mFireDamage->_12;
    }

    if (timer < _41E) {
        timer = _41E;
    }

    return timer;
}

bool Mario::damageFloorCheck() {
    if (((mMovementStates.mLowWord >> 0) & 1U)) {
        return false;
    }

    if (((mFlags20.mWord >> 9) & 1U)) {
        return false;
    }

    if (mActor->mFlag1064 != 0) {
        return doFireDanceWithInitialDamage(1);
    }

    switch (_960) {
    case CollisionFloorCode_Needle:
        if (checkCurrentFloorCodeSevere(CollisionFloorCode_Needle) && doNeedleWithInitialDamage(mGroundPolygon)) {
            return true;
        }

        break;
    case 0x81:
        if (checkCurrentFloorCodeSevere(0x81) && doFireDanceWithInitialDamage(1)) {
            return true;
        }

        break;
    case CollisionFloorCode_Death:
        MarioAccess::forceKill(3, 0);
        return true;
    case CollisionFloorCode_DamageFire:
        if (checkCurrentFloorCodeSevere(CollisionFloorCode_DamageFire) && doFireDanceWithInitialDamage(1)) {
            return true;
        }

        break;
    case CollisionFloorCode_DamageNormal:
        if (isDamaging()) {
            return false;
        }

        if (damage(_368 * 10.0f)) {
            return true;
        }

        break;
    case CollisionFloorCode_DamageElectric:
        if (doParalyze()) {
            return true;
        }

        break;
    case CollisionFloorCode_PullBack:
        if (doRecovery()) {
            return true;
        }

        break;
    }

    return false;
}

bool Mario::damageWallCheck() {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit0))) {
        return false;
    }

    TVec3f normal;
    if (checkWallCodeNorm(CollisionWallCode_Rebound, &normal, false)) {
        return doFlipJump(normal * 5.0f);
    }

    if (checkWallFloorCode(CollisionFloorCode_Death)) {
        mActor->forceKill(3);
        return true;
    }

    if (checkWallFloorCode(CollisionFloorCode_DamageFire) && doFireDanceWithInitialDamage(1)) {
        return true;
    }

    if (checkWallFloorCode(CollisionFloorCode_Needle) && doNeedleWithInitialDamage(1)) {
        return true;
    }

    if (checkWallFloorCode(CollisionFloorCode_DamageElectric) && doParalyze()) {
        return true;
    }

    if (checkWallFloorCode(CollisionFloorCode_PullBack) && doRecovery()) {
        return true;
    }

    if (checkWallFloorCode(CollisionFloorCode_DamageNormal) && damage(getWallNorm() * 10.0f)) {
        return true;
    }

    return false;
}

bool Mario::damagePolygonCheck(const Triangle* pTriangle) {
    if (((mMovementStates.mLowWord >> 0) & 1U)) {
        return false;
    }

    switch (_95C->getCode(pTriangle)) {
    case 0x81:
        if (doFireDanceWithInitialDamage(1)) {
            return true;
        }

        break;
    case CollisionFloorCode_Death:
        MarioAccess::forceKill(3, 0);
        return true;
    case CollisionFloorCode_DamageFire:
        if (doFireDanceWithInitialDamage(1)) {
            return true;
        }

        break;
    case CollisionFloorCode_DamageNormal:
        mSwim->addDamage(*MR::getNormal(pTriangle) * 10.0f);
        return true;
    case CollisionFloorCode_DamageElectric:
        if (doParalyze()) {
            return true;
        }

        break;
    case CollisionFloorCode_PullBack:
        if (doRecovery()) {
            return true;
        }

        break;
    case CollisionFloorCode_Needle:
        doNeedleWithInitialDamage(pTriangle);
        return true;
    case CollisionFloorCode_SinkDeath:
        return true;
    }

    return false;
}

bool Mario::fn_80394550(const TVec3f* arg) {
    if (this->damage(*arg)) {
        if (this->isStatusActive(6)) {
            this->mSwim->fn_803F7C00();
        } else {
            MarioConstTable* e = this->mActor->mConst->getTable();
            this->mDamage->setVecSize(e->mJumpDistLargeDamage, e->mJumpHeightLargeDamage);
            this->playSound("投げられ", -1);
        }
        return true;
    }
    return false;
}

bool Mario::flipLarge(const TVec3f& rDirection) {
    if (damage(rDirection)) {
        if (isStatusActive(MarioStatus_Swim)) {
            mSwim->fn_803F7C00();
            mSwim->fn_803F7C10();
        } else {
            mDamage->_11 = 1;
        }

        mDamage->setVecSize(rDirection.length(), 0.0f);
        return true;
    }

    return false;
}

bool Mario::isEnableAddDamage() const {
    if (getCurrentStatus() == MarioStatus_Talk) {
        return false;
    }

    if (isDamaging()) {
        return false;
    }

    if (mActor->_390) {
        return false;
    }

    if (isInvincible()) {
        return false;
    }

    return getDamageAfterTimer() == 0;
}

bool Mario::damage(const TVec3f& rDirection) {
    _7C4 = rDirection;
    if (!isEnableAddDamage()) {
        return false;
    }

    if (mFaint->_14 || mDamage->_16 || ((mMovementStates.mLowWord >> 4) & 1U)) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 16) & 1U)) {
        forceStopTornado();
    }

    mDamage->setVec(rDirection);
    stopWalk();
    forceStopTornado();
    if (isStatusActive(MarioStatus_Damage)) {
        closeStatus(mDamage);
    }

    if (isPlayerModeYoshi() && isStatusActive(MarioStatus_Swim)) {
        closeStatus(mSwim);
    }
    if (isStatusActive(MarioStatus_FpView)) {
        closeStatus(mFpView);
    }
    mActor->damageDropThrowMemoSensor();

    mMovementStates.mLowBits.mBit4 = 1;
    return true;
}

bool Mario::doAbyssDamage() {
    if (getCurrentStatus() == MarioStatus_AbyssDamage) {
        return false;
    }

    stopWalk();
    mActor->damageDropThrowMemoSensor();
    MR::removeAllClingingKarikari();
    mActor->_A6E = 0;
    changeStatus(mAbyssDamage);
    return true;
}

void Mario::connectToFireRun() {
    if (mActor->mHealth) {
        changeStatus(mFireRun);
        stopJump();
        mFireRun->_12 = 1;
    }
}

bool Mario::doFireDanceWithInitialDamage(u8 amount) {
    if (((mMovementStates.mLowWord >> 0) & 1U)) {
        return false;
    }

    bool started = doFireDance();
    if (started) {
        for (u32 i = 0; i < amount; i++) {
            mActor->decLife(0);
        }

        mActor->fn_803BE960();
        if (!mActor->mHealth) {
            mActor->forceGameOverNonStop();
        }
    }

    return started;
}

bool Mario::fn_803949E0() {
    if (this->isEnableAddDamage() == false) {
        return false;
    }
    if (this->isStatusActive(6)) {
        this->mMovementStates.mLowBits.mBit4 = 1;
        this->_7C4.zero();
        return this->checkWaterDamage();
    }
    return this->doFireDanceWithInitialDamage(1);
}

bool Mario::doNeedleWithInitialDamage(u8 amount) {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit0))) {
        return false;
    }

    if (getPlayerMode() == PlayerMode_Teresa) {
        doTeresaReflection(getWallNorm(), false);
        return false;
    }

    MarioActor* actor;
    bool started = doNeedle(nullptr);
    if (started) {
        for (u32 i = 0; i < amount; i++) {
            mActor->decLife(0);
        }

        actor = mActor;
        const TVec3f& normal = getWallNorm();
        actor->fn_803BE840(normal);

        if (!mActor->mHealth) {
            mActor->forceGameOverNonStop();
        }
    }

    return started;
}

bool Mario::doNeedleWithInitialDamage(const Triangle* pTriangle) {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit0))) {
        return false;
    }

    if (getPlayerMode() == PlayerMode_Teresa) {
        doTeresaReflection(*MR::getNormal(pTriangle), false);
        return false;
    }

    MarioActor* actor;
    bool started = doNeedle(pTriangle);
    if (started) {
        mActor->decLife(0);
        actor = mActor;
        const TVec3f& normal = *MR::getNormal(pTriangle);
        actor->fn_803BE840(normal);

        if (!mActor->mHealth) {
            mActor->forceGameOverNonStop();
        }
    }

    return started;
}

bool Mario::doNeedle(const Triangle* pTriangle) {
    if (getCurrentStatus() == MarioStatus_FireDamage) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireRun) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireDance) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 4) & 1U)) {
        return false;
    }

    if (getPlayerMode() == PlayerMode_Teresa) {
        if (pTriangle != nullptr) {
            doTeresaReflection(*MR::getNormal(pTriangle), false);
        }

        return false;
    }

    if (isInvincible()) {
        return false;
    }

    mActor->fn_803B94E0();
    getPlayer()->mMovementStates.mLowBits.mBit20 = 0;
    getPlayer()->mMovementStates.mLowBits.mBit21 = 0;
    mFireDance->_29 = 1;
    changeStatus(mFireDance);
    mActor->damageDropThrowMemoSensor();
    return true;
}

bool Mario::doFireDance() {
    if (getCurrentStatus() == MarioStatus_Paralyze) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireDamage) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireRun) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireDance) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 4) & 1U)) {
        return false;
    }

    if (isInvincible()) {
        return false;
    }

    if (getPlayerMode() == PlayerMode_Ice) {
        return false;
    }

    mActor->fn_803B94E0();
    getPlayer()->mMovementStates.mLowBits.mBit20 = 0;
    getPlayer()->mMovementStates.mLowBits.mBit21 = 0;
    mFireDance->_29 = 0;
    changeStatus(mFireDance);
    mActor->damageDropThrowMemoSensor();
    mFireDamage->_12 = 120;
    return true;
}

void Mario::checkKarikariDamage() {
    if (!(static_cast<u32>(mFlags20.mBits.mBit26)) || isDamaging()) {
        _7D0 = 120;
        return;
    }

    if (mActor->_934) {
        return;
    }

    if (mActor->_EA4) {
        return;
    }

    if (isStatusActive(MarioStatus_Talk)) {
        return;
    }

    if ((static_cast<u32>(mFlags20.mBits.mBit26)) && _7D0) {
        _7D0--;
        if (!_7D0) {
            if (mActor->mHealth == 1) {
                faint(mHeadVec);
                _7D0 = 120;
                return;
            }

            startPadVib(2);
            playSound("声小ダメージ");
            playSound("ダメージ");
            mActor->fn_803B9270();
            mActor->_BC4 = 16;
            _7D0 = 120;
            if (!mActor->mHealth) {
                mActor->forceGameOver();
            }
        }
    }
}

bool Mario::doDarkDamage() {
    if (getCurrentStatus() == MarioStatus_DarkDamage) {
        return false;
    }

    mActor->_3C0 = true;
    stopWalk();
    mActor->damageDropThrowMemoSensor();
    playSound("声沼沈み");
    playEffect("ダークマター死亡");
    setSeVersion(1);
    changeStatus(mDarkDamage);
    return true;
}

bool Mario::requestCrush() {
    mFlags14.mLowBits.mBit7 = 1;
    return true;
}

bool Mario::tryCrush() {
    if (((mMovementStates.mLowWord >> 0) & 1U)) {
        return false;
    }

    if (isInvincible()) {
        return false;
    }

    if (((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) || !((mMovementStates.mLowWord >> 30) & 1U)) {
        setTrans(mShadowPos);
    }

    if (getCurrentStatus() == MarioStatus_Crush) {
        return false;
    }

    mActor->fn_803B94E0();

    stopJump();
    stopWalk();

    changeStatus(mCrush);
    mActor->damageDropThrowMemoSensor();
    return true;
}

bool Mario::doFreeze() {
    if (((mMovementStates.mLowWord >> 0) & 1U)) {
        return false;
    }

    if (isInvincible()) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_Freeze) {
        return false;
    }

    if (mFreeze->_1C != 0) {
        return false;
    }

    mActor->fn_803B94E0();

    stopJump();
    stopWalk();

    cancelSquatMode();

    stopAnimationUpper(nullptr);
    changeStatus(mFreeze);
    mActor->damageDropThrowMemoSensor();
    return true;
}

bool Mario::doParalyze() {
    if (((mMovementStates.mLowWord >> 0) & 1U)) {
        return false;
    }

    if (isInvincible()) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_Paralyze) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireDamage) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 4) & 1U)) {
        return false;
    }

    if (getDamageAfterTimer() != 0) {
        return false;
    }

    if (isDamaging()) {
        return false;
    }

    mActor->fn_803B94E0();

    stopJump();
    stopWalk();

    changeStatus(mParalyze);
    mActor->damageDropThrowMemoSensor();
    return true;
}

bool Mario::fn_80395310() {
    if (this->mMovementStates.mLowWord & 1) {
        return false;
    }
    if (this->isInvincible() != 0) {
        return false;
    }
    u32 temp_r3 = this->mMovementStates.mLowWord;
    if (((temp_r3 >> 31) != 0) || !((temp_r3 >> 30) & 1)) {
        return false;
    }
    if (this->getCurrentStatus() == 0xE) {
        return false;
    }
    if (this->getCurrentStatus() == 0xF) {
        return false;
    }
    if (this->getCurrentStatus() == 8) {
        return false;
    }
    if ((this->mMovementStates.mLowWord >> 4) & 1) {
        return false;
    }
    this->mActor->fn_803B94E0();
    this->stopJump();
    this->stopWalk();
    this->changeStatus(this->mStun);
    return true;
}

void Mario::fn_80395400(u32 arg) {
    this->pushTask(&Mario::taskOnEffectCheck, arg);
}

bool Mario::taskOnEffectCheck(u32 flags) {
    if (flags & 0x2) {
        return mActor->checkEffectWaterColumn();
    }

    return true;
}

void Mario::checkEnforceMove() {
    checkEnforceMoveInner();
    updateOnimasu();
}

void Mario::checkEnforceMoveInner() {
    mMovementStates.mLowBits.mBit7 = 0;
    TVec3f v15(_184);
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) != 0 || (static_cast<u32>(mMovementStates.mLowBits.mBit31)) != 0) {
        _184.zero();
    }

    _8D4 = nullptr;
    u32 relativeMove = moveRelativePosition(0);
    bool same = relativeMove | moveRelativePositionWall();
    if (same) {
        if (MR::isSameMtx(*_45C->getBaseMtx(), *_45C->getPrevBaseMtx())) {
            f32 verticalLimit = ((getMovementStates().mLowWord >> 30) & 1U) ? 0.0f : mVerticalSpeed;

            if (mVelocity.dot(_368) < 0.0f) {
                if (!(static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
                    f32 element = MR::vecKillElement(mVelocity, _368, &mVelocity);
                    f32 v8 = element;

                    if (element < 0.0f) {
                        if (-element > mVerticalSpeed) {
                            v8 = -mVerticalSpeed;
                        }
                    }

                    mVelocity += _368 * v8;
                }
            }
        }

        mDrawStates.mBits.mBit20 = 1;

        if (!mActor->_F44) {
            addTrans(mVelocity);
        } else {
            bool v11 = false;
            TVec3f v14(mPosHolder->mPos);

            if ((static_cast<u32>(mMovementStates.mLowBits.mBit21)) == 0) {
                if (200.0f == _72C) {
                    v11 = true;
                }
            }

            addTrans(mVelocity);

            if (v11) {
                if (calcDistToCeil(false) < 200.0f) {
                    mPosHolder->fn_80405F90(v14);
                    mFlags20.mBits.mBit15 = 1;
                }
            }

            mActor->mBinder->clear();
        }

        mVelocity.zero();
        MarioActor* actor = mActor;
        actor->mPosition = mPosHolder->mPos;
        actor->calcCenterPos();
    }
}

void Mario::recordRelativePosition() {
    const Triangle* sameSensorFloor = nullptr;
    const TVec3f* pPosition;
    TVec3f strikeCenter(mActor->_240);
    _8CC[0]->mIdx = -1;
    _8CC[1]->mIdx = -1;

    f32 strikeRadius = 85.0f;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) && _45C->isValid() && MR::isSameMtx(*_45C->getBaseMtx(), *_45C->getPrevBaseMtx())) {
        strikeRadius = 81.0f;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit21))) {
        strikeRadius = 51.0f;
    }

    if (mActor->isJumpRising()) {
        invalidateRelativePosition();
        return;
    }

    const u32 strikeNum = static_cast< u32 >(fn_803B29B0(strikeCenter, strikeRadius));
    const f32 floorDotThreshold = 0.707f;
    const f32 wallNormalDotThreshold = 0.9f;

    for (u32 i = 0; i < strikeNum; i++) {
        const HitInfo* hitInfo = fn_803B2AA0(i);
        const Triangle* tri = &hitInfo->mParentTriangle;

        if (tri->mSensor == nullptr) {
            continue;
        }

        if (MR::isSameMtx(*tri->getBaseMtx(), *tri->getPrevBaseMtx())) {
            continue;
        }

        if (tri->getNormal(0)->dot(-*getGravityVec()) < floorDotThreshold) {
            if (!_8CC[0]->isValid()) {
                *_8CC[0] = *tri;
                continue;
            }

            if (tri->getNormal(0)->dot(*_8CC[0]->getNormal(0)) > wallNormalDotThreshold) {
                continue;
            }

            if (!_8CC[1]->isValid()) {
                *_8CC[1] = *tri;
                continue;
            }

            const f32 candidateDot = tri->getNormal(0)->dot(*_8CC[0]->getNormal(0));
            const f32 currentDot = _8CC[0]->getNormal(0)->dot(*_8CC[1]->getNormal(0));
            if (currentDot > candidateDot) {
                *_8CC[1] = *tri;
            }

            continue;
        }

        sameSensorFloor = tri;
        if (tri->mSensor != mHang->mWallSensor) {
            break;
        }
    }

    if (sameSensorFloor == nullptr) {
        if (isStatusActive(MarioStatus_Wall)) {
            *_8C8 = *mFrontWallTriangle;
        } else if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
            if ((static_cast<u32>(mFlags20.mBits.mBit12))) {
                *_8C8 = *mGroundPolygon;
            } else if (isUseAnotherMovingPolygon()) {
                *_8C8 = *mGroundPolygon;
            } else {
                invalidateRelativePosition();
            }
        } else {
            invalidateRelativePosition();
        }
    } else if ((static_cast<u32>(mFlags20.mBits.mBit12))) {
        if (sameSensorFloor->mSensor == mGroundPolygon->mSensor) {
            *_8C8 = *sameSensorFloor;
        } else if (isUseAnotherMovingPolygon()) {
            *_8C8 = *sameSensorFloor;
        } else {
            invalidateRelativePosition();
        }
    } else if (isUseAnotherMovingPolygon()) {
        *_8C8 = *sameSensorFloor;
    } else {
        invalidateRelativePosition();
    }

    if (_8C8->isValid()) {
        if (fn_803EFA30(_8C8->mSensor, "地形移動無視")) {
            invalidateRelativePosition();
        } else {
            pPosition = &mActor->mPosition;
            PSMTXMultVec(*_8C8->getBaseInvMtx(), pPosition, &_8A4);
        }
    }

    if (_8CC[0]->isValid()) {
        if (fn_803EFA30(_8CC[0]->mSensor, "地形移動無視")) {
            _8CC[0]->mIdx = -1;
        } else {
            pPosition = &mActor->mPosition;
            PSMTXMultVec(*_8CC[0]->getBaseInvMtx(), pPosition, &mRelativeWallPos[0]);
        }
    }

    if (_8CC[1]->isValid()) {
        if (fn_803EFA30(_8CC[1]->mSensor, "地形移動無視")) {
            _8CC[1]->mIdx = -1;
        } else {
            pPosition = &mActor->mPosition;
            PSMTXMultVec(*_8CC[1]->getBaseInvMtx(), pPosition, &mRelativeWallPos[1]);
        }
    }
}

void Mario::invalidateRelativePosition() {
    _8C8->mIdx = -1;
    _8CC[0]->mIdx = -1;
    _8CC[1]->mIdx = -1;
}

TPos3f* Mario::getMoveBaseMtx() const {
    if (_8C8->isValid()) {
        return _8C8->getBaseMtx();
    }

    return nullptr;
}

u32 Mario::moveRelativePosition(u32) {
    Triangle* rTri = _8C8;
    if (!rTri->isValid()) {
        return 0;
    }

    if (MR::isSameMtx(*rTri->getBaseMtx(), *rTri->getPrevBaseMtx())) {
        return 0;
    }

    if (mActor->_EA4 && !isStatusActive(MarioStatus_Talk)) {
        invalidateRelativePosition();
        return 0;
    }

    if (rTri->mSensor != nullptr
        && (MR::isDead(rTri->mSensor->mHost) || MR::isEqualSubString(rTri->mSensor->mHost->mName, "リピートブロック"))) {
        invalidateRelativePosition();
        return 0;
    }

    TVec3f movePos;
    PSMTXMultVec(*rTri->getBaseMtx(), &_8A4, &movePos);
    _184 = movePos - mPosHolder->mPos;

    if (isSwimming()) {
        TVec3f dist(mActor->_240);
        dist -= movePos;

        TVec3f normal(*rTri->getNormal(0));
        if (dist.dot(normal) > 0.0f && dist.dot(_184) > 0.0f) {
            addVelocity(_184);
        }

        return 0;
    }

    addVelocity(_184);

    if (_8D4 == nullptr || rTri->mSensor != mHang->mWallSensor) {
        _8D4 = rTri->mSensor;
    }

    if (!MR::isSameMtxRot(*rTri->getBaseMtx(), *rTri->getPrevBaseMtx())) {
        mMovementStates.mLowBits.mBit7 = 1;

        if (MR::isRotAxisY(*rTri->getBaseMtx(), *rTri->getPrevBaseMtx())) {
            mFlags14.mLowBits.mBit15 = 1;
        }

        TVec3f rotAxis;
        MR::calcMtxRotAxis(&rotAxis, *rTri->getBaseMtx(), *rTri->getPrevBaseMtx());
        if (MR::isSameDirection(rotAxis, getAirGravityVec())) {
            mFlags14.mLowBits.mBit15 = 1;
        }
    }

    return 1;
}

u32 Mario::moveRelativePositionWall() {
    const TVec3f* position;
    u32 moved = 0;

    for (u32 i = 0; i < 2; i++) {
        Triangle* tri = _8CC[i];
        if (!tri->isValid()) {
            continue;
        }

        if (MR::isSameMtx(*tri->getBaseMtx(), *tri->getPrevBaseMtx())) {
            continue;
        }

        if (tri->mSensor != nullptr && !tri->mSensor->isValid()) {
            tri->mIdx = -1;
            continue;
        }

        const TVec3f* relPos = &mRelativeWallPos[i];
        TVec3f movePos;
        PSMTXMultVec(*tri->getBaseMtx(), relPos, &movePos);

        position = &mPosHolder->mPos;
        TVec3f add(movePos);
        add -= *position;

        const f32 elem = MR::vecKillElement(add, *tri->getNormal(0), &add);
        if (elem > 0.0f) {
            addVelocity(*tri->getNormal(0) * elem);
        }

        if (!MR::isSameMtxRot(*tri->getBaseMtx(), *tri->getPrevBaseMtx())) {
            mMovementStates.mLowBits.mBit7 = 1;
        }

        if (_8D4 == nullptr || tri->mSensor != mHang->mWallSensor) {
            _8D4 = tri->mSensor;
        }

        moved = 1;
    }

    return moved;
}

void Mario::recordLastGround() {
    *_474 = *mGroundPolygon;
    PSMTXMultVec(*_474->getBaseInvMtx(), &mGroundPos, &_4BC);
}

void Mario::getLastGroundPos(TVec3f* pPos) const {
    if (_474->isValid()) {
        PSMTXMultVec(*_474->getBaseMtx(), &_4BC, pPos);
    } else {
        pPos->zero();
    }
}

u32 Mario::getLastGroundEdgeIndex(const TVec3f& rPos, const TVec3f& rDir) const {
    const TVec3f* edge0 = _474->calcAndGetEdgeNormal(0);
    const TVec3f* edge1 = _474->calcAndGetEdgeNormal(1);
    const TVec3f* edge2 = _474->calcAndGetEdgeNormal(2);

    f32 score0 = rDir.dot(*edge0);
    f32 score1 = rDir.dot(*edge1);
    f32 score2 = rDir.dot(*edge2);

    TVec3f centerAvg(*_474->calcAndGetPos(0) + *_474->calcAndGetPos(1) + *_474->calcAndGetPos(2));

    centerAvg *= 1.0f / 3.0f;

    TVec3f outDir(centerAvg - rPos);
    MR::normalizeOrZero(&outDir);

    const f32 add0 = outDir.dot(*edge0);
    const f32 add1 = outDir.dot(*edge1);
    const f32 add2 = outDir.dot(*edge2);
    if (add0 > 0.0f) {
        score0 += add0;
    }

    if (add1 > 0.0f) {
        score1 += add1;
    }

    if (add2 > 0.0f) {
        score2 += add2;
    }

    if (score0 < score1 && score0 < score2) {
        return 0;
    }

    if (score1 < score0 && score1 < score2) {
        return 1;
    }

    return 2;
}

// NONMATCHING: relocation labels; all scalar and string values verified.
void Mario::pushedByWind() {
    f32 windMag = _91C.length();

    TVec3f windDir(_91C);
    MR::normalizeOrZero(&windDir);

    if (MR::isNearZero(windDir)) {
        if (isAnimationRun("向かい風ふんばり")) {
            stopAnimation("向かい風ふんばり");
        }

        if (isAnimationRun("向かい風走り")) {
            stopAnimation("向かい風走り");
        }

        if (static_cast<u32>(mFlags14.mLowBits.mBit9)) {
            mFlags14.mLowBits.mBit9 = 0;
        }
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        const MarioConstTable* table = mActor->getConst().getTable();
        mJumpVec += _91C * table->mWindJumpingFriction;
        return;
    }

    if (mTargetWalkSpeedIndex == 0) {
        const MarioConstTable* table = mActor->getConst().getTable();
        if (windMag > table->mWindSlideLimit) {
            windMag -= table->mWindSlideLimit;
            windMag *= table->mWindSlideFriction;
            push(windDir * windMag);
            changeAnimation("向かい風ふんばり");
        }
        return;
    }

    TVec3f scaled(windDir * windMag);

    if (mVelocity.dot(windDir) > 0.0f) {
        const MarioConstTable* table = mActor->getConst().getTable();
        push(scaled * table->mWindForwardFriction);
        stopAnimation("向かい風ふんばり");
        stopAnimation("向かい風走り");
    } else {
        TVec3f killed;
        f32 element = MR::vecKillElement(scaled + mVelocity, windDir, &killed);

        const MarioConstTable* table = mActor->getConst().getTable();
        if (element > table->mWindSlideLimit) {
            element -= table->mWindSlideLimit;
            element *= table->mWindSlideFrictionAgainst;
        } else if (element > 0.0f) {
            element = 0.0f;
        }

        clearVelocity();

        scaled = killed + windDir * element;

        changeAnimation("向かい風走り");
    }
    push(scaled);
}

void Mario::pushedByReaction() {
    addVelocity(_928);
    _928 *= _934;
}

void Mario::addReaction(const TVec3f& rReact) {
    _928 += rReact;
}

void Mario::tryPushToVelocity() {
    _1B4 = mVelocity;
    pushedByWind();
    pushedByReaction();

    if (!MR::isNearZero(_350)) {
        if (isSwimming()) {
            TVec3f pushVec(_350);
            pushVec += _35C;
            _350.zero();
            _35C.zero();
            createAtField(true, pushVec.length());

            for (u32 i = 0; i < _578; i++) {
                TVec3f projected;
                if (MR::vecKillElement(pushVec, *_57C[i]->getNormal(0), &projected) < 0.0f) {
                    pushVec = projected;
                }
            }

            mVelocity += pushVec;
        } else if (!isStatusActive(MarioStatus_Hang)) {
            if (isStatusActive(MarioStatus_Bury)) {
                closeStatus(mBury);
            } else {
                if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && !(static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
                    TVec3f pushDir(_350);
                    MR::normalizeOrZero(&pushDir);

                    if (mJumpVec.dot(_350) < 0.0f) {
                        cutVecElementFromJumpVec(pushDir);
                    }

                    if (mVelocity.dot(_350) < 0.0f) {
                        MR::vecKillElement(mVelocity, pushDir, &mVelocity);
                    }
                } else if (isDamaging() && (static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
                    MR::vecKillElement(_350, *getGravityVec(), &_350);
                }

                mVelocity += _350;
            }
        }

        mFlags20.mBits.mBit10 = 1;
    }

    mVelocity += _35C;
    _350.zero();
    _35C.zero();
}

void Mario::push(const TVec3f& rVec) {
    _350 += rVec;
    mFlags20.mBits.mBit10 = 1;
}

void Mario::push2(const TVec3f& rVec) {
    _35C += rVec;
    mFlags20.mBits.mBit10 = 1;
}

void Mario::powerAreaMove() {
    _190.zero();

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) && !(static_cast<u32>(mMovementStates.mLowBits.mBit31)) && _960 == 28 && MR::calcAreaMoveVelocity(&_190, mPosHolder->mPos)) {
        if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mLowBits.mBit6)) || (static_cast<u32>(mMovementStates.mLowBits.mBit5))) {
            const TVec3f* norm = &getWallNorm();
            f32 dot = norm->dot(_190);

            if (dot < 0.0f) {
                MR::vecKillElement(_190, *norm, &_190);
                MR::vecKillElement(_190, *getGravityVec(), &_190);
            }
        }

        addVelocity(_190);
    }
}

void Mario::powerRailMove() {
    _19C.zero();

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) && !(static_cast<u32>(mMovementStates.mLowBits.mBit31)) && _960 == 0x1B) {
        const LiveActor* host = getGroundPolygon()->mSensor->mHost;
        if (host == nullptr) {
            return;
        }

        TVec3f railDir;
        MR::calcNearestRailDirection(&railDir, host, mPosHolder->mPos);

        s32 arg = -1;
        MR::getRailArg3NoInit(host, &arg);
        s32 speed = arg;
        if (speed == -1) {
            speed = 10;
        }

        MR::normalizeOrZero(&railDir);

        if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mLowBits.mBit6)) || (static_cast<u32>(mMovementStates.mLowBits.mBit5))) {
            const TVec3f* wallNorm = &getWallNorm();
            if (wallNorm->dot(railDir) < 0.0f) {
                MR::vecKillElement(railDir, *wallNorm, &railDir);
                MR::vecKillElement(railDir, *getGravityVec(), &railDir);
            }
        }

        TVec3f velocity(railDir);
        velocity.scale(speed);
        _19C = velocity;
        addVelocity(_19C);
    }
}

void Mario::recordJumpEnforceMove() {
    mFlags14.mLowBits.mBit8 = 1;
    _8D8 = _8D4;
    _8DC = _184;
    *_8E8 = *_8C8;
}

void Mario::doEnforceJump(f32 param) {
    if (mJumpVec.dot(_184) < 0.0f) {
        mDrawStates.mBits.mBit4 = 1;
        initActiveJumpVec();
    }

    _8DC = _184 * param;

    f32 element = MR::vecKillElement(_8DC, getAirGravityVec(), &_8DC);

    mJumpVec = _8DC + mJumpVec;

    if (element < 0.0f) {
        mJumpVec += getAirGravityVec() * element;
    }

    mVelocity -= _184;
    invalidateRelativePosition();
}

#include "../src/Game/Player/MarioSMG2_A.inc"
#include "../src/Game/Player/MarioSMG2_B.inc"
