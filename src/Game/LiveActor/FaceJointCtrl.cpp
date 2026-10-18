#include "Game/LiveActor/FaceJointCtrl.hpp"
#include "Game/LiveActor/DynamicJointCtrl.hpp"
#include "Game/Util/JointController.hpp"

#include "Game/Util.hpp"

namespace {
    const char* sTextOutFileName = "FaceJointCtrl";
    inline void normalizeFaceMatrix(TPos3f* matrix) {
            f32 scale = JGeometry::TUtil< f32 >::sqrt(
                (matrix->get(0, 0) * matrix->get(0, 0)) + (matrix->get(1, 0) * matrix->get(1, 0)) + (matrix->get(2, 0) * matrix->get(2, 0)) +
                (matrix->get(0, 1) * matrix->get(0, 1)) + (matrix->get(1, 1) * matrix->get(1, 1)) + (matrix->get(2, 1) * matrix->get(2, 1)) +
                (matrix->get(0, 2) * matrix->get(0, 2)) + (matrix->get(1, 2) * matrix->get(1, 2)) + (matrix->get(2, 2) * matrix->get(2, 2)));

            if (matrix) {
            f32 invLenX = JGeometry::TUtil< f32 >::inv_sqrt((matrix->get(0, 0) * matrix->get(0, 0)) + (matrix->get(1, 0) * matrix->get(1, 0)) +
                                                            (matrix->get(2, 0) * matrix->get(2, 0)));

            matrix->mMtx[0][0] = invLenX * matrix->get(0, 0);
            matrix->mMtx[1][0] = invLenX * matrix->get(1, 0);
            matrix->mMtx[2][0] = invLenX * matrix->get(2, 0);

            f32 invLenY = JGeometry::TUtil< f32 >::inv_sqrt((matrix->get(0, 1) * matrix->get(0, 1)) + (matrix->get(1, 1) * matrix->get(1, 1)) +
                                                            (matrix->get(2, 1) * matrix->get(2, 1)));

            matrix->mMtx[0][1] = invLenY * matrix->get(0, 1);
            matrix->mMtx[1][1] = invLenY * matrix->get(1, 1);
            matrix->mMtx[2][1] = invLenY * matrix->get(2, 1);

            f32 invLenZ = JGeometry::TUtil< f32 >::inv_sqrt((matrix->get(0, 2) * matrix->get(0, 2)) + (matrix->get(1, 2) * matrix->get(1, 2)) +
                                                            (matrix->get(2, 2) * matrix->get(2, 2)));

            matrix->mMtx[0][2] = invLenZ * matrix->get(0, 2);
            matrix->mMtx[1][2] = invLenZ * matrix->get(1, 2);
            matrix->mMtx[2][2] = invLenZ * matrix->get(2, 2);
            }
    }
}

FaceJointCtrlParam::FaceJointCtrlParam() : mDegreeLeft(45.0f), mDegreeRight(-1.0f), mDegreeVertical(45.0f) {}

FaceJointCtrl::FaceJointCtrl(LiveActor* actor, const char* joint, FaceJointCtrlParam* param)
    : mHostActor(actor), mJointName(joint), mParam(param), mDirection(0.0f, 0.0f, 1.0f), mJointController(nullptr) {
    mJointCtrlRate = new JointCtrlRate();
    mTargetPosition = nullptr;
    mRotation.identity();
    mJointController = MR::createJointDelegator< FaceJointCtrl >(this, mHostActor, &FaceJointCtrl::updateJointMtxCallBack, nullptr, mJointName);
}

void FaceJointCtrl::startCtrl(s32 time) {
    if (mJointController) {
        mJointCtrlRate->startCtrl(time);
        update();
    }
}

void FaceJointCtrl::endCtrl(s32 time) {
    if (mJointController) {
        mJointCtrlRate->endCtrl(time);
        update();
    }
}

void FaceJointCtrl::update() {
    if (!mJointController) {
        return;
    }

    mJointCtrlRate->update();
}

void FaceJointCtrl::setCallBackFunction() {
    if (!mJointController) {
        return;
    }

    mJointController->registerCallBack();
}

// NONMATCHING: regalloc and instruction scheduling
bool FaceJointCtrl::updateJointMtxCallBack(TPos3f* matrix, const JointControllerInfo&) {
    TVec3f scale;
    matrix->getScale(scale);
    normalizeFaceMatrix(matrix);
    TPos3f previous;
    previous.concat(*matrix, mRotation);
    TVec3f position;
    matrix->getTrans(position);
    TVec3f direction;
    if (mTargetPosition) {
        direction.x = mTargetPosition->x;
        direction.y = mTargetPosition->y;
        direction.z = mTargetPosition->z;
    } else {
        TPos3f head;
        MR::calcPlayerJointMtx(&head, "Head");
        head.getTrans(direction);
    }
    direction -= position;
    if (MR::isNearZero(direction, 0.001f)) {
        return false;
    }
    MR::normalize(&direction);
    TPos3f desired(*matrix);
    TVec3f up;
    up.set< f32 >(desired.mMtx[0][1], desired.mMtx[1][1], desired.mMtx[2][1]);
    TVec3f horizontal;
    JMAVECScaleAdd(&up, &direction, &horizontal, -up.dot(direction));
    if (!MR::normalizeOrZero(&horizontal)) {
        TVec3f front;
        desired.getZDir(front);
        if (front.dot(horizontal) >= 0.0f) {
            MR::turnMtxToXDirDegree(&desired, horizontal, mParam->mDegreeRight * mJointCtrlRate->_0);
        } else {
            MR::turnMtxToXDirDegree(&desired, horizontal, mParam->mDegreeLeft * mJointCtrlRate->_0);
        }
    }
    TVec3f side;
    TVec3f front;
    desired.getXDir(side);
    desired.getZDir(front);
    TVec3f vertical;
    JMAVECScaleAdd(&front, &direction, &vertical, -front.dot(direction));
    f32 factor = horizontal.dot(side);
    if (!MR::normalizeOrZero(&vertical) && factor > 0.0f) {
        MR::turnMtxToXDirDegree(&desired, vertical, mParam->mDegreeVertical * factor * mJointCtrlRate->_0);
    }
    TVec3f oldSide, baseSide, newSide;
    previous.getXDir(oldSide);
    matrix->getXDir(baseSide);
    desired.getXDir(newSide);
    TVec3f oldCross, newCross;
    PSVECCrossProduct(&oldSide, &baseSide, &oldCross);
    PSVECCrossProduct(&oldSide, &newSide, &newCross);
    TQuat4f goal, before;
    desired.getQuat(goal);
    previous.getQuat(before);
    TQuat4f current;
    current.set(before.x, before.y, before.z, before.w);
    current.slerp(goal, 0.1f);
    TPos3f result;
    result.setQT(current, position);
    TPos3f inverse(*matrix);
    inverse.invert(inverse);
    mRotation.concat(inverse, result);
    matrix->setInline(result);
    MR::preScaleMtx(*matrix, scale);
    return true;
}

FaceJointCtrlKeeper* FaceJointCtrlKeeper::tryCreate(LiveActor* actor) {
    const ResourceHolder* resource = MR::getResourceHolder(actor);
    if (!MR::isExistFileInArc(resource, "%s.bcsv", sTextOutFileName)) {
        return nullptr;
    }
    return new FaceJointCtrlKeeper(actor);
}

void FaceJointCtrlKeeper::update() {
    for (s32 i = 0; i < mCount; i++) mControllers[i]->update();
}

void FaceJointCtrlKeeper::setCallBackFunction() {
    for (s32 i = 0; i < mCount; i++) mControllers[i]->setCallBackFunction();
}

void FaceJointCtrlKeeper::startCtrl(s32 step) {
    for (s32 i = 0; i < mCount; i++) mControllers[i]->startCtrl(step);
}

void FaceJointCtrlKeeper::endCtrl(s32 step) {
    for (s32 i = 0; i < mCount; i++) mControllers[i]->endCtrl(step);
}

FaceJointCtrlKeeper::FaceJointCtrlKeeper(LiveActor* actor) {
    mHost = actor;
    mCount = 0;
    mControllers = nullptr;
    const ResourceHolder* resource = MR::getResourceHolder(actor);
    JMapInfo* csv = MR::tryCreateCsvParser(resource, "%s.bcsv", sTextOutFileName);
    if (!csv) return;
    mCount = csv->getNumEntries();
    mControllers = new FaceJointCtrl*[mCount];
    for (s32 i = 0; i < mCount; i++) {
        FaceJointCtrlParam* param = new FaceJointCtrlParam();
        const char* joint = nullptr;
        MR::getCsvDataStrOrNULL(&joint, csv, "JointName", i);
        if (MR::hasCsvItem(csv, "DegreeMaxHL")) {
            MR::getCsvDataF32(&param->mDegreeLeft, csv, "DegreeMaxHL", i);
            MR::getCsvDataF32(&param->mDegreeRight, csv, "DegreeMaxHR", i);
            MR::getCsvDataF32(&param->mDegreeVertical, csv, "DegreeMaxV", i);
            if (param->mDegreeRight < 0.0f) param->mDegreeRight = param->mDegreeLeft;
        } else {
            MR::getCsvDataF32(&param->mDegreeLeft, csv, "DegreeMax", i);
            MR::getCsvDataF32(&param->mDegreeVertical, csv, "DegreeMaxVertical", i);
            param->mDegreeRight = param->mDegreeLeft;
        }
        mControllers[i] = new FaceJointCtrl(actor, joint, param);
    }
}
