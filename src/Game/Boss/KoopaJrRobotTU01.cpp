#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvKoopaJrRobotBinderActor {
    NEW_NERVE(KoopaJrRobotBinderActorNrvFollow, KoopaJrRobotBinderActor, Follow);
};  // namespace NrvKoopaJrRobotBinderActor

KoopaJrRobotBinderActor::KoopaJrRobotBinderActor(HitSensor* pSensor)
    : LiveActor("\x83o\x83""C\x83\x93\x83_\x81[\x83""A\x83N\x83^\x81["), mSensor(pSensor) {}

void KoopaJrRobotBinderActor::init(const JMapInfoIter& rIter) {
    MR::getSensorHost(mSensor);
    MR::connectToSceneEnemyDecorationMovement(this);
    initBinder(MR::getSensorRadius(mSensor), 0.0f, 0);
    initNerve(&NrvKoopaJrRobotBinderActor::KoopaJrRobotBinderActorNrvFollow::sInstance, 0);
    makeActorAppeared();
}

void KoopaJrRobotBinderActor::initAfterPlacement() {
    MR::setPosition(this, *MR::getSensorPos(mSensor));
}

void KoopaJrRobotBinderActor::exeFollow() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
    }
    MR::setPosition(this, *MR::getSensorPos(mSensor));
}

KoopaJrRobotBinderActor::~KoopaJrRobotBinderActor() {}
