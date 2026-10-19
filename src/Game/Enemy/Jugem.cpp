#pragma auto_inline off
#include "Game/Enemy/Jugem.hpp"
#include "Game/Util.hpp"
#include <JSystem/JGeometry/TUtil.hpp>
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/Togezo.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

#include "Game/Util/YoshiLockOnUtil.hpp"
#include "Game/NameObj/NameObjFactory.hpp"

namespace NrvJugem {
    NEW_NERVE(JugemNrvWait, Jugem, Wait);
    NEW_NERVE(JugemNrvSearch, Jugem, Search);
    NEW_NERVE(JugemNrvFollow, Jugem, Follow);
    NEW_NERVE(JugemNrvApproach, Jugem, Approach);
    NEW_NERVE(JugemNrvPickTogezo, Jugem, PickTogezo);
    NEW_NERVE(JugemNrvMoveUp, Jugem, MoveUp);
    NEW_NERVE(JugemNrvThrowSign, Jugem, ThrowSign);
    NEW_NERVE(JugemNrvThrow, Jugem, Throw);
    NEW_NERVE(JugemNrvThrowEnd, Jugem, ThrowEnd);
    NEW_NERVE(JugemNrvHitReaction, Jugem, HitReaction);
    NEW_NERVE(JugemNrvStarPieceReaction, Jugem, StarPieceReaction);
    NEW_NERVE(JugemNrvBlowDown, Jugem, BlowDown);
    NEW_NERVE(JugemNrvPressDown, Jugem, PressDown);
    NEW_NERVE(JugemNrvDead, Jugem, Dead);
    NEW_NERVE(JugemNrvReBirth, Jugem, ReBirth);

    // Eaten by Yoshi: the nerve has no executor (empty execute, bare blr in SB4E01).
    class JugemNrvEaten : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static JugemNrvEaten sInstance;
    };
    JugemNrvEaten JugemNrvEaten::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(JugemNrvFallDownEaten, Jugem, FallDownEaten);
    NEW_NERVE(JugemNrvFallLand, Jugem, FallLand);
    NEW_NERVE(JugemNrvFallRecovery, Jugem, FallRecovery);
    NEW_NERVE(JugemNrvFallDownTrample, Jugem, FallDownTrample);
    NEW_NERVE(JugemNrvFallDownHipDrop, Jugem, FallDownHipDrop);
    NEW_NERVE(JugemNrvLandPressDown, Jugem, LandPressDown);
    NEW_NERVE(JugemNrvSupportFreeze, Jugem, SupportFreeze);
};  // namespace NrvJugem

#define JN(n) (&NrvJugem::JugemNrv##n::sInstance)

// Constant data of this TU (.bss, filled by the sinit at the end).
namespace {
    extern const f32 sFollowRange;
}
extern const f32 cHeight;
extern const f32 cRayLength;
extern const f32 cFallSpeed;
extern const f32 cThrowAngle;
extern const f32 cChildRange;
extern const f32 cChildRangeScale;
extern const f32 cReboundGravity;

static TVec2f sCloudSway(49.8047, 28.7109);
static TVec3f sCarryOffset(0.0f, 180.0f, 110.0f);
static TVec3f sBinderOffsetWait(0.0f, 0.0f, 0.0f);
static TVec3f sBinderOffsetFall(0.0f, 100.0f, 0.0f);
static TVec3f sWristOffset(70.0f, -70.0f, 0.0f);
static TVec3f sWristRotate(0.0f, 0.0f, 0.0f);

// turnJugemFrontToTarget (static): turn pFront toward a target position on the horizontal plane.
static void turnJugemFrontToTarget(TVec3f* pFront, LiveActor* pActor, const TVec3f& rTarget, f32 rate) {
    TVec3f dir(0.0f);
    MR::calcVecToTargetHorizon(&dir, pActor, &rTarget, &pActor->mGravity);
    if (!MR::normalizeOrZero(&dir)) {
        MR::turnVecToVecRadian(pFront, *pFront, dir, rate, pActor->mGravity);
    }
}

// NONMATCHING: vector temporary stack slots and velocity pointer register reuse differ
// moveJugemKeepingHeight (static): fly toward pTarget keeping a height above the ground, with velocity clamping.
static void moveJugemKeepingHeight(LiveActor* pActor, const TVec3f& rTarget, bool useShadow, f32 rate, f32 height, f32 maxDist, f32 speed, f32 shadowParam) {
    MR::attenuateVelocity(pActor, 0.97f);
    TVec3f toTarget(rTarget);
    toTarget -= pActor->mPosition;
    MR::vecKillElement(toTarget, pActor->mGravity, &toTarget);
    f32 mag = PSVECMag(&toTarget);
    MR::normalizeOrZero(&toTarget);
    f32 clamped = MR::normalize(mag, 50.0f, maxDist);
    MR::addVelocityMoveToDirection3D(pActor, toTarget, rate * clamped);
    if (MR::reboundVelocityFromCollision(pActor, 0.0f, 0.0f, 1.0f)) {
        MR::addVelocityToGravity(pActor, -cReboundGravity);
    } else if (useShadow) {
        TVec3f diff(rTarget);
        diff -= pActor->mPosition;
        if (-pActor->mGravity.dot(diff) > speed) {
            MR::addVelocityKeepHeight(pActor, rTarget, height, speed);
        } else if (!MR::addVelocityKeepHeightUseShadow(pActor, shadowParam, height, speed, nullptr)) {
            MR::addVelocityKeepHeight(pActor, rTarget, height, speed);
        }
    } else {
        MR::addVelocityKeepHeight(pActor, rTarget, height, speed);
    }
    f32 vDot = pActor->mVelocity.dot(pActor->mGravity);
    TVec3f vGrav;
    TVec3f gravScaled(pActor->mGravity);
    gravScaled *= vDot;
    vGrav = gravScaled;
    TVec3f vH;
    TVec3f horizontal(pActor->mVelocity);
    horizontal -= vGrav;
    vH = horizontal;
    if (PSVECMag(&vGrav) > 23.0f) {
        TVec3f tmp(pActor->mGravity);
        tmp *= 23.0f;
        vGrav = tmp;
    }
    if (PSVECMag(&vH) > 23.0f) {
        PSVECMag(&vH);
        PSVECNormalize(&vH, &vH);
        TVec3f tmp(vH);
        tmp *= 23.0f;
        vH = tmp;
    }
    TVec3f sum(vGrav);
    sum += vH;
    pActor->mVelocity = sum;
}

// findJugemNearestRailPoint (static): returns the index of the rail point nearest to pPos and writes that point to pOut.
static s32 findJugemNearestRailPoint(TVec3f* pOut, LiveActor* pActor, const TVec3f& rPos) {
    pOut->z = 0.0f;
    pOut->y = 0.0f;
    pOut->x = 0.0f;
    f32 minDist = 3.402823466e38f;
    s32 nearest;
    for (s32 i = 0; i < MR::getRailPointNum(pActor); i++) {
        TVec3f pt;
        MR::calcRailPointPos(&pt, pActor, i);
        f32 dist = PSVECDistance(&pt, &rPos);
        if (minDist > dist) {
            *pOut = pt;
            minDist = dist;
            nearest = i;
        }
    }
    return nearest;
}

Jugem::Jugem(const char* pName) : LiveActor(pName) {
    mRootJointController = nullptr;
    mWristJointController = nullptr;
    mAnimScaleCtrl = nullptr;
    mBindStarPointer = nullptr;
    mCloud = nullptr;
    mHoldTogezo = nullptr;
    mChildObjs = nullptr;
    mChildRebirthTimers = nullptr;
    mFrontVec.set(0.0f, 0.0f, 1.0f);
    mBasePos.set(0.0f, 0.0f, 0.0f);
    mBinderRadius = 0.0f;
    mChildNum = 0;
    mChildNumPlusOne = 0;
    mChildIndex = 0;
    mChildCount = 0;
    mWaitTime = 900;
    mGroundActor = nullptr;
    mCloudSwayOffset.set(0.0f, 0.0f, 0.0f);
    mHorizontalSwayStep = 0;
    mVerticalSwayStep = 0;
    mCloudSwayWeight = 0.0f;
    mTimer = 0;
    mNeedsCarryOffset = 0;
    mPointBindWithCloud = 0;
    mBaseMtx.identity();
    mHeldTogezoMatrix.identity();
}

JointController* Jugem::createJugemJointController(const char* pJointName, JointCalcFunc calcFunc) {
    JointControlDelegator<Jugem>* controller = new JointControlDelegator<Jugem>(this, calcFunc, nullptr);
    MR::setJointControllerParam(controller, this, pJointName);
    return controller;
}

void Jugem::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Jugem", false);
    mCloud = new JugemCloud(this, "\x89\x5F\x83\x82\x83\x66\x83\x8B");
    mCloud->mCheckHiddenHost = true;
    mCloud->initFixedPosition(MR::getJointMtx(this, "Root"), TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    mCloud->initWithoutIter();
    PSMTXCopy(getBaseMtx(), mBaseMtx.toMtxPtr());
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, mBaseMtx.toMtxPtr(), TVec3f(0.0f));
    mYoshiLockOn2 = MR::createYoshiLockOnTarget(mCloud, static_cast<MtxPtr>(nullptr), TVec3f(0.0f));
    MR::getJMapInfoArg0NoInit(rIter, &mWaitTime);
    mChildNum = MR::getChildObjNum(rIter);
    mChildNumPlusOne = mChildNum + 1;
    mChildObjs = new Togezo*[mChildNum];
    mChildPoses = new TVec3f[mChildNum];
    mChildRebirthTimers = new s32[mChildNum];
    mChildren = new LiveActor*[mChildNumPlusOne];
    for (s32 i = 0; i < mChildNum; i++) {
        mChildRebirthTimers[i] = 0;
        NameObj* pObj = NameObjFactory::initChildObj(rIter, i);
        mChildren[i] = static_cast< LiveActor* >(pObj);
        mChildObjs[i] = static_cast< Togezo* >(pObj);
        mChildObjs[i]->makeActorAppeared();
        mChildPoses[i] = mChildObjs[i]->mPosition;
    }
    Togezo* pTogezo = new Togezo("\x83\x67\x83\x51\x83\x5D\x81\x5B");
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
    initNerve(JN(Wait), 1);
    mRootJointController = createJugemJointController("AllRoot", &Jugem::calcRootJoint);
    mWristJointController = createJugemJointController("WristR", &Jugem::calcWristJoint);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mAnimScaleCtrl->setParamTight();
    mBindStarPointer = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mBindStarPointer, JN(SupportFreeze), "SupportFreeze");
    MR::useStageSwitchSyncAppear(this, rIter);
}

void Jugem::makeActorAppeared() {
    setNerve(JN(Wait));
    LiveActor::makeActorAppeared();
    mChildIndex = 0;
    MR::invalidateClipping(this);
    mCloud->makeActorAppeared();
    mTimer = 0;
}

void Jugem::control() {
    if (mNeedsCarryOffset) {
        if (!isNerve(JN(FallRecovery)) && !MR::isActionEnd(this)) {
            mNeedsCarryOffset = 0;
            TVec3f offset(sCarryOffset);
            TPos3f mtx;
            PSMTXCopy(getBaseMtx(), mtx.toMtxPtr());
            MR::setMtxTrans(mtx.toMtxPtr(), 0.0f, 0.0f, 0.0f);
            PSMTXMultVec(mtx.toMtxPtr(), &offset, &offset);
            mPosition += offset;
        }
    }
    if (mTimer > 0) {
        mTimer--;
    }
    updateJugemCloudSway();
    updateJugemGroundActor();
    mAnimScaleCtrl->updateNerve();
    PSMTXCopy(getBaseMtx(), mBaseMtx.toMtxPtr());
    MR::addTransMtxLocalY(mBaseMtx.toMtxPtr(), 50.0f);
    if (isJugemGroundedWithoutCloud()) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }
    if (isJugemRidingCloud()) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn2);
    }
    if (canJugemBindStarPointer() && mBindStarPointer->tryStartPointBind()) {
        mPointBindWithCloud = !isJugemFallen();
        killJugemHeldTogezo();
        setNerve(JN(SupportFreeze));
    }
}

void Jugem::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        MR::tryStartActionNotPlaying(mCloud, "Wait");
        MR::zeroVelocity(this);
        initBinder();
        mCloud->makeActorAppeared();
        mCloud->mCalcOwnMatrix = true;
    }
    if (!tryJugemPickTogezo()) {
        mStepCounter = 0;
        setNerve(JN(Follow));
    }
}

void Jugem::exeSearch() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        MR::tryStartActionNotPlaying(mCloud, "Wait");
    }
    if (mStepCounter <= 60 || !tryJugemPickTogezo()) {
        mStepCounter++;
        if (MR::isGreaterStep(this, 60)) {
            setNerve(JN(Follow));
        }
        MR::attenuateVelocity(this, 0.97f);
        turnJugemFrontToTarget(&mFrontVec, this, *MR::getPlayerPos(), 0.06f);
    }
}

// NONMATCHING: register allocation / instruction scheduling and float-constant pool layout differ from the target
void Jugem::exeFollow() {
    MR::isFirstStep(this);
    updateJugemMoveAnimation();
    if (mStepCounter <= 60 || !tryJugemPickTogezo()) {
        s32 followTime = gJugemFollowDuration;
        if (followTime > 0 && MR::isGreaterStep(this, followTime)) {
            setNerve(JN(Search));
        }
        TVec3f target(0.0f);
        calcJugemFollowPosition(&target, 350.0f);
        moveJugemKeepingHeight(this, target, true, 0.7f, 0.2f, sFollowRange, 100.0f, 350.0f);
        turnJugemFrontToTarget(&mFrontVec, this, *MR::getPlayerPos(), 0.06f);
        mStepCounter++;
    }
}

// NONMATCHING: register allocation / instruction scheduling and float-constant pool layout differ from the target
void Jugem::exeApproach() {
    if (MR::isFirstStep(this)) {
        mStepCounter = 0;
    }
    updateJugemMoveAnimation();
    TVec3f target(0.0f);
    calcJugemFollowPosition(&target, cHeight);
    const TVec3f& rPlayer = *MR::getPlayerPos();
    TVec3f diff(target);
    diff -= rPlayer;
    f32 d = MR::vecKillElement(diff, *MR::getPlayerGravity(), &diff);
    if (PSVECMag(&diff) > 900.0f) {
        MR::normalize(&diff);
        TVec3f a(diff);
        a *= 900.0f;
        TVec3f b(*MR::getPlayerGravity());
        b *= d;
        TVec3f c(*MR::getPlayerPos());
        c += b;
        TVec3f e(c);
        e += a;
        target = e;
    }
    moveJugemKeepingHeight(this, target, true, 0.7f, 0.3f, 600.0f, 200.0f, cHeight);
    turnJugemFrontToTarget(&mFrontVec, this, *MR::getPlayerPos(), 0.06f);
    TVec3f toTarget(target);
    toTarget -= mPosition;
    MR::vecKillElement(toTarget, mGravity, &toTarget);
    f32 mag = PSVECMag(&toTarget);
    TVec3f toTarget2(target);
    toTarget2 -= mPosition;
    f32 dotVal = toTarget2.dot(mGravity);
    if (150.0f < mag && mag < 900.0f && __fabsf(dotVal) < 200.0f) {
        mStepCounter++;
    }
    if (mStepCounter > 35) {
        setNerve(JN(Throw));
    }
}

void Jugem::exePickTogezo() {
    if (MR::isFirstStep(this)) {
        mStepCounter = 0;
    }
    updateJugemMoveAnimation();
    TVec3f target(0.0f);
    calcJugemPickupPosition(&target, mChildIndex);
    moveJugemKeepingHeight(this, target, true, 0.7f, 0.3f, 600.0f, 200.0f, 350.0f);
    turnJugemFrontToTarget(&mFrontVec, this, mChildPoses[mChildIndex], 0.06f);
    TVec3f toTarget(target);
    toTarget -= mPosition;
    MR::vecKillElement(toTarget, mGravity, &toTarget);
    f32 mag = PSVECMag(&toTarget);
    TVec3f toTarget2(target);
    toTarget2 -= mPosition;
    f32 dotVal = toTarget2.dot(mGravity);
    if (mag < 900.0f && __fabsf(dotVal) < 200.0f) {
        mStepCounter++;
    }
    if (mStepCounter > 35) {
        setNerve(JN(Throw));
    }
}

void Jugem::exeMoveUp() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "MoveUp");
        MR::tryStartAllAnim(mCloud, "MoveUp");
        mStepCounter = 0;
    }
    TVec3f target(0.0f);
    findJugemNearestRailPoint(&target, this, mPosition);
    moveJugemKeepingHeight(this, target, false, 0.7f, 0.3f, 600.0f, 200.0f, 350.0f);
    turnJugemFrontToTarget(&mFrontVec, this, mChildPoses[mChildIndex], 0.06f);
    TVec3f toTarget(target);
    toTarget -= mPosition;
    MR::vecKillElement(toTarget, mGravity, &toTarget);
    f32 mag = PSVECMag(&toTarget);
    TVec3f toTarget2(target);
    toTarget2 -= mPosition;
    f32 dotVal = toTarget2.dot(mGravity);
    if (mag < 600.0f && __fabsf(dotVal) < 200.0f) {
        mStepCounter++;
    }
    if (mStepCounter > 35) {
        setNerve(JN(Wait));
    }
}

void Jugem::exeThrowSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ThrowSign");
        MR::startAction(mCloud, "Wait");
    }
    if (MR::isActionEnd(this)) {
        setNerve(JN(Throw));
    }
}

void Jugem::exeThrow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Throw");
        MR::tryStartAllAnim(mCloud, "Throw");
    }
    if (MR::isLessStep(this, 21)) {
        MR::attenuateVelocity(this, 0.95f);
        turnJugemFrontToTarget(&mFrontVec, this, mChildPoses[mChildIndex], 0.06f);
    } else {
        MR::zeroVelocity(this);
    }
    if (MR::isStep(this, 22)) {
        mHoldTogezo = mChildObjs[mChildIndex];
        if (mHoldTogezo != nullptr) {
            mHoldTogezo->appearHold(mHeldTogezoMatrix.toMtxPtr());
        }
    }
    if (MR::isStep(this, 42)) {
        throwJugemHeldTogezo(mChildPoses[mChildIndex]);
    }
    if (MR::isActionEnd(this)) {
        setNerve(JN(ThrowEnd));
    }
}

void Jugem::exeThrowEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::tryStartActionNotPlaying(mCloud, "Wait");
    }
    turnJugemFrontToTarget(&mFrontVec, this, *MR::getPlayerPos(), 0.06f);
    if (MR::isGreaterStep(this, 80)) {
        setNerve(JN(Follow));
    }
}

void Jugem::exeHitReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HitReaction");
        MR::startAction(mCloud, "HitReaction");
    }
    turnJugemFrontToTarget(&mFrontVec, this, *MR::getPlayerPos(), 0.06f);
    if (MR::isActionEnd(this)) {
        setNerve(JN(Follow));
    }
}

void Jugem::exeStarPieceReaction() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "StarPieceReaction");
        MR::tryHitReaction(this, "\x8B\x43\x90\xE2\x8A\x4A\x8E\x6E\x81\x69\x83\x58\x83\x5E\x81\x5B\x83\x73\x81\x5B\x83\x58\x81\x6A");
    }
    if (MR::isGreaterStep(this, 120)) {
        setNerve(JN(Wait));
    }
}

void Jugem::exeBlowDown() {
    if (MR::isFirstStep(this)) {
        MR::tryStartAllAnim(this, "BlowDown");
        MR::tryStartActionNotPlaying(mCloud, "Wait");
        mCloud->disappear();
    }
    MR::attenuateVelocity(this, 0.97f);
    if (!MR::isBindedGround(this)) {
        MR::addVelocityToGravity(this, 1.5f);
        TVec3f up(-mVelocity);
        MR::turnDirectionDegree(this, &mFrontVec, up, 15.0f);
    }
    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f)) || MR::isGreaterStep(this, 120) || (MR::isGreaterStep(this, 10) && MR::isBindedGround(this))) {
        MR::emitItemHit(this);
        tryJugemDead();
    }
}

void Jugem::exePressDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "PressDown");
    }
    if (MR::isActionEnd(this) || MR::isPlayerHipDropLand() || MR::isBindedGround(this)) {
        mCloud->tryDisappear();
        MR::emitItemStomp(this);
        tryJugemDead();
    }
}

// NONMATCHING: register allocation / instruction scheduling and float-constant pool layout differ from the target
void Jugem::exeDead() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
        MR::startAction(this, "Dead");
        MR::setScale(this, TVec3f(1.0f));
        MR::zeroVelocity(this);
    }
    if (MR::isGreaterStep(this, mWaitTime)) {
        calcJugemFollowPosition(&mPosition, cHeight);
        TVec3f pos(mPosition);
        const TVec3f& rPlayer = *MR::getPlayerPos();
        TVec3f diff(mPosition);
        diff -= rPlayer;
        f32 d = MR::vecKillElement(diff, *MR::getPlayerGravity(), &diff);
        if (PSVECMag(&diff) > 900.0f) {
            MR::normalize(&diff);
            TVec3f a(diff);
            a *= 900.0f;
            TVec3f b(*MR::getPlayerGravity());
            b *= d;
            TVec3f c(*MR::getPlayerPos());
            c += b;
            TVec3f e(c);
            e += a;
            pos = e;
        }
        TVec3f g(mGravity);
        g *= cRayLength;
        TVec3f from(pos);
        from -= g;
        TVec3f g2(mGravity);
        g2 *= (cRayLength + cHeight);
        if (MR::getFirstPolyOnLineToMap(&pos, nullptr, from, g2)) {
            TVec3f h(mGravity);
            h *= cHeight;
            pos -= h;
        }
        MR::setPosition(this, pos);
        MR::showModelAndEffects(this);
        calcAnim();
        setNerve(JN(ReBirth));
    }
}

void Jugem::exeReBirth() {
    if (MR::isFirstStep(this)) {
        initBinder();
        MR::setScale(this, 1.0f);
        MR::setScale(mCloud, 1.0f);
        mCloud->appear();
        turnJugemFrontToTarget(&mFrontVec, this, *MR::getPlayerPos(), 1.0f);
        MR::startAction(this, "ReBirth");
        MR::startAction(mCloud, "ReBirth");
        mTimer = 0;
    }
    turnJugemFrontToTarget(&mFrontVec, this, *MR::getPlayerPos(), 0.06f);
    if (MR::isActionEnd(this)) {
        setNerve(JN(Wait));
    }
}

void Jugem::exeFallDownEaten() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::addVelocityToGravity(this, -cFallSpeed);
        MR::startAction(this, "FallDown");
        MR::tryHitReaction(this, "\x92\xC4\x97\x8E\x8A\x4A\x8E\x6E\x81\x69\x89\x5F\x82\xBD\x82\xD7\x82\xE7\x82\xEA\x81\x6A");
        initBinderFall();
        mStepCounter = 0;
    }
    mStepCounter++;
    MR::attenuateVelocity(this, 0.97f);
    if (!isJugemGroundActorValid()) {
        MR::addVelocityToGravity(this, 1.5f);
        MR::rotateDirectionGravityDegree(this, &mFrontVec, 15.0f);
    } else {
        setNerve(JN(FallLand));
        return;
    }
    if (mStepCounter > 180) {
        setNerve(JN(FallRecovery));
    }
    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        MR::emitItemHit(this);
        tryJugemDead();
    }
}

void Jugem::exeFallLand() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FallLand");
    }
    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(this, "FallDown");
    }
    mStepCounter++;
    if (!isJugemGroundActorValid()) {
        MR::addVelocityToGravity(this, 1.5f);
    } else {
        MR::zeroVelocity(this);
    }
    if (mStepCounter > 180) {
        setNerve(JN(FallRecovery));
    }
    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        MR::emitItemHit(this);
        tryJugemDead();
    }
}

void Jugem::exeFallRecovery() {
    if (MR::isFirstStep(this)) {
        JugemCloud* pCloud = mCloud;
        pCloud->LiveActor::appear();
        pCloud->mCalcOwnMatrix = true;
        MR::setScale(pCloud, 1.0f);
        pCloud->calcAnim();
        MR::startAction(this, "FallRecovery");
        MR::startAction(mCloud, "FallRecovery");
    }
    if (!isJugemGroundActorValid()) {
        MR::addVelocityToGravity(this, 1.5f);
    } else {
        MR::zeroVelocity(this);
    }
    if (MR::isActionEnd(this)) {
        setNerve(JN(MoveUp));
        mNeedsCarryOffset = 1;
    }
}

void Jugem::exeFallDownTrample() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FallDown");
        MR::tryHitReaction(this, "\x92\xC4\x97\x8E\x8A\x4A\x8E\x6E\x81\x69\x93\xA5\x82\xDC\x82\xEA\x81\x6A");
        MR::zeroVelocity(this);
        initBinderFall();
        mStepCounter = 0;
        mCloud->mCalcOwnMatrix = false;
    }
    mStepCounter++;
    MR::attenuateVelocity(this, 0.97f);
    if (!MR::isBindedGround(this)) {
        MR::addVelocityToGravity(this, 1.5f);
    } else {
        MR::zeroVelocity(this);
        mCloud->tryDisappear();
        setNerve(JN(LandPressDown));
        return;
    }
    if (mStepCounter > 180) {
        MR::emitItemStomp(this);
        mCloud->disappear();
        tryJugemDead();
    }
}

void Jugem::exeFallDownHipDrop() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FallDown");
        MR::tryHitReaction(this, "\x92\xC4\x97\x8E\x8A\x4A\x8E\x6E\x81\x69\x83\x71\x83\x62\x83\x76\x83\x68\x83\x8D\x83\x62\x83\x76\x81\x6A");
        MR::zeroVelocity(this);
        initBinderFall();
        mStepCounter = 0;
        mCloud->mCalcOwnMatrix = false;
    }
    if (!MR::isBindedGround(this)) {
        if (MR::isPlayerHipDropFalling()) {
            const TVec3f& rPlayer = *MR::getPlayerPos();
            TVec3f diff(mPosition);
            diff -= rPlayer;
            MR::vecKillElement(diff, mGravity, &diff);
            TVec3f pos(*MR::getPlayerPos());
            pos += diff;
            MR::setPosition(this, pos);
        } else {
            MR::addVelocityToGravity(this, 1.5f);
        }
    } else {
        MR::zeroVelocity(this);
    }
    mStepCounter++;
    if (MR::isPlayerHipDropLand() || mStepCounter > 180 || MR::isBindedGround(this)) {
        setNerve(JN(PressDown));
    }
}

void Jugem::exeLandPressDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "LandPressDown");
    }
    if (MR::isActionEnd(this)) {
        MR::emitItemStomp(this);
        mCloud->disappear();
        tryJugemDead();
    }
}

void Jugem::exeSupportFreeze() {
    if (MR::updateActorState(this)) {
        if (mPointBindWithCloud) {
            setNerve(JN(Wait));
            return;
        }
        setNerve(JN(FallRecovery));
    }
}

void Jugem::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFrontVec);
    MR::calcFrontVec(&mFrontVec, this);
    MR::updateBaseScale(this, mAnimScaleCtrl);
    if (!MR::isDead(this)) {
        mRootJointController->registerCallBack();
        mWristJointController->registerCallBack();
    }
}

void Jugem::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(JN(Dead))) {
        if (MR::isSensorEnemyAttack(pSender) && MR::isSensorPlayerOrRide(pReceiver) && canJugemAttackPlayer()) {
            if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                tryJugemHitReaction();
                return;
            }
            return;
        } else if ((!MR::isSensorEnemy(pReceiver) && !MR::isSensorMapObj(pReceiver) && !MR::isSensorYoshi(pReceiver)) ||
                   !MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender)) {
            if (MR::isSensorPlayerOrRide(pReceiver) && MR::isPlayerDamaging()) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    }
}

bool Jugem::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(JN(Dead))) {
        return false;
    }
    if (MR::isSensorEnemy(pReceiver)) {
        if (MR::isMsgLockOnStarPieceShoot(msg)) {
            return true;
        }
        if (MR::isMsgStarPieceAttack(msg)) {
            tryJugemStarPieceReaction();
            return true;
        }
        if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
            if (isJugemRidingCloud()) {
                mCloud->disappear();
                tryJugemLoseCloud();
                return true;
            }
            return tryJugemBlowDown(pSender, pReceiver);
        }
        if (isJugemRidingCloud()) {
            if (MR::isMsgPlayerTrample(msg)) {
                return tryJugemFallFromTrample(pSender, pReceiver);
            }
            if (MR::isMsgPlayerHipDrop(msg)) {
                return tryJugemFallFromHipDrop(pSender, pReceiver);
            }
        } else {
            if (MR::isMsgPlayerTrample(msg)) {
                return tryJugemPressDown(pSender, pReceiver);
            }
            if (MR::isMsgPlayerHipDrop(msg)) {
                return tryJugemPressDown(pSender, pReceiver);
            }
        }
        if (MR::isMsgJetTurtleAttack(msg) || MR::isMsgYoshiProjectile(msg) || MR::isMsgExplosionAttack(msg) || MR::isMsgYoshiEat(msg)) {
            return tryJugemBlowDown(pSender, pReceiver);
        }
        return false;
    }
    return false;
}

bool Jugem::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(JN(Dead))) {
        return false;
    }
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (isJugemGroundedWithoutCloud()) {
            tryJugemFallFromYoshiTongue(pSender, pReceiver);
            return false;
        }
        if (isJugemRidingCloud()) {
            mCloud->mCalcOwnMatrix = false;
            tryJugemLoseCloud();
            return true;
        }
        return false;
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return true;
    }
    if (MR::isMsgYoshiReleaseOrCancelEaten(msg)) {
        if (isNerve(JN(Eaten))) {
            setNerve(JN(Dead));
            return true;
        }
        mCloud->tryDisappear();
        return true;
    }
    if (MR::getSensorHost(pReceiver) == this && MR::isMsgPlayerKick(msg)) {
        return tryJugemKickBlowDown(pSender, pReceiver);
    }
    if (MR::isMsgInhaleBlackHole(msg)) {
        return tryJugemDead();
    }
    return false;
}

bool Jugem::canJugemAttackPlayer() {
    if (isNerve(JN(FallRecovery))) {
        return MR::isGreaterEqualStep(this, 69);
    }
    bool ret = false;
    if (!isJugemDefeated() && !isNerve(JN(FallDownEaten)) && !isNerve(JN(FallLand)) && !isNerve(JN(SupportFreeze))) {
        ret = true;
    }
    return ret;
}

bool Jugem::isJugemGroundedWithoutCloud() {
    bool ret = false;
    if (isNerve(JN(FallLand)) || (isNerve(JN(SupportFreeze)) && mPointBindWithCloud == 0)) {
        ret = true;
    }
    return ret;
}

bool Jugem::isJugemRidingCloud() {
    if (isNerve(JN(FallRecovery))) {
        return MR::isGreaterStep(this, 69);
    }
    bool ret = false;
    if (!isJugemDefeated() && !isNerve(JN(FallDownEaten)) && !isNerve(JN(FallLand))) {
        ret = true;
    }
    return ret;
}

bool Jugem::canJugemBindStarPointer() {
    bool ret;
    if (mTimer > 0) {
        ret = false;
    } else {
        ret = false;
        if (!isJugemDefeated() && !isNerve(JN(FallDownEaten)) && !isNerve(JN(FallRecovery)) &&
            (!isNerve(JN(StarPieceReaction)) || !MR::isLessStep(this, 60)) && !isNerve(JN(SupportFreeze))) {
            ret = true;
        }
    }
    return ret;
}

bool Jugem::isJugemDefeated() {
    bool ret = false;
    if (isNerve(JN(Dead)) || isNerve(JN(BlowDown)) || isNerve(JN(PressDown)) || isNerve(JN(FallDownTrample)) || isNerve(JN(FallDownHipDrop)) ||
        isNerve(JN(LandPressDown)) || isNerve(JN(Eaten))) {
        ret = true;
    }
    return ret;
}

bool Jugem::isJugemFallen() {
    bool ret = false;
    if (isNerve(JN(FallDownEaten)) || isNerve(JN(FallLand)) || (isNerve(JN(FallRecovery)) && MR::isLessStep(this, 30))) {
        ret = true;
    }
    return ret;
}

bool Jugem::tryJugemHitReaction() {
    if (isNerve(JN(Search)) || isNerve(JN(Follow)) || isNerve(JN(PickTogezo)) || isNerve(JN(Approach)) || isNerve(JN(ThrowEnd))) {
        setNerve(JN(HitReaction));
        return true;
    }
    return false;
}

bool Jugem::tryJugemLoseCloud() {
    if (isJugemRidingCloud()) {
        killJugemHeldTogezo();
        setNerve(JN(FallDownEaten));
        mTimer = 60;
        mGroundActor = nullptr;
        return true;
    }
    return false;
}

bool Jugem::tryJugemStarPieceReaction() {
    if (isNerve(JN(FallLand)) || isNerve(JN(FallRecovery)) || (isNerve(JN(SupportFreeze)) && mPointBindWithCloud == 0)) {
        if (isNerve(JN(FallRecovery))) {
            mCloud->disappear();
        }
        mGroundActor = nullptr;
        setNerve(JN(FallDownEaten));
        mStepCounter = 0;
        mTimer = 60;
        return true;
    }
    bool ret = false;
    if (isNerve(JN(FallDownEaten)) || isJugemDefeated()) {
        ret = true;
    }
    if (!ret) {
        killJugemHeldTogezo();
        setNerve(JN(StarPieceReaction));
        mTimer = 60;
        return true;
    }
    return false;
}

bool Jugem::tryJugemBlowDown(HitSensor* pSender, HitSensor* pReceiver) {
    bool ret = false;
    if (isJugemDefeated() || isNerve(JN(FallDownEaten))) {
        ret = true;
    }
    if (!ret) {
        killJugemHeldTogezo();
        MR::zeroVelocity(this);
        MR::setVelocityBlowAttack(this, pSender, pReceiver, 36.0f, 32.0f, 4);
        setNerve(JN(BlowDown));
        MR::tryHitReaction(this, "\x90\x81\x82\xC1\x94\xF2\x82\xD1");
        return true;
    }
    return false;
}

bool Jugem::tryJugemFallFromYoshiTongue(HitSensor*, HitSensor*) {
    bool ret = false;
    if (isJugemDefeated() || isNerve(JN(FallDownEaten))) {
        ret = true;
    }
    if (!ret) {
        killJugemHeldTogezo();
        MR::zeroVelocity(this);
        setNerve(JN(FallDownEaten));
        mGroundActor = nullptr;
        MR::tryHitReaction(this, "\x83\x88\x83\x62\x83\x56\x81\x5B\x90\xE3\x83\x41\x83\x4E\x83\x5A\x83\x58");
        return true;
    }
    return false;
}

bool Jugem::tryJugemPressDown(HitSensor*, HitSensor*) {
    bool ret = false;
    if (isJugemDefeated() || isNerve(JN(FallDownEaten))) {
        ret = true;
    }
    if (!ret) {
        killJugemHeldTogezo();
        MR::zeroVelocity(this);
        setNerve(JN(PressDown));
        MR::tryHitReaction(this, "\x93\xA5\x82\xDC\x82\xEA");
        return true;
    }
    return false;
}

bool Jugem::tryJugemFallFromTrample(HitSensor*, HitSensor*) {
    bool ret = false;
    if (isJugemDefeated() || isNerve(JN(FallDownEaten))) {
        ret = true;
    }
    if (!ret) {
        killJugemHeldTogezo();
        MR::zeroVelocity(this);
        setNerve(JN(FallDownTrample));
        MR::tryHitReaction(this, "\x93\xA5\x82\xDC\x82\xEA");
        return true;
    }
    return false;
}

bool Jugem::tryJugemFallFromHipDrop(HitSensor*, HitSensor*) {
    bool ret = false;
    if (isJugemDefeated() || isNerve(JN(FallDownEaten))) {
        ret = true;
    }
    if (!ret) {
        killJugemHeldTogezo();
        MR::zeroVelocity(this);
        setNerve(JN(FallDownHipDrop));
        MR::tryHitReaction(this, "\x93\xA5\x82\xDC\x82\xEA");
        return true;
    }
    return false;
}

bool Jugem::tryJugemKickBlowDown(HitSensor* pSender, HitSensor* pReceiver) {
    if (isJugemGroundedWithoutCloud() || (isNerve(JN(FallRecovery)) && MR::isLessEqualStep(this, 69))) {
        killJugemHeldTogezo();
        MR::zeroVelocity(this);
        MR::setVelocityBlowAttack(this, pSender, pReceiver, 36.0f, 32.0f, 4);
        setNerve(JN(BlowDown));
        MR::tryHitReaction(this, "\x90\x81\x82\xC1\x94\xF2\x82\xD1");
        return true;
    }
    return false;
}

bool Jugem::tryJugemDead() {
    if (!isNerve(JN(Dead))) {
        mCloud->tryDisappear();
        MR::tryHitReactionDead(this);
        setNerve(JN(Dead));
        return true;
    }
    return false;
}

// NONMATCHING: float scheduling of the ballistic solve; saved-fpr count differs
void Jugem::throwJugemHeldTogezo(const TVec3f& rTarget) {
    Togezo* pTogezo = mHoldTogezo;
    if (pTogezo != nullptr) {
        TVec3f trans(0.0f);
        MR::extractMtxTrans(mHeldTogezoMatrix.toMtxPtr(), &trans);
        TVec3f diff(rTarget);
        diff -= trans;
        f32 h = MR::vecKillElement(diff, mGravity, &diff);
        TVec3f vel(0.0f);
        f32 dist = PSVECMag(&diff);
        MR::normalize(&diff);
        f32 c = MR::cosDegree(cThrowAngle);
        f32 s = -MR::sinDegree(cThrowAngle);
        f32 t = c * c * (2.0f * ((h - dist * (s / c)) / (dist * dist)));
        f32 k = 0.8f / MR::fastSqrtf(t);
        f32 vUp = s * k;
        f32 vH = c * k;
        TVec3f up(mGravity);
        up *= vUp;
        vel.set(up);
        MR::vecScaleAdd(&vel, &diff, vH);
        pTogezo->releaseHold(vel);
        mHoldTogezo = nullptr;
    }
}

bool Jugem::tryJugemPickTogezo() {
    s32 nearest = -1;
    f32 minDist = 1000000.0f;
    for (s32 i = 0; i < mChildNum; i++) {
        if (MR::isDead(mChildObjs[i]) || mChildObjs[i]->isYoshiState()) {
            if (MR::isJudgedToClipFrustum(mChildPoses[i], 200.0f)) {
                if (MR::isDead(mChildObjs[i])) {
                    mChildRebirthTimers[i]++;
                    if (mChildRebirthTimers[i] >= 200) {
                        mChildObjs[i]->appearBound(mChildPoses[i], TVec3f(0.0f));
                        mChildRebirthTimers[i] = 0;
                    }
                }
            } else {
                f32 dist = MR::calcDistance(mChildObjs[i], this);
                if (dist < minDist) {
                    minDist = dist;
                    nearest = i;
                }
            }
        } else {
            mChildRebirthTimers[i] = 0;
        }
    }
    if (nearest >= 0) {
        LiveActor* pSpare = getJugemDeadChild();
        if (pSpare == nullptr) {
            return false;
        }
        mChildIndex = nearest;
        selectJugemChildSlot(mChildObjs[nearest]);
        mChildObjs[mChildIndex] = static_cast< Togezo* >(pSpare);
        setNerve(JN(PickTogezo));
        return true;
    }
    return false;
}

bool Jugem::killJugemHeldTogezo() {
    if (mHoldTogezo != nullptr) {
        mHoldTogezo->kill();
        return true;
    }
    return false;
}

// NONMATCHING: temporary vector stack layout / evaluation order differ
void Jugem::calcJugemPickupPosition(TVec3f* pOut, s32 index) {
    TVec3f pos(mChildPoses[index]);
    TVec3f grav(0.0f);
    MR::calcGravityVector(mChildObjs[index], pos, &grav, nullptr, 0);
    TVec3f up(-grav);
    TVec3f up2(up);
    up2 *= cHeight;
    pos += up2;
    TVec3f ref(0.0f);
    calcJugemFollowPosition(&ref, cHeight);
    TVec3f dir(pos);
    dir -= ref;
    MR::vecKillElement(dir, grav, &dir);
    if (MR::normalizeOrZero(&dir)) {
        TVec3f toMe(mPosition);
        toMe -= pos;
        dir = toMe;
        MR::vecKillElement(dir, grav, &dir);
        if (MR::normalizeOrZero(&dir)) {
            dir = mFrontVec;
            MR::normalizeOrZero(&dir);
        }
    }
    TVec3f up3(-dir);
    TVec3f up4(up3);
    up4 *= 1500.0f;
    pos += up4;
    const TVec3f& player = *MR::getPlayerPos();
    TVec3f diff(pos);
    diff -= player;
    f32 d = MR::vecKillElement(diff, *MR::getPlayerGravity(), &diff);
    if (PSVECMag(&diff) < 900.0f) {
        MR::normalize(&diff);
        TVec3f a(diff);
        a *= 900.0f;
        TVec3f b(*MR::getPlayerGravity());
        b *= d;
        TVec3f c(*MR::getPlayerPos());
        c += b;
        TVec3f e(c);
        e += a;
        pos = e;
    }
    *pOut = pos;
}

// NONMATCHING: vector stack slots and loop register allocation differ
void Jugem::calcJugemFollowPosition(TVec3f* pOut, f32 offset) {
    TVec3f nearest;
    s32 idx = findJugemNearestRailPoint(&nearest, this, *MR::getPlayerPos());
    *pOut = nearest;
    s32 arg = -1;
    MR::getRailPointArg0WithInit(this, idx, &arg);
    if (arg != 1) {
        s32 count = 0;
        TVec3f sum(0.0f);
        for (s32 i = 0; i < mChildNum; i++) {
            if (mChildObjs[i] != nullptr) {
                if (!MR::isDead(mChildObjs[i]) && !MR::isClipped(mChildObjs[i]) && !mChildObjs[i]->isYoshiState()) {
                    TVec3f vel(mChildObjs[i]->mGravity);
                    vel *= offset;
                    TVec3f pos(mChildObjs[i]->mPosition);
                    pos -= vel;
                    const TVec3f& player = *MR::getPlayerPos();
                    TVec3f diff(pos);
                    diff -= player;
                    if (!(PSVECMag(&diff) > cChildRange * cChildRangeScale)) {
                        sum += pos;
                        count++;
                    }
                }
            }
        }
        if (count == 1) {
            TVec3f t(*pOut);
            t += sum;
            TVec3f u(t);
            u *= 0.5f;
            *pOut = u;
        } else if (count > 0) {
            f32 scale = 1.0f / count;
            TVec3f t(sum);
            t *= scale;
            *pOut = t;
        }
        const TVec3f& player = *MR::getPlayerPos();
        TVec3f diff(*pOut);
        diff -= player;
        f32 d = MR::vecKillElement(diff, *MR::getPlayerGravity(), &diff);
        if (PSVECMag(&diff) < 900.0f) {
            MR::normalize(&diff);
            TVec3f a(diff);
            a *= 900.0f;
            TVec3f b(mGravity);
            b *= d;
            TVec3f c(*MR::getPlayerPos());
            c += b;
            TVec3f e(c);
            e += a;
            *pOut = e;
        }
    }
}

LiveActor* Jugem::getJugemDeadChild() {
    if (MR::isDead(mChildren[mChildCount])) {
        return mChildren[mChildCount];
    }
    return nullptr;
}

void Jugem::selectJugemChildSlot(LiveActor* pActor) {
    for (s32 i = 0; i < mChildNumPlusOne; i++) {
        if (mChildren[i] == pActor) {
            mChildCount = i;
            return;
        }
    }
}

// NONMATCHING: string-pool base register reuse / register allocation of the animation-name selection
void Jugem::updateJugemMoveAnimation() {
    TVec3f vel(mVelocity);
    TPos3f mtx;
    PSMTXCopy(getBaseMtx(), mtx.toMtxPtr());
    TVec3f zero(0.0f);
    MR::setMtxTrans(mtx.toMtxPtr(), zero.x, zero.y, zero.z);
    MR::calcLocalVec(&vel, vel, mtx.toMtxPtr());
    vel.y = 0.0f;
    const char* pName = nullptr;
    if (PSVECMag(&vel) < 8.0f) {
        if (MR::isBckPlaying(this, "MoveLeft")) {
            pName = "MoveEndLeft";
        } else if (MR::isBckPlaying(this, "MoveRight")) {
            pName = "MoveEndRight";
        } else if (MR::isBckPlaying(this, "MoveFront")) {
            pName = "MoveEndFront";
        } else if (MR::isBckPlaying(this, "MoveBack")) {
            pName = "MoveEndBack";
        } else {
            if (MR::isBckPlaying(this, "MoveEndLeft") || MR::isBckPlaying(this, "MoveEndRight") ||
                MR::isBckPlaying(this, "MoveEndFront") || MR::isBckPlaying(this, "MoveEndBack")) {
                return;
            }
            pName = "Move";
        }
        {
            MR::tryStartActionNotPlaying(this, pName);
            if (MR::fn_80008F00(mCloud, pName)) {
                MR::tryStartActionNotPlaying(mCloud, pName);
            }
        }
    } else {
        f32 angle = 57.29578f * MR::diffAngleSignedHorizontal(vel, TVec3f(0.0f, 0.0f, 1.0f), TVec3f(0.0f, 1.0f, 0.0f));
        if (-150.0f <= angle && angle < -30.0f) {
            pName = "MoveLeft";
        } else if (-30.0f <= angle && angle < 30.0f) {
            pName = "MoveFront";
        } else if (30.0f < angle && angle < 150.0f) {
            pName = "MoveRight";
        } else {
            pName = "MoveBack";
        }
        if (pName != nullptr) {
            f32 frame = MR::getBckFrame(this);
            if (MR::tryStartActionNotPlaying(this, pName)) {
                if (MR::fn_80008F00(mCloud, pName)) {
                    MR::tryStartActionNotPlaying(mCloud, pName);
                }
                MR::setAllAnimFrame(this, pName, frame);
                MR::setAllAnimFrame(mCloud, pName, frame);
            }
        }
    }
}

#pragma push
#pragma dont_inline on
void Jugem::updateJugemCloudSway() {
    if (++mHorizontalSwayStep >= 180) {
        mHorizontalSwayStep = 0;
    }
    if (++mVerticalSwayStep >= 90) {
        mVerticalSwayStep = 0;
    }
    if (isNerve(JN(Wait)) || isNerve(JN(Search)) || isNerve(JN(Follow)) || isNerve(JN(Approach)) || isNerve(JN(PickTogezo)) || isNerve(JN(MoveUp)) ||
        isNerve(JN(HitReaction)) || isNerve(JN(ThrowSign)) || isNerve(JN(Throw)) || isNerve(JN(ThrowEnd))) {
        mCloudSwayWeight += 0.1f;
        if (mCloudSwayWeight > 1.0f) {
            mCloudSwayWeight = 1.0f;
        }
    } else {
        mCloudSwayWeight -= 0.05f;
        if (mCloudSwayWeight < 0.0f) {
            mCloudSwayWeight = 0.0f;
        }
    }
    s32 yPeriod = 90;
    s32 xPeriod = 180;
    f32 y = sCloudSway.y * MR::sinDegree(360.0f * mVerticalSwayStep / yPeriod);
    mCloudSwayOffset = TVec3f(sCloudSway.x * -MR::cosDegree(360.0f * mHorizontalSwayStep / xPeriod), y, 0.0f);
    mCloudSwayOffset *= mCloudSwayWeight;
}


#pragma pop
bool Jugem::isJugemGroundActorValid() {
    if (mGroundActor != nullptr) {
        if (!MR::isDead(mGroundActor) && MR::isValidCollisionParts(mGroundActor)) {
            return true;
        }
        return false;
    }
    return false;
}

void Jugem::updateJugemGroundActor() {
    if (isNerve(JN(FallDownEaten)) || isNerve(JN(FallLand)) || isNerve(JN(FallRecovery))) {
        if (MR::isBindedGround(this)) {
            mGroundActor = MR::getGroundActor(this);
            return;
        }
        if (mGroundActor != nullptr && !MR::isDead(mGroundActor)) {
            if (!MR::isValidCollisionParts(mGroundActor)) {
                mGroundActor = nullptr;
            }
        } else {
            mGroundActor = nullptr;
        }
    } else {
        mGroundActor = nullptr;
    }
}

void Jugem::initBinder() {
    mBasePos = sBinderOffsetWait;
    mBinderRadius = 125.0f;
    MR::setBinderOffsetVec(this, &mBasePos, false);
    MR::setBinderRadius(this, mBinderRadius);
}

void Jugem::initBinderFall() {
    mBasePos = sBinderOffsetFall;
    mBinderRadius = 100.0f;
    MR::setBinderOffsetVec(this, &mBasePos, false);
    MR::setBinderRadius(this, mBinderRadius);
}

bool Jugem::calcRootJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    MR::addTransMtxLocal(pMtx->toMtxPtr(), mCloudSwayOffset);
    return true;
}

bool Jugem::calcWristJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    TPos3f mtx;
    PSMTXIdentity(mtx.toMtxPtr());
    MR::makeMtxTR(mtx.toMtxPtr(), sWristOffset, sWristRotate);
    PSMTXConcat(pMtx->toMtxPtr(), mtx.toMtxPtr(), mHeldTogezoMatrix.toMtxPtr());
    return false;
}

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
    MR::tryHitReaction(this, "\x8F\x6F\x8C\xBB");
}

void JugemCloud::kill() {
    LiveActor::kill();
}

void JugemCloud::control() {
    MR::startActionSound(this, "Float", static_cast< s32 >(100.0f * PSVECMag(&mHost->mVelocity)), -1, -1);
    if (!mIsAppear && MR::isActionEnd(this)) {
        kill();
    }
}

void JugemCloud::disappear() {
    mIsAppear = false;
    mCalcOwnMatrix = false;
    MR::startAction(this, "Disappear");
}

void JugemCloud::tryDisappear() {
    if (!MR::isDead(this) && mIsAppear) {
        disappear();
    }
}

bool JugemCloud::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return mJugem->receiveOtherMsg(msg, pSender, pReceiver);
}

Jugem::~Jugem() {}

JugemCloud::~JugemCloud() {}

const f32 cChildRange = 100.0f;
const f32 cChildRangeScale = 30.0f;
const f32 cReboundGravity = 1.3f;

const f32 cThrowAngle = 30.0f;

const f32 cHeight = 350.0f;
const f32 cRayLength = 1000.0f;

const f32 cFallSpeed = 27.0f;

extern const volatile s32 gJugemFollowDuration = 0;

namespace {
    const f32 sFollowRange = 350.0f;
}
