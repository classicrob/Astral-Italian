#pragma auto_inline off
#include "Game/Enemy/Jugem.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"

namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, MtxPtr, const TVec3f&);
};  // namespace MR

class Togezo : public LiveActor {
public:
    Togezo(const char*);
    u8 _90[0x48];
};

Jugem::Jugem(const char* pName) : LiveActor(pName) {
    mJointCtrlA = nullptr;
    mJointCtrlB = nullptr;
    mAnimScaleCtrl = nullptr;
    mBindStarPointer = nullptr;
    mCloud = nullptr;
    mCurrentTogezo = nullptr;
    mChildObjs = nullptr;
    mChildFlags = nullptr;
    mFrontVec.set(0.0f, 0.0f, 1.0f);
    mBasePos.set(0.0f, 0.0f, 0.0f);
    _138 = 0.0f;
    mChildNum = 0;
    mChildNumPlusOne = 0;
    mChildIndex = 0;
    mChildCount = 0;
    mWaitTime = 900;
    _154 = 0;
    _164.set(0.0f, 0.0f, 0.0f);
    _170 = 0;
    _174 = 0;
    _178 = 0.0f;
    mTimer = 0;
    mIsActive = 0;
    mIsHitDone = 0;
    mBaseMtx.identity();
    mSubMtx.identity();
}

// Global TVec3 constants (SB4E01 .bss 0x80729010..0x80729040, filled by the Jugem2 sinit).
extern TVec3f gJugemVecA;
extern TVec3f gJugemVecB;
extern TVec3f gJugemVecD;
extern TVec3f gJugemVecE;

// Nerve instances (sbss, defined in Jugem2.cpp). Referenced by their SB4E01 symbols so that
// `li rN, lbl_807D26xx@sda21` matches the target directly.
extern "C" u32 lbl_807D26B8;  // Wait
extern "C" u32 lbl_807D26BC;  // Ready
extern "C" u32 lbl_807D26C0;  // Aim
extern "C" u32 lbl_807D26C4;  // Approach
extern "C" u32 lbl_807D26C8;  // Move
extern "C" u32 lbl_807D26CC;  // MoveUp
extern "C" u32 lbl_807D26D0;  // ThrowSign
extern "C" u32 lbl_807D26D4;  // Throw
extern "C" u32 lbl_807D26D8;  // Chase
extern "C" u32 lbl_807D26DC;  // HitReaction
extern "C" u32 lbl_807D26E0;  // Fall
extern "C" u32 lbl_807D26E4;  // Drop
extern "C" u32 lbl_807D26E8;  // Land
extern "C" u32 lbl_807D26EC;  // Hide
extern "C" u32 lbl_807D26F0;  // Reset
extern "C" u32 lbl_807D26F4;  // None
extern "C" u32 lbl_807D26F8;  // BlowDown
extern "C" u32 lbl_807D26FC;  // FallDown
extern "C" u32 lbl_807D2700;  // Emerge
extern "C" u32 lbl_807D2704;  // Die
extern "C" u32 lbl_807D2708;  // FallLand
extern "C" u32 lbl_807D270C;  // FallRecovery
extern "C" u32 lbl_807D2710;  // SupportFreeze

#define JUGEM_NERVE(sym) reinterpret_cast< const Nerve* >(&sym)

namespace NameObjFactory {
    NameObj* initChildObj(const JMapInfoIter&, int);
};  // namespace NameObjFactory

static JointController* createJointCtrl(Jugem* pHost, const char* pJointName, bool (Jugem::*calcFunc)(TPos3f*, const JointControllerInfo&)) {
    return MR::createJointDelegator< Jugem >(pHost, pHost, calcFunc, nullptr, pJointName);
}

void Jugem::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Jugem", false);
    mCloud = new JugemCloud(this, "\x89\x5F\x83\x82\x83\x66\x83\x8B");
    mCloud->mCheckHiddenHost = true;
    mCloud->initFixedPosition(MR::getJointMtx(this, "Root"), TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    mCloud->initWithoutIter();
    PSMTXCopy(getBaseMtx(), mBaseMtx.toMtxPtr());
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, mBaseMtx.toMtxPtr(), TVec3f(0.0f));
    mYoshiLockOn2 = MR::createYoshiLockOnTarget(mCloud, nullptr, TVec3f(0.0f));
    MR::getJMapInfoArg0NoInit(rIter, &mWaitTime);
    mChildNum = MR::getChildObjNum(rIter);
    mChildNumPlusOne = mChildNum + 1;
    mChildObjs = new NameObj*[mChildNum];
    mChildPoses = new TVec3f[mChildNum];
    mChildFlags = new s32[mChildNum];
    mChildren = new LiveActor*[mChildNumPlusOne];
    for (s32 i = 0; i < mChildNum; i++) {
        mChildFlags[i] = 0;
        NameObj* pObj = NameObjFactory::initChildObj(rIter, i);
        mChildren[i] = static_cast< LiveActor* >(pObj);
        mChildObjs[i] = pObj;
        static_cast< LiveActor* >(mChildObjs[i])->makeActorAppeared();
        mChildPoses[i] = static_cast< LiveActor* >(mChildObjs[i])->mPosition;
    }
    Togezo* pTogezo = new Togezo("\x83\x67\x83\x51\x83\x5B\x81\x5B");
    mChildren[mChildNum] = pTogezo;
#pragma dont_inline on
    mChildren[mChildNum]->mPosition.set(mPosition);
#pragma dont_inline reset
    mChildren[mChildNum]->initWithoutIter();
    mChildren[mChildNum]->makeActorDead();
    mChildCount = mChildNum;
    initBinder();
    MR::moveCoordAndTransToNearestRailPos(this);
    MR::calcFrontVec(&mFrontVec, this);
    initNerve(JUGEM_NERVE(lbl_807D26B8), 1);
    mJointCtrlA = createJointCtrl(this, "AllRoot", &Jugem::calcRootJoint);
    mJointCtrlB = createJointCtrl(this, "WristR", &Jugem::calcWristJoint);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mAnimScaleCtrl->setParamTight();
    mBindStarPointer = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mBindStarPointer, JUGEM_NERVE(lbl_807D2710), "SupportFreeze");
    MR::useStageSwitchSyncAppear(this, rIter);
}

void Jugem::initBinder() {
    mBasePos = gJugemVecB;
    _138 = 0.0f;
    MR::setBinderOffsetVec(this, &mBasePos, false);
    MR::setBinderRadius(this, _138);
}

bool Jugem::calcRootJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    MR::addTransMtxLocal(pMtx->toMtxPtr(), _164);
    return true;
}

bool Jugem::calcWristJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    f32 mtx[3][4];
    PSMTXIdentity(mtx);
    MR::makeMtxTR(mtx, gJugemVecD, gJugemVecE);
    PSMTXConcat(pMtx->toMtxPtr(), mtx, mSubMtx.toMtxPtr());
    return false;
}

void Jugem::makeActorAppeared() {
    setNerve(JUGEM_NERVE(lbl_807D26B8));
    LiveActor::makeActorAppeared();
    mChildIndex = 0;
    MR::invalidateClipping(this);
    mCloud->makeActorAppeared();
    mTimer = 0;
}

void Jugem::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFrontVec);
    MR::calcFrontVec(&mFrontVec, this);
    MR::updateBaseScale(this, mAnimScaleCtrl);
    if (!MR::isDead(this)) {
        mJointCtrlA->registerCallBack();
        mJointCtrlB->registerCallBack();
    }
}

extern "C" void fn_800345E0(LiveActor*, const char*);

JugemCloud::JugemCloud(Jugem* pHost, const char* pName) : PartsModel(pHost, pName, "JugemCloud", nullptr, 19, false) {
    mJugem = pHost;
    mIsAppear = false;
}

void JugemCloud::makeActorAppeared() {
    PartsModel::makeActorAppeared();
    mIsAppear = true;
}

void JugemCloud::appear() {
    LiveActor::appear();
    mCalcOwnMatrix = true;
    MR::setScale(this, 1.0f);
    calcAnim();
    fn_800345E0(this, "\x8F\x6F\x8C\xBB");
}

void JugemCloud::kill() {
    LiveActor::kill();
}

void JugemCloud::control() {
    f32 speed = PSVECMag(&mHost->mVelocity);
    MR::startActionSound(this, "Float", static_cast< s32 >(100.0f * speed), -1, -1);
    if (!mIsAppear) {
        if (MR::isActionEnd(this)) {
            kill();
        }
    }
}

bool JugemCloud::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return mJugem->receiveOtherMsg(msg, pSender, pReceiver);
}

void JugemCloud::disappear() {
    mIsAppear = false;
    mCalcOwnMatrix = false;
    MR::startAction(this, "Disappear");
}

void JugemCloud::tryDisappear() {
    if (MR::isDead(this))
        return;
    if (!mIsAppear)
        return;
    disappear();
}
