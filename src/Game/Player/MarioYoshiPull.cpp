#include "Game/Player/MarioYoshiPull.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Player/YoshiTongueAction.hpp"
#include "Game/Util.hpp"
MarioYoshiPull::MarioYoshiPull(MarioActor* actor) : MarioState(actor, MarioStatus_YoshiPull),mPhase(0),mTimer(0) {}
bool MarioYoshiPull::start() {
    changeAnimation("ヨッシー引っ張り",static_cast<const char*>(nullptr));
    mActor->setBlendMtxTimer(10);
    clearVelocity();
    mPhase=0;mTimer=0;
    return true;
}
bool MarioYoshiPull::close() {
    stopAnimation("ヨッシー引っ張り",static_cast<const char*>(nullptr));
    stopAnimation("ヨッシー引っ張り歩き",static_cast<const char*>(nullptr));
    getPlayer()->startBas(nullptr,false,0.0f,0.0f);
    return true;
}
bool MarioYoshiPull::update() {
    if(!((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) return false;
    mTimer++;
    switch(mPhase) {
    case 0:return exePull();
    case 1:return true;
    default:return false;
    }
}
void MarioYoshiPull::fn_804090C0() { mPhase=1;mTimer=0; }
// NONMATCHING: host-load scheduling and GPR/FP register allocation only.
bool MarioYoshiPull::exePull() {
    TVec3f target;
    mActor->fn_803B9810()->fn_8041FB40(&target);
    TVec3f direction(target);
    direction-=mActor->mPosition;
    MR::vecKillElement(direction,getPlayer()->getAirGravityVec(),&direction);
    if(!MR::normalizeOrZero(&direction)) {
        getPlayer()->setFrontVecKeepUp(direction);
        TVec3f input(getWorldPadDir());
        MR::vecKillElement(input,getPlayer()->getAirGravityVec(),&input);
        MR::normalizeOrZero(&input);
        f32 dot=direction.dot(input);
        if(dot<0.0f) {
            if(!isAnimationRun("ヨッシー引っ張り歩き")) changeAnimation("ヨッシー引っ張り歩き",static_cast<const char*>(nullptr));
            else MR::startActionSound(mActor->getYoshi(),"Pull",-1,-1,-1);
            f32 power=YoshiTongueAction::fn_8041FB50();
            addVelocity((direction*dot)*power);
        } else if(!isAnimationRun("ヨッシー引っ張り")) changeAnimation("ヨッシー引っ張り",static_cast<const char*>(nullptr));
    }
    return true;
}
