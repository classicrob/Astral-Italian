#include "Game/Boss/Sandy.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util/FixedPosition.hpp"
#include <cstdio>

SandyStateBase::SandyStateBase(const char* name, Sandy* host) : ActorStateBase<Sandy>(name, host) {}

// Unnamed SMG2 helpers outside this unit.

// Shared state helpers
// 80100030
bool sandyStateOpenStart(ActorStateBase< Sandy >* pState, s32 step) {
    if (MR::isFirstStep(pState)) {
        MR::startAction(pState->mHost, "OpenStart");
    }
    return MR::isGreaterEqualStep(pState, step);
}

// 80100090
bool sandyStateOpenWait(ActorStateBase< Sandy >* pState, s32 step) {
    if (MR::isFirstStep(pState)) {
        MR::startAction(pState->mHost, "OpenWait");
    }
    return MR::isGreaterEqualStep(pState, step);
}

// 801000F0
bool sandyStateClose(ActorStateBase< Sandy >* pState, bool isAll) {
    if (MR::isFirstStep(pState)) {
        if (isAll) {
            pState->mHost->startActionAll("CloseStart", nullptr);
        } else {
            MR::startAction(pState->mHost, "CloseStart");
        }
    }
    return MR::isActionEnd(pState->mHost);
}

// 80100170
// NONMATCHING: commuted floating multiply operands in the half-interval conversion.
bool sandyStateThrow(ActorStateBase< Sandy >* pState, s32 count, s32 interval, bool isBoth) {
    if (MR::isFirstStep(pState)) {
        MR::startAction(pState->mHost, "WaitNormal");
        pState->mHost->getLeftHand()->setWait();
        {
            SandyHand* hand = pState->mHost->getLeftHand();
            hand->mIsThrower = true;
        }
        pState->mHost->getLeftHand()->mWaitStep = interval;
        if (isBoth) {
            {
                SandyHand* hand = pState->mHost->getRightHand();
                hand->mIsThrower = false;
            }
            pState->mHost->getRightHand()->forceWait();
        }
        pState->mHost->resetCount();
    }
    if (isBoth && MR::isStep(pState, (s32)((f32)interval / 2.0f))) {
        {
            SandyHand* hand = pState->mHost->getRightHand();
            hand->mIsThrower = true;
        }
        pState->mHost->getRightHand()->setWait();
        pState->mHost->getRightHand()->mWaitStep = interval;
    }
    if (pState->mHost->isTotalCount(count)) {
        pState->mHost->invalidateHands();
        return true;
    }
    return false;
}
