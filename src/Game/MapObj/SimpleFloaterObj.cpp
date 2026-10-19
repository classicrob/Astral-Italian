#include "Game/MapObj/SimpleFloaterObj.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapPartsFloatingForce.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

namespace {
    const char* cEffectName = "Ripple";
};  // namespace

namespace NrvSimpleFloaterObj {
    NEW_NERVE(SimpleFloaterObjNrvWait, SimpleFloaterObj, Wait);
};  // namespace NrvSimpleFloaterObj

SimpleFloaterObj::SimpleFloaterObj(const char* pName) : LiveActor(pName) {
    mObjectName = 0;
    mFloatingForce = nullptr;
    mEffectPos.set(0.0f, 0.0f, 0.0f);
}

SimpleFloaterObj::~SimpleFloaterObj() {
}

void SimpleFloaterObj::init(const JMapInfoIter& rrIter) {
    MR::initDefaultPos(this, rrIter);
    MR::getObjectName(&mObjectName, rrIter);
    MR::initActor(this, rrIter, mObjectName, false);
    MR::connectToSceneMapObj(this);
    initHitSensor(1);
    MR::addBodyMessageSensorMapObj(this);
    HitSensor* mSensor = getSensor(nullptr);
    MR::initCollisionParts(this, mObjectName, mSensor, nullptr);
    initEffectKeeper(0, nullptr, false);
    if (MR::isRegisteredEffect(this, ::cEffectName)) {
        mEffectPos.set(mPosition);
        MR::setEffectHostSRT(this, ::cEffectName, &mEffectPos, &mRotation, nullptr);
    }
    initSound(4, mObjectName, nullptr, TVec3f(0.0f));
    MR::setClippingTypeSphereContainsModelBoundingBox(this, 100.0f);
    mFloatingForce = new MapPartsFloatingForce(this);
    mFloatingForce->init(rrIter);
    mFloatingForce->setObjectName(mObjectName);
    mFloatingForce->start();
    initNerve(GET_NERVE(SimpleFloaterObj, SimpleFloaterObjNrvWait));
    MR::useStageSwitchAwake(this, rrIter);
    makeActorAppeared();
}

void SimpleFloaterObj::exeWait() {
    if (MR::isFirstStep(this)) {
    }

    if (MR::isRegisteredEffect(this, ::cEffectName)) {
        if (!isVelocityClose() && !MR::isEffectValid(this, ::cEffectName)) {
            MR::tryEmitEffect(this, ::cEffectName);
        } else if (isVelocityClose() && MR::isEffectValid(this, ::cEffectName)) {
            MR::tryDeleteEffect(this, ::cEffectName);
        }
    }

    mFloatingForce->movement();
}
