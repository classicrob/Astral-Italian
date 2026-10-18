#include "Game/Enemy/AttackRock.hpp"
f32 fn_801510C0();
bool fn_801510D0(LiveActor* pActor);
bool fn_801517C0(LiveActor* pActor, s32 arg);
void fn_801552A0(LiveActor* pActor);
f32 fn_801552E0(LiveActor* pActor, TQuat4f* pQuat);
void fn_80155360(TVec3f* pOut, TQuat4f* pQuat, HitSensor* pSender, HitSensor* pReceiver, f32 speed);
void fn_801553D0(TVec3f* pOut, TVec3f* pDir, HitSensor* pSender, HitSensor* pReceiver, f32 speed);
void fn_801554D0(TPos3f* pMtx, LiveActor* pActor);
void fn_80155540(ProjmapEffectMtxSetter* pSetter, TQuat4f* pQuat, TVec3f* pPos);
ProjmapEffectMtxSetter* fn_80155660(LiveActor* pActor);
#include "Game/Util.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/Boss/KoopaFinalBattlePos.hpp"
#include "Game/Boss/KoopaJrRobot.hpp"

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

// Unit-local helpers (unnamed in the target).

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
    if (MR::isMsgKoopaBA(msg)) {
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
void AttackRockStateFall::fn_80153DE0() {
    AttackRock* pHost = this->mHost;
    f32 dist = PSVECDistance(&this->mLandPos, &pHost->mPosition);
    f32 radius = 1.5f + fn_801510C0() * pHost->mScale.x;
    if (dist < radius) {
        f32 scale = pHost->mScale.x;
        f32 base = fn_801510C0() * scale;
        MR::setSensorRadius(this->mHost, "Land",
                            MR::getLinerValueFromMinMax(dist, 0.0f, radius, fn_801510C0() * pHost->mScale.x, 0.5f * base));
        this->mIsNear = true;
        return;
    }
    this->mIsNear = false;
}

// NONMATCHING: operator*/operator+ out-of-line vs inline decisions differ (target was a separate TU).
void AttackRockStateFall::fn_80153EE0() {
    TVec3f start(this->mHost->mPosition);
    f32 t = MR::calcNerveRate(this, 110);
    f32 s = 1.0f - t;
    f32 b = s * s;
    f32 m = 2.0f * t * s;
    TVec3f p0 = start * (t * t);
    TVec3f p1 = this->mFallPos * m;
    TVec3f p2 = this->mLandPos * b;
    TVec3f sum = p0 + p2;
    sum = sum + p1;
    TVec3f pos(sum);
    TVec3f vel = pos - this->mHost->mPosition;
    MR::setVelocity(this->mHost, vel);
}

void AttackRockStateFall::fn_80153FE0() {
    AttackRock* pHost = this->mHost;
    LiveActor* pDemo = pHost->mDemoObj;
    MR::setQuatUpNoSupport(pDemo, this->mLandNormal);
    MR::resetPosition(pDemo, this->mLandPos);
    MR::startAction(pDemo, "Appear");
    MR::invalidateCollisionParts(pDemo);
    pDemo->appear();
}

// NONMATCHING: written from the m2c output; stack/temporaries not verified.
void AttackRockStateFall::fn_80154060() {
    if (MR::isFirstStep(this)) {
        this->mIsNear = false;
        MR::startAction(this->mHost, "Wait");
        MR::invalidateCollisionParts(this->mHost);
    }
    TVec3f playerPos = *MR::getPlayerPos() + *MR::getPlayerLastMove() * 60.0f;
    TVec3f ground;
    MR::makeQuatUpNoSupport(&this->mHost->mQuatF0, this->mLandNormal);
    MR::setPosition(this->mVolumeDrawer, this->mLandPos);
    MR::setShadowProjection(this->mHost, nullptr, this->mLandPos, this->mLandNormal, true);
    this->mHost->mGravity.set(-this->mLandNormal);
    this->mFallPos.set(this->mLandPos + this->mLandNormal * 4000.0f);
    TVec3f toHost = this->mHost->mPosition - ground;
    this->mStartPos.set(this->mLandPos);
    MR::resetPosition(this->mHost, this->mStartPos);
    MR::showModel(this->mHost);
    MR::validateHitSensors(this->mHost);
    this->setNerve(&NrvAttackRockStateFall::AttackRockStateFallNrvWait::sInstance);
    AttackRock* pHost = this->mHost;
    MR::setShadowVolumeSphereRadius(this->mHost, nullptr, 0.05f * fn_801510C0() * pHost->mScale.x);
}

// NONMATCHING: written from the m2c output; stack/temporaries not verified.
void AttackRockStateFall::fn_80154280() {
    if (MR::isFirstStep(this)) {
        const char* pName = "Fall";
        if (this->mBreakModel != nullptr) {
            pName = "FallWave";
        }
        MR::startAction(this->mHost, pName);
        MR::invalidateShadow(this->mHost, nullptr);
        this->mIsNear = false;
        this->mVolumeDrawer->setScaleUpRate(60);
        MR::setShadowProjection(this->mHost, nullptr, this->mLandPos, this->mLandNormal, true);
    }
    AttackRock* pHost = this->mHost;
    f32 base = 0.05f * fn_801510C0() * pHost->mScale.x;
    f32 radius = MR::calcNerveEaseInOutValue(this->mHost, 88, base, 0.95f * fn_801510C0() * pHost->mScale.x);
    MR::setShadowVolumeSphereRadius(this->mHost, nullptr, radius);
    MR::setScale(this->mVolumeDrawer, TVec3f(1.2f * radius / 100.0f));
    this->fn_80153EE0();
    this->fn_80153DE0();
    if (MR::isGreaterEqualStep(this, 110)) {
        MR::setPosition(this->mHost, this->mLandPos);
        MR::zeroVelocity(this->mHost);
        this->mVolumeDrawer->setScaleDownRate(50);
        this->fn_80153FE0();
        this->setNerve(&NrvAttackRockStateFall::AttackRockStateFallNrvFall::sInstance);
    }
}

void AttackRockStateFall::fn_80154400() {
    if (MR::isFirstStep(this)) {
        const char* pName = "Land";
        if (this->mBreakModel != nullptr) {
            pName = "LandWave";
        }
        MR::startAction(this->mHost, pName);
        MR::setPosition(this->mHost, this->mLandPos);
        MR::zeroVelocity(this->mHost);
        MR::invalidateShadow(this->mHost, nullptr);
        MR::validateCollisionParts(this->mHost);
        MR::validateCollisionParts(this->mHost->mDemoObj);
        if (this->mBreakModel != nullptr) {
            reinterpret_cast< KoopaJrRobotShockWave* >(this->mBreakModel)->start(this->mHost->mPosition);
        }
    }
    if (MR::isGreaterEqualStep(this, 58)) {
        this->kill();
    }
}

namespace NrvAttackRockStateFall {
    inline void AttackRockStateFallNrvAppear::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockStateFall* >(pSpine->mExecutor))->fn_80154060();
    }
    inline void AttackRockStateFallNrvWait::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockStateFall* >(pSpine->mExecutor))->fn_80154280();
    }
    inline void AttackRockStateFallNrvFall::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockStateFall* >(pSpine->mExecutor))->fn_80154400();
    }
};  // namespace NrvAttackRockStateFall

// ---- AttackRockTutorial ----

