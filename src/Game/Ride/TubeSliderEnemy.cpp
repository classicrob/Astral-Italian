#include "Game/Ride/TubeSliderEnemy.hpp"
#include "Game/Ride/TubeSliderFunction.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
namespace {
    extern const f32 sSoundDistance;
}
namespace NrvTubeSliderEnemy {
NEW_NERVE(TubeSliderEnemyNrvWait, TubeSliderEnemy, Wait);
NEW_NERVE(TubeSliderEnemyNrvWalk, TubeSliderEnemy, Walk);
NEW_NERVE(TubeSliderEnemyNrvSupportFreeze, TubeSliderEnemy, SupportFreeze);
}
TubeSliderEnemy::TubeSliderEnemy(TubeSlider* slider) : LiveActor("敵（チューブスライダー用）"), mSlider(slider), mFront(0.0f, 0.0f, 1.0f), mRailCoord(0.0f), mAngle(-1.0f), mSpeed(3.0f), mScaleController(0), mBindState(0), mPlayerPassed(false), mPrevMicDistance(0.0f) {}
// NONMATCHING: call-argument load scheduling differs.
void TubeSliderEnemy::init(const JMapInfoIter& iter) {
    MR::getJMapInfoArg0NoInit(iter, &mSpeed);
    TubeSliderFunction::initDefaultPosOnTube(this, iter, &mRailCoord, &mAngle, &mFront, 0.0f, mSlider);
    mFront.x *= -1.0f;
    mFront.y *= -1.0f;
    mFront.z *= -1.0f;
    MR::initActor(this, iter, "Togezo", "TubeSlider", false);
    initNerve(&NrvTubeSliderEnemy::TubeSliderEnemyNrvWait::sInstance, 1);
    mScaleController = new AnimScaleController(0);
    mBindState = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mBindState, &NrvTubeSliderEnemy::TubeSliderEnemyNrvSupportFreeze::sInstance, "SupportFreeze");
    makeActorAppeared();
}
void TubeSliderEnemy::initAfterPlacement() {
    TubeSliderFunction::unknown(this, &mFront, mSlider, mRailCoord, mAngle, 0.0f);
    mFront.x *= -1.0f;
    mFront.y *= -1.0f;
    mFront.z *= -1.0f;
}
void TubeSliderEnemy::control() {
    mScaleController->updateNerve();
    if (!isNerve(&NrvTubeSliderEnemy::TubeSliderEnemyNrvSupportFreeze::sInstance) && mBindState->tryStartPointBind()) {
        setNerve(&NrvTubeSliderEnemy::TubeSliderEnemyNrvSupportFreeze::sInstance);
        return;
    }
    f32 micDistance = MR::getMicDistance(this);
    f32 distance = MR::calcDistanceToPlayer(this);
    if (distance <= 450.0f) mPlayerPassed = true;
    TVec3f velocity = *MR::getPlayerVelocity() - mVelocity;
    f32 speed = PSVECMag(&velocity);
    s32 baseVolume = 50;
    s32 basePitch = -100;
    f32 volume = MR::getLinerValueFromMinMax(speed, 27.0f, 34.0f, 0.0f, MR::clamp(baseVolume + 1000.0f * (100.0f * (2.3f / micDistance)), 0.0f, 1000.0f));
    f32 pitch = basePitch + 1000.0f * (50.0f / (50.0f + (micDistance - mPrevMicDistance)));
    if (distance <= sSoundDistance && mPlayerPassed) MR::startActionSound(this, "MarioPassLvSlider", volume, MR::clamp((s32)pitch, 50, 1000), -1);
    mPrevMicDistance = micDistance;
}
void TubeSliderEnemy::exeWait() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Wait");
    if (MR::isValidSwitchA(this) && MR::isOnSwitchA(this)) setNerve(&NrvTubeSliderEnemy::TubeSliderEnemyNrvWalk::sInstance);
}
void TubeSliderEnemy::exeWalk() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Walk");
    mRailCoord -= mSpeed;
    TubeSliderFunction::unknown(this, &mFront, mSlider, mRailCoord, mAngle, 0.0f);
    mFront.x *= -1.0f;
    mFront.y *= -1.0f;
    mFront.z *= -1.0f;
    if (MR::isGreaterStep(this, 600)) kill();
}
void TubeSliderEnemy::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront);
    MR::updateBaseScale(this, mScaleController);
}
void TubeSliderEnemy::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (MR::isSensorRide(receiver)) {
        if (MR::sendMsgEnemyAttack(receiver, sender)) return;
    }
}
bool TubeSliderEnemy::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) { return MR::isMsgStarPieceReflect(msg); }
TubeSliderEnemy::~TubeSliderEnemy() {}
void TubeSliderEnemy::exeSupportFreeze() {
    if (MR::updateActorState(this)) setNerve(&NrvTubeSliderEnemy::TubeSliderEnemyNrvWait::sInstance);
}

namespace {
    const f32 sSoundDistance = 1000.0f;
}
