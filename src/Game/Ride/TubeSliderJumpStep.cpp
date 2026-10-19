#include "Game/Ride/TubeSliderJumpStep.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
namespace NrvTubeSliderJumpStep {
NEW_NERVE(TubeSliderJumpStepNrvWait, TubeSliderJumpStep, Wait);
NEW_NERVE(TubeSliderJumpStepNrvJump, TubeSliderJumpStep, Jump);
}
TubeSliderJumpStep::TubeSliderJumpStep(const TVec3f& front) : LiveActor("ジャンプ台（チューブスライダー用）"), mFront(front) {}
void TubeSliderJumpStep::init(const JMapInfoIter& iter) {
    MR::setScale(this, 1.5f);
    MR::initActor(this, iter, "TubeSliderJumpStep", false);
    initNerve(&NrvTubeSliderJumpStep::TubeSliderJumpStepNrvWait::sInstance);
    makeActorAppeared();
}
void TubeSliderJumpStep::calcAndSetBaseMtx() { MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront); }
void TubeSliderJumpStep::exeWait() { if (MR::isFirstStep(this)) MR::startAction(this, "Wait"); }
void TubeSliderJumpStep::exeJump() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Jump");
    if (MR::isActionEnd(this)) setNerve(&NrvTubeSliderJumpStep::TubeSliderJumpStepNrvWait::sInstance);
}
void TubeSliderJumpStep::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (isNerve(&NrvTubeSliderJumpStep::TubeSliderJumpStepNrvWait::sInstance) && MR::isSensorRide(receiver) && MR::sendMsgJump(receiver, sender))
        setNerve(&NrvTubeSliderJumpStep::TubeSliderJumpStepNrvJump::sInstance);
}
TubeSliderJumpStep::~TubeSliderJumpStep() {}
