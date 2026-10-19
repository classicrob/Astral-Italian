#include "Game/Map/Sky.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/LiveActor/MirrorReflectionModel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/SpaceInner.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/RhythmLib/AudRhythmTiming.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/WorldMapResultCommand.hpp"

namespace {
    const char* cChangeAnimName = "Change";
};  // namespace

namespace NrvSky {
    NEW_NERVE(HostTypeWait, Sky, Wait);
    NEW_NERVE(HostTypeChange, Sky, Change);
};  // namespace NrvSky

Sky::Sky(const char* pName) : LiveActor(pName), mModelName(), mSpaceInner(), mReflectionModel() {
}

Sky::Sky(const char* pName, const char* pModelName) : LiveActor(pName), mModelName(pModelName), mSpaceInner(), mReflectionModel() {
}

void Sky::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);

    const char* objName = nullptr;
    getModelName(&objName, rIter);

    initModel(objName);
    MR::connectToSceneSky(this);
    MR::initUseStageSwitchReadA(this, rIter);
    MR::initUseStageSwitchReadB(this, rIter);

    if (MR::isEqualString(objName, "SummerSky")) {
        mSpaceInner = new SpaceInner("内側宇宙");
        mSpaceInner->initWithoutIter();

        if (MR::isValidSwitchB(this)) {
            MR::listenStageSwitchOnOffB(this, MR::Functor(this, &Sky::appearSpaceInner), MR::Functor(this, &Sky::disappearSpaceInner));
        }
    }

    s32 arg0 = -1;
    if (MR::isValidInfo(rIter)) MR::getJMapInfoArg0NoInit(rIter, &arg0);

    if (arg0 == 0) {
        mReflectionModel = new MirrorReflectionModel(this, "鏡内モデル", objName, getBaseMtx());
        mReflectionModel->initWithoutIter();
    }

    MR::tryStartAllAnim(this, objName);

    if (mReflectionModel) {
        MR::tryStartAllAnim(mReflectionModel, objName);
    }

    MR::invalidateClipping(this);
    MR::registerDemoSimpleCastAll(this);
    initNerve(GET_NERVE(Sky, HostTypeWait));

    MR::useStageSwitchAwake(this, rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void Sky::calcAnim() {
    MR::setPosition(this, MR::getCamPos());
    LiveActor::calcAnim();
}

void Sky::getModelName(const char** pOut, const JMapInfoIter& rIter) {
    if (mModelName != nullptr) {
        *pOut = mModelName;
        return;
    }

    MR::getObjectName(pOut, rIter);
}

void Sky::initModel(const char* pModelName) {
    MR::initActor(this, pModelName, false);
}

void Sky::control() {
    if (mSpaceInner != nullptr && MR::isValidSwitchB(this)) {
        if (MR::isDead(mSpaceInner)) {
            MR::showModelIfHidden(this);
        } else if (mSpaceInner->isAppeared()) {
            MR::hideModelIfShown(this);
        } else {
            MR::showModelIfHidden(this);
        }
    }
}

void Sky::appearSpaceInner() {
    if (mSpaceInner != nullptr && MR::isValidSwitchB(this)) {
        mSpaceInner->appear();
    }
}

void Sky::disappearSpaceInner() {
    if (mSpaceInner != nullptr && MR::isValidSwitchB(this)) {
        mSpaceInner->disappear();
    }
}

void Sky::exeWait() {
    updateWait();
    if (MR::isValidSwitchA(this) && MR::isOnSwitchA(this)) {
        setNerve(GET_NERVE(Sky, HostTypeChange));
    }
}

void Sky::exeChange() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, ::cChangeAnimName);
    }
}

ProjectionMapSky::ProjectionMapSky(const char* pSkyName) : Sky(pSkyName), mProjmapEffectMtxSetter() {
}

void ProjectionMapSky::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();

    if (mProjmapEffectMtxSetter != nullptr) {
        mProjmapEffectMtxSetter->updateMtxUseBaseMtx();
    }
}

void ProjectionMapSky::initModel(const char* pName) {
    MR::initActor(this, pName, true);

    mProjmapEffectMtxSetter = MR::initDLMakerProjmapEffectMtxSetter(this);
    MR::newDifferedDLBuffer(this);
    mProjmapEffectMtxSetter->updateMtxUseBaseMtx();
}

SoundSyncSky::SoundSyncSky(const char* name) : Sky(name) {}
void SoundSyncSky::updateWait() { tryStartBeatAction(); }
bool SoundSyncSky::tryStartBeatAction() {
    if (!AudRhythmWrap::isBeatTiming(8, 7.94f)) return false;
    MR::startAction(this, "Appear");
    return true;
}

namespace {
    const char* cWorldMapSkyModels[] = {nullptr, "WorldMap01Sky", "WorldMap02Sky", "WorldMap03Sky", "WorldMap04Sky", "WorldMap05Sky", "WorldMap06Sky", "WorldMap07Sky"};
    const char* cWorldMapSkySoundModels[] = {"WorldMap06Sky"};
}

WorldMapSky::WorldMapSky(const char* name) : Sky(name), mProjmapEffectMtxSetter(nullptr) {}
void WorldMapSky::appear() {
    LiveActor::appear();
    if (MR::isRegisteredEffect(this, MR::getModelResName(this))) MR::emitEffect(this, MR::getModelResName(this));
}
void WorldMapSky::getModelName(const char** out, const JMapInfoIter&) {
    *out = cWorldMapSkyModels[GameDataFunction::getCurrentWorldId()];
}
void WorldMapSky::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    if (mProjmapEffectMtxSetter) mProjmapEffectMtxSetter->updateMtxUseBaseMtx();
}
void WorldMapSky::initModel(const char* name) {
    MR::initActor(this, name, true);
    if (MR::isExistProjMapTexMtx(this)) {
        mProjmapEffectMtxSetter = MR::initDLMakerProjmapEffectMtxSetter(this);
        MR::newDifferedDLBuffer(this);
        mProjmapEffectMtxSetter->updateMtxUseBaseMtx();
    } else if (MR::isExistBrk(this, name) || MR::isExistBtk(this, name)) {
        MR::newDifferedDLBuffer(this);
    }
    if (MR::isExistString(name, cWorldMapSkySoundModels, 1)) initSound(4, nullptr, nullptr, TVec3f(0.0f));
    initEffectKeeper(0, nullptr, false);
    setWorldMapResultActor(this);
    appear();
}

SoundSyncSky::~SoundSyncSky() {}
WorldMapSky::~WorldMapSky() {}
