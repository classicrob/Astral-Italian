#include "Game/Enemy/Nyoropon.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JointController.hpp"
#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <JSystem/J2DGraph/J2DPicture.hpp>
#include <revolution/mtx.h>

extern const f32 cSegmentCount;
extern const f32 cBodyHeight;
extern const f32 cBodyCount;
extern const f32 cOne;
extern const f32 cHalf;
extern const f32 cWanderWidth;
extern const f32 cWanderHeightScale;
extern const f32 cSegmentFraction;
extern const f32 cStretchMin;
__declspec(section ".sdata2") extern volatile s32 cWavePeriod;

// Joint names of the 7 body joints, and the joint index -> body index table filled by init.
static const char* sJointNames[7] = {"HeadPos", "Body5", "Body4", "Body3", "Body2", "Body1", "AllRoot"};
static s32 sJointIndexTable[7] = {-1, -1, -1, -1, -1, -1, -1};

// Scratch matrices/vectors shared by all instances.
static TVec3f sBodyRotation(0.0f, 0.0f, 90.0f);
static TVec3f sTmpVec(0.0f, -20.0f, 0.0f);
static TPos3f sRotateMtx;
static TPos3f sJointInvMtx[7];

namespace NrvNyoropon {
    NEW_NERVE(NyoroponNrvSleep, Nyoropon, Sleep);
    NEW_NERVE(NyoroponNrvWait, Nyoropon, Wait);
    NEW_NERVE(NyoroponNrvFind, Nyoropon, Find);
    NEW_NERVE(NyoroponNrvRecover, Nyoropon, Recover);
    NEW_NERVE(NyoroponNrvStarPiece, Nyoropon, StarPiece);
    NEW_NERVE(NyoroponNrvDownStretch, Nyoropon, DownStretch);
    NEW_NERVE(NyoroponNrvSpin, Nyoropon, Spin);
    NEW_NERVE(NyoroponNrvSearchLight, Nyoropon, SearchLight);
    NEW_NERVE(NyoroponNrvDownSpin, Nyoropon, DownSpin);
};  // namespace NrvNyoropon

// SupportFreeze nerve executor.
void Nyoropon::exeSupportFreeze() {
    if (MR::updateActorState(this)) {
        if (mStretchGoal > 0.0f) {
            setNerve(&NrvNyoropon::NyoroponNrvRecover::sInstance);
            return;
        }
        setNerve(&NrvNyoropon::NyoroponNrvWait::sInstance);
    }
}

namespace NrvNyoropon {
    NEW_NERVE(NyoroponNrvSupportFreeze, Nyoropon, SupportFreeze);
};  // namespace NrvNyoropon

#define NRV(n) (&NrvNyoropon::NyoroponNrv##n::sInstance)

Nyoropon::Nyoropon(const char* pName)
    : LiveActor(pName), mJointCtrls(nullptr), mAnimScaleCtrl(nullptr), mBindStar(nullptr), mHead(nullptr), mFront(0.0f, 0.0f, 1.0f),
      mTarget(0.0f, 5.0f, 0.0f), mStretchRate(0.0f), mStretch(0.0f), mStretchGoal(0.0f), mStretchStart(0.0f), mVel(0.0f, 0.0f, 0.0f),
      mPrevVel(0.0f, 0.0f, 0.0f), mAmp(1.0f), mFrame(0), mSavedNerve(nullptr), mInitialized(false), mIsAsleep(false) {
    mMtxs = new TPos3f[7];
    mPoints = new TVec3f[7];
    mDirs = new TVec3f[7];
    mLengths = new f32[7];
    for (s32 i = 0; i < 7; i++) {
        PSMTXIdentity(mMtxs[i].toMtxPtr());
        mPoints[i] = TVec3f(0.0f, 0.0f, 0.0f);
        mDirs[i] = TVec3f(0.0f, 1.0f, 0.0f);
        mLengths[i] = 0.0f;
    }
}

// NONMATCHING: vector temporary stack order differs
void Nyoropon::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "NyoroponBody", false);
    for (s32 i = 0; i < 7; i++) {
        sJointIndexTable[MR::getJointIndex(this, sJointNames[i])] = i;
    }
    MR::calcUpVec(&mGravity, this);
    mGravity.negate();
    TVec3f offset(mGravity);
    offset *= 50.0f;
    TVec3f pos(mPosition);
    pos += offset;
    mPosition = pos;
    LiveActor::calcAndSetBaseMtx();
    MR::calcFrontVec(&mFront, this);
    mHead = new PartsModel(this, "ニョロポン頭", "NyoroponHead", MR::getJointMtx(this, "HeadPos"), 0x13, false);
    mHead->initWithoutIter();
    MR::getJMapInfoArg7NoInit(rIter, &mIsAsleep);
    initBodyJointControllers();
    setFaceSleep();
    initNerve(NRV(Sleep), 1);
    MR::addToAttributeGroupSearchTurtle(this);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mAnimScaleCtrl->setParamTight();
    mBindStar = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mBindStar, NRV(SupportFreeze), "SupportFreeze");
    makeActorAppeared();
    mHead->appear();
    calcAnim();
    MR::makeMtxRotate(sRotateMtx.toMtxPtr(), 0.0f, 0.0f, 90.0f);
    TPos3f rootInv;
    MtxPtr pRoot = MR::getJointMtx(this, "AllRoot");
    PSMTXInverse(pRoot, rootInv.toMtxPtr());
    for (s32 i = 6; i >= 0; i--) {
        PSMTXConcat(rootInv.toMtxPtr(), MR::getJointMtx(this, sJointNames[i]), sJointInvMtx[i].toMtxPtr());
        PSMTXInverse(sJointInvMtx[i].toMtxPtr(), sJointInvMtx[i].toMtxPtr());
        PSMTXInverse(MR::getJointMtx(this, sJointNames[i]), rootInv.toMtxPtr());
    }
    mInitialized = true;
}

void Nyoropon::makeActorAppeared() {
    mAmp = 1.0f;
    mStretchRate = 0.0f;
    mStretch = 0.0f;
    mStretchGoal = 0.0f;
    resetBodySegments();
    LiveActor::makeActorAppeared();
    mHead->makeActorAppeared();
    MR::startAction(mHead, "Wait");
    setFaceNormal();
}

// NONMATCHING: equivalent sensor guards compile with a different branch layout.
void Nyoropon::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Light")) {
        return;
    }
    if (!MR::isEqualSensor(pSender, this, "HeadReceive")) {
        if (MR::isSensorPlayer(pReceiver)) {
            if (isLightTargetNerve()) {
                if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
                    MR::tryStartActionNotPlaying(mHead, "AttackSuccess");
                }
            } else {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else if (MR::isSensorEnemy(pReceiver) || MR::isSensorMapObj(pReceiver)) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool Nyoropon::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Light")) {
        if (MR::isMsgSearchlightAttack(msg) && isHittableNerve()) {
            const TVec3f& rRecvPos = MR::getSensorHostPos(pReceiver);
            TVec3f diff(MR::getSensorHostPos(pSender));
            diff -= rRecvPos;
            if (diff.dot(mFront) > 0.0f) {
                if (isStretchedNerve()) {
                    setNerve(NRV(DownStretch));
                    return true;
                }
                setNerve(NRV(SearchLight));
                return true;
            }
        }
        if (MR::isMsgSupportTicoSpin(msg) && isHittableNerve()) {
            if (isStretchedNerve()) {
                setNerve(NRV(DownSpin));
            } else {
                setNerve(NRV(Spin));
            }
            return true;
        }
        return false;
    }
    if (MR::isMsgJetTurtleAttack(msg) && isHittableNerve()) {
        setNerve(NRV(Spin));
        return true;
    }
    if (!MR::isEqualSensor(pReceiver, this, "HeadReceive")) {
        return MR::isMsgStarPieceReflect(msg);
    }
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceAttack(msg) && isHittableNerve()) {
        setNerve(NRV(StarPiece));
        return true;
    }
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && isHittableNerve()) {
        if (isStretchedNerve()) {
            setNerve(NRV(DownSpin));
        } else {
            setNerve(NRV(Spin));
        }
        return true;
    }
    return false;
}

void Nyoropon::control() {
    s32 numJoints = 7;
    s32 recoverFrames = 120;
    if (isNerve(NRV(Spin)) || isNerve(NRV(StarPiece)) || isNerve(NRV(DownStretch)) || isNerve(NRV(SearchLight))) {
        mStretchRate += 1.0f / numJoints;
    } else if (!isNerve(NRV(Recover)) && !isNerve(NRV(SupportFreeze))) {
        mStretchRate -= 1.0f / recoverFrames;
    }
    mStretchRate = MR::clamp(mStretchRate, 0.0f, 1.0f);
    mAnimScaleCtrl->updateNerve();
    if (isIdleNerve() && mBindStar->tryStartPointBind()) {
        setFaceSleep();
        mSavedNerve = mSpine->getCurrentNerve();
        setNerve(NRV(SupportFreeze));
        return;
    }
    if (mIsAsleep) {
        if (MR::isStageStateScenarioOpeningCamera()) {
            setFaceNormal();
            return;
        }
        if (isNerve(NRV(Sleep))) {
            setFaceSleep();
        }
    }
}

bool Nyoropon::isHittableNerve() const {
    if (isNerve(NRV(Sleep)) || isNerve(NRV(Wait)) || isNerve(NRV(Find)) || isNerve(NRV(Recover)) || isNerve(NRV(StarPiece)) ||
        isNerve(NRV(SupportFreeze))) {
        return true;
    }
    return false;
}

bool Nyoropon::isLightTargetNerve() const {
    if (isNerve(NRV(Sleep)) || isNerve(NRV(Wait)) || isNerve(NRV(Find)) || isNerve(NRV(Recover))) {
        return true;
    }
    return false;
}

bool Nyoropon::isIdleNerve() const {
    if (isNerve(NRV(Sleep)) || isNerve(NRV(Wait)) || isNerve(NRV(Find))) {
        return true;
    }
    return false;
}

bool Nyoropon::isStretchedNerve() const {
    if (isNerve(NRV(StarPiece)) || isNerve(NRV(Recover)) ||
        (isNerve(NRV(SupportFreeze)) && mStretchRate > 0.0f)) {
        return true;
    }
    return false;
}

f32 Nyoropon::calcBodyStretchScale() const {
    return cStretchMin + ((cOne - cStretchMin) * (cOne - mStretch));
}

void Nyoropon::exeSleep() {
    if (MR::isFirstStep(this)) {
        setFaceSleep();
        MR::tryStartActionNotPlaying(this, "Wait");
    }
    if (MR::isBckOneTimeAndStopped(mHead)) {
        MR::tryStartActionNotPlaying(mHead, "Wait");
    }
    updateWanderTarget();
    updateBodySegments(0.3f, 0.3f, 3.5f);
    f32 dist = MR::calcDistanceToPlayer(this);
    if (dist < 1100.0f) {
        setNerve(NRV(Find));
        return;
    }
    s32 range = 2000;
    if (dist < range) {
        setNerve(NRV(Wait));
    }
}

void Nyoropon::exeWait() {
    if (MR::isFirstStep(this)) {
        setFaceNormal();
        MR::tryStartActionNotPlaying(this, "Wait");
    }
    if (MR::isBckOneTimeAndStopped(mHead)) {
        MR::tryStartActionNotPlaying(mHead, "Wait");
    }
    updateWanderTarget();
    updateBodySegments(0.3f, 0.3f, 3.5f);
    if (MR::calcDistanceToPlayer(this) < 1100.0f) {
        setNerve(NRV(Find));
        return;
    }
    playIdleSound();
}

// NONMATCHING: target reloads duplicate 1.0f literals where the compiler reuses a float register
void Nyoropon::exeFind() {
    if (MR::isFirstStep(this)) {
        setFaceAttack();
        MR::tryStartActionNotPlaying(this, "Wait");
        MR::tryStartActionNotPlaying(mHead, "Find");
    }
    if (MR::isBckOneTimeAndStopped(mHead)) {
        MR::tryStartActionNotPlaying(mHead, "Wait");
    }
    updatePlayerTarget();
    updateBodySegments(1.0f, 1.0f, 1.0f);
    if (MR::isGreaterStep(this, 30) && MR::calcDistanceToPlayer(this) > 1100.0f) {
        setNerve(NRV(Wait));
    }
    playIdleSound();
    if (PSVECMag(&mVel) > 2.0f) {
        MR::startActionSound(mHead, "Stretch", (s32)(100.0f * PSVECMag(&mVel)), -1, -1);
    }
}

void Nyoropon::exeRecover() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Recover");
        MR::startAction(mHead, "Recover");
        mStretchStart = mStretch;
    }
    mStretch -= MR::calcNerveEaseInValue(this, 50, 0.0f, 0.01f);
    mStretchGoal = mStretch;
    if (MR::isActionEnd(this)) {
        mStretch = 0.0f;
        if (MR::calcDistanceToPlayer(this) < 1100.0f) {
            setNerve(NRV(Find));
            return;
        }
        setNerve(NRV(Wait));
        return;
    }
    updateWanderTarget();
    updateBodySegments(1.0f, 1.0f, 3.5f);
}

void Nyoropon::exeStarPiece() {
    if (MR::isFirstStep(this)) {
        if (mStretchGoal >= 0.9f) {
            setNerve(NRV(DownStretch));
            return;
        }
        mStretchStart = mStretch;
        mStretchGoal += 1.0f;
        MR::startAction(this, "Starpiece");
        MR::startAction(mHead, "Starpiece");
    }
    if (MR::isBckOneTimeAndStopped(mHead)) {
        MR::startAction(mHead, "Wait");
    }
    updateStretchTransition(15);
    if (MR::isGreaterStep(this, 120)) {
        setNerve(NRV(Recover));
        return;
    }
    updateWanderTarget();
    updateBodySegments(10.0f, 0.0f, 3.5f);
}

void Nyoropon::exeDownStretch() {
    if (MR::isFirstStep(this)) {
        mStretchStart = mStretch;
        mStretchGoal = 1.0f;
        MR::startAction(this, "Down");
        MR::startAction(mHead, "Down");
    }
    updateStretchTransition(15);
    updateWanderTarget();
    updateBodySegments(10.0f, 0.0f, 3.5f);
    if (MR::isActionEnd(this)) {
        ScoreAttackFunction::score100Points(mHead);
        MR::appearCoinPop(this, mHead->mPosition, 1);
        MR::tryHitReactionDead(this);
        MR::tryHitReactionDead(mHead);
        kill();
    }
}

void Nyoropon::exeSpin() {
    if (MR::isFirstStep(this)) {
        mStretchStart = mStretch;
        mStretchGoal = 1.0f;
        MR::startAction(this, "Spin");
        MR::startAction(mHead, "Spin");
    }
    updateStretchTransition(15);
    updateWanderTarget();
    updateBodySegments(10.0f, 0.0f, 3.5f);
    if (MR::isActionEnd(this)) {
        ScoreAttackFunction::score100Points(mHead);
        MR::emitItemHitAtPos(this, &mHead->mPosition);
        MR::tryHitReactionDead(this);
        MR::tryHitReactionDead(mHead);
        kill();
    }
}

void Nyoropon::exeSearchLight() {
    if (MR::isFirstStep(this)) {
        mStretchStart = mStretch;
        mStretchGoal = 1.0f;
        MR::startAction(this, "SearchLight");
        MR::startAction(mHead, "SearchLight");
    }
    updateStretchTransition(15);
    updateWanderTarget();
    updateBodySegments(10.0f, 0.0f, 3.5f);
    if (MR::isActionEnd(this)) {
        ScoreAttackFunction::score100Points(mHead);
        MR::emitItemHitAtPos(this, &mHead->mPosition);
        MR::tryHitReactionDead(this);
        MR::tryHitReactionDead(mHead);
        kill();
    }
}

void Nyoropon::exeDownSpin() {
    if (MR::isFirstStep(this)) {
        mStretchStart = mStretch;
        mStretchGoal = 1.0f;
        MR::startAction(this, "Down");
        MR::startAction(mHead, "Down");
    }
    updateStretchTransition(15);
    updateWanderTarget();
    updateBodySegments(10.0f, 0.0f, 3.5f);
    if (MR::isActionEnd(this)) {
        ScoreAttackFunction::score100Points(mHead);
        MR::emitItemHitAtPos(this, &mHead->mPosition);
        MR::tryHitReactionDead(this);
        MR::tryHitReactionDead(mHead);
        kill();
    }
}

void Nyoropon::calcAndSetBaseMtx() {
    MR::updateBaseScale(this, mAnimScaleCtrl);
    if (!MR::isDead(this)) {
        for (s32 i = 0; i < 7; i++) {
            mJointCtrls[i]->registerCallBack();
        }
    }
}

void Nyoropon::setFaceSleep() {
    if (!MR::isBtpPlaying(mHead, "Sleep")) {
        MR::startBtp(mHead, "Sleep");
        MR::tryHitReaction(mHead, "顔変化：睡眠");
    }
}

void Nyoropon::setFaceNormal() {
    if (!MR::isBtpPlaying(mHead, "Normal")) {
        MR::startBtp(mHead, "Normal");
        MR::tryHitReaction(mHead, "顔変化：通常");
    }
}

void Nyoropon::setFaceAttack() {
    if (!MR::isBtpPlaying(mHead, "Search")) {
        MR::startBtp(mHead, "Search");
        MR::tryHitReaction(mHead, "顔変化：攻撃");
    }
}

// NONMATCHING: joint controller and loop registers differ
void Nyoropon::initBodyJointControllers() {
    mJointCtrls = new JointController*[7];
    for (s32 i = 0; i < 7; i++) {
        JointControlDelegator< Nyoropon >::func calcFunc = &Nyoropon::calcJointMatrix;
        const char* pName = sJointNames[i];
        JointControlDelegator< Nyoropon >* pCtrl = new JointControlDelegator< Nyoropon >(this, calcFunc, nullptr);
        MR::setJointControllerParam(pCtrl, this, pName);
        mJointCtrls[i] = pCtrl;
    }
}

void Nyoropon::resetBodySegments() {
    TPos3f mtx;
    PSMTXIdentity(mtx.toMtxPtr());
    MR::makeMtxRotate(mtx.toMtxPtr(), 0.0f, 0.0f, 0.0f);
    TVec3f pos(0.0f);
    TVec3f dir(0.0f, 1.0f, 0.0f);
    f32 len = (cBodyHeight / cBodyCount) - cOne;
    for (s32 i = 6; i >= 0; i--) {
        mPoints[i] = pos;
        mDirs[i] = dir;
        mLengths[i] = len;
        mMtxs[i] = mtx;
        MR::setMtxTrans(mMtxs[i].toMtxPtr(), pos.x, pos.y, pos.z);
        f32 segmentLength = mLengths[i];
        TVec3f step(dir);
        step *= segmentLength;
        pos += step;
    }
    mVel = TVec3f(0.0f);
    mPrevVel = TVec3f(0.0f);
}

// NONMATCHING: float register allocation and product scheduling differ
void Nyoropon::updateWanderTarget() {
    f32 half = cHalf * cWanderWidth;
    TVec3f goal;
    goal.x = 0.0f;
    goal.y = cWanderHeightScale * cBodyHeight;
    goal.z = 0.0f;
    s32 period = 500;
    f32 phase = 2.0f * (((f32)(((s32)(cHalf * period) + getNerveStep()) % period) / period) - cHalf);
    f32 wave;
    if (phase < 0.0f) {
        wave = 1.0f + (2.0f * phase);
    } else {
        wave = 1.0f - (2.0f * phase);
    }
    goal.x += half * wave;
    MR::vecBlend(&mTarget, mTarget, goal, 0.3f);
}

// NONMATCHING: stack slot order of the TVec3f temporaries only
void Nyoropon::updatePlayerTarget() {
    TVec3f playerPos(*MR::getPlayerPos());
    TVec3f local(0.0f, 0.0f, 0.0f);
    MR::calcLocalVec(&local, playerPos, getBaseMtx());
    local.z = 0.0f;
    if (local.y < 150.0f) {
        local.y = 150.0f;
    }
    if (PSVECMag(&local) < 1.0f) {
        local.y = 100.0f;
    }
    TVec3f diff(local);
    diff -= sTmpVec;
    f32 d = PSVECMag(&diff);
    if (d < 500.0f) {
        if (d < 0.5f) {
            local = mTarget;
        } else {
            MR::normalize(&diff);
            TVec3f a(diff);
            a *= 500.0f;
            TVec3f b(sTmpVec);
            b += a;
            local = b;
        }
    } else {
        MR::normalize(&diff);
        TVec3f a(diff);
        a *= 500.0f;
        TVec3f b(sTmpVec);
        b += a;
        local = b;
    }
    if (local.y < 150.0f) {
        local.y = 150.0f;
    }
    TVec3f cur(mTarget);
    cur -= sTmpVec;
    local -= sTmpVec;
    MR::vecBlendSphere(&cur, cur, local, 0.03f);
    TVec3f res(cur);
    res += sTmpVec;
    mTarget = res;
}

// NONMATCHING: vector temporary stack slots, saved float registers, and arithmetic scheduling differ
void Nyoropon::updateBodySegments(f32 pushA, f32 pushB, f32 ampScale) {
    mPrevVel = mVel;
    TVec3f head(mTarget);
    f32 scale = cSegmentFraction * cBodyHeight * calcBodyStretchScale();
    mVel *= 0.92f;
    TVec3f d = head - mPoints[0];
    f32 dist = PSVECMag(&d);
    if (dist > 0.5f) {
        d *= 1.0f / dist;
        TVec3f perp(-d.y, d.x, 0.0f);
        if (mTarget.x < 0.0f) {
            perp = -perp;
        }
        f32 k = MR::clamp(MR::normalize(dist, 0.0f, 100.0f), 0.0f, 1.0f);
        mVel += d * 0.7f * k * pushA;
        mVel += perp * 0.2f * k * pushB;
    }
    f32 n = MR::normalize(mPoints[0].y, 0.0f, 200.0f);
    mVel.y += MR::clamp(1.0f - n, 0.0f, 1.0f) * 0.1f;
    mPoints[0] += mVel;
    TVec3f seg = mPoints[0] - mPoints[6];
    f32 nn = MR::normalize(PSVECMag(&seg), 0.0f, 500.0f);
    f32 slack = 1.0f - nn;
    f32 shift = slack * (5.0f * -MR::sign(mTarget.x));
    for (s32 i = 1; i < 7; i++) {
        mPoints[i].x += shift;
    }
    head = mPoints[0];
    for (s32 i = 1; i < 7; i++) {
        TVec3f e = head - mPoints[i];
        f32 l = PSVECMag(&e);
        if (l > 0.5f) {
            e *= 1.0f / l;
            mPoints[i] = head - e * scale;
        }
        head = mPoints[i];
    }
    for (s32 k2 = 6; k2 >= 2; k2--) {
        TVec3f d2 = mPoints[k2 - 2] - mPoints[k2];
        f32 m2 = PSVECMag(&d2);
        f32 nb = MR::normalize(m2, 0.0f, 2.0f * scale);
        f32 bendRate = 1.0f - nb;
        MR::normalizeOrZero(&d2);
        TVec3f perp2(-d2.y, d2.x, 0.0f);
        f32 bend = (1.0f - (cSegmentFraction * (f32)k2)) * (100.0f * bendRate);
        f32 hh = (0.5f * m2) + (0.5f * bend);
        f32 sq = (scale * scale) - (hh * hh);
        f32 s = MR::fastSqrtf(sq);
        TVec3f mid = mPoints[k2] + d2 * hh;
        TVec3f toK = mid - mPoints[k2 - 1];
        mPoints[k2 - 2] += d2 * bend;
        f32 sg = MR::sign(perp2.dot(toK));
        mPoints[k2 - 1] = mid - perp2 * s * sg;
    }
    for (s32 i = 1; i < 7; i++) {
        mDirs[i] = mPoints[i - 1] - mPoints[i];
        mLengths[i] = PSVECMag(&mDirs[i]);
        mDirs[i] *= 1.0f / mLengths[i];
    }
    mDirs[0] = mDirs[1];
    mPoints[0] = mPoints[1] + mDirs[1] * scale;
    for (s32 i = 0; i < 7; i++) {
        mPoints[i] -= mPoints[6];
    }
    TVec3f dir5(mPoints[5]);
    dir5 -= mPoints[6];
    MR::normalizeOrZero(&dir5);
    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::clampVecAngleDeg(&dir5, up, 30.0f);
    TVec3f nv = mPoints[6] + dir5 * (cBodyHeight / cSegmentCount);
    TVec3f off = nv - mPoints[5];
    for (s32 i = 0; i < 6; i++) {
        mPoints[i] += off;
    }
    for (s32 i = 1; i < 7; i++) {
        mDirs[i] = mPoints[i - 1] - mPoints[i];
        mLengths[i] = PSVECMag(&mDirs[i]);
        mDirs[i] *= 1.0f / mLengths[i];
    }
    mDirs[0] = mDirs[1];
    mPoints[0] = mPoints[1] + mDirs[1] * scale;
    updateBodyWaveMatrices(ampScale);
}

// NONMATCHING: vector array stack order, register allocation, and arithmetic scheduling differ
void Nyoropon::updateBodyWaveMatrices(f32 ampScale) {
    TVec3f up(0.0f, 0.0f, 1.0f);
    f32 segmentFraction = cSegmentFraction;
    f32 step = cBodyHeight * segmentFraction;
    TVec3f d = mTarget - mPoints[0];
    f32 n = MR::clamp(MR::normalize(PSVECMag(&d), 0.0f, 200.0f), 0.0f, 1.0f);
    f32 amp = ampScale * (1.0f - n);
    f32 diff = amp - mAmp;
    if (__fabsf(diff) <= 0.01f) {
        mAmp = amp;
    }
    f32 ampStep = 0.01f * MR::sign(diff);
    s32 frame = mFrame;
    s32 period = cWavePeriod;
    mFrame = frame + 1;
    f32 phase = (360.0f * (f32)frame) / (f32)period;
    mAmp += ampStep;
    if (mFrame > period) {
        mFrame = 0;
    }
    TVec3f out[7];
    TVec3f outDir[7];
    for (s32 i = 0; i < 7; i++) {
        TVec3f next(i == 6 ? mPoints[i] : mPoints[i + 1]);
        TVec3f prev(i == 0 ? mPoints[i] : mPoints[i - 1]);
        TVec3f dv(prev);
        dv -= next;
        MR::normalize(&dv);
        J2DPicture::swap(dv.x, dv.y);
        dv.y = -dv.y;
        f32 t1 = 1.0f - (segmentFraction * (f32)i);
        f32 ph = phase + (360.0f * (t1 * 0.8f));
        f32 w = MR::clamp(MR::normalize(t1, 0.0f, 0.75f), 0.0f, 1.0f);
        f32 s = JMASinDegree(ph);
        f32 amount = mAmp * (w * (30.0f * s)) * calcBodyStretchScale();
        out[i] = mPoints[i];
        out[i] += dv * amount;
    }
    for (s32 i = 1; i < 7; i++) {
        outDir[i] = out[i - 1] - out[i];
        mLengths[i] = PSVECMag(&outDir[i]);
        outDir[i] *= 1.0f / mLengths[i];
    }
    outDir[0] = outDir[1];
    out[0] = out[1] + outDir[1] * step;
    for (s32 i = 6; i >= 0; i--) {
        mLengths[i] = step;
    }
    for (s32 i = 0; i < 7; i++) {
        MR::makeMtxUpFrontPos(&mMtxs[i], outDir[i], up, out[i]);
    }
}

bool Nyoropon::calcJointMatrix(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    if (!mInitialized) {
        return false;
    }
    s32 idx = sJointIndexTable[rInfo.mJoint->getJntNo()];
    if (idx == 6) {
        mRootMtx = *pMtx;
        PSMTXInverse(mRootMtx.toMtxPtr(), mRootInvMtx.toMtxPtr());
        return false;
    }
    TVec3f x, y, z;
    MR::extractMtxXYZDir(pMtx->toMtxPtr(), &x, &y, &z);
    f32 sz = PSVECMag(&z);
    f32 sy = PSVECMag(&y);
    TVec3f scl(PSVECMag(&x), sy, sz);
    f32 inv = 1.0f - mStretchRate;
    TPos3f tmp;
    PSMTXCopy(pMtx->toMtxPtr(), tmp.toMtxPtr());
    MR::preScaleMtx(tmp.toMtxPtr(), 1.0f / scl.x, 1.0f / scl.y, 1.0f / scl.z);
    TPos3f blend;
    blend.concat(mMtxs[idx], sRotateMtx);
    PSMTXConcat(getBaseMtx(), blend.toMtxPtr(), blend.toMtxPtr());
    MR::blendMtx(tmp.toMtxPtr(), blend.toMtxPtr(), inv, pMtx->toMtxPtr());
    MR::preScaleMtx(pMtx->toMtxPtr(), scl);
    PSMTXCopy(pMtx->toMtxPtr(), mRootMtx.toMtxPtr());
    return true;
}

// NONMATCHING: float multiply operands use the opposite register order
bool Nyoropon::updateStretchTransition(s32 step) {
    f32 rate = MR::calcNerveEaseOutRate(this, step);
    mStretch = mStretchStart + ((mStretchGoal - mStretchStart) * rate);
    if (mStretch >= 1.0f) {
        mStretch = 1.0f;
        return true;
    }
    return false;
}

void Nyoropon::playIdleSound() {
    if (mFrame == 0) {
        MR::startActionSound(mHead, "WaitA", (s32)(100.0f * PSVECMag(&mVel)), -1, -1);
    } else if (mFrame == 90) {
        MR::startActionSound(mHead, "WaitB", (s32)(100.0f * PSVECMag(&mVel)), -1, -1);
    }
}

Nyoropon::~Nyoropon() {
}

const f32 cBodyHeight = 500.0f;
const f32 cBodyCount = 7.0f;
const f32 cOne = 1.0f;
const f32 cHalf = 0.5f;
const f32 cWanderWidth = 300.0f;
const f32 cWanderHeightScale = 1.3f;
const f32 cSegmentFraction = 0.16666667f;
const f32 cStretchMin = 0.05f;
__declspec(section ".sdata2") volatile s32 cWavePeriod = 180;

const f32 cSegmentCount = 6.0f;
