#include "Game/MapObj/AnmModelObj.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/MapObj/StageEffectDataTable.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace {
    const char* cFollowJointName = "Move";
    const char* cAnimFileName = "Move";
    const char* cEndLoopEffectName = "EndLoop";
    const char* cReverseName = "Reverse";
};  // namespace

namespace NrvAnmModelObj {
    NEW_NERVE(HostTypeWait, AnmModelObj, Wait);
    NEW_NERVE(HostTypeMove, AnmModelObj, Move);
    NEW_NERVE(HostTypeDone, AnmModelObj, Done);
};  // namespace NrvAnmModelObj

AnmModelObj::AnmModelObj(const char* pName) : MapObjActor(pName), mJointPos(0.0f, 0.0f, 0.0f) {
}

void AnmModelObj::init(const JMapInfoIter& rIter) {
    MapObjActor::init(rIter);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoSimpleMapObj(&info);
    info.setupNerve(GET_NERVE(AnmModelObj, HostTypeWait));
    MapObjActorUtil::setupInitInfoTypical(&info, mObjectName);
    initialize(rIter, info);

    if (MR::isExistJoint(this, ::cFollowJointName)) {
        MR::copyJointPos(this, ::cFollowJointName, &mJointPos);
    } else {
        mJointPos.set(mPosition);
    }

    f32 boundRadius;
    MR::calcModelBoundingRadius(&boundRadius, this);
    MR::setClippingTypeSphere(this, boundRadius, &mJointPos);
}

bool AnmModelObj::isDone() const {
    return MR::isBckOneTimeAndStopped(this);
}

bool AnmModelObj::isInvalidateCollision() const {
    return false;
}

void AnmModelObj::exeWait() {
    if (isOnStartAnmTrigger()) {
        setNerve(GET_NERVE(AnmModelObj, HostTypeMove));
    }
}

void AnmModelObj::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::tryStartAllAnim(this, ::cAnimFileName);
        MR::startActionSoundAuto(this, "AnmModelMoveStart", -1, -1, -1);
        MR::StageEffect::tryStageEffectStart(this, mObjectName);
        MR::StageEffect::shakeCameraMoving(this, mObjectName);
        onStartMove();
    }

    MR::startActionSoundAuto(this, "AnmModelMoveLoop", -1, -1, -1);
    MR::StageEffect::rumblePadMoving(this, mObjectName);
    onLoopMove();

    if (MR::isExistJoint(this, ::cFollowJointName)) {
        MR::copyJointPos(this, ::cFollowJointName, &mJointPos);
    } else {
        mJointPos.set(mPosition);
    }

    if (isDone()) {
        MR::StageEffect::rumblePadStop(this, mObjectName);
        MR::StageEffect::shakeStopCamera(this, mObjectName);
        MR::StageEffect::stopShakingCameraMoving(this, mObjectName);
        onStopMove();

        if (isInvalidateCollision()) {
            MR::invalidateCollisionParts(this);
        }

        if (isKilledAtMoveDone()) {
            kill();
        } else if (isRepeat()) {
            setNerve(GET_NERVE(AnmModelObj, HostTypeWait));
        } else {
            setNerve(GET_NERVE(AnmModelObj, HostTypeDone));
        }
    }
}

void AnmModelObj::exeDone() {
    if (MR::isFirstStep(this)) {
        if (MR::isRegisteredEffect(this, ::cEndLoopEffectName)) {
            MR::emitEffect(this, ::cEndLoopEffectName);
        }

        MR::startActionSoundAuto(this, "AnmModelStop", -1, -1, -1);
    }

    onDone();
}

AnmModelSwitchMove::AnmModelSwitchMove(const char* pName) : AnmModelObj(pName) {
}

void AnmModelSwitchMove::init(const JMapInfoIter& rIter) {
    AnmModelObj::init(rIter);

    if (MR::isDemoCast(this, nullptr)) {
        if (MR::tryRegisterDemoActionNerve(this, GET_NERVE(AnmModelObj, HostTypeMove), nullptr)) {
            setNerve(GET_NERVE(AnmModelObj, HostTypeWait));
        }
    }
}

bool AnmModelSwitchMove::isOnStartAnmTrigger() const {
    if (MR::isDemoCast(this, nullptr) && MR::isRegisteredDemoActionNerve(this)) {
        return false;
    }

    if (!MR::isValidSwitchA(this)) {
        return true;
    }

    return MR::isOnSwitchA(this);
}

void AnmModelSwitchMoveValidateCollision::init(const JMapInfoIter& rIter) {
    AnmModelObj::init(rIter);

    if (MR::isDemoCast(this, nullptr)) {
        if (MR::tryRegisterDemoActionNerve(this, GET_NERVE(AnmModelObj, HostTypeMove), nullptr)) {
            setNerve(GET_NERVE(AnmModelObj, HostTypeWait));
        }
    }

    MR::invalidateCollisionParts(this);
}

void AnmModelSwitchMoveValidateCollision::onStopMove() {
    MR::validateCollisionParts(this);
}

void AnmModelSwitchMoveEndKill::kill() {
    MapObjActor::kill();
    MR::startActionSoundAuto(this, "AnmModelKill", -1, -1, -1);
}

bool AnmModelSwitchMoveEndKill::isKilledAtMoveDone() const {
    return true;
}

void AnmModelSwitchMoveEndKillAnyAnim::init(const JMapInfoIter& rIter) {
    AnmModelObj::init(rIter);

    if (MR::isDemoCast(this, nullptr)) {
        if (MR::tryRegisterDemoActionNerve(this, GET_NERVE(AnmModelObj, HostTypeMove), nullptr)) {
            setNerve(GET_NERVE(AnmModelObj, HostTypeWait));
        }
    }

    MR::tryStartAllAnim(this, ::cAnimFileName);
    MR::setAllAnimFrameAndStop(this, ::cAnimFileName, 0.0f);
}

bool AnmModelSwitchMoveEndKillAnyAnim::isDone() const {
    return MR::isActionEnd(this);
}

AnmModelSwitchSwapVisibility::AnmModelSwitchSwapVisibility(const char* pName) : AnmModelObj(pName) {
}

void AnmModelSwitchSwapVisibility::init(const JMapInfoIter& rIter) {
    AnmModelObj::init(rIter);
    MR::startBva(this, ::cAnimFileName);
    MR::setBvaFrameAndStop(this, 0.0f);

    if (MR::isDemoCast(this, nullptr)) {
        if (MR::tryRegisterDemoActionNerve(this, GET_NERVE(AnmModelObj, HostTypeMove), nullptr)) {
            setNerve(GET_NERVE(AnmModelObj, HostTypeWait));
        }
    }
}

bool AnmModelSwitchSwapVisibility::isOnStartAnmTrigger() const {
    if (MR::isDemoCast(this, nullptr) && MR::isRegisteredDemoActionNerve(this)) {
        return false;
    }

    if (!MR::isValidSwitchA(this)) {
        return true;
    }

    return MR::isOnSwitchA(this);
}

bool AnmModelSwitchSwapVisibility::isDone() const {
    return false;
}

AnmModelSwitchSyncBrk::AnmModelSwitchSyncBrk(const char* pName) : AnmModelObj(pName) {
}

void AnmModelSwitchSyncBrk::init(const JMapInfoIter& rIter) {
    AnmModelObj::init(rIter);

    if (MR::isDemoCast(this, nullptr)) {
        if (MR::tryRegisterDemoActionNerve(this, GET_NERVE(AnmModelObj, HostTypeMove), nullptr)) {
            setNerve(GET_NERVE(AnmModelObj, HostTypeWait));
        }
    }

    if (MR::startBrkIfExist(this, ::cReverseName)) {
        MR::setBrkFrameEndAndStop(this);
    }
}

bool AnmModelSwitchSyncBrk::isDone() const {
    return MR::isBrkOneTimeAndStopped(this);
}

void AnmModelSwitchSyncBrk::onLoopMove() {
    checkForReverse();
}

void AnmModelSwitchSyncBrk::onDone() {
    checkForReverse();
}

bool AnmModelSwitchSyncBrk::isOnStartAnmTrigger() const {
    if (MR::isDemoCast(this, nullptr) && MR::isRegisteredDemoActionNerve(this)) {
        return false;
    }

    if (!MR::isValidSwitchA(this)) {
        return true;
    }

    if (MR::isOnSwitchA(this)) {
        if (!MR::isBrkOneTimeAndStopped(this)) {
            MR::stopBrk(this);
        }

        return true;
    }

    return false;
}

void AnmModelSwitchSyncBrk::checkForReverse() {
    if (MR::isValidSwitchA(this) && !MR::isOnSwitchA(this)) {
        MR::startBrk(this, ::cReverseName);
        MR::startActionSoundAuto(this, "AnmModelReverse", -1, -1, -1);
        setNerve(GET_NERVE(AnmModelObj, HostTypeWait));
    }
}

#ifdef GALAXY_NATIVE
AnmModelSwitchMoveInvalidateCollision::~AnmModelSwitchMoveInvalidateCollision() {}

bool AnmModelSwitchMoveInvalidateCollision::isInvalidateCollision() const {
    return true;
}
#endif
