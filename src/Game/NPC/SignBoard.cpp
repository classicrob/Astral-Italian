#define JGEOMETRY_VEC3_SET_NOINLINE
#include "Game/NPC/SignBoard.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/NPCUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

namespace NrvSignBoard {
    NEW_NERVE(SignBoardNrvWait, SignBoard, Wait);
};  // namespace NrvSignBoard

SignBoard::SignBoard(const char* pName) : NPCActor(pName) {
    mSupportPlayMode = -1;
    mSupportPlayActive = 0;
}

// NONMATCHING: initialization scheduling and scalar-pool relocations differ.
void SignBoard::init(const JMapInfoIter& rIter) {
    const char* name;
    MR::getObjectName(&name, rIter);
    NPCActorCaps caps(name);
    caps.setDefault();
    caps.mWaitNerve = GET_NERVE(SignBoard, SignBoardNrvWait);
    caps.mYoshiInteraction = false;
    caps.mMessageId = "SignBoard";

    MR::getJMapInfoArg0NoInit(rIter, &mSupportPlayMode);
    if (mSupportPlayMode == 0) {
        MR::joinSupportPlayGroup(this);
        if (MR::isExistStageSwitchAppear(rIter)) {
            caps.mSwitchAppear = true;
        }
    }

    initialize(rIter, caps, name, nullptr, nullptr);

    TVec3f up;
    mOrientationQuat.getYDir(up);

    MR::setShadowProjection(this, nullptr, mPosition, up, true);
    MR::offCalcShadow(this, nullptr);

    if (mSupportPlayMode == 0 || MR::isValidSwitchAppear(this)) {
        makeActorDead();
    } else {
        makeActorAppeared();
    }
}

void SignBoard::exeWait() {
    MR::tryTalkNearPlayer(getMsgCtrl());
}

void SignBoard::appear() {
    if (mSupportPlayMode == 0) {
        if (mSupportPlayActive == 0) {
            return;
        }
        if (MR::isDead(this)) {
            MR::tryHitReaction(this, "煙");
        }
        LiveActor::appear();
    } else {
        LiveActor::appear();
    }
}

void SignBoard::kill() {
    MR::tryHitReaction(this, "煙");
    NPCActor::kill();
}

void SignBoard::makeActorDead() {
    if (mSupportPlayMode == 0 && MR::isValidSwitchAppear(this)) {
        if (!MR::isDead(this) && mSupportPlayActive != 0) {
            MR::tryHitReaction(this, "煙");
        }
        NPCActor::makeActorDead();
    } else {
        NPCActor::makeActorDead();
    }
}

bool SignBoard::receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) {
    return false;
}

bool SignBoard::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mSupportPlayMode != 0) {
        return false;
    }

    if (MR::isMsgSupportPlayGroupStart(msg)) {
        mSupportPlayActive = 1;
        if (!MR::isValidSwitchAppear(this) || MR::isOnSwitchAppear(this)) {
            appear();
        }
        return true;
    }

    if (MR::isMsgSupportPlayGroupStop(msg)) {
        mSupportPlayActive = 0;
        if (!MR::isDead(this)) {
            kill();
        }
        return true;
    }
    return false;
}
