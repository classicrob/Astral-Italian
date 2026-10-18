#pragma auto_inline off
#include "Game/Boss/TwoLegs.hpp"
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
    f32 fov;
    TVec3f center;
    calcFov(&fov, &center);
    mFovScale = fov;
    MR::startActorCameraProgrammable(mHost, -1);
}

void TwoLegsCamera::keepAlive() {
    if (mActive && !MR::isEventCameraActive()) {
        MR::startActorCameraProgrammable(mHost, 120);
    }
}

void TwoLegsCamera::end(bool) {
    mActive = false;
    MR::endActorCameraProgrammable(mHost, -1, true);
}

// NONMATCHING: the TVec3 copy-assign / add calls are out of line here, the target inlines the copy (psq) and emits add as its own weak fn
// (0, 0, 1) seed vector for makeMtxUpFront (SB4E01 .data lbl_806478D8)
extern "C" TVec3f lbl_806478D8;

void TwoLegsCamera::update() {
    if (mActive) {
        f32 fov;
        TVec3f center;
        calcFov(&fov, &center);
        fov = MR::lerp(mFovScale, fov, 0.2f);
        JMAVECLerp(&mTarget, &center, &center, 0.2f);
        TVec3f up;
        up = lbl_806478D8;
        TVec3f front;
        MR::calcDirectionVec(&front, *MR::getPlayerCenterPos(), center);
        TPos3f mtx;
        MR::makeMtxUpFront(&mtx, up, front);
        TVec3f right;
        right.set(mtx.mMtx[0][0], mtx.mMtx[1][0], mtx.mMtx[2][0]);
        MR::rotateVecDegree(&up, right, -20.0f);
        TVec3f scaled;
        scaled = up;
        scaled.x = up.x * fov;
        scaled.y = up.y * fov;
        scaled.z = up.z * fov;
        TVec3f eye;
        eye.add(center, scaled);
        MR::setProgrammableCameraParamFovy(mHost, 15.0f);
        TVec3f upVec(0.0f, 1.0f, 0.0f);
        MR::setProgrammableCameraParam(mHost, center, eye, upVec);
        mNumTargets = 0;
        mFovScale = fov;
        mTarget.set(center);
    }
}

void TwoLegsCamera::addTarget(LiveActor* pTarget) {
    if (mActive) {
        u32 num = mNumTargets;
        if (num < 2) {
            mTargets[num] = pTarget;
            mNumTargets = num + 1;
        }
    }
}

void TwoLegsCamera::reset() {
    mActive = true;
    mNumTargets = 0;
}

// NONMATCHING: the target emits TVec3::add as an out-of-line weak fn (fn_80105F30-area, 0x24) and calls it; ours inlines add/sub
void TwoLegsCamera::calcBox(TVec3f* pBox) {
    TVec3f center;
    center.set(*MR::getPlayerCenterPos());

    TVec3f root;
    static_cast< TwoLegs* >(mHost)->calcAllRootPosOnPlane(&root);

    TVec3f half;
    half.setAll< f32 >(350.0f);
    pBox->sub(center, half);
    TVec3f* pMax = reinterpret_cast< TVec3f* >(reinterpret_cast< u8* >(pBox) + 0xC);
    pMax->add(center, half);

    TwoLegsSphere sphere;
    sphere.mPos.set(root);
    sphere.mRadius = 600.0f;
    expandBox(pBox, &sphere);

    for (s32 i = 0; i < mNumTargets; i++) {
        sphere.mPos.set(*reinterpret_cast< TVec3f* >(reinterpret_cast< u8* >(mTargets[i]) + 0x14));
        sphere.mRadius = 250.0f;
        expandBox(pBox, &sphere);
    }
}

namespace {
    // grows the box (min at +0, max at +0xC) so that it contains the sphere
    void expandBox(TVec3f* pBox, const TwoLegsSphere* pSphere) {
        TVec3f radius;
        radius.setAll< f32 >(pSphere->mRadius);
        TVec3f lo;
        lo.sub(pSphere->mPos, radius);
        TVec3f hi;
        hi.add(pSphere->mPos, radius);
        TVec3f* pMax = reinterpret_cast< TVec3f* >(reinterpret_cast< u8* >(pBox) + 0xC);
        if (lo.x < pBox->x) {
            pBox->x = lo.x;
        }
        if (lo.y < pBox->y) {
            pBox->y = lo.y;
        }
        if (lo.z < pBox->z) {
            pBox->z = lo.z;
        }
        if (hi.x > pMax->x) {
            pMax->x = hi.x;
        }
        if (hi.y > pMax->y) {
            pMax->y = hi.y;
        }
        if (hi.z > pMax->z) {
            pMax->z = hi.z;
        }
    }
};  // namespace

// NONMATCHING: the fovy/centre computation uses the JMath sin/cos table inline; approximated via sin/cos
void TwoLegsCamera::calcFov(f32* pFov, TVec3f* pCenter) {
    TVec3f box[2];
    calcBox(&box[0]);
    f32 theta = 0.5f * 0.2617993950843811f;
    f32 tanHalf = JMath::sSinCosTable.sinRadian(theta) / JMath::sSinCosTable.cosRadian(theta);
    f32 dist = (0.5f * static_cast< f32 >(MR::getScreenHeight())) / tanHalf;
    f32 fovX = (dist * (box[1].x - box[0].x)) / 608.0f;
    f32 fovY = (dist * (box[1].y - box[0].y)) / static_cast< f32 >(MR::getScreenHeight());
    if (fovX < fovY) {
        fovX = fovY;
    }
    *pFov = fovX;
    JMAVECLerp(&box[1], &box[0], pCenter, 0.5f);
}

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
    // XjointTransform+0x68: user matrix pointer read by the joint callbacks
    *reinterpret_cast< TPos3f** >(reinterpret_cast< u8* >(mJointL) + 0x68) = &mMtxR;
    *reinterpret_cast< TPos3f** >(reinterpret_cast< u8* >(mJointR) + 0x68) = &mMtxL;
    mCtrl = new JointControlDelegator< TwoLegs >(static_cast< TwoLegs* >(pHost), nullptr, &TwoLegs::calcJointCallback);
    MR::setJointControllerParam(mCtrl, pHost, "EyeBall");
}

void TwoLegsJointCtrl::update() {
    if (mSpin) {
        f32 degree = 2.0f;
        MR::rotateMtxLocalYDegree(mMtxR.toMtxPtr(), degree);
        MR::rotateMtxLocalYDegree(mMtxL.toMtxPtr(), -degree);
    }
}

void TwoLegsJointCtrl::registerCallBack() {
    mCtrl->registerCallBack();
}

// NONMATCHING: mDir.set(front) is an out-of-line call here but an inline copy in the target (TVec3 assign codegen)
bool TwoLegsJointCtrl::calcJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    TVec3f pos;
    pos.set(pMtx->mMtx[0][3], pMtx->mMtx[1][3], pMtx->mMtx[2][3]);
    TVec3f up;
    up.set(pMtx->mMtx[0][1], pMtx->mMtx[1][1], pMtx->mMtx[2][1]);
    TVec3f front;
    if (mTrack) {
        TVec3f dir;
        MR::calcDirectionVec(&dir, pos, *MR::getPlayerCenterPos());
        TVec3f axis(0.0f, 0.0f, 1.0f);
        TVec3f negUp;
        negUp.x = -up.x;
        negUp.y = -up.y;
        negUp.z = -up.z;
        f32 angle = MR::diffAngleSigned(dir, negUp, axis);
        f32 clamped = angle * 1.5f;
        if (1.5707963705062866f < angle + clamped) {
            clamped = 1.5707963705062866f - angle;
        } else if (angle + clamped < -1.5707963705062866f) {
            clamped = -1.5707963705062866f - angle;
        }
        MR::rotateVecRadian(&dir, axis, clamped);
        MR::turnVecToVecDegree(&front, axis, dir, 10.0f, up);
    } else {
        front.set(pMtx->mMtx[0][2], pMtx->mMtx[1][2], pMtx->mMtx[2][2]);
    }
    if (mReturn) {
        f32 deg = mTrack ? 0.3f : 5.0f;
        f32 rest = MR::turnVecToVecDegree(&front, mDir, front, deg, up);
        if (MR::isNearZero(rest, 0.001f)) {
            mReturn = false;
        }
    }
    mDir.set(front);
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
    mPos.set(0.0f, 0.0f, 0.0f);
    mFront.set(0.0f, 0.0f, -1.0f);
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
    TVec3f axis(0.0f, 0.0f, 1.0f);
    MR::rotateVecDegree(&mDir, axis, degree);
    setNerve(&NrvTwoLegsReflectCtrl::TwoLegsReflectCtrlNrvReflect::sInstance);
}

// NONMATCHING: pos = joint pos + scaled dir; copy-ctor and scale order differ from target
void TwoLegsReflectCtrl::setIK(f32 rate) {
    TVec3f jointPos;
    MR::copyJointPos(mHost, mJointName, &jointPos);
    TVec3f scaled(mDir);
    scaled.x *= 100.0f;
    scaled.y *= 100.0f;
    scaled.z *= 100.0f;
    TVec3f pos(jointPos);
    pos.add(scaled);
    mJointCtrl->setIKEndPosition(mJointName, pos, rate);
}

TwoLegsReflectCtrl::~TwoLegsReflectCtrl() {}

namespace NrvTwoLegsReflectCtrl {
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
