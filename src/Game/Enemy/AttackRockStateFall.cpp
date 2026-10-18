#include "Game/Enemy/AttackRock.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Util/FixedPosition.hpp"

class BaseMatrixFollowValidater;
class KoopaBattleAccessor {
public:
    void addAttackRockFinal(AttackRockFinal*);
};

namespace MR {
    bool isMsgKoopaCastAttackRockStart(u32);
    bool isMsgKoopaCastAttackRockEnd(u32);
    bool isMsgKoopaPunchAttack(u32);
    bool isMsgKoopaReleaseAttackRock(u32);
    bool isMsgKoopaFinalBattleStart(u32);
    bool isMsgAutoRushBegin(u32);
    bool isMsgUpdateBaseMtx(u32);
    bool isMsgRushCancel(u32);
    bool isMsgFloorTouch(u32);
    f32 calcNerveEaseOutValue(const NerveExecutor*, s32, s32, f32, f32);
    void createKoopaBattleAccessor();
    KoopaBattleAccessor* getKoopaBattleAccessor();
};  // namespace MR

// Unnamed SMG2 helpers outside this unit (Koopa battle functions).
extern "C" void fn_800CB710(HitSensor*);
extern "C" void fn_800CB090(TVec3f*);
extern "C" void fn_800CB6D0(TVec3f*);
extern "C" f32 fn_800CB410(const TVec3f*);
extern "C" void fn_800CAF80(TVec3f*, const TVec3f*);
extern "C" void fn_800CAFE0(TVec3f*, const TVec3f*);
extern "C" void fn_800CB460(TVec3f*);
extern "C" void fn_800CC210(HitSensor*, HitSensor*);
extern "C" bool fn_800CC1F0(HitSensor*, HitSensor*);
extern "C" void fn_800CC220(LiveActor*, HitSensor*);
extern "C" bool fn_800CC230(LiveActor*, HitSensor*);
extern "C" bool fn_800CC270(u32);
extern "C" bool fn_800CC280(u32);
extern "C" bool fn_800CC290(u32);
extern "C" bool fn_800CC2B0(u32);
extern "C" void fn_800E4420(void*);
extern "C" void fn_800E4700(const TVec3f*);
extern "C" void fn_80010020(LiveActor*, const TVec3f*, LiveActor*);
extern "C" void fn_800513C0(TQuat4f*, TVec3f*);
extern "C" TVec3f* fn_800514B0(TQuat4f*, TVec3f*);
extern "C" void fn_8000E850(TVec3f*, const LiveActor*);

// Unit-local helpers (unnamed in the target).
extern "C" {
f32 fn_801510C0() NO_INLINE;
bool fn_801510D0(LiveActor*) NO_INLINE;
void fn_80151160(AttackRock*) NO_INLINE;
bool fn_801517C0(LiveActor*, s32) NO_INLINE;
void fn_80151850(void*) NO_INLINE;
void fn_801552A0(LiveActor*) NO_INLINE;
void fn_80155360(TVec3f*, TQuat4f*, HitSensor*, HitSensor*, f32) NO_INLINE;
void fn_801553D0(TVec3f*, TVec3f*, HitSensor*, HitSensor*, f32) NO_INLINE;
void fn_801554D0(TPos3f*, LiveActor*) NO_INLINE;
void fn_80155540(ProjmapEffectMtxSetter*, TQuat4f*, TVec3f*) NO_INLINE;
ProjmapEffectMtxSetter* fn_80155660(LiveActor*) NO_INLINE;
void fn_80152F10(AttackRockRide*) NO_INLINE;
void fn_80152F20(AttackRockRide*) NO_INLINE;
bool fn_80152E20(AttackRockRide*, u32, HitSensor*, HitSensor*) NO_INLINE;
void fn_80152A00(AttackRockRide*) NO_INLINE;
void fn_80152C00(AttackRockRide*) NO_INLINE;
void fn_801529E0(AttackRockRide*, const TVec3f*) NO_INLINE;
AttackRockRide* fn_801528E0(AttackRockRide*, LiveActor*) NO_INLINE;
bool fn_80153400(AttackRockStateHold*) NO_INLINE;
bool fn_80153460(AttackRockStateHold*) NO_INLINE;
f32 fn_801552E0(LiveActor*, TQuat4f*) NO_INLINE;
}

namespace NrvAttackRockStateFall {
    class AttackRockStateFallNrvAppear : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockStateFallNrvAppear sInstance;
    };
    class AttackRockStateFallNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockStateFallNrvWait sInstance;
    };
    class AttackRockStateFallNrvFall : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockStateFallNrvFall sInstance;
    };
    AttackRockStateFallNrvAppear AttackRockStateFallNrvAppear::sInstance ATTRIBUTE_WEAK;
    AttackRockStateFallNrvWait AttackRockStateFallNrvWait::sInstance ATTRIBUTE_WEAK;
    AttackRockStateFallNrvFall AttackRockStateFallNrvFall::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvAttackRockStateFall

extern "C" {
void fn_80153DE0(AttackRockStateFall*) NO_INLINE;
void fn_80153EE0(AttackRockStateFall*) NO_INLINE;
void fn_80153FE0(AttackRockStateFall*) NO_INLINE;
void fn_80154060(AttackRockStateFall*) NO_INLINE;
void fn_80154280(AttackRockStateFall*) NO_INLINE;
void fn_80154400(AttackRockStateFall*) NO_INLINE;
}

AttackRockStateFall::AttackRockStateFall(AttackRock* pRock, s32 rockNum)
    : ActorStateBase< AttackRock >("\215U\214\202\212\342\203N\203b\203p\215U\214\202", pRock), mBreakModel(nullptr), mVolumeDrawer(nullptr), mFallPos(0.0f, 0.0f, 0.0f),
      mStartPos(0.0f, 0.0f, 0.0f), mLandPos(0.0f, 0.0f, 0.0f), mLandNormal(0.0f, 1.0f, 0.0f), mIsNear(false) {
    initNerve(&NrvAttackRockStateFall::AttackRockStateFallNrvAppear::sInstance, 0);
    if (rockNum >= 2) {
        mBreakModel = new BreakModel(mHost, "\222n\220k\224g", nullptr, nullptr, nullptr);
        mBreakModel->initWithoutIter();
    }
    MR::setSensorFollowPos(mHost, "Land", &mLandPos);
    mVolumeDrawer = MR::createVolumeDrawer("\222\205\222n\225`\211\346", "VolumeSphere", nullptr);
    Color8 color(0xF0, 0x64, 0x28, 0x7F);
    mVolumeDrawer->setColor(color);
    mVolumeDrawer->setDrawPasses(5);
    mVolumeDrawer->setScale(TVec3f(0.05f));
    mVolumeDrawer->setScaleDownRate(-1);
    MR::setPosition(mVolumeDrawer, mLandPos);
    MR::setScale(mVolumeDrawer, TVec3f(4.0f));
}

AttackRockStateFall::~AttackRockStateFall() {
}

void AttackRockStateFall::appear() {
    mIsDead = false;
    setNerve(&NrvAttackRockStateFall::AttackRockStateFallNrvAppear::sInstance);
    mIsNear = false;
}

void AttackRockStateFall::kill() {
    mIsDead = true;
    mVolumeDrawer->setScaleDownRate(-1);
}

void AttackRockStateFall::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, mHost, "Body") && isNerve(&NrvAttackRockStateFall::AttackRockStateFallNrvWait::sInstance)) {
        MR::sendMsgEnemyAttackStrongOrPush(pReceiver, pSender);
        fn_800CC210(pReceiver, pSender);
        return;
    }
    if (MR::isEqualSensor(pSender, mHost, "Land") && mIsNear) {
        TVec3f dir;
        if (MR::isNearZero(dir, 0.001f)) {
            MR::makeAxisVerticalZX(&dir, mLandNormal);
        }
        if (!isNerve(&NrvAttackRockStateFall::AttackRockStateFallNrvWait::sInstance) ||
            !MR::sendMsgEnemyAttackStrongToDir(pReceiver, pSender, dir)) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool AttackRockStateFall::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (fn_800CC280(msg)) {
        if (mBreakModel != nullptr) {
            mBreakModel->makeActorDead();
        }
        kill();
        return true;
    }
    if (MR::isMsgKoopaCastAttackRockEnd(msg) && isNerve(&NrvAttackRockStateFall::AttackRockStateFallNrvAppear::sInstance)) {
        kill();
        return true;
    }
    return false;
}

// NONMATCHING: written from the m2c output; stack/temporaries not verified.
extern "C" void fn_80153DE0(AttackRockStateFall* pState) {
    AttackRock* pHost = pState->mHost;
    f32 dist = PSVECDistance(&pState->mLandPos, &pHost->mPosition);
    f32 radius = 1.5f + fn_801510C0() * pHost->mScale.x;
    if (dist < radius) {
        f32 scale = pHost->mScale.x;
        f32 base = fn_801510C0() * scale;
        MR::setSensorRadius(pState->mHost, "Land",
                            MR::getLinerValueFromMinMax(dist, 0.0f, radius, fn_801510C0() * pHost->mScale.x, 0.5f * base));
        pState->mIsNear = true;
        return;
    }
    pState->mIsNear = false;
}

// NONMATCHING: operator*/operator+ out-of-line vs inline decisions differ (target was a separate TU).
extern "C" void fn_80153EE0(AttackRockStateFall* pState) {
    TVec3f start(pState->mHost->mPosition);
    f32 t = MR::calcNerveRate(pState, 110);
    f32 s = 1.0f - t;
    f32 b = s * s;
    f32 m = 2.0f * t * s;
    TVec3f p0 = start * (t * t);
    TVec3f p1 = pState->mFallPos * m;
    TVec3f p2 = pState->mLandPos * b;
    TVec3f sum = p0 + p2;
    sum = sum + p1;
    TVec3f pos(sum);
    TVec3f vel = pos - pState->mHost->mPosition;
    MR::setVelocity(pState->mHost, vel);
}

extern "C" void fn_80153FE0(AttackRockStateFall* pState) {
    AttackRock* pHost = pState->mHost;
    LiveActor* pDemo = pHost->mDemoObj;
    fn_80010020(pDemo, &pState->mLandNormal, pHost);
    MR::resetPosition(pDemo, pState->mLandPos);
    MR::startAction(pDemo, "Appear");
    MR::invalidateCollisionParts(pDemo);
    pDemo->appear();
}

// NONMATCHING: written from the m2c output; stack/temporaries not verified.
extern "C" void fn_80154060(AttackRockStateFall* pState) {
    if (MR::isFirstStep(pState)) {
        pState->mIsNear = false;
        MR::startAction(pState->mHost, "Wait");
        MR::invalidateCollisionParts(pState->mHost);
    }
    TVec3f playerPos = *MR::getPlayerPos() + *MR::getPlayerLastMove() * 60.0f;
    TVec3f ground;
    fn_800CAFE0(&pState->mLandPos, &playerPos);
    fn_800CAF80(&pState->mLandNormal, &playerPos);
    MR::makeQuatUpNoSupport(&pState->mHost->mQuatF0, pState->mLandNormal);
    MR::setPosition(pState->mVolumeDrawer, pState->mLandPos);
    MR::setShadowProjection(pState->mHost, nullptr, pState->mLandPos, pState->mLandNormal, true);
    pState->mHost->mGravity.set(-pState->mLandNormal);
    pState->mFallPos.set(pState->mLandPos + pState->mLandNormal * 4000.0f);
    TVec3f toHost = pState->mHost->mPosition - ground;
    pState->mStartPos.set(pState->mLandPos);
    MR::resetPosition(pState->mHost, pState->mStartPos);
    MR::showModel(pState->mHost);
    MR::validateHitSensors(pState->mHost);
    pState->setNerve(&NrvAttackRockStateFall::AttackRockStateFallNrvWait::sInstance);
    AttackRock* pHost = pState->mHost;
    MR::setShadowVolumeSphereRadius(pState->mHost, nullptr, 0.05f * fn_801510C0() * pHost->mScale.x);
}

// NONMATCHING: written from the m2c output; stack/temporaries not verified.
extern "C" void fn_80154280(AttackRockStateFall* pState) {
    if (MR::isFirstStep(pState)) {
        const char* pName = "Fall";
        if (pState->mBreakModel != nullptr) {
            pName = "FallWave";
        }
        MR::startAction(pState->mHost, pName);
        MR::invalidateShadow(pState->mHost, nullptr);
        pState->mIsNear = false;
        pState->mVolumeDrawer->setScaleUpRate(60);
        MR::setShadowProjection(pState->mHost, nullptr, pState->mLandPos, pState->mLandNormal, true);
    }
    AttackRock* pHost = pState->mHost;
    f32 base = 0.05f * fn_801510C0() * pHost->mScale.x;
    f32 radius = MR::calcNerveEaseInOutValue(pState->mHost, 88, base, 0.95f * fn_801510C0() * pHost->mScale.x);
    MR::setShadowVolumeSphereRadius(pState->mHost, nullptr, radius);
    MR::setScale(pState->mVolumeDrawer, TVec3f(1.2f * radius / 100.0f));
    fn_80153EE0(pState);
    fn_80153DE0(pState);
    if (MR::isGreaterEqualStep(pState, 110)) {
        MR::setPosition(pState->mHost, pState->mLandPos);
        MR::zeroVelocity(pState->mHost);
        pState->mVolumeDrawer->setScaleDownRate(50);
        fn_80153FE0(pState);
        pState->setNerve(&NrvAttackRockStateFall::AttackRockStateFallNrvFall::sInstance);
    }
}

extern "C" void fn_80154400(AttackRockStateFall* pState) {
    if (MR::isFirstStep(pState)) {
        const char* pName = "Land";
        if (pState->mBreakModel != nullptr) {
            pName = "LandWave";
        }
        MR::startAction(pState->mHost, pName);
        MR::setPosition(pState->mHost, pState->mLandPos);
        MR::zeroVelocity(pState->mHost);
        MR::invalidateShadow(pState->mHost, nullptr);
        MR::validateCollisionParts(pState->mHost);
        MR::validateCollisionParts(pState->mHost->mDemoObj);
        if (pState->mBreakModel != nullptr) {
            fn_800E4700(&pState->mHost->mPosition);
        }
    }
    if (MR::isGreaterEqualStep(pState, 58)) {
        pState->kill();
    }
}

namespace NrvAttackRockStateFall {
    inline void AttackRockStateFallNrvAppear::execute(Spine* pSpine) const {
        fn_80154060(reinterpret_cast< AttackRockStateFall* >(pSpine->mExecutor));
    }
    inline void AttackRockStateFallNrvWait::execute(Spine* pSpine) const {
        fn_80154280(reinterpret_cast< AttackRockStateFall* >(pSpine->mExecutor));
    }
    inline void AttackRockStateFallNrvFall::execute(Spine* pSpine) const {
        fn_80154400(reinterpret_cast< AttackRockStateFall* >(pSpine->mExecutor));
    }
};  // namespace NrvAttackRockStateFall

// ---- AttackRockTutorial ----

