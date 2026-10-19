#include "Game/Player/MarioFlow.hpp"
#include "Game/Util.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Util/MathUtil.hpp"

bool MarioFlow::start() {
    mFrameCounter = 0;
    mPhase = Airborne;

    changeAnimationNonStop("中ダメージ");
    startPadVib(2);

    getPlayer()->mMovementStates.mLowWord &= 0xBFFFFFFFU;
    getPlayer()->mMovementStates.mLowWord |= Mario::MovementStates::Jumping;
    getPlayer()->mMovementStates.mLowWord &= 0xFFEFFFFFU;
    getPlayer()->mMovementStates.mHighWord |= 0x00100000U;

    const MarioConstTable* table = mActor->mConst->getTable();
    const TVec3f& front = getFrontVec();
    mVelocity = front * table->mJumpDistFlow;
    mVelocity += -mActor->mAppliedGravityVec * mActor->mConst->getTable()->mJumpHeightFlow;
    getPlayer()->mJumpVec = mVelocity;

    addVelocity(mVelocity);

    getPlayer()->stopWalk();

    return true;
}

bool MarioFlow::update() {
    mFrameCounter++;

    switch (mPhase) {
    case Airborne:
        addVelocity(mVelocity);

        mVelocity += mActor->mAppliedGravityVec * mActor->getConst().getTable()->mGravityFlow;

        if (mFrameCounter == 20) {
            changeAnimation("中ダメージ空中");
        }

        if (mFrameCounter > 30 && mActor->isRequestRush()) {
            getPlayer()->trySpinJump(0);

            return false;
        }

        if (((getPlayer()->getMovementStates().mLowWord >> 30) & 1U)) {
            getPlayer()->mMovementStates.mLowWord &= ~Mario::MovementStates::Jumping;

            changeAnimation("中ダメージ着地");

            playEffect("共通ダメージ着地");
            playSound("吹っ飛び倒れ");

            MR::vecKillElement(mVelocity, mActor->mAppliedGravityVec, &mVelocity);

            mFrameCounter = 0;
            mPhase++;
        }
        break;
    case Landed:
        if (((getPlayer()->getMovementStates().mLowWord >> 30) & 1U) == false) {
            getPlayer()->mMovementStates.mLowWord |= Mario::MovementStates::Jumping;
            mPhase = Airborne;
        } else {
            addVelocity(mVelocity);

            mVelocity.x *= 0.95f;
            mVelocity.y *= 0.95f;
            mVelocity.z *= 0.95f;

            if (!isAnimationRun("中ダメージ着地")) {
                return false;
            }

            if (mFrameCounter > 15 && checkTrgA()) {
                getPlayer()->tryJump();

                return false;
            }
        }
        break;
    }

    getPlayer()->mJumpVec = mVelocity;

    return true;
}

bool MarioFlow::close() {
    stopAnimation("ダメージ");
    stopAnimation("ダメージ着地", "基本");

    return true;
}

MarioFlow::MarioFlow(MarioActor* pActor) : MarioState(pActor, MarioStatus_Flow), mFrameCounter(), mPhase() {
    mVelocity.zero();
}
