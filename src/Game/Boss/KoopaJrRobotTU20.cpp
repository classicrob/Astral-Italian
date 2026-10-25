#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvKoopaJrRobotChain {
    NEW_NERVE(KoopaJrRobotChainNrvWait, KoopaJrRobotChain, Wait);
    NEW_NERVE(KoopaJrRobotChainNrvDisappear, KoopaJrRobotChain, Disappear);
    NEW_NERVE(KoopaJrRobotChainNrvDisappearEnd, KoopaJrRobotChain, DisappearEnd);
};  // namespace NrvKoopaJrRobotChain

KoopaJrRobotChain::KoopaJrRobotChain(const char* pName) : LiveActor(pName), mFixedPosition(nullptr), mMtxSetter(nullptr) {}

void KoopaJrRobotChain::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "KoopaSealChain", true);
    mMtxSetter = MR::initDLMakerProjmapEffectMtxSetter(this);
    MR::newDifferedDLBuffer(this);
    initNerve(&NrvKoopaJrRobotChain::KoopaJrRobotChainNrvWait::sInstance, 0);
    makeActorDead();
}

void KoopaJrRobotChain::calcAndSetBaseMtx() {
    if (mFixedPosition != nullptr) {
        mFixedPosition->calc();
        MR::extractMtxTrans(mFixedPosition->mMtx.toMtxPtr(), &mPosition);
        MR::setBaseTRMtx(this, mFixedPosition->mMtx);
    } else {
        LiveActor::calcAndSetBaseMtx();
    }
    if (mMtxSetter != nullptr) {
        mMtxSetter->updateMtxUseBaseMtx();
    }
}

bool KoopaJrRobotChain::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    return MR::isMsgAttackRockReflect(msg);
}

void KoopaJrRobotChain::setFixedPosition(LiveActor* pHost, const char* pJoint) {
    mFixedPosition = new FixedPosition(pHost, pJoint, pHost);
}

void KoopaJrRobotChain::startDisappear() {
    setNerve(&NrvKoopaJrRobotChain::KoopaJrRobotChainNrvDisappear::sInstance);
}

void KoopaJrRobotChain::startDisappearEnd() {
    setNerve(&NrvKoopaJrRobotChain::KoopaJrRobotChainNrvDisappearEnd::sInstance);
}

bool KoopaJrRobotChain::isDisappearEnd() const {
    return isNerve(&NrvKoopaJrRobotChain::KoopaJrRobotChainNrvDisappearEnd::sInstance);
}

void KoopaJrRobotChain::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
}

void KoopaJrRobotChain::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Disappear");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvKoopaJrRobotChain::KoopaJrRobotChainNrvDisappearEnd::sInstance);
    }
}

void KoopaJrRobotChain::exeDisappearEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "DisappearEnd");
    }
}

KoopaJrRobotChain::~KoopaJrRobotChain() {}
