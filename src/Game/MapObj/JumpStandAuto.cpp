#include "Game/MapObj/JumpStandAuto.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvJumpStandAuto {
    NEW_NERVE(JumpStandAutoNrvWait, JumpStandAuto, Wait);
    NEW_NERVE(JumpStandAutoNrvExtend, JumpStandAuto, Extend);
}
JumpStandAuto::JumpStandAuto(const char* name) : LiveActor(name) {}
JumpStandAuto::~JumpStandAuto() {}
void JumpStandAuto::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    initNerve(&NrvJumpStandAuto::JumpStandAutoNrvWait::sInstance);
    MR::calcGravity(this);
    makeActorAppeared();
    MR::useStageSwitchAwake(this, iter);
}
void JumpStandAuto::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (MR::isEqualSensor(sender, this, "YoshiPush") && MR::isSensorYoshi(receiver) && isNerve(&NrvJumpStandAuto::JumpStandAutoNrvExtend::sInstance)) MR::sendMsgPush(receiver, sender);
}
void JumpStandAuto::exeWait() {
    if (!MR::isPlayerDamaging() && MR::isActorOnPlayer(this)) {
        jumpPlayer(30.0f);
        setNerve(&NrvJumpStandAuto::JumpStandAutoNrvExtend::sInstance);
    }
}
void JumpStandAuto::exeExtend() {
    if (!MR::isPlayerDamaging()) {
        if (MR::isFirstStep(this)) {
            MR::startPlayerSoundForYoshiMode("JumpStandAuto", "JumpStandAuto", -1);
            MR::startAction(this, "Extend");
        }
        if (MR::isGreaterStep(this, 10) && MR::isActorOnPlayer(this)) {
            jumpPlayer(30.0f);
            setNerve(&NrvJumpStandAuto::JumpStandAutoNrvExtend::sInstance);
            return;
        }
        MR::setNerveAtBckStopped(this, &NrvJumpStandAuto::JumpStandAutoNrvWait::sInstance);
    }
}
void JumpStandAuto::jumpPlayer(f32 speed) {
    TVec3f velocity(*MR::getPlayerLastMove() * 0.4f);
    MR::vecKillElement(velocity, mGravity, &velocity);
    if (PSVECMag(&velocity) > 100.0f) velocity.setLength(100.0f);
    TVec3f toPlayer(0.0f);
    MR::calcVecToPlayer(&toPlayer, this);
    toPlayer.dot(-mGravity);
    velocity -= mGravity * speed;
    MR::startBckPlayerJ("ジャンプB");
    MR::forceJumpPlayer(velocity);
    MR::clearPlayerStateFlags00300000();
}
