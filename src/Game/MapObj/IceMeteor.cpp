#pragma auto_inline off
#include "Game/MapObj/IceMeteor.hpp"
#include "Game/MapObj/IceMeteorParts.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"

namespace { const char* sMeteorSensor = "Meteor"; const char* sShadowName = "Body"; const char* sBodySensor = "Body"; }
namespace NrvIceMeteor {
NEW_NERVE(IceMeteorNrvWait, IceMeteor, Wait);
NEW_NERVE(IceMeteorNrvFall, IceMeteor, Fall);
NEW_NERVE(IceMeteorNrvSpread, IceMeteor, Spread);
NEW_NERVE(IceMeteorNrvSpreadWait, IceMeteor, SpreadWait);
NEW_NERVE(IceMeteorNrvShrink, IceMeteor, Shrink);
NEW_NERVE(IceMeteorNrvShrinkWait, IceMeteor, ShrinkWait);
}
// This shared zero is loaded from the original read-only zero pool.
extern const f32 lbl_807E30A0;
IceMeteor::IceMeteor(const char* name) : LiveActor(name), mIsSpread(false), mIceRadius(lbl_807E30A0), mFallVelocity(0.0f, 0.0f, 0.0f), mUp(0.0f, 0.0f, 0.0f), mWaitTime(0), mShrinkWaitTime(0), mFallDistance(2000.0f), mFallTime(180), mParts(0) {}
void IceMeteor::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    initNerve(&NrvIceMeteor::IceMeteorNrvWait::sInstance);
    MR::calcGravity(this);
    MR::calcUpVec(&mUp, this);
    MR::getJMapInfoArg0NoInit(iter, &mWaitTime);
    MR::getJMapInfoArg1NoInit(iter, &mShrinkWaitTime);
    MR::getJMapInfoArg2NoInit(iter, &mFallDistance);
    MR::getJMapInfoArg3NoInit(iter, &mFallTime);
    MR::setGroupClipping(this, iter, 16);
    MR::createIceMeteorHolder();
    mParts = new IceMeteorParts(this, "アイスメテオ隕石部分");
    mParts->initWithoutIter();
    MR::setSensorFollowPos(this, sMeteorSensor, &mParts->mPosition);
    MR::setShadowDropPositionPtr(this, sShadowName, &mParts->mPosition);
    MR::startAction(mParts, "IceMeteorBall");
    makeActorAppeared();
}
void IceMeteor::initPlayerResource(const JMapInfoIter&) { MR::enablePlayerResourceFreezeIce(); }
void IceMeteor::exeWait() {
    if (MR::isFirstStep(this)) resetMeteor();
    if (MR::isGreaterStep(this, mWaitTime)) setNerve(&NrvIceMeteor::IceMeteorNrvFall::sInstance);
}
void IceMeteor::exeFall() {
    updateMeteor();
    if (MR::isStep(this, 10)) MR::startActionSound(mParts, "Fall");
    if (MR::isStep(this, mFallTime - 15)) setNerve(&NrvIceMeteor::IceMeteorNrvSpread::sInstance);
}
void IceMeteor::exeSpread() {
    if (MR::isFirstStep(this)) { MR::startAction(this, "Spread"); mIsSpread = true; MR::invalidateHitSensor(this, sMeteorSensor); }
    updateMeteor();
    if (MR::isGreaterStep(this, 15)) spreadIce();
    if (MR::isActionEnd(this)) setNerve(&NrvIceMeteor::IceMeteorNrvSpreadWait::sInstance);
}
void IceMeteor::exeSpreadWait() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Wait");
    spreadIce();
    if (MR::isGreaterStep(this, 240)) setNerve(&NrvIceMeteor::IceMeteorNrvShrink::sInstance);
}
void IceMeteor::exeShrink() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Shrink");
    shrinkIce();
    if (MR::isActionEnd(this)) setNerve(&NrvIceMeteor::IceMeteorNrvShrinkWait::sInstance);
}
void IceMeteor::exeShrinkWait() {
    if (MR::isGreaterStep(this, mShrinkWaitTime)) { resetMeteor(); setNerve(&NrvIceMeteor::IceMeteorNrvFall::sInstance); }
}
void IceMeteor::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (!MR::isSensorPlayer(receiver)) return;
    if (MR::isEqualSensor(sender, this, sMeteorSensor)) { MR::sendMsgEnemyAttackFreeze(receiver, sender); return; }
    if (MR::isEqualSensor(sender, this, sBodySensor)) {
        if (isPlayerOutsideIce(sender)) { MR::requestIceMeteorFireDance(receiver, sender); return; }
        MR::cancelIceMeteorFireDance(receiver, sender);
    }
}
// NONMATCHING: integer-conversion constant base uses r5 instead of r4.
void IceMeteor::resetMeteor() {
    mParts->setOffset(mUp * mFallDistance);
    f32 speed = mFallDistance / mFallTime;
    mFallVelocity = -mUp * speed;
    MR::validateHitSensor(this, sMeteorSensor);
    mIsSpread = false;
    mIceRadius = 0.0f;
}
// NONMATCHING: paired-single subtraction uses different floating-point registers.
void IceMeteor::updateMeteor() {
    TVec3f offset(mParts->getOffset());
    TVec3f distance(offset);
    distance -= mPosition;
    if (distance.dot(mUp) > -500.0f) { offset += mFallVelocity; mParts->setOffset(offset); }
}
bool IceMeteor::isPlayerOutsideIce(HitSensor* sensor) {
    TVec3f up;
    MR::calcUpVec(&up, this);
    const TVec3f* player = MR::getPlayerPos();
    const TVec3f* pos = MR::getSensorPos(sensor);
    f32 radial, height;
    MR::func_8003DEF0(&radial, &height, pos, player, &up);
    if (height > 50.0f) return false;
    return radial >= mIceRadius;
}
void IceMeteor::spreadIce() { s32 duration = 82; mIceRadius += (420.0f - lbl_807E30A0) / duration; mIceRadius = MR::clamp(mIceRadius, lbl_807E30A0, 420.0f); }
void IceMeteor::shrinkIce() { s32 duration = 90; mIceRadius -= 420.0f / duration; mIceRadius = MR::clamp(mIceRadius, 0.0f, 420.0f); }
IceMeteor::~IceMeteor() {}
