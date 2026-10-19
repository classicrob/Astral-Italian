#include "Game/Enemy/Gesso.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/EnemyUtilAll.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

namespace {
    // static const f32 sReceivePushPower = ;
    // static const f32 sAttenuatePushRate = ;
    // static const f32 sWaterJudgeOffset = ;
    // static const f32 sOutBoxSpeedUp = ;
    // static const f32 sOutBoxSpeedFront = ;
    // static const a32 sStepForAppear = ;
    // static const f32 sDriftSpeedMax = ;
    // static const f32 sWaitTurnRate = ;
    // static const s32 sCountToInitPos = ;
    // static const f32 sSearchRadiusIn = ;
    // static const f32 sSearchRadiusOut = ;
    // static const f32 sSearchTurnRate = ;
    // static const f32 sHighSpeedRadius = ;
    // static const f32 sWalkSpeedFront = ;
    // static const f32 sWalkSpeedUpMax = ;
    // static const f32 sWalkSpeedSide = ;
    // static const f32 sWalkSinEndDegree = ;
    // static const f32 sUpVecTurnRate = ;
    // static const f32 sSinkSpeedUpMax = ;
    // static const f32 sSinkSinEndDegree = ;
    // static const s32 sStepForSink = ;
    // static const s32 sAttackReaction = ;
    // static const s32 sStepForStopReaction = ;
    // static const s32 sStepForSearchOff = ;
    // static const s32 sPunchDownReaction = ;
    // static const s32 sStepForPunchDown = ;
    // static const f32 sComeBackSpeed = ;
    // static const f32 sJudgeForComeBackRadius = ;
    // static const s32 sStepForWalkCharge = ;
    static const s32 sStepForWalk = 43;
};  // namespace

// SMG2-only MR:: helper (unnamed in symbols.txt): *pDst = normalize(rPos - pActor->mPosition)
// SMG2-only MR:: helper (unnamed): *pDst = normalize(*MR::getPlayerPos() - pActor->mPosition)
// SMG2-only helper (unnamed, LiveActor area), called before kill() on defeat

namespace NrvGesso {
    NEW_NERVE(GessoNrvComeFromBox, Gesso, ComeFromBox);
    NEW_NERVE(GessoNrvWait, Gesso, Wait);
    NEW_NERVE(GessoNrvSearch, Gesso, Search);
    NEW_NERVE(GessoNrvWalkCharge, Gesso, WalkCharge);
    NEW_NERVE(GessoNrvWalk, Gesso, Walk);
    NEW_NERVE(GessoNrvSink, Gesso, Sink);
    NEW_NERVE(GessoNrvLostPlayer, Gesso, LostPlayer);
    NEW_NERVE(GessoNrvAttack, Gesso, Attack);
    NEW_NERVE(GessoNrvPunchDown, Gesso, PunchDown);
    NEW_NERVE(GessoNrvComeBack, Gesso, ComeBack);
    NEW_NERVE(GessoNrvDPDSwoon, Gesso, DPDSwoon);
    NEW_NERVE(GessoNrvRotate, Gesso, Rotate);

};  // namespace NrvGesso

inline bool Gesso::isMarioUp() const {
    TVec3f* pos = MR::getPlayerPos();
    TVec3f playerDist;
    playerDist.sub(*pos, mPosition);
    TVec3f upVec;
    upVec.negate(mGravity);
    return playerDist.dot(upVec) > 0.0f;
}

bool Gesso::isMarioLeft() const {
    TVec3f* pos = MR::getPlayerPos();
    TVec3f playerDist;
    playerDist.sub(*pos, mPosition);
    TVec3f sideVec;
    MR::calcSideVec(&sideVec, this);
    return playerDist.dot(sideVec) > 0.0f;
}

Gesso::Gesso(const char* pName)
    : LiveActor(pName), mScaleController(), mStateBindStarPointer(), mHomePos(0.0f, 0.0f, 0.0f), mFront(0.0f, 0.0f, 1.0f), mUp(0.0f, 1.0f, 0.0f),
      mTargetUp(0.0f, 1.0f, 0.0f), mHomeResetStep(), mPushVelocity(0.0f, 0.0f, 0.0f), mIsMarioLeft(), mIsHighSpeedMode() {
}

void Gesso::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    mHomePos = mPosition;
    MR::initActor(this, rIter, "Gesso", false);
    MR::connectToSceneEnemy(this);
    MR::initLightCtrl(this);
    initHitSensor(1);
    TVec3f sensorOffset;
    sensorOffset.x = 0.0f;
    sensorOffset.y = 0.0f;
    sensorOffset.z = 0.0f;
    MR::addHitSensor(this, "Body", 34 /* KILLER_TARGET_ENEMY */, 8, 100.0f, sensorOffset);
    initBinder(150.0f, 0.0f, 0);
    initEffectKeeper(2, nullptr, false);
    MR::addEffectHitNormal(this, nullptr);
    initSound(4, "Gesso", nullptr, TVec3f(0.0f));
    TVec3f pointerOffset;
    pointerOffset.x = 0.0f;
    pointerOffset.y = 0.0f;
    pointerOffset.z = 0.0f;
    MR::initStarPointerTarget(this, 100.0f, pointerOffset);
    mScaleController = new AnimScaleController(nullptr);
    mStateBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initShadowVolumeSphere(this, 90.0f);
    MR::calcFrontVec(&mFront, this);
    MR::calcUpVec(&mUp, this);
    mTargetUp.set(mUp);
    MR::addToAttributeGroupSearchTurtle(this);
    initNerve(GET_NERVE(Gesso, GessoNrvWait), 1);
    MR::initActorState(this, mStateBindStarPointer, GET_NERVE(Gesso, GessoNrvDPDSwoon), "SupportFreeze");
    if (MR::useStageSwitchSyncAppear(this, rIter)) {
        s32 initialBehavior;
        MR::getJMapInfoArg0WithInit(rIter, &initialBehavior);
        if (initialBehavior == 0) {
            setNerve(GET_NERVE(Gesso, GessoNrvComeFromBox));
        } else {
            setNerve(GET_NERVE(Gesso, GessoNrvWait));
        }

        makeActorDead();
    } else {
        makeActorAppeared();
    }
}

void Gesso::kill() {
    MR::emitEffect(this, "Death");
    LiveActor::kill();
}

// NONMATCHING: instruction order only (one addi scheduled earlier)
void Gesso::control() {
    mScaleController->updateNerve();
    if (tryDPDSwoon()) {
        return;
    }

    if (clipAndInitPos()) {
        return;
    }

    TVec3f playerDirection;
    TVec3f up;
    TVec3f offset;
    MR::calcDirToPlayer(&playerDirection, this);
    MR::turnVecToVecCosOnPlane(&mUp, mTargetUp, mFront, MR::cosDegree(0.5f));
    if (mVelocity.dot(mPushVelocity) < 0.0f) {
        const TVec3f& rVelocity = mVelocity;
        mVelocity.scaleAdd(-mPushVelocity.dot(rVelocity), mPushVelocity, rVelocity);
    }

    mVelocity.add(mPushVelocity);
    mPushVelocity *= 0.7f;
    if (MR::isNearZero(mPushVelocity)) {
        mPushVelocity.zero();
    }

    up.negate(mGravity);
    offset.scale(50.0f, up);
    if (!MR::isInWater(this, offset) && mVelocity.dot(up) > 0.0f) {
        const TVec3f& rVelocity = mVelocity;
        mVelocity.scaleAdd(-up.dot(rVelocity), up, rVelocity);
    }
}

void Gesso::calcAndSetBaseMtx() {
    TPos3f mtx;
    MR::makeMtxUpFrontPos(&mtx, mUp, mFront, mPosition);
    MR::setBaseTRMtx(this, mtx);
    MR::updateBaseScale(this, mScaleController);
}

void Gesso::exeComeFromBox() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FromBox");
        MR::startActionSound(this, "EmGessoSwimL");
        MR::invalidateHitSensors(this);
        mPosition.scaleAdd(-100.0f, mGravity, mPosition);
        mTargetUp.set(0.0f, 1.0f, 0.3f);
        MR::normalize(&mTargetUp);
        mUp.set(mTargetUp);
    }

    if (MR::isLessStep(this, 10)) {
        calcAndSetVelocity(0.0f, 10.0f, 5.0f);
    } else {
        mVelocity.zero();
    }

    if (MR::isActionEnd(this)) {
        mVelocity.zero();
        MR::validateHitSensors(this);
        setNerve(GET_NERVE(Gesso, GessoNrvWait));
    }
}

void Gesso::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        calcAndSetUpVecTarget(0.0f, 0.0f, 0.0f);
    }

    if (MR::isStep(this, 2)) {
        MR::invalidateClipping(this);
    }

    mVelocity.scale(1.5f * (MR::sinDegree(45.0f + (2.0f * getNerveStep()))), mGravity);
    MR::rotateVecDegree(&mFront, mGravity, 0.3f);
    if (MR::isNearPlayer(this, 1400.0f) && MR::isPlayerInWaterMode()) {
        MR::validateClipping(this);
        setNerve(GET_NERVE(Gesso, GessoNrvSearch));
    }
}

void Gesso::exeSearch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Search");
        MR::startActionSound(this, "EmGessoFind");
        calcAndSetUpVecTarget(0.0f, 0.0f, 0.0f);
    }

    mVelocity.scale(1.5f * (MR::sinDegree(45.0f + (2.0f * getNerveStep()))), mGravity);
    MR::turnDirectionToPlayerDegree(this, &mFront, 1.5f);
    if (MR::isFaceToPlayerHorizontalDegree(this, mFront, 1.5f) && MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Gesso, GessoNrvWalkCharge));
    }
}

void Gesso::exeWalkCharge() {
    if (MR::isFirstStep(this)) {
        if (tryChangeHighSpeedMode()) {
            MR::startAction(this, "WalkFast");
            mTargetUp.set(0.0f, 1.0f, 1.0f);
            MR::normalize(&mTargetUp);
        } else {
            MR::startAction(this, "Walk");
            mTargetUp.set(0.0f, 1.0f, 1.0f);
            MR::normalize(&mTargetUp);
        }
    }

    mVelocity.mult(0.995f);
    if (MR::isStep(this, ::sStepForWalk)) {
        mIsMarioLeft = isMarioLeft();
        setNerve(GET_NERVE(Gesso, GessoNrvWalk));
    }
}

void Gesso::exeWalk() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "EmGessoSwimL");
    }

    MR::turnDirectionToTargetDegree(this, &mFront, *MR::getPlayerPos(), 1.5f);
    if (calcWalkMove(getNerveStep())) {
        selectNextNerve();
    }
}

void Gesso::exeSink() {
    if (MR::isFirstStep(this)) {
        if (MR::isNearPlayer(this, 1000.0f)) {
            MR::startAction(this, "SinkFast");
        } else {
            MR::startAction(this, "Sink");
        }
    }

    MR::turnDirectionToTargetDegree(this, &mFront, *MR::getPlayerPos(), 1.5f);
    if (MR::isBindedGround(this)) {
        setNerve(GET_NERVE(Gesso, GessoNrvWalkCharge));
    } else if (calcSinkMove(getNerveStep())) {
        selectNextNerve();
    }
}

void Gesso::exeLostPlayer() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "CoolDown");
        calcAndSetUpVecTarget(0.0f, 0.0f, 0.0f);
    }

    mVelocity.scale(1.5f * (MR::sinDegree(45.0f + (2.0f * getNerveStep()))), mGravity);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Gesso, GessoNrvWait));
    }
}

void Gesso::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::startActionSound(this, "EmGessoInk");
        calcAndSetUpVecTarget(0.0f, 0.0f, 0.0f);
    }

    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(this, "Wait");
    }

    if (MR::isBckPlaying(this, "Attack")) {
        mVelocity.mult(0.9f);
    } else {
        mVelocity.scale(1.5f * (MR::sinDegree(45.0f + (2.0f * getNerveStep()))), mGravity);
    }

    if (MR::isStep(this, 240)) {
        setNerve(GET_NERVE(Gesso, GessoNrvWait));
    }
}

void Gesso::exePunchDown() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
        MR::stopScene(5);
        MR::startAction(this, "PunchDown");
        MR::startActionSound(this, "EmGessoHitPunch");
        MR::startBlowHitSound(this);
    }

    MR::turnDirectionToTargetDegree(this, &mFront, *MR::getPlayerPos(), 1.5f);
    if (MR::isStep(this, 20) || MR::isBinded(this)) {
        MR::startActionSound(this, "EmGessoDead");
        MR::emitItemHit(this);
        kill();
    }
}

void Gesso::exeComeBack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "CoolDown");
    }

    if (MR::isBckPlaying(this, "CoolDown")) {
        mVelocity.scale(1.5f * (MR::sinDegree(45.0f + (2.0f * getNerveStep()))), mGravity);
    } else {
        if (MR::isBckOneTimeAndStopped(this)) {
            MR::startAction(this, "Sink");
        }

        if (MR::isPlayerInWaterMode()) {
            setNerve(GET_NERVE(Gesso, GessoNrvWait));
        } else {
            MR::turnDirectionToTargetDegree(this, &mFront, *MR::getPlayerPos(), 1.5f);
            TVec3f stack_14;
            MR::fn_8000E960(&stack_14, this, &mHomePos);
            mVelocity.scale(3.0f, stack_14);
            if (MR::isNear(this, mHomePos, 100.0f)) {
                setNerve(GET_NERVE(Gesso, GessoNrvWait));
            }
        }
    }
}

void Gesso::exeRotate() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StarPiece");
        mVelocity.zero();
    }

    MR::startActionSound(this, "EmLvGessoStarPieceHit");
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Gesso, GessoNrvWait));
    }
}

void Gesso::exeDPDSwoon() {
    MR::updateActorStateAndNextNerve(this, GET_NERVE(Gesso, GessoNrvWait));
}

void Gesso::endDPDSwoon() {
    mStateBindStarPointer->kill();
}

void Gesso::knockOut(HitSensor* pSender, HitSensor* pReceiver) {
    TVec3f dir;
    MR::calcSensorDirectionNormalize(&dir, pSender, pReceiver);
    mVelocity.scale(30.0f, dir);
    MR::turnDirectionToTarget(this, &mFront, *MR::getSensorPos(pSender), -1.0f);
    setNerve(GET_NERVE(Gesso, GessoNrvPunchDown));
}

void Gesso::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(Gesso, GessoNrvPunchDown))) {
        return;
    }

    if (MR::isSensorEnemy(pSender) && MR::isSensorPlayer(pReceiver)) {
        if (isNerve(GET_NERVE(Gesso, GessoNrvDPDSwoon))) {
            MR::sendMsgPush(pReceiver, pSender);
        } else if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
            MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, nullptr);
            TVec3f dir;
            MR::calcSensorDirectionNormalize(&dir, pReceiver, pSender);
            mVelocity.scale(25.0f, dir);
            setNerve(GET_NERVE(Gesso, GessoNrvAttack));
        }
        return;
    }
    if (MR::isSensorYoshi(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool Gesso::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(Gesso, GessoNrvPunchDown))) {
        return false;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        setNerve(GET_NERVE(Gesso, GessoNrvRotate));
        return true;
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        knockOut(pSender, pReceiver);
        return true;
    }

    return false;
}

bool Gesso::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(Gesso, GessoNrvPunchDown))) {
        return false;
    }

    if (MR::isMsgExplosionAttack(msg)) {
        knockOut(pSender, pReceiver);
        return true;
    }

    if (MR::isSensorType(pSender, 0x28)) {
        knockOut(pSender, pReceiver);
        return false;
    }

    return false;
}

bool Gesso::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pSender) || MR::isSensorMapObj(pSender)) {
        TVec3f stack_08;
        stack_08.sub(mPosition, MR::getSensorHost(pSender)->mPosition);
        MR::normalizeOrZero(&stack_08);
        mPushVelocity.scaleAdd(0.2f, mPushVelocity, stack_08);
        return true;
    }

    return false;
}

void Gesso::selectNextNerve() {
    if (!MR::isPlayerInWaterMode()) {
        setNerve(GET_NERVE(Gesso, GessoNrvComeBack));
    } else if (!MR::isNearPlayer(this, 2400.0f)) {
        setNerve(GET_NERVE(Gesso, GessoNrvLostPlayer));
    } else if (isMarioUp()) {
        setNerve(GET_NERVE(Gesso, GessoNrvWalkCharge));
    } else if (!isNerve(GET_NERVE(Gesso, GessoNrvSink))) {
        setNerve(GET_NERVE(Gesso, GessoNrvSink));
    }
}

bool Gesso::clipAndInitPos() {
    if (!MR::isNearPlayer(this, 10000.0f) || MR::isJudgedToClipFrustum(mPosition, MR::getSensorRadius(getSensor(nullptr)))) {
        if (mHomeResetStep >= 300) {
            MR::setPosition(this, mHomePos);
            setNerve(GET_NERVE(Gesso, GessoNrvWait));
            MR::validateClipping(this);
            mHomeResetStep = 0;
            return true;
        }

        mHomeResetStep++;
        return false;
    }

    mHomeResetStep = 0;
    return false;
}

bool Gesso::calcWalkMove(s32 step) {
    f32 verticalSpeed, sideSign, angle;

    if (mIsMarioLeft) {
        sideSign = 1.0f;
    } else {
        sideSign = -1.0f;
    }

    angle = 0.990099f * step;

    if (mIsHighSpeedMode) {
        verticalSpeed = MR::cosDegree(angle);
        TVec3f* vel = &mVelocity;
        TVec3f upVec;
        TVec3f sideVec;
        verticalSpeed = 7.0f * verticalSpeed;
        MR::calcUpVec(&upVec, this);
        MR::calcSideVec(&sideVec, this);
        mVelocity.scale(verticalSpeed, upVec);
        vel->scaleAdd((sideSign * 3.0f) / 5.0f, sideVec, *vel);
        vel->scaleAdd(2.3f, mFront, *vel);
    } else {
        verticalSpeed = MR::cosDegree(angle);
        TVec3f* vel = &mVelocity;
        TVec3f upVec;
        TVec3f sideVec;
        verticalSpeed = 7.0f * verticalSpeed;
        MR::calcUpVec(&upVec, this);
        MR::calcSideVec(&sideVec, this);
        mVelocity.scale(verticalSpeed, upVec);
        vel->scaleAdd(sideSign * 3.0f, sideVec, *vel);
        vel->scaleAdd(2.3f, mFront, *vel);
    }

    if (MR::isFirstStep(this)) {
        if (mIsHighSpeedMode) {
            calcAndSetUpVecTarget((3.0f * sideSign) / 5.0f, MR::cosDegree(angle) * 7.0f, 4.6f);
        } else {
            calcAndSetUpVecTarget((3.0f * sideSign) / 2.0f, MR::cosDegree(angle) * 7.0f, 4.6f);
        }
    }

    if (angle >= 70.0f) {
        mTargetUp.set(0.0f, 1.0f, 1.0f);
        MR::normalize(&mTargetUp);
    }

    return angle >= 100.0f;
}

bool Gesso::calcSinkMove(s32 step) {
    f32 verticalSpeed, sideSign, angle;

    if (mIsMarioLeft) {
        sideSign = 1.0f;
    } else {
        sideSign = -1.0f;
    }

    angle = 100.0f + 3.0f * step;
    if (angle >= 180.0f) {
        angle = 180.0f;
    }

    verticalSpeed = MR::cosDegree(angle) * 3.0f;
    TVec3f* vel = &mVelocity;
    TVec3f upVec;
    TVec3f sideVec;
    MR::calcUpVec(&upVec, this);
    MR::calcSideVec(&sideVec, this);

    mVelocity.scale(verticalSpeed, upVec);
    vel->scaleAdd(sideSign * 3.0f, sideVec, *vel);
    vel->scaleAdd(2.3f, mFront, *vel);
    mTargetUp.set(0.0f, 1.0f, 1.0f);
    MR::normalize(&mTargetUp);

    return angle >= 180.0f;
}

void Gesso::calcAndSetVelocity(f32 f1, f32 f2, f32 f3) {
    TVec3f* vel = &mVelocity;
    TVec3f sideVec;
    TVec3f upVec;
    MR::calcUpVec(&upVec, this);
    MR::calcSideVec(&sideVec, this);
    mVelocity.scale(f2, upVec);
    vel->scaleAdd(f1, sideVec, *vel);
    vel->scaleAdd(f3, mFront, *vel);
}

void Gesso::calcAndSetUpVecTarget(f32 f1, f32 f2, f32 f3) {
    if (f1 == 0.0f && f2 == 0.0f && f3 == 0.0f) {
        mTargetUp.set(0.0f, 1.0f, 0.0f);
        return;
    }

    mTargetUp.set(f1, f2, f3);
    MR::normalize(&mTargetUp);
    return;
}

bool Gesso::tryChangeHighSpeedMode() {
    bool change = false;
    if (MR::isNearPlayer(this, 1000.0f) && !MR::isPlayerInWaterMode()) {
        change = true;
    }

    mIsHighSpeedMode = change;
    return change;
}

bool Gesso::tryDPDSwoon() {
    if (isNerve(GET_NERVE(Gesso, GessoNrvDPDSwoon))) {
        return false;
    }

    if (isNerve(GET_NERVE(Gesso, GessoNrvPunchDown))) {
        return false;
    }

    if (isNerve(GET_NERVE(Gesso, GessoNrvRotate))) {
        return false;
    }

    if (!mStateBindStarPointer->tryStartPointBind()) {
        return false;
    }

    setNerve(GET_NERVE(Gesso, GessoNrvDPDSwoon));
    return true;
}

Gesso::~Gesso() {
}
