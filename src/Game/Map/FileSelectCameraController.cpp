#include "Game/Map/FileSelectCameraController.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

namespace {
    // const Vec cTitleTargetPos = {};
    // const Vec cTitlePos = {};
    const Vec cFarTarget = {0.0f, 1500.0f, 0.0f};
    const Vec cFarPoint = {0.0f, 1500.0f, 10000.0f};
    const Vec cNearTargetOffset = {0.0f, 1400.0f, 0.0f};
    const Vec cNearPointOffset = {0.0f, 0.0f, 4800.0f};
    const f32 cTitleFovy = 60.0f;
    const f32 cFarFovy = 40.0f;
    const f32 cNearFovy = 50.0f;
};  // namespace

namespace NrvFileSelectCameraController {
    NEW_NERVE(FileSelectCameraControllerNrvTitle, FileSelectCameraController, Title);
    NEW_NERVE(FileSelectCameraControllerNrvMoveToFarPoint, FileSelectCameraController, MoveToFarPoint);
    NEW_NERVE(FileSelectCameraControllerNrvFarPoint, FileSelectCameraController, FarPoint);
    NEW_NERVE(FileSelectCameraControllerNrvMoveToNearPoint, FileSelectCameraController, MoveToNearPoint);
    NEW_NERVE(FileSelectCameraControllerNrvNearPoint, FileSelectCameraController, NearPoint);
};  // namespace NrvFileSelectCameraController

FileSelectCameraController::FileSelectCameraController(const char* pName)
    : LiveActor(pName), mWPoint(0.0f, 0.0f, 0.0f), mPrevWPoint(0.0f, 0.0f, 0.0f), mFovy(::cTitleFovy), mPrevFovy(::cTitleFovy),
      mUpVec(0.0f, 1.0f, 0.0f) {
}

void FileSelectCameraController::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    MR::initActorCameraProgrammable(this);
    initNerve(GET_NERVE(FileSelectCameraController, FileSelectCameraControllerNrvTitle));
    makeActorDead();
}

// NONMATCHING: chained vector-assignment code generation differs; camera state and parameters match.
void FileSelectCameraController::appear() {
    LiveActor::appear();
    setNerve(GET_NERVE(FileSelectCameraController, FileSelectCameraControllerNrvFarPoint));
    MR::startActorCameraProgrammable(this, 0);
    mUpVec.set(0.0f, 1.0f, 0.0f);
    mPrevWPoint = TVec3f(::cFarTarget);
    mWPoint = mPrevWPoint;
    mFovy = mPrevFovy = ::cFarFovy;
    MR::setPosition(this, TVec3f(::cFarPoint));
    MR::setProgrammableCameraParam(this, mPrevWPoint, mPosition, mUpVec);
    MR::setProgrammableCameraParamFovy(this, mPrevFovy);
}

void FileSelectCameraController::kill() {
    LiveActor::kill();
    MR::endActorCameraProgrammable(this, -1, true);
}

void FileSelectCameraController::goToFarPoint() {
    setNerve(GET_NERVE(FileSelectCameraController, FileSelectCameraControllerNrvMoveToFarPoint));
}

void FileSelectCameraController::goToNearPoint(const TVec3f& rPoint) {
    TVec3f target = rPoint + TVec3f(::cNearTargetOffset);
    mNearTarget.x = target.x;
    mNearTarget.y = target.y;
    mNearTarget.z = target.z;

    setNerve(GET_NERVE(FileSelectCameraController, FileSelectCameraControllerNrvMoveToNearPoint));
}

bool FileSelectCameraController::isAtFarPoint() const {
    return isNerve(GET_NERVE(FileSelectCameraController, FileSelectCameraControllerNrvFarPoint));
}

bool FileSelectCameraController::isAtNearPoint() const {
    return isNerve(GET_NERVE(FileSelectCameraController, FileSelectCameraControllerNrvNearPoint));
}

bool FileSelectCameraController::isToOrAtFarPoint() const {
    return isNerve(GET_NERVE(FileSelectCameraController, FileSelectCameraControllerNrvMoveToFarPoint)) || isNerve(GET_NERVE(FileSelectCameraController, FileSelectCameraControllerNrvFarPoint));
}

bool FileSelectCameraController::isToOrAtNearPoint() const {
    return isNerve(GET_NERVE(FileSelectCameraController, FileSelectCameraControllerNrvMoveToNearPoint)) || isNerve(GET_NERVE(FileSelectCameraController, FileSelectCameraControllerNrvNearPoint));
}

void FileSelectCameraController::exeTitle() {
    if (MR::isFirstStep(this)) {
        mWPoint.set< f32 >(::cFarTarget.x, ::cFarTarget.y + 15000.0f, ::cFarTarget.z);
        MR::setPosition(this, ::cFarPoint.x, ::cFarTarget.y + 15000.0f, ::cFarPoint.z);
        mUpVec.set(0.0f, 1.0f, 0.0f);
    }
}

void FileSelectCameraController::exeMoveToFarPoint() {
    if (MR::isFirstStep(this)) {
        mUpVec.set(0.0f, 1.0f, 0.0f);
    }

    f32 squaredTime = getNerveStep() / 60.0f;

    squaredTime *= squaredTime;
    mWPoint += (TVec3f(::cFarTarget) - mWPoint) * squaredTime;
    mFovy += (::cFarFovy - mFovy) * squaredTime;
    mPosition.add((TVec3f(::cFarPoint) - mPosition) * squaredTime);

    MR::setNerveAtStep(this, GET_NERVE(FileSelectCameraController, FileSelectCameraControllerNrvFarPoint), 60);
}

void FileSelectCameraController::exeFarPoint() {
    mUpVec.set(0.0f, 1.0f, 0.0f);
    mWPoint = ::cFarTarget;
    mFovy = ::cFarFovy;
    MR::setPosition(this, TVec3f(::cFarPoint));
}

void FileSelectCameraController::exeMoveToNearPoint() {
    TVec3f nearPoint = mNearTarget + ::cNearPointOffset;
    f32 squaredTime = getNerveStep() / 60.0f;

    squaredTime *= squaredTime;
    mWPoint += (mNearTarget - mWPoint) * squaredTime;
    mFovy += (::cNearFovy - mFovy) * squaredTime;
    mPosition += (nearPoint - mPosition) * squaredTime;

    MR::setNerveAtStep(this, GET_NERVE(FileSelectCameraController, FileSelectCameraControllerNrvNearPoint), 60);
}

void FileSelectCameraController::exeNearPoint() {
    TVec3f nearPoint = mNearTarget + ::cNearPointOffset;

    mFovy = ::cNearFovy;
    MR::setPosition(this, nearPoint);
}

void FileSelectCameraController::control() {
    MR::setProgrammableCameraParam(this, mPrevWPoint, mPosition, mUpVec);
    MR::setProgrammableCameraParamFovy(this, mPrevFovy);

    mPrevWPoint = mWPoint;
    mPrevFovy = mFovy;
}
