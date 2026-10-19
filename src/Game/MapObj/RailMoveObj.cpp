#include "Game/MapObj/RailMoveObj.hpp"
#include "Game/Util.hpp"
#include "Game/Demo/DemoCtrlBase.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/MapObj/MapPartsRailRotator.hpp"
#include "Game/MapObj/StageEffectDataTable.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapPartsUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    const char* cMoveBckName = "Move";
};  // namespace

namespace NrvRailMoveObj {
    NEW_NERVE(HostTypeWait, RailMoveObj, Wait);
    NEW_NERVE(HostTypeWaitForPlayerOn, RailMoveObj, Wait);
    NEW_NERVE(HostTypeMove, RailMoveObj, Move);
    NEW_NERVE(HostTypeDone, RailMoveObj, Wait);
    NEW_NERVE(RailMoveObjHostTypeDemoMove, RailMoveObj, DemoMove);
};  // namespace NrvRailMoveObj

RailMoveObj::RailMoveObj(const char* pName) : MapObjActor(pName), _D0(gZeroVec) {
    _DC = 0;
}

RailMoveObj::RailMoveObj(const char* pName, const char* pObjName) : MapObjActor(pName, pObjName), _D0(gZeroVec) {
    _DC = 0;
}

void RailMoveObj::init(const JMapInfoIter& rIter) {
    MapObjActor::init(rIter);
    MapObjActorInitInfo info;
    info.setupHioNode("地形オブジェ");
    info.setupDefaultPos();
    info.setupConnectToScene();
    info.setupEffect(0);
    info.setupSound(0x6);
    info.setupSoundPos(getSoundCalcPos());
    info.setupGroupClipping(0x20);
    info.setupSeAppear();
    info.setupRailMover();
    info.setupRailPosture();
    info.setupShadow(0);
    info.setupBaseMtxFollowTarget();
    info.setupNerve(GET_NERVE(RailMoveObj, HostTypeMove));
    MapObjActorUtil::setupInitInfoTypical(&info, mObjectName);

    const char* objName = mObjectName;
    if (MR::isEqualString("GravityFollowLift", objName) || MR::isEqualString("GravityFollowLiftB", objName) ||
        MR::isEqualString("GravityFollowLiftLongPanel", objName) || MR::isEqualString("GravityFollowPanel", objName) ||
        MR::isEqualString("GravityFollowLiftBig", objName) || MR::isEqualString("KoopaLv2LavaLift", objName) ||
        MR::isEqualString("LavaBeforeRepeatBlockFloater", objName) || MR::isEqualString("LavaAcrossFloater", objName)) {
        info.setupProjmapMtx(true, false);
    }

    setupInitInfo(rIter, &info);
    initialize(rIter, info);

    if (MR::isConnectedWithRail(rIter)) {
        f32 radius = 0.0f;
        MR::calcModelBoundingRadius(&radius, this);
        MR::initAndSetRailClipping(&_D0, this, 100.0f, radius);
    } else {
        setNerve(GET_NERVE(RailMoveObj, HostTypeDone));
    }

    s32 condition_type = 0;
    MR::getMapPartsArgMoveConditionType(&condition_type, rIter);

    if (!MR::isMoveStartTypeUnconditional(condition_type)) {
        setNerve(GET_NERVE(RailMoveObj, HostTypeWaitForPlayerOn));
    }

    initDemoAction();
}

void RailMoveObj::initDemoAction() {
    if (MR::isDemoCast(this, 0) && MR::tryRegisterDemoActionNerve(this, GET_NERVE(RailMoveObj, HostTypeMove), 0)) {
        setNerve(GET_NERVE(RailMoveObj, HostTypeWait));
    }
}

TVec3f* RailMoveObj::getSoundCalcPos() {
    return 0;
}

void RailMoveObj::startClipped() {
    MapObjActor::startClipped();

    if (mRailMover) {
        MR::StageEffect::stopShakingCameraMoving(this, mObjectName);
    }
}

void RailMoveObj::endClipped() {
    MapObjActor::endClipped();

    if (mRailMover) {
        if (MapObjActorUtil::isRailMoverWorking(this)) {
            if (!isNerve(GET_NERVE(RailMoveObj, HostTypeDone))) {
                MR::StageEffect::shakeCameraMoving(this, mObjectName);
            }
        }
    }
}

void RailMoveObj::initCaseUseSwitchB(const MapObjActorInitInfo& rInitInfo) {
    MapObjActor::initCaseUseSwitchB(rInitInfo);
    setNerve(GET_NERVE(RailMoveObj, HostTypeWait));
}

void RailMoveObj::initCaseNoUseSwitchB(const MapObjActorInitInfo&) {
}

void RailMoveObj::startMoveInner() {
    MR::StageEffect::tryStageEffectStart(this, mObjectName);

    if (MR::isExistBck(this, ::cMoveBckName)) {
        MR::startAction(this, ::cMoveBckName);
    }

    MR::StageEffect::shakeCameraMoving(this, mObjectName);
}

void RailMoveObj::move() {
    if (MapObjActorUtil::isRailMoverWorking(this)) {
        MR::StageEffect::tryStageEffectMoving(this, mObjectName);
    }
}

void RailMoveObj::doAtEndPoint() {
    MR::StageEffect::tryStageEffectStop(this, mObjectName);

    if (MR::isExistBck(this, ::cMoveBckName) && MR::isBckPlaying(this, ::cMoveBckName)) {
        MR::stopBck(this);
    }

    MR::StageEffect::stopShakingCameraMoving(this, mObjectName);
}

bool RailMoveObj::endMove() {
    doAtEndPoint();
    return true;
}

bool RailMoveObj::isMoving() const {
    return MapObjActorUtil::isRailMoverWorking(this);
}

bool RailMoveObj::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xDC /* ACTMES_MAPPARTS_DISAPPEAR_WITH_BLINK */ &&
        (isNerve(GET_NERVE(RailMoveObj, HostTypeMove)) || isNerve(GET_NERVE(RailMoveObj, HostTypeDone)))) {
        kill();

        return true;
    }

    return false;
}

void RailMoveObj::exeWait() {
    if (!isNerve(GET_NERVE(RailMoveObj, HostTypeDone)) && (!MR::isDemoCast(this, 0) || !MR::isRegisteredDemoActionNerve(this)) &&
        (!MR::isValidSwitchB(this) || MR::isOnSwitchB(this)) && (!isNerve(GET_NERVE(RailMoveObj, HostTypeWaitForPlayerOn)) || MR::isOnPlayer(this))) {
        tryStartMove();
    }
}

void RailMoveObj::setNerveMove() {
    setNerve(GET_NERVE(RailMoveObj, HostTypeMove));
}

void RailMoveObj::setNerveWait() {
    setNerve(GET_NERVE(RailMoveObj, HostTypeWait));
}

bool RailMoveObj::tryStartMove() {
    setNerve(GET_NERVE(RailMoveObj, HostTypeMove));
    return true;
}

void RailMoveObj::exeDemoMove() {
    if (MR::isFirstStep(this)) {
        mRailMover->end();
        MR::startAction(this, "DemoMove");
    }

    if (MR::isActionEnd(this)) {
        kill();
    }
}

void RailMoveObj::exeMove() {
    if (MR::isFirstStep(this)) {
        startMapPartsFunctions();
    }

    if (!_DC && MapObjActorUtil::isRailMoverWorking(this)) {
        startMoveInner();
    }

    _DC = MapObjActorUtil::isRailMoverWorking(this);
    move();

    if (mRailMover->isReachedEnd()) {
        if (mRailMover->isDone() && endMove()) {
            setNerve(GET_NERVE(RailMoveObj, HostTypeDone));
        } else {
            doAtEndPoint();
        }
    }
}

RailMoveObjPress::RailMoveObjPress(const char* pName) : RailMoveObj(pName) {
}

void RailMoveObjPress::init(const JMapInfoIter& rIter) {
    RailMoveObj::init(rIter);
    MR::setBodySensorType(this, ATYPE_MAP_OBJ_PRESS);
}

RailMoveObjBreakAtEnd::RailMoveObjBreakAtEnd(const char* pName) : RailMoveObj(pName) {
}

void RailMoveObjBreakAtEnd::doAtEndPoint() {
    RailMoveObj::doAtEndPoint();

    if (MR::isRegisteredEffect(this, "Break")) {
        MR::emitEffect(this, "Break");
    }

    kill();
}

void RailMoveIndirectObj::connectToScene(const MapObjActorInitInfo& rInfo) {
    MR::connectToSceneIndirectMapObj(this);
}

void RailMoveObjSwitchShadow::init(const JMapInfoIter& rIter) {
    RailMoveObj::init(rIter);
    MR::invalidateShadow(this, 0);
}

void RailMoveObjSwitchShadow::initCaseUseSwitchA(const MapObjActorInitInfo& rInfo) {
    MR::listenStageSwitchOnA(this, MR::FunctorV0M< RailMoveObjSwitchShadow*, void (RailMoveObjSwitchShadow::*)() >(this, &RailMoveObjSwitchShadow::startOnShadow));
}

void RailMoveObjSwitchShadow::startOnShadow() {
    MR::validateShadow(this, 0);
}

RailDemoMoveObj::RailDemoMoveObj(const char* pName) : RailMoveObj(pName) {
    mDemoControl = 0;
}

void RailDemoMoveObj::init(const JMapInfoIter& rIter) {
    RailMoveObj::init(rIter);
    mDemoControl = new DemoCtrlBase(this, mObjectName);
    mDemoControl->init(rIter);
    makeActorAppeared();
}

bool RailDemoMoveObj::tryStartMove() {
    if (!mDemoControl->tryStart()) {
        return false;
    }

    setNerve(GET_NERVE(RailMoveObj, HostTypeMove));
    return true;
}

void RailDemoMoveObj::startMoveInner() {
}

void RailDemoMoveObj::move() {
    mDemoControl->update();
}

bool RailDemoMoveObj::endMove() {
    MR::validateClipping(this);

    if (!mDemoControl->isExistEndFrame()) {
        mDemoControl->end();
    }

    return mDemoControl->isDone();
}

RailRotateMoveObj::RailRotateMoveObj(const char* pName) : RailMoveObj(pName) {
}

void RailRotateMoveObj::setupInitInfo(const JMapInfoIter& rIter, MapObjActorInitInfo* pInfo) {
    pInfo->setupRailRotator();
}

bool RailRotateMoveObj::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xD8 /* ACTMES_MAPPARTS_START_ROTATE_AT_POINT */) {
        return tryStartRotateAtPoint();
    }

    if (msg == 0xD9 /* ACTMES_MAPPARTS_END_ROTATE_AT_POINT */) {
        mRailMover->endRotateAtPoint();
        return true;
    }

    if (msg == 0xDA /* ACTMES_MAPPARTS_START_ROTATE_BETWEEN_POINTS */) {
        return tryStartRotateBetweenPoints();
    }

    return RailMoveObj::receiveOtherMsg(msg, pSender, pReceiver);
}

void RailRotateMoveObj::initCaseUseSwitchB(const MapObjActorInitInfo& rInfo) {
    setNerve(GET_NERVE(RailMoveObj, HostTypeWait));
    MR::listenStageSwitchOffB(this, MR::FunctorV0M< MapObjActor*, void (MapObjActor::*)() >(this, &MapObjActor::pauseMapPartsFunctions));
}

bool RailRotateMoveObj::tryStartRotateAtPoint() {
    s32 cur_point = MR::getCurrentRailPointNo(this);
    if (!mRailRotator->hasRotation(cur_point)) {
        return false;
    }

    if (mRailRotator->hasRotationBetweenPoints(cur_point)) {
        return false;
    }

    mRailRotator->rotateAtPoint(cur_point);
    return true;
}

bool RailRotateMoveObj::tryStartRotateBetweenPoints() {
    MapPartsRailRotator* rail_rotator = mRailRotator;
    s32 cur_point = MR::getCurrentRailPointNo(this);

    if (!rail_rotator->hasRotationBetweenPoints(cur_point)) {
        return false;
    }

    f32 next_time = 0.0f;
    mRailMover->calcTimeToNextRailPoint(&next_time);
    MapPartsRailRotator* uhm = mRailRotator;
    s32 new_point = MR::getCurrentRailPointNo(this);
    uhm->rotateBetweenPoints(new_point, next_time);
    return true;
}

RailAndRotateMoveObj::RailAndRotateMoveObj(const char* pName) : RailMoveObj(pName) {
}

void RailAndRotateMoveObj::init(const JMapInfoIter& rIter) {
    RailMoveObj::init(rIter);
}

void RailAndRotateMoveObj::setupInitInfo(const JMapInfoIter& rIter, MapObjActorInitInfo* pInfo) {
    pInfo->setupRotator();
}

RailMoveShadowDropYObj::RailMoveShadowDropYObj(const char* pName) : RailMoveObj(pName) {
    mShadowDropDir.x = 0.0f;
    mShadowDropDir.y = 0.0f;
    mShadowDropDir.z = 0.0f;
}

void RailMoveShadowDropYObj::init(const JMapInfoIter& rIter) {
    RailMoveObj::init(rIter);
    MR::calcUpVec(&mShadowDropDir, this);
    TVec3f negated = -mShadowDropDir;
    mShadowDropDir = negated;
    MR::setShadowDropDirectionPtrAll(this, &mShadowDropDir);
}

void RailMoveShadowDropYObj::control() {
    MapObjActor::control();
    MR::calcUpVec(&mShadowDropDir, this);
    TVec3f negated = -mShadowDropDir;
    mShadowDropDir = negated;
}


bool RailMoveObjSwitchWarpEnd::tryStartMove() {
    if (MR::isValidSwitchA(this) && MR::isOnSwitchA(this)) {
        mRailMover->moveToEndPos();
        MR::setPosition(this, mRailMover->_28);
        MR::onSwitchDead(this);
        setNerve(GET_NERVE(RailMoveObj, HostTypeDone));
        return true;
    }

    setNerve(GET_NERVE(RailMoveObj, HostTypeMove));
    return true;
}

void RailMoveObjSwitchWarpEnd::startMoveInner() {
    if (MR::isRegisteredEffect(this, "Start")) {
        MR::emitEffect(this, "Start");
    }

    if (MR::isExistBck(this, "Move")) {
        MR::startAction(this, "Move");
    }
}

void RailMoveObjSwitchWarpEnd::move() {
    if (!mRailMover->isStarting()) {
        if (!_DD) {
            MR::StageEffect::rumblePadStart(this, mObjectName);
            MR::StageEffect::shakeStartCamera(this, mObjectName);
            MR::StageEffect::shakeCameraMoving(this, mObjectName);
            _DD = 1;
        }

        if (MapObjActorUtil::isRailMoverWorking(this)) {
            MR::StageEffect::tryStageEffectMoving(this, mObjectName);
        }
    }
}

void RailMoveObjSwitchWarpEnd::doAtEndPoint() {
    RailMoveObj::doAtEndPoint();
    setNerve(GET_NERVE(RailMoveObj, HostTypeDone));
    MR::onSwitchDead(this);
}

RailMoveObjShakeCamera::RailMoveObjShakeCamera(const char* pName) : RailMoveObj(pName) {
}

void RailMoveObjShakeCamera::doAtEndPoint() {
    if (!MR::isRailGoingToEnd(this)) {
        MR::StageEffect::tryStageEffectStop(this, mObjectName);
    }

    if (MR::isExistBck(this, ::cMoveBckName) && MR::isBckPlaying(this, ::cMoveBckName)) {
        MR::stopBck(this);
    }

    MR::StageEffect::stopShakingCameraMoving(this, mObjectName);
}

RailMoveWithIndirectModelObj::RailMoveWithIndirectModelObj(const char* pName) : RailMoveObj(pName) {
    mIndirectModel = 0;
}

void RailMoveWithIndirectModelObj::init(const JMapInfoIter& rIter) {
    RailMoveObj::init(rIter);
    mIndirectModel = MR::createIndirectModel(this, getBaseMtx());
}

void RailSeesawMoveObj::setupInitInfo(const JMapInfoIter& rIter, MapObjActorInitInfo* pInfo) {
    MapObjActorUtil::setupInitInfoSeesaw(pInfo, rIter, 0, 0.0f);
}

RailMoveDemoActionObj::RailMoveDemoActionObj(const char* pName) : RailMoveObj(pName) {
}

void RailMoveDemoActionObj::initDemoAction() {
    if (MR::isDemoCast(this, 0)) {
        MR::tryRegisterDemoActionNerve(this, GET_NERVE(RailMoveObj, RailMoveObjHostTypeDemoMove), 0);
    }
}

void RailMoveWithReverseAnimObj::startMoveInner() {
    MR::StageEffect::tryStageEffectStart(this, mObjectName);

    if (MR::isRailGoingToEnd(this)) {
        if (MR::isExistBck(this, ::cMoveBckName)) {
            MR::startAction(this, ::cMoveBckName);
        }
    } else {
        if (MR::isExistBck(this, "Reverse")) {
            MR::startAction(this, "Reverse");
        }
    }

    MR::StageEffect::shakeCameraMoving(this, mObjectName);
}

void RailMoveWithReverseAnimObj::doAtEndPoint() {
    MR::StageEffect::tryStageEffectStop(this, mObjectName);

    if (MR::isExistBck(this, ::cMoveBckName) && MR::isBckPlaying(this, ::cMoveBckName)) {
        MR::stopBck(this);
    }

    if (MR::isExistBck(this, "Reverse") && MR::isBckPlaying(this, "Reverse")) {
        MR::stopBck(this);
    }

    MR::StageEffect::stopShakingCameraMoving(this, mObjectName);
}

RailMoveLavaProjmapObj::RailMoveLavaProjmapObj(const char* pName) : RailMoveObj(pName) {
    _E0.x = 0.0f;
    _E0.y = 0.0f;
    _E0.z = 0.0f;
    _EC = 0.0f;
    _F0 = 0.0f;
}

// PLACEHOLDER(objects): fn_800389C0 (TVec3f* out, LiveActor*) is an unnamed MapUtil helper (calcGravityVector, calcModelBoundingRadius); resolve when named
extern "C" void fn_800389C0(TVec3f*, const LiveActor*);

// NONMATCHING: same ops and calls; target keeps extra f32/TVec3f stack temporaries (frame 0x60, gravity at 0x44) and PS-register allocation differs
void RailMoveLavaProjmapObj::initAfterPlacement() {
    MR::calcGravity(this);
    TVec3f gravity(mGravity);
    gravity *= 1000.0f;

    if (!MR::getFirstPolyOnLineToMapExceptActor(&_E0, 0, mPosition, gravity, this)) {
        fn_800389C0(&_E0, this);
    }

    _EC = PSVECDistance(&mPosition, &_E0);

    TVec3f diff(mPosition);
    diff -= _E0;
    if (diff.dot(mGravity) < 0.0f) {
        _EC *= -1.0f;
    }

    if (mMatrixSetter) {
        TVec3f offset(0.0f, _EC, 0.0f);
        mMatrixSetter->updateMtxUseBaseMtxWithLocalOffset(offset);
    }
}

void RailMoveLavaProjmapObj::updateProjmapMtx() {
}
