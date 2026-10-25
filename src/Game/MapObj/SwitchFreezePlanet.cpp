#include "Game/Util/Functor.hpp"
#include "Game/MapObj/SwitchFreezePlanet.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/Map/PlanetMapFunction.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include <cstdio>

namespace NrvSwitchFreezePlanet {
    NEW_NERVE(SwitchFreezePlanetNrvWait, SwitchFreezePlanet, Wait);
    NEW_NERVE(SwitchFreezePlanetNrvFreeze, SwitchFreezePlanet, Freeze);
    NEW_NERVE(SwitchFreezePlanetNrvFreezeWait, SwitchFreezePlanet, FreezeWait);
    NEW_NERVE(SwitchFreezePlanetNrvFreezeImmediately, SwitchFreezePlanet, FreezeImmediately);
    NEW_NERVE(SwitchFreezePlanetNrvThaw, SwitchFreezePlanet, Thaw);
}

SwitchFreezePlanet::SwitchFreezePlanet(const char* name) : MapObjActor(name),
    mWaterModel(nullptr), mIceModel(nullptr), mIndirectModel(nullptr),
    mIceCollision(nullptr), mReplaceBody(false), mEffectCount(0) {}

void SwitchFreezePlanet::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoPlanet(&info);
    MapObjActorUtil::setupInitInfoTypical(&info, mObjectName);
    bool frozen = false;
    MR::getJMapInfoArg0NoInit(iter, &frozen);
    info.setupSound(1, "SwitchFreezePlanet");
    if (frozen)
        info.setupNerve(&NrvSwitchFreezePlanet::SwitchFreezePlanetNrvFreezeImmediately::sInstance);
    else
        info.setupNerve(&NrvSwitchFreezePlanet::SwitchFreezePlanetNrvWait::sInstance);
    initialize(iter, info);
    bool skateHole = false;
    if (MR::isEqualString(mObjectName, "SkateHolePlanet"))
        skateHole = true;
    mWaterModel = MR::createWaterModel(this, nullptr);
    mIceModel = MR::createIceModel(this, getBaseMtx(), skateHole);
    mIndirectModel = MR::createIndirectPlanetModel(this, nullptr);
    MR::tryCreateCollisionWaterSurface(this, getSensor("Body"));
    char iceName[256];
    snprintf(iceName, sizeof(iceName), "%sIce", mObjectName);
    mIceCollision = MR::createCollisionPartsFromLiveActor(mIceModel, iceName, getSensor("Body"), (MR::CollisionScaleType)2);
    MR::invalidateCollisionParts(mIceCollision);
    PlanetMapFunction::verifyEffects(this, &mEffectCount, mObjectName, 16);
    if (MR::isEqualString(mObjectName, "StarCreekPlanet")) {
        if (MR::isDemoCast(this, nullptr)) {
            MR::registerDemoActionFunctor(this, MR::FunctorV0M<SwitchFreezePlanet*, void (SwitchFreezePlanet::*)()>(this, &SwitchFreezePlanet::startFreeze), nullptr);
            MR::tryRegisterDemoCast(mWaterModel, iter);
            MR::tryRegisterDemoCast(mIceModel, iter);
        }
        MR::registerDemoSimpleCastAll(mWaterModel);
        MR::registerDemoSimpleCastAll(mIceModel);
    }
    if (MR::isEqualString(mObjectName, "SkateHolePlanet")) {
        mReplaceBody = true;
        mIceModel->mCheckHiddenHost = true;
    }
    mIceModel->makeActorDead();
}

void SwitchFreezePlanet::startClipped() {
    MapObjActor::startClipped();
    PlanetMapFunction::tryForceDeleteEffects(this, mObjectName, mEffectCount);
}

void SwitchFreezePlanet::endClipped() {
    MapObjActor::endClipped();
    if (isNerve(&NrvSwitchFreezePlanet::SwitchFreezePlanetNrvWait::sInstance) && !MR::isHiddenModel(this))
        PlanetMapFunction::tryEmitEffects(this, mObjectName, mEffectCount);
}

void SwitchFreezePlanet::control() {
    if (MR::isValidSwitchB(this)) {
        if (MR::isOnSwitchB(this) && !MR::isHiddenModel(this)) {
            MR::hideModel(this);
            PlanetMapFunction::tryForceDeleteEffects(this, mObjectName, mEffectCount);
        }
        else if (!MR::isOnSwitchB(this) && MR::isHiddenModel(this)) {
            MR::showModel(this);
            PlanetMapFunction::tryEmitEffects(this, mObjectName, mEffectCount);
        }
    }
}

void SwitchFreezePlanet::startFreeze() {
    swapCollisionFreeze();
    setNerve(&NrvSwitchFreezePlanet::SwitchFreezePlanetNrvFreeze::sInstance);
}

void SwitchFreezePlanet::connectToScene(const MapObjActorInitInfo&) { MR::connectToScenePlanet(this); }
void SwitchFreezePlanet::initCaseUseSwitchA(const MapObjActorInitInfo&) {}
void SwitchFreezePlanet::initCaseUseSwitchB(const MapObjActorInitInfo&) {}

void SwitchFreezePlanet::swapCollisionFreeze() {
    MR::stopScene(5);
    mIceModel->makeActorAppeared();
    MR::validateCollisionParts(mIceCollision);
    PlanetMapFunction::tryDeleteEffects(this, mObjectName, mEffectCount);
    if (mReplaceBody)
        MR::invalidateCollisionParts(this);
}

void SwitchFreezePlanet::swapCollisionNormal() {
    if (mWaterModel)
        mWaterModel->makeActorAppeared();
    mIceModel->makeActorDead();
    MR::invalidateCollisionParts(mIceCollision);
    if (mReplaceBody)
        MR::validateCollisionParts(this);
}

void SwitchFreezePlanet::exeWait() {
    if (MR::isFirstStep(this))
        PlanetMapFunction::tryEmitEffects(this, mObjectName, mEffectCount);
    if (MR::isValidSwitchA(this) && MR::isOnSwitchA(this)) {
        swapCollisionFreeze();
        setNerve(&NrvSwitchFreezePlanet::SwitchFreezePlanetNrvFreeze::sInstance);
    }
}

void SwitchFreezePlanet::exeFreeze() {
    if (MR::isFirstStep(this)) {
        if (mReplaceBody)
            MR::hideModel(this);
        bool animated = MR::tryStartAllAnim(mIceModel, "Freeze");
        MR::startActionSound(this, "Freeze", -1, -1, -1);
        if (!animated) {
            setNerve(&NrvSwitchFreezePlanet::SwitchFreezePlanetNrvFreezeWait::sInstance);
            if (mWaterModel)
                mWaterModel->makeActorDead();
            return;
        }
    }
    if (MR::isActionEnd(mIceModel)) {
        setNerve(&NrvSwitchFreezePlanet::SwitchFreezePlanetNrvFreezeWait::sInstance);
        if (mWaterModel)
            mWaterModel->makeActorDead();
    }
}

void SwitchFreezePlanet::exeFreezeWait() {
    if (MR::isFirstStep(this))
        MR::tryStartAllAnim(mIceModel, "FreezeWait");
    if (MR::isValidSwitchA(this) && !MR::isOnSwitchA(this)) {
        if (mReplaceBody && MR::isHiddenModel(this)) {
            MR::showModel(this);
        }
        else {
            swapCollisionNormal();
            setNerve(&NrvSwitchFreezePlanet::SwitchFreezePlanetNrvThaw::sInstance);
        }
    }
}

void SwitchFreezePlanet::exeFreezeImmediately() {
    if (MR::isFirstStep(this)) {
        if (mWaterModel)
            mWaterModel->makeActorDead();
        mIceModel->makeActorAppeared();
        MR::validateCollisionParts(mIceCollision);
        if (mReplaceBody)
            MR::invalidateCollisionParts(this);
        if (MR::tryStartAllAnim(mIceModel, "FreezeWait"))
            MR::setAllAnimFrameAtEnd(mIceModel, "FreezeWait");
    }
}

void SwitchFreezePlanet::exeThaw() {
    if (MR::isFirstStep(this) && !MR::tryStartAllAnim(mIceModel, "Thow")) {
        setNerve(&NrvSwitchFreezePlanet::SwitchFreezePlanetNrvWait::sInstance);
        return;
    }
    if (MR::isActionEnd(mIceModel))
        setNerve(&NrvSwitchFreezePlanet::SwitchFreezePlanetNrvWait::sInstance);
}

SwitchFreezePlanet::~SwitchFreezePlanet() {}
