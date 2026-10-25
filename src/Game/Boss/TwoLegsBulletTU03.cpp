#define JGEOMETRY_VEC3_ADD2_NOINLINE
#include "Game/Boss/TwoLegs.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"

// TU03: TwoLegs programmable camera, coin popper, joint controller, body direction, and the "reflect" IK
// controller (TwoLegsReflectCtrl) with its three nerves. Functions are written in target address order.

namespace {
    // (point, radius) pair used to grow the camera box
    struct TwoLegsSphere {
        TVec3f mPos;
        f32 mRadius;
    };

    void expandBox(TVec3f* pBox, const TwoLegsSphere* pSphere);
};  // namespace

// ---------------------------------------------------------------------------------------------
// TwoLegsCamera
// ---------------------------------------------------------------------------------------------

TwoLegsCamera::TwoLegsCamera(LiveActor* pHost) {
    mHost = pHost;
    mActive = false;
    mFovScale = 0.0f;
    mTarget.set(0.0f, 0.0f, 0.0f);
    mNumTargets = 0;
    MR::initActorCameraProgrammable(pHost);
}

void TwoLegsCamera::start() {
    mActive = true;
    mNumTargets = 0;
    calcFov(&mFovScale, &mTarget);
    MR::startActorCameraProgrammable(mHost, -1);
}

void TwoLegsCamera::keepAlive() {
    if (mActive && !MR::isEventCameraActive()) {
        MR::startActorCameraProgrammable(mHost, 120);
    }
}

void TwoLegsCamera::end() {
    mActive = false;
    MR::endActorCameraProgrammable(mHost, -1, true);
}

// (0, 0, 1) seed vector for makeMtxUpFront (SB4E01 .data lbl_806478D8)
namespace {
    const Vec sCameraUp = {0.0f, 0.0f, 1.0f};
    extern const f32 sSpinDegree;
    extern const f32 sCameraFovyRadian;
}

namespace {
    inline TVec3f scaleCameraOffset(const TVec3f& up, f32 fov) {
        TVec3f scaled;
        scaled = up;
        scaled *= fov;
        return scaled;
    }
}

void TwoLegsCamera::update() {
    if (mActive) {
        f32 fov;
        TVec3f center;
        TVec3f eye;
        TVec3f front;
        TVec3f right;
        calcFov(&fov, &center);
        fov = MR::lerp(mFovScale, fov, 0.2f);
        JMAVECLerp(&mTarget, &center, &center, 0.2f);
        TVec3f up(sCameraUp);
        MR::calcDirectionVec(&front, *MR::getPlayerCenterPos(), center);
        TPos3f mtx;
        MR::makeMtxUpFront(&mtx, up, front);
        mtx.getXDir(right);
        MR::rotateVecDegree(&up, right, -20.0f);
        TVec3f scaled = scaleCameraOffset(up, fov);
        eye.add(center, scaled);
        MR::setProgrammableCameraParamFovy(mHost, 15.0f);
        TVec3f upVec;
        upVec.x = 0.0f;
        upVec.y = 1.0f;
        upVec.z = 0.0f;
        MR::setProgrammableCameraParam(mHost, center, eye, upVec);
        mNumTargets = 0;
        mFovScale = fov;
        mTarget.set(center);
    }
}


void TwoLegsCamera::addTarget(LiveActor* pTarget) {
    if (mActive) {
        if (mNumTargets < 2) {
            mTargets[mNumTargets++] = pTarget;
        }
    }
}

void TwoLegsCamera::reset() {
    mActive = true;
    mNumTargets = 0;
}

void TwoLegsCamera::calcBox(TVec3f* pBox) {
    TVec3f center;
    TVec3f root;
    TwoLegsSphere playerSphere;
    TwoLegsSphere rootSphere;
    TwoLegsSphere targetSphere;
    TVec3f half;
    center.set<f32>(*MR::getPlayerCenterPos());
    static_cast<TwoLegs*>(mHost)->calcAllRootPosOnPlane(&root);
    playerSphere.mPos.set<f32>(center);
    playerSphere.mRadius = 350.0f;
    half.setAll<f32>(playerSphere.mRadius);
    pBox->sub(playerSphere.mPos, half);
    pBox[1].add(playerSphere.mPos, half);
    rootSphere.mPos.set<f32>(root);
    rootSphere.mRadius = 600.0f;
    expandBox(pBox, &rootSphere);
    for (s32 i = 0; i < (s32)mNumTargets; i++) {
        targetSphere.mPos.set<f32>(mTargets[i]->mPosition);
        targetSphere.mRadius = 250.0f;
        expandBox(pBox, &targetSphere);
    }
}

namespace {
    // grows the box (min at +0, max at +0xC) so that it contains the sphere
    void expandBox(TVec3f* pBox, const TwoLegsSphere* pSphere) {
        TVec3f bounds[2];
        TVec3f radius;
        radius.setAll<f32>(pSphere->mRadius);
        bounds[0].sub(pSphere->mPos, radius);
        bounds[1].add(pSphere->mPos, radius);
        if (pBox->x >= bounds[0].x) pBox->x = bounds[0].x;
        if (pBox->y >= bounds[0].y) pBox->y = bounds[0].y;
        if (pBox->z >= bounds[0].z) pBox->z = bounds[0].z;
        if (pBox[1].x <= bounds[1].x) pBox[1].x = bounds[1].x;
        if (pBox[1].y <= bounds[1].y) pBox[1].y = bounds[1].y;
        if (pBox[1].z <= bounds[1].z) pBox[1].z = bounds[1].z;
    }
};  // namespace


void TwoLegsCamera::calcFov(f32* pFov, TVec3f* pCenter) {
    TVec3f box[2];
    calcBox(&box[0]);
    f32 height;
    f32 fovy = sCameraFovyRadian;
    f32 tanHalf = JMath::sSinCosTable.sinRadian(0.5f * fovy) / JMath::sSinCosTable.cosRadian(0.5f * fovy);
    f32 width;
    f32 halfHeight = 0.5f * static_cast<f32>(MR::getScreenHeight());
    width = box[1].x - box[0].x;
    f32 dist = halfHeight / tanHalf;
    f32 fovX = (dist * width) / 608.0f;
    height = box[1].y - box[0].y;
    f32 fovY = (dist * height) / static_cast< f32 >(MR::getScreenHeight());
    fovX = (fovX >= fovY) ? fovX : fovY;
    *pFov = fovX;
    JMAVECLerp(&box[1], &box[0], pCenter, 0.5f);
}

namespace { const f32 sCameraFovyRadian = 0.2617993950843811f; }

// ---------------------------------------------------------------------------------------------
// TwoLegsCoin
// ---------------------------------------------------------------------------------------------

TwoLegsCoin::TwoLegsCoin(LiveActor* pHost) {
    mHost = pHost;
    MR::declareCoin(pHost, 3);
}

void TwoLegsCoin::appear(const TVec3f& rPos) {
    MR::appearCoinPop(mHost, rPos, 1);
}

// ---------------------------------------------------------------------------------------------
// TwoLegsJointCtrl
// ---------------------------------------------------------------------------------------------

TwoLegsJointCtrl::TwoLegsJointCtrl(LiveActor* pHost) {
    mSpin = false;
    mJointL = nullptr;
    mJointR = nullptr;
    mTrack = false;
    mReturn = false;
    mCtrl = nullptr;
    mDir.set(0.0f, 1.0f, 0.0f);
    mMtxR.identity();
    mMtxL.identity();
    MR::initJointTransform(pHost);
    mJointL = MR::getJointTransform(pHost, "BoltL");
    mJointR = MR::getJointTransform(pHost, "BoltR");
    mJointL->mPostTransform = mMtxR.toMtxPtr();
    mJointR->mPostTransform = mMtxL.toMtxPtr();
    mCtrl = MR::createJointDelegator<TwoLegs>(static_cast<TwoLegs*>(pHost), pHost, nullptr, &TwoLegs::calcJointCallback, "EyeBall");
}

void TwoLegsJointCtrl::update() {
    if (mSpin) {
        MR::rotateMtxLocalYDegree(mMtxR.toMtxPtr(), sSpinDegree);
        MR::rotateMtxLocalYDegree(mMtxL.toMtxPtr(), -sSpinDegree);
    }
}

namespace { const f32 sSpinDegree = 2.0f; }

void TwoLegsJointCtrl::registerCallBack() {
    mCtrl->registerCallBack();
}

bool TwoLegsJointCtrl::calcJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    TVec3f pos;
    pMtx->getTrans(pos);
    TVec3f up;
    pMtx->getYDir(up);
    TVec3f dir;
    TVec3f front;
    if (mTrack) {
        MR::calcDirectionVec(&dir, pos, *MR::getPlayerCenterPos());
        TVec3f negUp;
        negUp.negate(up);
        f32 angle = MR::diffAngleSigned(dir, negUp, TVec3f(0.0f, 0.0f, 1.0f));
        f32 clamped = angle;
        clamped *= 1.5f;
        if (1.5707963705062866f < angle + clamped) {
            clamped = 1.5707963705062866f - angle;
        } else if (angle + clamped < -1.5707963705062866f) {
            clamped = -1.5707963705062866f - angle;
        }
        MR::rotateVecRadian(&dir, TVec3f(0.0f, 0.0f, 1.0f), clamped);
        MR::turnVecToVecDegree(&front, TVec3f(0.0f, 0.0f, 1.0f), dir, 10.0f, up);
    } else {
        pMtx->getZDir(front);
    }
    if (mReturn) {
        f32 deg = mTrack ? 0.3f : 5.0f;
        f32 rest = MR::turnVecToVecDegree(&front, mDir, front, deg, up);
        if (MR::isNearZero(rest, 0.001f)) {
            mReturn = false;
        }
    }
    mDir.x = front.x;
    mDir.y = front.y;
    mDir.z = front.z;
    MR::makeMtxFrontUp(pMtx, front, up);
    pMtx->mMtx[0][3] = pos.x;
    pMtx->mMtx[1][3] = pos.y;
    pMtx->mMtx[2][3] = pos.z;
    return true;
}

// ---------------------------------------------------------------------------------------------
// TwoLegsBodyDir
// ---------------------------------------------------------------------------------------------

TwoLegsBodyDir::TwoLegsBodyDir() {
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mFront.x = 0.0f;
    mFront.y = 0.0f;
    mFront.z = -1.0f;
}

f32 TwoLegsBodyDir::getRadius() const {
    return 500.0f;
}

// ---------------------------------------------------------------------------------------------
// TwoLegsReflectCtrl and its nerves
// ---------------------------------------------------------------------------------------------

// nerve classes; sinit order Wait, Reflect, Return (vtables 0x8067B350, 0x8067B340, 0x8067B330)
namespace NrvTwoLegsReflectCtrl {
    class TwoLegsReflectCtrlNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static TwoLegsReflectCtrlNrvWait sInstance;
    };
    class TwoLegsReflectCtrlNrvReflect : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static TwoLegsReflectCtrlNrvReflect sInstance;
    };
    class TwoLegsReflectCtrlNrvReturn : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static TwoLegsReflectCtrlNrvReturn sInstance;
    };
};  // namespace NrvTwoLegsReflectCtrl

TwoLegsReflectCtrl::TwoLegsReflectCtrl(LiveActor* pHost)
    : NerveExecutor("\x93\xf1\x8b\x72\x83\x7b\x83\x58\x92\x65\x82\xab\x95\xd4\x82\xb5\x90\xa7\x8c\xe4") {
    mHost = pHost;
    mJointCtrl = nullptr;
    mJointName = nullptr;
    mDir.set(0.0f, 1.0f, 0.0f);
    mJointCtrl = new ActorJointCtrl(mHost);
    initNerve(&NrvTwoLegsReflectCtrl::TwoLegsReflectCtrlNrvWait::sInstance, 0);
}

void TwoLegsReflectCtrl::update() {
    mJointCtrl->startUpdate();
    updateNerve();
    mJointCtrl->endUpdate();
}

void TwoLegsReflectCtrl::setCallBack() {
    mJointCtrl->setCallBackFunction();
}

void TwoLegsReflectCtrl::start(bool isLeft) {
    if (isLeft) {
        mJointName = "FootL";
    } else {
        mJointName = "FootR";
    }
    TVec3f jointPos;
    MR::copyJointPos(mHost, mJointName, &jointPos);
    MR::calcDirectionVec(&mDir, static_cast< TwoLegs* >(mHost)->getBodyDir()->mPos, jointPos);
    f32 degree;
    if (isLeft) {
        degree = 10.0f;
    } else {
        degree = -10.0f;
    }
    TVec3f axis;
    axis.x = 0.0f;
    axis.y = 0.0f;
    axis.z = 1.0f;
    MR::rotateVecDegree(&mDir, axis, degree);
    setNerve(&NrvTwoLegsReflectCtrl::TwoLegsReflectCtrlNrvReflect::sInstance);
}

// NONMATCHING: saved registers/load order and inline paired-single addition emitted as a PSVECAdd call
void TwoLegsReflectCtrl::setIK(f32 rate) {
    TVec3f jointPos;
    MR::copyJointPos(mHost, mJointName, &jointPos);
    TVec3f scaled(mDir);
    scaled.x *= 100.0f;
    scaled.y *= 100.0f;
    scaled.z *= 100.0f;
    TVec3f pos(jointPos);
    pos += scaled;
    mJointCtrl->setIKEndPosition(mJointName, pos, rate);
}

TwoLegsReflectCtrl::~TwoLegsReflectCtrl() {}

#ifndef GALAXY_NATIVE
namespace NrvTwoLegsReflectCtrl {
#endif
    // the exe bodies are inlined into the nerve execute stubs below (the target has no standalone exeX symbols)

    // Wait: releases the IK control on the first step
    inline void TwoLegsReflectCtrl::exeWait() {
        if (MR::isFirstStep(this)) {
            mJointCtrl->endIKCtrlAll();
        }
    }

    // Reflect: eases the IK out, then hands over to Return
    inline void TwoLegsReflectCtrl::exeReflect() {
        setIK(MR::calcNerveEaseOutRate(this, 5));
        MR::setNerveAtStep(this, &NrvTwoLegsReflectCtrl::TwoLegsReflectCtrlNrvReturn::sInstance, 5);
    }

    // Return: eases the IK back in, then hands over to Wait
    inline void TwoLegsReflectCtrl::exeReturn() {
        setIK(1.0f - MR::calcNerveEaseInRate(this, 10));
        MR::setNerveAtStep(this, &NrvTwoLegsReflectCtrl::TwoLegsReflectCtrlNrvWait::sInstance, 10);
    }

#ifdef GALAXY_NATIVE
namespace NrvTwoLegsReflectCtrl {
#endif
    TwoLegsReflectCtrlNrvWait TwoLegsReflectCtrlNrvWait::sInstance ATTRIBUTE_WEAK;
    TwoLegsReflectCtrlNrvReflect TwoLegsReflectCtrlNrvReflect::sInstance ATTRIBUTE_WEAK;
    TwoLegsReflectCtrlNrvReturn TwoLegsReflectCtrlNrvReturn::sInstance ATTRIBUTE_WEAK;

    void TwoLegsReflectCtrlNrvWait::execute(Spine* pSpine) const {
        reinterpret_cast< TwoLegsReflectCtrl* >(pSpine->mExecutor)->exeWait();
    }
    void TwoLegsReflectCtrlNrvReflect::execute(Spine* pSpine) const {
        reinterpret_cast< TwoLegsReflectCtrl* >(pSpine->mExecutor)->exeReflect();
    }
    void TwoLegsReflectCtrlNrvReturn::execute(Spine* pSpine) const {
        reinterpret_cast< TwoLegsReflectCtrl* >(pSpine->mExecutor)->exeReturn();
    }
};  // namespace NrvTwoLegsReflectCtrl
