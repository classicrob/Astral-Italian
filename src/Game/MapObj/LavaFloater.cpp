#include "Game/MapObj/LavaFloater.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/FloaterFloatingForceTypeNormal.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "math_types.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/types.h>

namespace {
    const char* const cEffectName = "LavaBubble";
};  // namespace

namespace NrvLavaFloater {
    NEW_NERVE(HostTypeFloat, LavaFloater, Float);
    NEW_NERVE(HostTypeSink, LavaFloater, Sink);
};  // namespace NrvLavaFloater

LavaFloater::LavaFloater(const char* pName)
    : LiveActor(pName), mFloatingForce(), mMtxSetter(), mLodCtrlPlanet(), mObjectName(), _A0(gZeroVec), _AC(), _B0() {
}

LavaFloater::~LavaFloater() {
}

// NONMATCHING: FloaterFloatingForceTypeNormal is 0x60 in ours vs __nw(0x68) in SMG2 (shared header); register allocation of rIter/this differs.
void LavaFloater::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::getObjectName(&mObjectName, rIter);

    const char* const names[] = {"LavaHomeFloaterB", "LavaFloatingStepA", "LavaFloatingStepB", "LavaFloatingStepC",
                                 "LavaFloatingStepD", "FloaterOtakingFloaterA", "LavaYoshiActionFloaterA", "LavaFloaterTrapezoid"};

    if (MR::isExistString(mObjectName, names, 8)) {
        MR::initActor(this, rIter, mObjectName, true);
        mMtxSetter = MR::initDLMakerProjmapEffectMtxSetter(this);
        MR::newDifferedDLBuffer(this);
    } else {
        MR::initActor(this, rIter, mObjectName, false);
    }

    initEffectKeeper(0, nullptr, false);
    MR::setEffectHostSRT(this, ::cEffectName, &_A0, &mRotation, nullptr);

    const char* const names2[] = {"LavaFloaterTrapezoid", "LavaYoshiActionFloaterA"};
    _B0 = MR::isExistString(mObjectName, names2, 2);
    if (_B0) {
        initSound(4, mObjectName, nullptr, TVec3f(0.0f));
    } else {
        initSound(4, "LavaFloater", nullptr, TVec3f(0.0f));
    }

    initHitSensor(1);
    MR::initCollisionParts(this, mObjectName, MR::addBodyMessageSensorMapObj(this), nullptr);
    MR::setClippingTypeSphereContainsModelBoundingBox(this, 100.0f);
    MR::connectToSceneCollisionMapObj(this);
    initNerve(GET_NERVE(LavaFloater, HostTypeFloat), 0);
    MR::onCalcGravity(this);
    const char* as = mObjectName;
    mFloatingForce = new FloaterFloatingForceTypeNormal(this, as);
    mFloatingForce->init(rIter);
    mFloatingForce->start();
    MR::useStageSwitchAwake(this, rIter);
    makeActorAppeared();
}

void LavaFloater::initAfterPlacement() {
    TVec3f v6(mGravity);
    v6 *= 1000.0f;

    if (!MR::getFirstPolyOnLineToMap(&_A0, nullptr, mPosition, v6)) {
        MR::calcMapGroundUpper(&_A0, this);
    }

    _AC = mPosition.distance(_A0);

    if ((mPosition - _A0).dot(mGravity) < 0.0f) {
        _AC *= -1.0f;
    }

    if (mMtxSetter != nullptr) {
        TVec3f uselessVec(0.0f, _AC, 0.0f);
        mMtxSetter->updateMtxUseBaseMtxWithLocalOffset(TVec3f(0.0f, _AC, 0.0f));
    }

    MR::offCalcGravity(this);
}

void LavaFloater::makeActorAppeared() {
    LiveActor::makeActorAppeared();

    if (mLodCtrlPlanet != nullptr) {
        mLodCtrlPlanet->validate();
    }
}

void LavaFloater::makeActorDead() {
    if (mLodCtrlPlanet != nullptr) {
        mLodCtrlPlanet->invalidate();
    }

    LiveActor::makeActorDead();
}

void LavaFloater::control() {
    bool isMove = true;
    if (MR::isValidSwitchA(this) && MR::isOnSwitchA(this)) {
        isMove = false;
    }

    if (isMove) {
        mFloatingForce->movement();
        mFloatingForce->updateHostTrans(&mPosition);
        mFloatingForce->updateHostVelocity(&mVelocity);
    }

    if (mLodCtrlPlanet != nullptr) {
        mLodCtrlPlanet->update();
    }
}

void LavaFloater::startClipped() {
    LiveActor::startClipped();
    MR::deleteEffectAll(this);
}

void LavaFloater::endClipped() {
    LiveActor::endClipped();

    if (isNerve(GET_NERVE(LavaFloater, HostTypeSink))) {
        MR::emitEffect(this, ::cEffectName);
    }
}

f32 LavaFloater::getCurrentSinkDepth() const {
    TPos3f depthMtx;
    TVec3f v1, v2;
    depthMtx.set(getBaseMtx());
    depthMtx.getTrans(v1);
    depthMtx.getYDir(v2);
    f32 sign = MR::sign(v2.dot(_A0 - v1));
    return sign * _A0.distance(v1);
}

void LavaFloater::exeFloat() {
    if (MR::isFirstStep(this)) {
        MR::deleteEffect(this, ::cEffectName);
    }

    if (_AC < getCurrentSinkDepth()) {
        setNerve(GET_NERVE(LavaFloater, HostTypeSink));
    }
}

void LavaFloater::exeSink() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, ::cEffectName);
    }

    if (MR::isNearZero(getCurrentSinkDepth() - _AC, 1.0f)) {
        setNerve(GET_NERVE(LavaFloater, HostTypeFloat));
    }
}

