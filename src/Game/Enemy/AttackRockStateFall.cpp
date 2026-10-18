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
#include "Game/Boss/KoopaFinalBattlePos.hpp"
#include "Game/Boss/KoopaJrRobot.hpp"

#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"

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
        mBreakModel = new KoopaJrRobotShockWave("\222n\220k\224g");
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
        MR::sendMsgKoopaPunchAttack(pReceiver, pSender);
        return;
    }
    if (MR::isEqualSensor(pSender, mHost, "Land") && mIsNear) {
        TVec3f dir;
        MR::calcSensorHorizonNormalize(&dir, mLandNormal, pSender, pReceiver);
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

// NONMATCHING: Commutative FP multiply operand order and FP register allocation differ.
void AttackRockStateFall::fn_80153DE0() {
    f32 dist = PSVECDistance(&this->mLandPos, &this->mHost->mPosition);
    AttackRock* pHost = this->mHost;
    f32 fnRadius = pHost->fn_801510C0();
    f32 radius = 1.5f + fnRadius * pHost->mScale.y;
    if (dist < radius) {
        f32 fnBase = pHost->fn_801510C0();
        f32 base = fnBase * pHost->mScale.y;
        f32 half = 0.5f * base;
        MR::setSensorRadius(this->mHost, "Land", MR::getLinerValueFromMinMax(dist, 0.0f, radius, pHost->fn_801510C0() * pHost->mScale.y, half));
        this->mIsNear = true;
        return;
    }
    this->mIsNear = false;
}

#pragma auto_inline off
// NONMATCHING: Vector expression temporaries and inlining differ.
void AttackRockStateFall::fn_80153EE0() {
    TVec3f start(this->mHost->mPosition);
    f32 t = MR::calcNerveRate(this, 110);
    f32 s = 1.0f - t;
    f32 b = s * s;
    f32 m = 2.0f * t * s;
    TVec3f p0 = this->mLandPos * (t * t);
    TVec3f p1 = this->mStartPos * m;
    TVec3f p2 = this->mFallPos * b;
    TVec3f sum = (p2 + p1) + p0;
    TVec3f pos;
    pos.set(sum);
    TVec3f vel = pos - start;
    MR::setVelocity(this->mHost, vel);
}
#pragma auto_inline on

void AttackRockStateFall::fn_80153FE0() {
    AttackRock* pHost = this->mHost;
    LiveActor* pDemo = pHost->mDemoObj;
    MR::setQuatUpNoSupport(pDemo, this->mLandNormal);
    MR::resetPosition(pDemo, this->mLandPos);
    MR::startAction(pDemo, "Appear");
    MR::invalidateCollisionParts(pDemo);
    pDemo->appear();
}

#pragma auto_inline off
// NONMATCHING: Commutative floating-point multiply operand order differs.
void AttackRockStateFall::fn_80154060() {
    if (MR::isFirstStep(this)) {
        this->mIsNear = false;
        MR::startAction(this->mHost, "Wait");
        MR::invalidateCollisionParts(this->mHost);
    }
    TVec3f move = *MR::getPlayerLastMove() * 60.0f;
    TVec3f playerPos;
    playerPos.set(*MR::getPlayerPos() + move);
    MR::calcKoopaBattleEdgePos(&mLandPos, playerPos);
    MR::calcKoopaBattleDirFromBase(&mLandNormal, playerPos);
    MR::makeQuatUpNoSupport(&this->mHost->mQuatF0, this->mLandNormal);
    MR::setPosition(this->mVolumeDrawer, this->mLandPos);
    MR::setShadowProjection(this->mHost, nullptr, this->mLandPos, this->mLandNormal, true);
    this->mHost->mGravity.set(-this->mLandNormal);
    this->mStartPos.set(this->mLandPos + this->mLandNormal * 4000.0f);
    TVec3f toHost = this->mHost->mPosition - this->mLandPos;
    TVec3f horizon;
    JMAVECScaleAdd(&mLandNormal, &toHost, &horizon, -mLandNormal.dot(toHost));
    if (MR::normalizeOrZero(&horizon)) {
        mFallPos.set(mLandPos + mLandNormal * 6000.0f);
    } else {
        mFallPos.set(mStartPos + horizon * 2000.0f);
    }
    MR::resetPosition(this->mHost, this->mFallPos);
    this->setNerve(&NrvAttackRockStateFall::AttackRockStateFallNrvWait::sInstance);
    MR::showModel(this->mHost);
    MR::validateHitSensors(this->mHost);
    AttackRock* pHost = this->mHost;
    MR::setShadowVolumeSphereRadius(this->mHost, nullptr, 0.05f * pHost->fn_801510C0() * pHost->mScale.y);
}
#pragma auto_inline on

// NONMATCHING: Host reloads, GPR allocation and floating-point scheduling differ.
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
    f32 base = 0.05f * pHost->fn_801510C0() * pHost->mScale.y;
    f32 radius = MR::calcNerveEaseInOutValue(this->mHost, 88, base, 0.95f * pHost->fn_801510C0() * pHost->mScale.y);
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

// NONMATCHING: Host load is scheduled after the action-name selection.
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
            this->mBreakModel->start(this->mHost->mPosition);
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

