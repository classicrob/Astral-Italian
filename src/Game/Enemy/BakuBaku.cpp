#pragma auto_inline off
#include "Game/MapObj/ClipAreaDropHolder.hpp"
#include "Game/LiveActor/ActorPoseTQ.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Enemy/BakuBaku.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/MapObj/ClipAreaHolder.hpp"
#include "Game/LiveActor/ActorParam.hpp"

namespace NrvBakuBaku {
    NEW_NERVE(BakuBakuNrvMove, BakuBaku, Move);
    NEW_NERVE(BakuBakuNrvTurn, BakuBaku, Turn);

    class BakuBakuNrvFreeze : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static BakuBakuNrvFreeze sInstance;
    };
    BakuBakuNrvFreeze BakuBakuNrvFreeze::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(BakuBakuNrvAttackSuccess, BakuBaku, Success);
};  // namespace NrvBakuBaku

#define GET_BAKUBAKU_NERVE(name) (&NrvBakuBaku::name::sInstance)

BakuBaku::BakuBaku(const char* pName) : LiveActor(pName) {
    mPose = new ActorPoseTQ();
    mAnimScaleCtrl = nullptr;
    mStarPointBind = nullptr;
    mAnchorPos.set(0.0f, 0.0f, 0.0f);
    mMoveLimit = 5.0f;
    MR::createClipAreaDropHolder();
}

void BakuBaku::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "BakuBaku", false);
    MR::initActorPoseTQ(mPose, this);
    initNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvMove), 1);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mStarPointBind = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mStarPointBind, GET_BAKUBAKU_NERVE(BakuBakuNrvFreeze), "SupportFreeze");
    if (MR::isExistRail(this)) {
        MR::moveCoordAndTransToNearestRailPos(this);
        MR::offBind(this);
        s32 arg;
        if (MR::getRailArg0NoInit(this, &arg)) mMoveLimit = arg;
    }
    MR::useStageSwitchSyncAppear(this, rIter);
}

void BakuBaku::initAfterPlacement() {
    MR::trySetMoveLimitCollision(this);
}

void BakuBaku::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorRide(pReceiver) && isMoveOrTurnNerve() && MR::sendMsgToEnemyAttackRoll(pReceiver, pSender)) return;
    if (MR::isSensorPlayerOrYoshi(pReceiver) && MR::isOnGroundPlayer() && isMoveOrTurnNerve() &&
    MR::sendMsgEnemyAttackStrongOrPush(pReceiver, pSender)) {
        setNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvAttackSuccess));
        return;
    }
    MR::sendMsgPush(pReceiver, pSender);
}

bool BakuBaku::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg) || MR::isMsgPlayerSpinAttackOrSupportTico(msg) || MR::isMsgPlayerTrample(msg)) {
        return isAttackable();
    }

    if (MR::isMsgPlayerHipDrop(msg) && isAttackable()) {
        MR::sendMsgAwayJump(pSender, pReceiver);
        return true;
    }

    return false;
}

bool BakuBaku::isAttackable() const {
    if (isNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvMove)) ||
    (isNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvTurn)) && MR::isGreaterStep(this, 20))) {
        const_cast<BakuBaku*>(this)->setNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvTurn));
        MR::zeroVelocity(const_cast<BakuBaku*>(this));
        return true;
    }
    return false;
}

void BakuBaku::control() {
    mAnimScaleCtrl->updateNerve();
    MR::blendActorPoseFromGroundAndFront(mPose, this, 0.05f, 0.5f);
    if (this->isMoveTurnOrSuccessNerve() && mStarPointBind->tryStartPointBind()) {
        setNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvFreeze));
    }
}

void BakuBaku::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPose);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

void BakuBaku::exeTurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Turn");
        if (MR::isExistRail(this)) MR::reverseRailDirection(this);
        mPose->mFront.negate();
        ActorPoseTQ* pose = mPose;
        TVec3f up = -mGravity;
        MR::makeQuatUpFront(pose, up, mPose->mFront);
    }
    MR::zeroVelocity(this);
    if (MR::isActionEnd(this)) setNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvMove));
}

void BakuBaku::exeMove() {
    if (MR::isExistRail(this)) updateBakuRailMove();
    else updateFreeMove();
    f32 scale = mScale.x;
    if (PSVECDistance(&mPosition, &mAnchorPos) > 140.0f * scale &&
    MR::appearClipAreaDrop(mPosition, 120.0f * scale, 5, 250, 300)) mAnchorPos.set(mPosition);
}

void BakuBaku::exeSuccess() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSuccess");
    }

    MR::zeroVelocity(this);
    if (MR::isActionEnd(this)) {
        setNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvMove));
    }
}

// NONMATCHING: compiler reloads the 120.0f clip radius after calcPositionFrontOffset; target retains it in f1.
void BakuBaku::playBiteOrEatSound() {
    if (MR::checkPassBckFrame(this, 14.0f) || MR::checkPassBckFrame(this, 27.0f)) {
        TVec3f pos;
        MR::calcPositionFrontOffset(&pos, this, 120.0f);
        if (MR::isInClipArea(pos)) {
            MR::startActionSound(this, "Bite", -1, -1, -1);
            return;
        }
        MR::startActionSound(this, "Eat", -1, -1, -1);
    }
}

void BakuBaku::updateBakuRailMove() {
    if (MR::isFirstStep(this)) {
        MR::startActionAtRandomFrame(this, "Move");
    }
    playBiteOrEatSound();
    MR::moveCoordAndFollowTrans(this, mMoveLimit);
    TVec3f* front = &mPose->mFront;
    front->set(MR::getRailDirection(this));
    if (MR::isRailReachedGoal(this)) {
        MR::reverseRailDirection(this);
    }
}

void BakuBaku::updateFreeMove() {
    if (MR::isFirstStep(this)) MR::startActionAtRandomFrame(this, "Move");
    playBiteOrEatSound();
    MR::turnDirectionToPlayerDegree(this, &mPose->mFront, 1.0f);
    MR::addVelocityMoveToDirection(this, mPose->mFront, 0.6f);
    if (!MR::isBindedGround(this)) MR::addVelocityToGravity(this, 1.0f);
    MR::attenuateVelocity(this, 0.9f);
}

bool BakuBaku::isMoveOrTurnNerve() {
    if (isNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvMove)) || isNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvTurn))) return true;
    return false;
}

bool BakuBaku::isMoveTurnOrSuccessNerve() {
    if (isNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvMove)) || isNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvTurn)) ||
    isNerve(GET_BAKUBAKU_NERVE(BakuBakuNrvAttackSuccess))) return true;
    return false;
}

BakuBaku::~BakuBaku() {
}

inline void NrvBakuBaku::BakuBakuNrvFreeze::execute(Spine* pSpine) const {
    MR::updateActorStateAndNextNerve(reinterpret_cast<LiveActor*>(pSpine->mExecutor), GET_BAKUBAKU_NERVE(BakuBakuNrvMove));
}
