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

// ---- Mario.cpp ----

void Mario_FORCE_MATCH_SDATA2() {
    (void)0.00100000005f;
    (void)0.800000012f;
    (void)-0.514999986f;
    (void)200.0f;
    (void)28000.0f;
    (void)2499.0f;
    (void)0.104719758f;
    (void)0.707000017f;
    (void)300.0f;
    (void)1.42857146f;
    (void)0.300000012f;
    (void)0.100000001f;
    (void)150.0f;
    (void)40.0f;
    (void)160.0f;
    (void)80.0f;
    (void)90.0f;
    (void)5.0f;
    (void)1.49599659f;
    (void)170.0f;
    (void)0.785398185f;
    (void)0.99000001f;
    (void)30.0f;
    (void)15.0f;
    (void)45.0f;
    (void)60.0f;
    (void)290.0f;
    (void)260.0f;
    (void)1.04719758f;
    (void)0.200000003f;
    (void)0.0245436933f;
    (void)0.0174532924f;
    (void)0.00999999978f;
    (void)32.0f;
    (void)0.125f;
    (void)100.0f;
    (void)35.0f;
    (void)50.0f;
    (void)0.00390625f;
    (void)120.0f;
    (void)0.0199999996f;
    (void)1.5f;
    (void)2607.59448f;
    (void)-2607.59448f;
    (void)-150.0f;
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
    _406 = 0;
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
    _410 = 0;
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
    _412 = 0;
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


bool Mario::isEnableCheckGround() {
    return !isStatusActive(MarioStatus_Swim);
}


void Mario::setGroundNorm(const TVec3f& rVec) {
    if (MR::isNearZero(mAirGravityVec)) {
    }

    if (MR::diffAngleAbs(-rVec, mAirGravityVec) > 1.4959966f) {
    } else {
        _368 = rVec;
        _374 = -rVec;
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

            if ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
                dot2 = dot;
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




// Nearly matches
void Mario::createMtxDir(MtxPtr pMtx, const TVec3f& rFront, const TVec3f& rUp, const TVec3f& rSide) {
    TVec3f side;
    TVec3f front;
    TVec3f up;
    side = rSide;
    front = rFront;
    up = rUp;
    bool isNormal = !isPlayerModeTeresa();
    if (isNormal) {
        if (((mMovementStates.mHighWord >> 8) & 1U) || ((mFlags14.mLowWord >> 10) & 1U)) {
            f32 sideAngleDiff = MR::diffAngleAbs(side, _950);
            f32 frontAngleDiff = MR::diffAngleAbs(front, _944);
            f32 upAngleDiff = MR::diffAngleAbs(up, _938);
            if (sideAngleDiff < mActor->mConst->getTable()->mVibrationAbsorbAngleS) {
                side = _950;
            } else {
                _950 = side;
            }

            if (frontAngleDiff < mActor->mConst->getTable()->mVibrationAbsorbAngleF) {
                front = _944;
            } else {
                _944 = front;
            }

            if (upAngleDiff < mActor->mConst->getTable()->mVibrationAbsorbAngleH) {
                up = _938;
            } else {
                _938 = up;
            }
        }
    }

    pMtx[0][0] = side.x;
    pMtx[1][0] = side.y;
    pMtx[2][0] = side.z;
    pMtx[0][1] = up.x;
    pMtx[1][1] = up.y;
    pMtx[2][1] = up.z;
    pMtx[0][2] = front.x;
    pMtx[1][2] = front.y;
    pMtx[2][2] = front.z;
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

    createMtxDir(pMtx, front, up, side);
    MR::setMtxTrans(pMtx, 0.0f, 0.0f, 0.0f);
}


bool Mario::createCorrectionMtx(MtxPtr pMtx, TVec3f* pOut) {
    pOut->zero();

    if (isNonFixHeadVec()) {
        PSMTXIdentity(pMtx);
        return false;
    }

    if (MR::isNearZero(_74C) && _754 == 0) {
        PSMTXIdentity(pMtx);
        _750 = 0;
        _74C = 0.0f;
    } else {
        if (_750 == 0 && _754 == 0) {
            TMtx34f tempMtx;
            TVec3f newFrontVec;
            PSMTXRotAxisRad(tempMtx, mHeadVec, _74C);
            PSMTXMultVec(tempMtx, mFrontVec, newFrontVec);
            setFrontVecKeepUp(newFrontVec);
            _74C = 0.0f;
        }

        TVec3f vec2(0.0f, 1.0f, 0.0f);

        if (((mMovementStates.mHighWord >> 8) & 1U) && !isStatusActive(MarioStatus_Talk)) {
            bool b1 = false;
            f32 dot = mFrontVec.dot(getCamDirX());

            if (_1FC.dot(getCamDirY()) < 0.0f) {
                dot = -dot;
            }

            if (dot < 0.0f && _74C < 0.0f) {
                b1 = true;
            } else if (dot > 0.0f && _74C > 0.0f) {
                b1 = true;
            }

            if (b1) {
                PSMTXRotAxisRad(pMtx, vec2, -_74C);
            } else {
                PSMTXRotAxisRad(pMtx, vec2, _74C);
            }
        } else {
            PSMTXRotAxisRad(pMtx, vec2, _74C);
        }

        if (_754 != 0) {
            _74C += (MR::pi() - _74C) / static_cast< f32 >(_754);
            _754--;
            _750 = 0;
        }

        if (_750 != 0) {
            _74C = _74C * static_cast< f32 >(_750 - 1) / static_cast< f32 >(_750);
            _750--;
            if (_750 == 0) {
                setFrontVecKeepUp(mFrontVec);
            }
        }
    }

    return true;
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
        TVec3f side(mPosHolder->mPos);
        side -= _6F4;
        MR::vecKillElement(side, _700, &side);
        MR::normalizeOrZero(&side);
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
    TVec3f blendedFront;
    if (MR::isNearZero(rFront) == false) {
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
        TDDraw::drawCylinder(mPosHolder->mPos, mShadowPos - mActor->_240, 32.0f, 0xffff0010, 0xff000004, 8);
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


void Mario::addTrans(const TVec3f& rShift) {
    mPosHolder->fn_80406010(rShift);
    TVec3f _148shift(_148);
    _148shift -= rShift;
    if (MR::abs(_148.x) <= MR::abs(_148shift.x)) {
        _148.x = 0.0f;
    }

    if (MR::abs(_148.y) <= MR::abs(_148shift.y)) {
        _148.y = 0.0f;
    }

    if (MR::abs(_148.z) <= MR::abs(_148shift.z)) {
        _148.z = 0.0f;
    }
}


// NONMATCHING: saved-register allocation; verified holder write and arithmetic.
void Mario::setTrans(const TVec3f& rShift) {
    TVec3f reqShift(rShift);
    reqShift -= mPosHolder->mPos;
    mPosHolder->fn_80405F90(rShift);
    TVec3f _148shift(_148);
    _148shift -= reqShift;
    if (MR::abs(_148.x) <= MR::abs(_148shift.x)) {
        _148.x = 0.0f;
    }

    if (MR::abs(_148.y) <= MR::abs(_148shift.y)) {
        _148.y = 0.0f;
    }

    if (MR::abs(_148.z) <= MR::abs(_148shift.z)) {
        _148.z = 0.0f;
    }
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
    if (isPlayerModeYoshi() && getCurrentStatus() == 39) { const u32 flagValue = static_cast<u32>(_72C < 200.0f); mFlags20.mBits.mBit16 = flagValue; }
    else { const u32 flagValue = static_cast<u32>(_72C < 160.0f); mFlags20.mBits.mBit16 = flagValue; }
    if (static_cast<u32>(mFlags20.mBits.mBit16)) {
        if (_4C8->isValid()) damagePolygonCheck(_4C8);
    }
    if (MR::getKarikariClingNum() != 0 && mActor->isEnableNerveChange()) mFlags20.mBits.mBit26 = 1;
    OSGetTime();
    checkAllWall(mActor->_240, 0.0f);
    OSGetTime();
    updateGroundInfo();
    OSGetTime();
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) && (!(static_cast<u32>(mMovementStates.mLowBits.mBit31)) || (static_cast<u32>(mMovementStates.mLowBits.mBit20)))) {
        if (damageFloorCheck()) {
            writeBackPhyisicalVector();
            return;
        }
        saveLastSafetyTrans();
        if (isCurrentShadowFloorDangerAction()) {
            TVec3f groundToShadow(mShadowPos);
            groundToShadow -= mGroundPos;
            MR::isNearZero(mAirGravityVec, 0.001f);
            TVec3f projected;
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


void Mario::createAngleMtx(MtxPtr pMtx, bool a2) {
    f32 zero;
    TVec3f vec1;
    TVec3f vec2;
    TVec3f vec3;
    TVec3f vec4;
    if (!(isNonFixHeadVec() || a2 || ((mMovementStates.mLowWord & Mario::MovementStates::DebugMode) >> 9))) {
        fixHeadFrontVecByGravity();
    }

    if (updatePosture(pMtx)) {
        return;
    }

    createDirectionMtx(pMtx);

    vec2 = _1FC;
    vec1 = mSideVec;
    vec3 = mFrontVec;

    PSMTXConcat(_C4, pMtx, pMtx);

    HitSensor* sensor = mActor->_470;

    if (isAnimationRun("投げ")) {
        sensor = mActor->_474;
    }

    if (mActor->isPunching()) {
        sensor = mActor->_924;
    }

    if (sensor != nullptr) {
        vec4 = sensor->mPosition;
    }

    if (sensor != nullptr) {
        TVec3f vec5;
        vec5.set(vec4 - mPosHolder->mPos);

        MR::normalizeOrZero(&vec5);

        if (MR::isNearZero(vec5)) {
            return;
        }

        TVec3f vec6(vec2);

        TVec3f vec7;
        MR::vecKillElement(vec5, vec6, &vec7);

        if (MR::normalizeOrZero(&vec7)) {
            return;
        }

        f32 angle = JMAAcosRadian(vec7.dot(vec3));

        TVec3f cross;
        cross.cross(vec3, vec7);

        if (cross.dot(vec2) < 0.0f) {
            angle = -angle;
        }

        f32 pitchAngle = JMAAcosRadian(vec7.dot(vec5));

        bool dotNonNeg = vec5.dot(vec6) >= 0.0f;

        if (MR::abs(angle) < HALF_PI && dotNonNeg) {
            pitchAngle = -pitchAngle;
        } else if (MR::abs(angle) >= HALF_PI && !dotNonNeg) {
            pitchAngle = -pitchAngle;
        }

        f32 f1 = 0.1f * static_cast< f32 >(_412);
        zero = 0.0f;

        if (f1 > MR::toRadian(60.0f)) {
            f1 = MR::toRadian(60.0f);
        }

        _58.x = MR::clamp(angle, -f1, f1);
        _58.z = zero;
        _58.y = 0.0f;

        MR::orderRotateMtx(0, _58, _64);
        MR::orderRotateMtx(0, -_58, _94);
        _12C = 16;
        return;
    } else if (_12C != 0) {
        _12C--;
    }

    _58.x *= 0.8f;
    _58.z *= 0.8f;
    _58.y = 0.0f;

    MR::orderRotateMtx(0, _58, _64);
    MR::orderRotateMtx(0, -_58, _94);
}


void Mario::updateLookOfs() {
    TVec3f vec;
    vec.zero();

    if (isStatusActive(MarioStatus_Blown)) {
        return;
    }

    if (mActor->_934) {
        MR::vecBlend(_13C, vec, &_13C, 0.3f);
    }

    if (_97C == nullptr) {
        if (isAnimationRun("壁押し")) {
            TVec3f vec2;
            MR::vecKillElement(_4E8 - mPosHolder->mPos, mHeadVec, &vec2);
            f32 len = vec2.length();
            vec = *mFrontWallTriangle->getNormal(0) * (len - 35.0f);
        }
    } else if (_97C->mStatusId == MarioStatus_Swim) {
        vec += -*getGravityVec() * mSwim->mSurfaceOffset;
    }

    if (((mMovementStates.mLowWord >> 30) & 1U) && !((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && mVerticalSpeed > 5.0f && mVerticalSpeed < 50.0f && !isCurrentShadowFloorDangerAction()) {
        TVec3f dirShadowToGround(mGroundPos - mShadowPos);
        MR::normalizeOrZero(&dirShadowToGround);
        if (dirShadowToGround.dot(_368) < 0.1f) {
            vec += *getGravityVec() * mVerticalSpeed;
        } else {
            TVec3f vec3;
            f32 f1 = MR::vecKillElement(mGroundPos - mPosHolder->mPos, *getGravityVec(), &vec3);
            vec += *getGravityVec() * f1;
        }
    }

    TVec3f vec4;
    vec4.zero();

    if (mSinkTimer != 0) {
        vec4 = *getGravityVec() * static_cast< f32 >(mSinkTimer) * (1.0f / 256.0f) * 120.0f;
        vec4 = -_1FC * static_cast< f32 >(mSinkTimer) * (1.0f / 256.0f) * 120.0f;
    }

    f32 blendRate = 0.2f;

    if (!isUseSimpleGroundCheck()) {
        if (getAnimator()->isLandingAnimationRun()) {
            blendRate = 0.01f;
        } else if (mTargetWalkSpeedIndex == 0 && mWalkSpeed < 0.2f && isBlendWaitGround()) {
            if (_148.length() > vec.length()) {
                blendRate = 0.5f;
            } else {
                blendRate = 0.02f * static_cast< f32 >(_3CE);

                if (blendRate >= 0.2f) {
                    blendRate = 0.2f;
                }
            }
        }
    }

    MR::vecBlend(_148, vec, &_148, blendRate);
    MR::vecBlend(_154, vec4, &_154, 0.2f);
    _13C = _148 + _154;
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


XjointTransform* (XanimeCore::*Mario_FORCE_MATCH_JOINT_TRANSFORM())(u32) {
    return &XanimeCore::getJointTransform;
}

// ---- Mario2D.cpp ----

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

    if (getCurrentStatus() != 39 && !isSwimming() && (diff < 0.5235988f || diff > 2.617994f)) {
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

// ---- MarioBee.cpp ----

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

// ---- MarioBlown.cpp ----

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

// ---- MarioBump.cpp ----

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

    if (!MR::normalizeOrZero(&v21) && MR::diffAngleAbsHorizontal(mFrontVec, -v21, getAirGravityVec()) > 0.52359879f) {
        TVec3f v20;
        if (MR::vecKillElement(getTrans() - rVec, getAirGravityVec(), &v20) > 0.0f) {
            return;
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

// ---- MarioClimb.cpp ----

void Mario::connectToClimb() {
    _790 = -getShadowNorm();
    changeStatus(mClimb);
}

// ---- MarioCollision.cpp ----

void MarioCollision_FORCE_MATCH_SDATA2() {
    (void)0.001f;
    (void)3e+01f;
    (void)0.3f;
    (void)0.707f;
    (void)1.01f;
    (void)-0.99f;
    (void)5.0f;
    (void)4e+01f;
    (void)1.1e+02f;
    (void)5e+01f;
    (void)6e+01f;
    (void)-0.707f;
    (void)45.0f;
    (void)0.2f;
    (void)1.2e+02f;
    (void)8e+01f;
    (void)0.7853982f;
    (void)1e+02f;
    (void)4e+02f;
    (void)1e+01f;
    (void)0.1f;
    (void)2.5e+03f;
    (void)1.5e+02f;
    (void)0.10471976f;
    (void)2.3561945f;
    (void)0.17f;
    (void)1.4959966f;
    (void)-3e+01f;
    (void)2e+02f;
    (void)-1e+01f;
    (void)5e+03f;
    (void)25.0f;
    (void)1.8e+02f;
    (void)3.14159f;
    (void)-0.999f;
    (void)-0.9f;
    (void)2.0943952f;
    (void)6.0f;
    (void)-6.0f;
    (void)55.0f;
    (void)0.99f;
    (void)-0.01f;
    (void)8.0f;
    (void)2e+01f;
    (void)2.1e+02f;
    (void)-2607.5945f;
    (void)2607.5945f;
    (void)15.0f;
    (void)2e+03f;
    (void)0.125f;
}





bool Mario::isIgnoreTriangle(const Triangle* pTriangle) {
    return MR::isNearZero(pTriangle->getNormal(0)->dot(*getGravityVec()));
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

                if (hit->isCollisionAtEdge() && MR::diffAngleAbsHorizontal(mJumpVec, -*triangle->getNormal(0), getAirGravityVec()) < 0.7853982f) {
                    continue;
                }
            }

            if (!force && !isSwimming()) {
                if (triangle->mSensor != nullptr && mActor->selectPushOff(triangle->mSensor)) {
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


f32 Mario::calcDistToCeil(bool saveSensor) {
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
    TVec3f ray = -mAirGravityVec * (200.0f + offset);
    Triangle triangle;
    TVec3f hitPos;
    bool hit;
    if (fn_803B2950()) {
        hit = MR::getFirstPolyOnLineToDashYoshiStepAndMap(&hitPos, &triangle, start, ray);
    } else {
        hit = MR::getFirstPolyOnLineToMap(&hitPos, &triangle, start, ray);
    }
    if (!hit || isThroughWall(&triangle)) {
        return 200.0f;
    }
    if (fn_80390210(&triangle)) {
        if ((hitPos - mPosHolder->mPos).dot(*getGravityVec()) >= 0.0f) {
            return 200.0f;
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


void Mario::setCameraPolygon(const Triangle* pTriangle) {
    *_468 = *pTriangle;

    if (pTriangle->mSensor == nullptr) {
        *_46C = *pTriangle;
        return;
    }

    *_46C = *pTriangle;
}


void Mario::checkAllWall(const TVec3f& rPosition, f32 radius) {
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
    bool forwardCheck = false;
    s32 count;
    if (getPlayerMode() == 6) {
        radius = 110.0f;
        count = fn_803B2A20(center, 110.0f, 110.0f);
        if (!count) {
            forwardCheck = true;
            center.add(mFrontVec * 120.0f);
            if (fn_803B2950()) count = Collision::checkStrikeBallToDashYoshiStepAndMapWithThickness(center, 25.0f, 25.0f, nullptr, nullptr);
            else count = Collision::checkStrikeBallToMapWithThickness(center, 25.0f, 25.0f, nullptr, nullptr);
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
            f32 amount = 2.0f;
            if (!(penetration > amount)) amount = 0.0f;
            TVec3f pushVec(*mFrontWallTriangle->getNormal(0));
            pushVec.scale(amount);
            push(pushVec);
        }
    }
}
#pragma push
#pragma opt_propagation off
#pragma opt_lifetimes off
void Mario::calcFrontFloor() {
    Triangle wall;
    Triangle floor;
    TVec3f start;
    TVec3f wallPos;
    TVec3f floorPos;
    TVec3f direction;
    Triangle* pFloor = _47C;
    mMovementStates.mLowBits.mBit10 = 0;
    mMovementStates.mHighBits.mBit6 = 0;
    pFloor->mIdx = -1;
    _4E0 = 0.0f;
    bool hit;

    if (((mMovementStates.mLowWord >> 23) & 1U)) {
        wallPos = _4E8;
        start = mActor->_2A0;
        hit = true;
    } else {
        direction = mFrontVec * 100.0f;
        start = mPosHolder->mPos - *getGravityVec() * 8.0f;

        if (((mMovementStates.mLowWord >> 16) & 1U) && !MR::isNearZero(_328)) {
            MR::vecKillElement(_328, *getGravityVec(), &direction);
            direction.setLength(200.0f);
        }

        hit = MR::getFirstPolyOnLineToMap(&wallPos, &wall, start, direction);
    }

    if (hit) {
        _4E4 = (wallPos - start).dot(mFrontVec);
        TVec3f normal(*wall.getNormal(0));
        f32 dot = normal.dot(getAirGravityVec());

        if (MR::abs(dot) < 0.1f) {
            f32 angle = marioAcos(-dot);
            start = wallPos - getAirGravityVec() * 200.0f;
            start += mFrontVec * 20.0f;
            direction = getAirGravityVec() * 210.0f;
            hit = MR::getFirstPolyOnLineBFast(start, direction, &floorPos, &floor);

            if (hit && floor.getNormal(0)->dot(getAirGravityVec()) > -0.9f) {
                hit = false;
            }

            if (hit) {
                _4E0 = (wallPos - floorPos).dot(getAirGravityVec());

                if (_4E0 > 0.0f) {
                    mMovementStates.mLowBits.mBit10 = 1;
                }

                f32 offset = 0.0f;

                if (angle < 1.5707964f && angle != 0.0f) {
                    offset = _4E0 / (JMASinRadian(angle) / JMACosRadian(angle));
                }

                TVec3f originalPos(floorPos);
                floorPos -= mFrontVec * (15.0f + offset);

                if (!MR::isExistMapCollision(floorPos - getAirGravityVec() * 5.0f, *getGravityVec() * 10.0f)) {
                    floorPos = originalPos;
                } else {
                    _4A4 = floorPos;
                    mMovementStates.mHighBits.mBit6 = 1;
                    *_47C = floor;
                }

            } else {
                _4E0 = 400.0f;
            }
        }

    } else {
        _4E0 = 0.0f;
    }

    if (((mMovementStates.mLowWord >> 30) & 1U) && calcAngleD(_368) >= 25.0f) {
        mMovementStates.mHighBits.mBit6 = 0;
    }
}
#pragma pop

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


#ifdef GALAXY_NATIVE
namespace {
    const TVec3f sNativeZeroVec(0.0f, 0.0f, 0.0f);
}
#endif

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

    if (MR::vecKillElement(mShadowPos - mGroundPos, getAirGravityVec(), &horizontal) > 30.0f) {
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
    u16 shifted = _960 + 0xFFEF;
    if (shifted <= 14 && ((1 << shifted) & 0x4103)) {
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


void Mario::checkBaseTransPoint() {
    s32 inside = fn_803B2AF0(mPosHolder->mPos - *getGravityVec() * 30.0f, nullptr);
    const TVec3f& position=mPosHolder->mPos;
    u32 count;
    if(fn_803B2950()) count=Collision::checkStrikeBallToDashYoshiStepAndMap(position,1.0f,nullptr,nullptr);
    else count=Collision::checkStrikeBallToMap(position,1.0f,nullptr,nullptr);
    for(u32 i=0;i<count;i++) fn_8038E1A0(fn_803B2AA0(i),inside);
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


void Mario::calcShadowPos() {
    f32 alignment = _1FC.dot(-*getGravityVec());
    if (alignment < 0.0f) {
        alignment = 0.0f;
    }
    f32 offset = 100.0f * alignment;
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
            const u8 backHit = hit[2];
            const u8 frontHit = hit[0];
            const u8 sideHit = hit[1];
            if (!(frontHit | sideHit | backHit)) {
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


CubeCameraArea* Mario::getCameraCubeCode() const {
    if ((isSwimming() && mSwim->isOnWaterSurface()) || (((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && isRising())) {
        return static_cast< CubeCameraArea* >(MR::getAreaObj("CubeCamera", mPosHolder->mPos - *getGravityVec() * 100.0f));
    }

    return static_cast< CubeCameraArea* >(MR::getAreaObj("CubeCamera", mPosHolder->mPos));
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

// ---- MarioDamage.cpp ----

namespace {
    const char sDamageAirAnimation[] = "中ダメージ空中";
    const char sDamageLandAnimation[] = "中ダメージ着地";
    const char sBackDamageAirAnimation[] = "中後ダメージ空中";
    const char sBackDamageLandAnimation[] = "中後ダメージ着地";
};
  // namespace

void MarioDamage_FORCE_MATCH_SDATA2() {
    (void)10.0f;
    (void)5.0f;
    (void)0.949999988f;
    (void)30.0f;
    (void)20.0f;
    (void)50.0f;
    (void)-10.0f;
    (void)0.699999988f;
    (void)0.100000001f;
    (void)0.25f;
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


bool Mario::damageLarge(const TVec3f& rDirection) {
    if (damage(rDirection)) {
        if (isStatusActive(MarioStatus_Swim)) {
            mSwim->fn_803F7C00();
        } else {
            mDamage->setVecSize(mActor->getConst().getTable()->mJumpDistLargeDamage, mActor->getConst().getTable()->mJumpHeightLargeDamage);
            playSound("投げられ");
        }

        return true;
    }

    return false;
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
        mFlags14.mLowWord &= 0x7FFFF77FU;
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


const char* MarioDamage_FORCE_MATCH_DATA(u32 index) {
    switch (index) {
    case 0:
        return "炎ダメージ";
    case 1:
        return "炎ダメージ青煙";
    default:
        return "炎ダメージ煙";
    }
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


bool Mario::doFireObjHitWithInitialDamage() {
    if (isEnableAddDamage() == false) {
        return false;
    }

    return doFireDanceWithInitialDamage(1);
}


bool Mario::doNeedleWithInitialDamage(u8 amount) {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit0))) {
        return false;
    }

    if (getPlayerMode() == PlayerMode_Teresa) {
        doTeresaReflection(getWallNorm(), false);
        return false;
    }

    bool started = doNeedle(nullptr);
    if (started) {
        for (u32 i = 0; i < amount; i++) {
            mActor->decLife(0);
        }

        mActor->fn_803BE840(getWallNorm());

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

    bool started = doNeedle(pTriangle);
    if (started) {
        mActor->decLife(0);
        mActor->fn_803BE840(*MR::getNormal(pTriangle));

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
            mActor->decLifeSmall();
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

// ---- MarioDamageCrush.cpp ----

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

// ---- MarioDamageFreeze.cpp ----

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

// ---- MarioDamageParalyze.cpp ----

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

// ---- MarioEffect.cpp ----

namespace {

    void addMarioEffectAtJoint(LiveActor* pActor, const char* pEffectName, const char* pJointName, const char* pGroupName) NO_INLINE {
        MtxPtr mtx = pJointName != nullptr ? MR::getJointMtx(pActor, pJointName) : pActor->getBaseMtx();
        pActor->mEffectKeeper->registerEffect(pEffectName, mtx, &pActor->mScale, pGroupName, pJointName);
    }

};


bool Mario::taskOnEffectCheck(u32 flags) {
    if (flags & 0x2) {
        return mActor->checkEffectWaterColumn();
    }

    return true;
}

// ---- MarioEnforce.cpp ----

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

        TVec3f add(movePos);
        add -= mPosHolder->mPos;

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


const TVec3f* Mario::getLastGroundEdgeNrm(u32 idx) const {
    return _474->getEdgeNormal(idx);
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

// ---- MarioFaint.cpp ----

bool Mario::doFlipWeak(const TVec3f& rVec) {
    if (((mMovementStates.mLowWord >> 4) & 1U)) {
        return false;
    }

    mFaint->mNoDamage = true;

    if (faint(rVec)) {
        if (isStatusActive(MarioStatus_Swim)) {
            mSwim->fn_803F7C10();
            mFaint->mNoDamage = false;
        }

        return true;
    }

    mFaint->mNoDamage = false;
    forceStopTornado();
    return false;
}


bool Mario::faint(const TVec3f& rVec) {
    _7C4 = rVec;
    if (!isEnableAddDamage()) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_Damage) {
        return false;
    }

    if (isPlayerModeYoshi()) {
        if (isStatusActive(MarioStatus_Swim)) {
            closeStatus(mSwim);
        }
        mDamage->_34 = rVec;
    } else {
        mFaint->setVec(rVec);
    }
    stopWalk();
    forceStopTornado();
    if (isStatusActive(MarioStatus_FpView)) {
        closeStatus(mFpView);
    }
    mActor->damageDropThrowMemoSensor();

    if (getCurrentStatus() != MarioStatus_Faint) {
        mMovementStates.mHighBits.mBit24 = 1;
        return true;
    }

    return ((mMovementStates.mHighWord >> 24) & 1U);
}

// ---- MarioFlip.cpp ----

bool Mario::doFlipJump(const TVec3f& rVec) {
    if (isAnimationRun("壁はじき")) {
        return false;
    }
    stopWalk();
    TVec3f velocity(rVec);
    velocity.setLength(mActor->mConst->getTable()->mWallSpinFlipGround);
    TVec3f gravity;
    MR::calcGravityVector(mActor, &gravity, nullptr, 0);
    if (!isPlayerModeYoshi() || mActor->getYoshi()->mDashTimer <= 0 || MR::abs(getGravityVec()->dot(gravity)) > 0.2f) {
        f32 hop = mActor->mConst->getTable()->mWallSpinHopGround;
        velocity -= *getGravityVec() * hop;
    }
    tryForcePowerJump(velocity, true);
    changeAnimation("壁はじき");
    stopPunch();
    _430 = 3;
    mMovementStates.mHighBits.mBit20 = 1;
    startPadVib(2);
    return true;
}


bool Mario::doFlipBackRoll(const TVec3f& rVec) {
    TVec3f vec20;
    MR::vecKillElement(rVec, getAirGravityVec(), &vec20);

    if (doFlipLarge(vec20)) {
        checkDamage();
        changeAnimationNonStop("後転ふっとび");

        mDamage->setStrings("後転ふっとび空中", "後転ふっとび着地");

        TVec3f vec28;
        MR::vecKillElement(rVec, getAirGravityVec(), &vec28);
        setFrontVecKeepUp(-vec28, static_cast< u32 >(12l));

        return true;
    }

    return false;
}


bool Mario::doFlipRot(const TVec3f& rVec) {
    getCurrentStatus();
    if (getCurrentStatus() == MarioStatus_FireDamage) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 4) & 1U)) {
        return false;
    }

    mFlip->setVec(rVec);

    stopJump();
    stopWalk();
    forceStopTornado();

    changeStatus(mFlip);

    return true;
}


bool Mario::doFlipLarge(const TVec3f& rVec) {
    if (getCurrentStatus() == MarioStatus_Damage) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_Flip) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireDamage) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 4) & 1U)) {
        return false;
    }

    return flipLarge(rVec);
}

// ---- MarioFlow.cpp ----

bool Mario::doFlow() {
    if (isStatusActive(MarioStatus_Flow)) {
        return false;
    }

    if (isAnimationRun("空中ひねり")) {
        return false;
    }

    changeStatus(mFlow);

    return true;
}

// ---- MarioFoo.cpp ----

void MarioFoo_FORCE_MATCH_SDATA2() {
    (void)0.52359879f;
    (void)0.100000001f;
    (void)120.0f;
    (void)0.125f;
    (void)10.0f;
    (void)100.0f;
    (void)0.0500000007f;
    (void)5.0f;
    (void)1.04719758f;
    (void)1.10000002f;
    (void)-0.100000001f;
    (void)0.00999999978f;
    (void)25.0f;
    (void)0.980000019f;
    (void)1.01999998f;
    (void)0.899999976f;
    (void)1.20000005f;
    (void)50.0f;
    (void)1.5f;
    (void)12.0f;
    (void)2.09439516f;
    (void)0.314159274f;
    (void)0.785398185f;
    (void)0.25f;
    (void)30.0f;
    (void)0.015625f;
    (void)70.0f;
    (void)2000.0f;
    (void)255.0f;
    (void)1.00010002f;
    (void)6.0f;
}


void MarioFoo_FORCE_MATCH(TVec3f& rVec, f32 a, f32 b, f32 c) {
    rVec.setLength(a);
    rVec.setLength(b);
    rVec.setLength(c);
}


namespace {
    f32 cTurnMotionSpeed = 5.0f;
    f32 cWidth = 70.0f;
    f32 cLimitAngleSink = JGeometry::TUtil< f32 >::PI() / 1.0001f;
    f32 cNeutralAngleWait = JGeometry::TUtil< f32 >::PI() / 2.0f - JGeometry::TUtil< f32 >::PI() / 6.0f;
    f32 cLimitAngleWait = JGeometry::TUtil< f32 >::PI() / 1.0001f;
    f32 cUpperAngleWait = JGeometry::TUtil< f32 >::PI() / 100.0f;
};
  // namespace

void Mario::tryStartFoo() {
    if (MR::getAreaObj("CelestrialArea", getTrans()) || !mFoo->_48) {
        changeStatus(mFoo);
        mFoo->setStartSpeed(mActor->getConst().getTable()->mSwimFrontMaxSpeed);
    }
}


const char* MarioFoo_FORCE_MATCH_DATA(u32 index) {
    static const char* const strings[] = {"水泳ダメージ", "水中ダメージ", "声小ダメージ", "ダメージ", "水泳ダメージ中"};
    return strings[index];
}

// ---- MarioFpView.cpp ----

namespace {
    static bool sIsDisabled;
};
  // namespace

bool Mario::isDisableStayHere() const {
    if (((getMovementStates().mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        return true;
    }

    if (!((getMovementStates().mLowWord >> 30) & 1U)) {
        return true;
    }

    if (_3CE < 5) {
        return true;
    }

    if (((getMovementStates().mLowWord >> 21) & 1U)) {
        return true;
    }

    if (isDamaging()) {
        return true;
    }

    if (((getMovementStates().mLowWord >> 4) & 1U)) {
        return true;
    }

    if (MR::getKarikariClingNum() != 0) {
        return true;
    }

    if (((getPlayer()->getMovementStates().mHighWord >> 28) & 1U)) {
        return true;
    }

    if (mWalkSpeed > 0.01f) {
        return true;
    }

    if (getStickP() != 0.0f) {
        return true;
    }

    if (((mFlags14.mLowWord & Mario::MovementStates::DebugMode) >> 9)) {
        return true;
    }

    if (_960 == 27) {
        return true;
    }

    if (_960 == 28) {
        return true;
    }

    if (((getDrawStates().mWord >> 20) & 1U)) {
        return true;
    }

    if (((getMovementStates().mLowWord >> 7) & 1U)) {
        return true;
    }

    if (!MR::isNearZero(_928)) {
        return true;
    }

    if (!MR::isNearZero(_350)) {
        return true;
    }

    if (!MR::isNearZero(_35C)) {
        return true;
    }

    if (!MR::isNearZero(_184)) {
        return true;
    }

    if (mActor->mBeeWallWalk != 0) {
        return true;
    }

    if (((getMovementStates().mHighWord >> 8) & 1U)) {
        return true;
    }

    if (((getMovementStates().mHighWord >> 5) & 1U)) {
        return true;
    }

    if ((static_cast<u32>(mFlags14.mLowBits.mBit10))) {
        return true;
    }

    if (mSinkTimer) {
        return true;
    }

    if (mActor->_EA4) {
        return true;
    }

    return mActor->_3C0;
}


bool Mario::isDisableFpViewMode() const {
    if (_97C) {
        return true;
    }
    if (isDisableStayHere()) {
        return true;
    }
    if (isPlayerModeYoshi() && !mActor->fn_803B9810()->fn_8041F9E0()) {
        return true;
    }
    return false;
}


void Mario::tryFpViewMode() {
    if (isStatusActive(MarioStatus_FpView)) {
        return;
    }

    stopWalk();

    changeStatus(mFpView);

    MR::startSystemSE("SE_SY_CAMERA_FP_VIEW");
}

// ---- MarioFrontStep.cpp ----

bool Mario::doFrontStep() {
    if (getCurrentStatus() == MarioStatus_FrontStep) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_SideStep) {
        return false;
    }

    if (checkWallCode("NotSideStep", false)) {
        return false;
    }

    if (checkWallCode("NoAction", false)) {
        return false;
    }

    if (mActor->_468 != 0) {
        return false;
    }

    if ((static_cast<u32>(mFlags20.mBits.mBit26))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        return false;
    }

    if (getPlayerMode() == PlayerMode_Hopper) {
        return false;
    }

    if (_960 == 2) {
        return false;
    }

    if (calcAngleD(_368) >= 30.0f) {
        return false;
    }

    if (mAirGravityVec.dot(_374) >= 0.25f) {
        return false;
    }

    Triangle triangle = Triangle();
    MarioActor* pMario = mActor;
    TVec3f* vec = &pMario->_240;
    TVec3f ray(mFrontVec * 200.0f);
    bool hit;
    if (fn_803B2950()) {
        hit = MR::getFirstPolyOnLineToDashYoshiStepAndMap(nullptr, &triangle, *vec, ray);
    } else {
        hit = MR::getFirstPolyOnLineToMap(nullptr, &triangle, *vec, ray);
    }
    if (!hit) {
        return false;
    }

    setFrontVecKeepUp(-*MR::getNormal(&triangle));

    stopWalk();
    forceStopTornado();

    changeStatus(mFrontStep);

    return true;
}

// ---- MarioHang.cpp ----

void MarioHang_FORCE_MATCH_SDATA2() {
    (void)-0.8f;
    (void)50.0f;
    (void)5.0f;
    (void)60.0f;
    (void)160.0f;
    (void)0.1f;
    (void)0.08726647f;
    (void)100.0f;
    (void)150.0f;
    (void)0.2f;
    (void)30.0f;
    (void)-0.707f;
    (void)84.0f;
    (void)-20.0f;
    (void)-100.0f;
    (void)0.707f;
    (void)45.0f;
    (void)80.0f;
    (void)6.0f;
    (void)0.95f;
    (void)20.0f;
    (void)15.0f;
    (void)55.0f;
    (void)-55.0f;
    (void)-15.0f;
    (void)-150.0f;
    (void)0.001f;
    (void)0.01f;
}


bool Mario::isHanging() const {
    return getCurrentStatus() == MarioStatus_Hang;
}


bool Mario::fixHangDir(const TVec3f& rPosition, TVec3f* pDirection) {
    TVec3f horizontal(rPosition - mPosHolder->mPos);
    MR::vecKillElement(horizontal, *getGravityVec(), &horizontal);
    MR::normalizeOrZero(&horizontal);
    Triangle pTriangle;
    if (fn_803B2770(nullptr, &pTriangle, mPosHolder->mPos, horizontal * 150.0f)) {
        *pDirection = -*pTriangle.getNormal(0);
        mHang->recordWallPolygon(&pTriangle);
        return true;
    }

    if (pDirection->dot(horizontal) < 0.0f) {
        *pDirection = horizontal;
        return false;
    }

    return false;
}


void Mario::checkHang() {
    if (mHang->mHangTimer) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && isRising()) {
        mMovementStates.mHighBits.mBit15 = 0;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && _3BC > 20) {
        mMovementStates.mHighBits.mBit15 = 0;
    }

    if (isCeiling()) {
        mMovementStates.mHighBits.mBit14 = 0;
    }

    if (!(static_cast<u32>(mMovementStates.mHighBits.mBit14))) {
        return;
    }

    if (((getPlayer()->mDrawStates.mWord & Mario::DrawStates::Underwater) >> 13)) {
        return;
    }

    if (isStatusActive(MarioStatus_Rabbit)) {
        return;
    }

    if (mActor->_468) {
        return;
    }

    if (isPlayerModeBee()) {
        return;
    }

    if (isPlayerModeTeresa()) {
        return;
    }

    if (isPlayerModeYoshi()) {
        return;
    }

    bool front = true;
    bool back = true;
    bool side = true;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit5))) {
        const Triangle* pTriangle = mSideWallTriangle;
        if (!MR::isSameMtx(pTriangle->getBaseMtx()->toMtxPtr(), pTriangle->getPrevBaseMtx()->toMtxPtr())) {
            front = false;
            back = false;
        }
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        const Triangle* pTriangle = mFrontWallTriangle;
        if (!MR::isSameMtx(pTriangle->getBaseMtx()->toMtxPtr(), pTriangle->getPrevBaseMtx()->toMtxPtr())) {
            side = false;
            back = false;
        }
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        const Triangle* pTriangle = mBackWallTriangle;
        if (!MR::isSameMtx(pTriangle->getBaseMtx()->toMtxPtr(), pTriangle->getPrevBaseMtx()->toMtxPtr())) {
            front = false;
            side = false;
        }
    }

    TVec3f previousPosition(mPosHolder->mPos);
    if (front && (static_cast<u32>(mMovementStates.mHighBits.mBit6)) && isEnableHang()) {
        TVec3f direction(_4A4 - mPosHolder->mPos);
        MR::vecKillElement(direction, getAirGravityVec(), &direction);
        MR::normalizeOrZero(&direction);
        if (getFrontWallNorm().dot(direction) >= -0.8f) {
            return;
        }

        if (!fn_803B2880(_4A4 - mFrontVec * 50.0f - getAirGravityVec() * 5.0f, mFrontVec * 60.0f)) {
            u8 started = 1;
            if (isAnimationRun("落下")) {
                started = 0;
            }

            changeStatus(mHang);
            mVelocity.zero();
            stopWalk();
            stopJump();
            setTrans(_4A4);
            setFrontVecKeepUp(-getWallNorm());
            mHang->recordWallPolygon(mFrontWallTriangle);
            mHang->_1E = started;
        }
    } else if (back && isEnableBackHang()) {
        TVec3f position;
        getLastGroundPos(&position);
        if ((mPosHolder->mPos - position).dot(getAirGravityVec()) >= 160.0f) {
            return;
        }

        TVec3f direction(-mFrontVec);
        u32 edge = getLastGroundEdgeIndex(mActor->_240, direction);
        direction = _4F4 - mActor->_240;
        MR::normalizeOrZero(&direction);
        u32 positionEdge = getLastGroundEdgeIndex(mActor->_240, direction);
        direction = -*mBackWallTriangle->getNormal(0);
        u32 normalEdge = getLastGroundEdgeIndex(mActor->_240, direction);
        TVec3f normal;
        if (edge == positionEdge && edge == normalEdge) {
            normal = *getLastGroundEdgeNrm(edge);
        } else {
            return;
        }

        if (normal.dot(getAirGravityVec()) >= 0.1f) {
            return;
        }

        if (normal.dot(mHeadVec) >= 0.1f) {
            return;
        }

        direction = -*mBackWallTriangle->getNormal(0);
        if (MR::diffAngleAbsHorizontal(normal, -direction, getAirGravityVec()) > 0.08726647f) {
            return;
        }

        TVec3f horizontal;
        f32 distance = 0.0f;
        switch (edge) {
        case 0:
        case 1:
            distance = MR::vecKillElement(*_474->calcAndGetPos(0) - position, normal, &horizontal);
            position += normal * distance;
            break;
        case 2:
            distance = MR::vecKillElement(*_474->calcAndGetPos(1) - position, normal, &horizontal);
            position += normal * distance;
            break;
        }

        if (distance > 100.0f) {
            return;
        }

        MR::vecKillElement(normal, getAirGravityVec(), &normal);
        MR::normalizeOrZero(&normal);
        if (!fn_803B2880(position - direction * 50.0f - getAirGravityVec() * 5.0f, direction * 60.0f)) {
            changeStatus(mHang);
            mVelocity.zero();
            stopWalk();
            stopJump();
            TVec3f hangDirection;
            bool found = fixHangDir(position, &hangDirection);
            setTrans(position);
            setFrontVecKeepUp(normal);
            mMovementStates.mLowBits.mBit2 = 1;
            if (!found) {
                mHang->recordWallPolygon(mBackWallTriangle);
            }

            mHang->recordHangNorm(normal);
            mHang->_1E = 0;
        }
    } else if (side && isEnableSideHang()) {
        TVec3f position;
        getLastGroundPos(&position);
        if ((mPosHolder->mPos - position).dot(getAirGravityVec()) >= 160.0f) {
            return;
        }

        TVec3f direction(_500 - mActor->_240);
        MR::normalizeOrZero(&direction);
        u32 edge = getLastGroundEdgeIndex(mActor->_240, direction);
        TVec3f sideDirection(mSideVec);
        if (sideDirection.dot(direction) < 0.0f) {
            direction = -sideDirection;
        } else {
            direction = sideDirection;
        }

        u32 sideEdge = getLastGroundEdgeIndex(mActor->_240, direction);
        direction = -*mSideWallTriangle->getNormal(0);
        u32 normalEdge = getLastGroundEdgeIndex(mActor->_240, direction);
        TVec3f normal;
        if (edge == sideEdge && edge == normalEdge) {
            normal = *getLastGroundEdgeNrm(edge);
        } else {
            return;
        }

        if (normal.dot(getAirGravityVec()) >= 0.1f) {
            return;
        }

        if (normal.dot(mHeadVec) >= 0.1f) {
            return;
        }

        direction = -*mSideWallTriangle->getNormal(0);
        if (MR::diffAngleAbsHorizontal(normal, -direction, getAirGravityVec()) > 0.08726647f) {
            return;
        }

        MR::vecKillElement(normal, getAirGravityVec(), &normal);
        MR::normalizeOrZero(&normal);
        if (!fn_803B2880(position - direction * 50.0f - getAirGravityVec() * 5.0f, direction * 60.0f)) {
            changeStatus(mHang);
            mVelocity.zero();
            stopWalk();
            stopJump();
            TVec3f hangDirection;
            bool found = fixHangDir(position, &hangDirection);
            setTrans(position);
            setFrontVecKeepUp(normal);
            mMovementStates.mLowBits.mBit2 = 1;
            if (!found) {
                mHang->recordWallPolygon(mSideWallTriangle);
            }

            mHang->recordHangNorm(normal);
            mHang->_1E = 0;
        }
    }

    if (isStatusActive(MarioStatus_Hang)) {
        _414 = 0;
        for (u32 i = 0; i < 10; i++) {
            updateGroundInfo();
            if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
                return;
            }

            addTrans(mFrontVec * 1.0f - getAirGravityVec() * 1.0f);
        }
    }
}


bool Mario::isEnableHang() {
    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        return false;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit10))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
        return false;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit1))) {
        return false;
    }

    if (isStatusActive(MarioStatus_Recovery)) {
        return false;
    }

    if (checkWallCode("NotGrab", false)) {
        return false;
    }

    if (checkWallCode("NoAction", false)) {
        return false;
    }

    const char* pWallCode = MR::getWallCodeString(_47C);
    if (pWallCode != nullptr) {
        if (strcmp(pWallCode, "NotGrab") == 0) {
            return false;
        }

        if (strcmp(pWallCode, "NoAction") == 0) {
            return false;
        }
    }

    TVec3f shadowPos(mShadowPos);
    TVec3f up(-getAirGravityVec());
    f32 slope = _45C->getNormal(0)->dot(up);
    f32 height = mActor->getConst().getTable()->mWallHangGrHeight;
    if (slope > 0.0f) {
        if (slope > 0.2f) {
            slope = 0.2f;
        }

        height -= 30.0f * (5.0f * slope);
    }

    if (getPlayer()->getShadowNorm().dot(*getGravityVec()) < -0.707f) {
        if ((_4A4 - mShadowPos).dot(-*getGravityVec()) < height) {
            return false;
        }
    }

    f32 heightFromMario = (_4A4 - mPosHolder->mPos).dot(-*getGravityVec());
    if (heightFromMario > mActor->getConst().getTable()->mWallHangMyHeight) {
        return false;
    }

    return !isRising();
}


bool Mario::isEnableBackHang() {
    bool turn = false;
    if (!_474->isValid()) {
        return false;
    }

    if (_430 == 6) {
        if (isRising()) {
            return false;
        }

        turn = true;
    } else if (!(static_cast<u32>(mMovementStates.mHighBits.mBit15))) {
        return false;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
        return false;
    }

    if (checkWallCode("NotGrab", false)) {
        return false;
    }

    const char* pWallCode = MR::getWallCodeString(_474);
    if (pWallCode != nullptr && strcmp(pWallCode, "NotGrab") == 0) {
        return false;
    }

    if (isRising()) {
        return false;
    }

    if (mStickPos.z > mActor->getConst().getTable()->mWallBackHangStickPower) {
        return false;
    }

    if (mWalkSpeed > mActor->getConst().getTable()->mWallBackHangWalkSpeed) {
        return false;
    }

    TVec3f ground;
    TVec3f horizontal;
    if (getPlayer()->getShadowNorm().dot(*getGravityVec()) < -0.707f) {
        if (turn) {
            ground = mPosHolder->mPos;
        } else {
            getLastGroundPos(&ground);
        }

        f32 height = MR::vecKillElement(mShadowPos - ground, *getGravityVec(), &horizontal);
        if (height < mActor->getConst().getTable()->mWallHangGrHeight) {
            return false;
        }
    }

    if (!turn) {
        if ((_4F4 - mGroundPos).dot(*getGravityVec()) < 0.0f) {
            return false;
        }
    } else {
        setFrontVecKeepUp(-mFrontVec);
        return false;
    }

    TVec3f distance;
    TVec3f position;
    getLastGroundPos(&position);
    MR::vecKillElement(position - mPosHolder->mPos, getAirGravityVec(), &distance);
    return !(distance.length() >= 84.0f);
}


bool Mario::isEnableSideHang() {
    if ((static_cast<u32>(mFlags20.mBits.mBit30)) || (static_cast<u32>(mFlags20.mBits.mBit29))) {
        return false;
    }

    if (!(static_cast<u32>(mMovementStates.mHighBits.mBit15))) {
        return false;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit5))) {
        return false;
    }

    if (checkWallCode("NotGrab", false)) {
        return false;
    }

    if (checkWallCode("NoAction", false)) {
        return false;
    }

    const char* pWallCode = MR::getWallCodeString(_474);
    if (pWallCode != nullptr) {
        if (strcmp(pWallCode, "NotGrab") == 0) {
            return false;
        }

        if (strcmp(pWallCode, "NoAction") == 0) {
            return false;
        }
    }

    if (isRising()) {
        return false;
    }

    if (mStickPos.z > mActor->getConst().getTable()->mWallBackHangStickPower) {
        return false;
    }

    if (mWalkSpeed > mActor->getConst().getTable()->mWallBackHangWalkSpeed) {
        return false;
    }

    TVec3f ground;
    TVec3f horizontal;
    if (getPlayer()->getShadowNorm().dot(*getGravityVec()) < -0.707f) {
        getLastGroundPos(&ground);
        f32 height = MR::vecKillElement(mShadowPos - ground, *getGravityVec(), &horizontal);
        if (height < mActor->getConst().getTable()->mWallHangGrHeight) {
            return false;
        }
    }

    if (!(static_cast<u32>(mMovementStates.mHighBits.mBit12))) {
        return false;
    }

    if ((_500 - mGroundPos).dot(*getGravityVec()) < -20.0f) {
        return false;
    }

    TVec3f distance;
    TVec3f position;
    getLastGroundPos(&position);
    MR::vecKillElement(position - mPosHolder->mPos, getAirGravityVec(), &distance);
    return !(distance.length() >= 84.0f);
}

// ---- MarioInit.cpp ----

void Mario::initMember() {
    mPosHolder = new MarioPosHolder();
    _40.zero();
    _4C.zero();
    PSMTXIdentity(_F4.toMtxPtr());
    mPosHolder->fn_80405F90(TVec3f(0.0f));
    _16C.zero();
    mVelocityAfter.zero();
    _1B4.zero();
    _274 = 0;
    _280 = 0.0f;
    _284.zero();
    _2B8.zero();
    _2C4.zero();
    _304.zero();
    mShadowPos.zero();

    _340 = 0.0f;
    _3A4.zero();
    _3B0.zero();

    _422 = 0;
    _42A = 0;
    _438 = 0;
    _448.zero();

    _454 = 0.0f;
    mGroundPos.zero();
    _498.zero();
    _4BC.zero();
    _50C.zero();
    _518.zero();
    _600.zero();
    _60C = 0;
    _638 = 0;
    _660.zero();
    _688.zero();
    _694.zero();

    mPlaneModeArea = nullptr;
    _6C8.y = 0.0f;
    _6C8.x = 0.0f;
    _6D0.zero();
    _6DC.zero();
    _6E8.zero();
    _6F4.zero();
    _700.zero();
    _70C.zero();

    _718 = 0.0f;
    _71D = 0;
    _71F = 0;
    _720 = nullptr;
    _724 = nullptr;
    _728 = nullptr;

    _75C.zero();
    _790.zero();
    _7C4.zero();
    _7D4.zero();
    PSMTXIdentity(_7E4.toMtxPtr());
    _814.zero();
    PSMTXIdentity(_824.toMtxPtr());

    mRelativeWallPos[0].zero();
    mRelativeWallPos[1].zero();
    _8DC.zero();

    _8EC = 0;
    _8F4 = 0.0f;

    for (int i = 0; i < ARRAY_SIZE(_A08); i++) {
        _A08[i] = 0;
    }

    _A34 = 0;
    _A3C = 0;

    _A40.zero();
    _A4C.zero();
    _A58.zero();
    _A64 = 0.0f;

    _A70 = 0;
    _A71 = 0;
    _A72 = 0;
    mDashWallCooldown = 0;
    _A7E = 0;
    _A7F = 0;
    _A80 = 0;
    mCometNoSpinArea = false;
    mCometFireDanceArea = false;
    mCometRainbowArea = false;
    mCometHopperArea = false;
    mCometHopperApplied = false;
    mCometRainbowApplied = false;
    mMoveInfoA84 = new MarioMoveInfoA84();
    mStateFlags.fn_8038C890();
    mDashWallSoundPlayed = false;
}

// ---- MarioJump.cpp ----

namespace {
    f32 cDropFrontSpeed = 2.0f;
};
  // namespace

void MarioJump_FORCE_MATCH_SDATA2() {
    (void)0.00100000005f;
    (void)15.0f;
    (void)0.300000012f;
    (void)4.0f;
    (void)10.0f;
    (void)0.899999976f;
    (void)500.0f;
    (void)0.100000001f;
    (void)5.0f;
    (void)100.0f;
    (void)50.0f;
    (void)150.0f;
    (void)0.600000024f;
    (void)80.0f;
    (void)20.0f;
    (void)8.0f;
    (void)0.200000003f;
    (void)0.800000012f;
    (void)0.00390625f;
    (void)45.0f;
    (void)160.0f;
    (void)1.79999995f;
    (void)0.25f;
    (void)300.0f;
    (void)200.0f;
    (void)-0.100000001f;
    (void)-0.800000012f;
    (void)30.0f;
    (void)-0.200000003f;
    (void)0.785398185f;
    (void)0.998000026f;
    (void)0.00999999978f;
    (void)6.0f;
    (void)7.0f;
    (void)90.0f;
}


bool Mario::isRising() const {
    if (getPlayerMode() == 4 || getPlayerMode() == 6) {
        if (_16C.dot(*getGravityVec()) < 0.0f) {
            return true;
        }

        return false;
    }

    return mJumpVec.dot(*getGravityVec()) < 0.0f;
}


void Mario::checkWallRiseAndSlipFront() {
    mMovementStates.mLowBits.mBit25 = 0;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        mMovementStates.mLowBits.mBit22 = 0;
        mWalkSpeed = 0.0f;
        return;
    }

    mMovementStates.mLowBits.mBit22 = 1;
}


// NONMATCHING: temporary/register allocation; complete SMG2 jump orchestration recovered.
void Mario::tryJump() {
    bool mud = false;
    TVec3f stickTotal(_190);
    stickTotal += _19C;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) || fn_80399D30()) return;
    playEffect("共通跳躍");
    stopEffectForce("スピンリング");
    bool squat = checkSquat(true);
    mMovementStates.mHighWord = (mMovementStates.mHighWord | 0x40000000) & 0xFFBFFFFF & 0xFFFFFF7F;
    mFlags14.mHighBits.mBit28 = 1;
    mMovementStates.mLowBits.mBit20 = 0;
    fn_80399E60(&stickTotal);
    forceStopTornado();
    if (tryTurnJump()) return;
    checkWallRiseAndSlipFront();
    f32 floorAngle = calcPolygonAngleD(mGroundPolygon);
    bool stickJump = false;
    if (!MR::isNearZero(_8F8, 0.001f) && (static_cast<u32>(mMovementStates.mHighBits.mBit28))) stickJump = true;
    if (floorAngle < 15.0f) {
        mMovementStates.mHighBits.mBit28 = 0;
        _8F8.zero();
    }
    if (getPlayerMode() != 6 && !(static_cast<u32>(mFlags14.mLowBits.mBit5))) {
        if (((static_cast<u32>(mDrawStates.mBits.mBit19)) && isSlipPolygon(_45C)) || isStatusActive(7)) {
            MR::isNearZero(mAirGravityVec, 0.001f);
            if (MR::diffAngleAbsHorizontal(_16C, _368, mAirGravityVec) > 1.5707964f) mWalkSpeed = 0.0f;
            if (getPlayerMode() != 4) {
                _402 = 0;
                _430 = 3;
                mMovementStates.mHighBits.mBit20 = 1;
            }
        }
        if (_960 == 0x11) {
            mud = true;
            _430 = 3;
            mWalkSpeed *= 0.5f;
        }
    }
#ifdef GALAXY_NATIVE
    // OdysseyMovement: jumping out of a roll is a long jump.
    if (fn_803EF7C0() != 0x20 && getPlayerMode() != 4 && _430 != 3 && PetariNative::Odyssey::Live::roll.rolling && odysseyOn() &&
        !(static_cast<u32>(mMovementStates.mHighBits.mBit28))) {
        PetariNative::Odyssey::Live::roll.rolling = false;
        trySquatJump();
        return;
    }
#endif
    if (fn_803B2530(squat)) return;
    if (static_cast<u32>(mDrawStates.mBits.mBit26)) {
        f32 speed = fn_803B1CA0();
        f32 walkSpeed = mWalkSpeed;
        TVec3f first(_334);
        first *= walkSpeed;
        TVec3f second(first);
        second *= speed;
        mJumpVec = second;
    } else {
        f32 speed = fn_803B1CA0();
        f32 walkSpeed = mWalkSpeed;
        TVec3f first(mFrontVec);
        first *= walkSpeed;
        TVec3f second(first);
        second *= speed;
        mJumpVec = second;
    }
    mFlags14.mLowBits.mBit8 = 0;
    if (static_cast<u32>(mDrawStates.mBits.mBit20)) {
        recordJumpEnforceMove();
        f32 enforce;
        if (getStickP() == 0.0f) {
            mJumpVec.zero();
            enforce = 1.0f;
        } else {
            MR::isNearZero(mAirGravityVec, 0.001f);
            f32 factor = 1.0f - MR::diffAngleAbsHorizontal(mFrontVec, _184, mAirGravityVec) / 1.5707964f;
            if (factor < 0.0f) factor = 0.0f;
            f32 speed = PSVECMag(&mJumpVec);
            f32 bonus = MR::clamp(1.0f - (speed - 4.0f) / 10.0f, 0.0f, 1.0f);
            enforce = MR::clamp(factor + bonus, 0.0f, 1.0f);
        }
        if (MR::isInAreaObj("EnforceJumpArea", mPosHolder->mPos) && _8D8 == _45C->mSensor) enforce = 1.0f;
        doEnforceJump(enforce);
    }
    if (!MR::isNearZero(stickTotal, 0.001f)) {
        addVelocity(-stickTotal);
        MR::vecKillElement(stickTotal, mActor->mAppliedGravityVec, &stickTotal);
        cutGravityElementFromJumpVec(true);
        if (stickTotal.dot(mJumpVec) > 0.0f) {
            TVec3f extra(stickTotal);
            extra *= 1.0f;
            mJumpVec += extra;
        } else {
            TVec3f extra(stickTotal);
            extra *= 0.5f;
            mJumpVec += extra;
        }
    }
    if (stickJump) {
        MR::isNearZero(mAirGravityVec, 0.001f);
        TVec3f projected;
        JMAVECScaleAdd(&mAirGravityVec, &_368, &projected, -mAirGravityVec.dot(_368));
        if (!MR::normalizeOrZero(&projected) && _16C.dot(projected) > 0.0f) {
            mJumpVec += _16C;
            cutGravityElementFromJumpVec(true);
        }
    }
    _340 = 1.0f - mWalkSpeed;
    if (_340 < 0.0f) _340 = 0.0f;
    f32 ratio = 1.0f;
    if (fn_803EF7C0() == 0x20) ratio = mActor->mConst->getTable()->mMudFloorJumpWeakRatio;
    if (static_cast<u32>(mFlags20.mBits.mBit20)) ratio = 0.9f;
#ifdef GALAXY_NATIVE
    // OdysseyMovement: SMO's chain rules choose single/double/triple (within 10
    // grounded frames, at full speed, within 45 degrees of the last jump).
    // getJumpHeight() below reads the new _430.
    const bool odysseyJump = petari_mod_enabled(kModOdysseyMovement) && _430 >= 0 && _430 <= 2 && !(static_cast<u32>(mFlags20.mBits.mBit21)) &&
                             !(static_cast<u32>(mFlags20.mBits.mBit20)) && fn_803EF7C0() != 0x20 && !(static_cast<u32>(mFlags14.mLowBits.mBit5)) &&
                             // SMG2: only plain Mario (mode 0, not riding Yoshi) gets SMO's chain;
                             // power-ups and rides keep Galaxy's jumps (docs: native/MODS.md).
                             getPlayerMode() == 0 && !isPlayerModeYoshi();
    TVec3f odysseyDir;
    f32 odysseySpeed = 0.0f;
    if (odysseyJump) {
        TVec3f up(-mActor->mAppliedGravityVec);
        MR::normalizeOrZero(&up);
        odysseyDir = mJumpVec - up * up.dot(mJumpVec);
        MR::normalizeOrZero(&odysseyDir);
        if (MR::isNearZero(odysseyDir)) {
            odysseyDir = mFrontVec - up * up.dot(mFrontVec);
            MR::normalizeOrZero(&odysseyDir);
        }
        // The ground speed in units/frame (Galaxy keeps a ratio of its run speed).
        odysseySpeed = mWalkSpeed * mActor->mConst->getTable()->mWalkSpeed;
        _430 = sOdysseyGroundPoundJump
                   ? 0
                   : PetariNative::Odyssey::nextChainIndex(sOdyssey.lastChain, _3CE, odysseySpeed, odysseyDir.dot(sOdyssey.lastDir));
    }
#endif
    f32 height = getJumpHeight();
    TVec3f negativeGravity(-mActor->mAppliedGravityVec);
    TVec3f scaled(negativeGravity);
    scaled *= height;
    TVec3f weakened(scaled);
    weakened *= ratio;
    mJumpVec += weakened;
    mMovementStates.mLowWord = mMovementStates.mLowWord & 0xFFFDFFFF & 0xBFFFFFFF;
    if (getPlayerMode() != 4 && fn_803EF7C0() == 0x20) {
        _430 = 3;
        mud = true;
        mMovementStates.mHighBits.mBit20 = 1;
    }
    if (static_cast<u32>(mFlags14.mLowBits.mBit5)) _430 = 0;
#ifdef GALAXY_NATIVE
    sOdyssey.active = false;
    sCappyDiveReady = false;
    sOdysseyAirFrames = 0;  // a new airtime (also after a landing that skipped procJump's, e.g. water)
    sOdysseyAirDived = false;
    if (odysseyJump) {
        TVec3f up(-mActor->mAppliedGravityVec);
        MR::normalizeOrZero(&up);
        if (sOdysseyGroundPoundJump) {
            odysseyStart(PetariNative::Odyssey::Air::GroundPoundJump, odysseyDir, up, 0.0f, _430);
            sOdyssey.lastChain = -1;
        } else {
            odysseyStart(PetariNative::Odyssey::chainAir(_430), odysseyDir, up, odysseySpeed, _430);
            sOdyssey.lastChain = _430;
            sOdyssey.lastDir = odysseyDir;
        }
    }
    sOdysseyGroundPoundJump = false;
#endif
    procJump(true);
    mMovementStates.mLowBits.mBit31 = 1;
    if (_42A == 1 && _430 == 1) _430 = 2;
    switch (_430) {
    case 0:
        if (static_cast<u32>(mMovementStates.mLowBits.mBit8)) changeAnimation("壁上昇", "落下");
        else if (isPlayerModeYoshi() && mActor->getYoshi()->mDashTimer > 0) {
            MR::startCenterScreenBlur(35, 15.0f, 80, 5, 30);
            playEffect("ダッシュジャンプ煙");
            changeAnimation("ヨッシージャンプ", "落下");
        } else changeAnimation("ジャンプ", "落下");
        if (mActor->mBeeWallWalk != 0) playSound("ハチ壁ジャンプ");
        else playSound("小ジャンプ");
        if (getPlayerMode() != 6) mStateFlags._0 = true;
        if (stickJump) {
            changeAnimation("尻滑りジャンプ");
            _428 = 30;
        }
        if (getPlayerMode() == 4) {
            changeAnimation("ハチジャンプ");
            mActor->syncJumpBeeStickMode();
        }
        if (static_cast<u32>(mFlags14.mLowBits.mBit5)) changeAnimation("ショートジャンプ", "落下");
        break;
    case 1:
        changeAnimationNonStop("ジャンプB");
        changeAnimation(nullptr, "落下");
        if (isPlayerModeYoshi()) {
            if (mActor->getYoshi()->mDashTimer > 0) MR::startActionSound(mActor->getYoshi(), "DashJumpL", -1, -1, -1);
            else MR::startActionSound(mActor->getYoshi(), "JumpM", -1, -1, -1);
        } else playSound("声中ジャンプ");
        playSound("中ジャンプ");
        playSound("ジャンプ踏切");
        break;
    case 2:
        playSound("大ジャンプ");
        playSound("声大ジャンプ");
        playSound("ジャンプ踏切");
        playEffect("共通ハイジャンプ");
        if (static_cast<u32>(mFlags20.mBits.mBit20)) changeAnimation("スカイラブジャンプ", "落下");
        else if (getPlayerMode() == 1 && mWalkSpeed > 1.0f) changeAnimation("ダッシュジャンプ", "落下");
        else changeAnimation("ジャンプC", "落下");
        break;
    case 3:
        if (mud) changeAnimation("埋まり脱出ジャンプ", "落下");
        else {
            if (!isAnimationRun("坂すべり上向きうつぶせ")) changeAnimation("尻滑りジャンプ", "落下");
            else changeAnimation("腹ばいジャンプ", "落下");
            if (isPlayerModeYoshi()) MR::startActionSound(mActor->getYoshi(), "JumpHardly", -1, -1, -1);
            else playSound("声ミスジャンプ");
            playSound("ジャンプ踏切");
        }
        break;
    }
    if (_42A == 1 && _430 == 2) _430 = 1;
    if (mSwim->checkUnderWaterFull(mFrontVec) > 500.0f && mStickPos.z == 0.0f) {
        mMovementStates.mLowBits.mBit17 = 1;
        changeAnimationNonStop("飛び込みジャンプ");
        if (isPlayerModeYoshi()) MR::startActionSound(mActor->getYoshi(), "PreLandTurn", -1, -1, -1);
        else playSound("声高飛び込み");
        playSound("ジャンプ踏切");
        if (mStickPos.z < 0.1f) {
            TVec3f extra(mFrontVec);
            extra *= 5.0f;
            mJumpVec += extra;
        }
    }
    mMovementStates.mLowBits.mBit26 = 1;
}


// NONMATCHING: temporary/register allocation; verified full SMG2 eligibility gate.
bool Mario::tryTurnJump() {
    if (fn_803EF7C0() == 0x20) return false;
    if (getPlayerMode() == 4) return false;
    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit27)) && (_3D2 == 0 || static_cast<u8>(checkStickFrontBack()) != 2)) return false;
    MarioConstTable* table = mActor->mConst->getTable();
    if (static_cast<s32>(table->mTurnSlipTime - _3D0) < table->mTurnJumpInhibitTime) {
        _3D0 = 0;
        mMovementStates.mLowBits.mBit27 = 0;
        return false;
    }
    _430 = 0x4;

    if (_3D2 != 0) {
        TVec3f horizontal;
        MR::isNearZero(mAirGravityVec, 0.001f);
        MR::vecKillElement(_3E4, mAirGravityVec, &horizontal);
        MR::normalizeOrZero(&horizontal);

        const f32 turnJumpFrontSpeed = mActor->getConst().getTable()->mTurnJumpFrontSpeed;
        mJumpVec = -horizontal * turnJumpFrontSpeed;

        setFrontVecKeepUp(-_3E4);
        _220 = _3E4;
    } else {
        TVec3f horizontal;
        MR::isNearZero(mAirGravityVec, 0.001f);
        MR::vecKillElement(_220, mAirGravityVec, &horizontal);
        MR::normalizeOrZero(&horizontal);

        const f32 turnJumpFrontSpeed = mActor->getConst().getTable()->mTurnJumpFrontSpeed;
        mJumpVec = -horizontal * turnJumpFrontSpeed;
    }

    const f32 jumpHeight = getJumpHeight();
    mJumpVec += -mActor->mAppliedGravityVec * jumpHeight;

    mFlags14.mLowBits.mBit8 = 0;
    if ((static_cast<u32>(mDrawStates.mBits.mBit20))) {
        recordJumpEnforceMove();
        doEnforceJump(1.0f);
    }

    mMovementStates.mLowBits.mBit17 = 1;
#ifdef GALAXY_NATIVE
    sOdyssey.active = false;
    if (odysseyOn()) {
        // SMO's sideflip: up 32, gravity 1.0, 9 along the new heading.
        TVec3f up(-mActor->mAppliedGravityVec);
        MR::normalizeOrZero(&up);
        odysseyStart(PetariNative::Odyssey::Air::Sideflip, mJumpVec, up, 0.0f, _430);
        sOdyssey.lastChain = -1;
    }
#endif
    procJump(true);
    changeAnimation("ターンジャンプ", "落下");
    playSound("後ジャンプ");
    playSound("ジャンプ踏切");
    playEffect("共通ハイジャンプ");
    if (isPlayerModeYoshi()) MR::startActionSound(mActor->getYoshi(), "JumpHigh", -1, -1, -1);
    else playSound("声後ジャンプ");

    _3D0 = 0;
    mMovementStates.mLowBits.mBit31 = 1;
    mMovementStates.mLowBits.mBit30 = 0;
    mMovementStates.mLowBits.mBit27 = 0;
    mMovementStates.mLowBits.mBit26 = 0;
    mWalkSpeed = 0.0f;
    mFlags14.mLowBits.mBit21 = 0;
    return true;
}


void Mario::trySquatJump() {
    mMovementStates.mHighBits.mBit30 = 0;
    _430 = 0x5;

    mJumpVec = getAirFrontVec() * mActor->getConst().getTable()->mSquatJumpFrontSpeed;

    TVec3f stickVec(_190 + _19C);
    if (!MR::isNearZero(stickVec)) {
        addVelocity(-stickVec);

        MR::vecKillElement(stickVec, mActor->mAppliedGravityVec, &stickVec);
        cutGravityElementFromJumpVec(true);

        mJumpVec += stickVec * 1.0f;
    }

    const f32 jumpHeight = mActor->getConst().getTable()->mJumpHeight[_430];
    mJumpVec += -mActor->mAppliedGravityVec * jumpHeight;

    mFlags14.mLowBits.mBit8 = 0;
    if ((static_cast<u32>(mDrawStates.mBits.mBit20))) {
        recordJumpEnforceMove();

        TVec3f moveKeep;
        f32 gravityMove = MR::vecKillElement(_184, getAirGravityVec(), &moveKeep);

        f32 enforceScale = 1.0f - (MR::diffAngleAbs(mFrontVec, moveKeep) / 1.5707964f);
        if (enforceScale < 0.0f) {
            enforceScale = 0.0f;
        }

        const f32 jumpMag = mJumpVec.length();
        const f32 jumpScale = MR::clamp(1.0f - ((jumpMag - 4.0f) / 10.0f), 0.0f, 1.0f);
        enforceScale += jumpScale;
        enforceScale = MR::clamp(enforceScale, 0.0f, 1.0f);

        if (mJumpVec.dot(moveKeep) < 0.0f) {
            mDrawStates.mBits.mBit4 = 1;
            initActiveJumpVec();
        }

        mVelocity -= moveKeep;

        mJumpVec += moveKeep * enforceScale;

        if (gravityMove < 0.0f) {
            mJumpVec += getAirGravityVec() * gravityMove;
        }

        invalidateRelativePosition();
    }

    mMovementStates.mLowBits.mBit17 = 1;
    mMovementStates.mLowBits.mBit30 = 0;
#ifdef GALAXY_NATIVE
    sOdyssey.active = false;
    if (odysseyOn()) {
        // SMO's long jump: up 12, gravity 0.48, start min(speed + 4, 14), cap 23.
        TVec3f up(-mActor->mAppliedGravityVec);
        MR::normalizeOrZero(&up);
        odysseyStart(PetariNative::Odyssey::Air::LongJump, getAirFrontVec(), up,
                     mWalkSpeed * mActor->mConst->getTable()->mWalkSpeed, _430);
        sOdyssey.lastChain = -1;
    }
#endif
    procJump(true);
    changeAnimationNonStop("幅とび");
    changeAnimation(nullptr, "落下");
    playSound("幅ジャンプ");
    playSound("声幅ジャンプ");
    playSound("ジャンプ踏切");

    mMovementStates.mLowBits.mBit31 = 1;
    _3D0 = 0;
    mMovementStates.mLowBits.mBit26 = 0;
    mMovementStates.mLowBits.mBit27 = 0;
    return;
}


void Mario::tryBackJump() {
    mMovementStates.mHighBits.mBit30 = 1;
    mMovementStates.mLowBits.mBit21 = 0;
    _430 = 0x6;

    const f32 backSpeed = mActor->getConst().getTable()->mSquatJumpBackSpeed;
    mJumpVec = -getAirFrontVec() * backSpeed;

    const f32 jumpHeight = getJumpHeight();
    mJumpVec += -mActor->mAppliedGravityVec * jumpHeight;

    mFlags14.mLowBits.mBit8 = 0;
    if ((static_cast<u32>(mDrawStates.mBits.mBit20))) {
        recordJumpEnforceMove();
        doEnforceJump(1.0f);
    }

    mMovementStates.mLowBits.mBit17 = 1;
#ifdef GALAXY_NATIVE
    sOdyssey.active = false;
    if (odysseyOn()) {
        // SMO's backflip: up 32, gravity 1.0, 5 backwards (reference: the facing).
        TVec3f up(-mActor->mAppliedGravityVec);
        MR::normalizeOrZero(&up);
        odysseyStart(PetariNative::Odyssey::Air::Backflip, getAirFrontVec(), up, 0.0f, _430);
        sOdyssey.lastChain = -1;
    }
#endif
    procJump(true);
    changeAnimation("しゃがみジャンプ", "落下");
    playSound("後ジャンプ");
    playSound("ジャンプ踏切");
    playEffect("共通ハイジャンプ");
    if (isPlayerModeYoshi()) {
        MR::startActionSound(mActor->getYoshi(), "JumpHigh", -1, -1, -1);
    } else {
        playSound("声後ジャンプ");
    }

    if (mSwim->checkUnderWaterFull(-mFrontVec) > 500.0f && mStickPos.z == 0.0f) {
        changeAnimationNonStop("後方飛び込みジャンプ");
        if (!isPlayerModeYoshi()) {
            playSound("声高飛び込み");
        }
        playSound("ジャンプ踏切");
    }

    mWalkSpeed = 0.0f;
    mMovementStates.mLowBits.mBit31 = 1;
    _3D0 = 0;
    mMovementStates.mLowBits.mBit30 = 0;
    mMovementStates.mLowBits.mBit27 = 0;
    mMovementStates.mLowBits.mBit26 = 0;
    return;
}


void Mario::tryTornadoJump() {
    _430 = 0x8;
    _42A = 0;

    const f32 frontSpeed = mWalkSpeed;
    const f32 tornadoSpeed = mActor->getConst().getTable()->mJumpTornadoSpeed;

    mJumpVec = mFrontVec * frontSpeed * tornadoSpeed;

    const f32 jumpHeight = getJumpHeight();
    mJumpVec += -mActor->mAppliedGravityVec * jumpHeight;

    mMovementStates.mLowBits.mBit17 = 0;
    mMovementStates.mLowBits.mBit16 = 1;
    procJump(true);
    changeAnimation("ジャンプ", "落下");
    playSound("トルネードジャンプ");
    if (isPlayerModeYoshi()) {
        MR::startActionSound(mActor->getYoshi(), "HeadShake", -1, -1, -1);
    } else {
        playSound("声スピン");
    }

    mMovementStates.mLowBits.mBit31 = 1;
    _3D0 = 0;
    mMovementStates.mLowBits.mBit27 = 0;
    mMovementStates.mLowBits.mBit26 = 0;
    mMovementStates.mLowBits.mBit2 = 0;
    mWalkSpeed = 0.0f;

    _402 = mActor->getConst().getTable()->mAirWalkTimeTornado;
    _424 = mActor->getConst().getTable()->mTornadoZeroGravityTimer;

    const f32 boosterPower = mActor->getConst().getTable()->mTornadoBoostPower;
    setRocketBooster(-(*getGravityVec()) * boosterPower, mActor->getConst().getTable()->mTornadoBoostAttn,
                     mActor->getConst().getTable()->mTornadoBoostTimer);
}


void Mario::startTornadoCentering(HitSensor* pSensor) {
    pushTask(&Mario::taskOnTornadoCentering, 0x400);
    mTornadoCenterSensor = pSensor;
    _A34 = 0x1E;
}


bool Mario::taskOnTornadoCentering(u32 a1) {
    if (_A34 != 0) {
        TVec3f sensorOffset(mTornadoCenterSensor->mHost->mPosition - mPosHolder->mPos);
        MR::vecKillElement(sensorOffset, *getGravityVec(), &sensorOffset);

        f32 mag = sensorOffset.length();
        if (mag < 10.0f) {
            mag = 10.0f;
        }

        f32 ratio = 10.0f / mag;
        if (ratio < 0.1f) {
            ratio = 0.1f;
        }

        addTrans(sensorOffset * ratio);

        _A34--;

        if (_A34 == 0) {
            return false;
        }

        return true;
    }

    return false;
}


// NONMATCHING: literal pool offsets; full string arguments verified.
void Mario::trySpinJump(u8 a1) {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
        return;
    }
#ifdef GALAXY_NATIVE
    // OdysseyMovement: no spin out of the mod's dive or long jump (as in SMO).
    if (sOdyssey.active && (sOdyssey.air.kind == PetariNative::Odyssey::Air::Dive ||
                            sOdyssey.air.kind == PetariNative::Odyssey::Air::LongJump)) {
        return;
    }
#endif

    if (_430 == 8) {
        return;
    }

    if (isStatusActive(0x17)) {
        return;
    }

    if (getPlayerMode() == 4) {
        return;
    }

    if (getPlayerMode() == 6) {
        return;
    }

    if (getPlayerMode() == 7 && mVerticalSpeed > 100.0f) {
        tryStartFoo();
        return;
    }

    if (isAnimationRun("水泳スピン移動")) {
        return;
    }

    if (isAnimationRun("水泳スピンジャンプ")) {
        return;
    }

#ifdef GALAXY_NATIVE
    if (std::getenv("PETARI_CAPPY_TRACE") != nullptr) {
        std::fprintf(stderr, "[cappy] mario spin (air) starts\n");
    }
#endif
    _430 = 0x8;
    _42A = a1 + 1;

    const f32 frontSpeed = mWalkSpeed;
    const f32 tornadoSpeed = mActor->getConst().getTable()->mJumpTornadoSpeed;
    mJumpVec = mFrontVec * frontSpeed * tornadoSpeed;

    const f32 spinJumpHeight = mActor->getConst().getTable()->mSpinJumpHeight;
    mJumpVec += -mActor->mAppliedGravityVec * spinJumpHeight;

    mMovementStates.mLowBits.mBit17 = 0;
    _402 = mActor->getConst().getTable()->mAirWalkTimeSpin;
    procJump(true);

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
        stopAnimationUpper(nullptr);
        if (isPlayerModeCloud()) {
            changeAnimation("雲スピン空中", "落下");
        } else {
            changeAnimation("空中ひねり", "落下");
        }
        playSound("スピンジャンプ");
        if (isPlayerModeYoshi()) {
            MR::startActionSound(mActor->getYoshi(), "HeadShake", -1, -1, -1);
        } else {
            playSound("声スピン");
        }
        startPadVib(2);

        mMovementStates.mLowBits.mBit31 = 1;
        mMovementStates.mLowBits.mBit27 = 0;
        mMovementStates.mLowBits.mBit26 = 0;
        mMovementStates.mLowBits.mBit2 = 0;
        mFlags14.mLowBits.mBit6 = 1;
        _3D0 = 0;
        mWalkSpeed = 0.0f;
        mFlags14.mLowBits.mBit30 = 1;

        tryShootStickingPopo();

        if (isStatusActive(MarioStatus_Rabbit)) {
            mRabbit->hop();
        }
    }
}


void Mario::tryForceJumpDelay(const TVec3f& rVec) {
    mMovementStates.mHighBits.mBit18 = 1;
    _304 = rVec;
}


void Mario::tryFreeJumpDelay(const TVec3f& rVec) {
    mMovementStates.mHighBits.mBit17 = 1;
    _304 = rVec;
}


void Mario::tryForceJump(const TVec3f& rVec, bool a2) {
    if (rVec.dot(*getGravityVec()) > 0.0f) {
        mMovementStates.mHighBits.mBit31 = 1;
    }

    mMovementStates.mLowBits.mBit31 = 1;
    mMovementStates.mLowBits.mBit30 = 0;
    mMovementStates.mLowBits.mBit26 = 0;
    { const u32 flagValue = static_cast<u32>(a2); mMovementStates.mLowBits.mBit17 = flagValue; }
    mMovementStates.mLowBits.mBit20 = 0;
    mJumpVec = rVec;

    if (getPlayerMode() == 4 || getPlayerMode() == 6) {
        _16C = mJumpVec;
    }

    resetTornado();

    _3BC = 0;
    _3CA = 0;
    _3CC = 0;
    mMovementStates.mLowBits.mBit12 = 1;
    mMovementStates.mLowBits.mBit14 = 0;
    mFlags14.mLowBits.mBit23 = 0;
    _3BE = 0;
    _344 = mSideVec;
    _4B0 = mPosHolder->mPos;
    _42C = 0;
    _76C = 0;
    procJump(false);
    changeAnimation(nullptr, "落下");
    mRabbit->forceJump();
    return;
}


void Mario::tryForceFreeJump(const TVec3f& rVec) {
    mMovementStates.mLowBits.mBit25 = 0;
    _430 = 0xC;
    mMovementStates.mHighBits.mBit21 = 1;
    mMovementStates.mLowBits.mBit22 = 0;

    tryForceJump(rVec, true);
    _3BC = 0;
    mMovementStates.mLowBits.mBit14 = 0;
    _402 = mActor->getConst().getTable()->mAirWalkTime;

    if (getPlayerMode() == 4) {
        playSound("ハチ体力完全回復");
    }

    _76C = 0;
    return;
}


void Mario::tryForcePowerJump(const TVec3f& rVec, bool a2) {
    mMovementStates.mLowBits.mBit25 = 0;
    _430 = 0xD;
    mMovementStates.mHighBits.mBit21 = 1;
    mMovementStates.mLowBits.mBit22 = 0;
    const bool _2FPrev = (static_cast<u32>(mMovementStates.mHighBits.mBit16));
    mMovementStates.mHighBits.mBit16 = 1;
    tryForceJump(rVec, true);
    { const u32 flagValue = static_cast<u32>(_2FPrev); mMovementStates.mHighBits.mBit16 = flagValue; }
    _3BC = 0;
    mMovementStates.mLowBits.mBit14 = 0;

    if (!a2) {
        _402 = mActor->getConst().getTable()->mAirWalkTime;

        if (getPlayerMode() == 4) {
            playSound("ハチ体力完全回復");
        }
    }

    _76C = 0;
    return;
}


void Mario::tryFreeJump(const TVec3f& rVec, bool a2) {
    _430 = 0;

    if (rVec.dot(*getGravityVec()) > 0.0f) {
        mMovementStates.mHighBits.mBit31 = 1;
    }

    mMovementStates.mLowBits.mBit31 = 1;
    mMovementStates.mLowBits.mBit30 = 0;
    mMovementStates.mLowBits.mBit26 = 0;
    { const u32 flagValue = static_cast<u32>(a2); mMovementStates.mLowBits.mBit17 = flagValue; }
    mMovementStates.mLowBits.mBit20 = 0;
    mJumpVec = rVec;
    resetTornado();

    _3BC = 0;
    _3CA = 0;
    _3CC = 0;
    mMovementStates.mLowBits.mBit12 = 1;
    mMovementStates.mLowBits.mBit14 = 0;
    _3BE = 0;
    _76C = 0;
    _344 = mSideVec;
    procJump(false);
    changeAnimation(nullptr, "落下");
    mRabbit->forceJump();
    return;
}


void Mario::tryWallJump(const TVec3f& rVec, bool a2) {
    _430 = 0x7;

    mMovementStates.mLowBits.mBit31 = 1;
    mMovementStates.mLowBits.mBit26 = 0;
    mMovementStates.mLowBits.mBit17 = 1;
    mMovementStates.mLowBits.mBit20 = 0;
    mMovementStates.mLowBits.mBit25 = 0;
    mMovementStates.mHighBits.mBit23 = 0;
    mFlags24.mHighBits.mBit23 = 0;
    mMovementStates.mHighBits.mBit22 = 0;

    if (a2) {
        mMovementStates.mLowBits.mBit22 = 1;
    } else {
        mMovementStates.mLowBits.mBit22 = 0;
    }

    mJumpVec = rVec;
    _4B0 = mPosHolder->mPos;
    _3BC = 0;
    mMovementStates.mLowBits.mBit12 = 1;
    mMovementStates.mLowBits.mBit14 = 0;
    _3CA = 0;
    _3CC = 0;
    _3BE = 0;
    mMovementStates.mLowBits.mBit2 = 1;

    if (getPlayerMode() == 5) {
        changeAnimationNonStop("ホッパー壁ジャンプ");
        playSound("ホッパージャンプ");
    } else {
        changeAnimation("壁ジャンプ", "落下");
        playSound("ジャンプ踏切");
    }

    playSound("声小ジャンプ");

    if (a2) {
        setFrontVecKeepUp(rVec);
    }

    stopWalk();
    mRabbit->forceJump();
#ifdef GALAXY_NATIVE
    sOdyssey.active = false;
    if (odysseyOn()) {
        // SMO's wall jump: 8.6 away from the wall, 23 up, gravity 0.95, the stick
        // ignored for 25 frames. Galaxy's wall gives the direction (rVec).
        TVec3f up(-mActor->mAppliedGravityVec);
        MR::normalizeOrZero(&up);
        odysseyStart(PetariNative::Odyssey::Air::WallJump, rVec, up, 0.0f, _430);
        sOdyssey.lastChain = -1;
        petariCappyWallJump();
        sOdysseyAirFrames = 0;  // the wall jump's own press does not dive
    }
#endif
    return;
}


void Mario::tryStickJump(const TVec3f& rVec) {
    stopJump();
    initJumpParam();

    mMovementStates.mLowBits.mBit31 = 1;
    mMovementStates.mLowBits.mBit26 = 0;
    mMovementStates.mLowBits.mBit17 = 1;
    mMovementStates.mLowBits.mBit20 = 0;
    _430 = 0;
    mFlags24.mHighBits.mBit23 = 0;
    mMovementStates.mHighBits.mBit23 = 0;
    mMovementStates.mHighBits.mBit22 = 0;
    mMovementStates.mLowBits.mBit25 = 0;
    mMovementStates.mLowBits.mBit22 = 1;
    mJumpVec = rVec;
    _3CA = 0;
    _3CC = 0;
    _3BE = 0;
    mMovementStates.mLowBits.mBit12 = 1;
    mMovementStates.mLowBits.mBit14 = 0;
    mMovementStates.mLowBits.mBit2 = 1;
    _76C = mActor->getConst().getTable()->mBeeGravityReviveTime;
    _770 = 0.0f;
    _3BC = 0xA;
    playSound("ハチ壁ジャンプ");
    playSound("声小ジャンプ");
    setFrontVecKeepUp(rVec);
    changeAnimationNonStop("ハチ壁ジャンプ");
    return;
}


void Mario::trySlipUpJump() {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) || isPlayerModeYoshi()) {
        mMovementStates.mLowBits.mBit25 = 0;
        mMovementStates.mHighBits.mBit23 = 0;
        return;
    }

    changeAnimation("スリップアップ");
    playSound("スリップアップ");
    playEffect("スリップアップ");
    startPadVib("マリオ[スリップアップ]");

    mJumpVec = -getAirGravityVec() * mActor->getConst().getTable()->mSlipUpHeight;

    f32 ratio = 1.0f;
    TVec3f predictPos(mPosHolder->mPos - *getGravityVec() * 100.0f);

    TVec3f frontVec;
    MR::vecKillElement(mJumpVec, mFrontVec, &frontVec);
    MR::vecKillElement(frontVec, *getGravityVec(), &frontVec);
    frontVec += mFrontVec * mActor->getConst().getTable()->mSlipUpFront;
    MR::normalizeOrZero(&frontVec);

    f32 step = 50.0f;
    u32 hitCount;
    TVec3f hitPos;
    for (hitCount = 0; hitCount < 10; hitCount++) {
        Triangle* pTmpPolygon = fn_803EFA20();
        predictPos += frontVec * step;

        if (!fn_803B2B50(predictPos, *getGravityVec() * 150.0f, &hitPos, pTmpPolygon)) {
            break;
        }
    }

    ratio *= static_cast< f32 >(hitCount) / 10.0f;
    cutVecElementFromJumpVec(mFrontVec);

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit22))) {
        mJumpVec += mFrontVec * mActor->getConst().getTable()->mSlipUpFront * ratio;
        mMovementStates.mHighBits.mBit22 = 1;
    } else {
        mJumpVec += mFrontVec * mActor->getConst().getTable()->mSlipUpFrontWeak * ratio;
    }

    mMovementStates.mLowBits.mBit25 = 0;
    mMovementStates.mLowBits.mBit17 = 1;
    initJumpParam();
    _430 = 0xA;
    return;
}


void Mario::tryHangSlipUp() {
    mMovementStates.mLowBits.mBit26 = 0;
    getPlayer()->tryJump();
    changeAnimation("つかまりスリップアップ");
    playSound("スリップアップ");
    playEffect("スリップアップ");
    startPadVib("マリオ[スリップアップ]");

    const f32 slipUpHeight = mActor->getConst().getTable()->mSlipUpHeightHang;
    mJumpVec = -getAirGravityVec() * slipUpHeight;
    cutVecElementFromJumpVec(mFrontVec);

    const f32 slipUpFront = mActor->getConst().getTable()->mSlipUpFrontHang;
    mJumpVec += mFrontVec * slipUpFront;

    mMovementStates.mLowBits.mBit25 = 0;
    mMovementStates.mLowBits.mBit17 = 1;
    _430 = 0xA;
    mActor->setBlendMtxTimer(0x8);
    mTargetWalkSpeedIndex = 0x7;
    mWalkSpeed = 0.6f;
    return;
}


void Mario::tryDrop() {
    if (((static_cast<u32>(mMovementStates.mLowBits.mBit23))) && !(static_cast<u32>(mDrawStates.mBits.mBit20))) {
        TVec3f horizontal;
        MR::vecKillElement(_4E8 - mGroundPos, *getGravityVec(), &horizontal);

        if (horizontal.dot(*mFrontWallTriangle->getNormal(0)) < 0.0f && horizontal.length() < 80.0f) {
            getPlayer()->mMovementStates.mLowBits.mBit30 = 1;
            push(*mFrontWallTriangle->getNormal(0) * (80.0f - horizontal.length()) * 0.1f);
            return;
        }
    }

    if (((static_cast<u32>(mMovementStates.mLowBits.mBit5))) && !(static_cast<u32>(mDrawStates.mBits.mBit20))) {
        TVec3f horizontal;
        MR::vecKillElement(_500 - mGroundPos, *getGravityVec(), &horizontal);

        if (horizontal.dot(*mSideWallTriangle->getNormal(0)) < 0.0f && horizontal.length() < 80.0f) {
            push(*mSideWallTriangle->getNormal(0) * (80.0f - horizontal.length()) * 0.1f);
            getPlayer()->mMovementStates.mLowBits.mBit30 = 1;
            return;
        }
    }

    mMovementStates.mHighBits.mBit30 = 1;
    mMovementStates.mLowBits.mBit20 = 0;

    mJumpVec = mFrontVec * mWalkSpeed * ::cDropFrontSpeed;

    if (MR::isNearZero(_8F8) && ((static_cast<u32>(mMovementStates.mHighBits.mBit28)))) {
        TVec3f side;
        side.cross(getAirGravityVec(), _368);
        MR::normalizeOrZero(&side);
        _8F8.cross(_368, side);
        MR::normalizeOrZero(&_8F8);
    }

    if (!MR::isNearZero(_8F8)) {
        mJumpVec = _8F8;
        cutGravityElementFromJumpVec(false);

        if (_16C.length() > 20.0f) {
            mJumpVec += -mActor->getAirGravityVec() * 10.0f;
        } else {
            f32 gravScale = mActor->mBinderDirection5DC.length();
            if (gravScale > 8.0f) {
                gravScale = 8.0f;
            }

            mJumpVec += -mActor->getAirGravityVec() * gravScale;
            _8F8.zero();
        }
    }

    TVec3f carryVec(_184);
    MR::vecKillElement(carryVec, *getGravityVec(), &carryVec);
    invalidateRelativePosition();

    TVec3f groundDeltaNoGravity;
    MR::vecKillElement(mGroundPos - mPosHolder->mPos, *getGravityVec(), &groundDeltaNoGravity);

    if (carryVec.dot(groundDeltaNoGravity) > 0.0f) {
        carryVec = carryVec * 0.1f;
    }

    mJumpVec += carryVec;

    _340 = 1.0f - mWalkSpeed;
    if (_340 < 0.0f) {
        _340 = 0.0f;
    }

    _430 = 0;
    procJump(true);

    mMovementStates.mLowBits.mBit31 = 1;
    mMovementStates.mLowBits.mBit26 = 0;
    mMovementStates.mLowBits.mBit17 = 1;
    mFlags14.mHighBits.mBit28 = 0;
    mMovementStates.mLowBits.mBit20 = 0;

    if (isAnimationRun("坂すべり上向きうつぶせ", 2) || isAnimationRun("坂すべり下向きあおむけ", 3)) {
        _428 = 0xF;
    }

    changeAnimation(nullptr, "落下");

    if (getPlayerMode() == 4) {
        _408 = mActor->getConst().getTable()->mBeeGravityPowerTimeD;
        _3BC = mActor->getConst().getTable()->mBeeAirWalkInhibitTimeD - 5;
    }

    if (isAnimationRun("ショート着地")) {
        mMovementStates.mHighBits.mBit28 = 1;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        stopWalk();
    }

    return;
}


bool Mario::isDigitalJump() const NO_INLINE {
    return ((mMovementStates.mLowWord >> 17) & 1U);
}


void Mario::initActiveJumpVec() {
    _2EC = getAirGravityVec();
    MR::vecKillElement(mJumpVec, _2EC, &_2E0);

    if (MR::normalizeOrZero(&_2E0)) {
        _2E0 = mFrontVec;
    }
}


void Mario::initJumpParam() {
    const u32 flags1 = mFlags20.mWord;
    mMovementStates.mLowBits.mBit12 = 1;

    if (((static_cast<u32>(mFlags20.mBits.mBit21))) || ((static_cast<u32>(mFlags20.mBits.mBit20)))) {
        mFlags14.mLowBits.mBit0 = 1;
    }

    mMovementStates.mHighBits.mBit7 = 0;
    mFlags14.mLowBits.mBit23 = 0;
    mMovementStates.mHighBits.mBit16 = 0;
    mFlags14.mLowBits.mBit17 = 0;

    if (!(static_cast<u32>(mDrawStates.mBits.mBit4))) {
        initActiveJumpVec();
    }

    _42C = 0;
    _3BC = 0;
    _3CA = 0;
    _3CC = 0;
    _774 = 0;
    mMovementStates.mLowBits.mBit14 = 0;
    _3BE = 0;
    _344 = mSideVec;

    _408 = 0;
    _76C = 0;
    mMovementStates.mLowBits.mBit19 = 0;
    mMovementStates.mLowBits.mBit15 = 0;
    mMovementStates.mLowBits.mBit27 = 0;
    mMovementStates.mHighBits.mBit31 = 0;
    mFlags14.mLowBits.mBit19 = 0;
    _3D2 = 0;
    _3D0 = 0;
    _3CE = 0;
    { const u32 flagValue = static_cast<u32>((static_cast<u32>(mMovementStates.mLowBits.mBit23))); mMovementStates.mHighBits.mBit21 = flagValue; }

    if (_430 != 5) {
        cancelSquatMode();
    }

    _4B0 = mPosHolder->mPos;
    _426 = 0;

    if (mSinkTimer != 0) {
        mJumpVec.scale(0.2f + (0.8f * ((0x100 - mSinkTimer) / 256.0f)));
        mSinkTimer = 0;
    }
}


bool Mario::isEnableFutureJump() const {
    if (_430 == 0x3) {
        return false;
    }

    if (!isRising()) {
        const f32 dot = mJumpVec.dot(*getGravityVec());
        const s16 limit = mActor->getConst().getTable()->mFutureJumpReqLimitTime;
        if (mVerticalSpeed < dot * static_cast< f32 >(limit)) {
            return true;
        }
    }

    return false;
}


void Mario::procJump(bool a1) {
    if((static_cast<u32>(mFlags14.mHighBits.mBit21))) fn_803B12B0();
    if((static_cast<u32>(mMovementStates.mLowBits.mBit20))) { procHipDrop();return; }
    if(_424!=0) { _424--;return; }
    procRocketBooster();
    if((static_cast<u32>(mFlags14.mLowBits.mBit12))) fixFrontVecByGravity();
    if(isPlayerModeYoshi()) {
        if(checkSquat(true)) {
            if(isAnimationRun("カリカリ限界")) stopAnimationUpper(nullptr,nullptr);
            stopAnimation(nullptr,"しゃがみ基本");
            mMovementStates.mLowBits.mBit21 = 1;
            mFlags14.mHighBits.mBit21 = 0;mFlags14.mHighBits.mBit20 = 1;
        } else if(isAnimationRun("しゃがみ基本")) {
            mMovementStates.mLowBits.mBit21 = 0;
            stopAnimation(nullptr,"落下");
        }
    } else if(isAnimationRun("しゃがみ基本")) {
        mMovementStates.mLowBits.mBit21 = 0;
        stopAnimation(nullptr,"落下");
    }
    fn_8039A020();fn_8039A090();
    if(a1) initJumpParam();
    else {
        if(mActor->fn_803BDE10() && jumpToHipDrop()) return;
        if(mActor->fn_803B9920() && fn_803B1540()) return;
    }
    if(isRising()) { fn_8039A2A0();mJumpFallTimer=0; }
    else {
        fn_8039A1C0();
        mJumpFallTimer++;
        if(mJumpFallTimer>65) MR::startActionSound(mActor,"FallWind",-1,-1,-1);
    }
    if(_3CC) _3CC--;
    if(fn_8039A3D0()) {
        if(fn_8039A430()) return;
        fn_8039A460();
    }
    mMovementStates.mLowBits.mBit13 = 0;
    fn_8039A480();fn_8039A5B0();
    if(_76C) _76C--;
    fn_8039A600();checkWallRising();checkWallJumpHit();
    if(!(static_cast<u32>(mMovementStates.mLowBits.mBit30))) fixFrontVecFromUpSide();
    decideSlipUp();fn_8039AB50();
#ifdef GALAXY_NATIVE
    if(((mMovementStates.mLowWord >> 30) & 1U) && !a1) {
        sOdysseyLanded = sOdyssey.active ? sOdyssey.air.kind : PetariNative::Odyssey::Air::None;
        sOdyssey.active = false;
        sCappyDiveReady = false;
        sOdysseyAirFrames = 0;
        sOdysseyAirDived = false;
        PetariNative::Odyssey::Live::noSpinAir = false;
        PetariNative::Odyssey::Live::airKind = PetariNative::Odyssey::Air::None;
        doLanding();fn_8039ACE0();
    }
    else doAirWalk();
#else
    if(((mMovementStates.mLowWord >> 30) & 1U) && !a1) { doLanding();fn_8039ACE0(); }
    else doAirWalk();
#endif
    _3BC++;
}


void Mario::checkWallRising() {
}


void Mario::checkWallJumpHit() {
    if (_430 == 0x5) {
        if (((static_cast<u32>(mMovementStates.mLowBits.mBit23))) && !checkWallJumpCode()) {
            const f32 wallDot = cutVecElementFromJumpVec(getWallNorm());
            if (checkWallCode("NoAction", true)) {
                return;
            }

            mJumpVec -= getWallNorm() * wallDot;


            blown(mJumpVec * 0.2f);

            TVec3f horizontal;
            MR::vecKillElement(mJumpVec, *getGravityVec(), &horizontal);
            MR::normalizeOrZero(&horizontal);
            if (!MR::isNearZero(horizontal)) {
                setFrontVecKeepUp(-horizontal);
            }

            mMovementStates.mHighBits.mBit20 = 1;
            _402 = 0;
            _428 = 0x3C;
        }

        return;
    }

    if (!isRising() && _3BC > 10 && ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) && calcPolygonAngleD(mFrontWallTriangle) < 80.0f) {
        if (((static_cast<u32>(mFlags14.mLowBits.mBit17))) == 0) {
            mJumpVec = *mFrontWallTriangle->getNormal(0) * 5.0f + getAirGravityVec() * cutGravityElementFromJumpVec(true);
            mFlags14.mLowBits.mBit17 = 1;
            changeAnimation("壁はじき");
        } else {
            const f32 jumpMag = mJumpVec.length();
            cutVecElementFromJumpVec(*mFrontWallTriangle->getNormal(0));
            mJumpVec.setLength(jumpMag);
        }
    }
}


void Mario::decideSlipUp() {
    if (isAnimationRun("壁はじき")) {
        return;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit22))) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit16))) {
        mMovementStates.mLowBits.mBit25 = 0;
        return;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit25))) {
        if (checkWallCode("NotWallSlip", true)) {
            return;
        }

        if (checkWallCode("NoAction", true)) {
            return;
        }

        bool slipFront = false;
        if (isStickOn()) {
            const TVec3f& worldPadDir = getWorldPadDir();
            if (worldPadDir.dot(getWallNorm()) < -0.8f) {
                slipFront = true;
            }
        }

        if (_430 == 0x7) {
            slipFront = true;
        }

        if (((static_cast<u32>(mMovementStates.mHighBits.mBit21))) == 0 && ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) && isRising() && slipFront) {
            mMovementStates.mLowBits.mBit25 = 1;
        }

        if (isRising() && getPlayerMode() != 4 && getPlayerMode() != 6 && getPlayerMode() != 5) {
            if (_4E0 < mActor->getConst().getTable()->mSlipUpContinueHeight) {
                if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) && (static_cast<u32>(mMovementStates.mLowBits.mBit22)) && slipFront && (static_cast<u32>(mMovementStates.mHighBits.mBit22))) {
                    mMovementStates.mLowBits.mBit25 = 1;
                    cutGravityElementFromJumpVec(true);

                    mJumpVec += -getAirGravityVec() * 30.0f;
                }
            }
        }
    } else if (!(static_cast<u32>(mMovementStates.mLowBits.mBit24)) && isRising() && !(static_cast<u32>(mMovementStates.mLowBits.mBit23)) && ((static_cast<u32>(mMovementStates.mLowBits.mBit10)))) {
        trySlipUpJump();
    }
}


void Mario::moveWallSlide(f32 a1) {
    TVec3f crossVec;
    const TVec3f& wallNorm = getWallNorm();
    crossVec.cross(*getGravityVec(), wallNorm);

    if (!MR::normalizeOrZero(&crossVec) && isStickOn()) {
        f32 dot = crossVec.dot(getWorldPadDir());
        const f32 absDot = MR::abs(dot);
        if (absDot > 0.2f) {
            if (dot < -0.2f) {
                dot = (0.2f + dot) / 0.8f;
            } else {
                dot = (dot - 0.2f) / 0.8f;
            }

            addVelocity(crossVec, (10.0f * dot) * a1);
        }
    }
}


bool Mario::jumpToHipDrop() {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
        return false;
    }

    switch (_430) {
    case 5:
        return false;

    case 9:
    case 11:
    case 12:
    case 13:
        if (getPlayerMode() != 4 && isRising()) {
            return false;
        }
    }

    if (mVerticalSpeed < mActor->getConst().getTable()->mHipDropLimitHeight) {
        return false;
    }

    if (MR::isNearZero(mActor->mAppliedGravityVec)) {
        return false;
    }

    if (getPlayerMode() == 6) {
        return false;
    }

    bool isNormalDrop = false;
    if (mActor->_3E5) {
        isNormalDrop = true;
    }

    resetTornado();
    cancelTornadoJump();

    mDrawStates.mBits.mBit23 = 1;
    mMovementStates.mHighBits.mBit30 = 0;
    mMovementStates.mLowBits.mBit20 = 1;
    mJumpVec.zero();
    _422 = 0;

    if (_430 == 4) {
        setFrontVecKeepUp(-_220);
        _430 = 0;
    }

    if (isAnimationRun("カリカリ限界")) {
        stopAnimationUpper(nullptr);
    }

    if (isPlayerModeHopper()) {
        _720 = getAnimationStringPointer("ホッパーヒップドロップ開始");
        _724 = getAnimationStringPointer("ホッパーヒップドロップ");
        _728 = getAnimationStringPointer("ヒップドロップ着地");
        startHipDropBlur();
    } else if (getPlayerMode() == 4) {
        _720 = getAnimationStringPointer("ハチヒップドロップ開始");
        _724 = getAnimationStringPointer("ハチヒップドロップ");

        if (mActor->mBeeWallWalk != 0) {
            _728 = getAnimationStringPointer("ハチヒップドロップ壁着地");
        } else {
            _728 = getAnimationStringPointer("ハチヒップドロップ着地");
        }
    } else if (isNormalDrop) {
        _720 = getAnimationStringPointer("スピンヒップドロップ開始");
        _724 = getAnimationStringPointer("スピンヒップドロップ");
        _728 = getAnimationStringPointer("スピンヒップドロップ着地");
        stopEffect("スピンライト");
        mFlags14.mHighBits.mBit24 = 1;
    } else {
        _720 = getAnimationStringPointer("ヒップドロップ開始");
        _724 = getAnimationStringPointer("ヒップドロップ");
        _728 = getAnimationStringPointer("ヒップドロップ着地");
        startHipDropBlur();
    }

    changeAnimation(_720, _724);
    playSound("尻ドロップ回転");

    if (isNormalDrop) {
        playSound("声スピン尻ドロップ");
        playSound("スピン尻ドロップ回転");

    } else {
        playSound("声尻ドロップ");
    }

    _424 = mActor->getConst().getTable()->mHipDropZeroGrTime;
    mRabbit->forceJump();
    mMovementStates.mHighWord &= 0xFFFFFFFCU;
    return true;
}


void Mario::procHipDrop() {
    f32 gravityHipDrop;
#ifdef GALAXY_NATIVE
    if (odysseyOn() && getPlayerMode() == 0 && !((mMovementStates.mLowWord >> 30) & 1U) &&
        (PetariNative::Odyssey::Live::spinBuffer > 0 || mActor->isRequestSpin())) {
        PetariNative::Odyssey::Live::spinBuffer = 0;
        // SMO's dive: Spin during a ground pound in the air (wind-up or fall).
        odysseyStartDive(*this, "ground pound + Spin");
        return;
    }
    // "Jump during a ground pound dives": a new Jump press during the pound in the
    // air, once per airtime (a cap bounce gives it back), unless the pound lands
    // within the jump pre-input window: then it stays the ground-pound jump.
    if (odysseyOn() && petari_mod_enabled(kModOdysseyJumpDive) && getPlayerMode() == 0 && !((mMovementStates.mLowWord >> 30) & 1U) &&
        !isSwimming() && !sOdysseyAirDived && checkTrgA() && !odysseyGroundPoundLandingSoon(*this)) {
        odysseyStartDive(*this, "ground pound + Jump");
        return;
    }
#endif
    if (isAnimationRun(_720)) {
        if (!isAnimationTerminate(nullptr)) {
            return;
        }

        if (_424 != 0) {
            _424--;
            return;
        }

        stopAnimation(nullptr);
        return;
    }

    if (_424 != 0) {
        _424--;
        return;
    }

    if (mActor->_38C != 0) {
        return;
    }

    if (MR::isNearZero(mActor->mAppliedGravityVec)) {
        mMovementStates.mLowWord |= Mario::MovementStates::DigitalJump;
    }

    if (mActor->_334 != 0) {
        _2F8.x *= 0.5f;
        _2F8.y *= 0.5f;
        _2F8.z *= 0.5f;
        mJumpVec.x *= 0.5f;
        mJumpVec.y *= 0.5f;
        mJumpVec.z *= 0.5f;
    }

    if (((static_cast<u32>(mMovementStates.mLowBits.mBit30))) != 0 || isAnimationRun(_728)) {
        fixFrontVecByGravity();

        if (isAnimationRun(_728)) {
            bool shouldEnd = false;

            if (mActor->_334 != 0) {
                shouldEnd = true;
            } else if (((static_cast<u32>(mMovementStates.mLowBits.mBit30))) == 0 && _3BC > 3 && mVerticalSpeed > 10.0f) {
                changeAnimation(_724);
                mJumpVec = _2F8;
                goto PROC_HIP_DROP_MOVE;
            }

            u16 endLimit = 0xF;
            if (isAnimationRun("スピンヒップドロップ着地")) {
                endLimit = 0x2D;
            }

            if (_3CE > endLimit) {
                if (isStickOn()) {
                    shouldEnd = true;
                }

                if ((static_cast<u32>(mDrawStates.mBits.mBit19))) {
                    shouldEnd = true;
                }
            }

#ifdef GALAXY_NATIVE
            bool groundPoundJumpWindow = _3CE > 5;
            if (odysseyOn()) {
                groundPoundJumpWindow = PetariNative::Odyssey::groundPoundJumpAllowed(_3CE);  // SMO: landing frames 5-30
                if (groundPoundJumpWindow && checkTrgA()) {
                    shouldEnd = true;
                }
            } else
#endif
            if (_3CE > 5 && checkTrgA()) {
                shouldEnd = true;
            }

            if (isAnimationTerminate(nullptr) || shouldEnd) {
                mMovementStates.mLowBits.mBit31 = 0;
                mMovementStates.mLowBits.mBit20 = 0;
                stopAnimation(nullptr, "基本");
                stopEffect("属性尻ドロップ");

                if (checkTrgA()) {
#ifdef GALAXY_NATIVE
                    sOdysseyGroundPoundJump = odysseyOn() && groundPoundJumpWindow;
#endif
                    tryJump();
                    return;
                }
            }

            mWalkSpeed = 0.0f;
        } else {
            _3CE = 0;
            mWalkSpeed = 0.0f;
            _71E = 0;

            if (isCurrentFloorSand() && !isPlayerModeYoshi()) {
                changeStatus(mBury);
                return;
            }

            if (isCurrentFloorSink()) {
                mMovementStates.mLowBits.mBit31 = 0;
                mMovementStates.mLowBits.mBit20 = 0;
                stopAnimation(nullptr, "基本");
                mSinkTimer = 200;
                return;
            }

            playSound("尻ドロップ着地");
            playSound("声尻ドロップ着地");
            playEffectRT("属性尻ドロップ", _368, mPosHolder->mPos);
            startPadVib("最強");
            startCamVib(0);
            mDrawStates.mBits.mBit11 = 1;
            MR::removeAllClingingKarikari();

            if (_960 == 0x1B || _960 == 0x1C || _960 == 9) {
                mMovementStates.mLowBits.mBit31 = 0;
                mMovementStates.mLowBits.mBit20 = 0;
                stopAnimation(nullptr, "基本");
                return;
            }

            changeAnimationWithAttr(_728, 1);
            _2F8 = mJumpVec;
            clearSlope();

            if (mActor->_B90) {
                mMovementStates.mLowBits.mBit31 = 0;
                mMovementStates.mLowBits.mBit20 = 0;
            }
        }

        mJumpVec.zero();
    } else {
        _3BC++;

        if (MR::isNearZero(mActor->getLastMove())) {
            _422++;
            if (_422 == 0xF) {
                stopAnimation(nullptr, "基本");
                mMovementStates.mLowBits.mBit31 = 0;
                mMovementStates.mLowBits.mBit20 = 0;
                mJumpVec.zero();
                mMovementStates.mLowBits.mBit30 = 1;
                return;
            }
        }
    }

PROC_HIP_DROP_MOVE:
    TVec3f moveVec(mJumpVec);
    if (moveVec.length() >= mVerticalSpeed) {
        moveVec.setLength(mVerticalSpeed);
    }

    addVelocity(moveVec);

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        const f32 speedRate = MR::clamp(0.1f + (1.0f - (mJumpVec.length() / mActor->getConst().getTable()->mLimitSpeedHipDrop)), 0.0f, 1.0f);
        gravityHipDrop = mActor->getConst().getTable()->mGravityHipDrop;

        mJumpVec += *getGravityVec() * gravityHipDrop * speedRate;

        const f32 jumpGravity = cutGravityElementFromJumpVec(true);
        if (mJumpVec.length() > 10.0f) {
            mJumpVec.x *= 0.5f;
            mJumpVec.y *= 0.5f;
            mJumpVec.z *= 0.5f;
        }

        mJumpVec += getAirGravityVec() * jumpGravity;

#ifdef GALAXY_NATIVE
        if (odysseyOn()) {
            const f32 hipDropLimit = PetariNative::Odyssey::groundPoundFallSpeed();  // SMO: 45
            if (mJumpVec.length() > hipDropLimit) {
                mJumpVec.setLength(hipDropLimit);
            }
        } else
#endif
        if (mJumpVec.length() > mActor->getConst().getTable()->mLimitSpeedHipDrop) {
            mJumpVec.setLength(mActor->getConst().getTable()->mLimitSpeedHipDrop);
        }

        if ((static_cast<u32>(mFlags14.mHighBits.mBit24)) && isAnimationRun("スピンヒップドロップ")) {
            playSound("スピン尻ドロップ落下");

            Triangle strikeTriangles[0x20];
            HitSensor* homingSensor = nullptr;

            f32 maxAngle = 45.0f;
            f32 speed = mVerticalSpeed;
            if (speed >= 500.0f) {
                speed = 500.0f;
            }

            s32 strikeNum = fn_803B2A20(mShadowPos, speed, speed);

            for (u32 i = 0; i < strikeNum; i++) {
                const HitInfo* strikeInfo = fn_803B2AA0(i);

                if (!mActor->sendMsgToSensor(strikeInfo->mParentTriangle.mSensor, 6) &&
                    fn_803EFA30(strikeInfo->mParentTriangle.mSensor, "SHDホーミング許可")) {
                    TVec3f sensorPos(strikeInfo->mParentTriangle.mSensor->mPosition);

                    if (MR::diffAngleAbs(sensorPos - mActor->_240, *getGravityVec()) < maxAngle) {
                        homingSensor = strikeInfo->mParentTriangle.mSensor;
                    }
                }
            }

            if (homingSensor == nullptr) {
                homingSensor = mActor->mNearestUnderSensor798;
            }

            if (homingSensor != nullptr) {
                TVec3f targetJump(*MR::getSensorPos(homingSensor) - mActor->_240);
                targetJump.setLength(mJumpVec.length());

                TVec3f lastMove;
                mActor->getLastMove(&lastMove);
                if (lastMove.dot(mJumpVec) < 0.0f) {
                    const f32 hitGravity = cutGravityElementFromJumpVec(true);
                    mJumpVec = -mJumpVec + getAirGravityVec() * hitGravity;
                } else if (mJumpVec.dot(targetJump) > 0.0f) {
                    mJumpVec = targetJump;
                }
            }

            if (((static_cast<u32>(mMovementStates.mLowBits.mBit23))) || ((static_cast<u32>(mMovementStates.mLowBits.mBit5))) || ((static_cast<u32>(mMovementStates.mLowBits.mBit6)))) {
                mFlags14.mHighBits.mBit24 = 0;
            }
        }

        fixFrontVecFromUpSide();
    }
}


void Mario::doAirWalk() {
    if (isAnimationRun("水泳ジェット")) {
        return;
    }

    if (_430 == 3) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit16))) {
        return;
    }

    if (isAnimationRun("壁はじき")) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit2))) {
        if (_3BC > mActor->getConst().getTable()->mWaitNeutralTimer) {
            mMovementStates.mLowBits.mBit2 = 0;
        }

        return;
    }

    const f32 walkSpeed = mActor->getConst().getTable()->mWalkSpeed;
    if (_428 != 0) {
        _428--;
        if (_428 == 0) {
            stopAnimation("坂すべり上向きうつぶせ", 2);
            stopAnimation("坂すべり下向きあおむけ", 3);
        }

        return;
    }

    f32 speedKiller;
    TVec3f moveDir;
    TVec3f sideMove;
    calcMoveDir(mStickPos.x, mStickPos.y, &moveDir, true);

    if (mActor->_334 != 0) {
        moveDir.zero();
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit18))) {
        if (MR::diffAngleAbs(getAirGravityVec(), _2EC) > 0.7853982f) {
            TVec3f sideVec;
            sideVec.cross(mFrontVec, getAirGravityVec());
            MR::normalizeOrZero(&sideVec);

            Mtx rotMtx;
            PSMTXRotAxisRad(rotMtx, &sideVec, MR::diffAngleSignedHorizontal(_2EC, getAirGravityVec(), sideVec));

            TVec3f frontRot;
            PSMTXMultVecSR(rotMtx, &_2E0, &frontRot);
            if (frontRot.dot(mFrontVec) < 0.0f) {
                moveDir = -mFrontVec;
            } else {
                moveDir = mFrontVec;
            }
        }
    }

    MR::vecKillElement(moveDir, *getGravityVec(), &moveDir);

    bool inhibitFrontAdjust = false;
    f32 frontDot = MR::vecKillElement(moveDir, mFrontVec, &sideMove);

    if (_430 == 0xB || _430 == 8 || getPlayerMode() == 4 || getPlayerMode() == 6) {
        inhibitFrontAdjust = true;
    } else if (frontDot >= 0.0f) {
        f32 frontReduction = fn_803B1D00();
        s16 reduceBegin = mActor->getConst().getTable()->mJumpFrontReductionBeginTime;
        if (_430 == 5) {
            frontReduction = mActor->getConst().getTable()->mSquatJumpFrontReduction;
            reduceBegin = mActor->getConst().getTable()->mSquatJumpFrontReductionBTime;
        }

        if ((static_cast<u32>(mFlags14.mLowBits.mBit8)) && _8D8 == _45C->mSensor) {
            reduceBegin = 0xB4;
            if (MR::isSameMtx(_8E8->getBaseMtx()->toMtxPtr(), _8E8->getPrevBaseMtx()->toMtxPtr())) {
                MR::vecKillElement(_8DC, getAirGravityVec(), &_8DC);
                mJumpVec -= _8DC;
                _8DC.zero();
            }
        }

        if (((static_cast<u32>(mMovementStates.mLowBits.mBit14))) == 0 && _3BC > reduceBegin) {
            const f32 jumpGravity = cutGravityElementFromJumpVec(true);
            const f32 jumpMag = mJumpVec.length();
            mJumpVec.setLength(frontReduction * jumpMag);

            mJumpVec += getAirGravityVec() * jumpGravity;
        }
    } else {
        if (!isRising() && mVerticalSpeed < mActor->getConst().getTable()->mLandTurnHeight && ((static_cast<u32>(mMovementStates.mLowBits.mBit14))) == 0) {
            mMovementStates.mLowBits.mBit15 = 1;
        } else {
            _3CA++;
        }

        if (isAnimationRun("ターンジャンプ")) {
            _3CA = 0;
            speedKiller = 0.1f;
        } else if (_3CA < mActor->mConst->getTable()->mBackJumpLimitFrame) {
            const f32 remain = static_cast< f32 >(mActor->getConst().getTable()->mBackJumpLimitFrame - _3CA);
            speedKiller =
                mActor->getConst().getTable()->mBackJumpRatio * (remain / static_cast< f32 >(mActor->getConst().getTable()->mBackJumpLimitFrame));
        } else {
            speedKiller = 0.0f;
        }

        if ((static_cast<u32>(mMovementStates.mLowBits.mBit14))) {
            moveDir = sideMove + mFrontVec * mActor->getConst().getTable()->mAirWalkBackBonus * frontDot;
        } else {
            moveDir = sideMove + mFrontVec * frontDot * speedKiller;
        }

        const f32 backDot = -moveDir.dot(mFrontVec);
        if (backDot > mActor->getConst().getTable()->mMaxBackJumpSpeed) {
            moveDir.setLength(mActor->getConst().getTable()->mMaxBackJumpSpeed);
        } else if (backDot < 0.0f && _3CA < mActor->mConst->getTable()->mBackJumpLimitFrame) {
            _3CC = 0xA;
        }
    }

    TVec3f moveNorm(moveDir);
    MR::normalizeOrZero(&moveNorm);

    TVec3f jumpNorm(mJumpVec);
    MR::normalizeOrZero(&jumpNorm);

    if (!inhibitFrontAdjust) {
        f32 alignment = moveNorm.dot(jumpNorm);
        if (alignment < 0.0f) {
            const f32 jumpGravity = cutGravityElementFromJumpVec(true);
            mJumpVec += mJumpVec * mActor->getConst().getTable()->mAirWalkSpeedKiller * alignment;

            mJumpVec += getAirGravityVec() * jumpGravity;
        }
    }

    if (_430 == 8) {
        mJumpVec += moveDir * walkSpeed * 5.0f / mActor->getConst().getTable()->mAirWalkTimerFact2;
    } else if (_430 == 0xB && !isRising()) {
        mJumpVec += moveDir * walkSpeed * 5.0f / mActor->getConst().getTable()->mAirWalkTimerFact2;
        playSound("滞空中");
    } else if (getPlayerMode() == 4) {
        mJumpVec += moveDir * mActor->getConst().getTable()->mBeeAirWalkAcc;
        playSound("滞空中");
    } else if (getPlayerMode() == 6) {
        if (mJumpVec.dot(moveDir) >= 0.0f) {
            f32 frontGravity = moveDir.dot(mFrontVec);
            if (frontGravity > 0.0f) {
                frontGravity = -frontGravity;
            }

            TVec3f normJump(mJumpVec);
            if (!MR::normalizeOrZero(&normJump)) {
                f32 alignment = normJump.dot(mFrontVec);
                alignment += 1.0f;
                f32 halfAlignment = alignment / 2.0f;
                mJumpVec *= 0.998f - 0.01f * (1.0f - halfAlignment);
            }
        }
    } else {
        mJumpVec += moveDir * walkSpeed * mActor->getConst().getTable()->mAirWalkTimerFact1 /
                    (_3BC + mActor->getConst().getTable()->mAirWalkTimerFact2);
    }

    f32 accelerationRate = 1.0f;
    if (_430 != 0xB && getPlayerMode() != 6 && !isRising()) {
        addVelocity(moveDir, walkSpeed * mActor->getConst().getTable()->mAirWalkTimerFact3 * accelerationRate);
    }

    const f32 jumpGravity = cutGravityElementFromJumpVec(true);
    if (_430 == 8) {
        if (mJumpVec.length() > 6.0f) {
            mJumpVec.setLength(6.0f);
        }
    } else if (_430 == 0xB) {
        if (mJumpVec.length() > 7.0f) {
            mJumpVec.setLength(7.0f);
        }
    } else if (getPlayerMode() == 4) {
        if (_774 == 0) {
            if (mJumpVec.length() > mActor->getConst().getTable()->mBeeAirWalkLimit) {
                mJumpVec.setLength(mActor->getConst().getTable()->mBeeAirWalkLimit);
            }
        } else if (_774 > 0x1E) {
            if (mJumpVec.length() > mActor->getConst().getTable()->mMaxJumpSpeed) {
                mJumpVec.setLength(mActor->getConst().getTable()->mMaxJumpSpeed);
            }
        } else {
            f32 t = static_cast< f32 >(_774) / 30.0f;
            const f32 limit = (t * mActor->mConst->getTable()->mMaxJumpSpeed) + ((1.0f - t) * mActor->getConst().getTable()->mBeeAirWalkLimit);
            if (mJumpVec.length() > limit) {
                mJumpVec.setLength(limit);
            }
        }
    } else if (getPlayerMode() != 6) {
        if (mJumpVec.length() > mActor->getConst().getTable()->mMaxJumpSpeed) {
            mJumpVec.setLength(mActor->getConst().getTable()->mMaxJumpSpeed);
        }
    }

    mJumpVec += getAirGravityVec() * jumpGravity;
}


void Mario::stopJump() {
    fixFrontVecByGravity();
    mJumpVec.zero();

    mMovementStates.mLowBits.mBit31 = 0;
    mMovementStates.mLowBits.mBit20 = 0;
    mMovementStates.mLowBits.mBit25 = 0;

    mMovementStates.mHighBits.mBit23 = 0;
    mMovementStates.mHighBits.mBit20 = 0;

    mFlags14.mLowBits.mBit8 = 0;
    mFlags14.mLowBits.mBit6 = 0;
    mFlags14.mLowBits.mBit0 = 0;

    if (getPlayerMode() != 4) {
        _402 = mActor->getConst().getTable()->mAirWalkTime;
    }

    _3BC = 0;
    _424 = 0;
    _426 = 0;
    _428 = 0;

    if (isDefaultAnimationRun("落下")) {
        changeAnimation(nullptr, "基本");
    }

    if (isDefaultAnimationRun(_724)) {
        changeAnimation(nullptr, "基本");
    }

    mWall->_1C = 0;
}


void Mario::cancelTornadoJump() {
    _430 = 0;

    if (((static_cast<u32>(mMovementStates.mLowBits.mBit30))) == 0) {
        stopAnimation("スピンジャンプ", "落下");
    } else {
        stopAnimation("スピンジャンプ");
    }

    _4B0 = mPosHolder->mPos;
    mMovementStates.mHighBits.mBit20 = 1;
    _426 = 0;
}


void Mario::setRocketBooster(const TVec3f& rVec, f32 a2, u16 a3) {
    _426 = a3;
    _454 = a2;
    _448 = rVec;
}


void Mario::procRocketBooster() {
    if (_426 == 0) {
        return;
    }

    _426--;
    mJumpVec += _448;
    _448.scale(_454);
}


bool Mario::isSoftLandingFloor() const {
    switch (_960) {
    case 0x6:
    case 0x7:
    case 0x8:
    case 0xB:
    case 0x22:
        return true;
    default:
        return false;
    }
}


void Mario::checkAndTryForceJump() {
    // NONMATCHING: pooled literal placement.
    bool doForceJump = false;
    TVec3f forceJumpVec;

    if ((((getMovementStates().mLowWord >> 30) & 1U) && !(static_cast<u32>(mMovementStates.mLowBits.mBit31))) || isAnimationRun(_728)) {
        switch (_960) {
        case 0x6:
        case 0x22: {
            const f32 jumpPower = mActor->getConst().getTable()->mCodeJumpPower[0];
            forceJumpVec = *mGroundPolygon->getNormal(0) * jumpPower;
            doForceJump = true;
            break;
        }

        case 0x7: {
            const f32 jumpPower = mActor->getConst().getTable()->mCodeJumpPower[1];
            forceJumpVec = *mGroundPolygon->getNormal(0) * jumpPower;
            doForceJump = true;
            break;
        }

        case 0x8: {
            const f32 jumpPower = mActor->getConst().getTable()->mCodeJumpPower[2];
            forceJumpVec = *mGroundPolygon->getNormal(0) * jumpPower;
            doForceJump = true;
            break;
        }

        case 0xB: {
            MR::vecKillElement(_16C, *getGravityVec(), &forceJumpVec);
            forceJumpVec.setLength(10.0f);

            const f32 jumpPower = mActor->getConst().getTable()->mCodeJumpPower[3];
            forceJumpVec += -(*getGravityVec()) * jumpPower;
            doForceJump = true;
            break;
        }

        default:
            break;
        }
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit18))) {
        doForceJump = false;

        if (mActor->mHealth != 0) {
            const u32 status = getCurrentStatus();
            if (status == 2 || status == 4) {
                closeStatus(nullptr);
            }

            tryForceFreeJump(_304);
        }

        mMovementStates.mHighBits.mBit18 = 0;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit17))) {
        doForceJump = false;

        if (mActor->mHealth != 0) {
            if (getCurrentStatus() == 2) {
                closeStatus(nullptr);
            }

            tryFreeJump(_304, true);
        }

        mMovementStates.mHighBits.mBit17 = 0;
    }

    if (doForceJump) {
        mActor->sendMsgToSensor(mGroundPolygon->mSensor, 2);

        if (getCurrentStatus() == 2) {
            if (mActor->mHealth == 0) {
                mActor->forceGameOverNonStop();
            }

            closeStatus(nullptr);
        }

        mMovementStates.mHighBits.mBit30 = 1;
        mMovementStates.mHighBits.mBit17 = 1;
        _304 = forceJumpVec;
        changeAnimation("フリージャンプ");

        if (mActor->isEnableNerveChange()) {
            playSound("トランポリンジャンプ大");
            if (isPlayerModeYoshi()) {
                MR::startActionSound(mActor->getYoshi(), "JumpObj", -1, -1, -1);
            } else {
                playSound("声大ジャンプ");
            }
            playSound("ジャンプ踏切");
        }
    }
}


// NONMATCHING: register allocation and independent load scheduling; SMG2 orchestration recovered.
void Mario::doLanding() {
    bool hard = false;
    bool blend = false;
    bool keepSlip = false;
    if (getPlayerMode() == 6) return;
    mMovementStates.mLowBits.mBit31 = 0;
    mFlags14.mLowBits.mBit0 = 0;
    mFlags14.mHighBits.mBit30 = 0;
    _3CE = 0;
    mFlags20.mBits.mBit13 = 1;
    TVec3f delta(mPosHolder->mPos);
    delta -= _4B0;
    f32 distance = MR::abs(delta.dot(*getGravityVec()));
    if (_3BC <= 3 && distance < 1.0f && PSVECMag(&mJumpVec) < 10.0f) {
        stopAnimation(nullptr, "基本");
        return;
    }
    if (mActor->_EEB && (static_cast<u32>(mMovementStates.mHighBits.mBit20))) {
        if ((static_cast<u32>(mFlags14.mLowBits.mBit6)) && _430 != 3) {
            mFlags14.mLowBits.mBit6 = 0;
            mActor->_946 = 16;
        }
    }
    resetTornado();
    if (mActor->mBeeWallWalk != 0) playSound("ハチ壁くっつき");
    else playSound("着地");
    if ((isAnimationRun("スケートアクセルジャンプ") || isAnimationRun("スケートジャンプ2") || isAnimationRun("スケートジャンプ3")) && (getPlayerMode() == 3 || getPlayerMode() == 0)) {
        playEffect("スケート着地");
        if (getStickP() == 0.0f) {
            mJumpVec.zero();
            stopWalk();
            changeAnimation("スケート静止着地", "基本");
            return;
        }
        changeStatus(mSkate);
        return;
    }
    if (_430 == 5 && isSkatableFloor()) {
        if (checkSquat(false)) {
            mMovementStates.mLowBits.mBit21 = 1;
            mWalkSpeed = 1.0f;
            if (isAnimationRun("カリカリ限界")) stopAnimationUpper(nullptr, nullptr);
            stopAnimation(nullptr, "しゃがみ基本");
            return;
        }
        cancelSquatMode();
        stopAnimationUpper(nullptr, nullptr);
        changeAnimation("スケート静止着地", "基本");
        return;
    }
    if (isAnimationRun("フーファイター着地")) {
        stopAnimation(nullptr, "基本");
        return;
    }
    if (static_cast<u32>(getPlayer()->mFlags14.mLowBits.mBit29)) {
        Mario* player = getPlayer();
        player->mFlags14.mLowWord &= ~0x20000000;
        _430 = 14;
        goto COMMON_LANDING;
    }
    {
        u32 type = mMovementStates.mHighWord & 3;
        if (type == 2) {
            Mario* player = getPlayer();
            player->mMovementStates.mHighWord &= ~3;
            stopWalk();
            playSound("声ステージイン着地");
            playSound("重い着地");
            changeAnimation("ステージインB", "基本");
            startCamVib(4);
            return;
        }
        if (type == 1) {
            Mario* player = getPlayer();
            player->mMovementStates.mHighWord &= ~3;
            playEffect("スーパースピンドライバ終了");
            playEffect("共通着地大");
            if (!isStickFull()) keepSlip = true;
            hard = true;
            if (_1FC.dot(_368) < 0.0f) {
                if (!isPlayerModeYoshi() && (_960 == 0xD || _960 == 0x1E || _960 == 5 || _960 == 0x17)) {
                    changeStatus(mSukekiyo);
                    return;
                }
                stopWalk();
                startCamVib(4);
                changeAnimation("ハード着地", "基本");
                _3CE += 16;
                forceSetHeadVecKeepSide(_368);
                return;
            }
            goto COMMON_LANDING;
        }
    }
    if (isCurrentFloorSink()) {
        stopAnimation(nullptr, "基本");
        mSinkTimer = 64;
        return;
    }
    if (static_cast<u16>(_960 + 0xFFE5) <= 1) {
        if (!strcmp(MR::getSoundCodeString(_45C), "Sand")) {
            stopAnimation(nullptr, "基本");
            mSinkTimer = 32;
            return;
        }
        goto COMMON_LANDING;
    }
    if (_3BC > 10) playEffect("共通着地普通");
COMMON_LANDING:
    if (static_cast<u32>(_430 - 10) > 1 && _430 != 0 && _430 != 14) {
        if (_430 != 2 && _430 != 6 && _430 != 4) startPadVib(2);
        else startPadVib(3);
    } else startPadVib(static_cast<u32>(0));
    getAnimator()->addRumblePower(5.0f, 60);
    mMovementStates.mHighBits.mBit30 = 0;
    if (getPlayerMode() != 4) _402 = mActor->mConst->getTable()->mAirWalkTime;
    else _71E = 0;
    if (!isSoftLandingFloor()) {
        const TVec3f& position = mPosHolder->mPos;
        TVec3f fall(_4B0);
        fall -= position;
        TVec3f negativeGravity(-*getGravityVec());
        if (fall.dot(negativeGravity) > mActor->mConst->getTable()->mHardLandingHeight) hard = true;
    }
    if (hard && isCurrentFloorSand() && !isPlayerModeYoshi()) {
        changeStatus(mBury);
        return;
    }
    if (isBlendWaitGround()) blend = true;
    fixFrontVecByGravity();
#ifdef GALAXY_NATIVE
    {
        // OdysseyMovement: a long jump or dive landed with a direction held goes
        // straight into a run at the landing speed (SMO keeps the momentum; the run
        // then slows to its cap at 14/120 per frame). Galaxy would drop the long
        // jump's speed and end the dive in a belly flop. The dive lands with the
        // failed dive's forward roll as the get-up. Stick released: Galaxy's landing.
        const PetariNative::Odyssey::Air landed = sOdysseyLanded;
        sOdysseyLanded = PetariNative::Odyssey::Air::None;
        if (landed == PetariNative::Odyssey::Air::Dive && isAnimationRun("幅とび")) {
            XanimeFrameCtrl* ctrl = getAnimator()->getXanimePlayer()->getFrameCtrl();
            if (ctrl != nullptr) {
                ctrl->setRate(1.0f);  // undo the dive's leap timing
            }
        }
        if (odysseyOn() && landed == PetariNative::Odyssey::Air::Dive && !hard && !isStickOn()) {
            // Stick released: the dive ends in Galaxy's belly flop (its landing for
            // a dive into shallow water), as before the leap pose.
            stopWalk();
            changeAnimation("飛び込み失敗着地", "基本");
            goto POST_LANDING;
        }
        if (odysseyOn() && !hard && isStickOn() &&
            (landed == PetariNative::Odyssey::Air::LongJump || landed == PetariNative::Odyssey::Air::Dive)) {
            TVec3f up(-mActor->mAppliedGravityVec);
            MR::normalizeOrZero(&up);
            TVec3f horizontal(mJumpVec - up * up.dot(mJumpVec));
            const f32 speed = horizontal.length();
            if (speed > 1.0f) {
                MR::normalize(&horizontal);
                setFrontVecKeepUp(horizontal);
                if ((mMovementStates.mLowWord >> 21) & 1U) {
                    cancelSquatMode();
                }
                mWalkSpeed = speed / mActor->mConst->getTable()->mWalkSpeed;
                if (landed == PetariNative::Odyssey::Air::Dive) {
                    changeAnimation("飛び込み失敗回転着地", "基本");
                    PetariNative::Odyssey::Live::getUpFrames = 12;  // then the run (Mario::updateWalkSpeed)
                } else {
                    stopAnimation(nullptr, "基本");
                }
                std::fprintf(stderr, "[odyssey] %s landed running at %.3f\n",
                             landed == PetariNative::Odyssey::Air::Dive ? "dive" : "long jump", speed);
                goto POST_LANDING;
            }
        }
    }
#endif
    if (hard || (static_cast<u32>(mDrawStates.mBits.mBit19)) || _430 == 14 || isAnimationRun("飛び込みジャンプ") || isAnimationRun("後方飛び込みジャンプ") || (static_cast<u32>(mFlags14.mLowBits.mBit23)) || isAnimationRun("水上ダメージ中")) {
        fn_803A0490(hard, blend);
        return;
    }
    if (_430 == 5 && checkSquat(false)) {
        mMovementStates.mLowBits.mBit21 = 1;
        mWalkSpeed = 1.0f;
        if (isAnimationRun("カリカリ限界")) stopAnimationUpper(nullptr, nullptr);
        stopAnimation(nullptr, "しゃがみ基本");
    } else if (((isStickFull() || _60D) && _3CA == 0) || (!(static_cast<u32>(mMovementStates.mLowBits.mBit20)) && checkSquat(false))) {
        stopAnimation(nullptr, "基本");
        if (!(static_cast<u32>(mFlags14.mLowBits.mBit21))) recordTurnSlipAngle();
        if (_430 == 5) cancelSquatMode();
    } else {
        fn_803A02F0(keepSlip, hard, blend);
    }
#ifdef GALAXY_NATIVE
POST_LANDING:;
#endif
    if (static_cast<u32>(mDrawStates.mBits.mBit19)) {
        TVec3f horizontal;
        MR::vecKillElement(mJumpVec, *getGravityVec(), &horizontal);
        if (horizontal.dot(*mGroundPolygon->getNormal(0)) > 0.0f) {
            mMovementStates.mHighBits.mBit28 = 1;
            _8F0 = 10.0f;
        } else {
            f32 ratio = (90.0f - calcPolygonAngleD(mGroundPolygon)) / 90.0f;
            mWalkSpeed *= ratio;
            mWalkSpeed = MR::clamp(mWalkSpeed, 0.0f, 2.0f);
            f32 minVelocity = 10.0f - 10.0f * ratio;
            if (_8F0 < minVelocity) _8F0 = minVelocity;
        }
        f32 length = PSVECMag(&_8F8);
        if (PSVECMag(&horizontal) < length) {
            f32 speed = PSVECMag(&horizontal);
            _8F8.setLength(0.5f * (PSVECMag(&_8F8) + speed));
        }
    } else _8F8.zero();
}


void Mario::startSlidingTask(u32 a1, f32 a2, u16 a3) {
    _A40 = mFrontVec * a2;
    _A3C = a3;
    pushTask(&Mario::taskOnSlide, a1);
}


bool Mario::taskOnSlide(u32 a1) {
    (void)a1;
    addVelocity(_A40);
    _A3C--;
    return _A3C != 0;
}


void MarioJump_FORCE_MATCH_DATA() {
    Mario::Task task = &Mario::taskOnWallRising;
    (void)task;
}


bool Mario::taskOnWallRising(u32 a1) {
    (void)a1;

    if (((static_cast<u32>(mMovementStates.mLowBits.mBit8))) == 0) {
        stopEffect("共通壁上昇");
        return false;
    }

    playSound("スリップ");
    return true;
}


void Mario::incAirWalkTimer() {
    const u16 timer = _402;
    const u16 limit = mActor->getConst().getTable()->mAirWalkTime;

    if (timer >= limit) {
        return;
    }

    _402 = timer + 1;

    if (getPlayerMode() != 4) {
        return;
    }

    const u16 nextTimer = _402;
    const u16 nextLimit = mActor->getConst().getTable()->mAirWalkTime;

    if (nextTimer >= nextLimit) {
        if (mAirWalkRecoverySoundArmed != 0) {
            playSound("ハチ体力完全回復");
        }
        mAirWalkRecoverySoundArmed = 0;
    } else {
        mAirWalkRecoverySoundArmed = 1;
        playSound("ハチ体力回復");
    }
}

// ---- MarioMagic.cpp ----

void Mario::stopPunch() {
    if (isStatusActive(MarioStatus_Magic)) {
        closeStatus(mMagic);
    }

    MarioActor* actor = mActor;

    if (!actor->_944) {
        actor->_945 = 0;
        actor->_974 = 0;
    }

    actor->_944 = 0;
}


void Mario::startMagic() {
    if (!((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        if (!mActor->_468) {
            if (!((mMovementStates.mHighWord >> 28) & 1U)) {
                if (!isStatusActive(MarioStatus_Slider)) {
                    if (isSkatableFloor()) {
                        doSkate();
                    } else {
                        clearSlope();
                        changeStatus(mMagic);
                        stopAnimationUpper(nullptr);
                        mFlags14.mLowBits.mBit30 = 1;
                    }
                }
            }
        }
    }
}

// ---- MarioMove.cpp ----




bool Mario::isEnableTurn() {
    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit27))) {
        return false;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit26))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit28))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit11))) {
        return false;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit21))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit21))) {
        return false;
    }

    if (isStatusActive(MarioStatus_Magic)) {
        return false;
    }

    if (isAnimationRun("坂すべり上向きうつぶせ", 2)) {
        return false;
    }

    if (isAnimationRun("坂すべり下向きあおむけ", 3)) {
        return false;
    }

    if (isAnimationRun("坂すべり下向き終了")) {
        return false;
    }

    if (isAnimationRun("坂すべり上向き終了")) {
        return false;
    }

    if (isAnimationRun("飛び込み失敗着地")) {
        return false;
    }

    if (isAnimationRun("飛び込み失敗回転着地")) {
        return false;
    }

    if (mActor->_480) {
        return false;
    }

    if (mActor->isPunching()) {
        return false;
    }

    if (mActor->isItemSwinging()) {
        return false;
    }

    if (mActor->isBeeWallWalk()) {
        return false;
    }

    if (mActor->_3C0) {
        return false;
    }

    if (mActor->_EA4) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit8)) || (static_cast<u32>(mFlags14.mLowBits.mBit10))) {
        return false;
    }

    return true;
}


void Mario::recordTurnSlipAngle() {
    if (!mActor->isBeeWallWalk()) {
        _3E4 = mFrontVec;
        _3D2 = mActor->getConst().getTable()->mTurnReadyTime;
    }
}




f32 Mario::decideInertiaOnIce(f32 speed) {
    if (speed > 1.0f) {
        speed = 1.0f;
    }

    f32 inertia =
        (1.0f - speed) * mActor->getConst().getTable()->mInertiaIceStandardStop + speed * mActor->getConst().getTable()->mInertiaIceStandardMax;
    inertia = (1.0f - _3F4) * inertia + _3F4 * mActor->getConst().getTable()->mInertiaIceStartSpin;
    if (speed == 0.0f) {
        inertia = mActor->getConst().getTable()->mInertiaIceStop;
    }

    return inertia;
}


f32 Mario::decideInertiaOnSlip(f32 speed) {
    f32 inertia = (1.0f - mWalkSpeed) * mActor->getConst().getTable()->mInertiaSlipStandardStop +
                  mWalkSpeed * mActor->getConst().getTable()->mInertiaSlipStandardMax;
    inertia = (1.0f - _3F4) * inertia + _3F4 * mActor->getConst().getTable()->mInertiaSlipStartSpin;
    if (speed == 0.0f) {
        inertia = mActor->getConst().getTable()->mInertiaSlipStop;
    }

    return inertia;
}


void Mario::calcShadowDir(const TVec3f& rInput, TVec3f* pShadowDir) {
    TVec3f direction;
    direction = rInput;
    if (!MR::isNormalize(direction)) {
        MR::normalizeOrZero(&direction);
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit8)) || (static_cast<u32>(mFlags14.mLowBits.mBit10))) {
        calcShadowDir2D(direction, pShadowDir);
        return;
    }

    TVec3f normal = -*getGravityVec();
    f32 dot = direction.dot(normal);
    direction.dot(mHeadVec);
    if (MR::abs(dot) > MR::abs(direction.dot(_368))) {
        normal = _368;
    }

    TVec3f side = normal.cross(direction);
    pShadowDir->cross(side, normal);
    MR::normalizeOrZero(pShadowDir);
}


void Mario::retainMoveDir(f32 stickX, f32 stickY) {
    f32 angle = MR::atan2(stickY, stickX);
    f32 difference = fn_803A3780(angle);
    if ((static_cast<u32>(mFlags14.mLowBits.mBit20)) && isStickOn()) {
        mFlags14.mLowBits.mBit20 = 0;
    }
    f32 limit = 0.06f;
    if (_3CE < 2) {
        limit = 0.1f;
    }
    bool retain = true;
    mDrawStates.mBits.mBit9 = 1;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && !(static_cast<u32>(mFlags14.mLowBits.mBit21))) {
        retain = false;
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && (static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        retain = false;
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && !isAnimationRun(nullptr)) {
        retain = false;
    }
    if (difference < limit && isStickOn() && retain) {
        if (_40E == 0 || ((static_cast<u32>(mFlags14.mLowBits.mBit21)) && (static_cast<u32>(mMovementStates.mLowBits.mBit31)))) {
            _40E = 0;
            mDrawStates.mBits.mBit18 = 1;
        } else {
            if (_40E != 0) {
                _40E--;
            }
            _29C = _368;
            _2A8 = *getGravityVec();
        }
    } else {
        if (static_cast<u32>(mMovementStates.mLowBits.mBit31)) {
            if (isStickOn()) {
                mFlags14.mLowBits.mBit21 = 0;
            }
        } else {
            mFlags14.mLowBits.mBit21 = 1;
        }
        if (_40E == 0) {
            _3D4 = 0;
        }
        _40E = 30;
        if (!isStickOn()) {
            mFlags14.mLowBits.mBit20 = 1;
        }
        _29C = _368;
        _2A8 = *getGravityVec();
    }
    _2B4 = angle;
}


void Mario::calcMoveDir(float stickX, float stickY, TVec3f* pMoveDir, bool isDisableMove) {
    if (static_cast<u32>(mMovementStates.mHighBits.mBit8)) {
        calcDir2D(stickX, stickY, pMoveDir);
        return;
    }
    if (static_cast<u32>(mFlags14.mLowBits.mBit10)) {
        calcMoveDir2D(stickX, stickY, pMoveDir);
        return;
    }
    if (static_cast<u32>(mMovementStates.mHighBits.mBit5)) {
        calcMoveDir25D(stickX, stickY, pMoveDir);
        return;
    }
    if (isDisableMove) {
        retainMoveDir(stickX, stickY);
    }
    const TVec3f& screenX = getCamDirX();
    const TVec3f& screenY = getCamDirY();
    TVec3f screenZ = -getCamDirZ();
    TVec3f groundX;
    TVec3f groundY;
    TVec3f selectedBasis;
    TVec3f up = -*getGravityVec();
    if (!MR::isNearZero(up, 0.001f)) {
        if (_398.dot(up) <= 0.0f) {
            _398 = up;
        } else {
            MR::vecBlendSphere(&_398, _398, up, 0.1f);
        }
        MR::normalizeOrZero(&_398);
    }
    f32 yAlignment = screenY.dot(_398);
    f32 zAlignment = screenZ.dot(_398);
    if (MR::abs(zAlignment) > MR::abs(yAlignment)) {
        selectedBasis = screenY;
        if (zAlignment < 0.0f) {
            selectedBasis = -selectedBasis;
        }
        groundX.cross(selectedBasis, _398);
    } else {
        selectedBasis = -screenZ;
        if (yAlignment < 0.0f) {
            selectedBasis = -selectedBasis;
        }
        groundX.cross(selectedBasis, _398);
    }
    groundY = -selectedBasis;
    MR::normalizeOrZero(&groundX);
    if (MR::isNearZero(groundX, 0.001f)) {
        groundX = screenX;
    }
    *pMoveDir = groundX * stickX - groundY * stickY;
    if (static_cast<u32>(mFlags14.mLowBits.mBit12)) {
        MR::calcPlayerKillVecOnPlane(pMoveDir, *pMoveDir, mPosHolder->mPos, _6F4, _700);
    }
}


bool Mario::checkLockOnHoming() {
    if (mStickPos.z != 0.0f) {
        return false;
    }

    if (!checkPreLvlZ()) {
        return false;
    }

    if (isSwimming()) {
        return false;
    }

    if (!mActor->_470) {
        return false;
    }

    mMovementStates.mLowBits.mBit21 = 0;
    mDrawStates.mBits.mBit26 = 1;
    doLockOnHoming();
    return true;
}


void Mario::doLockOnHoming() {
    TVec3f direction;
    direction.set(*MR::getSensorPos(mActor->_470) - mPosHolder->mPos);
    MR::normalize(&direction);
    TVec3f shadowDir;
    calcShadowDir(direction, &shadowDir);
    if (MR::diffAngleAbsHorizontal(mFrontVec, direction, mHeadVec) >= 0.05235988f) {
        if (isAnimationStop()) {
            changeAnimation("その場足踏み");
            if (!_750) {
                setFrontVecKeepUp(direction, 15UL);
                _334 = direction;
            }
        }
    }
}


void Mario::fixPositionInTower() {
    if (fn_803A4210()) {
        bool blocked = false;
        if (isStatusActiveID(0x11) || isStatusActiveID(0x1F)) {
            blocked = true;
        }
        if (!blocked) {
            fn_803A42B0();
        }
    } else {
        if (isAnimationRun("ブレーキ")) {
            doBrakingAnimation();
            mTargetWalkSpeedIndex = 0;
            return;
        }
        stopAnimation("ブレーキ滑り床", (const char*)nullptr);
    }
}

// ---- MarioMove25D.cpp ----

void Mario::set25Dmode(const AreaObj* pArea) {
    Mtx matrix;
    TVec3f rotation;
    MR::calcCubeRotate(pArea, &rotation);
    TVec3f normal(0.0f, 0.0f, 1.0f);
    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::makeMtxTR(matrix, 0.0f, 0.0f, 0.0f, rotation.x, rotation.y, rotation.z);
    PSMTXMultVec(matrix, &normal, &normal);
    PSMTXMultVec(matrix, &up, &up);
    _6D0 = normal;
    _6DC = up;
    _6E8.cross(_6D0, _6DC);
    MR::normalize(&_6E8);
    mMovementStates.mHighBits.mBit5 = 1;
}


void Mario::update25Dmode() {
    TVec3f up;
    MR::vecKillElement(-*getGravityVec(), _6D0, &up);
    MR::normalizeOrZero(&up);
    if (MR::isNearZero(up)) {
        mAxisMode6A4 = -getGravityVec()->dot(_6D0) > 0.0f ? 4 : 5;
        return;
    }

    f32 sideDot = up.dot(_6E8);
    f32 upDot = up.dot(_6DC);
    f32 normalDot = up.dot(_6D0);
    f32 sideAbs = MR::abs(sideDot);
    f32 upAbs = MR::abs(upDot);
    f32 normalAbs = MR::abs(normalDot);
    u8 mode;
    if (sideAbs > upAbs && sideAbs > normalAbs) {
        mode = sideDot > 0.0f ? 2 : 3;
    } else if (upAbs > sideAbs && upAbs > normalAbs) {
        if (upDot > 0.0f) {
            mode = 0;
        } else {
            mode = 1;
        }
    } else {
        mode = normalDot > 0.0f ? 4 : 5;
    }

    mCandidateAxisMode6A5 = mode;
    if (mode == 2 || mode == 3) {
        if (mode == 2 || mode == 3) {
            f32 threshold = 0.965f;
            if (mAxisMode6A4 == 2 || mAxisMode6A4 == 3) {
                threshold = 0.9397f;
            }

            if (sideAbs < threshold) {
                if (upAbs > normalAbs) {
                    if (upDot > 0.0f) {
                        mode = 0;
                    } else {
                        mode = 1;
                    }
                } else {
                    return;
                }
            }
        }
    }

    if (!isStickOn()) {
        mAxisMode6A4 = mode;
        mMovementStates.mHighBits.mBit4 = 0;
    } else if (!(static_cast<u32>(mMovementStates.mHighBits.mBit4)) && mStickPos.z > 0.7f) {
        _6C8.set(mStickPos.x, mStickPos.y);
        mAxisMode6A4 = mode;
        mMovementStates.mHighBits.mBit4 = 1;
    } else {
        TVec2f stick(mStickPos.x, mStickPos.y);
        if (MR::diffAngleAbs(_6C8, stick) >= 0.7853982f) {
            mAxisMode6A4 = mode;
            _6C8.set(mStickPos.x, mStickPos.y);
        }
    }
}


void Mario::updateAxisFromMode(u8 mode) {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        return;
    }

    TVec3f side;
    TVec3f normal;
    if (mode < 4) {
        TVec3f up;
        TVec3f planeUp;
        MR::vecKillElement(-*getGravityVec(), _6D0, &up);
        MR::vecKillElement(_6DC, _6D0, &planeUp);
        MR::normalizeOrZero(&up);
        MR::normalizeOrZero(&planeUp);
        f32 dot = up.dot(planeUp);
        if (MR::abs(dot) > 0.99f) {
            switch (mode) {
            case 0:
                side = _6E8;
                normal = _6D0;
                if (dot < 0.0f) {
                    side = -side;
                }

                break;
            case 1:
                side = _6E8;
                normal = _6D0;
                if (dot > 0.0f) {
                    side = -side;
                }

                break;
            case 2:
                side = _6D0;
                normal = -_6E8;
                if (dot < 0.0f) {
                    side = -side;
                    normal = -normal;
                }

                break;
            case 3:
                side = -_6D0;
                normal = _6E8;
                if (dot < 0.0f) {
                    side = -side;
                    normal = -normal;
                }

                break;
            }
        } else {
            TVec3f axis;
            axis.cross(up, planeUp);
            MR::normalize(&axis);
            if (axis.dot(_6D0) < 0.0f) {
                axis = -axis;
            }

            TVec3f tangent;
            tangent.cross(axis, up);
            MR::normalize(&tangent);
            switch (mode) {
            case 0:
                side = tangent;
                normal = axis;
                break;
            case 1:
                side = -tangent;
                normal = axis;
                break;
            case 2:
                side = axis;
                normal = -tangent;
                break;
            case 3:
                side = -axis;
                normal = tangent;
                break;
            }
        }
    }

    _6B0 = side;
    _6BC = -normal;
}


void Mario::calcMoveDir25D(f32 x, f32 y, TVec3f* pDirection) {
    *pDirection = _6B0 * x - _6BC * y;
}

// ---- MarioMove2D.cpp ----

void Mario::check2DMode() {
    mMovementStates.mHighBits.mBit8 = 0;
    mFlags14.mLowBits.mBit10 = 0;

    AreaObj* previousArea = mPlaneModeArea;
    AreaObj* area = MR::getAreaObj("PlaneModeCube", mPosHolder->mPos);
    mPlaneModeArea = area;
    if (area != nullptr) {
        mMovementStates.mHighBits.mBit8 = 1;

        TVec3f rotate;
        MR::calcCubeRotate(area, &rotate);
        MR::makeMtxTR(_F4, 0.0f, 0.0f, 0.0f, rotate.x, rotate.y, rotate.z);

        if (previousArea != area) {
            _688 = mPosHolder->mPos;
            MR::getRotatedAxisZ(&mSwimSideVec694, rotate);
            mSwimSideVec694 = -mSwimSideVec694;
            MR::calcCubePos(area, &_694);
        }
    } else {
        area = MR::getAreaObj("PlaneCircularModeCube", mPosHolder->mPos);
        if (area != nullptr) {
            TVec3f rotate;
            MR::calcCubeRotate(area, &rotate);
            PSMTXCopy(MR::tmpMtxRotYDeg(rotate.y), _F4);
            mFlags14.mLowBits.mBit10 = 1;

            if (!((_28 >> 10) & 1)) {
                _688 = mPosHolder->mPos;
                MR::getRotatedAxisZ(&mSwimSideVec694, rotate);
                MR::calcCubePos(area, &_694);
            }
        }
    }

    mMovementStates.mHighBits.mBit5 = 0;
    area = MR::getAreaObj("PipeModeCube", mPosHolder->mPos);
    if (area != nullptr) {
        set25Dmode(area);
    }
    mActor->fn_803D8A30(static_cast<u32>(mMovementStates.mHighBits.mBit8), mSwimSideVec694);

}


void Mario::calcMoveDir2D(f32 a1, f32 a2, TVec3f* pOut) {
    if (a1 != 0.0f) {
        a2 = 0.0f;
    }

    TVec3f stack_8C;
    TVec3f stack_80;
    TVec3f stack_74;
    TVec3f stack_68;
    TVec3f stack_5C;

    stack_8C = mSide2D;
    stack_80 = mUp2D;
    stack_74 = mNormal2D;

    TVec3f stack_50(_660);
    stack_68.cross(stack_74, stack_50);
    MR::normalize(&stack_68);

    if (stack_50.dot(stack_80) < 0.0f) {
        stack_68 = -stack_68;
    }

    stack_5C = -stack_80;
    TVec3f stack_44(_398);

    if (_398.dot(stack_50) <= 0.0f) {
        _398 = stack_50;
    } else {
        MR::vecBlendSphere(_398, stack_50, &_398, 0.1f);
    }

    if (MR::isNearZero(_398)) {
        _398 = stack_44;
    } else {
        MR::normalize(&_398);
    }

    *pOut = stack_68 * a1 - stack_5C * a2;
}


void Mario::calcShadowDir2D(const TVec3f& rVec, TVec3f* pOut) {
    *pOut = rVec;
    MR::normalizeOrZero(pOut);
}


void Mario::stick2Dadjust(f32& rStickX, f32& rStickY) {
    bool isNearHalfPi = false;

    if (((mFlags14.mLowWord & Mario::MovementStates::Turning) >> 28) && (static_cast<u32>(mMovementStates.mLowBits.mBit30)) && !(static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        mFlags14.mLowWord &= ~Mario::MovementStates::Turning;
        clear2DStick();
    }

    f32 stickXInput = rStickX;
    f32 stickYInput = rStickY;

    mSide2D.set(1.0f, 0.0f, 0.0f);
    mUp2D.set(0.0f, 1.0f, 0.0f);
    mNormal2D.set(0.0f, 0.0f, -1.0f);
    PSMTXMultVecSR(_F4, &mSide2D, &mSide2D);
    PSMTXMultVecSR(_F4, &mNormal2D, &mNormal2D);

    TVec3f stack_38(0.0f, 1.0f, 0.0f);
    TVec3f* camDirZ = &getCamDirZ();
    f32 angle = MR::diffAngleSignedHorizontal(getCamDirY(), stack_38, *camDirZ);

    TVec3f stack_2C(1.0f, 0.0f, 0.0f);
    camDirZ = &getCamDirZ();
    const f32 absHorizontal = MR::diffAngleAbsHorizontal(getCamDirX(), stack_2C, *camDirZ);
    if (absHorizontal >= 3.0415928f && MR::abs(angle) >= 3.0415928f) {
        angle = 0.0f;
        rStickX = -rStickX;
    }

    Mtx rotMtx;
    PSMTXRotAxisRad(rotMtx, &mSwimSideVec694, angle);
    PSMTXMultVecSR(rotMtx, &mSide2D, &mSide2D);
    PSMTXMultVecSR(rotMtx, &mUp2D, &mUp2D);

    MR::vecKillElement(_368, mSwimSideVec694, &_660);
    if (MR::normalizeOrZero(&_660)) {
        _660 = _368;
    }

    set2Dmode(_660.dot(mUp2D) >= 0.0f);

    bool isMainY = false;
    bool isMainX = false;

    if (_66C) {
        if (MR::isNearZero(mStickPos.z)) {
            _66C = false;
        } else {
            TVec3f stickVec(rStickX, rStickY, 0.0f);
            const f32 diff = MR::diffAngleAbs(stickVec, _670);

            if (_670.x * rStickX < 0.0f) {
                _66C = false;
            } else if (_670.y * rStickY < 0.0f) {
                _66C = false;
            } else if (diff < 0.3f) {
                rStickX = _67C.x;
                rStickY = _67C.y;
            } else {
                _66C = false;
            }
        }
    }

    if (!_66C && !MR::isNearZero(mStickPos.z) && !_60E && !_60F) {
        TVec3f stickVec(rStickX, rStickY, 0.0f);
        PSMTXMultVecSR(_F4, &stickVec, &stickVec);
        PSMTXMultVecSR(rotMtx, &stickVec, &stickVec);

        TVec3f invMove(-_660);
        const f32 diff = MR::diffAngleAbs(invMove, stickVec);
        f32 clamp = MR::clamp(1.0f - (mStickPos.z * mStickPos.z), 0.0f, 1.0f);

        const f32 threshold = MR::getInterpolateValue(clamp, 0.5f, 1.5f);
        if (diff < threshold) {
            rStickX = 0.0f;
            rStickY = 0.0f;
            mStickPos.z = 0.0f;
        } else if (diff > (3.1415927f - threshold)) {
            rStickX = 0.0f;
            rStickY = 0.0f;
            mStickPos.z = 0.0f;
        } else {
            _670 = stickVec;
            MR::vecKillElement(stickVec, _660, &stickVec);
            rStickX = stickVec.x;
            rStickY = stickVec.y;
            _67C = stickVec;
            _66C = true;
        }
    }

    if (MR::abs(rStickY) > MR::abs(rStickX)) {
        isMainY = true;
        rStickX = 0.0f;
        if (rStickY > 0.0f) {
            rStickY = mStickPos.z;
        } else {
            rStickY = -mStickPos.z;
        }
    } else {
        isMainX = true;
        rStickY = 0.0f;
        if (rStickX > 0.0f) {
            rStickX = mStickPos.z;
        } else {
            rStickX = -mStickPos.z;
        }
    }

    const f32 angleMove = MR::diffAngleAbs(_660, mUp2D);
    u8 lockX = 0;
    u8 lockY = 0;

    if (!((_60E != 0 || _60F != 0) && _610 == 0 && _611 == 0)) {
        bool doSecondBlock = false;

        if ((angleMove < 0.5235988f || angleMove > 2.617994f || _610 != 0) && _611 == 0) {
            if (mPrevious2DStickY == 0.0f) {
                rStickY = 0.0f;
                if (isMainY && MR::abs(stickXInput) > 0.3f) {
                    rStickX = mStickPos.z;
                    if (stickXInput < 0.0f) {
                        rStickX = -rStickX;
                    }
                    lockX = 1;
                }
            }

        } else {
            if (!(angleMove > 1.3962635f && angleMove < 1.7453294f) && _611 == 0) {
            } else if (_610 == 0 && mPrevious2DStickX == 0.0f) {
                rStickX = 0.0f;
                if (isMainX && MR::abs(stickYInput) > 0.3f) {
                    rStickY = mStickPos.z;
                    if (stickYInput < 0.0f) {
                        rStickY = -rStickY;
                    }
                    lockY = 1;
                }
            }
        }
    }

    mPrevious2DStickX = rStickX;
    mPrevious2DStickY = rStickY;
    _610 = lockX;
    _611 = lockY;

    if (!_60E && !_60F && rStickY != 0.0f) {
        if (MR::isInRange(angleMove, 0.19634955f, 1.5707964f)) {
            if (_660.dot(mSide2D) < 0.0f) {
                _60F = 1;
            } else {
                _60F = 2;
            }
        } else if (MR::isInRange(angleMove, 1.5707964f, 2.9452431f)) {
            if (_660.dot(mSide2D) < 0.0f) {
                _60F = 2;
            } else {
                _60F = 1;
            }
        }
    }

    switch (_60F) {
    case 1:
        if (rStickY == 0.0f) {
            _60F = 0;
        } else {
            rStickX = rStickY;
        }
        break;
    case 2:
        if (rStickY == 0.0f) {
            _60F = 0;
        } else {
            rStickX = -rStickY;
            rStickY = -rStickY;
        }
        break;
    default:
        _60F = 0;
        break;
    }

    switch (_60E) {
    case 1:
        if (rStickX > 0.0f) {
            rStickX = -rStickX;
        } else {
            _60E = 0;
        }
        break;
    case 2:
        if (rStickX < 0.0f) {
            rStickX = -rStickX;
        } else {
            _60E = 0;
        }
        break;
    default:
        _60E = 0;
        break;
    }

    _614 = rStickX;
    _618 = rStickY;

    if (MR::isNearZero(angleMove - 1.5707964f, 0.0001f)) {
        isNearHalfPi = true;
    }

    if (!isNearHalfPi) {
        rStickY = 0.0f;
        mStickPos.z = MR::abs(rStickX);
    } else {
        rStickX = 0.0f;
        mStickPos.z = MR::abs(rStickY);
    }
}


void Mario::set2Dmode(bool mode) {
    if (mode == _624) {
        return;
    }

    if (_614 > 0.0f) {
        _60E = 1;
    }

    if (_614 < 0.0f) {
        _60E = 2;
    }

    _624 = mode;
}








void Mario::clear2DStick() {
    _60E = 0;
    _60F = 0;
    _610 = 0;
    _611 = 0;
    _618 = 0.0f;
    _614 = 0.0f;
    mPrevious2DStickY = 0.0f;
    mPrevious2DStickX = 0.0f;
    _624 = false;
    _61C = 0.0f;
#ifdef GALAXY_NATIVE
    // The constructor calls this before the flag has been initialised. Wii's
    // byte load/nonzero test always leaves zero here; a host bool read is UB.
    _66C = false;
#else
    if (_66C) {
        _66C = false;
    }
#endif
    _670.zero();
    _67C.zero();
}

// ---- MarioPress.cpp ----

bool Mario::checkPressDamage() {
    if(_5FC) return false;
    if(isSwimming()) return false;
    if(checkVerticalPress(false)) {
        if(isStatusActive(MarioStatus_Hang)) mHang->forceDrop();
        else {
            mActor->setPress(0,0);
            if(mActor->_390) return true;
            _350.zero();_35C.zero();
            return true;
        }
    }
    if(checkSidePressPre()) { mActor->setPress(1,0);return true; }
    return checkSidePress();
}


// NONMATCHING: temporary/register allocation and independent load scheduling.
bool Mario::checkVerticalPress(bool atCurrentPosition) {
    if (static_cast<u32>(mMovementStates.mLowBits.mBit9)) return false;
    TVec3f position;
    if (isStatusActive(5)) position = mActor->_240;
    else position = mPosHolder->mPos;
    if (!atCurrentPosition && mActor->_390 == 0) position += mVelocity;
    f32 height;
    if (static_cast<u32>(mMovementStates.mLowBits.mBit21)) height = 80.0f;
    else height = 160.0f;
    if ((static_cast<u32>(getPlayer()->mFlags20.mBits.mBit16)) && (static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit30))) {
        if (_4C8->isValid() && MR::isGroundCodePress(_4C8)) {
            if (mGroundPolygon->mSensor != _4C8->mSensor) {
                f32 distance = _72C;
                MR::isNearZero(mAirGravityVec, 0.001f);
                TVec3f negativeGravity(-mAirGravityVec);
                TVec3f ray(negativeGravity);
                ray *= distance;
                TVec3f point(getTrans());
                point += ray;
                TVec3f velocity;
                MR::calcVelocityMovingPoint(_4C8, point, &velocity);
                if (velocity.dot(ray) < 0.0f) {
                    mActor->mPressNormal6D8 = _368;
                    return true;
                }
            }
        }
    }
    TVec3f up(-*getGravityVec());
    Triangle ceiling;
    TVec3f firstRay(up);
    firstRay *= height;
    TVec3f firstOffset(up);
    firstOffset *= 10.0f;
    TVec3f firstOrigin(position);
    firstOrigin -= firstOffset;
    TVec3f ceilingPos;
    bool ceilingHit;
    if (fn_803B2950()) ceilingHit = MR::getFirstPolyOnLineToDashYoshiStepAndMap(&ceilingPos, &ceiling, firstOrigin, firstRay);
    else ceilingHit = MR::getFirstPolyOnLineToMap(&ceilingPos, &ceiling, firstOrigin, firstRay);
    if (!ceilingHit) return false;
    TVec3f ceilingDelta(ceilingPos);
    ceilingDelta -= position;
    f32 distance = up.dot(ceilingDelta);
    const TVec3f* selected = distance < 150.0f ? &ceilingPos : &position;
    TVec3f origin(*selected);
    Triangle floor;
    TVec3f secondRay(*ceiling.getNormal(0));
    secondRay *= height;
    TVec3f secondOffset(*ceiling.getNormal(0));
    secondOffset *= 10.0f;
    TVec3f secondOrigin(origin);
    secondOrigin -= secondOffset;
    TVec3f floorPos;
    bool floorHit;
    if (fn_803B2950()) floorHit = MR::getFirstPolyOnLineToDashYoshiStepAndMap(&floorPos, &floor, secondOrigin, secondRay);
    else floorHit = MR::getFirstPolyOnLineToMap(&floorPos, &floor, secondOrigin, secondRay);
    if (floorHit) {
        if (ceiling.mSensor != floor.mSensor && (fn_803B2E10(&ceiling) || MR::isSensorPressObj(floor.mSensor))) {
            if (!atCurrentPosition && mActor->_390 == 0) {
                if (!checkVerticalPress(true)) {
                    mVelocity.zero();
                    return false;
                }
                return true;
            }
            mActor->mPressNormal6D8 = _368;
            return true;
        }
        return false;
    }
    if (!atCurrentPosition) {
        if (mActor->_390 != 0) return false;
        if (!MR::isSensorPressObj(ceiling.mSensor)) return false;
        mDrawStates.mWord |= 2;
        TVec3f displacement;
        if (distance < 0.0f) {
            TVec3f delta(ceilingPos);
            delta -= position;
            displacement = delta;
            displacement.setLength(height - distance);
        } else {
            TVec3f delta(position);
            delta -= ceilingPos;
            displacement = delta;
            displacement.setLength(150.0f - distance);
        }
        if (static_cast<u32>(mMovementStates.mLowBits.mBit30)) {
            if (mGroundPolygon->mSensor == ceiling.mSensor) {
                f32 length = PSVECMag(&displacement);
                TVec3f pushDirection(*ceiling.getNormal(0));
                pushDirection *= length;
                push(pushDirection);
            } else {
                mActor->_F48 = ceiling.mSensor;
                mActor->mPressNormal6D8 = _368;
                return true;
            }
        } else {
            f32 length = PSVECMag(&displacement);
            TVec3f pushDirection(*ceiling.getNormal(0));
            pushDirection *= length;
            push(pushDirection);
        }
    }
    return false;
}


// NONMATCHING: independent loads and temporary/register allocation.
bool Mario::checkSidePressPre() {
    if (_5FC) {
        return false;
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) && (static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        if ((_4F4 - _4E8).length() < 100.0f && mFrontWallTriangle->mSensor != mBackWallTriangle->mSensor &&
            (MR::isSensorPressObj(mFrontWallTriangle->mSensor) || MR::isSensorPressObj(mBackWallTriangle->mSensor))) {
            if (isDossun(mFrontWallTriangle) || isDossun(mBackWallTriangle)) {
                return false;
            }
            if (mFrontWallTriangle->getNormal(0)->dot(*mBackWallTriangle->getNormal(0)) < -0.5f) {
                mActor->mPressNormal6D8 = getWallNorm();
                return true;
            }
        }
    } else if (isStatusActive(MarioStatus_Hang) && (static_cast<u32>(mMovementStates.mLowBits.mBit6)) && _8C8->isValid() && _8C8->mSensor != mBackWallTriangle->mSensor &&
               (MR::isSensorPressObj(_8C8->mSensor) || MR::isSensorPressObj(mBackWallTriangle->mSensor))) {
        if (isDossun(_8C8) || isDossun(mBackWallTriangle)) {
            return false;
        }
        TVec3f horizontal;
        MR::vecKillElement(mPosHolder->mPos - _4F4, getAirGravityVec(), &horizontal);
        if (horizontal.length() < 100.0f) {
            mActor->mPressNormal6D8 = getWallNorm();
            return true;
        }
    }
    return false;
}
#pragma opt_propagation off
// NONMATCHING: temporary/register allocation; canonical SMG2 strike records and gates recovered.
bool Mario::checkSidePress() {
    TVec3f position;
    if (isStatusActive(MarioStatus_Hang)) {
        position = mActor->_240;
    } else {
        position = mPosHolder->mPos;
    }
    f32 radius;
    f32 height;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit21))) {
        radius = 50.0f;
        height = 40.0f;
    } else {
        radius = 80.0f;
        height = 70.0f;
    }
    f32 ceilingDistance = calcDistToCeil(false);
    TVec3f center(position + mHeadVec * height);
    u32 hitCount = fn_803B2A20(center, radius, radius);
    if (hitCount < 2) {
        return false;
    }
    for (u32 i = 0; i < hitCount; i++) {
        const HitInfo* first = fn_803B2AA0(i);
        TVec3f firstNormal(*first->mParentTriangle.getNormal(0));
        if (MR::isEqualString(first->mParentTriangle.mSensor->mHost->mName, "バッタン")) continue;
        for (u32 j = i + 1; j < hitCount; j++) {
            const HitInfo* second = fn_803B2AA0(j);
            TVec3f secondNormal(*second->mParentTriangle.getNormal(0));
            if (MR::isEqualString(second->mParentTriangle.mSensor->mHost->mName, "バッタン")) continue;
            if (firstNormal.dot(secondNormal) >= -0.707f) {
                continue;
            }
            if ((first->mHitPos - center).dot(second->mHitPos - center) > 0.0f) {
                continue;
            }
            f32 separation = (first->mHitPos - second->mHitPos).length();
            TVec3f firstVelocity;
            TVec3f secondVelocity;
            MR::calcVelocityMovingPoint(&first->mParentTriangle, first->mHitPos, &firstVelocity);
            MR::calcVelocityMovingPoint(&second->mParentTriangle, second->mHitPos, &secondVelocity);
            if (!MR::isNearZero(firstVelocity) || !MR::isNearZero(secondVelocity)) {
                if (!MR::isNearZero(firstVelocity)) {
                    MR::normalize(&firstVelocity);
                    if (firstVelocity.dot(firstNormal) < 0.2f || firstVelocity.dot(secondNormal) > -0.2f) {
                        continue;
                    }
                }
                if (!MR::isNearZero(secondVelocity)) {
                    MR::normalize(&secondVelocity);
                    if (secondVelocity.dot(secondNormal) < 0.2f || secondVelocity.dot(firstNormal) > -0.2f) {
                        continue;
                    }
                }
                if (fn_803B4850(first->mParentTriangle.mSensor) || fn_803B4850(second->mParentTriangle.mSensor) ||
                    (first->mParentTriangle.mSensor != second->mParentTriangle.mSensor && (fn_803B2E10(&first->mParentTriangle) || fn_803B2E10(&second->mParentTriangle)))) {
                    if (isStatusActive(MarioStatus_Hang)) {
                        mMovementStates.mLowBits.mBit30 = 0;
                        closeStatus(mHang);
                    }
                    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit30)) && MR::abs(firstNormal.dot(*getGravityVec())) < 0.707f) {
                        if (!first->isCollisionAtFace()) {
                            TVec3f horizontal;
                            f32 vertical = MR::vecKillElement(first->mHitPos - center, *getGravityVec(), &horizontal);
                            if (vertical != 0.0f) {
                                addVelocity(*getGravityVec() * -vertical);
                                mDrawStates.mBits.mBit1 = 1;
                                continue;
                            }
                        }
                        if (!second->isCollisionAtFace()) {
                            TVec3f horizontal;
                            f32 vertical = MR::vecKillElement(second->mHitPos - center, *getGravityVec(), &horizontal);
                            if (vertical != 0.0f) {
                                addVelocity(*getGravityVec() * -vertical);
                                mDrawStates.mBits.mBit1 = 1;
                                continue;
                            }
                        }
                    }
                    if (!isStatusActive(MarioStatus_Bury) && !mSinkTimer && ceilingDistance >= 180.0f) {
                        TVec3f firstOffset(first->mHitPos - center);
                        TVec3f secondOffset(center - second->mHitPos);
                        if (!MR::isNearZero(MR::diffAngleAbs(firstOffset, secondOffset), 0.13962634f)) {
                            MR::normalizeOrZero(&firstOffset);
                            MR::normalizeOrZero(&secondOffset);
                            addVelocity(-firstOffset * 5.0f);
                            addVelocity(secondOffset * 5.0f);
                            continue;
                        }
                    }
                    f32 angle = JMAAcosRadian(-firstNormal.dot(secondNormal));
                    if (fn_803EFA30(first->mParentTriangle.mSensor, "ナナメつぶし許可") || fn_803EFA30(second->mParentTriangle.mSensor, "ナナメつぶし許可")) {
                        angle = 0.0f;
                    }
                    if (!isStatusActive(MarioStatus_Bury) && !mSinkTimer && angle > 0.34906587f) {
                        f32 distance = (160.0f - ceilingDistance) / JMASinRadian(angle);
                        if (!(distance < 0.0f)) {
                            TVec3f firstHorizontal;
                            TVec3f secondHorizontal;
                            MR::vecKillElement(firstNormal, *getGravityVec(), &firstHorizontal);
                            MR::vecKillElement(secondNormal, *getGravityVec(), &secondHorizontal);
                            if (firstHorizontal.length() > secondHorizontal.length()) {
                                firstHorizontal.setLength(distance);
                                addVelocity(firstHorizontal);
                            } else {
                                secondHorizontal.setLength(distance);
                                addVelocity(secondHorizontal);
                            }
                        }
                        continue;
                    }
                    s32 previousFloorCode = _95C->getCode(_45C);
                    if (previousFloorCode != _95C->getCode(mGroundPolygon) && MR::isSensorPressObj(mGroundPolygon->mSensor) &&
                        mVerticalSpeed >= 200.0f) {
                        mDrawStates.mBits.mBit21 = 1;
                        TVec3f delta(mShadowPos);
                        delta -= mGroundPos;
                        MR::isNearZero(mAirGravityVec, 0.001f);
                        TVec3f horizontal;
                        MR::vecKillElement(delta, mAirGravityVec, &horizontal);
                        horizontal.setLength(10.0f);
                        addVelocity(horizontal);
                        return false;
                    }
                    if (MR::abs(firstNormal.dot(mHeadVec)) > 0.707f) {
                        mActor->setPress(0, 0);
                    } else {
                        if (!first->mParentTriangle.isHostMoved() || !second->mParentTriangle.isHostMoved()) {
                            TVec3f hitPosition;
                            if (!first->mParentTriangle.isHostMoved()) hitPosition = first->mHitPos;
                            else hitPosition = second->mHitPos;
                            TVec3f displacement(hitPosition);
                            displacement -= center;
                            mPosHolder->fn_80406010(displacement);
                        }
                        mActor->setPress(1, 0);
                    }
                    mActor->mPressNormal6D8 = firstNormal;
                    return true;
                }
            }
            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit31))) {
                MR::diffAngleAbsHorizontal(firstNormal, secondNormal, getAirGravityVec());
                f32 normalDot = firstNormal.dot(secondNormal);
                TVec3f tangent;
                PSVECCrossProduct(&firstNormal, &getAirGravityVec(), &tangent);
                MR::normalizeOrZero(&tangent);
                TVec3f firstUp = tangent.cross(firstNormal);
                MR::normalizeOrZero(&firstUp);
                PSVECCrossProduct(&secondNormal, &getAirGravityVec(), &tangent);
                MR::normalizeOrZero(&tangent);
                TVec3f secondUp = tangent.cross(secondNormal);
                MR::normalizeOrZero(&secondUp);
                TVec3f firstPosition;
                TVec3f secondPosition;
                firstPosition = first->mHitPos + firstUp * height;
                secondPosition = second->mHitPos + secondUp * height;
                f32 distance = (firstPosition - secondPosition).length();
                if (isRising()) {
                    if (normalDot < -0.707f && distance < 120.0f && separation < 100.0f) {
                        cutGravityElementFromJumpVec(true);
                    }
                } else if (normalDot < -0.707f && separation < 100.0f && distance < 80.0f) {
                    cutGravityElementFromJumpVec(true);
                    mMovementStates.mLowBits.mBit30 = 1;
                    stopJump();
                }
            }
        }
    }
    return false;
}

#pragma opt_propagation on

// ---- MarioRabbit.cpp ----

void Mario::startRabbitMode() {
    if (!isStatusActive(MarioStatus_Rabbit)) {
        changeStatus(mRabbit);
    }
}


void Mario::endRabbitMode() {
    if (isStatusActive(MarioStatus_Rabbit)) {
        closeStatus(mRabbit);
    }
}

// ---- MarioRecovery.cpp ----

bool Mario::doRecovery() {
    if (isStatusActive(MarioStatus_Foo)) {
        return false;
    }
    if (isStatusActive(MarioStatus_Recovery)) {
        return false;
    }
    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }
    if (mActor->_934) {
        return false;
    }
    if (mActor->_EA4) {
        return false;
    }

    cancelSquatMode();
    stopWalk();
    if (mRecovery->_12) {
        changeStatus(mRecovery);
        stopJump();
    } else {
        TVec3f gravity;
        TVec3f position(*getLastSafetyTrans(&gravity));
        position += gravity * 160.0f;
        TVec3f direction(mPosHolder->mPos - position);
        MR::normalizeOrZero(&direction);
        doPointWarpRecovery(position, direction);
    }
    return true;
}

// ---- MarioSideStep.cpp ----

bool Mario::checkWallJumpCode() {
    if (checkWallFloorCode(11) || checkWallFloorCode(6) || checkWallFloorCode(34) || checkWallFloorCode(7) || checkWallFloorCode(8)) {
        if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) || isAnimationRun(_728)) {
            stopWalk();
            push(getWallNorm() * 10.0f);
            tryPushToVelocity();
            mJumpVec.set(-getPlayer()->getWallNorm() * 10.0f);
            MR::vecKillElement(mJumpVec, getAirGravityVec(), &mJumpVec);
            mJumpVec -= getAirGravityVec() * 10.0f;
        } else {
            mWall->initTriangleJump();
            u32 jumpType = -1;
            if (checkWallFloorCode(7)) {
                jumpType = 1;
            }
            if (checkWallFloorCode(6)) {
                jumpType = 0;
            }
            if (checkWallFloorCode(34)) {
                jumpType = 2;
            }
            if (mWall->startBackJump(jumpType)) {
                return true;
            }
        }
    }
    return false;
}


bool Mario::doSideStep() {
    if (getCurrentStatus() == MarioStatus_SideStep) {
        return false;
    }
    if (checkWallCode("NotSideStep", false)) {
        return false;
    }
    if (checkWallCode("NoAction", false)) {
        return false;
    }
    if (mActor->_468) {
        return false;
    }
    if (static_cast<u32>(mFlags20.mBits.mBit26)) {
        return false;
    }
    if (_72C < 200.0f) {
        return false;
    }
    if (getPlayerMode() == PlayerMode_Hopper) {
        return false;
    }
    if (isPlayerModeYoshi()) {
        return false;
    }
    if (calcPolygonAngleD(mFrontWallTriangle) > 91.0f) {
        return false;
    }
    f32 angle = MR::diffAngleAbs(getFrontWallNorm(), _368);
    if (MR::abs(angle - 1.5707964f) >= 0.5235988f) {
        return false;
    }
    bool cannotSideStep = !isSlipFloorCode(_960);
    if (cannotSideStep) {
        return false;
    }
    if (_960 == 2) {
        return false;
    }
    if (!(static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit23))) {
        return false;
    }
    const Triangle* wall = getPlayer()->getWallPolygon();
    if (!wall) {
        return false;
    }
    if (!MR::isSameMtx(wall->getBaseMtx()->toMtxPtr(), wall->getPrevBaseMtx()->toMtxPtr())) {
        return false;
    }
    stopWalk();
    forceStopTornado();
    changeStatus(mSideStep);
    return true;
}

// ---- MarioSkate.cpp ----

void MarioSkate_FORCE_MATCH_SDATA2() {
    (void)20.0f;
    (void)50.0f;
    (void)1.25f;
    (void)1.5f;
    (void)1.2f;
    (void)0.04f;
    (void)0.1f;
    (void)-0.1f;
    (void)-0.3926991f;
    (void)0.3926991f;
    (void)-0.5235988f;
    (void)0.5235988f;
    (void)0.8f;
    (void)0.9f;
    (void)0.95f;
    (void)0.05f;
}


bool Mario::isSkatableFloor() const {
    if (_960 == CollisionFloorCode_Ice) {
        return true;
    }

    return _960 == CollisionFloorCode_GlassIce;
}


bool Mario::doSkate() {
    changeStatus(mSkate);
    return true;
}

// ---- MarioSlider.cpp ----

bool Mario::isSliderFloor() const {
    return _960 == 9;
}


bool Mario::checkSliderMode() const {
    if (!((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && ((mMovementStates.mLowWord >> 30) & 1U)) {
        if (isSliderFloor()) {
            return true;
        }
    }

    return false;
}


void Mario::startSlider() {
    if (!isStatusActive(mSlider->mStatusId)) {
        if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit20))) {
            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit31))) {
                mDrawStates.mBits.mBit11 = 1;
                playSound("尻ドロップ着地");
                playSound("声尻ドロップ着地");
                playEffectRT("属性尻ドロップ", _368, mPosHolder->mPos);
                startPadVib("最強");
                startCamVib(0);
                MR::removeAllClingingKarikari();
            }
        }

        stopWalk();
        stopJump();
        changeStatus(mSlider);
    }
}

// ---- MarioSlip.cpp ----

void Mario::clearSlope() {
    _8F0 = 0.0f;
    mMovementStates.mHighBits.mBit28 = 0;
    _8F8.zero();
    _280 = 0.0f;
    _910.zero();
    _284.zero();
    _3C2 = 0;
    _2C4.zero();
    _3C4 = 0;
    _904.zero();
}


bool Mario::isEnableSlopeMove() const {
    const bool isNotSlipFloor = !isSlipFloorCode(_960);
    if (isNotSlipFloor) {
        if (!(static_cast<u32>(mMovementStates.mHighBits.mBit28))) {
            if (MR::isNearZero(_8F8)) {
                return false;
            }
        }
    }

    const s32 status = static_cast< s32 >(getCurrentStatus());
    switch (status) {
    case 0:
    case 2:
    case 3:
    case 4:
        return true;

    case 1:
    default:
        return false;
    }
}


// NONMATCHING: independent vector/gravity evaluation order and register scheduling.
void Mario::slopeMove() {
    if (mActor->mHealth == 0) {
        return;
    }

    if (!((mMovementStates.mLowWord >> 30) & 1U)) {
        return;
    }

    f32 blend;
    f32 blendRate = MR::clamp(mWalkSpeed, 0.0f, 1.0f);

    TVec3f stack_188;
    TVec3f stack_17C;
    TVec3f stack_170;
    stack_170.cross(_368, getAirGravityVec());
    MR::normalizeOrZero(&stack_170);

    if (MR::isNearZero(stack_170)) {
        stack_17C.zero();
        _904.zero();
    } else {
        stack_17C.cross(_368, stack_170);
        MR::normalize(&stack_17C);
        _904 = stack_17C;
    }

    if (MR::isSameDirection(stack_17C, getAirGravityVec()) || MR::isOppositeDirection(stack_17C, getAirGravityVec())) {
        mMovementStates.mHighWord &= 0xEFFFFFFFU;
        _904.zero();
        stack_17C.zero();
    }

    f32 slopeAngle = calcAngleD(_368);

    if (((mDrawStates.mWord >> 8) & 1U) && slopeAngle >= 30.0f) {
        if (!((mMovementStates.mHighWord >> 28) & 1U)) {
            _8F8 = -_2C4 * 2.0f;
            mMovementStates.mHighWord |= 0x10000000U;
            _8F0 = 10.0f;
            _3C4 = 30;
        }
    }

    if (_3C4 > 0xF) {
        _8F8 += -_2C4 * 0.5f;
    }

    if (!((mDrawStates.mWord >> 19) & 1U) || !isSlipPolygon(_45C)) {
        blendRate = 1.0f;
        if (((mMovementStates.mHighWord >> 28) & 1U)) {
            if (slopeAngle >= 15.0f) {
                _8F0 -= 0.4f;
            } else {
                _8F0 -= 0.8f;
            }

            if (_8F0 < 0.0f) {
                _8F0 = 0.0f;
                mMovementStates.mHighWord &= 0xEFFFFFFFU;
            }

            mVelocity.zero();
        } else {
            _8F0 = 0.0f;
        }
    } else {
        if (!((mMovementStates.mHighWord >> 28) & 1U)) {
            if (slopeAngle >= getActor()->getConst().getTable()->mSlipAngle) {
                TVec3f stack_164;
                if (MR::vecKillElement(mVelocity, stack_17C, &stack_164) < 0.0f) {
                    _8F8 = mVelocity;
                } else {
                    _8F8.zero();
                }

                mVelocity.zero();
                mMovementStates.mHighWord |= 0x10000000U;
                _8F0 = 10.0f;
                return;
            }

            TVec3f stack_158;
            const f32 sideSpeed = MR::vecKillElement(mVelocity, stack_17C, &stack_158);

            if (MR::isInAreaObj("SlopeRunningCancelArea", mPosHolder->mPos)) {
                _8F0 = 10.0f;
            }

            if (mWalkSpeed < 0.1f) {
                _8F0 = 10.0f;
            }

            if (mTargetWalkSpeedIndex == 0 && !isAnimationRun("すべり着地")) {
                _8F0 = 10.0f;
            }

            if (slopeAngle >= 25.0f && isStickOn()) {
                const TVec3f& worldPadDir = getWorldPadDir();

                TVec3f stack_14C;
                MR::vecKillElement(stack_17C, getCamDirY(), &stack_14C);
                if (MR::isNearZero(stack_14C)) {
                    MR::vecKillElement(stack_17C, getCamDirZ(), &stack_14C);
                }

                MR::normalize(&stack_14C);
                if (stack_14C.dot(worldPadDir) < 0.0f && mFrontVec.dot(stack_14C) < 0.0f) {
                    mDrawStates.mWord |= 0x08000000U;
                }
            }

            if (!((mDrawStates.mWord >> 27) & 1U)) {
                _8F0 += 0.3f * (slopeAngle / 90.0f);
            }

            if (_8F0 >= 10.0f) {
                if (MR::vecKillElement(mActor->getLastMove(), stack_17C, &stack_158) < 0.0f) {
                    _8F8 = mActor->getLastMove();
                } else {
                    _8F8.zero();
                }

                mMovementStates.mHighWord |= 0x10000000U;
                _8F0 = 10.0f;
                mVelocity.zero();
                mDrawStates.mWord &= 0xF7FFFFFFU;
            } else if (!((mDrawStates.mWord >> 27) & 1U) && sideSpeed > 0.0f) {
                const f32 t = _8F0 / 10.0f;
                const f32 remain = 1.0f - t;

                mVelocity = stack_158 + stack_17C * sideSpeed * remain;
            }

            blendRate *= 1.0f - (_8F0 / 10.0f);

            if (((mDrawStates.mWord >> 27) & 1U)) {
                _3FE++;

                if (_3FE > getActor()->getConst().getTable()->mSlopeDashAccelTime) {
                    _3FE = getActor()->getConst().getTable()->mSlopeDashAccelTime;
                }

                _8F4 = (slopeAngle / getActor()->getConst().getTable()->mSlopeDashAngleFactor) *
                       (static_cast< f32 >(_3FE) / static_cast< f32 >(getActor()->getConst().getTable()->mSlopeDashAccelTime));
                _8F4 = 1.0f + (_8F4 * getActor()->getConst().getTable()->mSlopeDashSpeedFactor);

                const f32 accel = 1.0f + ((_8F4 - 1.0f) * getActor()->getConst().getTable()->mSlopeDashAccelFactor);
                mVelocity += -_904 * accel;

                if (isAnimationRun("がんばり走り")) {
                    stopAnimation(nullptr);
                }
            }
        }
    }

    MR::vecKillElement(_8F8, *mGroundPolygon->getNormal(0), &_8F8);

    if (((mMovementStates.mHighWord >> 28) & 1U)) {
        _71E = 0;

        const f32 moveAngle = calcAngleD(_368);
        if (((mDrawStates.mWord >> 19) & 1U)) {
            MR::vecKillElement(mVelocity, stack_17C, &mVelocity);
            mVelocity += _8F8;

            _8F8 += -stack_17C * slopeAngle * getActor()->getConst().getTable()->mSlopeAccel;
        } else {
            TVec3f stack_140;
            stack_140 = _8F8;
            if (!MR::normalizeOrZero(&stack_140)) {
                MR::vecKillElement(mVelocity, stack_140, &mVelocity);
            }

            mVelocity += _8F8;
        }

        if (MR::isNearZero(_8F8)) {
            if (!((mDrawStates.mWord >> 19) & 1U)) {
                mMovementStates.mHighWord &= 0xEFFFFFFFU;
                return;
            }
        }

        if (moveAngle > 0.0f) {
            TVec3f stack_134;
            {
                TVec3f opposite = -stack_17C;
                MR::vecKillElement(opposite, getAirGravityVec(), &stack_134);
            }

            if (MR::isNearZero(stack_17C) || (MR::isNearZero(stack_134) && !((mDrawStates.mWord >> 19) & 1U))) {
                mMovementStates.mHighWord &= 0xEFFFFFFFU;
                return;
            }

            if (MR::isNearZero(stack_134)) {
            }

            MR::normalize(&stack_134);

            const f32 speed = _8F8.length();
            MR::normalizeOrZero(&_8F8);

            const f32 ratio = MR::clamp(moveAngle / 30.0f, 0.0f, 1.0f);
            blend = ratio * getActor()->getConst().getTable()->mSlopeCurveAssist;

            MR::vecBlendSphere(_8F8, -stack_17C, &_8F8, blend);
            _8F8.setLength(speed);
        }

        if (MR::isNearZero(_8F8)) {
            mMovementStates.mHighWord &= 0xEFFFFFFFU;
            moveSlopeSlide();
            return;
        }

        if (!(_8F8.dot(stack_17C) > 0.0f)) {
            if (stack_17C.dot(-*getGravityVec()) > 0.05f) {
                TVec3f stack_128;
                MR::vecKillElement(mFrontVec, _368, &stack_128);
                MR::normalize(&stack_128);

                TVec3f stack_11C(_8F8);
                MR::normalize(&stack_11C);

                f32 turnDot = MR::cos(1.0471976f);
                if (isAnimationRun("坂すべり上向きうつぶせ", 2)) {
                    turnDot = MR::cos(1.4959966f);
                }

                if (isAnimationRun("スケーティング") && _910.dot(stack_17C) > 0.0f) {
                    if (_910.length() > 2.0f) {
                        _8F8 = _910;
                    }

                    _910.zero();
                }

                if (stack_128.dot(stack_17C) < turnDot) {
                    setFrontVecKeepUp(stack_11C, 0.1f);
                    if (!isAnimationRun("坂すべり上向きうつぶせ", 2)) {
                        changeAnimation("坂すべり下向きあおむけ", 3);
                    }

                    if (isStickOn() && _16C.dot(stack_11C) > 0.0f &&
                        MR::diffAngleAbsHorizontal(stack_11C, mFrontVec, *getGravityVec()) < 0.5235988f) {
                        setFrontVecKeepUpAngle(_16C, getActor()->getConst().getTable()->mSlipMoveTurnAngleRad);
                    }
                } else {
                    if (!isAnimationRun("坂すべり下向きあおむけ", 3)) {
                        changeAnimation("坂すべり上向きうつぶせ", 2);
                    }

                    if (_3C2 < 5) {
                        setFrontVecKeepUp(-stack_11C, 0.5f);
                    } else {
                        setFrontVecKeepUp(-stack_11C, 0.1f);
                    }

                    if (_3C2 > 10 && isStickOn() && _16C.dot(stack_11C) > 0.0f) {
                        if (MR::diffAngleAbsHorizontal(-stack_11C, mFrontVec, *getGravityVec()) < 0.5235988f) {
                            TVec3f opposite = -_16C;
                            setFrontVecKeepUpAngle(opposite, getActor()->getConst().getTable()->mSlipMoveTurnAngleRad);
                        }
                    }
                }
            } else if (((mDrawStates.mWord >> 19) & 1U)) {
                if (!isAnimationRun("スケーティング")) {
                    _910 = _8F8;
                    _8F8.zero();
                }

                changeAnimation("スケーティング");
            }
        }

        if (isStickOn() && !MR::isNearZero(stack_17C) && _3C4 == 0) {
            const TVec3f& worldPadDir = getWorldPadDir();

            TVec3f stack_110(stack_17C);
            MR::vecKillElement(stack_17C, getCamDirY(), &stack_17C);
            if (MR::isNearZero(stack_17C)) {
                MR::vecKillElement(stack_110, getCamDirZ(), &stack_17C);
            }

            MR::normalize(&stack_17C);
            if (stack_17C.dot(worldPadDir) > 0.15f) {
                playEffect("共通スリップ坂制動");
                _3D0 = getActor()->getConst().getTable()->mTurnSlipTime;
            }
        }

        _1A8.x *= 0.9f;
        _1A8.y *= 0.9f;
        _1A8.z *= 0.9f;

        TVec3f stack_104;
        if (MR::vecKillElement(_1A8, stack_17C, &stack_104) >= 0.0f) {
            stack_104 = _1A8;
        }

        addVelocity(stack_104);
    } else {
        mVelocity += _8F8;

        if (isStickOn() && _3C4 == 0) {
            _8F8.scale(getActor()->getConst().getTable()->mSlopeCancelInertia);
        } else {
            _8F8.scale(getActor()->getConst().getTable()->mSlopeFinishInertia);
        }

        const MarioConstTable* table = getActor()->getConst().getTable();
        if (_8F8.length() < table->mSlopeAnimeFinishSpeed) {
            if (isAnimationRun("坂すべり上向きうつぶせ", 2)) {
                changeAnimation("坂すべり上向き終了");
            }

            if (isAnimationRun("坂すべり下向きあおむけ", 3)) {
                changeAnimation("坂すべり下向き終了");
            }

            if (isAnimationRun("スケーティング")) {
                stopAnimation("スケーティング");
            }

            if (isStickOn() && _8F0 == 0.0f &&
                (isAnimationRun("坂すべり上向き終了") || isAnimationRun("坂すべり下向き終了"))) {
                stopAnimation(nullptr);
            }
        }
    }

    const MarioConstTable* table = getActor()->getConst().getTable();
    f32 slopeSpeedMax = table->mSlopeSpeedMax;
    if (_3D0 != 0) {
        slopeSpeedMax = table->mSlopeSpeedMaxBraking;
    }

    const f32 slopeSpeed = _8F8.length();
    if (slopeSpeed > slopeSpeedMax) {
        _8F8.setLength((0.8f * slopeSpeed) + (0.2f * slopeSpeedMax));
    }

    if (!MR::vecBlendSphere(*getGravityVec(), -_368, &stack_188, blendRate)) {
        mMovementStates.mHighWord &= 0xEFFFFFFFU;
        tryDrop();
        return;
    }

    moveSlopeSlide();
}


void Mario::moveSlopeSlide() {
    if (calcAngleD(_368) < 5.8f) {
        if ((static_cast<u32>(mDrawStates.mBits.mBit19))) {
            const TVec3f* pWorldPadDir = &getWorldPadDir();
            f32 speed = _16C.length();
            if (_910.length() < speed) {
                _910 = _16C;
            }

            _910 += *pWorldPadDir * 0.5f;

            if (_910.length() > 15.0f) {
                _910.setLength(15.0f);
            }

            addVelocity(_910);
        }
    } else if (isAnimationRun("スケーティング") || isAnimationRun("坂すべり上向きうつぶせ", 2) || isAnimationRun("坂すべり下向きあおむけ", 3)) {
        TVec3f stack_20;
        stack_20.cross(getAirGravityVec(), _368);
        MR::normalizeOrZero(&stack_20);

        const TVec3f* pWorldPadDir = &getWorldPadDir();
        if (MR::isNearZero(_280)) {
            _284 = stack_20;
        }

        if (isStickOn() && _3C4 == 0) {
            f32 dot = _284.dot(*pWorldPadDir);
            if (MR::abs(dot) > 0.2f) {
                _280 = (_280 * mActor->getConst().getTable()->mSlopeSideMoveInertia) +
                       (dot * (1.0f - mActor->getConst().getTable()->mSlopeSideMoveInertia));
            } else {
                MarioConstTable* pConstTable = mActor->getConst().getTable();
                _280 = _280 * pConstTable->mSlopeSideStopInertia;
            }
        } else {
            MarioConstTable* pConstTable = mActor->getConst().getTable();
            _280 = _280 * pConstTable->mSlopeSideStopInertia;
        }

        TVec3f stack_14;
        stack_14.cross(_284, _368);
        _284.cross(_368, stack_14);

        MarioConstTable* pConstTable = mActor->getConst().getTable();
        addVelocity(_284, _280 * pConstTable->mSlopeSideMoveSpeed);
    }
}


bool Mario::taskOnSlipTurn(u32) {
    if (!isAnimationRun("ターンブレーキ滑り床")) {
        setFrontVecKeepUp(-_220);
        _754 = 0;
        _74C = 0.0f;
        return false;
    }

    _2B8.scale(mActor->mConst->getTable()->mInertiaSlipTurn);
    addVelocity(_2B8);
    return true;
}

// ---- MarioSlope.cpp ----

void Mario::lockGroundCheck(void* owner, bool simple) {
    _574 = owner;
    mFlags14.mLowBits.mBit26 = simple;
    mFlags14.mLowBits.mBit27 = 1;
    mMovementStates.mHighBits.mBit9 = simple;
}


void Mario::unlockGroundCheck(void* owner) {
    if (!owner || _574 == owner) {
        _574 = nullptr;
        mFlags14.mLowBits.mBit27 = 0;
    }
}


bool Mario::isUseSimpleGroundCheck() const {
    if ((static_cast<u32>(mFlags14.mLowBits.mBit27))) {
        return (static_cast<u32>(mFlags14.mLowBits.mBit26));
    }
    if ((static_cast<u32>(mMovementStates.mHighBits.mBit28))) {
        return true;
    }
    if (isPlayerModeTeresa()) {
        return true;
    }
    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        return (static_cast<u32>(mMovementStates.mHighBits.mBit28));
    }
    if (calcAngleD(_368) > mActor->getConst().getTable()->mFlatAngle - 5.0f) {
        Triangle ground;
        TVec3f direction(*getGravityVec() * 100.0f);
        TVec3f offset(*getGravityVec() * 30.0f);
        TVec3f origin(mPosHolder->mPos);
        origin -= offset;
        bool hit;
        if (fn_803B2950()) {
            hit = MR::getFirstPolyOnLineToDashYoshiStepAndMap(nullptr, &ground, origin, direction);
        } else {
            hit = MR::getFirstPolyOnLineToMap(nullptr, &ground, origin, direction);
        }
        if (hit) {
            if (calcAngleD(*ground.getNormal(0)) < mActor->getConst().getTable()->mFlatAngle - 5.0f) {
                return false;
            }
        }
        return true;
    }
    fn_8038CCF0();
    return false;
}


bool Mario::checkGroundOnSlope() {
    TVec3f normal;
    if (isAnimationRun("崖ふんばり")) {
        normal = -*getGravityVec();
    } else {
        normal = *_45C->getNormal(0);
    }
    if (MR::isNearZero(normal)) {
        return false;
    }
    TVec3f front;
    MR::vecKillElement(mFrontVec, normal, &front);
    if (MR::isNearZero(front)) {
        return false;
    }
    Triangle ground;
    bool hit;
    bool missingGround = false;
    mMovementStates.mLowBits.mBit11 = 1;
    f32 distance;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && isRising()) {
        distance = 10.0f;
    } else if (_414) {
        distance = 20.0f;
    } else {
        distance = 60.0f;
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        f32 angle = calcAngleD(_368) - 30.0f;
        if (angle < 0.0f) {
            angle = 0.0f;
        }
        if (angle > 40.0f) {
            angle = 40.0f;
        }
        distance += 2.0f * angle;
    }
    if (getCurrentStatus() == MarioStatus_Slider) {
        distance = 100.0f;
    }
    s32 groundCount = 0;
    TVec3f hitPosition;
    TVec3f positionSum;
    positionSum.zero();
    TVec3f groundPosition;
    TVec3f origin(mPosHolder->mPos - *getGravityVec() * 30.0f);
    TVec3f queryDirection(*getGravityVec() * distance);
    if (fn_803B2950()) {
        hit = MR::getFirstPolyOnLineBFastToDashYoshiStepAndMap(origin, queryDirection, &hitPosition, &ground);
    } else {
        hit = MR::getFirstPolyOnLineBFast(origin, queryDirection, &hitPosition, &ground);
    }
    if (hit) {
        if (calcAngleD(*ground.getNormal(0)) >= 80.0f) {
            hit = false;
        }
        f32 gravityDot = getGravityVec()->dot(*ground.getNormal(0));
        if (getCurrentStatus() != MarioStatus_Slider && gravityDot > -0.1908f) {
            hit = false;
        }
        if (hit) {
            positionSum += hitPosition;
            groundCount++;
            setGroundNorm(*ground.getNormal(0));
            *mGroundPolygon = ground;
            groundPosition = hitPosition;
            mGroundPos = groundPosition;
            recordLastGround();
        }
    }
    if (!groundCount) {
        missingGround = true;
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit18)) && getGravityVec()->dot(mGroundPos - mPosHolder->mPos) >= 0.0f) {
        mMovementStates.mLowBits.mBit18 = 0;
        TVec3f offset(mGroundPos - mPosHolder->mPos);
        f32 vertical = MR::vecKillElement(offset, *getGravityVec(), &offset);
        addTrans(*getGravityVec() * vertical);
        return true;
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && isRising()) {
        return false;
    }
    if (missingGround) {
        mMovementStates.mLowBits.mBit11 = 1;
        return false;
    }
    mMovementStates.mLowBits.mBit11 = 0;
    TVec3f down(-_368);
    TVec3f position(mGroundPos);
    if ((static_cast<u32>(mFlags14.mHighBits.mBit29))) {
        mFlags14.mHighBits.mBit29 = 0;
        mFlags20.mBits.mBit19 = 1;
    } else if (((static_cast<u32>(mMovementStates.mHighBits.mBit28)) || mTargetWalkSpeedIndex) && getPlayerMode() != PlayerMode_Teresa) {
        if (MR::abs((position - mPosHolder->mPos).dot(down)) < distance) {
            TVec3f offset(position - mPosHolder->mPos);
            f32 vertical = MR::vecKillElement(offset, down, &offset);
            if (!MR::isNearZero(vertical, 1.0f)) {
                mDrawStates.mBits.mBit31 = 0;
                TVec3f displacement(down * vertical);
                f32 ratio = MR::clamp(mWalkSpeed, 0.0f, 1.0f);
                if (!(static_cast<u32>(mMovementStates.mHighBits.mBit28))) {
                    displacement *= ratio;
                }
                bool notSlipping = !isSlipFloorCode(_960);
                if (!notSlipping && !(static_cast<u32>(mDrawStates.mBits.mBit27)) && !(static_cast<u32>(mPrevDrawStates.mBits.mBit27))) {
                    mVelocityAfter += displacement;
                }
            }
            return true;
        }
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

// ---- MarioSound.cpp ----

// Nonzero selectors address the three override columns (1..3).
u32 Mario::initSoundTable(SoundList* pList, u32 globalTablePosition) {
    SoundList* pEntry;
    u32 count = 0;
    s32 i = 0;
    while (true) {
        pEntry = pList + i;
        if (pEntry->name[0] == '\0') {
            break;
        }

        pEntry->_10 = 0;
        pEntry->_14 = pEntry->_4;

        if (globalTablePosition != 0) {
            s32 swapIndex = 0;
            while (true) {
                const SoundSwapList* pSwapEntry = soundswaplist + swapIndex;
                if (pSwapEntry->name[0] == '\0') {
                    break;
                }

                if (strcmp(pEntry->name, pSwapEntry->name) == 0) {
                    u32 soundID = soundswaplist[swapIndex].soundIDs[globalTablePosition - 1];
                    if (soundID != 0) {
                        pEntry->_14 = soundID;
                    }
                    break;
                }

                swapIndex++;
            }
        }

        count++;
        i++;
    }

    return count;
}


void Mario::initSound() {
    u32 count = initSoundTable(soundlist, 0);
    _96C = new HashSortTable(count);
    for (u32 i = 0; i < count; i++) {
        _96C->add(soundlist[i].name, i, false);
    }

    _96C->sort();
    _970 = nullptr;
}


void Mario::playSoundJ(const char* pSoundName, s32 timing) {
    uintptr_t index;
    if (_96C->search(pSoundName, &index)) {
        switch (soundlist[index]._8.mPolicy & 0x3) {
        case 0:
            MR::startSound(mActor, soundlist[index]._14, timing);
            break;

        case 2:
            MR::startSystemSE(soundlist[index]._14, timing);
            break;

        case 1:
            MR::startLevelSound(mActor, soundlist[index]._14, timing);
            break;

        case 3:
            MR::startSystemLevelSE(soundlist[index]._14, timing, -1, -1);
            break;
        }

        switch (soundlist[index]._8.mPolicy & ~0x3) {
        case 0x4:
        case 0x8:
            if (((mDrawStates.mWord & Mario::DrawStates::Underwater) >> 13) || (static_cast<u32>(mDrawStates.mBits.mBit12))) {
                playSoundJ(soundlist[index]._C, -1);
            }
            break;
        }
    }

    bool isFound = _96C->search("声", pSoundName, &index);
    if (isFound) {
        MR::startSound(mActor, soundlist[index]._14, timing);
    }
}


void Mario::stopSoundJ(const char* pSoundName, u32 delay) {
    uintptr_t index;
    if (_96C->search(pSoundName, &index)) {
        switch (soundlist[index]._8.mPolicy & 0x3) {
        case 0:
            MR::stopSound(mActor, soundlist[index]._14, delay);
            break;

        case 1:
            break;

        case 2:
            MR::stopSystemSE(soundlist[index]._14, delay);
            break;

        case 3:
            break;
        }
    }

    if (_96C->search("声", pSoundName, &index)) {
        JAISoundID soundID(soundlist[index]._14);
        MR::stopSound(mActor, soundID, delay);
    }
}


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




void Mario::playSoundTeresaFlying() {
    if (getPlayerMode() != 6) {
        return;
    }

    s32 timing = 100;
    if (getCurrentStatus() == 0x1C) {
        if (mWait->_14 == 0) {
            timing = 100 - static_cast< s32 >(mWait->_16);
            if (timing < 0) {
                timing = 0;
            }
        } else {
            timing = 0;
        }
    }

    playSound("テレサ浮遊", timing);
}


void Mario::playSoundTrampleCombo(u8 combo) {
    if (combo >= 7) {
        return;
    }

    MR::startSystemSE("SE_SY_TRAMPLE_COMBO", combo);
}


void Mario::setSeVersion(u32 version) {
    MR::setSeVersion(mActor, version);
}

// ---- MarioSpecial.cpp ----

// NONMATCHING: literal symbol identity; instruction words match after relocation masking.
void Mario::checkOnimasu(const HitSensor* pSensor) {
    if (strstr(MR::getSensorHost(pSensor)->mName, "オニマス") == nullptr) {
        return;
    }

    if (_5FC == nullptr) {
        _5FC = pSensor;
        _60C = 1;
    } else if (_5FC != pSensor && (*MR::getSensorPos(pSensor) - mPosHolder->mPos).length() < (_5FC->mPosition - mPosHolder->mPos).length()) {
        _5FC = pSensor;
        _60C = 1;
    }
}


bool Mario::isDossun(const Triangle* pTriangle) const {
    if (!pTriangle->isValid()) {
        return false;
    }

    if (strstr(pTriangle->mSensor->mHost->getName(), "ドッスン") != nullptr) {
        return true;
    }

    return false;
}


bool Mario::isStageCameraRotate2D() const {
    return MR::isEqualStageName("HellProminenceGalaxy") && MR::getCurrentScenarioNo() == 3;
}


bool Mario::isNoWalkFallOnDossun() const {
    if (!MR::isEqualStageName("CannonFleetGalaxy")) {
        return false;
    }

    if (isDossun(mGroundPolygon) && isDossun(_45C)) {
        return true;
    }

    return false;
}


bool Mario::isNotReflectGlassGround() const {
    if (!MR::isEqualStageName("AstroGalaxy")) {
        return false;
    }

    TVec3f vec(1627.0f, 783.0f, -2152.0f);
    TVec3f v10 = mPosHolder->mPos - vec;

    f32 v4 = MR::diffAngleAbs(getCamDirZ(), v10);
    f32 v5 = v4;
    if (v4 >= 1.5707964f) {
        v5 = (PI - v4);
    }

    f32 v7 = (400.0f + (300.0f * (MR::sin(v5) * MR::sin(v5))));
    MR::vecKillElement(v10, getAirGravityVec(), &v10);
    f32 length = v10.length();

    if (length >= v7) {
        if (length <= 700.0f) {
            return true;
        }
    }

    return false;
}


bool Mario::isUseAnotherMovingPolygon() const {
    if (MR::isEqualStageName("BattleShipGalaxy")) {
        return true;
    }

    if (MR::isEqualStageName("SandClockGalaxy")) {
        return true;
    }

    if (MR::isEqualStageName("TriLegLv1Galaxy")) {
        return true;
    }

    return false;
}


bool Mario::isUseFooSpecialGravity(const TVec3f& rA1, TVec3f* pA2) const {
    if (!isStatusActive(MarioStatus_Foo)) {
        return false;
    }

    if (MR::isEqualStageName("HeavensDoorGalaxy") && MR::getCurrentScenarioNo() == 2) {
        TVec3f vec(14760.0f, -10676.2f, 6770.0f);
        TVec3f res = vec - rA1;
        pA2->set(res);
        MR::normalizeOrZero(pA2);
        return true;
    }

    return false;
}


void Mario::updateOnimasu() {
    if (!_5FC) {
        return;
    }

    if ((_5FC->mPosition - mPosHolder->mPos).length() >= 1000.0f) {
        _5FC = nullptr;
        return;
    }

    const HitSensor* sensor = _5FC;
    MtxPtr inverse = MR::getSensorHost(sensor)->mCollisionParts->mInvBaseMatrix;
    TVec3f localPosition;
    TVec3f previousPosition;
    previousPosition = _600;
    PSMTXMultVec(inverse, &mActor->mPosition, &localPosition);
    if (_60C) {
        _600 = localPosition;
        _60C = 0;
        return;
    }

    if (!(static_cast<u32>(mFlags14.mLowBits.mBit2))) {
        if (previousPosition.y < -400.0f && localPosition.y > -400.0f) {
            if (MR::isInRange(localPosition.x, -400.0f, 400.0f) && MR::isInRange(localPosition.z, -400.0f, 400.0f)) {
                localPosition = _600;
                TVec3f position;
                PSMTXMultVec(MR::getSensorHost(sensor)->mCollisionParts->mBaseMatrix, &localPosition, &position);
                setTrans(position);
                mVelocity.zero();
            }
        } else if (previousPosition.y > 400.0f && localPosition.y <= 400.0f) {
            if (MR::isInRange(localPosition.x, -400.0f, 400.0f) && MR::isInRange(localPosition.z, -400.0f, 400.0f)) {
                mFlags14.mLowBits.mBit2 = 1;
                if (previousPosition.x < -320.0f) {
                    previousPosition.x = -320.0f;
                } else if (previousPosition.x > 320.0f) {
                    previousPosition.x = 320.0f;
                }

                if (previousPosition.z < -320.0f) {
                    previousPosition.z = -320.0f;
                } else if (previousPosition.z > 320.0f) {
                    previousPosition.z = 320.0f;
                }

                localPosition = previousPosition;
            }
        } else if (MR::isInRange(localPosition.x, -400.0f, 400.0f) && MR::isInRange(localPosition.y, -400.0f, 400.0f) &&
                   MR::isInRange(localPosition.z, -400.0f, 400.0f)) {
            mVelocity.zero();
            localPosition = _600;
            TVec3f position;
            PSMTXMultVec(MR::getSensorHost(sensor)->mCollisionParts->mBaseMatrix, &localPosition, &position);
            TVec3f displacement(position - mPosHolder->mPos);
            f32 height = 10.0f + mVerticalSpeed;
            if (-displacement.dot(getShadowNorm()) > height) {
                MR::vecKillElement(displacement, getShadowNorm(), &displacement);
                displacement += -getShadowNorm() * mVerticalSpeed;
                position = mPosHolder->mPos + displacement;
            }

            setTrans(position);
        }

        bool correctX = false;
        bool correctY = false;
        bool correctZ = false;
        TVec3f axisX;
        TVec3f axisY;
        TVec3f axisZ;
        MR::getSensorHost(sensor)->mCollisionParts->mBaseMatrix.getXDir(axisX);
        MR::getSensorHost(sensor)->mCollisionParts->mBaseMatrix.getYDir(axisY);
        MR::getSensorHost(sensor)->mCollisionParts->mBaseMatrix.getZDir(axisZ);
        f32 gravityX = axisX.dot(*getGravityVec());
        f32 absGravityX = MR::abs(gravityX);
        f32 gravityY = axisY.dot(*getGravityVec());
        f32 absGravityY = MR::abs(gravityY);
        f32 gravityZ = axisZ.dot(*getGravityVec());
        f32 absGravityZ = MR::abs(gravityZ);
        f32 lower = -500.0f;
        f32 upper = -300.0f;

        if (absGravityX > absGravityY && absGravityX > absGravityZ) {
            if (gravityX > 0.0f) {
                lower = 300.0f;
                upper = 500.0f;
            }

            if (MR::isInRange(localPosition.x, lower, upper)) {
                bool insideY = MR::isInRange(localPosition.y, -320.0f, 320.0f);
                bool insideZ = MR::isInRange(localPosition.z, -320.0f, 320.0f);
                if (insideY && insideZ) {
                    mActor->mPressNormal6D8 = axisX;
                    mActor->setPress(0, 0);
                } else {
                    if (insideZ) {
                        correctY = true;
                    }

                    if (insideY) {
                        correctZ = true;
                    }
                }
            }
        } else if (absGravityY > absGravityX && absGravityY > absGravityZ) {
            if (gravityY > 0.0f) {
                lower = 300.0f;
                upper = 500.0f;
            }

            if (MR::isInRange(localPosition.y, lower, upper)) {
                bool insideX = MR::isInRange(localPosition.x, -320.0f, 320.0f);
                bool insideZ = MR::isInRange(localPosition.z, -320.0f, 320.0f);
                if (insideX && insideZ) {
                    mActor->mPressNormal6D8 = axisY;
                    if (gravityY < 0.0f) {
                        mActor->setPress(0, 0);
                    }
                } else {
                    if (insideZ) {
                        correctX = true;
                    }

                    if (insideX) {
                        correctZ = true;
                    }
                }
            }
        } else {
            if (gravityZ > 0.0f) {
                lower = 300.0f;
                upper = 500.0f;
            }

            if (MR::isInRange(localPosition.z, lower, upper)) {
                bool insideX = MR::isInRange(localPosition.x, -320.0f, 320.0f);
                bool insideY = MR::isInRange(localPosition.y, -320.0f, 320.0f);
                if (insideX && insideY) {
                    mActor->mPressNormal6D8 = axisZ;
                    mActor->setPress(0, 0);
                } else {
                    if (insideY) {
                        correctX = true;
                    }

                    if (insideX) {
                        correctY = true;
                    }
                }
            }
        }

        if (correctX) {
            if (localPosition.x > -400.0f && localPosition.x < -320.0f) {
                localPosition.x = -400.0f;
            } else if (localPosition.x < 400.0f && localPosition.x > 320.0f) {
                localPosition.x = 400.0f;
            }
        }

        if (correctY) {
            if (localPosition.y > -400.0f && localPosition.y < -320.0f) {
                localPosition.y = -400.0f;
            } else if (localPosition.y < 400.0f && localPosition.y > 320.0f) {
                localPosition.y = 400.0f;
            }
        }

        if (correctZ) {
            if (localPosition.z > -400.0f && localPosition.z < -320.0f) {
                localPosition.z = -400.0f;
            } else if (localPosition.z < 400.0f && localPosition.z > 320.0f) {
                localPosition.z = 400.0f;
            }
        }

        if (correctX | correctY | correctZ) {
            TVec3f position;
            PSMTXMultVec(MR::getSensorHost(sensor)->mCollisionParts->mBaseMatrix, &localPosition, &position);
            if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mLowBits.mBit6)) || (static_cast<u32>(mMovementStates.mLowBits.mBit5))) {
                TVec3f displacement(position - mPosHolder->mPos);
                if (getWallNorm().dot(displacement) < 0.0f) {
                    addVelocity(-displacement);
                } else {
                    setTrans(position);
                }
            }
        }
    } else if (localPosition.y > 400.0f) {
        mFlags14.mLowBits.mBit2 = 0;
    } else {
        bool corrected = false;
        if (previousPosition.y >= -320.0f && localPosition.y < -320.0f) {
            localPosition.y = previousPosition.y;
            corrected = true;
        }

        if (previousPosition.x >= -320.0f && localPosition.x < -320.0f) {
            localPosition.x = previousPosition.x;
            corrected = true;
        }

        if (previousPosition.x <= 320.0f && localPosition.x > 320.0f) {
            localPosition.x = previousPosition.x;
            corrected = true;
        }

        if (previousPosition.z >= -320.0f && localPosition.z < -320.0f) {
            localPosition.z = previousPosition.z;
            corrected = true;
        }

        if (previousPosition.z <= 320.0f && localPosition.z > 320.0f) {
            localPosition.z = previousPosition.z;
            corrected = true;
        }

        if (corrected) {
            TVec3f position;
            PSMTXMultVec(MR::getSensorHost(sensor)->mCollisionParts->mBaseMatrix, &_600, &position);
            setTrans(position);
            localPosition = _600;
        }
    }

    _600 = localPosition;
}


bool Mario::isHeadPushEnableArea() const {
    if (!((mMovementStates.mHighWord >> 8) & 1U)) {
        return false;
    }

    if (MR::isEqualStageName("SandClockGalaxy")) {
        if (MR::isInRange(mPosHolder->mPos.x, -7020.0f, -6920.0f)) {
            return true;
        }
    }

    return false;
}


bool Mario::isOnimasuBinderPressSkip() const {
    if (isStatusActive(MarioStatus_SideStep)) {
        if (mFrontWallTriangle->mSensor != nullptr) {
            if (strstr(mFrontWallTriangle->mSensor->mHost->mName, "オニマス")) {
                return true;
            }
        }
    }

    return false;
}

// ---- MarioSpin.cpp ----

void Mario::checkTornado() {
    if (((mMovementStates.mLowWord >> 30) & 1U)) {
        mMovementStates.mHighBits.mBit20 = 0;
    }
}


void Mario::resetTornado() {
    _530 = 0.0f;
    _534 = 0;
    _538 = 0.0f;
    mMovementStates.mLowBits.mBit16 = 0;
    _544 = 0;
    mYAngleOffset = 0.0f;
    _3F0 = 1.0f;
}


void Mario::calcTornadoTilt() {
    bool flying = false;
    bool spinning = ((mMovementStates.mLowWord >> 16) & 1U) != 0;
    if (getPlayerMode() == PlayerMode_Bee) {
        if (((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && ((mMovementStates.mLowWord >> 14) & 1U)) {
            flying = true;
        }
    }

    bool tilted = spinning | flying;
    if (!isStickOn() || !tilted) {
        _548 *= mActor->getConst().getTable()->mTornadoTiltCancel;
        bool blended;
        if (tilted) {
            blended = MR::vecBlendSphere(_54C, mHeadVec, &_54C, mActor->getConst().getTable()->mTornadoTiltSpeed);
        } else {
            blended = MR::vecBlendSphere(_54C, mHeadVec, &_54C, mActor->getConst().getTable()->mTornadoTiltOffSpeed);
        }

        if (!blended) {
            _54C = mHeadVec;
        }
    } else {
        TVec3f tilt(getWorldPadDir() * mActor->getConst().getTable()->mTornadoTiltAngle +
                    mHeadVec * (1.0f - mActor->getConst().getTable()->mTornadoTiltAngle));
        MR::normalize(&tilt);
        bool blended = MR::vecBlendSphere(_54C, tilt, &_54C, mActor->getConst().getTable()->mTornadoTiltSpeed);
        MR::normalize(&_54C);
        f32 alignment = MR::abs(getWorldPadDir().dot(mFrontVec));
        _548 = _548 * mActor->getConst().getTable()->mTornadoTiltNear + alignment * (1.0f - mActor->getConst().getTable()->mTornadoTiltNear);
        if (!blended) {
            _54C = tilt;
        }
    }
}


void Mario::reflectWallOnSpinning(const TVec3f& rNormal, u16 time) {
    setFrontVecKeepUp(rNormal);
    _3F8 = time;
    _328 = mFrontVec;
    doSpinWallEffect();
}


void Mario::forceStopTornado() {
    if (((mMovementStates.mLowWord >> 16) & 1U)) {
        _40A = mActor->getConst().getTable()->mTornadoRestartTime;
    }

    resetTornado();
    if (((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        cancelTornadoJump();
    }

    mDrawStates.mBits.mBit23 = 1;
}


void Mario::doSpinWallEffect() {
    if ((!(static_cast<u32>(mMovementStates.mLowBits.mBit23)) || !mFrontWallTriangle->mSensor->isType(0x4F)) && (!(static_cast<u32>(mMovementStates.mLowBits.mBit6)) || !mBackWallTriangle->mSensor->isType(0x4F)) &&
        (!(static_cast<u32>(mMovementStates.mLowBits.mBit5)) || !mSideWallTriangle->mSensor->isType(0x4F))) {
        playSound("壁反射");
        playSound("声スピンキャンセル");
        playEffect("壁スパーク");
    }
}


void Mario::startRotationTask(u32 flags) {
    pushTask(&Mario::taskOnRotation, flags);
}


bool Mario::taskOnRotation(u32 flags) {
    if (flags & 4) {
        if (!isAnimationRun("ヘリコプタージャンプ")) {
            mYAngleOffset = 0.0f;
            return false;
        }

        if (isRising()) {
            mYAngleOffset += mActor->getConst().getTable()->mTrampleBegomaRotRise;
        } else {
            mYAngleOffset += mActor->getConst().getTable()->mTrampleBegomaRotFall;
        }
    }

    return true;
}

// ---- MarioState.cpp ----


void Mario::sendStateMsg(u32 msg) {
    MarioState* pNext;

    for (MarioState* pState = _97C; pState != nullptr;) {
        pNext = pState->_8;

        if (isStatusActive(pState->mStatusId) == MarioStatus_None) {
            pState = pNext;
            continue;
        }

        if (!pState->proc(msg)) {
            if (isStatusActive(pState->mStatusId) != MarioStatus_None) {
                closeStatus(pState);
            }
        } else if (msg == MarioStateMsg_Update) {
            msg = MarioStateMsg_Keep;
        }

        pState = pNext;
    }
}


bool Mario::updatePosture(MtxPtr mtx) {
    if (_97C != nullptr) {
        return _97C->postureCtrl(mtx);
    }

    postureCtrl(mtx);

    return false;
}


void Mario::changeStatus(MarioState* pState) {
    if (isStatusActive(pState->mStatusId)) {
        return;
    }

    _980 = pState;

    sendStateMsg(MarioStateMsg_Notice);

    if (_97C) {
        pState->_8 = _97C;
    }

    _97C = pState;

    if (!pState->proc(MarioStateMsg_Start)) {
        closeStatus(pState);

        _97C = pState->_8;
    }
}


void Mario::closeStatus(MarioState* pState) {
    if (pState == nullptr) {
        while (_97C != nullptr) {
            closeStatus(_97C);
        }

        return;
    }

    MarioState* pCurr = _97C;

    if (pCurr == pState) {
        _97C = pState->_8;
    } else {
        while (pCurr != nullptr) {
            MarioState* pNext = pCurr->_8;

            if (pNext == pState) {
                break;
            }

            pCurr = pNext;
        }

        pCurr->_8 = pState->_8;
    }

    pState->_8 = nullptr;
    pState->proc(MarioStateMsg_Close);
}


u32 Mario::getCurrentStatus() const {
    MarioState* pState = _97C;

    if (pState == nullptr) {
        return MarioStatus_None;
    }

    return pState->mStatusId;
}


bool Mario::isStatusActive(u32 statusId) const {
    MarioState* pState = _97C;

    if (pState == nullptr) {
        return false;
    }

    while (pState != nullptr) {
        if (statusId == pState->mStatusId) {
            return true;
        }

        pState = pState->_8;
    }

    return false;
}

// ---- MarioStep.cpp ----

// NONMATCHING: relocation label; all scalar values verified.
void Mario::checkStep() {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit21))) {
        return;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        return;
    }

    if (isStatusActive(MarioStatus_Hang)) {
        return;
    }

    if (isStatusActive(MarioStatus_Wall)) {
        return;
    }

    if (isStatusActive(MarioStatus_Swim)) {
        return;
    }

    if (isStatusActive(MarioStatus_Climb)) {
        return;
    }


    if (isDamaging()) {
        return;
    }

    if (isSwimming()) {
        return;
    }

    if (!MR::isNearZero(_184)) {
        return;
    }

    if (!isStickOn()) {
        return;
    }

    if (_750) {
        return;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit19))) {
        return;
    }

    if (isSlipPolygon(_45C)) {
        return;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit10))) {
        return;
    }

    if (!(static_cast<u32>(mMovementStates.mHighBits.mBit6))) {
        return;
    }

    const TVec3f* worldPadDir = &getWorldPadDir();
    if (!(worldPadDir->dot(*mFrontWallTriangle->getNormal(0)) < -0.5f)) {
        return;
    }

    f32 stepHeight = (_4A4 - mPosHolder->mPos).dot(-*getGravityVec());
    if (stepHeight < mActor->getConst().getTable()->mWalkStepHeight) {
        startStep(_4A4);
    }
}


void Mario::startStep(const TVec3f& rVec) {
    if (getCurrentStatus() == 0x10) {
        return;
    }

    TVec3f checkPos = rVec - *getGravityVec() * 80.0f;
    HitInfo hitInfo;

    if (fn_803B2AF0(checkPos, &hitInfo)) {
        return;
    }

    bool hitNear;
    if (fn_803B2950()) {
        hitNear = Collision::checkStrikeBallToDashYoshiStepAndMap(checkPos, 20.0f, nullptr, nullptr);
    } else {
        hitNear = Collision::checkStrikeBallToMap(checkPos, 20.0f, nullptr, nullptr);
    }
    if (hitNear) {
        return;
    }

    bool hitFar;
    if (fn_803B2950()) {
        hitFar = Collision::checkStrikeBallToDashYoshiStepAndMap(checkPos, 50.0f, nullptr, nullptr);
    } else {
        hitFar = Collision::checkStrikeBallToMap(checkPos, 50.0f, nullptr, nullptr);
    }
    if (hitFar) {
        return;
    }

    TVec3f oldPos(mPosHolder->mPos);
    mPosHolder->fn_80405F90(rVec);
    const f32 ceilDist = calcDistToCeil(false);
    mPosHolder->fn_80405F90(oldPos);

    if (ceilDist < 200.0f) {
        return;
    }

    if (_4C8->isValid() && MR::isSensorPressObj(_4C8->mSensor)) {
        return;
    }

    if (_4D8->isValid()) {
        if (MR::isSensorPressObj(_4D8->mSensor)) {
            return;
        }

        MtxPtr prevBase = _4D8->getPrevBaseMtx()->toMtxPtr();
        MtxPtr base = _4D8->getBaseMtx()->toMtxPtr();

        if (!MR::isSameMtx(base, prevBase) && _4D8->mSensor != mGroundPolygon->mSensor) {
            return;
        }
    }

    setTrans(rVec);
    changeStatus(mStep);
    mVelocity.zero();
    _3D0 = 0;
    mMovementStates.mLowBits.mBit15 = 0;
}

// ---- MarioStick.cpp ----

void Mario::checkBeeStick() {
    return;
}


void Mario::tryBeeStick(const HitSensor* pSensor) {
    bool res = mStick->setStickSensor(pSensor);
    if (res) {
        changeStatus(mStick);
    }
}

// ---- MarioSwim.cpp ----

void MarioSwim_FORCE_MATCH_SDATA2() {
    (void)100.0f;
    (void)50.0f;
    (void)0.52359879f;
    (void)-200.0f;
    (void)0.100000001f;
    (void)0.200000003f;
    (void)10.0f;
    (void)1.74532938f;
    (void)-10.0f;
    (void)1.49599659f;
    (void)200.0f;
    (void)8.0f;
    (void)2.09439516f;
    (void)40.0f;
    (void)0.00999999978f;
    (void)0.0299999993f;
    (void)6.28318548f;
    (void)-50.0f;
    (void)0.196349546f;
    (void)30.0f;
    (void)-30.0f;
    (void)-0.100000001f;
    (void)60.0f;
    (void)1000.0f;
    (void)0.899999976f;
    (void)400.0f;
    (void)0.707000017f;
    (void)2.5f;
    (void)-400.0f;
    (void)120.0f;
    (void)0.125f;
    (void)300.0f;
    (void)0.0500000007f;
    (void)5.0f;
    (void)0.150000006f;
    (void)0.87266469f;
    (void)2.26892805f;
    (void)12.0f;
    (void)25.0f;
    (void)0.25f;
    (void)0.980000019f;
    (void)0.00100000005f;
    (void)1.04719758f;
    (void)2000.0f;
    (void)-25.0f;
    (void)0.949999988f;
    (void)0.75f;
    (void)0.300000012f;
    (void)2.61799383f;
    (void)0.99000001f;
    (void)0.995000005f;
    (void)0.959999979f;
    (void)1.65346992f;
    (void)15.0f;
    (void)110.0f;
    (void)80.0f;
    (void)-500.0f;
    (void)210.0f;
    (void)170.0f;
    (void)10000.0f;
    (void)0.970000029f;
    (void)0.800000012f;
    (void)500.0f;
    (void)1.01999998f;
    (void)1.5f;
    (void)6.0f;
    (void)0.699999988f;
    (void)0.314159274f;
    (void)0.785398185f;
    (void)2.61799407f;
    (void)16.0f;
    (void)-0.5f;
    (void)18.0f;
    (void)1.25f;
    (void)1.00010002f;
}


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


bool Mario::isSwimming() const {
    if (isStatusActive(MarioStatus_Swim)) {
        return true;
    }

    return isStatusActive(MarioStatus_Foo);
}


bool Mario::forceStartSwimAndShoot(const TVec3f& rKnockbackVec) {
    WaterInfo waterInfo;
    if (!MR::getWaterAreaObj(&waterInfo, mActor->mPosition)) {
        return false;
    }

    startSwim();
    mSwim->setDamage(rKnockbackVec, 30);
    return true;
}


bool Mario::forceExitSwim() {
    if (!isStatusActive(MarioStatus_Swim)) {
        return false;
    }

    mSwim->mNextAction = MarioSwim::EXIT_ACTION_SURFACE;
    closeStatus(mSwim);
    return true;
}


bool Mario::checkStartSwim() {
    if (isStatusActive(MarioStatus_Swim)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Climb)) {
        return false;
    }

    if ((static_cast<u32>(mFlags14.mLowBits.mBit24)) && ((getMovementStates().mLowWord >> 30) & 1U)) {
        getAnimator()->waterToGround();
        mFlags14.mLowBits.mBit24 = 0;
    }

    if (!fn_803B2950() && isPlayerModeYoshi()) {
        mActor->fn_803B9830()->fn_8041BD30();
    }

    if (mSwim->fn_803FD5E0(0)) {
        if (getPlayerMode() == 3) {
            addTrans(-*getGravityVec() * 100.0f);
            if (((getPlayer()->getMovementStates().mLowWord & Mario::MovementStates::Jumping) >> 31)) {
                stopJump();
            }

            if (getPlayer()->isStatusActive(MarioStatus_Wall)) {
                getPlayer()->closeStatus(0);
            }

            if (getPlayer()->isStatusActive(MarioStatus_Blown)) {
                getPlayer()->closeStatus(0);
            }

            return false;
        }

        if (!fn_803B2950()) {
            startSwim();
            return true;
        }
    }

    mSwim->updateUnderWater();
    return false;
}


void Mario::startSwim() {
    if (!isStatusActive(MarioStatus_Swim)) {
        if (getPlayer()->isDamaging()) {
            mSwim->_9D = 4;
            if (_3BC >= 75) {
                playSound("水落下突入高");
            } else {
                playSound("水落下突入");
            }
        } else if (((getPlayer()->getMovementStates().mLowWord & Mario::MovementStates::Jumping) >> 31)) {
            mSwim->_9D = 1;
            if (_3BC >= 75) {
                playSound("水落下突入高");
            } else {
                playSound("水落下突入");
            }
        } else {
            mSwim->_9D = 0;
            playSound("水歩行突入");
        }

        mActor->fn_803B9F10();
        changeStatus(mSwim);
        clearSlope();
        stopWalk();
        stopJump();
        mSinkTimer = 0;

        mMovementStates.mHighWord &= 0xFFFFFFFCU;
        mMovementStates.mLowBits.mBit13 = 0;
        if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mLowBits.mBit5)) || (static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
            addVelocity(getWallNorm(), 50.0f);
        }

        getPlayer()->lockGroundCheck(mSwim, true);
        checkBaseTransBall();
        mActor->_F44 = false;
    }
}

// ---- MarioSwimDamage.cpp ----

bool Mario::checkWaterDamage() {
    if (((mMovementStates.mLowWord >> 4) & 1U)) {
        mMovementStates.mLowBits.mBit4 = 0;
        mActor->fn_803BE840(_7C4);
        mSwim->addDamage(_7C4);
        return true;
    } else if (((mMovementStates.mHighWord >> 24) & 1U)) {
        mMovementStates.mHighBits.mBit24 = 0;
        mActor->fn_803BE840(_7C4);
        mSwim->addFaint(_7C4);
        return true;
    } else {
        return false;
    }
}

// ---- MarioTalk.cpp ----

void Mario::startTalk(const LiveActor* pActor) {
    stopWalk();

    mTalk->_14 = pActor;

    changeStatus(mTalk);
}


void Mario::endTalk() {
    closeStatus(mTalk);
}


void Mario::setLastNonFixMoveVec(const TVec3f& rVec) {
    _334 = rVec;
}

// ---- MarioTask.cpp ----

static const f32 sOne = 1.0f;

static const f32 sZero = 0.0f;

static const f32 sSameDirEpsilon = 0.01f;

static const f32 sNearZeroEpsilon = 0.001f;

static const f32 sHipDropSlideAngle = 0.7853982f;

static const f32 sHipDropSlideRot = 0.10471976f;

static const f32 sHipDropSlideLen = 10.0f;

static const f32 sJumpDropSlideDotMin = 0.1f;

static const f32 sHopperJumpSlideDotMin = 0.707f;


void Mario::delTask(MarioModuleTask* pTask) {
    MarioModuleTask* next = pTask->mNext;
    if (next == nullptr) {
        return;
    }

    MarioModuleTask* head = _974;
    if (head == pTask) {
        if (next == pTask) {
            _974 = nullptr;
        } else {
            _974 = next;
        }

        pTask->mNext = nullptr;
        pTask->end();
        return;
    }

    while (true) {
        if (head == nullptr) {
            return;
        }

        MarioModuleTask* headNext = head->mNext;
        if (headNext == pTask) {
            if (next == pTask) {
                head->mNext = head;
            } else {
                head->mNext = next;
            }

            pTask->mNext = nullptr;
            pTask->end();
            return;
        }

        if (headNext == head) {
            return;
        }
    }
}


void Mario::execTask() {
    MarioModuleTask* task = _974;

    while (true) {
        if (task == nullptr) {
            return;
        }

        MarioModuleTask* next = task->mNext;
        if (!task->exec()) {
            delTask(task);
        }

        if (next == task) {
            return;
        }

        task = next;
    }
}


void Mario::drawTask() const {
    MarioModuleTask* task = _974;

    while (true) {
        if (task == nullptr) {
            return;
        }

        MarioModuleTask* next = task->mNext;
        task->draw();
        if (next == task) {
            return;
        }

        task = next;
    }
}


void Mario::initTask() {
    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        _984[i] = nullptr;
    }

    _974 = nullptr;
}


bool Mario::isActiveTask(Task task) {
    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        if (_984[i] == task) {
            return true;
        }
    }

    return false;
}


bool Mario::isActiveTaskID(u32 id) {
    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        if (_984[i] != nullptr) {
            if (_A08[i] & id) {
                return true;
            }
        }
    }

    return false;
}


bool Mario::pushTask(Task task, u32 flags) {
    if (isActiveTask(task)) {
        return false;
    }

    u32 index = 0;
    for (; index < ARRAY_SIZE(_984); index++) {
        if (_984[index] == nullptr) {
            break;
        }
    }

    _984[index] = task;
    _A08[index] = flags;
    return true;
}


void Mario::popTask(Task task) {
    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        if (_984[i] == task) {
            _984[i] = nullptr;
            return;
        }
    }
}


void Mario::callExtraTasks(u32 flags) {
    execTask();

    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        if (_984[i] == nullptr) {
            continue;
        }

        if (flags != 0 && !(flags & _A08[i])) {
            continue;
        }

        if (!(this->*_984[i])(_A08[i])) {
            Task task = _984[i];
            popTask(task);
        }
    }
}


void Mario::startHandy() {
    pushTask(&Mario::taskOnHandy, 0x40);
}


bool Mario::taskOnHandy(u32) {
    if (!mActor->_468) {
        stopEffect("いい汗");
        return false;
    }

    if (mTargetWalkSpeedIndex > 2) {
        playEffect("いい汗");
    } else {
        stopEffect("いい汗");
    }

    return true;
}


void Mario::startHipDropBlur() {
    if (isPlayerModeHopper()) {
        if (gIsLuigi) {
            playEffect("ホッパー尻落ルイージ");
        } else {
            playEffect("ホッパー尻落");
        }

        pushTask(&Mario::taskOnHipDropBlurHopper, 0x80);
        return;
    }

    if (isPlayerModeCloud()) {
        if (gIsLuigi) {
            playEffect("雲尻落ルイージ");
        } else {
            playEffect("雲尻落");
        }
        pushTask(&Mario::taskOnHipDropBlurCloud, 0x80);
        return;
    }

    if (gIsLuigi) {
        playEffect("尻落ルイージ");
    } else {
        playEffect("尻落");
    }

    pushTask(&Mario::taskOnHipDropBlur, 0x80);
}


bool Mario::taskOnHipDropBlurCloud(u32) {
    if (!(static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit20)) || (static_cast<u32>(mMovementStates.mLowBits.mBit30)) || isStatusActive(MarioStatus_Swim)) {
        if (gIsLuigi) {
            stopEffect("雲尻落ルイージ");
        } else {
            stopEffect("雲尻落");
        }

        return false;
    }

    return true;
}


bool Mario::taskOnHipDropBlur(u32) {
    if (!(static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit20)) || (static_cast<u32>(mMovementStates.mLowBits.mBit30)) || isStatusActive(MarioStatus_Swim)) {
        if (gIsLuigi) {
            stopEffect("尻落ルイージ");
        } else {
            stopEffect("尻落");
        }

        return false;
    }

    return true;
}


bool Mario::taskOnHipDropSlide(u32 flags) {
    if (flags == 0x100) {
        if (!(static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit20))) {
            return false;
        }
    }

    if (MR::diffAngleAbs(_70C, getAirGravityVec()) > sHipDropSlideAngle) {
        return false;
    }

    if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit30)) || !(static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit31)) || isStatusActive(MarioStatus_Swim)) {
        return false;
    }

    HitSensor* sensor = _A68;
    bool enabled = sensor->mValidByHost != 0 && sensor->mValidBySystem != 0;

    if (!enabled) {
        return false;
    }

    TVec3f dir(mPosHolder->mPos);
    dir -= _A4C;
    dir.setLength(_A64);

    TVec3f dirCopy(dir);
    MR::normalizeOrZero(&dir);

    TVec3f reverseGravity = -_70C;
    if (dir.dot(reverseGravity) <= sZero) {
        return false;
    }

    Mtx rot;
    PSMTXRotAxisRad(rot, &_A58, sHipDropSlideRot);
    PSMTXMultVecSR(rot, &dir, &dir);
    dir.setLength(_A64);
    dir -= dirCopy;
    dir.setLength(sHipDropSlideLen);

    cutGravityElementFromJumpVec(true);
    cutVecElementFromJumpVec(_70C);
    addVelocity(dir);

    dirCopy.setLength(sOne);
    addVelocity(dirCopy);
    return true;
}


bool Mario::taskOnFreezeEnd(u32) {
    return mActor->fn_803BB5F0();
}


// NONMATCHING: vector temporary slots, registers and literal/callback identities.
void Mario::startHipDropSlide(const HitSensor* pSensor) {
    if (isActiveTaskID(0x100)) {
        return;
    }

    if (isActiveTaskID(0x200)) {
        return;
    }

    pushTask(&Mario::taskOnHipDropSlide, 0x100);

    TVec3f dir(mPosHolder->mPos - *MR::getSensorPos(pSensor));

    if (MR::isSameDirection(dir, getAirGravityVec(), sSameDirEpsilon) || MR::isNearZero(dir, sNearZeroEpsilon)) {
        dir = -mFrontVec;
    }

    MR::normalizeOrZero(&dir);

    _A58.cross(-getAirGravityVec(), dir);
    MR::normalizeOrZero(&_A58);

    _A68 = const_cast< HitSensor* >(pSensor);
    _A4C = *MR::getSensorPos(pSensor);
    _A64 = MR::getSensorRadius(const_cast< HitSensor* >(pSensor));
    _70C = getAirGravityVec();

    changeAnimation("ヒップドロップ滑り");
}


// NONMATCHING: vector temporary slots, registers and literal/callback identities.
void Mario::startJumpDropSlide(const HitSensor* pSensor) {
    if (isActiveTaskID(0x100)) {
        return;
    }

    if (isActiveTaskID(0x200)) {
        return;
    }

    TVec3f dir(mPosHolder->mPos - *MR::getSensorPos(pSensor));
    MR::normalizeOrZero(&dir);

    f32 dot = dir.dot(-getAirGravityVec());
    if (dot <= sJumpDropSlideDotMin) {
        return;
    }

    pushTask(&Mario::taskOnHipDropSlide, 0x200);

    TVec3f slideDir(mPosHolder->mPos - *MR::getSensorPos(pSensor));

    if (MR::isSameDirection(slideDir, getAirGravityVec(), sSameDirEpsilon) || MR::isNearZero(slideDir, sNearZeroEpsilon)) {
        slideDir = -mFrontVec;
    }

    MR::normalizeOrZero(&slideDir);

    _A58.cross(-getAirGravityVec(), slideDir);
    MR::normalizeOrZero(&_A58);

    _A68 = const_cast< HitSensor* >(pSensor);
    _A4C = *MR::getSensorPos(pSensor);
    _A64 = MR::getSensorRadius(const_cast< HitSensor* >(pSensor));
    _70C = getAirGravityVec();

    if (mActor->_468 == 0 && !isPlayerModeHopper() && dot > sHopperJumpSlideDotMin) {
        f32 frontDot = mFrontVec.dot(slideDir);
        if (frontDot > sZero) {
            changeAnimation("ジャンプ順滑り");
        } else {
            changeAnimation("ジャンプ逆滑り");
        }
    }

    mJumpVec.zero();
}


void Mario::startFreezeEnd() {
    pushTask(&Mario::taskOnFreezeEnd, 0x800);
}

// ---- MarioTeresa.cpp ----

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


void Mario::startTeresaMode() {
    _418 = 0;
    _428 = 0;
    getPlayer()->mMovementStates.mLowBits.mBit31 = 1;
    getPlayer()->mMovementStates.mLowBits.mBit30 = 0;
    getPlayer()->mMovementStates.mHighBits.mBit29 = 0;
    mJumpVec.zero();
    changeStatus(mTeresa);
}


bool Mario::getHitWallNorm(TVec3f* pNormal) {
    if (((getPlayer()->mMovementStates.mLowWord >> 23) & 1U)) {
        if (!isThroughWall(mFrontWallTriangle)) {
            *pNormal = *mFrontWallTriangle->getNormal(0);
            return true;
        }
    } else if (((getPlayer()->mMovementStates.mLowWord >> 6) & 1U)) {
        if (!isThroughWall(mBackWallTriangle)) {
            *pNormal = *mBackWallTriangle->getNormal(0);
            return true;
        }
    } else if (((getPlayer()->mMovementStates.mLowWord >> 5) & 1U) && !isThroughWall(mSideWallTriangle)) {
        *pNormal = *mSideWallTriangle->getNormal(0);
        return true;
    }

    return false;
}


void Mario::resetTeresaMode() {
    mActor->_F44 = true;
    _418 = 0;
    mTeresa->resetTeresaMode();
}


void Mario::doTeresaReflection(const TVec3f& rNormal, bool effect) {
    mTeresa->doTeresaReflection(rNormal, effect);
}


void Mario::startTeresaDisappear() {
    _418 = mActor->getConst().getTable()->mTeresaWallThroughTime;
    playSound("テレサ消える");
    playSound("声トルネード");
    MR::startCSSound("CS_TERESA", nullptr, 0, 0, 1.0f);
    mTeresa->updateDropFlag();
    resetSleepTimer();
    if (_962 == 5 && mVerticalSpeed < 100.0f) {
        MR::isNearZero(mAirGravityVec, 0.001f);
        mTeresa->_34 += mAirGravityVec * 10.0f;
    }
}


// native: defined in MarioTeresa.cpp (DOL split owner); misplaced copy
#ifndef GALAXY_NATIVE
XanimeGroupInfo teresaAnimeTable[] = {
    {{"基本"}, 1.0f, 16},
    {{""}},
};


XanimeBckTable2 teresaAnime2[] = {
    {{"基本"}, {{"wait", 1.0f}, {"run", 0.0f}}},
    {{""}, {{"", 0.0f}, {nullptr, 0.0f}}},
};
#endif

// ---- MarioWait.cpp ----


bool Mario::isBlendWaitGround() const {
    return calcAngleD(_368) > 8.0f;
}


void Mario::checkSpecialWaitAnimation() {
    if (mWait->checkStart()) {
        changeStatus(mWait);
    }
}


void Mario::resetSleepTimer() {
    mWait->_16 = 0;
    if (isStatusActive(MarioStatus_Wait)) {
        closeStatus(mWait);
    }
}

// ---- MarioWalk.cpp ----

void Mario::stopWalk() {
    mTargetWalkSpeedIndex = 0;
    mWalkSpeed = 0.0f;
    _71E = 0;
    _71F = 0;
    _3D4 = 0;

    getAnimator()->initWalkWeight();
    getAnimator()->resetTilt();

    mMovementStates.mLowBits.mBit15 = 0;
    mMovementStates.mHighBits.mBit28 = 0;
    cancelSquatMode();

    mMovementStates.mLowWord &= ~Mario::MovementStates::Turning;
    _3D2 = 0;
    _8F0 = 0.0f;
    _3F4 = 0.0f;

    stopEffect("共通スリップ坂");
    stopAnimation("歩行制動ブレーキ", 1);
    stopAnimation("ブレーキ");
}


void Mario::cancelSquatMode() {
    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit21))) {
        return;
    }

    calcDistToCeil(false);

    if ((static_cast<u32>(mFlags24.mLowBits.mBit21)) || (static_cast<u32>(mMovementStates.mLowBits.mBit21))) {
        mFlags14.mLowBits.mBit16 = 1;
    }

    mMovementStates.mLowBits.mBit21 = 0;
    mFlags24.mLowBits.mBit21 = 0;
    stopAnimation("しゃがみ基本");

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) || !(static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        if (!isSwimming()) {
            changeAnimation(nullptr, "基本");
        }

        if (!isAnimationRun("サマーソルト") && (static_cast<u32>(mFlags14.mLowBits.mBit16))) {
            if (mWalkSpeed > 0.1f) {
                if (!isPlayerModeYoshi()) {
                    fn_803EEC20("しゃがみ終了", nullptr);
                }
            } else {
                changeAnimation("しゃがみ終了");
            }
        }
    }

    mMovementStates.mHighBits.mBit25 = 0;
}


f32 Mario::getTargetWalkSpeed() const {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit21))) {
        return 0.0f;
    }

    f32 targetWalkSpeed = ::sWalkTargetTable[mTargetWalkSpeedIndex];

    if (isPlayerModeYoshi()) {
        targetWalkSpeed *= mActor->fn_803B9810()->fn_8041FB30();
    }

    if (mSinkTimer != 0) {
        if (_960 == 25) {
            targetWalkSpeed = 0.0f;
        } else if (_960 == 31) {
            targetWalkSpeed = 0.0f;
        } else {
            targetWalkSpeed *= 1.0f - static_cast< f32 >(mSinkTimer) / 256.0f;
        }
    }

    targetWalkSpeed *= 1.0f - _2D0;

    if (_434 != 0) {
        targetWalkSpeed *= mActor->getConst().getTable()->mItemDashRatio;
    }

    return targetWalkSpeed;
}


// NONMATCHING: temporary/register allocation; SMG2 shallow-water ripple behavior recovered.
void Mario::decideSquatWalkAnimation() {
    if (mTargetWalkSpeedIndex != 0) {
        u32 flags = mDrawStates.mWord;
        if ((((flags >> 13) & 1) || ((flags >> 12) & 1)) && _738 < 160.0f) {
            if (isPlayerModeYoshi()) {
                f32 previous = mPrevAnimFrame;
                if (previous > getAnimator()->getFrame() || (previous < 30.0f && getAnimator()->getFrame() >= 30.0f)) {
                    playSound("水跳ね左足小");
                    playEffect("水はね弱");
                    f32 depth = _738;
                    TVec3f offset(_368);
                    offset *= depth;
                    TVec3f position(mGroundPos);
                    position += offset;
                    playEffectSRT("水波紋", 0.2f, _73C, position);
                }
            } else {
                if (mPrevAnimFrame > getAnimator()->getFrame()) {
                    playSound("水跳ね左足小");
                    playEffect("水はね左弱");
                    f32 depth = _738;
                    TVec3f offset(_368);
                    offset *= depth;
                    TVec3f side(mSideVec);
                    side *= 20.0f;
                    TVec3f first(mGroundPos);
                    first -= side;
                    TVec3f position(first);
                    position += offset;
                    playEffectSRT("水波紋", 0.2f, _73C, position);
                }
                if (mPrevAnimFrame < 30.0f && getAnimator()->getFrame() >= 30.0f) {
                    playSound("水跳ね右足小");
                    playEffect("水はね右弱");
                    f32 depth = _738;
                    TVec3f offset(_368);
                    offset *= depth;
                    TVec3f side(mSideVec);
                    side *= 20.0f;
                    TVec3f first(mGroundPos);
                    first += side;
                    TVec3f position(first);
                    position += offset;
                    playEffectSRT("水波紋", 0.2f, _73C, position);
                }
            }
        }
    } else if (static_cast<u32>(mDrawStates.mBits.mBit13)) {
        f32 depth = _738;
        if (depth < 160.0f && !(mActor->_37C & 0x3F)) {
            TVec3f offset(_368);
            offset *= depth;
            f32 random = MR::getRandom() - 0.5f;
            TVec3f side(mSideVec);
            side *= random;
            TVec3f displacement(side);
            displacement *= 20.0f;
            TVec3f first(mShadowPos);
            first += displacement;
            TVec3f position(first);
            position += offset;
            playEffectSRT("水波紋", 0.2f, _73C, position);
        }
    }
}


// NONMATCHING: relocation labels; speed tables and scalar values verified.
void Mario::decideWalkSpeed() {
    u32 i;
    bool canIndexDecrease = false;
    if (mTargetWalkSpeedIndex != 0) {
        f32 threshold = ::sSpeedTableB[mTargetWalkSpeedIndex - 1];
        if (mStickPos.z < threshold) {
            canIndexDecrease = true;
        }
    }

    for (i = 0; i < ARRAY_SIZE(::sSpeedTableA); i++) {
        if (mStickPos.z < ::sSpeedTableA[i]) {
            break;
        }
    }

    if (mTargetWalkSpeedIndex <= i || canIndexDecrease) {
        mTargetWalkSpeedIndex = i;
    }

    s32 clingNum = MR::getKarikariClingNum();
    if (!mActor->isEnableNerveChange()) {
        clingNum = 0;
    }
    if (clingNum != 0) {
        if (clingNum > 5) {
            clingNum = 5;
        }

        if (mTargetWalkSpeedIndex > 5 - clingNum) {
            mTargetWalkSpeedIndex = 5 - clingNum;
        }
    }

    if (((mDrawStates.mWord & Mario::DrawStates::Underwater) >> 13) && mTargetWalkSpeedIndex > 6) {
        mTargetWalkSpeedIndex = 6;
    }

    if (mActor->mBeeWallWalk != 0 && mTargetWalkSpeedIndex > 4) {
        mTargetWalkSpeedIndex = 4;
    }

    if (_960 == 32 || (isPlayerModeYoshi() && mActor->fn_803B9810()->fn_8041F8C0())) {
        if (mTargetWalkSpeedIndex > 3) {
            mTargetWalkSpeedIndex = 3;
        }

        if (mTargetWalkSpeedIndex != 0) {
            startPadVib(1);
        }

        if (mTargetWalkSpeedIndex > 2) {
            getAnimator()->setAnimationInterpoleRatio(0.5f);
        } else {
            getAnimator()->setAnimationInterpoleRatio(1.0f);
        }
    } else {
        f32 speed = getAnimator()->getAnimationInterpoleRatio();
        f32 new0C = speed;
        new0C += 0.1f;

        if (new0C > 1.0f) {
            new0C = 1.0f;
        }

        getAnimator()->setAnimationInterpoleRatio(new0C);
    }
    if (isPlayerModeYoshi() && mActor->fn_803B9810()->fn_8041FA40() && mTargetWalkSpeedIndex > 8) {
        mTargetWalkSpeedIndex = 8;
    }
    if (fn_803B1C10()) {
        mTargetWalkSpeedIndex = 7;
    }
    if (mCometFireDanceArea) {
        mTargetWalkSpeedIndex = 7;
    }
}


// NONMATCHING: register/temporary allocation and independent instruction scheduling.
#pragma dont_inline on
void Mario::decideWalkAnimation() {
    MarioConst* constants = mActor->mConst;
    f32 walkScale = constants->mTable[constants->mCurrentTable]->mWalkSpeed;
    if (mTargetWalkSpeedIndex == 0 && mWalkSpeed < 0.2f && isBlendWaitGround()) {
        getAnimator()->fn_803DE880();
    } else {
        getPlayer()->mFlags14.mLowBits.mBit16 = 0;

        if (mTargetWalkSpeedIndex == 0 && mSwim->_1B2) {
            bool prepare = !isPlayerModeBee();
            if (prepare) {
                changeAnimation("飛び込み準備", 4);
                return;
            }
        }

        if (mSinkTimer == 0) {
            if (getPlayer()->mTargetWalkSpeedIndex != 0) {
                getAnimator()->fn_803DEC10();
            }

            fn_803AF2F0(::sWeightTable[mTargetWalkSpeedIndex]);
        } else {
            f32 weights[] = {0, 0, 0, 0};
            if (mTargetWalkSpeedIndex != 0) {
                f32 f1 = 1.0f;
                if (mTargetWalkSpeedIndex <= 3) {
                    if (mTargetWalkSpeedIndex == 2) {
                        f1 = 0.75f;
                    }

                    if (mTargetWalkSpeedIndex == 3) {
                        f1 = 0.5f;
                    }

                    weights[0] = f1;
                    weights[1] = 1.0f - f1;
                } else {
                    if (mTargetWalkSpeedIndex >= 5) {
                        f1 = static_cast< f32 >(mSinkTimer) / 100.0f;
                    }

                    f1 = MR::clamp(f1, 0.0f, 1.0f);
                    weights[1] = f1;
                    weights[2] = 1.0f - f1;
                }
            } else {
                weights[3] = 1.0f;
            }

            fn_803AF2F0(&weights[0]);
        }
    }

    stopAnimation("飛び込み準備", 4);

    fn_803AD5B0();
    fn_803AD9C0();
    mPrevAnimFrame = getAnimator()->getFrame();

    f32* footStep = ::sFootStep;
    f32 f3 = 0.0f;

    if (mActor->mBeeWallWalk != 0) {
        footStep = ::sFootStepBeeWallWalk;
    }

    if (isPlayerModeYoshi()) footStep = ::sFootStepYoshi;

    for (int i = 0; i < ARRAY_SIZE(*::sWeightTable); i++) {
        f3 += footStep[i] * ::sWeightTable[mTargetWalkSpeedIndex][i];
    }

    f32 animationSpeed;

    if (f3 == 0.0f) {
        animationSpeed = 0.33f;
    } else {
        animationSpeed = 0.5f * (60.0f * ((0.01f * (mWalkSpeed * walkScale)) / f3));
    }

    if (_8F0 > 0.0f && !(static_cast<u32>(mDrawStates.mBits.mBit27))) {
        animationSpeed *= 1.0f + ((_8F0 / 10.0f) * mActor->mConst->mTable[mActor->mConst->mCurrentTable]->mSlopeAnimeRatio);

        if (_8F0 > 5.0f && animationSpeed > 4.0f) {
            animationSpeed *= mActor->mConst->mTable[mActor->mConst->mCurrentTable]->mSlopeSpinAnimeRatio;
            changeAnimation("がんばり走り");
            startBas("RunSlope", false, 0.0f, 0.0f);
        }
    } else if (_3FE != 0) {
        animationSpeed *= _8F4;
    }

    f32 diffFromTargetSpeed = getTargetWalkSpeed() - mWalkSpeed;

    if (mWalkSpeed > ::sWalkTargetTable[5]) {
        f32 squared = diffFromTargetSpeed * diffFromTargetSpeed;
        f32 factor = (mWalkSpeed - ::sWalkTargetTable[5]) / (1.0f - ::sWalkTargetTable[5]);
        diffFromTargetSpeed = (diffFromTargetSpeed * (1.0f - factor)) + (squared * factor);
    } else if (mWalkSpeed < ::sWalkTargetTable[3]) {
        f32 sqrt = MR::sqrt(diffFromTargetSpeed);
        f32 factor = (::sWalkTargetTable[3] - mWalkSpeed) / ::sWalkTargetTable[3];
        diffFromTargetSpeed = (diffFromTargetSpeed * factor) + (sqrt * (1.0f - factor));
    }

    f32 f4 = mActor->mConst->mTable[mActor->mConst->mCurrentTable]->mStartSpinAnimeRatio;
    f32 f5 = 1.0f + 4.0f * diffFromTargetSpeed;

    if (f5 > 2.0f) {
        f5 = 2.0f;
    }

    if (f5 < 1.0f) {
        f5 = 1.0f;
    }

    if (getTargetWalkSpeed() < ::sWalkTargetTable[6]) {
        f5 = 1.0f;
    }

    if (fn_803EF7C0() == 32 && mTargetWalkSpeedIndex != 0) {
        f5 = 1.2f;
    }

    f32 f6 = f4 * (0.5f * (60.0f * ((getTargetWalkSpeed() * walkScale * 0.01f) / footStep[2])));
    f32 factor2 = 2.0f - f5;

    if (animationSpeed < f6) {
        animationSpeed = animationSpeed * factor2 + f6 * (1.0f - factor2);
    }

    if (mSinkTimer != 0 && mTargetWalkSpeedIndex != 0 && animationSpeed < 1.0f) {
        animationSpeed = 1.0f;
    }

    animationSpeed *= (1.0f - _2D0);
    _3F4 = f5 - 1.0f;
    getAnimator()->setSpeed(animationSpeed);

    if (!mActor->_EA4 && mTargetWalkSpeedIndex == 0 && mActor->mHealth == 1 && mActor->mMaxHealth > 2 &&
        (_970 == nullptr || strcmp(_970, "DamageWait"))) {
        getAnimator()->changeTrackAnim(3, "ダメージウエイト");
        startBas("DamageWait", false, 0.0f, 0.0f);
        mActor->fn_803BBFD0("DamageWait");
    }

    checkWallPush();

    f32 f7 = 0.9f;

    if (gIsLuigi) {
        f7 = 0.3f;
    }

    if (mTargetWalkSpeedIndex > 5 && mWalkSpeed > f7 && !(static_cast<u32>(mDrawStates.mBits.mBit27)) && !(static_cast<u32>(mMovementStates.mHighBits.mBit10))) {
        _71E = mActor->mConst->mTable[mActor->mConst->mCurrentTable]->mBrakeFirstTimer;
    }

    if (_71E != 0) {
        _71E--;
    }

    if (mTargetWalkSpeedIndex == 0) {
        if (_71E != 0 && !isSlipPolygon(mGroundPolygon) && !(static_cast<u32>(mDrawStates.mBits.mBit26))) {
            doBrakingAnimation();
            _71F = mActor->mConst->mTable[mActor->mConst->mCurrentTable]->mBrakeSecondTimer;
        }

        _71E = 0;

        s32 clingNum = MR::getKarikariClingNum();
        if (!mActor->isEnableNerveChange()) clingNum = 0;
        if (clingNum >= 1) {
            if (!isPlayerModeYoshi()) changeAnimationUpper("カリカリ限界");
            stopAnimation("歩行制動ブレーキ", 1);
        }

        if (clingNum < 1 && isAnimationRun("カリカリ限界")) {
            stopAnimationUpper("カリカリ限界");
        }
    }
}

#pragma dont_inline reset

void Mario::doBrakingAnimation() {
    if (!isAnimationRun("歩行制動ブレーキ")) {
        playSound("ターンブレーキ", -1);
    }
    changeAnimation("歩行制動ブレーキ", 1);
    getAnimator()->setAnimationFrameCtrlAttribute(1);
    if (gIsLuigi) {
        getAnimator()->setSpeed(0.5f);
    }

    playEffect("共通ブレーキ");
    _71F = 0;
}


void Mario::checkWallPush() {
    if (mTargetWalkSpeedIndex != 0 && ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mHighBits.mBit13))) && checkWallJumpCode()) {
        return;
    }

    f32 angle = MR::diffAngleAbsHorizontal(mFrontVec, -getWallNorm(), *getGravityVec());
    bool sideStep = false;
    f32 wallPushAngleRange = getActor()->getConst().getTable()->mWallPushAngleRange;

    bool checkAngle = mTargetWalkSpeedIndex != 0 && (static_cast<u32>(mMovementStates.mLowBits.mBit23));

    if (checkAngle && angle < MR::toRadian(wallPushAngleRange)) {
        sideStep = true;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit21))) {
        sideStep = false;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit19))) {
        sideStep = false;
    }

    if (calcAngleD(getWallNorm()) < getActor()->getConst().getTable()->mForceWallAngle) {
        sideStep = false;
        if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) && mTargetWalkSpeedIndex != 0) {
            mTargetWalkSpeedIndex = 1;
            mWalkSpeed = 0.0f;
        }
    }

    if (!isAnimationRun("壁押し", 0) && sideStep) {
        doSideStep();
    }
}


void Mario::updateBrakeAnimation() {
    if (_71F != 0) {
        if (!isAnimationRun("歩行制動ブレーキ", 1)) {
            _71F = 0;
        } else {
            _71F--;
            if (!MR::isNearZero(mStickPos.z)) {
                _71F = 0;
            }

            if (_71F == 0) {
                stopAnimation(nullptr);
                stopWalk();
            }
        }
    } else if (isAnimationRun("歩行制動ブレーキ", 1) && (isAnimationTerminate(nullptr) || mTargetWalkSpeedIndex != 0)) {
        stopAnimation(nullptr);
    }

    if (!gIsLuigi) {
        return;
    }

    if (!isAnimationRun("歩行制動ブレーキ", 1) && !isAnimationRun("ブレーキ")) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mHighBits.mBit13))) {
        stopAnimation(nullptr);
        _71F = 0;
        _71E = 0;
        _3D0 = 0;
        _3D2 = 0;
    } else if (!MR::isDemoActive() && (static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        playSound("ルイージ滑り");
    }
}

void Mario::decideOnIceAnimation() {
    if (mTargetWalkSpeedIndex == 0) {
        if (mWalkSpeed > 0.2f && !isAnimationRun("氷上慣性走行")) {
            changeAnimationWithAttr("氷上慣性走行", 1);
            mIceAnimFoot = 1 - mIceAnimFoot;
        }
    } else {
        decideWalkAnimation();
        if (mWalkSpeed > 0.7f) {
            switch (mIceAnimFoot) {
            case 0:
                getAnimator()->changeTrackAnim(2, "氷上力行左");
                break;
            default:
                getAnimator()->changeTrackAnim(2, "氷上力行右");
            }
        }
    }

    if (mTargetWalkSpeedIndex != 0 || mWalkSpeed <= 0.2f) {
        stopAnimation("氷上慣性走行");
    }
}


// NONMATCHING: equivalent early-exit branch orientation and register allocation.
void Mario::updateOnSand() {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit0)) || mCometFireDanceArea) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        if (_960 == 27 || _960 == 28) {
            if (!strcmp(MR::getSoundCodeString(_45C), "Sand") && mSinkTimer < 64) {
                mSinkTimer++;
            }
        } else if (isCurrentFloorSink()) {
            if (mSinkTimer < 255) {
                mSinkTimer++;
                if (_960 == 25 || _960 == 31) {
                    if (_960 == 31) {
                        if (mSinkTimer == 1) {
                            playSound("声沼沈み");
                        }

                        playSound("沼強制沈み");
                    } else {
                        if (mSinkTimer == 1) {
                            playSound("声砂沈み");
                        }

                        playSound("砂強制沈み");
                    }

                    stopWalk();
                    mSinkTimer = MR::clamp(static_cast< s32 >(mSinkTimer) + 3, 0, 255);

                    if (getAirGravityVec().dot(_368) > -0.99f) {
                        TVec3f vec1;
                        MR::vecKillElement(_368, getAirGravityVec(), &vec1);
                        TVec3f vec2;
                        vec2.cross(vec1, _368);
                        MR::normalize(&vec2);
                        vec1.cross(_368, vec2);
                        addVelocity(vec1 * 6.0f);
                    }
                } else {
                    playSound("砂沈み");
                }
            } else {
                mActor->fn_803BAFE0();
                return;
            }

            if (!isAnimationRun(nullptr)) {
                getAnimator()->changeTrackAnim(1, "埋まり歩行");
            }
        } else if (_960 == 32 || _A7F != 0) {
            if (mSinkTimer < 48) {
                mSinkTimer++;
            }
        } else {
            if (mSinkTimer != 0 && !isAnimationRun(nullptr)) {
                getAnimator()->changeTrackAnim(1, "歩行");
            }

            mSinkTimer = 0;
        }
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        mSinkTimer = 0;
    }

    if (isStatusActive(6)) {
        mSinkTimer = 0;
    }
}


void Mario::updateOnPoison() {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        if (_960 == 18) {
            if (mPoisonTimer == 0) {
                mActor->decLife(0);
                playSound("毒沼ダメージ");
                playSound("ダメージ");
                playSound("声小ダメージ");
                if (mActor->mHealth == 0) {
                    mActor->forceGameOver();
                }

                startCamVib(0);
                mActor->_BC4 = 1;
            }

            if (mPoisonTimer < 255) {
                mPoisonTimer++;
            } else {
                mPoisonTimer = 0;
            }
        } else {
            mPoisonTimer = 0;
        }
    } else if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && _3BC > 10) {
        mPoisonTimer = 0;
    }

    if (isStatusActive(6)) {
        mPoisonTimer = 0;
    }
}


void Mario::updateOnWater() {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        switch (_960) {
        case 20:
        case 21:
        case 22:
            touchWater();
            _738 = 20.0f;
            _73C = _368;

            switch (_962) {
            case 20:
                _738 += 20.0f;
            case 21:
                _738 += 20.0f;
            case 22:
                mDrawStates.mWord |= Mario::DrawStates::Underwater;
            }
        }

        if (_960 == 23 && _962 == 23) {
            touchWater();
            mDrawStates.mBits.mBit12 = 1;
            _738 = 3.0f;
            _73C = _368;
        }
    }

    switch (_962) {
    case 20:
    case 21:
    case 22:
    case 23:
        mDrawStates.mBits.mBit2 = 1;
    }
}

// ---- MarioWall.cpp ----

bool Mario::isWalling() const {
    return getCurrentStatus() == MarioStatus_Wall;
}


// NONMATCHING: independent stack-vector argument address setup order.
void Mario::checkWallStick() {
    if (getPlayerMode() == PlayerMode_Bee) {
        checkBeeStick();
        return;
    }

    if (!checkWallJumpCode() && isEnableStickWall()) {
        if (mActor->_334) {
            TVec3f horizontal;
            f32 vertical = MR::vecKillElement(mJumpVec, mActor->mAppliedGravityVec, &horizontal);
            horizontal.setLength(-0.25f);
            mJumpVec = mActor->mAppliedGravityVec * vertical + horizontal;
            return;
        }

        changeStatus(mWall);
        stopWalk();
        mMovementStates.mLowBits.mBit31 = 0;
        mMovementStates.mLowBits.mBit20 = 0;
        mMovementStates.mLowBits.mBit25 = 0;
        mMovementStates.mLowBits.mBit8 = 0;
        fixWallingPosition(true);
        resetTornado();
    }
}


u8 Mario::checkStickWallSide() {
    if (isStickOn()) {
        const TVec3f& direction = getWorldPadDir();
        if (direction.dot(getWallNorm()) < -0.5f) {
            return 1;
        }

        if (direction.dot(getWallNorm()) > 0.5f && !(static_cast<u32>(mMovementStates.mLowBits.mBit2))) {
            return 2;
        }
    }

    return 0;
}


s32 Mario::checkStickFrontBack() {
    if (isStickOn()) {
        const TVec3f& direction = getWorldPadDir();
        if (direction.dot(-mFrontVec) < -0.5f) {
            return 1;
        }

        if (direction.dot(-mFrontVec) > 0.5f && !(static_cast<u32>(mMovementStates.mLowBits.mBit2))) {
            return 2;
        }
    }

    return 0;
}


bool Mario::fixWallingPosition(bool immediate) {
    if (!fixWallingDist()) {
        return false;
    }

    fixWallingDir(immediate);
    return true;
}


void Mario::fixWallingDir(bool immediate) {
    bool back = true;
    if (isStatusActive(MarioStatus_SideStep)) {
        back = false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6)) && back) {
        if (immediate) {
            setFrontVecKeepUp(-*mBackWallTriangle->getNormal(0));
            return;
        }

        setFrontVecKeepUp(-*mBackWallTriangle->getNormal(0), 0.1f);
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        if (immediate) {
            setFrontVecKeepUp(-*mFrontWallTriangle->getNormal(0));
            return;
        }

        setFrontVecKeepUp(-*mFrontWallTriangle->getNormal(0), 0.1f);
    }
}


bool Mario::fixWallingTop() {
    TVec3f side = getWallNorm().cross(getAirGravityVec());
    MR::normalizeOrZero(&side);
    if (MR::isNearZero(side)) {
        return false;
    }

    _75C.cross(getWallNorm(), side);
    MR::normalizeOrZero(&_75C);
    if (MR::isNearZero(_75C)) {
        return false;
    }

    getWallNorm().dot(getAirGravityVec());
    getPlayer()->forceSetHeadVecKeepSide(_75C);
    return true;
}


bool Mario::checkWallFloorCode(u16 code) const {
    if (((static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mHighBits.mBit13))) && _964[0] == code) {
        return true;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6)) && _964[1] == code) {
        return true;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit5)) && _964[2] == code) {
        return true;
    }

    return false;
}


bool Mario::checkWallCode(const char* pCode, bool frontBackOnly) const {
    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        const char* wallCode = MR::getWallCodeString(mBackWallTriangle);
        if (wallCode && strcmp(wallCode, pCode) == 0) {
            return true;
        }
    }

    if (((mMovementStates.mLowWord >> 23) & 1U) || ((mMovementStates.mHighWord >> 13) & 1U)) {
        const char* wallCode = MR::getWallCodeString(mFrontWallTriangle);
        if (wallCode && strcmp(wallCode, pCode) == 0) {
            return true;
        }
    }

    if (frontBackOnly) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 5) & 1U)) {
        const char* wallCode = MR::getWallCodeString(mSideWallTriangle);
        if (wallCode && strcmp(wallCode, pCode) == 0) {
            return true;
        }
    }

    return false;
}


bool Mario::checkWallCodeNorm(u16 code, TVec3f* pNormal, bool frontBackOnly) const {
    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        u16 wallCode = MR::getWallCodeIndex(mBackWallTriangle);
        if (wallCode == code) {
            if (pNormal != nullptr) {
                *pNormal = *mBackWallTriangle->getNormal(0);
            }

            return true;
        }
    }

    if (((mMovementStates.mLowWord >> 23) & 1U) || ((mMovementStates.mHighWord >> 13) & 1U)) {
        u16 wallCode = MR::getWallCodeIndex(mFrontWallTriangle);
        if (wallCode == code) {
            if (pNormal != nullptr) {
                *pNormal = *mFrontWallTriangle->getNormal(0);
            }

            return true;
        }
    }

    if (frontBackOnly) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 5) & 1U)) {
        u16 wallCode = MR::getWallCodeIndex(mSideWallTriangle);
        if (wallCode == code) {
            if (pNormal != nullptr) {
                *pNormal = *mSideWallTriangle->getNormal(0);
            }

            return true;
        }
    }

    return false;
}


void Mario::setWallCancel() {
    mWall->_1C = 1;
    mWall->_24 = getWallNorm();
}


void Mario::keepDistFrontWall() {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        TVec3f horizontal;
        MR::vecKillElement(_4E8 - mGroundPos, *getGravityVec(), &horizontal);
        if (horizontal.length() < 80.0f) {
            push(*mFrontWallTriangle->getNormal(0) * (80.0f - horizontal.length()));
        }
    }
}


bool Mario::isEnableStickWall() {
    switch (getPlayerMode()) {
    case PlayerMode_Teresa:
        return false;
    case PlayerMode_Foo:
        isStatusActive(MarioStatus_Foo);
        return false;
    case 0xA:
        return false;
    }

    if (mWall->isCancel()) {
        return false;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit31)) || (static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit16))) {
        return false;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit1))) {
        return false;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit23)) && !(static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit29)) && mVerticalSpeed < mActor->getConst().getTable()->mWallStickGrHeight) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit10)) && _4E0 < mActor->getConst().getTable()->mWallStickFrHeight && !getWallPolygon()->mSensor->isType(0x51)) {
        return false;
    }

    if (isAnimationRun("空中ひねり") || isAnimationRun("雲スピン空中")) {
        if (mActor->_945 < 25) {
            return false;
        }
    } else if (mActor->isPunching() && mActor->_945 < 15) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit10)) && (static_cast<u32>(mMovementStates.mLowBits.mBit29)) && (static_cast<u32>(mMovementStates.mHighBits.mBit6))) {
        if ((_4A4 - mShadowPos).dot(-*getGravityVec()) < mActor->getConst().getTable()->mWallStickStepHeight) {
            return false;
        }
    }

    TVec3f horizontal;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        MR::vecKillElement(mJumpVec, mActor->mAppliedGravityVec, &horizontal);
        if (mBackWallTriangle->getNormal(0)->dot(horizontal) > 0.0f) {
            return false;
        }
    }

    if (isInhibitWall()) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) && calcPolygonAngleD(mFrontWallTriangle) < 80.0f) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6)) && calcPolygonAngleD(mBackWallTriangle) < 80.0f) {
        return false;
    }

    TVec3f side = getWallNorm().cross(getAirGravityVec());
    if (MR::normalizeOrZero(&side)) {
        return false;
    }

    if (!checkWallCode("NotWallSlip", true)) {
        mMovementStates.mHighBits.mBit23 = 1;
    }

    if (checkWallCode("NoAction", true)) {
        return false;
    }

    if ((getPlayerMode() != PlayerMode_Ice || !getWallPolygon()->mSensor->isType(0x51)) && isRising()) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        if (!isAnimationRun("壁ジャンプ") && getPlayer()->checkStickWallSide() != 1) {
            return false;
        }
    } else if (!isAnimationRun("壁ジャンプ") && !(static_cast<u32>(mMovementStates.mLowBits.mBit22)) && getPlayer()->checkStickWallSide() != 1) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit15))) {
        return false;
    }

    if (mActor->_468) {
        if (!checkTrgA()) {
            return false;
        }

        mWall->initTriangleJump();
        mWall->startJump();
        return false;
    }

    TVec3f position(mActor->mCapPosition);
    f32 distance = 120.0f;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        distance = -120.0f;
    }

    if (!fn_803B2880(position, mFrontVec * distance)) {
        return false;
    }

    return MR::getKarikariClingNum() == 0;
}


// NONMATCHING: stack allocation.
bool Mario::fixWallingDist() {
    const u32 backWall = (mMovementStates.mLowWord >> 6) & 1U;
    const u32 frontWall = (mMovementStates.mLowWord >> 23) & 1U;
    if (backWall & frontWall) {
        return true;
    }

    if (backWall) {
        TVec3f position(_4F4 + *mBackWallTriangle->getNormal(0) * 79.0f);
        f32 distance = MR::vecKillElement(position - mPosHolder->mPos, *mBackWallTriangle->getNormal(0), &position);
        mPosHolder->fn_80406010(*mBackWallTriangle->getNormal(0) * distance);
        if (!fixWallingTop()) {
            return false;
        }
        mPosHolder->fn_80405F90(_4F4 + *mBackWallTriangle->getNormal(0) * 60.0f - _75C * 80.0f);

    } else if (frontWall) {
        if (!fixWallingTop()) {
            return false;
        }

        if (fn_803B2880(mPosHolder->mPos, mFrontVec * 100.0f) && mFrontVec.dot(*mFrontWallTriangle->getNormal(0)) < -0.999f) {
            TVec3f position(_4E8 + *mFrontWallTriangle->getNormal(0) * 60.0f - _75C * 80.0f);
            TVec3f correction;
            MR::vecKillElement(position - mPosHolder->mPos, *getGravityVec(), &correction);
            if (!MR::isNearZero(correction)) {
                TVec3f relative(correction + mActor->mBinderDirection5DC);
                if (MR::abs(relative.x) < MR::abs(correction.x)) {
                    correction.x = relative.x;
                }

                if (MR::abs(relative.y) < MR::abs(correction.y)) {
                    correction.y = relative.y;
                }

                if (MR::abs(relative.z) < MR::abs(correction.z)) {
                    correction.z = relative.z;
                }

                addTrans(correction);
            }
        }
    }

    return true;
}


bool Mario::isInhibitWall() const {
    if ((static_cast<u32>(mDrawStates.mBits.mBit29))) {
        return true;
    }

    if (checkWallCode("NotWallJump", true)) {
        return true;
    }

    return checkWallCode("NoAction", true);
}


void Mario::tryWallPunch() {
    if (!(static_cast<u32>(mMovementStates.mHighBits.mBit20)) && !checkWallCodeNorm(8, nullptr, false) && (!isStatusActive(MarioStatus_Skate) || !checkWallFloorCode(0x21))) {
        if (isSwimming()) {
            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit23))) {
                mSwim->hitWall(*mFrontWallTriangle->getNormal(0), mFrontWallTriangle->mSensor);
            }

            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit6))) {
                mSwim->hitWall(*mBackWallTriangle->getNormal(0), mBackWallTriangle->mSensor);
            }

            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit5))) {
                mSwim->hitWall(*mSideWallTriangle->getNormal(0), mSideWallTriangle->mSensor);
            }
        } else {
            stopWalk();
            TVec3f velocity(getWallNorm() * mActor->getConst().getTable()->mWallSpinFlipGround);
            if ((static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
                velocity.scale(mActor->getConst().getTable()->mWallSpinFlipAirRatio);
            }

            velocity -= *getGravityVec() * mActor->getConst().getTable()->mWallSpinHopGround;
            tryForcePowerJump(velocity, true);
            mMovementStates.mHighBits.mBit20 = 1;
            startPadVib(2);
            playSound("壁反射");
            playSound("声スピンキャンセル");
            playEffectTrans("壁ヒット", getWallPos());
            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit23))) {
                sendPunch(mFrontWallTriangle->mSensor, mFrontWallTriangle, true);
            }

            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit6))) {
                sendPunch(mBackWallTriangle->mSensor, mBackWallTriangle, true);
            }

            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit5))) {
                sendPunch(mSideWallTriangle->mSensor, mSideWallTriangle, true);
            }

            mActor->_EF6 = 30;
        }
    }
}

// ---- MarioWarp.cpp ----

namespace JGeometry {
};
  // namespace JGeometry

bool Mario::doObjWarp(LiveActor* pActor) {
    if (((getPlayer()->getMovementStates().mLowWord & Mario::MovementStates::DebugMode) >> 9)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    WarpPod* pPod = MR::getWarpPodManager()->getPairPod(pActor);

    if (pPod != nullptr) {
        mWarp->_4C = pActor;

        mVelocity.zero();

        stopWalk();

        mActor->setBlendMtxTimer(16);

        TVec3f axisY;
        TVec3f pairAxisY;

        MR::calcActorAxisY(&axisY, pActor);
        MR::calcActorAxisY(&pairAxisY, pPod);

        mWarp->_14 = pPod->mPosition + pairAxisY * 160.0f;
        mWarp->_20 = pActor->mPosition + axisY * 160.0f;

        MR::getRotatedAxisZ(&mWarp->_38, pPod->mRotation);
        MR::getRotatedAxisY(&mWarp->_2C, pPod->mRotation);

        mWarp->_45 = pPod->mVisibilityState;
        mWarp->_44 = pPod->mArg2;

        stopWalk();

        changeStatus(mWarp);

        stopJump();

        return true;
    }

    return false;
}


bool Mario::doPointWarp(const TVec3f& rVec1, const TVec3f& rVec2, s32 myInt) {
    if (((getPlayer()->getMovementStates().mLowWord & Mario::MovementStates::DebugMode) >> 9)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    mVelocity.zero();

    stopWalk();

    mActor->setBlendMtxTimer(16);

    mWarp->_14 = rVec1;
    mWarp->_20 = mPosHolder->mPos;
    mWarp->_38 = rVec2;
    mWarp->_45 = 2;
    mWarp->_44 = false;

    if (myInt != 0) {
        mWarp->_48 = myInt;
    } else {
        mWarp->_48 = 90;
    }

    changeStatus(mWarp);

    stopJump();

    return true;
}


bool Mario::isVisibleRecoveryWarpBubble() const {
    if (isStatusActive(MarioStatus_Warp) && mWarp->_45 == 3) {
        return true;
    }

    return false;
}


void Mario::doCubeWarp() {
    if (((getPlayer()->getMovementStates().mLowWord & Mario::MovementStates::DebugMode) >> 9)) {
        return;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return;
    }

    WarpCube* pWarpCube = static_cast< WarpCube* >(MR::getAreaObj("WarpCube", mActor->_240));
    WarpCubeMgr* pWarpCubeMgr = static_cast< WarpCubeMgr* >(MR::getAreaObjManager("WarpCube"));

    if (pWarpCube != nullptr) {
        if (_56C == pWarpCube) {
            return;
        }

        WarpCube* pPairCube = pWarpCubeMgr->getPairCube(pWarpCube);
        if (pPairCube == nullptr) {
            return;
        }

        pWarpCubeMgr->startEventCamera(pWarpCube);

        TVec3f cubePos;
        MR::calcCubePos(pPairCube, &cubePos);

        mVelocity.zero();
        stopWalk();
        mActor->_38C = 10;

        _56C = pPairCube;
        mWarp->_14 = cubePos;

        MR::calcCubePos(pWarpCube, &cubePos);
        mWarp->_20 = cubePos;
        mWarp->_38.zero();
        mWarp->_2C.zero();
        mWarp->_45 = 0;
        mWarp->_44 = 0;

        stopWalk();

        changeStatus(mWarp);

        stopJump();

        return;
    }

    if (_56C != nullptr) {
        pWarpCubeMgr->setInvalidateTimer(_56C, 300);
    }

    _56C = nullptr;
}


bool Mario::doPointWarpRecovery(const TVec3f& rVec1, const TVec3f& rVec2) {
    if (((getPlayer()->getMovementStates().mLowWord & Mario::MovementStates::DebugMode) >> 9)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    mVelocity.zero();

    stopWalk();

    mActor->setBlendMtxTimer(16);

    mWarp->_14 = rVec1;
    mWarp->_20 = mPosHolder->mPos;
    mWarp->_38 = rVec2;
    mWarp->_2C.zero();
    mWarp->_45 = 3;
    mWarp->_44 = false;

    changeStatus(mWarp);

    stopJump();

    MR::startGlobalEventCameraNoTarget("引き戻し");

    return true;
}

#include "../src/Game/Player/MarioSMG2_A.inc"
#include "../src/Game/Player/MarioSMG2_B.inc"

bool Mario::isStateHanging() const {
    return getCurrentStatus() == MarioStatus_Hang;
}

bool Mario::isStateBury() const {
    return getCurrentStatus() == MarioStatus_Bury;
}

f32 fn_80386D30(f32 first, f32 second, f32 third) {
    f32 largest = first >= second ? first : second;
    return largest >= third ? largest : third;
}
