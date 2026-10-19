#include "Game/MapObj/RotateMoveObj.hpp"
#include "Game/Map/CollisionParts.hpp"
#include <revolution/mtx.h>
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/MapObj/MapPartsRotator.hpp"
#include "Game/MapObj/StageEffectDataTable.hpp"
#include "Game/Util.hpp"


namespace NrvRotateMoveObj {
    NEW_NERVE(HostTypeWaitForPlayerOn, RotateMoveObj, WaitForPlayerOn);
    NEW_NERVE(HostTypeWait, RotateMoveObj, Wait);
    NEW_NERVE(HostTypeMove, RotateMoveObj, Move);
    NEW_NERVE(HostTypeStop, RotateMoveObj, Stop);
};  // namespace NrvRotateMoveObj

void RotateMoveObj::init(const JMapInfoIter& rIter) {
    MapObjActor::init(rIter);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoSimpleMapObj(&info);
    info.setupRotator();
    info.setupBaseMtxFollowTarget();
    info.setupNerve(GET_NERVE(RotateMoveObj, HostTypeWait));
    MapObjActorUtil::setupInitInfoColorChangeArg0(&info, rIter);
    MapObjActorUtil::setupInitInfoTextureChangeArg1(&info, rIter);
    MapObjActorUtil::setupInitInfoTypical(&info, mObjectName);

    if (MR::isEqualSubString(mObjectName, "MagmaSliceRotateStep")) {
        MapObjActorUtil::setupProjmapMtx(&info, false, false);
    }

    initialize(rIter, info);

    bool v5 = true;
    s32 condType = 0;
    MR::getMapPartsArgMoveConditionType(&condType, rIter);

    if (!MR::isMoveStartTypeUnconditional(condType)) {
        setNerve(GET_NERVE(RotateMoveObj, HostTypeWaitForPlayerOn));
        v5 = false;
    }

    if (MR::isDemoCast(this, nullptr)) {
        if (MR::tryRegisterDemoActionFunctor(this, MR::Functor(this, &RotateMoveObj::setStateMove), nullptr)) {
            v5 = false;
        }
    }

    if (MR::isValidSwitchB(this)) {
        v5 = false;
    }

    if (v5) {
        startMapPartsFunctions();
        setNerve(GET_NERVE(RotateMoveObj, HostTypeMove));
    }
}

void RotateMoveObj::initCaseUseSwitchB(const MapObjActorInitInfo& rInfo) {
    MR::listenStageSwitchOnOffB(this, MR::Functor(this, &RotateMoveObj::setStateMove),
                                MR::FunctorV0M< MapObjActor*, void (MapObjActor::*)() >(this, &MapObjActor::pauseMapPartsFunctions));
}

void RotateMoveObj::initCaseNoUseSwitchB(const MapObjActorInitInfo& rInfo) {
}

void RotateMoveObj::setStateMove() {
    if (MR::isRegisteredEffect(this, "Appear")) {
        MR::emitEffect(this, "Appear");
    }

    startMapPartsFunctions();
    setNerve(GET_NERVE(RotateMoveObj, HostTypeMove));
}

void RotateMoveObj::exeWaitForPlayerOn() {
    if (MR::isOnPlayer(this)) {
        setStateMove();
    }
}

void RotateMoveObj::exeWait() {
}


void RotateMoveObj::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::startActionSoundAuto(this, "RotateMoveObjStart", -1, -1, -1);
        MR::StageEffect::tryStageEffectStart(this, mObjectName);
    }

    MR::startActionSoundAuto(this, "RotateMoveObjMove", -1, -1, -1);

    if (!MapObjActorUtil::isRotatorMoving(this)) {
        setNerve(GET_NERVE(RotateMoveObj, HostTypeStop));
    } else if (mRotator->mIsActive) {
        if (MR::isValidSwitchParam(this)) {
            const char* pObjectName = mObjectName;
            bool isOn = MR::isOnSwitchParam(this);
            MR::StageEffect::tryStageEffectMoving(this, pObjectName, isOn);
        } else {
            MR::StageEffect::tryStageEffectMoving(this, mObjectName);
        }
    }
}

void RotateMoveObj::exeStop() {
    if (MapObjActorUtil::isRotatorMoving(this)) {
        setNerve(GET_NERVE(RotateMoveObj, HostTypeMove));
    } else if (MR::isFirstStep(this)) {
        MR::startActionSoundAuto(this, "RotateMoveObjStop", -1, -1, -1);
        MR::StageEffect::tryStageEffectStop(this, mObjectName);
    }
}
