#include "Game/Boss/Yukkina.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/TriangleFilter.hpp"

namespace NrvYukkina {
    NEW_NERVE(YukkinaNrvHidden, Yukkina, Hidden);
    NEW_NERVE(YukkinaNrvAppear, Yukkina, Appear);
    NEW_NERVE(YukkinaNrvAppearEnd, Yukkina, AppearEnd);
    NEW_NERVE(YukkinaNrvWait, Yukkina, Wait);
    NEW_NERVE(YukkinaNrvBreak, Yukkina, Break);
    NEW_NERVE(YukkinaNrvBreakWait, Yukkina, BreakWait);
    NEW_NERVE(YukkinaNrvJump, Yukkina, Jump);
    NEW_NERVE(YukkinaNrvJumpWait, Yukkina, JumpWait);
    NEW_NERVE(YukkinaNrvWaitTurn, Yukkina, WaitTurn);
    NEW_NERVE(YukkinaNrvSwitchDead, Yukkina, SwitchDead);
    NEW_NERVE(YukkinaNrvDemoStart, Yukkina, DemoStart);
    NEW_NERVE(YukkinaNrvDemoEnd, Yukkina, DemoEnd);

    class YukkinaNrvWaitPowerStar : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            Yukkina* pActor = reinterpret_cast< Yukkina* >(pSpine->mExecutor);
            if (MR::isEndPowerStarAppearDemo(pActor)) {
                MR::showPlayer();
                pActor->kill();
            }
        }
        static YukkinaNrvWaitPowerStar sInstance;
    };
    YukkinaNrvWaitPowerStar YukkinaNrvWaitPowerStar::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvYukkina


Yukkina::Yukkina(const char* pName)
    : LiveActor(pName), mCameraInfo(NULL), mPoseQuat(0.0f, 0.0f, 0.0f, 1.0f), mBreakBaseQuat(0.0f, 0.0f, 0.0f, 1.0f), mFront(0.0f, 0.0f, 1.0f),
      mInitialQuat(0.0f, 0.0f, 0.0f, 1.0f), mInitialPos(0.0f, 0.0f, 0.0f), mCameraTargetPos(0.0f, 0.0f, 0.0f), mCameraRegisterId(-1), mCounter(0), mStep(3), _124(0.0f),
      _128(0.0f), _12C(0.0f), mLowAngle(1.0f), mHighAngle(3.0f), mRollSpeed(1.5f), mIdx(1), mHasPlayerLink(0) {
    mBaseMtx.identity();
}

void Yukkina::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    MR::makeQuatAndFrontFromRotate(&mPoseQuat, &mFront, this);
    mInitialQuat.set(mPoseQuat);
    mInitialPos.set(mPosition);

    MR::setAllEffectHostMtx(this, mBaseMtx);
    mCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initAnimCamera(this, mCameraInfo, "DemoOpening");
    MR::initAnimCamera(this, mCameraInfo, "DemoDown");
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "\x83\x49\x81\x5b\x83\x76\x83\x6a\x83\x93\x83\x4f\x83\x66\x83\x82\x83\x58\x83\x4c\x83\x62\x83\x76");
    MR::getJMapInfoArg7WithInit(rIter, &mCameraRegisterId);
    if (mCameraRegisterId != -1) {
        MR::declareCameraRegisterVec(this, mCameraRegisterId, &mCameraTargetPos);
        MR::calcPositionUpOffset(&mCameraTargetPos, this, 250.0f);
    }

    TriangleFilterDelegator< Yukkina >* pFilter = MR::createTriangleFilterDelegator(this, &Yukkina::isInvalidTriangle);
    MR::addBaseMatrixFollowTarget(this, rIter, NULL, reinterpret_cast< BaseMatrixFollowValidater* >(pFilter));
    mHasPlayerLink = MR::tryRegisterNamePosLinkObj(this, rIter, "\x83\x7d\x83\x8a\x83\x49\x88\xca\x92\x75");
    initNerve(&NrvYukkina::YukkinaNrvHidden::sInstance, 0);
    MR::hideModel(this);
    MR::declarePowerStar(this);
    MR::initUseStageSwitchWriteDead(this, rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
    MR::startBva(this, "Hp");
    MR::setBvaFrameAndStop(this, 0.0f);
}

void Yukkina::kill() {
    LiveActor::kill();
}

void Yukkina::control() {
    if (mCameraRegisterId != -1) {
        mCameraTargetPos.set(mPosition);
        JMAVECScaleAdd(&mGravity, &mCameraTargetPos, &mCameraTargetPos, 250.0f);
    }
    TVec3f up;
    JGeometry::negateInternal(&mGravity.x, &up.x);
    MR::makeMtxUpNoSupportPos(&mBaseMtx, up, mPosition);
    if (MR::isBindedGround(this)) {
        mCounter = 0;
        return;
    }
    mCounter += 1;
}

void Yukkina::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPoseQuat);
}

void Yukkina::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver) && MR::isEqualSensor(pSender, this, "Body")) {
        if (isNerve(&NrvYukkina::YukkinaNrvWait::sInstance) && MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
            setNerve(&NrvYukkina::YukkinaNrvJump::sInstance);
            MR::setVelocitySeparateHV(this, pReceiver, pSender, 5.0f, 15.0f);
            return;
        }
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool Yukkina::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && isNerve(&NrvYukkina::YukkinaNrvWait::sInstance)) {
        if (MR::isEqualSensor(pReceiver, this, "Nose")) {
            mStep -= 1;
            if (mStep == 1) {
                MR::stopStageBGM(10);
            }
            if (mStep <= 0) {
                MR::zeroVelocity(this);
                setNerve(&NrvYukkina::YukkinaNrvSwitchDead::sInstance);
                return true;
            }
            mIdx += 1;
            TVec3f sp8;
            MR::calcSensorHorizonNormalize(&sp8, mGravity, pSender, pReceiver);
            mFront.set(sp8);
            MR::setVelocitySeparateHV(this, pSender, pReceiver, 20.0f, 40.0f);
            setNerve(&NrvYukkina::YukkinaNrvBreak::sInstance);
            return true;
        }
    }
    return false;
}

void Yukkina::exeHidden() {
    if (MR::isFirstStep(this)) {
        mPoseQuat.set(mInitialQuat);
        MR::resetPosition(this, mInitialPos);
        MR::zeroVelocity(this);
        MR::stopStageBgmForDemo();
    }
    if (MR::isGreaterEqualStep(this, 4)) {
        if (MR::tryStartDemoMarioPuppetable(this, "\x83\x49\x81\x5b\x83\x76\x83\x6a\x83\x93\x83\x4f")) {
            MR::hidePlayer();
            if (mHasPlayerLink != 0) {
                MR::showPlayer();
                MR::setPlayerLinkPosAndWait(this, "\x83\x7d\x83\x8a\x83\x49\x88\xca\x92\x75", false);
                MR::startBckPlayer("BattleWait", NULL);
            }
            MR::startAnimCameraTargetSelf(this, mCameraInfo, "DemoOpening", 0, false, 1.0f);
            MR::clearPlayerFootprints();
            setNerve(&NrvYukkina::YukkinaNrvAppear::sInstance);
        }
    }
}

void Yukkina::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::startAction(this, "DemoOpening");
    }
    if (MR::tryEndDemo(this, "\x83\x49\x81\x5b\x83\x76\x83\x6a\x83\x93\x83\x4f", MR::isActionEnd(this))) {
        MR::endAnimCamera(this, mCameraInfo, "DemoOpening", -1, true);
        MR::showPlayer();
        if (MR::isDemoEndWaitCameraInterpolating()) {
            startWaitAction();
            MR::tryForceDeleteEffectAll(this);
            MR::startMultiActorCameraTargetSelf(this, mCameraInfo, "\x83\x49\x81\x5b\x83\x76\x83\x6a\x83\x93\x83\x4f\x83\x66\x83\x82\x83\x58\x83\x4c\x83\x62\x83\x76", 0);
            setNerve(&NrvYukkina::YukkinaNrvAppearEnd::sInstance);
            return;
        }
        setNerve(&NrvYukkina::YukkinaNrvWait::sInstance);
    }
}

void Yukkina::exeAppearEnd() {
    MR::isFirstStep(this);
    if (MR::isGreaterStep(this, 3)) {
        MR::endMultiActorCamera(this, mCameraInfo, "\x83\x49\x81\x5b\x83\x76\x83\x6a\x83\x93\x83\x4f\x83\x66\x83\x82\x83\x58\x83\x4c\x83\x62\x83\x76", false, -1);
        setNerve(&NrvYukkina::YukkinaNrvWait::sInstance);
    }
}

void Yukkina::exeWait() {
    if (MR::isFirstStep(this)) {
        startWaitAction();
    }
    updateSound();
    if (mCounter < 5) {
        f32 turn = MR::isNearPlayer(this, 1000.0f) ? mLowAngle : mHighAngle;
        MR::turnDirectionToTargetUseGroundNormalDegree(this, &mFront, *MR::getPlayerPos(), turn);
        MR::addVelocityMoveToDirection(this, mFront, mRollSpeed);
    } else {
        MR::addVelocityToGravity(this, 1.5f);
    }
    if (MR::sendMsgEnemyAttackToBindedSensor(this, getSensor("Body"))) {
        TVec3f n;
        n = MR::getBindedFixReactionVector(this);
        MR::normalizeOrZero(&n);
        TVec3f push = n * 25.0f;
        MR::addVelocity(this, push);
    }
    MR::attenuateVelocity(this, mCounter < 5 ? 0.9f : 0.99f);
    MR::reboundVelocityFromEachCollision(this, -1.0f, 0.0f, 0.0f, 0.0f);
    TVec3f up;
    JGeometry::negateInternal(&mGravity.x, &up.x);
    MR::rotateQuatRollBall(&mPoseQuat, mVelocity, up, 400.0f * mScale.x);
}

void Yukkina::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::stopScene(5);
        MR::startAction(this, "Damage");
        setTurnParam(mIdx);
        TVec3f toPlayer;
        MR::calcVecToPlayerHorizon(&toPlayer, this, NULL);
        TVec3f up;
        JGeometry::negateInternal(&mGravity.x, &up.x);
        MR::makeQuatUpFront(&mBreakBaseQuat, up, toPlayer);
    }
    MR::addVelocityToGravity(this, 1.5f);
    MR::attenuateVelocity(this, 0.99f);
    TVec3f up;
    JGeometry::negateInternal(&mGravity.x, &up.x);
    MR::turnQuatZDirRate(&mBreakBaseQuat, mBreakBaseQuat, up, 0.1f);
    MR::rotateQuatXAxisDegree(&mPoseQuat, mBreakBaseQuat, MR::calcNerveEaseOutValue(this, 40, 0.0f, -360.0f));
    if (!MR::sendMsgEnemyAttackToBindedSensor(this, getSensor("Body"))) {
        if (MR::isGreaterStep(this, 10) && MR::isBindedGround(this)) {
            MR::zeroVelocity(this);
            MR::startAction(this, "Land");
            setNerve(&NrvYukkina::YukkinaNrvBreakWait::sInstance);
        }
    }
}

void Yukkina::exeBreakWait() {
    MR::isFirstStep(this);
    if (MR::isStep(this, 0x37) && mStep == 1) {
        MR::startStageBGMIfNotPlaying("STM_SMG2_BOSS_05B", false);
    }
    if (MR::isGreaterStep(this, 0x3C)) {
        setNerve(&NrvYukkina::YukkinaNrvWait::sInstance);
    }
}

void Yukkina::exeJump() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSuccess");
    }
    MR::addVelocityToGravity(this, 1.5f);
    MR::attenuateVelocity(this, 0.99f);
    TVec3f toPlayer;
    MR::calcVecToPlayerH(&toPlayer, this, NULL);
    MR::fn_800104F0(&mPoseQuat, this, &toPlayer, 0.2f, 0.1f);
    if (MR::isGreaterStep(this, 10) && MR::isBindedGround(this)) {
        MR::zeroVelocity(this);
        setNerve(&NrvYukkina::YukkinaNrvJumpWait::sInstance);
    }
}

void Yukkina::exeJumpWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSuccessWait");
    }
    TVec3f toPlayer;
    MR::calcVecToPlayerH(&toPlayer, this, NULL);
    MR::fn_800104F0(&mPoseQuat, this, &toPlayer, 0.2f, 0.1f);
    if (MR::isGreaterStep(this, 0x50)) {
        setNerve(&NrvYukkina::YukkinaNrvWaitTurn::sInstance);
    }
}

void Yukkina::exeWaitTurn() {
    if (MR::isFirstStep(this)) {
        MR::calcVecToPlayerH(&mFront, this, NULL);
        JGeometry::negateInternal(&mFront.x, &mFront.x);
    }
    updateSound();
    if (mCounter < 5) {
        MR::turnDirectionToTargetUseGroundNormalDegree(this, &mFront, *MR::getPlayerPos(), 0.0f);
        MR::addVelocityMoveToDirection(this, mFront, mRollSpeed);
    } else {
        MR::addVelocityToGravity(this, 1.5f);
    }
    if (MR::sendMsgEnemyAttackToBindedSensor(this, getSensor("Body"))) {
        TVec3f n;
        n = MR::getBindedFixReactionVector(this);
        MR::normalizeOrZero(&n);
        TVec3f push = n * 25.0f;
        MR::addVelocity(this, push);
    }
    MR::attenuateVelocity(this, mCounter < 5 ? 0.9f : 0.99f);
    MR::reboundVelocityFromEachCollision(this, -1.0f, 0.0f, 0.0f, 0.0f);
    TVec3f up;
    JGeometry::negateInternal(&mGravity.x, &up.x);
    MR::rotateQuatRollBall(&mPoseQuat, mVelocity, up, 400.0f * mScale.x);
    if (MR::isGreaterStep(this, 0x5A)) {
        setNerve(&NrvYukkina::YukkinaNrvWait::sInstance);
    }
}

void Yukkina::exeSwitchDead() {
    if (MR::isFirstStep(this)) {
        MR::stopStageBGM(0);
        MR::startActionSound(this, "DamageLast", -1, -1, -1);
        MR::startAction(this, "Dead");
        TVec3f toPlayer;
        MR::calcVecToPlayerHorizon(&toPlayer, this, NULL);
        TVec3f up;
        JGeometry::negateInternal(&mGravity.x, &up.x);
        MR::makeQuatUpFront(&mPoseQuat, up, toPlayer);
    }
    if (MR::isActionEnd(this)) {
        if (MR::isValidSwitchDead(this)) {
            MR::onSwitchDead(this);
        }
        setNerve(&NrvYukkina::YukkinaNrvDemoStart::sInstance);
    }
}

void Yukkina::exeDemoStart() {
    if (MR::tryStartDemoMarioPuppetable(this, "\x83\x5f\x83\x45\x83\x93")) {
        mPoseQuat.set(mInitialQuat);
        MR::resetPosition(this, mInitialPos);
        MR::zeroVelocity(this);
        MR::showModel(this);
        MR::startAnimCameraTargetSelf(this, mCameraInfo, "DemoDown", 0, false, 1.0f);
        setNerve(&NrvYukkina::YukkinaNrvDemoEnd::sInstance);
    }
}

void Yukkina::exeDemoEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "DemoDown");
        MR::hidePlayer();
    }
    if (MR::isActionEnd(this)) {
        MR::endDemo(this, "\x83\x5f\x83\x45\x83\x93");
        if (mHasPlayerLink != 0) {
            MR::setPlayerLinkPosAndWait(this, "\x83\x7d\x83\x8a\x83\x49\x88\xca\x92\x75", false);
        }
        if (MR::isDemoEndWaitCameraInterpolating()) {
            MR::startAction(this, "Wait");
            MR::tryForceDeleteEffectAll(this);
        } else {
            MR::tryHitReactionDead(this);
        }
        MR::hideModel(this);
        MR::endAnimCamera(this, mCameraInfo, "DemoDown", -1, true);
        MR::showPlayer();
        kill();
        MR::requestAppearPowerStarFromBelow(this);
        setNerve(&NrvYukkina::YukkinaNrvWaitPowerStar::sInstance);
    }
}

void Yukkina::setTurnParam(s32 arg) {
    switch (arg) {
    case 1:
        mLowAngle = 1.0f;
        mHighAngle = 3.0f;
        mRollSpeed = 1.5f;
        return;
    case 2:
        mLowAngle = 1.0f;
        mHighAngle = 3.0f;
        mRollSpeed = 2.0f;
        return;
    case 3:
        mLowAngle = 0.0f;
        mHighAngle = 3.0f;
        mRollSpeed = 3.0f;
        return;
    }
}

void Yukkina::startWaitAction() {
    MR::startAction(this, mStep == 1 ? "Angry" : "Wait");
    MR::startBva(this, "Hp");
    MR::setBvaFrameAndStop(this, (f32)(3 - mStep));
}

void Yukkina::updateSound() {
    f32 speed = MR::calcVelocityLength(this);
    MR::startActionSound(this, "Roll", (s32)(3.0f * (speed * speed)), -1, -1);
    if (isBindedAndHigh()) {
        MR::startActionSound(this, "Bound", -1, -1, -1);
    }
}

bool Yukkina::isInvalidTriangle(const Triangle*) {
    if (isNerve(&NrvYukkina::YukkinaNrvAppear::sInstance) || isNerve(&NrvYukkina::YukkinaNrvDemoEnd::sInstance) || isNerve(&NrvYukkina::YukkinaNrvWaitPowerStar::sInstance)) {
        return false;
    }
    return true;
}

bool Yukkina::isBindedAndHigh() {
    if (MR::isBindedGround(this) && mCounter >= 0x14) {
        return true;
    }
    return false;
}
