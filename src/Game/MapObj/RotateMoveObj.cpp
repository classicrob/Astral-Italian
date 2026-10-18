#include "Game/MapObj/RotateMoveObj.hpp"
#include "Game/MapObj/RotateHammerPressure.hpp"
#include "Game/Map/CollisionParts.hpp"
#include <revolution/mtx.h>
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/MapObj/MapPartsRotator.hpp"
#include "Game/MapObj/StageEffectDataTable.hpp"
#include "Game/Util.hpp"

extern const TVec3f sRotateHammerPressureOffsets[4];

namespace JGeometry {
    template <>
    void TRotation3<TMatrix34<SMatrix34C<f32> > >::setScale(f32 scale) {
                this->mMtx[0][0] = scale;
                this->mMtx[0][1] = 0.0f;
                this->mMtx[0][2] = 0.0f;
                this->mMtx[1][0] = 0.0f;
                this->mMtx[1][1] = scale;
                this->mMtx[1][2] = 0.0f;
                this->mMtx[2][0] = 0.0f;
                this->mMtx[2][1] = 0.0f;
                this->mMtx[2][2] = scale;
    }
}

// PLACEHOLDER(objects): fn_802CC2E0 is an unnamed 4-byte thunk (b setupProjmapMtx(bool, bool)) that the target calls directly; resolve when the thunk gets its name in symbols.txt
extern "C" void fn_802CC2E0(MapObjActorInitInfo*, bool, bool);

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
        fn_802CC2E0(&info, false, false);
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

// PLACEHOLDER(objects): unnamed target symbol fn_80310A40, declared extern "C" so the reloc name matches; resolve when fn_80310A40 gets its real name in symbols.txt
extern "C" void fn_80310A40(LiveActor*, const char*, bool);

void RotateMoveObj::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "RotateMoveObjStart", -1, -1, -1);
        MR::StageEffect::tryStageEffectStart(this, mObjectName);
    }

    MR::startActionSound(this, "RotateMoveObjMove", -1, -1, -1);

    if (!MapObjActorUtil::isRotatorMoving(this)) {
        setNerve(GET_NERVE(RotateMoveObj, HostTypeStop));
    } else if (mRotator->mIsActive) {
        if (MR::isValidSwitchParam(this)) {
            const char* pObjectName = mObjectName;
            bool isOn = MR::isOnSwitchParam(this);
            fn_80310A40(this, pObjectName, isOn);
        } else {
            MR::StageEffect::tryStageEffectMoving(this, mObjectName);
        }
    }
}

void RotateMoveObj::exeStop() {
    if (MapObjActorUtil::isRotatorMoving(this)) {
        setNerve(GET_NERVE(RotateMoveObj, HostTypeMove));
    } else if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "RotateMoveObjStop", -1, -1, -1);
        MR::StageEffect::tryStageEffectStop(this, mObjectName);
    }
}

void RotateHammerPressure::init(const JMapInfoIter& rIter) {
    RotateMoveObj::init(rIter);
    MR::calcUpVec(&(*reinterpret_cast<TVec3f*>(&_190)), this);
    MR::calcSideVec(&(*reinterpret_cast<TVec3f*>(&_19C)), this);
    s32 shadowLength = -1;
    MR::getJMapInfoArg2NoInit(rIter, &shadowLength);
    MR::setShadowDropDirection(this, "UpShadow", -(*reinterpret_cast<TVec3f*>(&_190)));
    MR::setShadowDropDirection(this, "SideShadow", (*reinterpret_cast<TVec3f*>(&_19C)));
    MR::setShadowDropPositionMtxPtr(this, "UpShadow", mMatrices[1], TVec3f(0.0f, 0.0f, 0.0f));
    MR::setShadowDropPositionMtxPtr(this, "SideShadow", mMatrices[2], TVec3f(0.0f, 0.0f, 0.0f));
    MR::getJMapInfoArg3NoInit(rIter, &_1A9);
    (*reinterpret_cast<CollisionParts**>(&_1AC)) = MR::createCollisionPartsFromLiveActor(this, "PushCollision", getSensor("Body"), static_cast<MR::CollisionScaleType>(2));
    MR::validateCollisionParts((*reinterpret_cast<CollisionParts**>(&_1AC)));
    if (shadowLength != -1) {
        _1A8 = true;
        MR::invalidateShadow(this, "UpShadow");
        MR::invalidateShadow(this, "SideShadow");
    }
    PSMTXCopy(getBaseMtx(), mMatrices[0]);
    MR::setEffectHostMtx(this, "Stop", mMatrices[3]);
}

// NONMATCHING: vector stack slots, floating-point scheduling, and folded scale arithmetic differ.
void RotateHammerPressure::control() {
    const TVec3f* offsets = sRotateHammerPressureOffsets;
    MapObjActor::control();
    if (!_1A8) {
        TVec3f sideOffset;
        TVec3f upOffset;
        PSMTXMultVecSR(mMatrices[0], &offsets[1], &sideOffset);
        PSMTXMultVecSR(mMatrices[0], &offsets[0], &upOffset);
        MR::setShadowProjection(this, "UpShadow", mPosition + upOffset, -(*reinterpret_cast<TVec3f*>(&_190)), true);
        MR::setShadowProjection(this, "SideShadow", mPosition + sideOffset, (*reinterpret_cast<TVec3f*>(&_19C)), true);
        TVec3f up;
        MR::calcUpVec(&up, this);
        f32 angle = 57.29578f * MR::diffAngleAbs((*reinterpret_cast<TVec3f*>(&_190)), up);
        if (angle < 40.0f) {
            MR::invalidateShadow(this, "UpShadow");
        } else {
            mMatrices[3].set(getBaseMtx());
            TVec3f pos = mPosition + upOffset;
            MR::setMtxTrans(mMatrices[3], pos.x, pos.y, pos.z);
            MR::validateShadow(this, "UpShadow");
        }
        if (angle > 50.0f) {
            MR::invalidateShadow(this, "SideShadow");
        } else {
            mMatrices[3].set(getBaseMtx());
            TVec3f pos = mPosition + sideOffset;
            MR::setMtxTrans(mMatrices[3], pos.x, pos.y, pos.z);
            MR::validateShadow(this, "SideShadow");
        }
        f32 upScale = MR::clamp(0.5f + (angle - 40.0f) / (90.0f - 40.0f), 0.1f, 1.0f);
        mMatrices[1].set(getBaseMtx());
        mMatrices[1].setScale(upScale);
        f32 sideScale = 1.0f - MR::clamp(angle / 50.0f - 0.5f, 0.0f, 1.0f - 0.1f);
        mMatrices[2].set(getBaseMtx());
        mMatrices[2].setScale(sideScale);
    }
    TVec3f offset(offsets[3]);
    if (_1A9) offset.x = -offset.x;
    TVec3f up(0.0f);
    mMatrices[0].getZDir(up);
    if (_1A9) {
        TVec3f gravity(-mGravity);
        PSVECCrossProduct(&gravity, &up, &up);
    } else {
        PSVECCrossProduct(&mGravity, &up, &up);
    }
    TPos3f collisionMtx;
    MR::makeMtxUpFront(&collisionMtx, up, -mGravity);
    TPos3f mtx;
    mtx.set(getBaseMtx());
    MR::addTransMtxLocal(mtx, offset);
    TVec3f pos(0.0f);
    mtx.getTrans(pos);
    MR::setMtxTrans(collisionMtx, pos.x, pos.y, pos.z);
    (*reinterpret_cast<CollisionParts**>(&_1AC))->setMtx(collisionMtx);
}

RotateHammerPressure::~RotateHammerPressure() {
}
