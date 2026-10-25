#define JGEOMETRY_VEC3_MUL_NOINLINE
#include "Game/Util/BossBussun.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/JointController.hpp"

namespace NrvBossBussunShot {
    class BossBussunShotNrvFly : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunShotNrvFly sInstance;
    };
    class BossBussunShotNrvHold : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunShotNrvHold sInstance;
    };
    class BossBussunShotNrvLandWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunShotNrvLandWait sInstance;
    };
    class BossBussunShotNrvLand : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunShotNrvLand sInstance;
    };
    class BossBussunShotNrvBind : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunShotNrvBind sInstance;
    };
    class BossBussunShotNrvBreak : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunShotNrvBreak sInstance;
    };

    BossBussunShotNrvFly BossBussunShotNrvFly::sInstance ATTRIBUTE_WEAK;
    BossBussunShotNrvHold BossBussunShotNrvHold::sInstance ATTRIBUTE_WEAK;
    BossBussunShotNrvLandWait BossBussunShotNrvLandWait::sInstance ATTRIBUTE_WEAK;
    BossBussunShotNrvLand BossBussunShotNrvLand::sInstance ATTRIBUTE_WEAK;
    BossBussunShotNrvBind BossBussunShotNrvBind::sInstance ATTRIBUTE_WEAK;
    BossBussunShotNrvBreak BossBussunShotNrvBreak::sInstance ATTRIBUTE_WEAK;

    inline void BossBussunShotNrvFly::execute(Spine* pSpine) const {
        reinterpret_cast< BossBussunShot* >(pSpine->mExecutor)->exeFly();
    }
    inline void BossBussunShotNrvHold::execute(Spine* pSpine) const {
        reinterpret_cast< BossBussunShot* >(pSpine->mExecutor)->exeHold();
    }
    inline void BossBussunShotNrvLandWait::execute(Spine* pSpine) const {
        reinterpret_cast< BossBussunShot* >(pSpine->mExecutor)->exeLandWait();
    }
    inline void BossBussunShotNrvLand::execute(Spine* pSpine) const {
        reinterpret_cast< BossBussunShot* >(pSpine->mExecutor)->exeLand();
    }
    inline void BossBussunShotNrvBind::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< BossBussunShot* >(pSpine->mExecutor), &BossBussunShotNrvFly::sInstance);
    }
    inline void BossBussunShotNrvBreak::execute(Spine* pSpine) const {
        reinterpret_cast< BossBussunShot* >(pSpine->mExecutor)->exeBreak();
    }
};  // namespace NrvBossBussunShot

BossBussunShot::BossBussunShot(const char* pName)
    : LiveActor(pName), mBindState(nullptr), mHoldSensor(nullptr), mJointCtrl(nullptr), mQuatA(0.0f, 0.0f, 0.0f, 1.0f),
      mQuatB(0.0f, 0.0f, 0.0f, 1.0f), _C0(0.0f, 0.0f, 0.0f), mHoldPos(0.0f, 0.0f, 0.0f), mHoldVel(0.0f, 0.0f, 0.0f), _F0(20), mGroundTimer(0),
      mStep(0) {
}

void BossBussunShot::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "BossBussunShot", false);
    MR::setBinderExceptSensorType(this, 0x55);
    MR::invalidateCollisionParts(this);
    mAnimScale = new AnimScaleController(nullptr);
    mBindState = new WalkerStateBindStarPointer(this, mAnimScale);
    mJointCtrl = MR::createJointDelegatorWithNullChildFunc(this, &BossBussunShot::calcJointMtx, "BossBussunShot");
    initNerve(&NrvBossBussunShot::BossBussunShotNrvFly::sInstance, 1);
    MR::initActorState(this, mBindState, &NrvBossBussunShot::BossBussunShotNrvBind::sInstance, "SupportFreeze");
    makeActorDead();
}

void BossBussunShot::initAfterPlacement() {
    MR::trySetMoveLimitCollision(this);
}

void BossBussunShot::makeActorDead() {
    LiveActor::makeActorDead();
}

void BossBussunShot::control() {
    mAnimScale->updateNerve();
    if (MR::isBindedGround(this)) {
        mGroundTimer = 10;
    } else if (mGroundTimer > 0) {
        mGroundTimer--;
    }
    MR::setScale(this, MR::getEaseInOutValue(static_cast< f32 >(mStep), 0.0f, 6.0f, 0.5f, 1.0f));
    mStep++;
}

void BossBussunShot::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuatA);
    MR::updateBaseScale(this, mAnimScale);
    mJointCtrl->registerCallBack();
}

bool BossBussunShot::calcJointMtx(TPos3f* pMtx, const JointControllerInfo&) {
    if (isNerve(&NrvBossBussunShot::BossBussunShotNrvLand::sInstance) || isNerve(&NrvBossBussunShot::BossBussunShotNrvLandWait::sInstance)) {
        return false;
    }

    TPos3f mtxA;
    mtxA.setQT(mQuatA, TVec3f(0.0f));
    mtxA.invert(mtxA);
    TPos3f mtxB;
    mtxB.setQT(mQuatB, TVec3f(0.0f));
    mtxB.concat(mtxA, mtxB);
    pMtx->concat(*pMtx, mtxB);
    return true;
}

void BossBussunShot::shoot(const TVec3f& rPos, const TVec3f& rTarget) {
    MR::resetPosition(this, rPos);
    MR::validateShadowAll(this);
    mHoldSensor = nullptr;
    MR::onBind(this);
    mStep = 0;
    MR::setScale(this, 0.5f);
    MR::makeQuatUpNoSupport(&mQuatA, -mGravity);
    appear();
    MR::invalidateClipping(this);
    MR::invalidateCollisionParts(this);
    MR::calcVecToTargetPosH(&mMoveDir, this, rTarget, nullptr);
    MR::zeroVelocity(this);
    setNerve(&NrvBossBussunShot::BossBussunShotNrvFly::sInstance);
}

void BossBussunShot::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvBossBussunShot::BossBussunShotNrvFly::sInstance) || isNerve(&NrvBossBussunShot::BossBussunShotNrvBind::sInstance)) {
        if (MR::isEqualSensor(pSender, this, "Body") && MR::isSensorPlayer(pReceiver)) {
            MR::sendMsgEnemyAttackFire(pReceiver, pSender);
            return;
        }
    }

    if ((isNerve(&NrvBossBussunShot::BossBussunShotNrvFly::sInstance) || isNerve(&NrvBossBussunShot::BossBussunShotNrvHold::sInstance) ||
         isNerve(&NrvBossBussunShot::BossBussunShotNrvLand::sInstance) || isNerve(&NrvBossBussunShot::BossBussunShotNrvLandWait::sInstance) ||
         isNerve(&NrvBossBussunShot::BossBussunShotNrvBreak::sInstance)) &&
        MR::isEqualSensor(pSender, this, "Body")) {
        if (MR::sendArbitraryMsg(0xD1, pReceiver, pSender)) {
            mHoldSensor = pReceiver;
        }
        if (isNerve(&NrvBossBussunShot::BossBussunShotNrvLand::sInstance) || isNerve(&NrvBossBussunShot::BossBussunShotNrvLandWait::sInstance) ||
            isNerve(&NrvBossBussunShot::BossBussunShotNrvHold::sInstance) || isNerve(&NrvBossBussunShot::BossBussunShotNrvBreak::sInstance)) {
            MR::sendArbitraryMsg(0xD2, pReceiver, pSender);
        }
    }
}

// NONMATCHING: register/stack or instruction scheduling (FP order/grouping proven by default rdclass a).
void BossBussunShot::exeFly() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shot");
        TVec3f vel = mMoveDir * 30.0f;
        MR::setVelocity(this, vel);
    }
    MR::addVelocityMoveToDirectionHorizon(this, mMoveDir, 1.0f);
    if (!MR::isBindedGround(this)) {
        MR::addVelocityToGravity(this, 1.2f);
    }
    if (MR::reboundVelocityFromCollision(this, 0.8f, 10.0f, 1.0f)) {
        mAnimScale->startScaleVelocityY(-0.08f);
    }
    MR::attenuateVelocity(this, 0.93f);
    MR::turnVecToPlane(&mMoveDir, mMoveDir, mGravity);
    MR::rotateQuatRollBall(&mQuatB, mVelocity, -mGravity, 150.0f);
    if (MR::isBindedGround(this) && mHoldSensor != nullptr) {
        mHoldPos.set(*MR::getSensorPos(mHoldSensor));
        setNerve(&NrvBossBussunShot::BossBussunShotNrvHold::sInstance);
        return;
    }
    if (mGroundTimer > 0) {
        MR::startActionSound(this, "Shot", -1, -1, -1);
    } else if (MR::isGreaterStep(this, 60)) {
        MR::startActionSound(this, "Fly", -1, -1, -1);
    }
    if (MR::isGreaterStep(this, 300)) {
        kill();
    }
    if (mBindState->tryStartPointBind()) {
        setNerve(&NrvBossBussunShot::BossBussunShotNrvBind::sInstance);
    }
}

// NONMATCHING: register/stack or instruction scheduling (FP order/grouping proven by default rdclass a).
void BossBussunShot::exeHold() {
    TVec3f dir;
    TVec3f dir2;

    if (MR::isFirstStep(this)) {
        mHoldVel.set(mVelocity);
        MR::zeroVelocity(this);
        MR::offBind(this);
    }
    MR::calcDirToTarget(&dir, this, &mHoldPos);
    f32 prevDist = MR::calcDistance(this, mHoldPos);
    TVec3f accel = dir * 3.0f;
    mHoldVel.add(accel);
    mHoldVel.x *= 0.88f;
    mHoldVel.y *= 0.88f;
    mHoldVel.z *= 0.88f;
    mPosition.add(mHoldVel);
    f32 dist = MR::calcDistance(this, mHoldPos);
    f32 limit = 0.95f * prevDist;
    if (limit < dist) {
        f32 over = dist - limit;
        MR::calcDirToTarget(&dir2, this, &mHoldPos);
        TVec3f push = dir2 * over;
        mHoldVel.add(push);
        TVec3f move = dir2 * over;
        mPosition.add(move);
    }
    MR::rotateQuatRollBall(&mQuatB, mHoldVel, -mGravity, 150.0f);
    if (dist < 25.0f) {
        setNerve(&NrvBossBussunShot::BossBussunShotNrvLand::sInstance);
    }
}

void BossBussunShot::exeLand() {
    if (MR::isFirstStep(this)) {
        MR::resetPosition(this, mHoldPos);
        MR::makeQuatUpNoSupport(&mQuatA, -mGravity);
        MR::invalidateShadowAll(this);
        MR::startAction(this, "Land");
        MR::tryHitReaction(this, "着弾");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvBossBussunShot::BossBussunShotNrvLandWait::sInstance);
    }
}

void BossBussunShot::exeLandWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "LandWait");
        MR::validateCollisionParts(this);
    }
    if (MR::isGreaterStep(this, 540)) {
        MR::invalidateCollisionParts(this);
        setNerve(&NrvBossBussunShot::BossBussunShotNrvBreak::sInstance);
    }
}

void BossBussunShot::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Break");
    }
    if (MR::isActionEnd(this)) {
        kill();
    }
}

void BossBussunFunction::setBinderExceptPlayer(LiveActor* pActor) {
    MR::setBinderExceptSensorType(pActor, 0x55);
}

BossBussunShot::~BossBussunShot() {
}
