#include "Game/Player/MarioYoshiShot.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Player/YoshiTongueAction.hpp"
#include "Game/Util.hpp"
#include <JSystem/JMath/JMath.hpp>
MarioYoshiShot::MarioYoshiShot(MarioActor* actor) : MarioState(actor, MarioStatus_YoshiShot),mShotVelocity(0.0f,0.0f,0.0f) {}
// NONMATCHING: zero float hoisting adds a saved FP register and stack space.
bool MarioYoshiShot::start() {
    changeAnimation("ヨッシーショット",static_cast<const char*>(nullptr));
    MR::startActionSound(mActor->fn_803B97F0(),"Shot",-1,-1,-1);
    MR::startCSSound1P("CS_YOSHI_VELCH_OUT",2,1.0f);
    TVec3f direction;
    direction.set<f32>(mActor->fn_803B9810()->_90);
    const TVec3f& gravity=getAirGravityVec();
    JMAVECScaleAdd(&gravity,&direction,&direction,-gravity.dot(direction));
    if(!MR::normalizeOrZero(&direction)) getPlayer()->setFrontVecKeepUp(direction);
    getPlayer()->mMovementStates.mLowWord &= 0xBFFFFFFFU;
    getPlayer()->mMovementStates.mLowWord |= Mario::MovementStates::Jumping;
    getPlayer()->mMovementStates.mLowWord &= 0xFFEFFFFFU;
    clearVelocity();
    getPlayer()->mWalkSpeed=0.0f;
    mShotVelocity.set<f32>(-getAirGravityVec()*15.0f);
    mInheritedVelocity=getPlayer()->_184;
    getPlayer()->fn_8039CFD0();
    mTimer=0;
    return true;
}
// NONMATCHING: true constant is hoisted into a saved GPR.
bool MarioYoshiShot::close() {
    mActor->fn_803B9810()->_9C=true;
    changeAnimation(nullptr,"落下");
    setJumpVec(fn_803EF300());
    return true;
}
bool MarioYoshiShot::update() {
    if(!mActor->fn_803B97F0()->fn_8040F490("ヨッシーショット")) return false;
    if(mTimer<5) {
        getPlayer()->mMovementStates.mLowWord &= 0xBFFFFFFFU;
        getPlayer()->mMovementStates.mLowWord |= Mario::MovementStates::Jumping;
    } else if(((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)==1) return false;
    TVec3f gravityStep = getAirGravityVec()*0.8f;
    mShotVelocity.add(gravityStep);
    TVec3f velocity(mShotVelocity);
    velocity.add(mInheritedVelocity);
    addVelocity(velocity);
    TVec3f jump(mShotVelocity);
    jump.add(mInheritedVelocity);
    setJumpVec(jump);
    mTimer++;
    return true;
}
