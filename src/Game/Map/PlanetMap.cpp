#include "Game/Map/PlanetMap.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Map/OceanHomeMapCtrl.hpp"
#include "Game/Effect/PeachCastleFireParticleCallBack.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/Map/PlanetMapFunction.hpp"
#include "Game/Util/ActorInitUtil.hpp"
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
    : LiveActor(pName), mModelName(pModelName), mClippingCenter(0.0f, 0.0f, 0.0f), mLODCtrl(), mBloomModel(), mWaterModel(), mIndirectModel(), mEffectNum() {
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
        f32 clippingDistance = 0.0f > v11 ? v12 : v11;
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

        MR::initCollisionParts(this, mModelName, getSensor("Body"), jointMTX);
    }

    MR::tryCreateCollisionMoveLimit(this, getSensor("Body"));
    MR::tryCreateCollisionWaterSurface(this, getSensor("Body"));
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
        mClippingCenter.set(mPosition + TVec3f(pInfo->mOffset));
        MR::setClippingTypeSphere(this, pInfo->mRadius, &mClippingCenter);
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
    MR::initActor(this, rIter, pModelName, useProjmapMtx());
    initProjmapMtx();
    s32 lowMovementType = getLowMovementType();
    f32 farClippingDistance = getFarClipDistance();
    mLODCtrl = MR::createLodCtrlPlanet(this, rIter, farClippingDistance, lowMovementType);
    initBloomModel(pModelName);
    mWaterModel = MR::createWaterModel(this, nullptr);
    mIndirectModel = MR::createIndirectPlanetModel(this, nullptr);
}

void PlanetMap::initBloomModel(const char* pModelName) {
    mBloomModel = MR::createBloomModel(this, getBaseMtx());

    if (mBloomModel != nullptr) {
        MR::setScale(mBloomModel, mScale);
        f32 radiusBound = 500.0f;
        MR::calcModelBoundingRadius(&radiusBound, this);
        MR::setClippingFarMax(mBloomModel);
        MR::setClippingTypeSphere(mBloomModel, radiusBound);
    }
}

void PlanetMap::control() {
    if (!MR::isValidSwitchB(this) || !MR::isOnSwitchB(this)) {
        mLODCtrl->update();
    }
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


FurPlanetMap::~FurPlanetMap() {
}
RailPlanetMap::~RailPlanetMap() {
}
PlanetMapAnimLow::~PlanetMapAnimLow() {
}

RailPlanetMap::RailPlanetMap(const char* pName) : PlanetMap(pName, nullptr) {
}


PlanetMapFarClippable::PlanetMapFarClippable(const char* pName) : PlanetMap(pName, nullptr), mFarClipDistance(50.0f) {
}

void PlanetMapFarClippable::init(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg3NoInit(rIter, &mFarClipDistance);
    PlanetMap::init(rIter);
}

UFOShapePassThroughPlanet::UFOShapePassThroughPlanet(const char* pName) : PlanetMap(pName) {
}

void UFOShapePassThroughPlanet::initDemoActionAppear(const JMapInfoIter&) {
    MR::registerDemoSimpleCastAll(this);
}

FurPlanetMap::FurPlanetMap(const char* pName) : PlanetMap(pName, nullptr) {
}

void FurPlanetMap::init(const JMapInfoIter& rIter) {
    PlanetMap::init(rIter);
    MR::initFurPlanet(this);
}

s32 PlanetMapAnimLow::getLowMovementType() const {
    return 0x1C;
}

PlanetMapStrongLight::PlanetMapStrongLight(const char* pName) : PlanetMap(pName) {
}

void PlanetMapStrongLight::connectToScene() {
    MR::connectToScene(this, 0x1C, 1, 0xB, -1);
}

PlanetMapProjmap::PlanetMapProjmap(const char* pName) : PlanetMap(pName), mProjmapEffectMtxSetter(nullptr) {
}

void PlanetMapProjmap::initProjmapMtx() {
    mProjmapEffectMtxSetter = MR::initDLMakerProjmapEffectMtxSetter(this);
    MR::newDifferedDLBuffer(this);
    mProjmapEffectMtxSetter->updateMtxUseBaseMtx();
}

void PlanetMapPress::init(const JMapInfoIter& rIter) {
    PlanetMap::init(rIter);
    MR::setBodySensorType(this, 0x71);
}

PeachCastleNightPlanet::PeachCastleNightPlanet(const char* pName) : PlanetMap(pName), mFireCallBack(nullptr) {
}

void PeachCastleNightPlanet::initMyEffect() {
    PlanetMapFunction::verifyEffects(this, &mEffectNum, mModelName, 16);
    mFireCallBack = new PeachCastleFireParticleCallBack(this);
}

bool PeachCastleNightPlanet::tryEmitMyEffect() {
    s32 num = mEffectNum;
    const char* pModelName = mModelName;

    if (num <= 0) {
        return false;
    }

    for (s32 i = 0; i < num; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s%02d", pModelName, i);
        MR::emitEffectWithParticleCallBack(this, buf, mFireCallBack);
    }

    return true;
}

void PlanetMapFunction::verifyEffects(LiveActor* pActor, s32* pNum, const char* pModelName, s32 maxNum) {
    *pNum = 0;

    for (s32 i = 0; i < maxNum; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s%02d", pModelName, i);

        if (!MR::isRegisteredEffect(pActor, buf)) {
            break;
        }

        (*pNum)++;
    }

    if (*pNum > 0) {
        MR::registerDemoSimpleCastAll(pActor);
    }
}

bool PlanetMapFunction::tryEmitEffects(LiveActor* pActor, const char* pModelName, s32 num) {
    if (num <= 0) {
        return false;
    }

    for (s32 i = 0; i < num; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s%02d", pModelName, i);
        MR::emitEffect(pActor, buf);
    }

    return true;
}

bool PlanetMapFunction::tryDeleteEffects(LiveActor* pActor, const char* pModelName, s32 num) {
    if (num <= 0) {
        return false;
    }

    for (s32 i = 0; i < num; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s%02d", pModelName, i);
        MR::tryDeleteEffect(pActor, buf);
    }

    return true;
}

bool PlanetMapFunction::tryForceDeleteEffects(LiveActor* pActor, const char* pModelName, s32 num) {
    if (num <= 0) {
        return false;
    }

    for (s32 i = 0; i < num; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s%02d", pModelName, i);
        MR::tryForceDeleteEffect(pActor, buf);
    }

    return true;
}
