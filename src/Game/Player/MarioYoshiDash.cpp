#include "Game/Player/MarioYoshiDash.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Util.hpp"
MarioYoshiDash::MarioYoshiDash(MarioActor* actor) : MarioState(actor, MarioStatus_YoshiDash) {}
bool MarioYoshiDash::start() { return true; }
bool MarioYoshiDash::close() { return true; }
bool MarioYoshiDash::update() {
    if(getStickP()!=0.0f || checkLvlA() || checkLvlZ() || mActor->fn_803BE0C0()) {
        stopAnimation(nullptr,static_cast<const char*>(nullptr));
        getPlayer()->fn_803A0BE0();
        return false;
    }
    if(mActor->isRequestRush() || getPlayer()->isStatusActive(MarioStatus_FpView) || getPlayer()->_1C._5==1 || !MR::isNearZero(getPlayer()->_184,0.001f) ||
       (getPlayer()->isPlayerModeYoshi() && mActor->fn_803B97F0()->mDashTimer>0)) {
        stopAnimation(nullptr,static_cast<const char*>(nullptr));
        return false;
    }
    return true;
}
