#pragma auto_inline off
#include "Game/MapObj/BeeJumpBall.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
namespace NrvBeeJumpBall {
    NEW_NERVE(BeeJumpBallNrvWait, BeeJumpBall, Wait);
    NEW_NERVE(BeeJumpBallNrvBound, BeeJumpBall, Bound);
    NEW_NERVE(BeeJumpBallNrvBounce, BeeJumpBall, Bound);
    NEW_NERVE(BeeJumpBallNrvBreak, BeeJumpBall, Break);
    NEW_NERVE(BeeJumpBallNrvRecover, BeeJumpBall, Recover);
}
BeeJumpBall::BeeJumpBall(const char* name) : LiveActor(name) {}
BeeJumpBall::~BeeJumpBall() {}
void BeeJumpBall::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    if (mScale.x == 2.0f) {
        MR::setSensorRadius(this, "Body", 160.0f);
        MR::setSensorRadius(this, "StarPieceHit", 210.0f);
    } else if (mScale.x == 1.5f) {
        MR::setSensorRadius(this, "Body", 100.0f);
        MR::setSensorRadius(this, "StarPieceHit", 160.0f);
    }
    initNerve(&NrvBeeJumpBall::BeeJumpBallNrvWait::sInstance);
    makeActorAppeared();
}
void BeeJumpBall::exeWait() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Wait");
}
void BeeJumpBall::exeBound() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Bound");
    if (MR::isGreaterStep(this, 30)) setNerve(&NrvBeeJumpBall::BeeJumpBallNrvWait::sInstance);
}
void BeeJumpBall::exeBreak() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Break");
    if (MR::isActionEnd(this)) setNerve(&NrvBeeJumpBall::BeeJumpBallNrvRecover::sInstance);
}
void BeeJumpBall::exeRecover() {
    if (MR::isStep(this, 180)) {
        MR::startAction(this, "Recover");
        MR::startActionSound(this, "Restore");
        return;
    }
    if (!MR::isLessStep(this, 180) && MR::isActionEnd(this)) {
        setNerve(&NrvBeeJumpBall::BeeJumpBallNrvWait::sInstance);
        MR::validateHitSensors(this);
        MR::validateClipping(this);
    }
}
// NONMATCHING: Break-nerve branch uses a direct exit instead of target's two-branch exit.
void BeeJumpBall::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (!MR::isEqualSensor(sender, this, "StarPieceHit")
        && MR::isSensorPlayer(receiver)
        && (!isNerve(&NrvBeeJumpBall::BeeJumpBallNrvBound::sInstance) || !MR::isLessStep(this, 15))
        && !isNerve(&NrvBeeJumpBall::BeeJumpBallNrvRecover::sInstance)) {
        if (isNerve(&NrvBeeJumpBall::BeeJumpBallNrvBreak::sInstance)) return;
        TVec3f direction;
        MR::calcDirToPlayer(&direction, this);
        f32 downSpeed = -MR::getPlayerVelocity()->dot(mGravity);
        if (downSpeed < 0.0f) downSpeed = 0.0f;
        if (MR::isPlayerElementModeBee()) {
            MR::tryHitReactionBetweenSensors(this, "弾く", sender, receiver);
            f32 scale = mScale.x;
            MR::startActionSound(this, "Bound", (s32)(300.0f + 1000.0f / scale));
            TVec3f jump(direction * 25.0f - mGravity * (25.0f + downSpeed));
            MR::clampLength(&jump, jump, 30.0f);
            MR::restrictVelocityToPlaneModeArea(&jump, mPosition);
            MR::jumpPlayer(jump);
            MR::clearPlayerMovementFlag20000000();
            MR::startBckPlayerJ("ハチボールジャンプ");
            MR::extendPlayerBeeAirWalk(20);
            setNerve(&NrvBeeJumpBall::BeeJumpBallNrvBound::sInstance);
            return;
        }
        MR::tryHitReactionBetweenSensors(this, "壊れる", sender, receiver);
        MR::invalidateHitSensors(this);
        MR::invalidateClipping(this);
        setNerve(&NrvBeeJumpBall::BeeJumpBallNrvBreak::sInstance);
    }
}
bool BeeJumpBall::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor* receiver) {
    if (!MR::isEqualSensor(receiver, this, "StarPieceHit")) return false;
    if (MR::isDead(this) || isNerve(&NrvBeeJumpBall::BeeJumpBallNrvRecover::sInstance) || isNerve(&NrvBeeJumpBall::BeeJumpBallNrvBreak::sInstance)) return false;
    if (MR::isMsgStarPieceAttack(msg)) {
        setNerve(&NrvBeeJumpBall::BeeJumpBallNrvBounce::sInstance);
        return true;
    }
    return false;
}
