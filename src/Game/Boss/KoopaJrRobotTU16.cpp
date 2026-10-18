#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

// PLACEHOLDER(objects): unnamed target symbol fn_800CB1A0, declared extern "C" so the reloc name matches; resolve when fn_800CB1A0 gets its real name in symbols.txt
extern "C" void fn_800CB1A0(TVec3f*);
// PLACEHOLDER(objects): unnamed target symbol fn_800CB140, declared extern "C" so the reloc name matches; resolve when fn_800CB140 gets its real name in symbols.txt
extern "C" void fn_800CB140(TVec3f*);
// PLACEHOLDER(objects): unnamed target symbol fn_800CB0E0, declared extern "C" so the reloc name matches; resolve when fn_800CB0E0 gets its real name in symbols.txt
extern "C" void fn_800CB0E0(TVec3f*);

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

void KoopaJrRobotRock::fn_800E40E0() {
    setNerve(&NrvKoopaJrRobotRock::KoopaJrRobotRockNrvAppear::sInstance);
    fn_800CB1A0(&mPosition);
    MR::resetPosition(this);
    TVec3f up;
    fn_800CB140(&up);
    TVec3f front;
    fn_800CB0E0(&front);
    MR::makeQuatUpFront(&mQuat, up, front);
    appear();
}

void KoopaJrRobotRock::fn_800E4160() {
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

void KoopaJrRobotRock::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Disappear");
    }
    if (MR::isActionEnd(this)) {
        kill();
    }
}

void KoopaJrRobotRock::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
}

KoopaJrRobotRock::~KoopaJrRobotRock() {}
