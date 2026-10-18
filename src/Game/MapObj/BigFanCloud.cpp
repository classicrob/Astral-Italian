#include "Game/MapObj/BigFanCloud.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JointController.hpp"
namespace NrvBigFanCloud {
    NEW_NERVE(BigFanCloudNrvWait, BigFanCloud, Wait);
    NEW_NERVE(BigFanCloudNrvStart, BigFanCloud, Start);
    NEW_NERVE(BigFanCloudNrvMove, BigFanCloud, Move);
    NEW_NERVE(BigFanCloudNrvStop, BigFanCloud, Stop);
}
BigFanCloud::BigFanCloud(const char* name) : LiveActor(name), mJointController(nullptr), mAngle(0.0f), mSpeed(0.0f) {}
BigFanCloud::~BigFanCloud() {}
void BigFanCloud::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, "BigFanCloud", false);
    mJointController = MR::createJointController(this, "Move", &BigFanCloud::calcJoint);
    if (MR::isValidSwitchB(this)) initNerve(&NrvBigFanCloud::BigFanCloudNrvWait::sInstance);
    else initNerve(&NrvBigFanCloud::BigFanCloudNrvMove::sInstance);
    makeActorAppeared();
}
void BigFanCloud::exeWait() {
    if (MR::isFirstStep(this)) mSpeed = 0.0f;
    tryStart();
}
void BigFanCloud::exeStart() {
    mSpeed = MR::calcNerveEaseInValue(this, 60, 0.0f, 10.0f);
    if (MR::isStep(this, 60)) {
        mSpeed = 10.0f;
        setNerve(&NrvBigFanCloud::BigFanCloudNrvMove::sInstance);
        return;
    }
    tryStop();
}
void BigFanCloud::exeMove() {
    if (MR::isFirstStep(this)) mSpeed = 10.0f;
    tryStop();
}
void BigFanCloud::exeStop() {
    mSpeed = MR::calcNerveEaseOutValue(this, 120, 10.0f, 0.0f);
    if (MR::isStep(this, 120)) {
        mSpeed = 0.0f;
        setNerve(&NrvBigFanCloud::BigFanCloudNrvWait::sInstance);
        return;
    }
    tryStart();
}
void BigFanCloud::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    mJointController->registerCallBack();
}
void BigFanCloud::control() {
    if (!isNerve(&NrvBigFanCloud::BigFanCloudNrvWait::sInstance)) mAngle += mSpeed;
}
bool BigFanCloud::calcJoint(TPos3f* mtx, const JointControllerInfo&) {
    MR::rotateMtxLocalYDegree(*mtx->toMtx(), mAngle);
    return true;
}
bool BigFanCloud::tryStart() {
    if (!MR::isValidSwitchB(this)) return false;
    if (!MR::isOnSwitchB(this)) return false;
    setNerve(&NrvBigFanCloud::BigFanCloudNrvStart::sInstance);
    return true;
}
bool BigFanCloud::tryStop() {
    if (!MR::isValidSwitchB(this)) return false;
    if (MR::isOnSwitchB(this)) return false;
    setNerve(&NrvBigFanCloud::BigFanCloudNrvStop::sInstance);
    return true;
}
