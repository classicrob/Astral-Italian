#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"

#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvKoopaJrRobotRock {
    NEW_NERVE(KoopaJrRobotRockNrvAppear, KoopaJrRobotRock, Appear);
    NEW_NERVE(KoopaJrRobotRockNrvWait, KoopaJrRobotRock, Wait);
    NEW_NERVE(KoopaJrRobotRockNrvDisappear, KoopaJrRobotRock, Disappear);
};  // namespace NrvKoopaJrRobotRock

KoopaJrRobotRock::KoopaJrRobotRock(const char* pName) : LiveActor(pName) {
    mQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
}

void KoopaJrRobotRock::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "KoopaPunchRock", false);
    initNerve(&NrvKoopaJrRobotRock::KoopaJrRobotRockNrvAppear::sInstance, 0);
    makeActorDead();
}

void KoopaJrRobotRock::control() {}

void KoopaJrRobotRock::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
}

void KoopaJrRobotRock::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrRobotRock::KoopaJrRobotRockNrvAppear::sInstance) || isNerve(&NrvKoopaJrRobotRock::KoopaJrRobotRockNrvWait::sInstance)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

void KoopaJrRobotRock::appearAtArenaEdge() {
    setNerve(&NrvKoopaJrRobotRock::KoopaJrRobotRockNrvAppear::sInstance);
    MR::calcKoopaBattleActorEdgePos(&mPosition);
    MR::resetPosition(this);
    TVec3f up;
    MR::calcKoopaBattleDirToActor(&up);
    TVec3f front;
    MR::getKoopaBattleUp(&front);
    MR::makeQuatUpFront(&mQuat, up, front);
    appear();
}

void KoopaJrRobotRock::startDisappear() {
    setNerve(&NrvKoopaJrRobotRock::KoopaJrRobotRockNrvDisappear::sInstance);
}

void KoopaJrRobotRock::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvKoopaJrRobotRock::KoopaJrRobotRockNrvWait::sInstance);
    }
}

void KoopaJrRobotRock::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
}

void KoopaJrRobotRock::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Disappear");
    }
    if (MR::isActionEnd(this)) {
        kill();
    }
}

KoopaJrRobotRock::~KoopaJrRobotRock() {}
