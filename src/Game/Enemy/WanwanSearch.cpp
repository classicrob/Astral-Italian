#pragma auto_inline off
#include "Game/Enemy/Wanwan.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/MapObj/KeySwitch.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

WanwanSearch::WanwanSearch(const char* pName) : LiveActor(pName), mBindState(nullptr), mBreakModel(nullptr), mInitPos(0.0f, 0.0f, 0.0f), mQuat(0.0f, 0.0f, 0.0f, 1.0f), mFront(0.0f, 0.0f, 1.0f), mGoalPos(0.0f, 0.0f, 0.0f), mGoalUp(0.0f, 1.0f, 0.0f), mGoalFront(0.0f, 0.0f, 0.0f), mAccel(1.0f), mRadius(250.0f), mAirCount(0), mBreakOnDeath(false), mVoiceTimer(0), mLastPos(0.0f, 0.0f, 0.0f) {
    mMtx.identity();
}

namespace NrvWanwanSearch {
    NEW_NERVE(WanwanSearchNrvWait, WanwanSearch, Wait);
    NEW_NERVE(WanwanSearchNrvMove, WanwanSearch, Move);
    NEW_NERVE(WanwanSearchNrvGoal, WanwanSearch, Goal);
    NEW_NERVE(WanwanSearchNrvBreak, WanwanSearch, Break);

    class WanwanSearchNrvFreeze : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* pActor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(pActor, &WanwanSearchNrvWait::sInstance);
        }
        static WanwanSearchNrvFreeze sInstance;
    };
    WanwanSearchNrvFreeze WanwanSearchNrvFreeze::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvWanwanSearch

void WanwanSearch::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    mInitPos.set(mPosition);
    mLastPos.set(mPosition);
    if (MR::isObjectName(rIter, "WanwanSearchBig")) {
        mAccel = 3.0f;
        mRadius = 500.0f;
    }
    MR::setAllEffectHostMtx(this, mMtx);
    MR::setBinderExceptSensorType(this, 83);
    initNerve(&NrvWanwanSearch::WanwanSearchNrvWait::sInstance, 1);
    mBindState = new WalkerStateBindStarPointer(this, nullptr);
    MR::initActorState(this, mBindState, &NrvWanwanSearch::WanwanSearchNrvFreeze::sInstance, "SupportFreeze");
    MR::getJMapInfoArg0WithInit(rIter, &mBreakOnDeath);
    mBreakModel = MR::tryCreateBreakModel(this, nullptr);
    if (MR::useStageSwitchSyncAppear(this, rIter)) {
        MR::invalidateClipping(this);
        setNerve(&NrvWanwanSearch::WanwanSearchNrvMove::sInstance);
    }
}

void WanwanSearch::kill() {
    LiveActor::kill();
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
}

void WanwanSearch::control() {
    if (MR::isBindedGround(this)) {
        mAirCount = 0;
    } else {
        mAirCount++;
    }
    TVec3f up = -mGravity;
    MR::makeMtxUpNoSupportPos(&mMtx, up, mPosition);
    if (isMove2() && mBindState->tryStartPointBind()) {
        setNerve(&NrvWanwanSearch::WanwanSearchNrvFreeze::sInstance);
    }
}

void WanwanSearch::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
}

void WanwanSearch::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        if (!isNerve(&NrvWanwanSearch::WanwanSearchNrvMove::sInstance) || !MR::sendMsgEnemyAttack(pReceiver, pSender)) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    } else if (MR::isSensorRide(pReceiver) || MR::isSensorEnemy(pReceiver) || MR::isSensorMapObj(pReceiver)) {
        if (!MR::sendMsgToEnemyAttackTrampleDeath(pReceiver, pSender) && !MR::sendMsgToEnemyAttackRoll(pReceiver, pSender)) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool WanwanSearch::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }
    if (MR::isMsgInvincibleAttack(msg) && isMoveOrFreeze()) {
        MR::invalidateClipping(this);
        setNerve(&NrvWanwanSearch::WanwanSearchNrvBreak::sInstance);
        return true;
    }
    return false;
}

bool WanwanSearch::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isMove() && MR::isMsgToEnemyAttackRoll(msg)) {
        TVec3f horizon;
        MR::calcSensorHorizonNormalize(&horizon, mGravity, pSender, pReceiver);
        mFront.set(horizon);
        if (mVelocity.dot(horizon) < 20.0f) {
            MR::addVelocity(this, horizon * 20.0f);
            MR::startActionSound(this, "Rebound", (s32)(100.0f * PSVECMag(&mVelocity)), -1, -1);
        }
        return true;
    }
    if (MR::isMsgToEnemyAttackBlow(msg) && isMoveOrFreeze()) {
        MR::invalidateClipping(this);
        setNerve(&NrvWanwanSearch::WanwanSearchNrvBreak::sInstance);
        return true;
    }
    return false;
}

bool WanwanSearch::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if ((MR::isSensorEnemy(pSender) || MR::isSensorRide(pSender)) && isMove()) {
        TVec3f horizon;
        MR::calcSensorHorizonNormalize(&horizon, mGravity, pSender, pReceiver);
        if (mVelocity.dot(horizon) < 4.0f) {
            MR::addVelocity(this, horizon * 4.0f);
            return true;
        }
    }
    return false;
}

bool WanwanSearch::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 171 && isNerve(&NrvWanwanSearch::WanwanSearchNrvMove::sInstance)) {
        MR::invalidateClipping(this);
        MR::offBind(this);
        mGoalPos.set(MR::getSensorHostPos(pSender));
        mGoalUp.set(-MR::getSensorHostGravity(pSender));
        mGoalFront.set(mVelocity);
        MR::zeroVelocity(this);
        setNerve(&NrvWanwanSearch::WanwanSearchNrvGoal::sInstance);
        return true;
    }
    return false;
}

bool WanwanSearch::isMove() const {
    return isNerve(&NrvWanwanSearch::WanwanSearchNrvMove::sInstance);
}

bool WanwanSearch::isMoveOrFreeze() const {
    if (isNerve(&NrvWanwanSearch::WanwanSearchNrvMove::sInstance) || isNerve(&NrvWanwanSearch::WanwanSearchNrvFreeze::sInstance)) {
        return true;
    }
    return false;
}

bool WanwanSearch::isMove2() const {
    return isNerve(&NrvWanwanSearch::WanwanSearchNrvMove::sInstance);
}

void WanwanSearch::startVoice() {
    if (mVoiceTimer >= 120) {
        if (MR::calcDistanceToPlayer(this) <= 600.0f) {
            MR::startActionSound(this, "EvWanwanrollWanwan", -1, -1, -1);
            mVoiceTimer = 0;
        }
    } else {
        mVoiceTimer++;
    }
}

void WanwanSearch::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateClipping(this);
    }
    MR::zeroVelocity(this);
    if (MR::isGreaterStep(this, 1)) {
        MR::invalidateClipping(this);
        setNerve(&NrvWanwanSearch::WanwanSearchNrvMove::sInstance);
    }
}

// NONMATCHING: death-test and collision-vector temporary stack allocation.
void WanwanSearch::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Move");
    }
    if (MR::isInDeath(this, TVec3f(0, 0, 0))) {
        if (mBreakOnDeath) {
            setNerve(&NrvWanwanSearch::WanwanSearchNrvBreak::sInstance);
        } else {
            MR::resetPosition(this, mInitPos);
            setNerve(&NrvWanwanSearch::WanwanSearchNrvWait::sInstance);
        }
        return;
    }
    bool onGround = MR::isBindedGround(this);
    if (onGround) {
        f32 rate;
        if (MR::isBindedGroundSinkDeath(this) && MR::isNearPlayer(this, 1000.0f)) {
            rate = 0.5f;
        } else {
            rate = 2.0f;
        }
        MR::turnDirectionToTargetUseGroundNormalDegree(this, &mFront, *MR::getPlayerPos(), rate);
        MR::addVelocityMoveToDirection(this, mFront, mAccel);
    } else {
        MR::addVelocityToGravity(this, 1.5f);
    }
    MR::attenuateVelocity(this, mAirCount >= 5 ? 0.99f : 0.9f);
    if (MR::reboundVelocityFromEachCollision(this, -1.0f, 0.0f, 0.0f, 0.0f) && MR::isBindedWall(this)) {
        TVec3f displacement(mPosition - mLastPos);
        MR::startActionSound(this, "Collision", (s32)displacement.length(), -1, -1);
        mLastPos = mPosition;
    }
    MR::sendMsgEnemyAttackTrampleDeathToGroundSensor(getSensor(nullptr));
    MR::sendMsgEnemyAttackRollToBindedSensor(this, getSensor(nullptr));
    MR::rotateQuatRollBall(&mQuat, mVelocity, -mGravity, mRadius * mScale.x);
    if (onGround) {
        MR::startActionSound(this, "Roll", (s32)(100.0f * PSVECMag(&mVelocity)), -1, 5);
    }
    startVoice();
}

void WanwanSearch::exeGoal() {
    MR::zeroVelocity(this);
    TVec3f prev(mPosition);
    f32 rate = MR::calcNerveRate(this, 120);
    MR::vecBlend(&mPosition, mPosition, mGoalPos, rate);
    MR::rotateQuatRollBall(&mQuat, mPosition - prev, -mGravity, mRadius * mScale.x);
    if (MR::isGreaterStep(this, 160)) {
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
        MR::startAction(this, "Goal");
    }
}

void WanwanSearch::exeBreak() {
    if (MR::isFirstStep(this)) {
        if (mBreakModel != nullptr) {
            mBreakModel->appear();
            TVec3f up = -mGravity;
            MR::setQuatUpNoSupport(mBreakModel, up);
        }
        MR::emitItemHit(this);
        kill();
    }
}

WanwanSearch::~WanwanSearch() {
}
