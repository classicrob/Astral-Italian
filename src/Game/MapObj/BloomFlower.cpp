#pragma auto_inline off
#include "Game/MapObj/BloomFlower.hpp"
#include "Game/MapObj/StarPiece.hpp"
#include "Game/MapObj/StarLightAreaHolder.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvBloomFlower {
NEW_NERVE(BloomFlowerNrvGrow, BloomFlower, Grow);
NEW_NERVE(BloomFlowerNrvWaitGrow, BloomFlower, WaitGrow);
NEW_NERVE(BloomFlowerNrvWither, BloomFlower, Wither);
NEW_NERVE(BloomFlowerNrvWaitWither, BloomFlower, WaitWither);
}
BloomFlower::BloomFlower(const char* name) : LiveActor(name), mStarPiece(0) {}
void BloomFlower::init(const JMapInfoIter& iter) {
    MR::initDefaultPos(this, iter);
    MR::initActor(this, iter, "BloomFlower", false);
    MR::connectToSceneMapObj(this);
    initHitSensor(1);
    MR::addHitSensorMapObj(this, "Body");
    MR::tryRegisterStarLightReceiver(getSensor("Body"));
    mStarPiece = new StarPiece("咲き花用スターピース");
    mStarPiece->initWithoutIter();
    MR::copyTransRotate(this, mStarPiece);
    TVec3f up(0.0f);
    MR::calcUpVec(&up, this);
    MR::vecScaleAdd(&mStarPiece->mPosition, &up, 100.0f);
    mStarPiece->makeActorDead();
    initNerve(&NrvBloomFlower::BloomFlowerNrvWaitGrow::sInstance);
    makeActorAppeared();
}
void BloomFlower::control() {}
bool BloomFlower::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) { if (MR::isMsgStarLightGrow(msg)) return tryGrow(); if (MR::isMsgStarLightShrink(msg)) return tryWither(); return false; }
void BloomFlower::exeGrow() { if (MR::isFirstStep(this)) { MR::startAction(this, "Grow"); if (!mStarPiece->isNerveToPlayerEnd()) mStarPiece->makeActorAppeared(); } }
void BloomFlower::exeWaitGrow() { if (MR::isFirstStep(this)) MR::validateClipping(this); }
void BloomFlower::exeWither() { if (MR::isFirstStep(this)) { MR::startAction(this, "Wither"); if (!mStarPiece->isNerveToPlayerEnd()) mStarPiece->makeActorDead(); } }
void BloomFlower::exeWaitWither() { if (MR::isFirstStep(this)) MR::validateClipping(this); }
bool BloomFlower::tryGrow() {
    if (isNerve(&NrvBloomFlower::BloomFlowerNrvWaitWither::sInstance) || isNerve(&NrvBloomFlower::BloomFlowerNrvWither::sInstance)) { MR::invalidateClipping(this); setNerve(&NrvBloomFlower::BloomFlowerNrvGrow::sInstance); return true; }
    return false;
}
bool BloomFlower::tryWither() {
    if (isNerve(&NrvBloomFlower::BloomFlowerNrvWaitWither::sInstance) || isNerve(&NrvBloomFlower::BloomFlowerNrvWither::sInstance)) return false;
    MR::invalidateClipping(this);
    setNerve(&NrvBloomFlower::BloomFlowerNrvWither::sInstance);
    return true;
}
BloomFlower::~BloomFlower() {}
