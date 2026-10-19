#include "Game/Util/BellyDragon.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

extern "C" const char lbl_8066CE68[];
extern "C" const char lbl_8066CEC4[];
extern "C" const char lbl_8066CECC[];
extern "C" const char lbl_8066CED4[];
extern "C" const char lbl_8066CEDC[];
extern "C" const char lbl_8066CEE4[];
extern "C" const f32 lbl_807D7820;
extern "C" const f32 lbl_807D7824;

namespace NrvBellyDragonFireGround {
    NEW_NERVE(BellyDragonFireGroundNrvAppear, BellyDragonFireGround, Appear);
    NEW_NERVE(BellyDragonFireGroundNrvBurn, BellyDragonFireGround, Burn);
    NEW_NERVE(BellyDragonFireGroundNrvDisappear, BellyDragonFireGround, Disappear);
};  // namespace NrvBellyDragonFireGround

BellyDragonFireGround::BellyDragonFireGround() : LiveActor(lbl_8066CE68), mBloom(NULL) {
    mBaseMtx.identity();
}

void BellyDragonFireGround::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, lbl_8066CE68 + 0x14, false);
    mBloom = (LiveActor*)MR::createDemoObj(lbl_8066CE68 + 0x2C, lbl_8066CE68 + 0x40, getBaseMtx());
    mBloom->makeActorDead();
    initNerve(&NrvBellyDragonFireGround::BellyDragonFireGroundNrvAppear::sInstance, 0);
    makeActorDead();
}

void BellyDragonFireGround::appear() {
    LiveActor::appear();
    mPosition.set(mBaseMtx.mMtx[0][3], mBaseMtx.mMtx[1][3], mBaseMtx.mMtx[2][3]);
    MR::setSensorRadius(this, lbl_8066CEC4, lbl_807D7820);
    mBloom->appear();
    setNerve(&NrvBellyDragonFireGround::BellyDragonFireGroundNrvAppear::sInstance);
}

void BellyDragonFireGround::kill() {
    LiveActor::makeActorDead();
    mBloom->makeActorDead();
}

void BellyDragonFireGround::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mBaseMtx);
}

void BellyDragonFireGround::attackSensor(HitSensor* pSensorA, HitSensor* pSensorB) {
    MR::sendMsgEnemyAttackFire(pSensorB, pSensorA);
}

void BellyDragonFireGround::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, lbl_8066CECC);
        MR::startAction(mBloom, lbl_8066CECC);
    }
    MR::setSensorRadius(this, lbl_8066CEC4, lbl_807D7824 * MR::calcNerveRate(this, 5));
    if (MR::isActionEnd(this)) {
        setNerve(&NrvBellyDragonFireGround::BellyDragonFireGroundNrvBurn::sInstance);
    }
}

void BellyDragonFireGround::exeBurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, lbl_8066CED4);
    }
    MR::startActionSound(this, lbl_8066CEDC, -1, -1, -1);
    if (MR::isGreaterEqualStep(this, 0x384)) {
        setNerve(&NrvBellyDragonFireGround::BellyDragonFireGroundNrvDisappear::sInstance);
    }
}

void BellyDragonFireGround::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, lbl_8066CEE4);
        MR::startAction(mBloom, lbl_8066CEE4);
    }
    MR::startActionSound(this, lbl_8066CEE4, -1, -1, -1);
    if (MR::isActionEnd(this)) {
        kill();
    }
}

BellyDragonFireGround::~BellyDragonFireGround() {
}
