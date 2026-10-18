#include "Game/Boss/Sandy.hpp"
// SMG2: MR::appearCoinPop returns bool (ObjUtil.hpp declares it void), declare the real one here.
#define appearCoinPop appearCoinPopUnused
#include "Game/Util.hpp"
#undef appearCoinPop
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util/FixedPosition.hpp"
#include <cstdio>

namespace NameObjFactory {
    NameObj* initChildObj(const JMapInfoIter&, int);
};  // namespace NameObjFactory

// Call a pointer-to-member on every element of a range (std::for_each like).
template < typename It, typename F >
F ForEachPtmf(It first, It last, F f) {
    for (; first != last; ++first) {
        ((*first)->*f)();
    }
    return f;
}

// Single-use inline copy of JGeometry::mulInternal (CW size mode only inlines functions with one call site; the target
// had each of these in its own translation unit).
static inline void mulScaleSandy(register const f32* a, register const f32* b, register f32* d) {
    register f32 xy1, xy2, res;
    __asm {
        psq_l xy1, 0(a), 0, 0
        psq_l xy2, 0(b), 0, 0
        ps_mul res, xy1, xy2
        psq_st res, 0(d), 0, 0
    }
    d[2] = a[2] * b[2];
}

namespace NrvSandy {
    class SandyNrvDemoStart : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyNrvDemoStart sInstance;
    };
    class SandyNrvDemoPowerUp : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyNrvDemoPowerUp sInstance;
    };
    class SandyNrvDemoDown : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyNrvDemoDown sInstance;
    };
    class SandyNrvPhase1 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyNrvPhase1 sInstance;
    };
    class SandyNrvPhase2 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyNrvPhase2 sInstance;
    };
    class SandyNrvPhase3 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyNrvPhase3 sInstance;
    };

    SandyNrvDemoStart SandyNrvDemoStart::sInstance ATTRIBUTE_WEAK;
    SandyNrvDemoPowerUp SandyNrvDemoPowerUp::sInstance ATTRIBUTE_WEAK;
    SandyNrvDemoDown SandyNrvDemoDown::sInstance ATTRIBUTE_WEAK;
    SandyNrvPhase1 SandyNrvPhase1::sInstance ATTRIBUTE_WEAK;
    SandyNrvPhase2 SandyNrvPhase2::sInstance ATTRIBUTE_WEAK;
    SandyNrvPhase3 SandyNrvPhase3::sInstance ATTRIBUTE_WEAK;

    inline void SandyNrvDemoStart::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< Sandy* >(pSpine->mExecutor), &SandyNrvPhase1::sInstance);
    }
    inline void SandyNrvDemoPowerUp::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< Sandy* >(pSpine->mExecutor), &SandyNrvPhase3::sInstance);
    }
    inline void SandyNrvDemoDown::execute(Spine* pSpine) const {
        Sandy* pActor = reinterpret_cast< Sandy* >(pSpine->mExecutor);
        if (MR::updateActorState(pActor)) {
            pActor->kill();
        }
    }
    inline void SandyNrvPhase1::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< Sandy* >(pSpine->mExecutor), &SandyNrvPhase2::sInstance);
    }
    inline void SandyNrvPhase2::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< Sandy* >(pSpine->mExecutor), &SandyNrvDemoPowerUp::sInstance);
    }
    inline void SandyNrvPhase3::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< Sandy* >(pSpine->mExecutor), &SandyNrvDemoDown::sInstance);
    }
};  // namespace NrvSandy

static TVec3f sMiniPosL(800.0f, 125.0f, 300.0f);
static TVec3f sMiniRotL(0.0f, -15.0f, 0.0f);
static TVec3f sMiniPosR(-800.0f, 125.0f, 300.0f);
static TVec3f sMiniRotR(0.0f, 15.0f, 0.0f);
static TVec3f sHandPos(231.66f, 35.08f, 319.28f);
static TVec3f sHandRot(90.0f, -90.0f, 180.0f);

// 800FAC80
Sandy::Sandy(const char* pName)
    : LiveActor(pName), mPhase1(nullptr), mPhase2(nullptr), mPhase3(nullptr), mHolder(nullptr), mBlur(nullptr), mJointCtrl(nullptr),
      mScaleController(nullptr), mScaleParam(nullptr), mCameraInfo(nullptr), mFollowPos(TVec3f(0.0f)) {
    mQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
    mTargetQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
    mMtx.identity();
}

// 800FAD40
void Sandy::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    MR::makeQuatRotateDegree(&mTargetQuat, mRotation);
    mQuat.x = mTargetQuat.x;
    mQuat.y = mTargetQuat.y;
    mQuat.z = mTargetQuat.z;
    mQuat.w = mTargetQuat.w;
    MR::setSensorFollowPos(this, "Body", &mFollowPos);
    mHolder = new SandyBallHolder(this);
    mHands[0] = createHand(0, sHandPos, sHandRot);
    mHands[1] = createHand(1, sHandPos, sHandRot);
    mMinis[0] = createMini(0, sMiniPosL, sMiniRotL);
    mMinis[1] = createMini(1, sMiniPosR, sMiniRotR);
    mJointCtrl = new ActorJointCtrl(this);
    mScaleParam = createScaleParam();
    mScaleController = new AnimScaleController(mScaleParam);
    mBlur = new SandyBlur("\203T\203\223\203f\203B\201[\227p\211\346\226\312\203u\203\211\201[\216\300\215s");
    initNerve(&NrvSandy::SandyNrvDemoStart::sInstance, 7);
    mPhase1 = new SandyStatePhase1(this);
    mPhase2 = new SandyStatePhase2(this);
    mPhase3 = new SandyStatePhase3(this);
    MR::initActorState(this, new SandyStateDemoStart(this), &NrvSandy::SandyNrvDemoStart::sInstance, nullptr);
    MR::initActorState(this, new SandyStateDemoPowerUp(this), &NrvSandy::SandyNrvDemoPowerUp::sInstance, nullptr);
    MR::initActorState(this, new SandyStateDemoDown(this), &NrvSandy::SandyNrvDemoDown::sInstance, nullptr);
    MR::initActorState(this, mPhase1, &NrvSandy::SandyNrvPhase1::sInstance, nullptr);
    MR::initActorState(this, mPhase2, &NrvSandy::SandyNrvPhase2::sInstance, nullptr);
    MR::initActorState(this, mPhase3, &NrvSandy::SandyNrvPhase3::sInstance, nullptr);
    mCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initAnimCamera(this, mCameraInfo, "DemoSandyStart");
    MR::initAnimCamera(this, mCameraInfo, "DemoSandyPowerUp");
    MR::initAnimCamera(this, mCameraInfo, "DemoSandyDown");
    MR::declareStarPiece(this, 16);
    MR::declarePowerStar(this);
    mChildren.init(MR::getChildObjNum(rIter));
    for (LiveActor** p = mChildren.begin(); p != mChildren.end(); p++) {
        const char* pName = nullptr;
        MR::getChildObjName(&pName, rIter, p - mChildren.begin());
        *p = reinterpret_cast< LiveActor* >(NameObjFactory::initChildObj(rIter, p - mChildren.begin()));
        (*p)->makeActorDead();
    }
    MR::useStageSwitchSyncAppear(this, rIter);
    MR::hideModelAndOnCalcAnim(this);
    MR::hideModelAndOnCalcAnim(mHands[0]);
    MR::hideModelAndOnCalcAnim(mHands[1]);
}

// 800FB0E0
// NONMATCHING: PTMF for_each loops: the target keeps two stack copies of the member-function pointer, CW elides the second one here.
void Sandy::movement() {
    LiveActor::movement();
    void (NameObj::*pHandMove)() = &NameObj::movement;
    ForEachPtmf(mHands, mHands + 2, pHandMove);
    void (NameObj::*pMiniMove)() = &NameObj::movement;
    ForEachPtmf(mMinis, mMinis + 2, pMiniMove);
    mBlur->updateNerve();
}

// 800FB1C0
// NONMATCHING: PTMF for_each loop (two stack copies of the member-function pointer in the target).
void Sandy::appear() {
    LiveActor::appear();
    void (LiveActor::*pAppear)() = &LiveActor::appear;
    ForEachPtmf(mHands, mHands + 2, pAppear);
    setNerve(&NrvSandy::SandyNrvDemoStart::sInstance);
}

// 800FB250
void Sandy::kill() {
    MR::appearPowerStarWithoutDemo(this);
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
    LiveActor::kill();
}

// 800FB2A0
// NONMATCHING: PTMF for_each call (two stack copies of the member-function pointer in the target).
void Sandy::killAll() {
    void (LiveActor::*pKill)() = &LiveActor::kill;
    ForEachPtmf(mChildren.begin(), mChildren.end(), pKill);
}

// 800FB390
// NONMATCHING: PTMF for_each call (two stack copies of the member-function pointer in the target).
void Sandy::appearAll() {
    void (LiveActor::*pAppear)() = &LiveActor::appear;
    ForEachPtmf(mChildren.begin(), mChildren.end(), pAppear);
}

SandyHand* Sandy::getLeftHand() const {
    return mHands[0];
}

SandyHand* Sandy::getRightHand() const {
    return mHands[1];
}

SandyMini* Sandy::getLeftMini() const {
    return mMinis[0];
}

SandyMini* Sandy::getRightMini() const {
    return mMinis[1];
}

// 800FB440
void Sandy::updateTarget() {
    turnToPlayer(&mTargetQuat, this, 1.2f);
    mFollowPos.x = mPosition.x;
    mFollowPos.y = mPosition.y;
    mFollowPos.z = mPosition.z;
    if (!MR::isDead(mMinis[0])) {
        mMinis[0]->calcPosFromFixedPos();
    }
    if (!MR::isDead(mMinis[1])) {
        mMinis[1]->calcPosFromFixedPos();
    }
}

// 800FB4C0
// NONMATCHING: TVec3f negate/+= expanded inline in the target (per-TU inline decision), out of line calls here.
void Sandy::appearStarPieceOnDamage() {
    if (!isNerve(&NrvSandy::SandyNrvPhase3::sInstance)) {
        TVec3f pos(mPosition);
        TVec3f down = -mGravity;
        TVec3f up(down);
        up.x *= 300.0f;
        up.y *= 300.0f;
        up.z *= 300.0f;
        pos += up;
        MR::appearStarPiece(this, pos, 8, 45.0f, 50.0f, false);
    }
}

// 800FB590
void Sandy::control() {
    mJointCtrl->update();
    mScaleController->updateNerve();
}

// 800FB5D0
void Sandy::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if ((MR::isEqualSensor(pSender, this, "Body") || MR::isEqualSensor(pSender, this, "Head")) && MR::isSensorPlayerOrRide(pReceiver)) {
        if (!MR::sendMsgEnemyAttackMaximum(pReceiver, pSender)) {
            mScaleController->startHitReaction();
            MR::startActionSound(this, "Touch", -1, -1, -1);
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

// 800FB690
void Sandy::calcAndSetBaseMtx() {
    mJointCtrl->setCallBackFunction();
    MR::setBaseTRMtx(this, mTargetQuat);
    TVec3f scale;
    mulScaleSandy(&mScaleController->_C.x, &mScale.x, &scale.x);
    MR::setBaseScale(this, scale);
}

// 800FB700
bool Sandy::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        mScaleController->startHitReaction();
        return true;
    }
    if (isNerve(&NrvSandy::SandyNrvPhase1::sInstance)) {
        return mPhase1->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&NrvSandy::SandyNrvPhase2::sInstance)) {
        return mPhase2->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&NrvSandy::SandyNrvPhase3::sInstance)) {
        return mPhase3->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    return false;
}

// 800FB810
Sandy::~Sandy() {
}
