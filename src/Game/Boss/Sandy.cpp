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

namespace MR {
    bool appearCoinPop(const NameObj*, const TVec3f&, s32);
};  // namespace MR

// Unnamed SMG2 helpers outside this unit.
// PLACEHOLDER(objects): unnamed target symbol fn_8003D2D0, declared extern "C" so the reloc name matches; resolve when fn_8003D2D0 gets its real name in symbols.txt
extern "C" void fn_8003D2D0(TQuat4f*, const TQuat4f*, const TVec3f&, f32);
// PLACEHOLDER(objects): unnamed target symbol fn_8009A780, declared extern "C" so the reloc name matches; resolve when fn_8009A780 gets its real name in symbols.txt
extern "C" void fn_8009A780(TPos3f*, const TQuat4f*, const TVec3f*);
// PLACEHOLDER(objects): unnamed target symbol fn_800DB560, declared extern "C" so the reloc name matches; resolve when fn_800DB560 gets its real name in symbols.txt
extern "C" void fn_800DB560(void*, s32);
// PLACEHOLDER(objects): unnamed target symbol fn_8001BA80, declared extern "C" so the reloc name matches; resolve when fn_8001BA80 gets its real name in symbols.txt
extern "C" void fn_8001BA80();

namespace MR {
    bool isGalaxyDarkCometAppearInCurrentStage();
};  // namespace MR

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

// Single-use inline copy of JGeometry::mulInternal (CW size mode only inlines functions with one call site; the target
// had each of these in its own translation unit).
static inline void mulScaleHand(register const f32* a, register const f32* b, register f32* d) {
    register f32 xy1, xy2, res;
    __asm {
        psq_l xy1, 0(a), 0, 0
        psq_l xy2, 0(b), 0, 0
        ps_mul res, xy1, xy2
        psq_st res, 0(d), 0, 0
    }
    d[2] = a[2] * b[2];
}

// Single-use inline copy of JGeometry::mulInternal (CW size mode only inlines functions with one call site; the target
// had each of these in its own translation unit).
static inline void mulScaleMini(register const f32* a, register const f32* b, register f32* d) {
    register f32 xy1, xy2, res;
    __asm {
        psq_l xy1, 0(a), 0, 0
        psq_l xy2, 0(b), 0, 0
        ps_mul res, xy1, xy2
        psq_st res, 0(d), 0, 0
    }
    d[2] = a[2] * b[2];
}

// Single-use copy of TRotation3::getEulerXYZ followed by the radian->degree scale (see mulScaleSandy).
static inline void calcEulerHand(const TPos3f& rMtx, TVec3f& rDest) {
    const f32 zx = rMtx.mMtx[2][0];
    if (zx - 1.0f >= -(f32)JGeometry::TUtil< f32 >::epsilon()) {
        rDest.set(JMAATan2(-rMtx.mMtx[0][1], rMtx.mMtx[1][1]), -HALF_PI, 0.0f);
    } else {
        f32 f1 = 1.0f;
        if (zx + f1 <= (f32)JGeometry::TUtil< f32 >::epsilon()) {
            rDest.set(JMAATan2(rMtx.mMtx[0][1], rMtx.mMtx[1][1]), HALF_PI, 0.0f);
        } else {
            rDest.x = JMAATan2(rMtx.mMtx[2][1], rMtx.mMtx[2][2]);
            rDest.z = JMAATan2(rMtx.mMtx[1][0], rMtx.mMtx[0][0]);
            rDest.y = JGeometry::TUtil< f32 >::asin(-rMtx.mMtx[2][0]);
        }
    }
    rDest = rDest * 57.29578f;
}

// Single-use copy of TRotation3::getEulerXYZ followed by the radian->degree scale (see mulScaleSandy).
static inline void calcEulerMini(const TPos3f& rMtx, TVec3f& rDest) {
    const f32 zx = rMtx.mMtx[2][0];
    if (zx - 1.0f >= -(f32)JGeometry::TUtil< f32 >::epsilon()) {
        rDest.set(JMAATan2(-rMtx.mMtx[0][1], rMtx.mMtx[1][1]), -HALF_PI, 0.0f);
    } else {
        f32 f1 = 1.0f;
        if (zx + f1 <= (f32)JGeometry::TUtil< f32 >::epsilon()) {
            rDest.set(JMAATan2(rMtx.mMtx[0][1], rMtx.mMtx[1][1]), HALF_PI, 0.0f);
        } else {
            rDest.x = JMAATan2(rMtx.mMtx[2][1], rMtx.mMtx[2][2]);
            rDest.z = JMAATan2(rMtx.mMtx[1][0], rMtx.mMtx[0][0]);
            rDest.y = JGeometry::TUtil< f32 >::asin(-rMtx.mMtx[2][0]);
        }
    }
    rDest = rDest * 57.29578f;
}

static AnimScaleParam* createScaleParam();

// Inline identity (the target stores the 12 elements directly, TMtx34::identity is out of line in our header).
static inline void setIdentityInline(TPos3f& rMtx) {
    rMtx.mMtx[0][3] = 0.0f;
    rMtx.mMtx[1][3] = 0.0f;
    rMtx.mMtx[2][3] = 0.0f;
    rMtx.mMtx[0][0] = 1.0f;
    rMtx.mMtx[0][1] = 0.0f;
    rMtx.mMtx[0][2] = 0.0f;
    rMtx.mMtx[1][0] = 0.0f;
    rMtx.mMtx[1][1] = 1.0f;
    rMtx.mMtx[1][2] = 0.0f;
    rMtx.mMtx[2][0] = 0.0f;
    rMtx.mMtx[2][1] = 0.0f;
    rMtx.mMtx[2][2] = 1.0f;
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

namespace NrvSandyBall {
    NEW_NERVE(SandyBallNrvWait, SandyBall, Wait);
    NEW_NERVE(SandyBallNrvFly, SandyBall, Fly);
    NEW_NERVE(SandyBallNrvBreak, SandyBall, Break);
    NEW_NERVE(SandyBallNrvLand, SandyBall, Land);
    NEW_NERVE(SandyBallNrvRoll, SandyBall, Roll);
    NEW_NERVE(SandyBallNrvExplosion, SandyBall, Explosion);
    NEW_NERVE(SandyBallNrvBindStarPointer, SandyBall, BindStarPointer);
};  // namespace NrvSandyBall

static const char* const sSandyBallNames[] = {"\203T\203{\203e\203\223", "\203S\201[\203\213\203h", "\203T\203\223\203h", "\224\232\222e", "\203T\203{\203e\203\223\226[", nullptr};
static TVec3f sBombTargetPos;

// 800FBA60
SandyBall::SandyBall(s32 type, const char* pName)
    : LiveActor(pName), mOwner(nullptr), mPath(new ParabolicPath()), mFlyFrame(0), mStep(0), mVel(TVec3f(0.0f)), mStartPos(TVec3f(0.0f)),
      mScaleRate(0.0f), mType(type), mCoinPopped(false), mSensorRadius(0.0f), mScaleController(nullptr), mBindStarPointer(nullptr),
      mPrevNerve(nullptr) {
    setName(sSandyBallNames[type]);
    mQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
}

// 800FBB60
void SandyBall::init(const JMapInfoIter& rIter) {
    const char* pActorName;
    if (mType == 2) {
        pActorName = "SandyBall";
    } else if (mType == 3) {
        pActorName = "SandyCactusBallBig";
    } else {
        pActorName = "SandyCactusBall";
    }
    MR::initActor(this, rIter, pActorName, false);
    initNerve(&NrvSandyBall::SandyBallNrvWait::sInstance, 1);
    mScaleController = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mBindStarPointer, &NrvSandyBall::SandyBallNrvBindStarPointer::sInstance, nullptr);
    MR::offBind(this);
    if (mType == 1) {
        MR::declareCoin(this, 1);
    }
    makeActorDead();
}

// 800FBC60
void SandyBall::appear() {
    LiveActor::appear();
    setBtpFrame();
    if (mType == 1) {
        MR::tryHitReaction(this, "\217o\214\273(\203S\201[\203\213\203h)");
    } else {
        MR::tryHitReactionAppear(this);
    }
    MR::invalidateShadow(this, nullptr);
    setNerve(&NrvSandyBall::SandyBallNrvWait::sInstance);
}

// 800FBCE0
void SandyBall::kill() {
    mOwner = nullptr;
    MR::offBind(this);
    MR::validateClipping(this);
    MR::invalidateHitSensors(this);
    mStep = 0;
    setNerve(&NrvSandyBall::SandyBallNrvWait::sInstance);
    LiveActor::kill();
}

// 800FBD50
// NONMATCHING: TVec3f operator+/negateInternal inline-vs-call decisions differ (target was a separate TU).
void SandyBall::calcTargetPos(TVec3f* pOut) {
    if (mType == 3) {
        SandyHand* pHand = static_cast< SandyHand* >(mOwner);
        Sandy* pSandy = pHand->mSandy;
        TVec3f groundPos;
        MR::getPlayerGroundPos(&groundPos);
        TVec3f diff = groundPos - pSandy->mPosition;
        if (PSVECMag(&diff) < 1700.0f) {
            diff.setLength(1700.0f);
        } else if (2500.0f < PSVECMag(&diff)) {
            diff.setLength(2500.0f);
        }
        groundPos.set(pSandy->mPosition + diff);
        TVec3f toGround = groundPos - mPosition;
        TVec3f hitPos(mPosition);
        TVec3f up(mGravity);
        up.scale(1000.0f);
        TVec3f start = (mPosition + toGround) - up;
        TVec3f dir(mGravity);
        dir.scale(2000.0f);
        if (MR::getFirstPolyOnLineToMap(&hitPos, nullptr, start, dir)) {
            toGround.set(hitPos - mPosition);
        }
        TVec3f handUp = -pHand->mGravity;
        if (pSandy->isMinisDownOrDead()) {
            TVec3f side(toGround);
            MR::vecKillElement(side, mGravity, &side);
            TVec3f cross;
            PSVECCrossProduct(&side, &handUp, &cross);
            if (!pHand->mIsRight) {
                cross = -cross;
            }
            cross.setLength(1.3f * MR::getSensorRadius(this, "Body"));
            pOut->set((mPosition + toGround) + cross);
        } else {
            f32 angle = 17.0f;
            if (pHand->mIsRight) {
                angle *= -1.0f;
            }
            MR::rotateVecDegree(&toGround, handUp, angle);
            pOut->set(toGround + mPosition);
        }
    } else {
        MR::getPlayerGroundPos(pOut);
    }
}

s32 SandyBall::getWaitSecond() const {
    return 30;
}

// 800FBFC0
bool SandyBall::tryShoot() {
    if (MR::isDead(this)) {
        return false;
    }
    if (isNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance)) {
        return false;
    }
    if (!isNerve(&NrvSandyBall::SandyBallNrvWait::sInstance)) {
        return false;
    }
    TVec3f target;
    calcTargetPos(&target);
    if (mType == 3) {
        sBombTargetPos.set(target);
    }
    TVec3f dir = target - mPosition;
    if (MR::normalizeOrZero(&dir)) {
        kill();
        return false;
    }
    MR::faceToVector(&mQuat, TVec3f(dir), 180.0f);
    shoot(mPosition, target);
    return true;
}

// 800FC0D0
void SandyBall::shoot(const TVec3f& rFrom, const TVec3f& rTo) {
    mPosition.set(rFrom);
    initPath(rFrom, rTo);
    MR::onBind(this);
    MR::invalidateClipping(this);
    mStep = 0;
    setNerve(&NrvSandyBall::SandyBallNrvFly::sInstance);
}

// 800FC150
void SandyBall::setOwner(LiveActor* pOwner) {
    mOwner = pOwner;
}

// 800FC160
bool SandyBall::isFree() const {
    bool ret = false;
    if (MR::isDead(this) && mOwner == nullptr) {
        ret = true;
    }
    return ret;
}

// 800FC1C0
bool SandyBall::tryBreak() {
    if (MR::isDead(this)) {
        return false;
    }
    if (isNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance)) {
        return false;
    }
    if (isNerve(&NrvSandyBall::SandyBallNrvRoll::sInstance)) {
        kill();
    } else {
        setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
    }
    return true;
}

// 800FC260
bool SandyBall::tryKill() {
    if (MR::isDead(this)) {
        return false;
    }
    makeActorDead();
    return true;
}

// 800FC2C0
bool SandyBall::tryStartBindStarPointer() {
    if (!mBindStarPointer->tryStartPointBind()) {
        return false;
    }
    mPrevNerve = mSpine->getCurrentNerve();
    setNerve(&NrvSandyBall::SandyBallNrvBindStarPointer::sInstance);
    return true;
}

// 800FC320
bool SandyBall::tryLand() {
    if (MR::isNoBind(this)) {
        return false;
    }
    if (!MR::isBinded(this)) {
        return false;
    }
    MR::tryHitReaction(this, "\222\205\222n");
    if (mType == 3) {
        setNerve(&NrvSandyBall::SandyBallNrvExplosion::sInstance);
        return true;
    }
    MR::offBind(this);
    MR::invalidateShadow(this, nullptr);
    MR::setBckRate(this, 0.0f);
    mVel.set(mVelocity);
    mStartPos.set(mPosition);
    f32 speed = PSVECMag(&mVel);
    mScaleRate = MR::getSensorRadius(this, "Body") / speed;
    setNerve(&NrvSandyBall::SandyBallNrvLand::sInstance);
    return true;
}

// 800FC420
void SandyBall::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
}

// 800FC430
void SandyBall::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pSender, this, "FireBall")) {
        if (mType == 2) {
            if (isNerve(&NrvSandyBall::SandyBallNrvFly::sInstance)) {
                if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender) && (MR::isSensorPlayer(pReceiver) || MR::isSensorMapObj(pReceiver))) {
                    setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
                }
            } else {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else if (mType == 3 && MR::isEqualSensor(pSender, this, "Explosion")) {
            if (isNerve(&NrvSandyBall::SandyBallNrvExplosion::sInstance)) {
                MR::sendMsgEnemyAttackExplosion(pReceiver, pSender);
            }
        } else {
            if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender) && (MR::isSensorPlayer(pReceiver) || MR::isSensorMapObj(pReceiver))) {
                setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
                return;
            }
            MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
        }
    }
}

// 800FC5A0
void SandyBall::setBtpFrame() {
    if (mType == 2) {
        return;
    }
    if (mType == 1) {
        MR::setBtpFrameAndStop(this, 1.0f);
        return;
    }
    if (mType == 3) {
        return;
    }
    MR::setBtpFrameAndStop(this, 0.0f);
}

// 800FC5D0
bool SandyBall::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg) && mType == 3) {
        return true;
    }
    if (MR::isMsgStarPieceAttack(msg) && mType != 3) {
        setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
        return true;
    }
    if (MR::isEqualSensor(pReceiver, this, "Body")) {
        bool isActive = false;
        if (isNerve(&NrvSandyBall::SandyBallNrvWait::sInstance) || isNerve(&NrvSandyBall::SandyBallNrvFly::sInstance) ||
            isNerve(&NrvSandyBall::SandyBallNrvLand::sInstance) || isNerve(&NrvSandyBall::SandyBallNrvBindStarPointer::sInstance)) {
            isActive = true;
        }
        if (isActive) {
            if (MR::isMsgFireBallAttack(msg)) {
                setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
                return true;
            }
            if (MR::isMsgSupportTicoSpin(msg) && mType != 3) {
                setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
                return true;
            }
            if (mType == 2) {
                bool isBreak = MR::isMsgLockOnStarPieceShoot(msg) ? false : !MR::isMsgPlayerJump(msg);
                if (isBreak) {
                    setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
                    return true;
                }
            }
        }
    }
    if (MR::isEqualSensor(pReceiver, this, "FireBall")) {
        bool isActive = false;
        if (isNerve(&NrvSandyBall::SandyBallNrvWait::sInstance) || isNerve(&NrvSandyBall::SandyBallNrvFly::sInstance) ||
            isNerve(&NrvSandyBall::SandyBallNrvLand::sInstance) || isNerve(&NrvSandyBall::SandyBallNrvBindStarPointer::sInstance)) {
            isActive = true;
        }
        if (isActive && MR::isMsgFireBallAttack(msg)) {
            setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
            return true;
        }
    }
    return false;
}

// 800FC820
bool SandyBall::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mType == 3) {
        if (MR::isEqualSensor(pReceiver, this, "Explosion")) {
            return false;
        }
    }
    if (MR::isMsgExplosionAttack(msg) && mType != 3 && !isNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance)) {
        setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
        return true;
    }
    if (MR::isEqualSensor(pReceiver, this, "Body") && !isNerve(&NrvSandyBall::SandyBallNrvFly::sInstance) &&
        !isNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance)) {
        setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
        return true;
    }
    return false;
}

// 800FC930
bool SandyBall::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (mType != 3) {
        return false;
    }
    if (!MR::isEqualSensor(pReceiver, this, "Body")) {
        return false;
    }
    const char* pBombName = "\224\232\222e";
    if (!MR::isEqualString(MR::getSensorHost(pSender)->mName, pBombName)) {
        return false;
    }
    bool ret = isNerve(&NrvSandyBall::SandyBallNrvFly::sInstance);
    return ret;
}

// 800FC9E0
// NONMATCHING: negateInternal is expanded inline in the target (single call site per TU).
void SandyBall::initPath(const TVec3f& rFrom, const TVec3f& rTo) {
    TVec3f negGravity = -mGravity;
    mPath->initFromUpVector(rFrom, rTo, negGravity, 650.0f);
    f32 speed;
    if (mType == 3) {
        speed = 24.0f;
    } else {
        speed = 20.0f;
    }
    s32 frame = mPath->getLength(0.0f, 1.0f, 32) / speed;
    s32 result;
    if (frame < 70) {
        result = 70;
    } else {
        result = 180;
        if (frame <= 180) {
            result = frame;
        }
    }
    mFlyFrame = result;
}

// 800FCAB0
void SandyBall::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateHitSensors(this);
        if (mType == 3) {
            MR::showModel(this);
            MR::invalidateHitSensor(this, "Explosion");
        }
        MR::startAction(this, "SpinX");
        MR::setBckRate(this, 0.0f);
    }
    if (MR::isDead(mOwner)) {
        setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
    }
}

// 800FCB50
void SandyBall::exeFly() {
    if (mStep == 0) {
        MR::startAction(this, "SpinX");
        MR::setBckRate(this, 1.0f);
        MR::offBind(this);
        MR::validateShadow(this, nullptr);
    }
    MR::startActionSound(this, "Fly", -1, -1, -1);
    if (!tryStartBindStarPointer()) {
        mStep++;
        f32 rate = MR::clamp((f32)mStep / (f32)mFlyFrame, 0.0f, 1.0f);
        TVec3f pos;
        mPath->calcPosition(&pos, rate);
        TVec3f vel = pos - mPosition;
        MR::setVelocity(this, vel);
        if (mStep == 20) {
            MR::onBind(this);
        }
        if (!tryLand() && mStep >= 180) {
            MR::zeroVelocity(this);
            setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
        }
    }
}

// 800FCCB0
void SandyBall::exeBreak() {
    if (MR::isFirstStep(this)) {
        if (mType == 1) {
            MR::tryHitReaction(this, "\224j\211\363(\203S\201[\203\213\203h)");
        } else {
            MR::tryHitReaction(this, "\224j\211\363");
        }
        MR::invalidateHitSensors(this);
        if (mType == 1 && MR::appearCoinPop(this, mPosition, 1)) {
            mCoinPopped = true;
        }
        kill();
    }
}

// 800FCD60
void SandyBall::exeLand() {
    if ((s32)mScaleRate <= mStep) {
        MR::zeroVelocity(this);
    }
    if (!tryStartBindStarPointer()) {
        if (getWaitSecond() * 60 < mStep) {
            setNerve(&NrvSandyBall::SandyBallNrvRoll::sInstance);
            return;
        }
        mStep++;
    }
}

// 800FCDF0
void SandyBall::exeRoll() {
    if (MR::isFirstStep(this)) {
        TVec3f vel(mGravity);
        vel.scale(3.0f);
        MR::setVelocity(this, vel);
    }
    f32 dist = PSVECDistance(&mStartPos, &mPosition);
    f32 radius = MR::getSensorRadius(this, "Body");
    if (2.0f * radius < dist) {
        MR::zeroVelocity(this);
        kill();
    }
}

// 800FCEB0
void SandyBall::exeExplosion() {
    if (MR::isFirstStep(this)) {
        MR::tryHitReaction(this, "\224\232\224\255");
        MR::invalidateHitSensors(this);
        MR::validateHitSensor(this, "Explosion");
        MR::hideModel(this);
        mSensorRadius = MR::getSensorRadius(this, "Explosion");
        MR::sendSimpleMsgToActor(100, mOwner);
    }
    s32 end = 12;
    MR::setSensorRadius(this, "Explosion", MR::getLinerValue(getNerveStep(), mSensorRadius, 300.0f, end));
    if (MR::isGreaterEqualStep(this, end)) {
        MR::setSensorRadius(this, "Explosion", mSensorRadius);
        MR::showModel(this);
        kill();
    }
}

// 800FCFE0
void SandyBall::exeBindStarPointer() {
    if (MR::updateActorState(this)) {
        if (mPrevNerve == &NrvSandyBall::SandyBallNrvLand::sInstance) {
            mStep = 0;
            MR::setBckRate(this, 0.0f);
        }
        setNerve(mPrevNerve);
        mPrevNerve = nullptr;
    }
}

// 800FD050
SandyBall::~SandyBall() {
}

static const bool sCactusTable[20] = {false, false, false, true, false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false};
static u32 sBallCount = 0;

// 800FD1F0
SandyBallHolder::SandyBallHolder(Sandy* pSandy)
    : NameObj("\203T\203\223\203f\203B\201[\202\314\203T\203{\203e\203\223\203{\201[\203\213\225\333\216\235"), mSandy(pSandy) {
    mBalls0.init(48);
    for (SandyBall** p = mBalls0.begin(); p != mBalls0.end(); p++) {
        *p = new SandyBall(0, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
        MR::setPosition(*p, mSandy->mPosition);
    }
    mBalls1.init(32);
    for (SandyBall** p = mBalls1.begin(); p != mBalls1.end(); p++) {
        *p = new SandyBall(2, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
        MR::setPosition(*p, mSandy->mPosition);
    }
    mBalls2.init(16);
    for (SandyBall** p = mBalls2.begin(); p != mBalls2.end(); p++) {
        *p = new SandyBall(1, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
        MR::setPosition(*p, mSandy->mPosition);
    }
    mBalls3.init(16);
    for (SandyBall** p = mBalls3.begin(); p != mBalls3.end(); p++) {
        *p = new SandyBall(3, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
    }
    mBalls4.init(2);
    for (SandyBall** p = mBalls4.begin(); p != mBalls4.end(); p++) {
        *p = new SandyBall(4, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
    }
}

// 800FD480
SandyBall* SandyBallHolder::getBall(LiveActor* pOwner, s32 type) {
    if (type == 2) {
        return getFreeBallType2(pOwner);
    }
    if (type == 3) {
        return getFreeBallType3(pOwner);
    }
    if (type == 4) {
        return getFreeBallType4(pOwner);
    }
    u32 idx = sBallCount;
    if (idx >= 20) {
        idx = 0;
    }
    sBallCount = idx + 1;
    if (sCactusTable[idx] && !MR::isGalaxyDarkCometAppearInCurrentStage()) {
        return getFreeBallType1(pOwner);
    }
    return getFreeBall(pOwner);
}

// 800FD540
SandyBall* SandyBallHolder::getFreeBall(LiveActor* pOwner) {
    for (SandyBall** p = mBalls0.begin(); p != mBalls0.end(); p++) {
        if ((*p)->isFree()) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return nullptr;
}

// 800FD5C0
SandyBall* SandyBallHolder::getFreeBallType2(LiveActor* pOwner) {
    for (SandyBall** p = mBalls1.begin(); p != mBalls1.end(); p++) {
        if ((*p)->isFree()) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return nullptr;
}

// 800FD640
SandyBall* SandyBallHolder::getFreeBallType1(LiveActor* pOwner) {
    for (SandyBall** p = mBalls2.begin(); p != mBalls2.end(); p++) {
        if ((*p)->isFree() && !(*p)->mCoinPopped) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return getFreeBall(pOwner);
}

// 800FD6E0
SandyBall* SandyBallHolder::getFreeBallType3(LiveActor* pOwner) {
    for (SandyBall** p = mBalls3.begin(); p != mBalls3.end(); p++) {
        if ((*p)->isFree()) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return getFreeBall(pOwner);
}

// 800FD770
SandyBall* SandyBallHolder::getFreeBallType4(LiveActor* pOwner) {
    for (SandyBall** p = mBalls4.begin(); p != mBalls4.end(); p++) {
        if ((*p)->isFree()) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return getFreeBall(pOwner);
}

// 800FD800
// NONMATCHING: PTMF for_each calls (two stack copies of the member-function pointer in the target).
void SandyBallHolder::killAll() {
    bool (SandyBall::*pKill0)() = &SandyBall::tryKill;
    ForEachPtmf(mBalls0.begin(), mBalls0.end(), pKill0);
    bool (SandyBall::*pKill1)() = &SandyBall::tryKill;
    ForEachPtmf(mBalls1.begin(), mBalls1.end(), pKill1);
    bool (SandyBall::*pKill2)() = &SandyBall::tryKill;
    ForEachPtmf(mBalls2.begin(), mBalls2.end(), pKill2);
    bool (SandyBall::*pKill3)() = &SandyBall::tryKill;
    ForEachPtmf(mBalls3.begin(), mBalls3.end(), pKill3);
    bool (SandyBall::*pKill4)() = &SandyBall::tryKill;
    ForEachPtmf(mBalls4.begin(), mBalls4.end(), pKill4);
}

// 800FDA70
SandyBallHolder::~SandyBallHolder() {
}

// 800FDB10
// NONMATCHING: register allocation (string/new pointer registers swapped).
SandyMini* Sandy::createMini(s32 index, const TVec3f& rPos, const TVec3f& rRot) {
    const char* pName = "\203T\203\223\203f\203B\201[\203~\203M\201[";
    if (index == 0) {
        pName = "\203T\203\223\203f\203B\201[\203q\203_\203\212\201[";
    }
    SandyMini* pMini = new SandyMini(this, pName, new FixedPosition(mMtx, rPos, rRot));
    pMini->initWithoutIter();
    return pMini;
}

// 800FDBC0
SandyHand* Sandy::createHand(s32 index, const TVec3f& rPos, const TVec3f& rRot) {
    bool isLeft = index == 0;
    TVec3f pos(rPos);
    TVec3f rot(rRot);
    if (!isLeft) {
        pos.z *= -1.0f;
    }
    SandyHand* pHand = new SandyHand(this, isLeft, mHolder, new FixedPosition(MR::getJointMtx(this, "Spine2"), pos, rot));
    pHand->initWithoutIter();
    pHand->makeActorDead();
    return pHand;
}

// 800FDCC0
SandyBall* SandyMini::takeBall() {
    return mSandy->mHolder->getBall(this, 0);
}

// 800FDCE0
// NONMATCHING: TPos3f::setInline/identity expanded inline in the target (single call site per TU).
void attachBallToJoint(const LiveActor* pActor, SandyBall* pBall, const char* pJoint, const TVec3f& rOffset) {
    TPos3f joint;
    joint.setInline(MR::getJointMtx(pActor, pJoint));
    TPos3f offset;
    offset.mMtx[0][3] = rOffset.x;
    offset.mMtx[1][3] = rOffset.y;
    offset.mMtx[2][3] = rOffset.z;
    TPos3f result;
    result.concat(joint, offset);
    TVec3f pos(result.mMtx[0][3], result.mMtx[1][3], result.mMtx[2][3]);
    MR::setPosition(pBall, pos);
    setIdentityInline(result);
    result.getQuat(pBall->mQuat);
}

// 800FDDE0
bool Sandy::isSensorBack(HitSensor* pSensor) const {
    TVec3f front;
    MR::calcFrontVec(&front, this);
    TVec3f dir = *MR::getSensorPos(pSensor) - mPosition;
    if (MR::normalizeOrZero(&dir)) {
        return false;
    }
    return front.dot(dir) < 0.0f;
}

// 800FDE70
void Sandy::copyQuat() {
    mTargetQuat.x = mQuat.x;
    mTargetQuat.y = mQuat.y;
    mTargetQuat.z = mQuat.z;
    mTargetQuat.w = mQuat.w;
}

// 800FDEA0
// NONMATCHING: getZDir/negateInternal expanded inline in the target (single call site per TU).
void turnToPlayer(TQuat4f* pQuat, LiveActor* pActor, f32 degree) {
    TVec3f dir = *MR::getPlayerPos() - pActor->mPosition;
    MR::vecKillElement(dir, pActor->mGravity, &dir);
    MR::normalize(&dir);
    TVec3f front;
    pQuat->getZDir(front);
    MR::turnDirectionToPlayerDegreeHorizon(pActor, &front, degree);
    TVec3f up = -pActor->mGravity;
    MR::makeQuatUpFront(pQuat, up, front);
}

// 800FDFB0
// NONMATCHING: operand order of the final add (register allocation).
bool Sandy::isTotalCount(s32 count) const {
    s32 rightHand = getRightHand()->mThrowCount;
    s32 leftHand = getLeftHand()->mThrowCount;
    s32 leftMini = getLeftMini()->mBallCount;
    return count == leftMini + leftHand + (rightHand + getRightMini()->mBallCount);
}

// 800FE030
void Sandy::resetCount() {
    SandyHand* pLeftHand = getLeftHand();
    pLeftHand->mThrowCount = 0;
    SandyHand* pRightHand = getRightHand();
    pRightHand->mThrowCount = 0;
    SandyMini* pLeftMini = getLeftMini();
    pLeftMini->mBallCount = 0;
    SandyMini* pRightMini = getRightMini();
    pRightMini->mBallCount = 0;
}

// 800FE090
void Sandy::validateHands() {
    SandyHand* pLeft = getLeftHand();
    pLeft->mIsThrower = true;
    SandyHand* pRight = getRightHand();
    pRight->mIsThrower = true;
}

// 800FE0E0
void Sandy::invalidateHands() {
    SandyHand* pLeft = getLeftHand();
    pLeft->mIsThrower = false;
    SandyHand* pRight = getRightHand();
    pRight->mIsThrower = false;
}

// 800FE130
void Sandy::forceWaitHands() {
    getLeftHand()->forceWait();
    getRightHand()->forceWait();
}

// 800FE170
void Sandy::setWaitHands() {
    getLeftHand()->setWait();
    getRightHand()->setWait();
}

// 800FE1B0
void Sandy::setHandsStep(s32 step) {
    getLeftHand()->mWaitStep = step;
    getRightHand()->mWaitStep = step;
}

// 800FE200
void Sandy::setHandsBallType0() {
    getLeftHand()->setBallType0();
    getRightHand()->setBallType0();
}

// 800FE240
void Sandy::setHandsBallType3() {
    getLeftHand()->setBallType3();
    getRightHand()->setBallType3();
}

// 800FE280
void Sandy::setMinisNext() {
    getLeftMini()->setNextNerve();
    getRightMini()->setNextNerve();
}

// 800FE2C0
bool SandyMini::tryStartNext() {
    if (!isNerveStop()) {
        return false;
    }
    resume();
    return true;
}

// 800FE310
bool Sandy::isMinisDownOrDead() const {
    bool ret = false;
    if (getLeftMini()->isDownOrDead()) {
        if (getRightMini()->isDownOrDead()) {
            ret = true;
        }
    }
    return ret;
}

// 800FE370
void Sandy::showAll() {
    MR::showModel(this);
    MR::showModel(getLeftHand());
    MR::showModel(getRightHand());
}

// 800FE3C0
void Sandy::startActionAll(const char* pName, const char* pHandName) {
    MR::startAction(this, pName);
    getLeftHand()->setDamageAction(pHandName != nullptr ? pHandName : pName);
    getRightHand()->setDamageAction(pHandName != nullptr ? pHandName : pName);
}

// 800FE440
bool Sandy::tryStartBlur() {
    return mBlur->tryStart();
}

// 800FE450
void Sandy::startScaleHitReaction() {
    mScaleController->startHitReaction();
}

// 800FE460
static AnimScaleParam* createScaleParam() {
    AnimScaleParam* pParam = new AnimScaleParam();
    pParam->_24 = 0.3f;
    pParam->_28 = 0.7f;
    return pParam;
}

namespace NrvSandyHand {
    NEW_NERVE(SandyHandNrvWait, SandyHand, Wait);
    NEW_NERVE(SandyHandNrvThrow, SandyHand, Throw);
    NEW_NERVE(SandyHandNrvThrowBall, SandyHand, ThrowBall);
    NEW_NERVE(SandyHandNrvThrowEnd, SandyHand, ThrowEnd);

    class SandyHandNrvStop : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
        }
        static SandyHandNrvStop sInstance;
    };
    SandyHandNrvStop SandyHandNrvStop::sInstance ATTRIBUTE_WEAK;

    class SandyHandNrvDamage : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            SandyHand* pActor = reinterpret_cast< SandyHand* >(pSpine->mExecutor);
            if (MR::isBckOneTimeAndStopped(pActor)) {
                pActor->setNerve(pActor->mPrevNerve);
            }
        }
        static SandyHandNrvDamage sInstance;
    };
    SandyHandNrvDamage SandyHandNrvDamage::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvSandyHand

static f32 sThrowRate = 1.0f;
static TVec3f sHandBallOffset(140.0f, -120.0f, 5.0f);

static inline s8 getHandType(bool isLeft) {
    s8 type = 1;
    if (isLeft) {
        type = 2;
    }
    return type;
}

// 800FE4A0
// NONMATCHING: register allocation in the ctor init list.
SandyHand::SandyHand(Sandy* pSandy, bool isLeft, SandyBallHolder* pHolder, FixedPosition* pFixedPos)
    : LiveActor(isLeft ? "\203T\203\223\203f\203B\201[\202\314\215\266\216\350" : "\203T\203\223\203f\203B\201[\202\314\211E\216\350"), mSandy(pSandy),
      mHolder(pHolder), mFixedPos(pFixedPos), mBall(nullptr), mIsRight(!isLeft), mIsThrower(isLeft), mHandType(getHandType(isLeft)), mWaitStep(30),
      mThrowCount(0), mBallType(2), mPrevNerve(nullptr), _B4(0), mScaleController(nullptr), mScaleParam(nullptr) {
}

// 800FE570
void SandyHand::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "SandyHand", mIsRight);
    if (mIsRight) {
        MR::initDLMakerTexMtx(this, reinterpret_cast< const char* >(&mHandType));
        MR::newDifferedDLBuffer(this);
        mScale.x *= -1.0f;
    }
    initNerve(&NrvSandyHand::SandyHandNrvStop::sInstance, 0);
    MR::registerDemoSimpleCastAll(this);
    mScaleParam = createScaleParam();
    mScaleController = new AnimScaleController(mScaleParam);
    makeActorDead();
}

// 800FE630
void SandyHand::appear() {
    LiveActor::appear();
    startActionWithBrk("WaitNormal");
    MR::startBrk(this, "Damage");
    MR::setBrkFrameEndAndStop(this);
    setNerve(&NrvSandyHand::SandyHandNrvStop::sInstance);
}

// 800FE690
void SandyHand::forceWait() {
    if (mIsThrower) {
        if (mBall != nullptr && !MR::isDead(mBall)) {
            mBall->kill();
        }
    }
    setNerve(&NrvSandyHand::SandyHandNrvStop::sInstance);
}

// 800FE700
void SandyHand::setWait() {
    setNerve(&NrvSandyHand::SandyHandNrvWait::sInstance);
}

// 800FE710
void SandyHand::setBallType0() {
    mBallType = 0;
}

// 800FE720
void SandyHand::setBallType3() {
    mBallType = 3;
}

// 800FE730
// NONMATCHING: register numbers of the two ps_mul operands.
void SandyHand::calcAndSetBaseMtx() {
    TVec3f scale;
    mulScaleHand(&mScaleController->_C.x, &mScale.x, &scale.x);
    MR::setBaseScale(this, scale);
    LiveActor::calcAndSetBaseMtx();
}

// 800FE790
void SandyHand::control() {
    mFixedPos->calc();
    mFixedPos->copyTrans(&mPosition);
    calcEulerHand(mFixedPos->mMtx, mRotation);
    mScaleController->updateNerve();
}

// 800FE8F0
bool SandyHand::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg)) {
        return mSandy->tryStartBlur();
    }
    return false;
}

// 800FE940
bool SandyHand::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg) || MR::isMsgFireBallAttack(msg)) {
        mScaleController->startHitReaction();
        return true;
    }
    return false;
}

// 800FE9B0
void SandyHand::startActionWithBrk(const char* pName) {
    if (!mIsRight) {
        MR::startAction(this, pName);
        return;
    }
    char buf[256];
    snprintf(buf, 256, "%sR", pName);
    if (!MR::tryStartAllAnim(this, buf)) {
        MR::startAction(this, pName);
    }
}

// 800FEA30
void SandyHand::startWaitAndSetNerve(bool isStop) {
    startActionWithBrk("WaitNoInterpole");
    setNerve(isStop ? static_cast< const Nerve* >(&NrvSandyHand::SandyHandNrvStop::sInstance) : &NrvSandyHand::SandyHandNrvWait::sInstance);
}

// 800FEA90
void SandyHand::setDamageAction(const char* pName) {
    if (!isNerve(&NrvSandyHand::SandyHandNrvDamage::sInstance)) {
        mPrevNerve = mSpine->getCurrentNerve();
    }
    setNerve(&NrvSandyHand::SandyHandNrvDamage::sInstance);
    startActionWithBrk(pName);
}

// 800FEB00
void SandyHand::exeWait() {
    if (MR::isFirstStep(this)) {
        startActionWithBrk("WaitNormal");
    }
    if (mIsThrower && MR::isGreaterStep(this, mWaitStep)) {
        mBall = mHolder->getBall(this, mBallType);
        setNerve(&NrvSandyHand::SandyHandNrvThrow::sInstance);
    }
}

// 800FEB90
void SandyHand::exeThrow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Throw");
        MR::setBckRate(this, 1.0f);
    }
    s32 frame = 14;
    if (MR::isGreaterEqualStep(this, (s32)((f32)frame / sThrowRate))) {
        mBall->appear();
        MR::zeroVelocity(mBall);
        mThrowCount++;
        setNerve(&NrvSandyHand::SandyHandNrvThrowBall::sInstance);
    }
}

// 800FEC60
// NONMATCHING: TPos3f::setInline/identity expanded inline in the target (single call site per TU).
void SandyHand::calcBallPos() {
    if (mBall != nullptr && !mBall->isFree()) {
        TPos3f joint;
        joint.setInline(MR::getJointMtx(this, "Hand1"));
        TPos3f offset;
        offset.mMtx[0][3] = sHandBallOffset.x;
        offset.mMtx[1][3] = sHandBallOffset.y;
        offset.mMtx[2][3] = sHandBallOffset.z;
        TPos3f result;
        result.concat(joint, offset);
        TVec3f pos;
        result.getTrans(pos);
        MR::setPosition(mBall, pos);
        setIdentityInline(result);
        result.getQuat(mBall->mQuat);
    }
}

// 800FED80
void SandyHand::exeThrowBall() {
    calcBallPos();
    if (MR::checkPassBckFrame(this, 58.0f)) {
        if (!mIsRight) {
            MR::startActionSound(this, "ThrowL", -1, -1, -1);
        } else {
            MR::startActionSound(this, "ThrowR", -1, -1, -1);
        }
    }
    s32 total = 59;
    s32 frame = 14;
    if (MR::isGreaterEqualStep(this, (s32)((f32)total / sThrowRate) - (s32)((f32)frame / sThrowRate))) {
        setNerve(&NrvSandyHand::SandyHandNrvThrowEnd::sInstance);
    }
}

// 800FEE90
void SandyHand::exeThrowEnd() {
    if (MR::isFirstStep(this)) {
        mBall->tryShoot();
        mBall = nullptr;
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvSandyHand::SandyHandNrvWait::sInstance);
    }
}

// 800FEEF0
SandyHand::~SandyHand() {
}

namespace NrvSandyMini {
    NEW_NERVE(SandyMiniNrvStart, SandyMini, Start);
    NEW_NERVE(SandyMiniNrvWait, SandyMini, Wait);
    NEW_NERVE(SandyMiniNrvDamage, SandyMini, Damage);
    NEW_NERVE(SandyMiniNrvDown, SandyMini, Down);
    NEW_NERVE(SandyMiniNrvRecover, SandyMini, Recover);
    NEW_NERVE(SandyMiniNrvShot, SandyMini, Shot);
    NEW_NERVE(SandyMiniNrvShotEnd, SandyMini, ShotEnd);

    class SandyMiniNrvStop : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
        }
        static SandyMiniNrvStop sInstance;
    };
    SandyMiniNrvStop SandyMiniNrvStop::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvSandyMini

static f32 sMiniShotRate = 1.0f;
static TVec3f sMiniMouthOffset(100.0f, 0.0f, 0.0f);

// 800FF0C0
SandyMini::SandyMini(Sandy* pSandy, const char* pName, FixedPosition* pFixedPos)
    : LiveActor(pName), mSandy(pSandy), mFixedPos(pFixedPos), mBall(nullptr), mBallCount(0), mWaitStep(0), mNextNerve(nullptr),
      mScaleController(nullptr), mScaleParam(nullptr) {
    mQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
}

// 800FF150
void SandyMini::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "SandyMini", false);
    initNerve(&NrvSandyMini::SandyMiniNrvWait::sInstance, 0);
    MR::registerDemoSimpleCastAll(this);
    mScaleParam = createScaleParam();
    mScaleController = new AnimScaleController(mScaleParam);
    makeActorDead();
}

// 800FF1E0
void SandyMini::appear() {
    LiveActor::appear();
    mBallCount = 0;
    setNerve(&NrvSandyMini::SandyMiniNrvStart::sInstance);
}

// 800FF220
void SandyMini::kill() {
    mBallCount++;
    LiveActor::kill();
}

// 800FF230
void SandyMini::setRecover() {
    mBallCount = 0;
    setNerve(&NrvSandyMini::SandyMiniNrvRecover::sInstance);
}

// 800FF240
void SandyMini::setNextNerve() {
    if (isNerve(&NrvSandyMini::SandyMiniNrvDown::sInstance) || isNerve(&NrvSandyMini::SandyMiniNrvDamage::sInstance)) {
        mNextNerve = &NrvSandyMini::SandyMiniNrvDown::sInstance;
    } else {
        mNextNerve = &NrvSandyMini::SandyMiniNrvWait::sInstance;
    }
    setNerve(&NrvSandyMini::SandyMiniNrvStop::sInstance);
}

// 800FF2B0
void SandyMini::resume() {
    if (mNextNerve != nullptr) {
        setNerve(mNextNerve);
    } else {
        setNerve(&NrvSandyMini::SandyMiniNrvWait::sInstance);
    }
}

// 800FF2D0
bool SandyMini::isNerveStop() const {
    return isNerve(&NrvSandyMini::SandyMiniNrvStop::sInstance);
}

// 800FF2E0
bool SandyMini::isDownOrDead() const {
    if (isNerve(&NrvSandyMini::SandyMiniNrvDown::sInstance)) {
        return true;
    }
    if (isNerve(&NrvSandyMini::SandyMiniNrvStop::sInstance)) {
        return mNextNerve == &NrvSandyMini::SandyMiniNrvDown::sInstance;
    }
    return MR::isDead(this);
}

// 800FF360
bool SandyMini::isShot() const {
    bool ret = false;
    if (isNerve(&NrvSandyMini::SandyMiniNrvShot::sInstance) || isNerve(&NrvSandyMini::SandyMiniNrvShotEnd::sInstance)) {
        ret = true;
    }
    return ret;
}

// 800FF3C0
void SandyMini::control() {
    mScaleController->updateNerve();
}

// 800FF3D0
void SandyMini::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Body") && MR::isSensorPlayerOrRide(pReceiver) && MR::sendMsgPush(pReceiver, pSender)) {
        mScaleController->startHitReaction();
        MR::startActionSound(this, "Touch", -1, -1, -1);
    }
}

// 800FF470
// NONMATCHING: register numbers of the two ps_mul operands.
void SandyMini::calcAndSetBaseMtx() {
    TVec3f scale;
    mulScaleMini(&mScaleController->_C.x, &mScale.x, &scale.x);
    MR::setBaseScale(this, scale);
    MR::setBaseTRMtx(this, mQuat);
}

// 800FF4D0
bool SandyMini::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        mScaleController->startHitReaction();
        return true;
    }
    if (!MR::isEqualSensor(pReceiver, this, "Mouth")) {
        return false;
    }
    if (!MR::isMsgFireBallAttack(msg)) {
        return false;
    }
    if (!canReceive()) {
        return false;
    }
    setNerve(&NrvSandyMini::SandyMiniNrvDamage::sInstance);
    return true;
}

// 800FF590
bool SandyMini::canReceive() const {
    if (isNerve(&NrvSandyMini::SandyMiniNrvDamage::sInstance)) {
        return false;
    }
    if (isNerve(&NrvSandyMini::SandyMiniNrvDown::sInstance)) {
        return false;
    }
    return !isNerve(&NrvSandyMini::SandyMiniNrvRecover::sInstance);
}

// 800FF600
// NONMATCHING: position/euler inline expansion differs (operator* and TVec3 copy are expanded inline in the target).
void SandyMini::calcPosFromFixedPos() {
    mFixedPos->calc();
    mFixedPos->copyTrans(&mPosition);
    calcEulerMini(mFixedPos->mMtx, mRotation);
    mFixedPos->mMtx.getQuat(mQuat);
}

// 800FF770
void SandyMini::startWait() {
    MR::startAction(this, "WaitNoInterpole");
    setNerve(&NrvSandyMini::SandyMiniNrvWait::sInstance);
}

// 800FF7B0
void SandyMini::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "DemoSandyPowerUp");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvSandyMini::SandyMiniNrvWait::sInstance);
    }
}

// 800FF810
void SandyMini::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "WaitNormal");
        MR::startBrkAndSetFrameAndStop(this, "Damage", 0.0f);
    }
    if (MR::isGreaterEqualStep(this, mWaitStep + 90)) {
        mWaitStep = 0;
        setNerve(&NrvSandyMini::SandyMiniNrvShot::sInstance);
    }
}

// 800FF8A0
void SandyMini::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Damage");
        if (mBall != nullptr && !MR::isDead(mBall)) {
            mBall->tryBreak();
        }
    }
    if (MR::isActionEnd(this)) {
        MR::tryHitReactionDead(this);
        setNerve(&NrvSandyMini::SandyMiniNrvDown::sInstance);
    }
}

// 800FF930
void SandyMini::exeDown() {
    if (MR::isFirstStep(this)) {
        mBallCount++;
        MR::startAction(this, "DamageWait");
    }
}

// 800FF980
void SandyMini::exeRecover() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Recover");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvSandyMini::SandyMiniNrvWait::sInstance);
    }
}

// 800FF9E0
void SandyMini::exeShot() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shot");
        MR::setBckRate(this, sMiniShotRate);
    }
    s32 shotFrame = 14;
    if (MR::isStep(this, (s32)((f32)shotFrame / sMiniShotRate))) {
        mBallCount++;
        mBall = takeBall();
        mBall->appear();
        MR::zeroVelocity(mBall);
    }
    if (MR::isGreaterEqualStep(this, (s32)((f32)shotFrame / sMiniShotRate))) {
        attachBallToJoint(this, mBall, "Mouth1", sMiniMouthOffset);
    }
    s32 endFrame = 30;
    if (MR::isGreaterEqualStep(this, (s32)((f32)endFrame / sMiniShotRate))) {
        setNerve(&NrvSandyMini::SandyMiniNrvShotEnd::sInstance);
    }
}

// 800FFB60
void SandyMini::exeShotEnd() {
    if (MR::isFirstStep(this)) {
        mBall->tryShoot();
        mBall = nullptr;
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvSandyMini::SandyMiniNrvWait::sInstance);
    }
}

// 800FFBC0
SandyMini::~SandyMini() {
}

namespace NrvSandyBlur {
    class SandyBlurNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
        }
        static SandyBlurNrvWait sInstance;
    };
    SandyBlurNrvWait SandyBlurNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(SandyBlurNrvBlur, SandyBlur, Blur);
};  // namespace NrvSandyBlur

// 800FFDA0
SandyBlur::SandyBlur(const char* pName) : NerveExecutor(pName) {
    initNerve(&NrvSandyBlur::SandyBlurNrvWait::sInstance, 0);
    MR::createCenterScreenBlur();
}

// 800FFDF0
bool SandyBlur::tryStart() {
    if (!isNerve(&NrvSandyBlur::SandyBlurNrvWait::sInstance)) {
        return false;
    }
    MR::startCenterScreenBlur(45, 10.0f, 80, 5, 30);
    setNerve(&NrvSandyBlur::SandyBlurNrvBlur::sInstance);
    return true;
}

// 800FFE60
SandyBlur::~SandyBlur() {
}

// 800FFF10
void SandyBlur::exeBlur() {
    if (MR::isGreaterStep(this, 45)) {
        setNerve(&NrvSandyBlur::SandyBlurNrvWait::sInstance);
    }
}

// Demo/camera names stored as const pointers (they live in .sdata2 in the target).
static const char* const sDemoStartName = "\203T\203\223\203f\203B\201[\223o\217\352\203f\203\202";
static const char* const sDemoStartCamera = "DemoSandyStart";
static const char* const sDemoStartAction = "DemoSandyStart";
static const char* const sDemoPowerUpName = "\203T\203\223\203f\203B\201[\203p\203\217\201[\203A\203b\203v\203f\203\202";
static const char* const sDemoPowerUpCamera = "DemoSandyPowerUp";
static const char* const sDemoPowerUpAction = "DemoSandyPowerUp";

// Shared state helpers
// 80100030
bool sandyStateOpenStart(ActorStateBase< Sandy >* pState, s32 step) {
    if (MR::isFirstStep(pState)) {
        MR::startAction(pState->mHost, "OpenStart");
    }
    return MR::isGreaterEqualStep(pState, step);
}

// 80100090
bool sandyStateOpenWait(ActorStateBase< Sandy >* pState, s32 step) {
    if (MR::isFirstStep(pState)) {
        MR::startAction(pState->mHost, "OpenWait");
    }
    return MR::isGreaterEqualStep(pState, step);
}

// 801000F0
bool sandyStateClose(ActorStateBase< Sandy >* pState, bool isAll) {
    if (MR::isFirstStep(pState)) {
        if (isAll) {
            pState->mHost->startActionAll("CloseStart", nullptr);
        } else {
            MR::startAction(pState->mHost, "CloseStart");
        }
    }
    return MR::isActionEnd(pState->mHost);
}

// 80100170
// NONMATCHING: register allocation / constant materialisation order.
bool sandyStateThrow(ActorStateBase< Sandy >* pState, s32 count, s32 interval, bool isBoth) {
    if (MR::isFirstStep(pState)) {
        MR::startAction(pState->mHost, "WaitNormal");
        pState->mHost->getLeftHand()->setWait();
        pState->mHost->getLeftHand()->mIsThrower = true;
        pState->mHost->getLeftHand()->mWaitStep = interval;
        if (isBoth) {
            pState->mHost->getRightHand()->mIsThrower = false;
            pState->mHost->getRightHand()->forceWait();
        }
        pState->mHost->resetCount();
    }
    if (isBoth && MR::isStep(pState, (s32)((f32)interval * 0.5f))) {
        pState->mHost->getRightHand()->mIsThrower = true;
        pState->mHost->getRightHand()->setWait();
        pState->mHost->getRightHand()->mWaitStep = interval;
    }
    if (pState->mHost->isTotalCount(count)) {
        pState->mHost->invalidateHands();
        return true;
    }
    return false;
}

namespace NrvSandyStateDamage {
    class SandyStateDamageNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
        }
        static SandyStateDamageNrvWait sInstance;
    };
    SandyStateDamageNrvWait SandyStateDamageNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(SandyStateDamageNrvDamageFirst, SandyStateDamage, DamageFirst);
    NEW_NERVE(SandyStateDamageNrvOpenWait, SandyStateDamage, OpenWait);
    NEW_NERVE(SandyStateDamageNrvOpenWaitTurn, SandyStateDamage, OpenWaitTurn);
    NEW_NERVE(SandyStateDamageNrvDamageMini, SandyStateDamage, DamageMini);
    NEW_NERVE(SandyStateDamageNrvDamage, SandyStateDamage, Damage);
};  // namespace NrvSandyStateDamage

// 80100320
// NONMATCHING: ActorStateBase<Sandy> ctor is expanded inline in the target for this class.
SandyStateDamage::SandyStateDamage(Sandy* pSandy, s32 damageStep, s32 openStep, s32 damageMax)
    : ActorStateBase< Sandy >("\203_\203\201\201[\203W", pSandy), mDamageCount(0), mFrontVec(0.0f, 0.0f, 1.0f), mTurnRate(0.0f), mIsEnd(false),
      mDamageStep(damageStep), mOpenStep(openStep), mPlayerPos(0.0f, 0.0f, 0.0f), mDamageMax(damageMax) {
    initNerve(&NrvSandyStateDamage::SandyStateDamageNrvWait::sInstance, 0);
}

// 801003E0
void SandyStateDamage::appear() {
    mIsDead = false;
    mDamageCount = 0;
    mIsEnd = false;
    mHost->forceWaitHands();
    setNerve(&NrvSandyStateDamage::SandyStateDamageNrvDamageFirst::sInstance);
}

// 80100430
void SandyStateDamage::kill() {
    MR::startBrk(mHost, "Sandy");
    mHost->setWaitHands();
    mIsDead = true;
}

// 80100480
bool SandyStateDamage::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pReceiver, mHost, "Mouth")) {
        return false;
    }
    if (MR::isMsgFireBallAttack(msg)) {
        bool isDamageable = false;
        if (isNerve(&NrvSandyStateDamage::SandyStateDamageNrvDamageFirst::sInstance) ||
            isNerve(&NrvSandyStateDamage::SandyStateDamageNrvOpenWait::sInstance) ||
            isNerve(&NrvSandyStateDamage::SandyStateDamageNrvOpenWaitTurn::sInstance)) {
            isDamageable = true;
        }
        if (isDamageable && !mHost->isSensorBack(pSender)) {
            incDamage();
            return true;
        }
        mHost->startScaleHitReaction();
        return true;
    }
    return false;
}

// 80100570
void SandyStateDamage::incDamage() {
    mDamageCount++;
    if (mDamageCount == 1) {
        const TVec3f* pPlayerPos = MR::getPlayerPos();
        mPlayerPos.x = pPlayerPos->x;
        mPlayerPos.y = pPlayerPos->y;
        mPlayerPos.z = pPlayerPos->z;
    }
    if (mDamageCount >= 4) {
        setNerve(&NrvSandyStateDamage::SandyStateDamageNrvDamage::sInstance);
        return;
    }
    setNerve(&NrvSandyStateDamage::SandyStateDamageNrvDamageMini::sInstance);
}

// 801005F0
void SandyStateDamage::exeDamageFirst() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("DamageFirst", nullptr);
        mHost->mTargetQuat.getZDir(mFrontVec);
        mTurnRate = 0.2f;
        MR::startActionSound(mHost, "BmSandyDamageCount", mDamageCount, -1, -1);
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvSandyStateDamage::SandyStateDamageNrvOpenWaitTurn::sInstance);
    }
}

// 80100690
void SandyStateDamage::exeOpenWait() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("DamageOpenWait", nullptr);
    }
    if (MR::isGreaterStep(this, mOpenStep)) {
        MR::startActionSound(mHost, "DamageOpenWaitEnd", -1, -1, -1);
        kill();
    }
}

// 80100720
void SandyStateDamage::exeOpenWaitTurn() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("DamageOpenWait", nullptr);
    }
    f32 rate = mTurnRate;
    TVec3f front;
    TVec3f up = -mHost->mGravity;
    fn_8003D2D0(&mHost->mTargetQuat, &mHost->mTargetQuat, up, 0.017453292f * rate);
    mHost->mTargetQuat.getZDir(front);
    if (20.0f < 57.29578f * MR::diffAngleAbs(mFrontVec, front)) {
        mTurnRate *= -1.0f;
    }
    if (MR::isGreaterStep(this, mDamageStep)) {
        MR::startActionSound(mHost, "DamageOpenWaitEnd", -1, -1, -1);
        kill();
    }
}

// 80100830
void SandyStateDamage::exeDamageMini() {
    if (MR::isFirstStep(this)) {
        const char* pName;
        if (mDamageCount == 2) {
            pName = "DamageMini2nd";
        } else {
            pName = "DamageMini";
            if (mDamageCount == 3) {
                pName = "DamageMini3rd";
            }
        }
        mHost->startActionAll(pName, nullptr);
        MR::startActionSound(mHost, "BmSandyDamageCount", mDamageCount * 2, -1, -1);
    }
    if (mDamageCount == 1) {
        TVec3f front;
        mHost->mTargetQuat.getZDir(front);
        MR::turnDirectionToTargetDegreeHorizon(mHost, &front, mPlayerPos, 3.0f);
        TVec3f up = -mHost->mGravity;
        MR::makeQuatUpFront(&mHost->mTargetQuat, up, front);
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvSandyStateDamage::SandyStateDamageNrvOpenWait::sInstance);
    }
}

// 80100930
void SandyStateDamage::exeDamage() {
    if (MR::isFirstStep(this)) {
        const char* pName = "DamageNormal";
        if (mDamageMax != -1) {
            pName = "DamageDown";
        }
        mHost->startActionAll(pName, "Damage");
        if (mDamageMax != -1) {
            MR::startBrk(mHost, "Damage");
        }
        MR::startActionSound(mHost, "EstablishDamage", -1, -1, -1);
        mHost->appearStarPieceOnDamage();
    }
    if ((mDamageMax > 0 && MR::isGreaterEqualStep(this, mDamageMax)) || MR::isActionEnd(mHost)) {
        mIsEnd = true;
        kill();
    }
}

// 80100A20
SandyStateDamage::~SandyStateDamage() {
}

namespace NrvSandyStateDemoStart {
    NEW_NERVE(SandyStateDemoStartNrvStart, SandyStateDemoStart, Start);
    NEW_NERVE(SandyStateDemoStartNrvDemo, SandyStateDemoStart, Demo);
};  // namespace NrvSandyStateDemoStart

// 80100B90
// NONMATCHING: ActorStateBase<Sandy> ctor is expanded inline in the target for this class.
SandyStateDemoStart::SandyStateDemoStart(Sandy* pSandy) : ActorStateBase< Sandy >("\217\363\221\324:\203f\203\202[\212J\216n]", pSandy) {
    initNerve(&NrvSandyStateDemoStart::SandyStateDemoStartNrvStart::sInstance, 0);
}

// 80100C00
void SandyStateDemoStart::appear() {
    mIsDead = false;
    MR::invalidateClipping(mHost);
}

// 80100C10
void SandyStateDemoStart::kill() {
    mHost->setWaitHands();
    mHost->appearAll();
    if (MR::isDemoEndWaitCameraInterpolating()) {
        MR::startAction(mHost, "WaitNoInterpole");
        mHost->getLeftHand()->startWaitAndSetNerve(true);
        mHost->getRightHand()->startWaitAndSetNerve(true);
    }
    mIsDead = true;
}

// 80100C90
void SandyStateDemoStart::exeStart() {
    if (MR::tryStartDemoMarioPuppetable(mHost, sDemoStartName)) {
        MR::startAnimCameraTargetSelf(mHost, mHost->mCameraInfo, sDemoStartCamera, 0, false, 1.0f);
        setNerve(&NrvSandyStateDemoStart::SandyStateDemoStartNrvDemo::sInstance);
    }
}

// 80100D00
void SandyStateDemoStart::exeDemo() {
    if (MR::isFirstStep(this)) {
        mHost->showAll();
        mHost->startActionAll(sDemoStartAction, nullptr);
        MR::setPlayerPos("\203}\203\212\203I\203f\203\202\210\312\222u");
        MR::startBckPlayer("BattleWait", nullptr);
        MR::overlayWithPreviousScreen(2);
        fn_8001BA80();
    }
    Sandy* pHost = mHost;
    if (MR::tryEndDemo(pHost, sDemoStartName, MR::isActionEnd(pHost))) {
        MR::endAnimCamera(mHost, mHost->mCameraInfo, sDemoStartCamera, 0, true);
        kill();
    }
}

// 80100DD0
SandyStateDemoStart::~SandyStateDemoStart() {
}

namespace NrvSandyStateDemoDown {
    NEW_NERVE(SandyStateDemoDownNrvStart, SandyStateDemoDown, Start);
    NEW_NERVE(SandyStateDemoDownNrvDemo, SandyStateDemoDown, Demo);
};  // namespace NrvSandyStateDemoDown

// 80100EA0
// NONMATCHING: ActorStateBase<Sandy> ctor is expanded inline in the target for this class.
SandyStateDemoDown::SandyStateDemoDown(Sandy* pSandy) : ActorStateBase< Sandy >("State:\203_\203E\203\223", pSandy), mPowerStar(nullptr) {
    initNerve(&NrvSandyStateDemoDown::SandyStateDemoDownNrvStart::sInstance, 0);
    mPowerStar = MR::createPowerStarDemoModel(mHost, "\203f\203\202\227p\203p\203\217\201[\203X\203^\201[", mHost->getBaseMtx());
    MR::registerDemoSimpleCastAll(mPowerStar);
    MR::invalidateClipping(mPowerStar);
    mPowerStar->makeActorDead();
}

// 80100F70
void SandyStateDemoDown::kill() {
    mPowerStar->makeActorDead();
    mHost->getRightHand()->kill();
    mHost->getLeftHand()->kill();
    mIsDead = true;
}

// 80100FF0
void SandyStateDemoDown::exeStart() {
    mHost->updateTarget();
    if (MR::tryStartDemo(mHost, "\203T\203\223\203f\203B\201[\203_\203E\203\223\203f\203\202")) {
        MR::startAnimCameraTargetSelf(mHost, mHost->mCameraInfo, "DemoSandyDown", 0, false, 1.0f);
        setNerve(&NrvSandyStateDemoDown::SandyStateDemoDownNrvDemo::sInstance);
    }
}

// 80101070
void SandyStateDemoDown::exeDemo() {
    if (MR::isFirstStep(this)) {
        mHost->copyQuat();
        mHost->forceWaitHands();
        mHost->getLeftMini()->kill();
        mHost->getRightMini()->kill();
        mHost->mHolder->killAll();
        MR::killPlayerFireBalls();
        mHost->killAll();
        MR::stopStageBGM(10);
        mHost->startActionAll("DemoSandyDown", nullptr);
        MR::startBrk(mHost, "Sandy");
        MR::startBrk(mHost->getLeftHand(), "SandyHand");
        MR::startBrk(mHost->getRightHand(), "SandyHand");
        mPowerStar->appear();
        MR::startAction(mPowerStar, "DemoSandyDown");
        MR::setPlayerPos("\203}\203\212\203I\203f\203\202\210\312\222u");
        MR::startBckPlayer("WatchUp", nullptr);
        MR::overlayWithPreviousScreen(2);
        MR::onSwitchA(mHost);
    }
    if (MR::isActionEnd(mHost)) {
        MR::endAnimCamera(mHost, mHost->mCameraInfo, "DemoSandyDown", 0, true);
        MR::endDemo(mHost, "\203T\203\223\203f\203B\201[\203_\203E\203\223\203f\203\202");
        kill();
    }
}

// 801011E0
SandyStateDemoDown::~SandyStateDemoDown() {
}

namespace NrvSandyStateDemoPowerUp {
    NEW_NERVE(SandyStateDemoPowerUpNrvStart, SandyStateDemoPowerUp, Start);
    NEW_NERVE(SandyStateDemoPowerUpNrvDemo, SandyStateDemoPowerUp, Demo);
};  // namespace NrvSandyStateDemoPowerUp

// 801012B0
// NONMATCHING: ActorStateBase<Sandy> ctor is expanded inline in the target for this class.
SandyStateDemoPowerUp::SandyStateDemoPowerUp(Sandy* pSandy) : ActorStateBase< Sandy >("\217\363\221\324:\203f\203\202[\203p\203\217\201[\203A\203b\203v]", pSandy) {
    initNerve(&NrvSandyStateDemoPowerUp::SandyStateDemoPowerUpNrvStart::sInstance, 0);
}

// 80101320
void SandyStateDemoPowerUp::kill() {
    if (MR::isDemoEndWaitCameraInterpolating()) {
        mHost->setWaitHands();
        MR::startAction(mHost, "WaitNoInterpole");
        mHost->getLeftHand()->startWaitAndSetNerve(false);
        mHost->getRightHand()->startWaitAndSetNerve(false);
        mHost->getLeftMini()->startWait();
        mHost->getRightMini()->startWait();
    }
    mIsDead = true;
}

// 801013B0
void SandyStateDemoPowerUp::exeStart() {
    mHost->updateTarget();
    if (MR::tryStartDemo(mHost, sDemoPowerUpName)) {
        MR::startAnimCameraTargetSelf(mHost, mHost->mCameraInfo, sDemoPowerUpCamera, 0, false, 1.0f);
        setNerve(&NrvSandyStateDemoPowerUp::SandyStateDemoPowerUpNrvDemo::sInstance);
    }
}

// 80101420
void SandyStateDemoPowerUp::exeDemo() {
    if (MR::isFirstStep(this)) {
        turnToPlayer(&mHost->mTargetQuat, mHost, 180.0f);
        MR::hidePlayer();
        MR::killPlayerFireBalls();
        mHost->startActionAll(sDemoPowerUpAction, nullptr);
        MR::overlayWithPreviousScreen(2);
        mHost->getLeftMini()->appear();
        mHost->getRightMini()->appear();
        SandyMini* pLeftMini = mHost->getLeftMini();
        pLeftMini->mWaitStep = 90;
        SandyMini* pRightMini = mHost->getRightMini();
        pRightMini->mWaitStep = 180;
    }
    if (MR::checkPassBckFrame(mHost->getLeftMini(), 133.0f)) {
        MR::startActionSound(mHost->getLeftMini(), "DemoSandyPowerUpL", -1, -1, -1);
    }
    if (MR::checkPassBckFrame(mHost->getRightMini(), 128.0f)) {
        MR::startActionSound(mHost->getRightMini(), "DemoSandyPowerUpR", -1, -1, -1);
    }
    mHost->updateTarget();
    mHost->mMtx.setQT(mHost->mTargetQuat, mHost->mPosition);
    mHost->getLeftMini()->calcPosFromFixedPos();
    mHost->getRightMini()->calcPosFromFixedPos();
    Sandy* pHost = mHost;
    if (MR::tryEndDemo(pHost, sDemoPowerUpName, MR::isActionEnd(pHost))) {
        MR::endAnimCamera(mHost, mHost->mCameraInfo, sDemoPowerUpCamera, 0, true);
        MR::showPlayer();
        kill();
    }
}

// 80101680
SandyStateDemoPowerUp::~SandyStateDemoPowerUp() {
}

namespace NrvSandyStatePhase1 {
    class SandyStatePhase1NrvStart;
    class SandyStatePhase1NrvClose;
    class SandyStatePhase1NrvThrow;
    class SandyStatePhase1NrvOpen;
    class SandyStatePhase1NrvOpenWait;
    class SandyStatePhase1NrvDamage;
    class SandyStatePhase1NrvDamageClose;

    NEW_NERVE(SandyStatePhase1NrvStart, SandyStatePhase1, Start);
    class SandyStatePhase1NrvClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase1NrvClose sInstance;
    };
    class SandyStatePhase1NrvThrow : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase1NrvThrow sInstance;
    };
    class SandyStatePhase1NrvOpen : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase1NrvOpen sInstance;
    };
    class SandyStatePhase1NrvOpenWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase1NrvOpenWait sInstance;
    };
    NEW_NERVE(SandyStatePhase1NrvDamage, SandyStatePhase1, Damage);
    class SandyStatePhase1NrvDamageClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase1NrvDamageClose sInstance;
    };
    SandyStatePhase1NrvClose SandyStatePhase1NrvClose::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase1NrvThrow SandyStatePhase1NrvThrow::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase1NrvOpen SandyStatePhase1NrvOpen::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase1NrvOpenWait SandyStatePhase1NrvOpenWait::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase1NrvDamageClose SandyStatePhase1NrvDamageClose::sInstance ATTRIBUTE_WEAK;

    inline void SandyStatePhase1NrvClose::execute(Spine* pSpine) const {
        SandyStatePhase1* pState = reinterpret_cast< SandyStatePhase1* >(pSpine->mExecutor);
        if (sandyStateClose(pState, false)) {
            pState->setNerve(&SandyStatePhase1NrvThrow::sInstance);
        }
    }
    inline void SandyStatePhase1NrvThrow::execute(Spine* pSpine) const {
        SandyStatePhase1* pState = reinterpret_cast< SandyStatePhase1* >(pSpine->mExecutor);
        if (sandyStateThrow(pState, 3, 20, false)) {
            pState->setNerve(&SandyStatePhase1NrvOpen::sInstance);
        }
    }
    inline void SandyStatePhase1NrvOpen::execute(Spine* pSpine) const {
        SandyStatePhase1* pState = reinterpret_cast< SandyStatePhase1* >(pSpine->mExecutor);
        if (sandyStateOpenStart(pState, 60)) {
            pState->setNerve(&SandyStatePhase1NrvOpenWait::sInstance);
        }
    }
    inline void SandyStatePhase1NrvOpenWait::execute(Spine* pSpine) const {
        SandyStatePhase1* pState = reinterpret_cast< SandyStatePhase1* >(pSpine->mExecutor);
        if (sandyStateOpenWait(pState, 240)) {
            pState->setNerve(&SandyStatePhase1NrvClose::sInstance);
        }
    }
    inline void SandyStatePhase1NrvDamageClose::execute(Spine* pSpine) const {
        SandyStatePhase1* pState = reinterpret_cast< SandyStatePhase1* >(pSpine->mExecutor);
        if (sandyStateClose(pState, true)) {
            pState->setNerve(&SandyStatePhase1NrvThrow::sInstance);
        }
    }
};  // namespace NrvSandyStatePhase1

// 80101750
SandyStatePhase1::SandyStatePhase1(Sandy* pSandy) : ActorStateBase< Sandy >("\203t\203F\201[\203Y\202P", pSandy), mDamage(nullptr) {
    initNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvThrow::sInstance, 1);
    mDamage = new SandyStateDamage(mHost, 240, 240, -1);
    MR::initActorState(this, mDamage, &NrvSandyStatePhase1::SandyStatePhase1NrvDamage::sInstance, nullptr);
}

// 801017F0
void SandyStatePhase1::appear() {
    mIsDead = false;
    setNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvStart::sInstance);
}

// 80101800
void SandyStatePhase1::control() {
    if (!isNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvDamage::sInstance)) {
        mHost->updateTarget();
    }
}

// 80101840
bool SandyStatePhase1::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvDamage::sInstance)) {
        return mDamage->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvOpenWait::sInstance)) {
        if (MR::isMsgFireBallAttack(msg)) {
            if (!mHost->isSensorBack(pSender)) {
                setNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvDamage::sInstance);
            } else {
                mHost->startScaleHitReaction();
            }
            return true;
        }
        return false;
    }
    if (!MR::isEqualSensor(pReceiver, mHost, "Mouth")) {
        if (MR::isMsgStarPieceReflect(msg) || MR::isMsgFireBallAttack(msg)) {
            mHost->startScaleHitReaction();
            return true;
        }
        return false;
    }
    return false;
}

// 80101970
void SandyStatePhase1::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "WaitNormal");
        mHost->forceWaitHands();
    }
    if (MR::isGreaterEqualStep(this, 180) && MR::isBckLooped(mHost)) {
        mHost->setWaitHands();
        setNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvThrow::sInstance);
    }
}

// 80101A00
void SandyStatePhase1::exeDamage() {
    if (MR::updateActorState(this)) {
        if (mDamage->mIsEnd) {
            kill();
        } else {
            setNerve(&NrvSandyStatePhase1::SandyStatePhase1NrvDamageClose::sInstance);
        }
    }
}

// 80101A70
SandyStatePhase1::~SandyStatePhase1() {
}

namespace NrvSandyStatePhase2 {
    class SandyStatePhase2NrvClose;
    class SandyStatePhase2NrvThrow;
    class SandyStatePhase2NrvOpen;
    class SandyStatePhase2NrvOpenWait;
    class SandyStatePhase2NrvDamageClose;

    NEW_NERVE(SandyStatePhase2NrvStart, SandyStatePhase2, Start);
    class SandyStatePhase2NrvClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase2NrvClose sInstance;
    };
    class SandyStatePhase2NrvThrow : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase2NrvThrow sInstance;
    };
    class SandyStatePhase2NrvOpen : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase2NrvOpen sInstance;
    };
    class SandyStatePhase2NrvOpenWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase2NrvOpenWait sInstance;
    };
    NEW_NERVE(SandyStatePhase2NrvDamage, SandyStatePhase2, Damage);
    class SandyStatePhase2NrvDamageClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase2NrvDamageClose sInstance;
    };
    SandyStatePhase2NrvClose SandyStatePhase2NrvClose::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase2NrvThrow SandyStatePhase2NrvThrow::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase2NrvOpen SandyStatePhase2NrvOpen::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase2NrvOpenWait SandyStatePhase2NrvOpenWait::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase2NrvDamageClose SandyStatePhase2NrvDamageClose::sInstance ATTRIBUTE_WEAK;

    inline void SandyStatePhase2NrvClose::execute(Spine* pSpine) const {
        SandyStatePhase2* pState = reinterpret_cast< SandyStatePhase2* >(pSpine->mExecutor);
        if (sandyStateClose(pState, false)) {
            pState->setNerve(&SandyStatePhase2NrvThrow::sInstance);
        }
    }
    inline void SandyStatePhase2NrvThrow::execute(Spine* pSpine) const {
        SandyStatePhase2* pState = reinterpret_cast< SandyStatePhase2* >(pSpine->mExecutor);
        if (sandyStateThrow(pState, 4, 90, true)) {
            pState->setNerve(&SandyStatePhase2NrvOpen::sInstance);
        }
    }
    inline void SandyStatePhase2NrvOpen::execute(Spine* pSpine) const {
        SandyStatePhase2* pState = reinterpret_cast< SandyStatePhase2* >(pSpine->mExecutor);
        if (sandyStateOpenStart(pState, 60)) {
            pState->setNerve(&SandyStatePhase2NrvOpenWait::sInstance);
        }
    }
    inline void SandyStatePhase2NrvOpenWait::execute(Spine* pSpine) const {
        SandyStatePhase2* pState = reinterpret_cast< SandyStatePhase2* >(pSpine->mExecutor);
        if (sandyStateOpenWait(pState, 120)) {
            pState->setNerve(&SandyStatePhase2NrvClose::sInstance);
        }
    }
    inline void SandyStatePhase2NrvDamageClose::execute(Spine* pSpine) const {
        SandyStatePhase2* pState = reinterpret_cast< SandyStatePhase2* >(pSpine->mExecutor);
        if (sandyStateClose(pState, true)) {
            pState->setNerve(&SandyStatePhase2NrvThrow::sInstance);
        }
    }
};  // namespace NrvSandyStatePhase2

// 80101D50
SandyStatePhase2::SandyStatePhase2(Sandy* pSandy) : ActorStateBase< Sandy >("\203t\203F\201[\203Y\202Q", pSandy), mDamage(nullptr) {
    initNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvThrow::sInstance, 1);
    mDamage = new SandyStateDamage(mHost, 120, 120, -1);
    MR::initActorState(this, mDamage, &NrvSandyStatePhase2::SandyStatePhase2NrvDamage::sInstance, nullptr);
}

// 80101DF0
void SandyStatePhase2::appear() {
    mIsDead = false;
    setNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvStart::sInstance);
}

// 80101E00
void SandyStatePhase2::control() {
    if (!isNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvDamage::sInstance)) {
        mHost->updateTarget();
    }
}

// 80101E40
bool SandyStatePhase2::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvDamage::sInstance)) {
        return mDamage->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvOpenWait::sInstance)) {
        if (MR::isMsgFireBallAttack(msg)) {
            if (!mHost->isSensorBack(pSender)) {
                setNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvDamage::sInstance);
            } else {
                mHost->startScaleHitReaction();
            }
            return true;
        }
        return false;
    }
    if (!MR::isEqualSensor(pReceiver, mHost, "Mouth")) {
        if (MR::isMsgStarPieceReflect(msg) || MR::isMsgFireBallAttack(msg)) {
            mHost->startScaleHitReaction();
            return true;
        }
        return false;
    }
    return false;
}

// 80101F70
void SandyStatePhase2::exeStart() {
    if (MR::isFirstStep(this)) {
        mHost->setHandsStep(90);
        mHost->setHandsBallType0();
    }
    setNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvThrow::sInstance);
}

// 80101FD0
void SandyStatePhase2::exeDamage() {
    if (MR::updateActorState(this)) {
        mHost->invalidateHands();
        if (mDamage->mIsEnd) {
            kill();
        } else {
            setNerve(&NrvSandyStatePhase2::SandyStatePhase2NrvDamageClose::sInstance);
        }
    }
}

// 80102040
SandyStatePhase2::~SandyStatePhase2() {
}

namespace NrvSandyStatePhase3 {
    class SandyStatePhase3NrvClose;
    class SandyStatePhase3NrvDamageClose;
    class SandyStatePhase3NrvOpen;

    NEW_NERVE(SandyStatePhase3NrvStart, SandyStatePhase3, Start);
    class SandyStatePhase3NrvClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase3NrvClose sInstance;
    };
    NEW_NERVE(SandyStatePhase3NrvOpen, SandyStatePhase3, Open);
    NEW_NERVE(SandyStatePhase3NrvOpenWait, SandyStatePhase3, OpenWait);
    NEW_NERVE(SandyStatePhase3NrvDamage, SandyStatePhase3, Damage);
    class SandyStatePhase3NrvDamageClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase3NrvDamageClose sInstance;
    };
    NEW_NERVE(SandyStatePhase3NrvThrow, SandyStatePhase3, Throw);
    NEW_NERVE(SandyStatePhase3NrvThrowBomb, SandyStatePhase3, ThrowBomb);
    SandyStatePhase3NrvClose SandyStatePhase3NrvClose::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase3NrvDamageClose SandyStatePhase3NrvDamageClose::sInstance ATTRIBUTE_WEAK;

    inline void SandyStatePhase3NrvClose::execute(Spine* pSpine) const {
        SandyStatePhase3* pState = reinterpret_cast< SandyStatePhase3* >(pSpine->mExecutor);
        if (sandyStateClose(pState, false)) {
            pState->setNerve(&SandyStatePhase3NrvThrow::sInstance);
        }
    }
    inline void SandyStatePhase3NrvDamageClose::execute(Spine* pSpine) const {
        SandyStatePhase3* pState = reinterpret_cast< SandyStatePhase3* >(pSpine->mExecutor);
        if (sandyStateClose(pState, true)) {
            pState->setNerve(&SandyStatePhase3NrvThrow::sInstance);
        }
    }
};  // namespace NrvSandyStatePhase3

// 80102320
SandyStatePhase3::SandyStatePhase3(Sandy* pSandy)
    : ActorStateBase< Sandy >("\203t\203F\201[\203Y\202R", pSandy), mDamage(nullptr), mMiniWait0(-1), mMiniWait1(-1) {
    initNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvThrow::sInstance, 1);
    mDamage = new SandyStateDamage(mHost, 120, 120, 40);
    MR::initActorState(this, mDamage, &NrvSandyStatePhase3::SandyStatePhase3NrvDamage::sInstance, nullptr);
}

// 801023D0
void SandyStatePhase3::appear() {
    mIsDead = false;
    setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvStart::sInstance);
}

// 801023E0
void SandyStatePhase3::control() {
    if (!isNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvDamage::sInstance)) {
        mHost->updateTarget();
        fn_8009A780(&mHost->mMtx, &mHost->mTargetQuat, &mHost->mPosition);
    } else {
        TQuat4f quat;
        mHost->mMtx.getQuat(quat);
        turnToPlayer(&quat, mHost, 1.2f);
        fn_8009A780(&mHost->mMtx, &quat, &mHost->mPosition);
    }
    updateMinis();
}

// 80102480
void SandyStatePhase3::updateMinis() {
    if (mHost->getLeftMini()->isDownOrDead() && mMiniWait0 < 0) {
        mMiniWait0 = 900;
    }
    if (mHost->getRightMini()->isDownOrDead() && mMiniWait1 < 0) {
        mMiniWait1 = 900;
    }
    if (mMiniWait0 == 0) {
        if (mHost->getLeftMini()->isDownOrDead()) {
            mHost->getLeftMini()->setRecover();
        }
    }
    if (mMiniWait1 == 0) {
        if (mHost->getRightMini()->isDownOrDead()) {
            mHost->getRightMini()->setRecover();
        }
    }
    if (mMiniWait0 >= 0) {
        mMiniWait0--;
    }
    if (mMiniWait1 >= 0) {
        mMiniWait1--;
    }
}

// 80102580
bool SandyStatePhase3::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvDamage::sInstance)) {
        return mDamage->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvOpenWait::sInstance)) {
        if (MR::isMsgFireBallAttack(msg)) {
            if (!mHost->isSensorBack(pSender)) {
                setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvDamage::sInstance);
            } else {
                mHost->startScaleHitReaction();
            }
            return true;
        }
        return false;
    }
    if (!MR::isEqualSensor(pReceiver, mHost, "Mouth")) {
        if (MR::isMsgStarPieceReflect(msg) || MR::isMsgFireBallAttack(msg)) {
            mHost->startScaleHitReaction();
            return true;
        }
        return false;
    }
    return false;
}

// 801026B0
void SandyStatePhase3::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "WaitNormal");
        mHost->setHandsStep(90);
        mHost->setHandsBallType0();
    }
    if (MR::isGreaterStep(this, 180) && MR::isBckLooped(mHost)) {
        setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvThrow::sInstance);
    }
}

// 80102740
void SandyStatePhase3::exeOpen() {
    if (MR::isFirstStep(this)) {
        mHost->invalidateHands();
    }
    if (sandyStateOpenStart(this, 60)) {
        setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvOpenWait::sInstance);
    }
}

// 801027A0
void SandyStatePhase3::exeOpenWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "OpenWait");
        mHost->invalidateHands();
        mHost->getLeftMini()->tryStartNext();
    }
    if (MR::isStep(this, 90)) {
        mHost->getRightMini()->tryStartNext();
    }
    if (MR::isGreaterStep(this, 105)) {
        setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvClose::sInstance);
    }
}

// 80102840
void SandyStatePhase3::exeDamage() {
    MR::updateActorState(this);
    if (mDamage->mIsDead) {
        mHost->invalidateHands();
        if (mDamage->mIsEnd) {
            kill();
        } else {
            setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvDamageClose::sInstance);
        }
    }
}

// 801028C0
bool SandyStatePhase3::isMinisReady() const {
    if (mHost->getLeftMini()->isDownOrDead()) {
        if (mHost->getRightMini()->isDownOrDead()) {
            return true;
        }
    }
    if (mHost->isTotalCount(4)) {
        if (!mHost->getLeftMini()->isShot()) {
            if (!mHost->getRightMini()->isShot()) {
                return true;
            }
        }
    }
    return false;
}

// 80102960
void SandyStatePhase3::exeThrow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "WaitNormal");
        mHost->invalidateHands();
        mHost->resetCount();
    }
    if (isMinisReady()) {
        setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvThrowBomb::sInstance);
    }
}

// 801029D0
void SandyStatePhase3::exeThrowBomb() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "WaitNormal");
        mHost->resetCount();
        mHost->setHandsBallType3();
        mHost->validateHands();
        mHost->setMinisNext();
    }
    if (MR::isGreaterStep(this, 60)) {
        setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvOpen::sInstance);
    }
}

// 80102A60
SandyStatePhase3::~SandyStatePhase3() {
}
