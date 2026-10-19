#include "Game/Boss/KingTossin.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"


namespace {
    extern const f32 sHalf;
    extern const f32 sRollingSpeed;
    extern const f32 sRollingSpeedStep;

    struct MoveParam {
        f32 _0;
        f32 _4;
        f32 _8;
        f32 _C;
    };

    const MoveParam sChaseParam = {2.1f, 1.0f, 0.87f, 0.7f};
    const MoveParam sTurnToPlayerParam = {0.0f, 2.0f, 0.95f, 1.5f};
    const MoveParam sRunRailParam = {1.5f, 1.0f, 0.85f, 2.0f};
    const MoveParam sStopParam = {0.0f, 1.0f, 0.85f, 3.0f};
    const MoveParam sDamageParam = {0.0f, 1.0f, 0.98f, 1.0f};
    const MoveParam sRunParam = {1.6f, 2.0f, 0.95f, 0.7f};
    const MoveParam sRollingEndParam = {0.0f, 2.0f, 0.99f, 0.7f};
    const MoveParam sRollingEndChanceParam = {0.0f, 2.0f, 0.99f, 0.7f};
    const MoveParam sSlidingParam = {0.5f, 2.0f, 1.0f, 4.0f};
    const MoveParam sTurnParam = {0.0f, 2.0f, 0.95f, 3.5f};
    const MoveParam sRollingLv2Param = {1.6f, 2.0f, 0.95f, 0.7f};

};  // namespace

// Brake tuning: zero consts the target loads from .sbss2 instead of folding them.
const volatile s32 sBrakeStartStep = 0;
const volatile f32 sBrakeEndSpeed = 0.0f;

namespace NrvKingTossin {
    NEW_NERVE(KingTossinNrvStandby, KingTossin, Standby);
    NEW_NERVE(KingTossinNrvWaitStart, KingTossin, WaitStart);
    NEW_NERVE(KingTossinNrvDemoOpening, KingTossin, DemoOpening);
    NEW_NERVE(KingTossinNrvDemoPowerUp, KingTossin, DemoPowerUp);
    NEW_NERVE(KingTossinNrvDemoDown, KingTossin, DemoDown);
    NEW_NERVE(KingTossinNrvAttack, KingTossin, Attack);
    NEW_NERVE(KingTossinNrvDamage, KingTossin, Damage);
    NEW_NERVE(KingTossinNrvDamageSecond, KingTossin, DamageSecond);
    NEW_NERVE(KingTossinNrvChanceSliding, KingTossin, ChanceSliding);
    NEW_NERVE(KingTossinNrvChanceEnd, KingTossin, ChanceEnd);
    NEW_NERVE(KingTossinNrvRun, KingTossin, Run);
    NEW_NERVE(KingTossinNrvRollingLv1, KingTossin, Rolling);
    NEW_NERVE(KingTossinNrvRollingLv3, KingTossin, Rolling);
    NEW_NERVE(KingTossinNrvRollingLv2, KingTossin, RollingLv2);
    NEW_NERVE(KingTossinNrvSliding, KingTossin, Sliding);
    NEW_NERVE(KingTossinNrvDownRecover, KingTossin, DownRecover);
    NEW_NERVE(KingTossinNrvGuard, KingTossin, Guard);
    NEW_NERVE(KingTossinNrvGuardReturn, KingTossin, GuardReturn);
    NEW_NERVE(KingTossinNrvRunRail, KingTossin, RunRail);
    NEW_NERVE(KingTossinNrvRollingStart, KingTossin, RollingStart);
    NEW_NERVE(KingTossinNrvRollingEnd, KingTossin, RollingEnd);
    NEW_NERVE(KingTossinNrvRollingEndChance, KingTossin, RollingEnd);
    NEW_NERVE(KingTossinNrvRollingAttack, KingTossin, RollingAttack);
    NEW_NERVE(KingTossinNrvSpinReaction, KingTossin, SpinReaction);
    NEW_NERVE(KingTossinNrvSpinReactionChance, KingTossin, SpinReaction);
    NEW_NERVE(KingTossinNrvTurn, KingTossin, Turn);
    NEW_NERVE(KingTossinNrvSearch, KingTossin, Search);
    NEW_NERVE(KingTossinNrvChase, KingTossin, Chase);
    NEW_NERVE(KingTossinNrvBrake, KingTossin, Brake);
    NEW_NERVE(KingTossinNrvTurnToPlayer, KingTossin, TurnToPlayer);
};  // namespace NrvKingTossin

namespace {
    TVec3f sCameraTargetOffset(0.0f, 0.0f, 0.0f);
};  // namespace

namespace KingTossinUtil {
    const TVec3f& getTargetPos() {
        if (MR::isPlayerInRush()) {
            return MR::getCurrentRushActor()->mPosition;
        }

        return *MR::getPlayerPos();
    }

    void calcBlendDirToTarget(TVec3f* pDir, const LiveActor* pActor, const TVec3f& rFront, f32 rate) {
        TVec3f dir(0.0f);
        MR::calcVecToTargetPosH(&dir, pActor, getTargetPos(), nullptr);

        if (MR::isNearZero(dir, 0.001f)) {
            dir = rFront;
        }

        if (rFront.dot(dir) < 0.0f) {
            dir = rFront;
        }

        MR::vecBlend(&dir, rFront, dir, rate);
        *pDir = dir;
    }

    bool tryEmitAngrySmoke(LiveActor* pActor) {
        if (MR::isEffectValid(pActor, "AngrySmoke")) {
            return false;
        }

        MR::emitEffect(pActor, "AngrySmoke");
        return true;
    }

    bool tryDeleteAngrySmoke(LiveActor* pActor) {
        if (!MR::isEffectValid(pActor, "AngrySmoke")) {
            return false;
        }

        MR::deleteEffect(pActor, "AngrySmoke");
        return true;
    }
};  // namespace KingTossinUtil

using namespace KingTossinUtil;

KingTossin::KingTossin(const char* pName)
    : LiveActor(pName), mJointCtrl(nullptr), mFront(0.0f, 0.0f, 0.0f), mMoveDir(0.0f, 0.0f, 0.0f), mSlideSpeed(0.0f), mLife(4),
      mRollCount(0), mTimer(0), mPowerStarId(-1), mIsTargetInSight(false), mCameraInfo(nullptr), mCameraTarget(0.0f, 0.0f, 0.0f),
      mOpeningCameraInfo(nullptr), mDownCameraInfo(nullptr), mPowerUpCameraInfo(nullptr) {
    mPlayerMtx.identity();
    mStartMtx.identity();
}

void KingTossin::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    MR::calcFrontVec(&mFront, this);
    initNerve(&NrvKingTossin::KingTossinNrvWaitStart::sInstance, 0);

    bool useCamera = false;
    MR::getJMapInfoArg0WithInit(rIter, &useCamera);

    if (useCamera) {
        MR::initActorCamera(this, rIter, &mCameraInfo);
        MR::declareCameraRegisterVec(this, 1, &mCameraTarget);
    }

    MR::getJMapInfoArg1WithInit(rIter, &mPowerStarId);

    if (mPowerStarId != -1) {
        MR::declarePowerStar(this, mPowerStarId);
    }

    MR::initFur(this);
    MR::initUseStageSwitchReadA(this, rIter);
    mOpeningCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initAnimCamera(this, mOpeningCameraInfo, "DemoOpening");
    mDownCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initAnimCamera(this, mDownCameraInfo, "DemoDown");
    mPowerUpCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initAnimCamera(this, mDownCameraInfo, "DemoPowerUp");
    mStartMtx.set(getBaseMtx());
    mJointCtrl = new ActorJointCtrl(this);
    makeActorAppeared();
}

void KingTossin::kill() {
    if (mPowerStarId != -1) {
        MR::requestAppearPowerStar(this);
    }

    if (mCameraInfo != nullptr) {
        MR::endActorCamera(this, mCameraInfo, false, -1);
    }

    LiveActor::kill();
}

void KingTossin::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront);
    mJointCtrl->setCallBackFunction();
}

void KingTossin::control() {
    updateCameraTarget();
    mJointCtrl->update();
}

// NONMATCHING: conditional branch scheduling.
void KingTossin::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isEnableAttack()) {
        return;
    }

    if (!MR::isSensorPlayerOrRide(pReceiver)) {
        return;
    }

    if (!MR::isSensorEnemyAttack(pSender)) {
        return;
    }

    if (isRolling()) {
        if (MR::isEqualSensor(pSender, this, "RollingAttack") && MR::sendMsgEnemyAttackStrongOrPush(pReceiver, pSender)) {
            mRollCount = 0;
            MR::tryHitReaction(this, "回転攻撃");
            setNerve(&NrvKingTossin::KingTossinNrvRollingAttack::sInstance);
        }
    } else {
        if (MR::isEqualSensor(pSender, this, "Attack") && isFrontToSensor(pSender)) {
            MR::sendMsgPush(pReceiver, pSender);
            return;
        }

        if (MR::sendMsgEnemyAttackStrongOrPush(pReceiver, pSender)) {
            deleteBrakeSmoke();
            mRollCount = 0;
            setNerve(&NrvKingTossin::KingTossinNrvAttack::sInstance);
            MR::tryHitReaction(this, "攻撃");
        }
    }
}

bool KingTossin::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }

    if (!isEnableReceiveAttack()) {
        return false;
    }

    if (isRolling()) {
        if (!MR::isEqualSensor(pReceiver, this, "RollingAttack")) {
            return false;
        }

        if (!MR::isMsgRockAttack(msg)) {
            return false;
        }

        if (MR::sendMsgEnemyAttackStrongOrPush(MR::getSensorHost(pSender)->getSensor("Body"), pReceiver)) {
            MR::tryHitReaction(this, "回転攻撃");
            deleteBrakeSmoke();
            mRollCount = 0;
            setNerve(&NrvKingTossin::KingTossinNrvRollingAttack::sInstance);
            return true;
        }
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        if (!MR::isEqualSensor(pReceiver, this, "Body")) {
            return false;
        }

        if (!isFrontToSensor(pReceiver)) {
            return false;
        }

        if (isNerve(&NrvKingTossin::KingTossinNrvChanceSliding::sInstance)) {
            if (!MR::isAnyAnimPlaying(this, "Chance")) {
                return false;
            }

            MR::tryHitReaction(this, "スピン反応[チャンス中]");
            setNerve(&NrvKingTossin::KingTossinNrvSpinReactionChance::sInstance);
            return true;
        }

        MR::tryHitReaction(this, "スピン反応");
        setNerve(&NrvKingTossin::KingTossinNrvSpinReaction::sInstance);
        return true;
    }

    if (!MR::isMsgRockAttack(msg)) {
        return false;
    }

    if (MR::isEqualSensor(pReceiver, this, "Body")) {
        TVec3f velocity(mFront);
        if (MR::reflectVelocityH(this, pReceiver, pSender, 1.0f)) {
            velocity = mVelocity;
            MR::normalizeOrZero(&velocity);
        }

        velocity *= 100.0f;
        MR::setVelocity(this, velocity);

        if (mLife == 4 || mLife == 1) {
            setNerve(&NrvKingTossin::KingTossinNrvDamageSecond::sInstance);
        } else {
            setNerve(&NrvKingTossin::KingTossinNrvDamage::sInstance);
        }

        MR::sendMsgIsRushEnable(MR::getSensorHost(pSender)->getSensor("Body"), pReceiver);
        mRollCount = 0;
        deleteBrakeSmoke();
    } else if (MR::isEqualSensor(pReceiver, this, "Head")) {
        if (!MR::sendMsgEnemyAttackStrongOrPush(MR::getSensorHost(pSender)->getSensor("Body"), pReceiver)) {
            return false;
        }

        MR::tryHitReaction(this, "ガード");
        deleteBrakeSmoke();
        mRollCount = 0;
        setNerve(&NrvKingTossin::KingTossinNrvGuard::sInstance);
        return true;
    }

    return false;
}

void KingTossin::exeWaitStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::hideModel(this);
        MR::stopStageBgmForDemo();
    }

    if (!MR::isValidSwitchA(this) || MR::isOnSwitchA(this)) {
        if (MR::tryStartDemoMarioPuppetable(this, "DemoOpening")) {
            MR::showModel(this);
            MR::overlayWithPreviousScreen(2);
            MR::startAnimCameraTargetSelf(this, mOpeningCameraInfo, "DemoOpening", 0, false, 1.0f);
            setNerve(&NrvKingTossin::KingTossinNrvDemoOpening::sInstance);
        }
    }
}

void KingTossin::exeDemoOpening() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "DemoOpening");
        MR::stopStageBgmForDemo();
        mPlayerMtx.set(MR::getPlayerBaseMtx());
        TPos3f mtx;
        mtx.identity();
        MR::findNamePos("キングトッシンデモ開始位置", mtx);
        MR::setPlayerBaseMtx(mtx);
        MR::startBckPlayer("BattleWait", (const char*)nullptr);
        mJointCtrl->endFaceCtrl(-1);
    }

    if (MR::tryEndDemoAndAnimCamera(this, mOpeningCameraInfo, "DemoOpening", 0)) {
        mJointCtrl->startFaceCtrl(-1);
        resetPosition();

        if (mCameraInfo != nullptr) {
            MR::startActorCameraTargetPlayer(this, mCameraInfo, 0);
        }

        MR::startAction(this, "Wait");
        setNerve(getWaitNerve());
    }
}

void KingTossin::exeDemoPowerUp() {
    if (MR::isFirstStep(this)) {
        mJointCtrl->endFaceCtrl(-1);
        MR::startAction(this, "DemoPowerUp");
        MR::hidePlayer();
        resetPosition();
    }

    if (MR::tryEndDemoAndAnimCamera(this, mPowerUpCameraInfo, "DemoPowerUp", 0)) {
        mJointCtrl->startFaceCtrl(-1);
        MR::showPlayer();
        MR::startAction(this, "Wait");

        if (mCameraInfo != nullptr) {
            MR::startActorCameraTargetPlayer(this, mCameraInfo, 0);
        }

        MR::emitItemBossDamage(this);
        setNerve(getWaitNerve());
    }
}

void KingTossin::exeDemoDown() {
    if (MR::isFirstStep(this)) {
        mJointCtrl->endFaceCtrl(-1);
        MR::startAction(this, "DemoDown");
        MR::setPlayerBaseMtx(mPlayerMtx);
        MR::hidePlayer();
        MR::deleteEffect(this, "AngrySmoke");
    }

    if (MR::isGreaterEqualStep(this, 329)) {
        MR::invalidateShadowAll(this);
    }

    if (MR::endDemoAndAnimCameraIfActionEnd(this, mDownCameraInfo, "DemoDown", 0)) {
        mJointCtrl->startFaceCtrl(-1);
        MR::showPlayer();
        kill();
    }
}

void KingTossin::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::zeroVelocity(this);
        mJointCtrl->endFaceCtrl(-1);
    }

    if (MR::isActionEnd(this)) {
        mJointCtrl->startFaceCtrl(-1);
        setNerve(getWaitNerve());
    } else if (mLife == 1) {
        MR::startActionSound(this, "Accell", -1, -1, -1);
    }
}

void KingTossin::exeDamage() {
    if (MR::isFirstStep(this)) {
        mLife--;
        MR::tryHitReaction(this, "ダメージ");
        mMoveDir = mFront;
        MR::startAction(this, "BackSideDamage");
        MR::emitItemBossDamage(this);

        TVec3f velocity(0.0f);
        TVec3f up(0.0f);
        MR::calcUpVec(&up, this);
        velocity = up * 10.0f;
        velocity += mMoveDir * 30.0f;
        MR::setVelocity(this, velocity);
        MR::startActionSound(this, "DamageHitSound", -1, -1, -1);
    }

    if (MR::isGreaterEqualStep(this, 5) && MR::isOnGround(this)) {
        MR::killVelocityVertical(this);
    }

    MR::vecKillElement(mMoveDir, mGravity, &mMoveDir);
    if (MR::normalizeOrZero(&mMoveDir)) {
        mMoveDir = mFront;
    }

    MR::moveAndTurnToDirection(this, &mFront, mMoveDir, sDamageParam._0, sDamageParam._4, sDamageParam._8, sDamageParam._C);

    if (MR::isActionEnd(this)) {
        MR::zeroVelocity(this);
        setNerve(&NrvKingTossin::KingTossinNrvDownRecover::sInstance);
    }
}

void KingTossin::exeDamageSecond() {
    if (MR::isFirstStep(this)) {
        mLife--;
        mMoveDir = mFront;
        MR::startAction(this, "BackSideDamageSecond");
        MR::tryHitReaction(this, "ダメージ[デモ移行]");
        tryDeleteAngrySmoke(this);

        if (mLife > 0) {
            MR::startActionSound(this, "DamageHitSound", -1, -1, -1);
        } else {
            MR::startActionSound(this, "DamageLastHitSound", -1, -1, -1);
        }

        if (mLife <= 0) {
            MR::stopStageBGM(10);
        }
    }

    TVec3f dir(mMoveDir);
    MR::moveAndTurnToDirection(this, &mFront, dir, sStopParam._0, sStopParam._4, sStopParam._8, 0.0f);
    MR::attenuateVelocity(this, sStopParam._8);
    MR::addVelocityToGravityOrGround(this, sStopParam._4);

    if (PSVECMag(&mVelocity) <= 3.0f) {
        MR::zeroVelocity(this);
    }

    if (MR::isActionEnd(this)) {
        MR::zeroVelocity(this);

        if (mLife <= 0) {
            if (MR::tryStartDemoMarioPuppetable(this, "DemoDown")) {
                setNerve(&NrvKingTossin::KingTossinNrvDemoDown::sInstance);
                MR::startAnimCameraTargetSelf(this, mDownCameraInfo, "DemoDown", 0, false, 1.0f);
            }
        } else if (mLife == 3) {
            if (MR::tryStartDemoMarioPuppetable(this, "DemoPowerUp")) {
                setNerve(&NrvKingTossin::KingTossinNrvDemoPowerUp::sInstance);
                MR::startAnimCameraTargetSelf(this, mPowerUpCameraInfo, "DemoPowerUp", 0, false, 1.0f);
                MR::overlayWithPreviousScreen(2);
            }
        }
    }
}

void KingTossin::exeChanceSliding() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ChanceSliding");
        mSlideSpeed = PSVECMag(&mVelocity);
        mRollCount = 0;
        MR::emitEffect(this, "BrakeSmokeL");
        MR::emitEffect(this, "BrakeSmokeR");
        mJointCtrl->endFaceCtrl(-1);
    }

    MR::vecKillElement(mMoveDir, mGravity, &mMoveDir);
    if (MR::normalizeOrZero(&mMoveDir)) {
        mMoveDir = mFront;
    }

    s32 start = sBrakeStartStep;
    s32 end = start + 60;
    f32 speed = MR::getEaseInValueWithNormalize(getNerveStep(), start, end, mSlideSpeed, sBrakeEndSpeed);
    TVec3f dir(mVelocity);
    MR::normalizeOrZero(&dir);
    mVelocity.set(dir * speed);
    MR::moveAndTurnToDirection(this, &mFront, mMoveDir, 0.0f, 2.0f, 1.0f, 0.0f);

    if (MR::isStep(this, end)) {
        deleteBrakeSmoke();
        MR::startAction(this, "Chance");
        MR::startActionSound(this, "ChanceVoice", -1, -1, -1);
    }

    if (MR::isGreaterEqualStep(this, start + 180)) {
        setNerve(&NrvKingTossin::KingTossinNrvChanceEnd::sInstance);
    }
}

void KingTossin::exeChanceEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ChanceEnd");
    }

    if (MR::isActionEnd(this)) {
        if (mLife == 1) {
            tryEmitAngrySmoke(this);
        }

        setNerve(getWaitNerve());
    }
}

// NONMATCHING: floating-point register allocation.
void KingTossin::exeRun() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Run");
        MR::calcVecToTargetPosH(&mMoveDir, this, getTargetPos(), nullptr);
        f32 speed = 15.0f + 3.0f * mRollCount;
        MR::setVelocity(this, mMoveDir * speed);
    }

    TVec3f dir(mMoveDir);
    f32 distance = MR::calcDistance(this, getTargetPos());

    if (isTargetInSight(90.0f, 1700.0f)) {
        f32 rate = MR::getEaseInValueWithNormalize(distance, 1100.0f, 1700.0f, 0.0f, 1.0f);
        calcBlendDirToTarget(&mMoveDir, this, mFront, rate);
    } else {
        calcBlendDirToTarget(&mMoveDir, this, mFront, 1.0f);
    }

    MR::vecKillElement(mMoveDir, mGravity, &mMoveDir);
    if (MR::normalizeOrZero(&mMoveDir)) {
        mMoveDir = mFront;
    }

    MR::moveAndTurnToDirection(this, &mFront, mMoveDir, sRunParam._0, sRunParam._4, sRunParam._8 + -0.0425f, sRunParam._C);

    if (isTargetInSight(30.0f, 1700.0f)) {
        MR::zeroVelocity(this);
        setNerve(&NrvKingTossin::KingTossinNrvRollingStart::sInstance);
    } else if (mLife == 1) {
        MR::startActionSound(this, "Accell", -1, -1, -1);
    }
}

void KingTossin::exeRolling() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RollingLoop");
        mIsTargetInSight = false;
        MR::calcVecToTargetPosH(&mMoveDir, this, getTargetPos(), nullptr);
        f32 speed = 15.0f + 3.0f * mRollCount;
        MR::setVelocity(this, mMoveDir * speed);
    }

    MR::startActionSound(this, "Rolling", -1, -1, -1);

    if (mLife == 1 && MR::isGreaterStep(this, 40)) {
        MR::startActionSound(this, "Accell", -1, -1, -1);
    }

    TVec3f dir(mMoveDir);
    f32 distance = MR::calcDistance(this, getTargetPos());
    f32 rate = MR::getEaseInValueWithNormalize(distance, 1100.0f, 1700.0f, 0.0f, 1.0f);
    calcBlendDirToTarget(&mMoveDir, this, mFront, rate);

    MR::vecKillElement(mMoveDir, mGravity, &mMoveDir);
    if (MR::normalizeOrZero(&mMoveDir)) {
        mMoveDir = mFront;
    }

    MR::moveAndTurnToDirection(this, &mFront, mMoveDir, sRunParam._0, sRunParam._4, sRunParam._8, sRunParam._C);

    if (tryEndTargetInSight()) {
        if (isNerve(&NrvKingTossin::KingTossinNrvRollingLv1::sInstance)) {
            setNerve(&NrvKingTossin::KingTossinNrvRollingEnd::sInstance);
        } else {
            mRollCount++;
            setNerve(&NrvKingTossin::KingTossinNrvRollingEnd::sInstance);
        }
    } else if (MR::isGreaterEqualStep(this, 120) && !isTargetInSight(30.0f, 100000.0f)) {
        setNerve(&NrvKingTossin::KingTossinNrvRollingEnd::sInstance);
    }
}

// NONMATCHING: temporary stack slots and integer register allocation.
void KingTossin::exeRollingLv2() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RollingLoop");
        mIsTargetInSight = false;
        MR::calcVecToTargetPosH(&mMoveDir, this, getTargetPos(), nullptr);
        f32 speed = sRollingSpeed + 3.0f * sRollingSpeedStep;
        MR::setVelocity(this, mMoveDir * speed);
        mTimer = 0;
        mRollCount = 0;
    }

    MR::startActionSound(this, "Rolling", -1, -1, -1);

    if (mLife == 1 && MR::isGreaterStep(this, 40)) {
        MR::startActionSound(this, "Accell", -1, -1, -1);
    }

    if (mTimer > 0) {
        mTimer--;
    } else {
        TVec3f dir(mMoveDir);
        f32 distance = MR::calcDistance(this, getTargetPos());
        f32 rate = MR::getEaseInValueWithNormalize(distance, 1100.0f, 1700.0f, 0.0f, 1.0f);
        calcBlendDirToTarget(&mMoveDir, this, mFront, rate);
    }

    MR::vecKillElement(mMoveDir, mGravity, &mMoveDir);
    if (MR::normalizeOrZero(&mMoveDir)) {
        mMoveDir = mFront;
    }

    MR::moveAndTurnToDirection(this, &mFront, mMoveDir, sRollingLv2Param._0, sRollingLv2Param._4, sRollingLv2Param._8, sRollingLv2Param._C);

    if (tryEndTargetInSight()) {
        mRollCount++;
        mIsTargetInSight = false;
        mTimer = 60;
        TVec3f up = -mGravity;
        PSVECCrossProduct(&up, &mMoveDir, &mMoveDir);
        MR::rotateVecDegree(&mMoveDir, mGravity, 45.0f * MR::getRandom(0L, 5L));
        MR::normalizeOrZero(&mMoveDir);

        if (mRollCount >= 2) {
            if (mLife == 2) {
                setNerve(&NrvKingTossin::KingTossinNrvRollingEndChance::sInstance);
                return;
            }

            setNerve(&NrvKingTossin::KingTossinNrvRollingEnd::sInstance);
        }
    }
}

void KingTossin::exeSliding() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Sliding");
        mMoveDir = mFront;
        mIsTargetInSight = false;
    }

    TVec3f dir(0.0f);
    MR::calcVecToTargetPosH(&dir, this, getTargetPos(), nullptr);
    f32 cos = MR::cosDegree(30.0f * sHalf);

    if (mFront.dot(dir) > cos) {
        MR::zeroVelocity(this);
        mIsTargetInSight = true;
        setNerve(&NrvKingTossin::KingTossinNrvRollingStart::sInstance);
        return;
    }

    s32 endStep = 30;
    f32 t = f32(getNerveStep()) / f32(endStep);
    if (t > 1.0f) {
        t = 1.0f;
    }

    f32 rate = 1.0f - 0.04f * MR::getEaseOutValue(t, 0.0f, 1.0f, 1.0f);
    MR::moveAndTurnToTarget(this, &mFront, getTargetPos(), sSlidingParam._0, sSlidingParam._4, rate, sSlidingParam._C);
}

void KingTossin::exeDownRecover() {
    if (MR::isFirstStep(this)) {
        if (mLife == 1) {
            MR::startAction(this, "DownRecoverAngry");
        } else {
            MR::startAction(this, "DownRecover");
        }

        MR::calcDirToTarget(&mMoveDir, this, &getTargetPos());
    }

    MR::moveAndTurnToDirection(this, &mFront, mMoveDir, sStopParam._0, sStopParam._4, sStopParam._8, sStopParam._C);

    if (mLife == 1 && MR::getBckFrame(this) > 32.0f) {
        tryEmitAngrySmoke(this);
    }

    if (mLife == 1 && MR::getBckFrame(this) > 40.0f) {
        MR::startActionSound(this, "Accell", -1, -1, -1);
    }

    if (MR::isActionEnd(this)) {
        setNerve(getWaitNerve());
    }
}

void KingTossin::exeGuard() {
    if (MR::isFirstStep(this)) {
        mJointCtrl->endFaceCtrl(-1);
        MR::startAction(this, "Guard");
        MR::zeroVelocity(this);
        MR::shakeCameraStrong();
        MR::tryRumblePadMiddle(this, 0);
    }

    if (MR::isActionEnd(this)) {
        setNerve(&NrvKingTossin::KingTossinNrvGuardReturn::sInstance);
    }
}

void KingTossin::exeGuardReturn() {
    if (MR::isStep(this, 180)) {
        MR::startAction(this, "GuardReturn");
    }

    if (MR::isActionEnd(this)) {
        mJointCtrl->startFaceCtrl(-1);

        if (mLife == 1) {
            tryEmitAngrySmoke(this);
        }

        if (MR::isActionEnd(this)) {
            setNerve(getWaitNerve());
        }
    }

    if (mLife == 1) {
        MR::startActionSound(this, "Accell", -1, -1, -1);
    }
}

void KingTossin::exeRunRail() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Run");
    }

    MR::moveAndTurnAlongRail(this, &mFront, nullptr, 1000.0f, sRunRailParam._0, sRunRailParam._4, sRunRailParam._8, sRunRailParam._C);
}

void KingTossin::exeRollingStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RollingStart");
    }

    MR::moveAndTurnToTarget(this, &mFront, getTargetPos(), sTurnParam._0, sTurnParam._4, sTurnParam._8, sTurnParam._C);

    if (mLife == 1) {
        MR::startActionSound(this, "Accell", -1, -1, -1);
    } else if (MR::checkPassBckFrame(this, 20.0f)) {
        MR::startActionSound(this, "RollingStartAccell", -1, -1, -1);
    }

    if (MR::isActionEnd(this)) {
        MR::startActionSound(this, "RollGnd", -1, -1, -1);
        setNerve(getRollingNerve());
    }
}

void KingTossin::exeRollingEnd() {
    s32 maxRoll = 2;
    if (mLife == 1) {
        maxRoll = 3;
    }

    bool isChance = false;
    if (isNerve(&NrvKingTossin::KingTossinNrvRollingEndChance::sInstance) || mRollCount >= maxRoll || mLife == 3) {
        isChance = true;
    }

    if (MR::isFirstStep(this)) {
        if (isChance) {
            tryDeleteAngrySmoke(this);
            MR::startAction(this, "ChanceStart");
            MR::setVelocity(this, mFront * 30.0f);
        } else {
            MR::startAction(this, "RollingEnd");
            MR::setVelocity(this, mFront * 25.0f);
        }

        mMoveDir = mFront;
    }

    if (MR::getBckFrame(this) > 25.0f) {
        MR::startActionSound(this, "Rolling", -1, -1, -1);
    }

    if (mLife == 1 && !isChance) {
        MR::startActionSound(this, "Accell", -1, -1, -1);
    }

    MR::vecKillElement(mMoveDir, mGravity, &mMoveDir);
    if (MR::normalizeOrZero(&mMoveDir)) {
        mMoveDir = mFront;
    }

    MoveParam param = sRollingEndParam;
    if (isChance) {
        param = sRollingEndChanceParam;
    }

    MR::moveAndTurnToDirection(this, &mFront, mMoveDir, param._0, param._4, param._8, param._C);

    if (MR::isActionEnd(this)) {
        if (isChance) {
            MR::setVelocity(this, mFront * 18.0f);
            setNerve(&NrvKingTossin::KingTossinNrvChanceSliding::sInstance);
            return;
        }

        MR::setVelocity(this, mFront * 25.0f);
        setNerve(&NrvKingTossin::KingTossinNrvSliding::sInstance);
    }
}

void KingTossin::exeRollingAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RollingAttack");
        MR::zeroVelocity(this);
    }

    if (mLife == 1) {
        MR::startActionSound(this, "Accell", -1, -1, -1);
        tryEmitAngrySmoke(this);
    }

    if (MR::isActionEnd(this)) {
        setNerve(getWaitNerve());
    }
}

void KingTossin::exeSpinReaction() {
    bool isChance = isNerve(&NrvKingTossin::KingTossinNrvSpinReactionChance::sInstance);

    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);

        if (isChance) {
            MR::startAction(this, "SpinReactionChance");
        } else {
            MR::startAction(this, "SpinReaction");
        }
    }

    if (MR::isActionEnd(this)) {
        if (isChance) {
            setNerve(&NrvKingTossin::KingTossinNrvChanceEnd::sInstance);
        } else {
            setNerve(getWaitNerve());
        }
    }

    if (!isChance && mLife == 1) {
        MR::startActionSound(this, "Accell", -1, -1, -1);
    }
}

void KingTossin::exeTurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Turn");
        mJointCtrl->startFaceCtrl(-1);

        if (mLife == 1) {
            tryEmitAngrySmoke(this);
        }
    }

    MR::moveAndTurnToTarget(this, &mFront, getTargetPos(), sTurnParam._0, sTurnParam._4, sTurnParam._8, sTurnParam._C);

    TVec3f dir(0.0f);
    MR::calcVecToTargetPosH(&dir, this, getTargetPos(), nullptr);
    f32 cos = MR::cosDegree(30.0f);

    if (dir.dot(mFront) > cos) {
        setNerve(&NrvKingTossin::KingTossinNrvSearch::sInstance);
        return;
    }

    if (mLife == 1) {
        MR::startActionSound(this, "Accell", -1, -1, -1);
    }
}

void KingTossin::exeSearch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Search");
    }

    MR::moveAndTurnToTarget(this, &mFront, getTargetPos(), sStopParam._0, sStopParam._4, sStopParam._8, sStopParam._C);

    if (MR::isActionEnd(this)) {
        if (mLife == 4) {
            setNerve(&NrvKingTossin::KingTossinNrvChase::sInstance);
            return;
        }

        if (isTargetInSight(90.0f, 1800.0f)) {
            setNerve(&NrvKingTossin::KingTossinNrvRollingStart::sInstance);
        } else {
            setNerve(&NrvKingTossin::KingTossinNrvRun::sInstance);
        }
    }

    if (mLife == 1) {
        MR::startActionSound(this, "Accell", -1, -1, -1);
    }
}

void KingTossin::exeChase() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Run");
        mTimer = 0;
        mMoveDir = mFront;
    }

    MR::vecKillElement(mMoveDir, mGravity, &mMoveDir);
    if (MR::normalizeOrZero(&mMoveDir)) {
        mMoveDir = mFront;
    }

    MR::moveAndTurnToDirection(this, &mFront, mMoveDir, sChaseParam._0, sChaseParam._4, sChaseParam._8, sChaseParam._C);

    if (isTargetInSight(45.0f, 100000.0f)) {
        mTimer = 0;
    } else {
        mTimer++;
    }

    if (mLife == 1) {
        MR::startActionSound(this, "Accell", -1, -1, -1);
    }

    if (mTimer >= 180) {
        setNerve(&NrvKingTossin::KingTossinNrvBrake::sInstance);
    }
}

// NONMATCHING: saved integer register allocation.
void KingTossin::exeBrake() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Sliding");
        mSlideSpeed = PSVECMag(&mVelocity);
        MR::emitEffect(this, "BrakeSmokeL");
        MR::emitEffect(this, "BrakeSmokeR");
    }

    MR::vecKillElement(mMoveDir, mGravity, &mMoveDir);
    if (MR::normalizeOrZero(&mMoveDir)) {
        mMoveDir = mFront;
    }

    s32 start = sBrakeStartStep;
    s32 end = start + 60;
    f32 speed = MR::getEaseInValueWithNormalize(getNerveStep(), start, end, mSlideSpeed, sBrakeEndSpeed);
    TVec3f dir(mVelocity);
    MR::normalizeOrZero(&dir);
    mVelocity.set(dir * speed);
    MR::moveAndTurnToDirection(this, &mFront, mMoveDir, 0.0f, 2.0f, 1.0f, 0.0f);

    if (MR::isGreaterEqualStep(this, end)) {
        MR::zeroVelocity(this);
        deleteBrakeSmoke();
        setNerve(&NrvKingTossin::KingTossinNrvTurnToPlayer::sInstance);
    }
}

void KingTossin::exeTurnToPlayer() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Turn");
    }

    MR::moveAndTurnToTarget(this, &mFront, getTargetPos(), sTurnToPlayerParam._0, sTurnToPlayerParam._4, sTurnToPlayerParam._8,
                            sTurnToPlayerParam._C);

    if (mLife == 1) {
        MR::startActionSound(this, "Accell", -1, -1, -1);
    }

    TVec3f dir(0.0f);
    MR::calcVecToTargetPosH(&dir, this, getTargetPos(), nullptr);
    f32 cos = MR::cosDegree(5.0f);

    if (dir.dot(mFront) > cos) {
        setNerve(&NrvKingTossin::KingTossinNrvSearch::sInstance);
    }
}

void KingTossin::updateCameraTarget() {
    TVec3f target(mPosition);
    target += sCameraTargetOffset;
    mCameraTarget = target;
}

bool KingTossin::tryEndTargetInSight() {
    if (mIsTargetInSight) {
        if (isTargetInSight(90.0f, 1100.0f)) {
            return false;
        }

        if (isTargetInSight(180.0f, 1100.0f * sHalf)) {
            return false;
        }

        mIsTargetInSight = false;
        return true;
    }

    if (isTargetInSight(90.0f, 1100.0f)) {
        mIsTargetInSight = true;
    }

    return false;
}

bool KingTossin::isFrontToSensor(HitSensor* pSensor) {
    TVec3f dir(getTargetPos() - *MR::getSensorPos(pSensor));
    MR::vecKillElement(dir, mGravity, &dir);

    if (MR::normalizeOrZero(&dir)) {
        return true;
    }

    f32 dot = mFront.dot(dir);
    return dot < MR::cosDegree(80.0f);
}

bool KingTossin::isEnableAttack() {
    bool isEnable = false;

    if (!isNerve(&NrvKingTossin::KingTossinNrvDamage::sInstance) && !isNerve(&NrvKingTossin::KingTossinNrvDamageSecond::sInstance)
        && !isNerve(&NrvKingTossin::KingTossinNrvDemoPowerUp::sInstance) && !isNerve(&NrvKingTossin::KingTossinNrvDemoDown::sInstance)
        && !isNerve(&NrvKingTossin::KingTossinNrvAttack::sInstance) && !isNerve(&NrvKingTossin::KingTossinNrvRollingAttack::sInstance)) {
        isEnable = true;
    }

    return isEnable;
}

bool KingTossin::isEnableReceiveAttack() {
    bool isEnable = false;

    if (!isNerve(&NrvKingTossin::KingTossinNrvDamage::sInstance) && !isNerve(&NrvKingTossin::KingTossinNrvDamageSecond::sInstance)
        && !isNerve(&NrvKingTossin::KingTossinNrvDemoPowerUp::sInstance) && !isNerve(&NrvKingTossin::KingTossinNrvDemoDown::sInstance)) {
        isEnable = true;
    }

    return isEnable;
}

bool KingTossin::isRolling() {
    bool isRolling = false;

    if (isNerve(&NrvKingTossin::KingTossinNrvRollingStart::sInstance) || isNerve(&NrvKingTossin::KingTossinNrvRollingEnd::sInstance)
        || isNerve(&NrvKingTossin::KingTossinNrvRollingLv1::sInstance) || isNerve(&NrvKingTossin::KingTossinNrvRollingLv2::sInstance)
        || isNerve(&NrvKingTossin::KingTossinNrvRollingLv3::sInstance) || isNerve(&NrvKingTossin::KingTossinNrvDownRecover::sInstance)
        || isNerve(&NrvKingTossin::KingTossinNrvChanceEnd::sInstance)) {
        isRolling = true;
    }

    return isRolling;
}

bool KingTossin::isTargetInSight(f32 angle, f32 distance) {
    TVec3f dirToTarget(0.0f);
    MR::calcVecToTargetPosH(&dirToTarget, this, getTargetPos(), nullptr);
    TVec3f front(0.0f);
    MR::vecKillElement(mFront, mGravity, &front);

    if (!MR::isNearAngleDegree(dirToTarget, front, angle)) {
        return false;
    }

    f32 dist = MR::calcDistance(this, getTargetPos());
    f32 distH = MR::calcDistanceHorizontal(this, getTargetPos());

    if (distH < dist - distH) {
        return false;
    }

    return dist < distance;
}

const Nerve* KingTossin::getWaitNerve() {
    switch (mLife) {
    case 4:
        return &NrvKingTossin::KingTossinNrvTurnToPlayer::sInstance;
    case 3:
        return &NrvKingTossin::KingTossinNrvTurn::sInstance;
    case 2:
        return &NrvKingTossin::KingTossinNrvTurn::sInstance;
    case 1:
        return &NrvKingTossin::KingTossinNrvTurn::sInstance;
    }

    return &NrvKingTossin::KingTossinNrvRunRail::sInstance;
}

const Nerve* KingTossin::getRollingNerve() {
    switch (mLife) {
    case 4:
        return &NrvKingTossin::KingTossinNrvRollingLv1::sInstance;
    case 3:
        return &NrvKingTossin::KingTossinNrvRollingLv2::sInstance;
    case 2:
        return &NrvKingTossin::KingTossinNrvRollingLv3::sInstance;
    case 1:
        return &NrvKingTossin::KingTossinNrvRollingLv3::sInstance;
    }

    return &NrvKingTossin::KingTossinNrvRun::sInstance;
}

bool KingTossin::deleteBrakeSmoke() {
    bool isDeleted = false;

    if (MR::isEffectValid(this, "BrakeSmokeL")) {
        MR::deleteEffect(this, "BrakeSmokeL");
        isDeleted = true;
    }

    // the target checks BrakeSmokeL again before deleting BrakeSmokeR
    if (MR::isEffectValid(this, "BrakeSmokeL")) {
        MR::deleteEffect(this, "BrakeSmokeR");
        isDeleted = true;
    }

    return isDeleted;
}

void KingTossin::resetPosition() {
    MR::setBaseTRMtx(this, mStartMtx);
    mStartMtx.getTrans(mPosition);
    MR::setPlayerBaseMtx(mPlayerMtx);
}

KingTossin::~KingTossin() {}

namespace {
    const f32 sHalf = 0.5f;
    const f32 sRollingSpeed = 15.0f;
    const f32 sRollingSpeedStep = 3.0f;
}
