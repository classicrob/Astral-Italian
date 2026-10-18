#include "Game/Map/PlanetMap.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Map/OceanHomeMapCtrl.hpp"
#include "Game/Map/PlanetMapFunction.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <cstdio>

namespace {
    const char* cFollowJointName = "Move";

    static PlanetMapClippingInfo sClippingInfo[] = {{"PhantomShipA", 3000.0f, {800.0f, 1300.0f, 0.0f}, 0}};
    PlanetMapClippingInfo* findClippingInfo(const char* pName) {
        for (u32 i = 0; i < ARRAY_SIZE(::sClippingInfo); i++) {
            PlanetMapClippingInfo* pInfo = &::sClippingInfo[i];
            if (MR::isEqualStringCase(pName, pInfo->mName)) {
                return pInfo;
            }
        }

        return nullptr;
    }
};  // namespace

PlanetMap::PlanetMap(const char* pName, const char* pModelName)
    : LiveActor(pName), mModelName(pModelName), _90(0.0f, 0.0f, 0.0f), mLODCtrl(), mBloomModel(), mWaterModel(), mIndirectModel(), mEffectNum() {
}

void PlanetMap::connectToScene() {
    MR::connectToScenePlanet(this);
}

void PlanetMap::initDemoActionAppear(const JMapInfoIter& rIter) {
    if (MR::tryRegisterDemoCast(this, rIter) && MR::isRegisteredDemoActionAppear(this)) {
        makeActorDead();
    }
}

void PlanetMap::initSwitchBListener(const JMapInfoIter& rIter) {
    if (MR::initUseStageSwitchReadB(this, rIter)) {
        MR::listenStageSwitchOnOffB(this, MR::Functor(this, &PlanetMap::hide), MR::Functor(this, &PlanetMap::show));
    }
}

void PlanetMap::hide() {
    MR::hideModel(this);
}

void PlanetMap::show() {
    MR::showModel(this);
}

void PlanetMap::initMyEffect() {
    PlanetMapFunction::verifyEffects(this, &mEffectNum, mModelName, 16);
}

void PlanetMap::init(const JMapInfoIter& rIter) {
    MR::getObjectName(&mModelName, rIter);
    if (MR::isValidInfo(rIter)) {
        f32 v11 = -1.0f;
        MR::getJMapInfoArg0NoInit(rIter, &v11);
        f32 v12 = -1.0f;
        MR::getJMapInfoArg2NoInit(rIter, &v12);
        f32 unknownUse = 0.0f > v11 ? 1.0f : -1.0f;
    }

    MR::initDefaultPos(this, rIter);
    initModel(mModelName, rIter);
    connectToScene();
    initEffectKeeper(0, nullptr, false);
    initMyEffect();
    initHitSensor(1);
    MR::addBodyMessageSensorMapObj(this);

    if (MR::isExistCollisionResource(this, mModelName)) {
        MtxPtr jointMTX = nullptr;
        if (MR::isExistJoint(this, ::cFollowJointName)) {
            jointMTX = MR::getJointMtx(this, ::cFollowJointName);
        }

        MR::initCollisionParts(this, mModelName, getSensor("body"), jointMTX);
    }

    MR::tryCreateCollisionMoveLimit(this, getSensor("body"));
    MR::tryCreateCollisionWaterSurface(this, getSensor("body"));
    initSound(4, mModelName, nullptr, TVec3f(0.0f));

    if (MR::isExistAnim(this, mModelName)) {
        MR::startAction(this, mModelName);
    } else if (!MR::isExistEffectTexMtx(this)) {
        calcAnim();
        MR::offCalcAnim(this);
    }

    initClipping(rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
    initDemoActionAppear(rIter);
    MR::initFurPlanet(this);
    MR::useStageSwitchAwake(this, rIter);
    initSwitchBListener(rIter);
}

void PlanetMap::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    tryEmitMyEffect();
    mLODCtrl->validate();
}

void PlanetMap::makeActorDead() {
    mLODCtrl->invalidate();
    tryDeleteMyEffect();
    LiveActor::makeActorDead();
}

void PlanetMap::initClipping(const JMapInfoIter& rIter) {
    f32 radius = 0.0f;
    if (MR::isValidInfo(rIter)) {
        MR::getJMapInfoArg1NoInit(rIter, &radius);
    }

    PlanetMapClippingInfo* pInfo = ::findClippingInfo(mModelName);

    if (pInfo != nullptr) {
        radius = pInfo->mRadius;
        _90.set(mPosition + TVec3f(pInfo->mOffset));
        MR::setClippingTypeSphere(this, pInfo->mRadius, &_90);
    } else {
        if (radius <= 0.0f) {
            MR::calcModelBoundingRadius(&radius, this);
            radius += 100.0f;
        }

        MR::setClippingTypeSphere(this, radius);
    }

    if (MR::isValidInfo(rIter)) {
        MR::setGroupClipping(this, rIter, 128);
    }
}

s32 PlanetMap::getLowMovementType() const {
    return -1;
}

void PlanetMap::initModel(const char* pModelName, const JMapInfoIter& rIter) {
    initModelManagerWithAnm(pModelName, nullptr, false);
    s32 lowMovementType = getLowMovementType();
    f32 farClippingDistance = getFarClipDistance();
    mLODCtrl = MR::createLodCtrlPlanet(this, rIter, farClippingDistance, lowMovementType);
    initBloomModel(pModelName);
    mWaterModel = MR::createWaterModel(this, nullptr);
    mIndirectModel = MR::createIndirectPlanetModel(this, nullptr);
}

void PlanetMap::initBloomModel(const char* pModelName) {
    if (!MR::isExistSubModel(pModelName, "Bloom")) {
        mBloomModel = nullptr;
        return;
    }

    char str[256];
    snprintf(str, 256, "%sBloom", mModelName);
    const char* name = mName;
    mBloomModel = MR::createModelObjBloomModel(name, str, getBaseMtx(), true);
    mBloomModel->mScale.set(mScale);

    f32 radiusBound = 500.0f;
    MR::calcModelBoundingRadius(&radiusBound, this);
    MR::setClippingFarMax(mBloomModel);
    MR::setClippingTypeSphere(mBloomModel, radiusBound);
}

void PlanetMap::control() {
    mLODCtrl->update();
}

void PlanetMap::startClipped() {
    LiveActor::startClipped();
    tryDeleteMyEffect();
}

void PlanetMap::endClipped() {
    LiveActor::endClipped();
    tryEmitMyEffect();
}

bool PlanetMap::tryEmitMyEffect() {
    return PlanetMapFunction::tryEmitEffects(this, mModelName, mEffectNum);
}

bool PlanetMap::tryDeleteMyEffect() {
    return PlanetMapFunction::tryForceDeleteEffects(this, mModelName, mEffectNum);
}

FurPlanetMap::FurPlanetMap(const char* pName) : PlanetMap(pName, nullptr) {
}

PlanetMap::~PlanetMap() {
}

void FurPlanetMap::init(const JMapInfoIter& rIter) {
    PlanetMap::init(rIter);
    MR::initFurPlanet(this);
}

RailPlanetMap::RailPlanetMap(const char* pName) : PlanetMap(pName, nullptr) {
}


s32 PlanetMapAnimLow::getLowMovementType() const {
    return 0x1C;
}

FurPlanetMap::~FurPlanetMap() {
}
RailPlanetMap::~RailPlanetMap() {
}
PlanetMapAnimLow::~PlanetMapAnimLow() {
}

f32 PlanetMap::getFarClipDistance() const {
    return -1.0f;
}

PlanetMapFarClippable::PlanetMapFarClippable(const char* pName) : PlanetMap(pName, nullptr), mFarClipDistance(50.0f) {
}

void PlanetMapFarClippable::init(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg3NoInit(rIter, &mFarClipDistance);
    PlanetMap::init(rIter);
}
