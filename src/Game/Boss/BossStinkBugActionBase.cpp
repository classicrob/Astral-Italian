#include "Game/Boss/BossStinkBugActionBase.hpp"
#include "Game/Util.hpp"
#include "Game/Boss/BossStinkBug.hpp"
#include "Game/Boss/BossStinkBugFunction.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/SpringValue.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/mtx.h>
#include <revolution/types.h>


BossStinkBugActionBase::BossStinkBugActionBase(const char* pName, BossStinkBug* pStinkBug)
    : ActorStateBase< BossStinkBug >(pName, pStinkBug), mSpringValue(), mMoveOffset(0.0f, 0.0f, 0.0f), mTurnStartFront(0.0f, 0.0f, 1.0f), mCruiseSpeed(15.0f), mRailHeight(), _40(), mRailHeightOffset(),
      mThrowBombTimer(), mWasRailGoingToEnd(true), mCameraDistance(), mPrevCameraDistance(), mApproachSoundTimer(), mIsNearPlayer(), mWasNearPlayer() {
    mSpringValue = new SpringValue(mRailHeight, mRailHeight, 0.02f, 0.95f, 0.0f);
}

bool BossStinkBugActionBase::updateGroundWalk(f32 f1) {
    if (MR::isFirstStep(this)) {
        MR::tryStartBck(getHost(), "GroundWalk");
    }

    BossStinkBugFunction::moveRail(getHost(), f1, 0.0f, 0.0f);
    mMoveSpeed = f1;

    MR::calcDistanceToPlayer(getHost());
    return false;
}

bool BossStinkBugActionBase::updateFlyRoll() {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "FlyRollRight");
    }

    startSoundFly(0);
    mMoveSpeed = mCruiseSpeed;

    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, mRailHeight, mRailHeightOffset);

    if (MR::isActionEnd(getHost())) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateFlyRollWait(s32 waitSteps) {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "FlyRollWaitRight");
    }

    startSoundFly(0);
    mMoveSpeed = mCruiseSpeed;

    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, mRailHeight, mRailHeightOffset);

    if (MR::isGreaterStep(this, waitSteps)) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateFlyRollEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "FlyRollEndRight");
    }

    startSoundFly(0);
    mMoveSpeed = mCruiseSpeed;

    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, mRailHeight, mRailHeightOffset);

    if (MR::isActionEnd(getHost())) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateFlyDash(f32 f1) {
    if (MR::isFirstStep(this)) {
        MR::tryStartBck(getHost(), "FlyWait");
        BossStinkBugFunction::calcDiffCurrentRailPosition(&mMoveOffset, getHost(), mRailHeight, mRailHeightOffset);
    }

    startSoundFly(0);

    if (MR::isLessStep(this, 30)) {
        mMoveSpeed = MR::calcNerveEaseInOutValue(this, 30, f1, 40.0f);
    } else {
        mMoveSpeed = MR::calcNerveEaseInOutValue(this, 30, 100, 40.0f, mCruiseSpeed);
    }

    BossStinkBugFunction::regainToRail(getHost(), mMoveOffset, MR::calcNerveRate(this, 100), mMoveSpeed, mRailHeight, mRailHeightOffset);

    if (MR::isGreaterStep(this, 100)) {
        return true;
    } else {
        return false;
    }
}

void BossStinkBugActionBase::throwBomb(s32 newWaitTime) {
    if (mThrowBombTimer == 0) {
        BossStinkBugFunction::throwBomb(getHost());
        mThrowBombTimer = newWaitTime;
    } else {
        mThrowBombTimer--;
    }
}

void BossStinkBugActionBase::resetThrowBombTiming() {
    mThrowBombTimer = 0;
}

f32 BossStinkBugActionBase::calcShakeOffSignSpeed(s32 a1) const {
    if (MR::isLessStep(this, 10)) {
        return MR::calcNerveEaseInOutValue(this, a1, mCruiseSpeed, 3.0f);
    } else {
        return 3.0f;
    }
}

f32 BossStinkBugActionBase::calcShakeOffSpeed() const {
    return MR::calcNerveEaseInOutValue(this, MR::getBckFrameMax(getHost(), "FlyRollRight"), 3.0f, mCruiseSpeed);
}

f32 BossStinkBugActionBase::calcShakeOffEndSpeed() const {
    return MR::calcNerveEaseInOutValue(this, MR::getBckFrameMax(getHost(), "FlyRollEndRight"), 35.0f, mCruiseSpeed);
}

s32 BossStinkBugActionBase::calcKeepRollTime(s32 a1) const {
    f32 flt1 = (MR::getRailTotalLength(getHost()) * (2.0f / 3.0f)) / mCruiseSpeed;

    f32 flt2 = 0.0f;
    flt2 += MR::getBckFrameMax(getHost(), "FlyRollRight");
    flt2 += MR::getBckFrameMax(getHost(), "FlyRollEndRight");
    flt2 += a1;

    s32 keepRollTime = (flt1 - flt2) / 2;

    if (keepRollTime < 0) {
        keepRollTime = 0;
    }

    return keepRollTime;
}

s32 BossStinkBugActionBase::calcFlySoundVol() const {
    s32 vol = 550;
    if (mApproachSoundTimer > 0) {
        if (mCameraDistance < 1500.0f) {
            vol = 1000;
            if (mApproachSoundTimer < 12) {
                vol = MR::getLinerValue(mApproachSoundTimer, 0.0f, 1.0f, 12.0f);
                vol = 450 * vol + 550;
            }
        } else if (mCameraDistance < 2500) {
            vol = MR::getLinerValueFromMinMax(mCameraDistance, 1500.0f, 2500.0f, 1000.0f, 550.0f);
            if (mApproachSoundTimer < 12) {
                vol = MR::getLinerValue(mApproachSoundTimer, 0.0f, 1.0f, 12.0f);
                vol = 450 * vol + 550;
            }
        } else {
            vol = 550;
        }
    }

    return vol;
}

s32 BossStinkBugActionBase::calcIIRCutOff() const {
    if (mCameraDistance < 2000.0f) {
        return 126;
    } else if (mCameraDistance < 4500.0f) {
        return MR::getLinerValueFromMinMax(mCameraDistance, 2000.0f, 4500.0f, 126.0f, 20.0f);
    }

    return 20;
}

void BossStinkBugActionBase::updateSound() {
    if (getHost()->isLegInWater()) {
        if (isAngry()) {
            MR::setSeVersion(getHost(), BossStinkBug::SeVersion_ANGRY_WATER);
        } else {
            MR::setSeVersion(getHost(), BossStinkBug::SeVersion_WATER);
        }
    } else {
        if (isAngry()) {
            MR::setSeVersion(getHost(), BossStinkBug::SeVersion_ANGRY);
        } else {
            MR::setSeVersion(getHost(), BossStinkBug::SeVersion_NORMAL);
        }
    }
}

void BossStinkBugActionBase::updateSoundFly() {
    mWasNearPlayer = mIsNearPlayer;

    if (MR::isNearPlayer(getHost(), 2000.0f)) {
        mIsNearPlayer = true;
    } else {
        mIsNearPlayer = false;
    }

    if (mApproachSoundTimer > 0) {
        mApproachSoundTimer--;
    }

    mPrevCameraDistance = mCameraDistance;
    mCameraDistance = MR::getCamPos().distance(getHost()->mPosition);

    if (mPrevCameraDistance - mCameraDistance > 0.0f) {
        mApproachSoundTimer = 12;
    }
}

bool BossStinkBugActionBase::updateGroundRegainRail(f32 f1) {
    if (MR::isFirstStep(this)) {
        MR::tryStartBck(getHost(), "GroundWalk");
        BossStinkBugFunction::calcDiffCurrentRailPosition(&mMoveOffset, getHost(), mRailHeight, mRailHeightOffset);
    }

    f32 nerveRate = MR::calcNerveRate(this, 90);
    BossStinkBugFunction::regainToRail(getHost(), mMoveOffset, nerveRate, f1, 0.0f, 0.0f);
    mMoveSpeed = f1;

    if (MR::isGreaterStep(this, 90)) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateGroundFind() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(getHost(), "BvBossBugAttackG");
        MR::startAction(getHost(), "Find");
        mMoveSpeed = 0.0f;
    }

    if (MR::isActionEnd(getHost())) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateGroundShakeOff() {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "GroundRide");
    }

    mMoveSpeed *= 0.95f;
    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, 0.0f, 0.0f);

    if (MR::isGreaterStep(this, 90)) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateGroundRun(f32 f1) {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "GroundRun");
        BossStinkBugFunction::validateAttack(getHost());
    }

    if (getHost()->isLegInWater()) {
        MR::startActionSound(getHost(), "BmLvBossBugRunAddWater");
    }

    mMoveSpeed = f1 * MR::calcNerveRate(this, 60);
    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, 0.0f, 0.0f);

    if (MR::isGreaterStep(this, 180)) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateGroundAttackSuccess() {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "AttackSuccess");
    }

    if (MR::isStep(this, 12)) {
        MR::startActionSound(getHost(), "BvBossBugGlad");
    }

    mMoveSpeed *= 0.75f;
    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, 0.0f, 0.0f);

    if (MR::isGreaterStep(this, 180)) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateGroundCoolDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "CoolDown");
    }

    MR::startActionSound(getHost(), "BmLvBossBugBrake");

    if (getHost()->isLegInWater()) {
        MR::startActionSound(getHost(), "BmLvBossBugRunAddWater");
    }

    mMoveSpeed *= 0.95f;
    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, 0.0f, 0.0f);

    if (MR::isActionEnd(getHost())) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateGroundTurn() {
    if (MR::isFirstStep(this)) {
        mTurnStartFront = getHost()->mFront;

        mWasRailGoingToEnd = MR::isRailGoingToEnd(getHost());

        if (mWasRailGoingToEnd) {
            MR::startAction(getHost(), "GroundTurnLeft");
        } else {
            MR::startAction(getHost(), "GroundTurnRight");
        }

        mMoveSpeed = 0.0f;
    }

    BossStinkBugFunction::turnRailGround(getHost(), mTurnStartFront, MR::calcNerveRate(this, 120), mWasRailGoingToEnd);

    if (MR::isGreaterStep(this, 160)) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateGroundDamage() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(getHost());
        MR::startActionSound(getHost(), "BmBossBugPressed");
        MR::startAction(getHost(), "GroundDamage");

        MR::tryRumblePadStrong(getHost(), WPAD_CHAN0);
        MR::shakeCameraStrong();

        getHost()->startEventCamera("Fall");
        getHost()->disposeBomb();

        mMoveSpeed = 0.0f;
        MR::offPlayerControl();
    }

    if (MR::isStep(this, 9)) {
        MR::startActionSound(getHost(), "BvBossBugDamage");
    }

    if (MR::isActionEnd(getHost())) {
        MR::onPlayerControl(true);
        getHost()->endEventCamera("Fall");
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateFly() {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "FlyWait");
    }

    startSoundFly(0);

    mMoveSpeed = mCruiseSpeed;
    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, mRailHeight, mRailHeightOffset);
    return false;
}

bool BossStinkBugActionBase::updateFlyShakeOffSign(s32 numSteps) {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "FlyRide");
        mSpringValue->mSpringValue = mRailHeight;
        mSpringValue->mRestValue = mRailHeight;
        mSpringValue->mVelocity = -15.0f;
    }

    startSoundFly(0);

    mMoveSpeed = calcShakeOffSignSpeed(numSteps);

    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, mSpringValue->update(), mRailHeightOffset);

    if (MR::isGreaterStep(this, numSteps)) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateFlyShakeOff() {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "FlyRollRight");
    }

    startSoundFly(3);

    mMoveSpeed = calcShakeOffSpeed();

    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, mSpringValue->update(), mRailHeightOffset);

    if (MR::isActionEnd(getHost())) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateFlyShakeOffWait(s32 numSteps) {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "FlyRollWaitRight");
    }

    startSoundFly(0);

    mMoveSpeed = 35.0f;
    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, mRailHeight, mRailHeightOffset);

    if (MR::isGreaterStep(this, numSteps)) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateFlyShakeOffEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "FlyRollEndRight");
    }

    startSoundFly(0);

    mMoveSpeed = calcShakeOffEndSpeed();
    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, mRailHeight, mRailHeightOffset);

    if (MR::isActionEnd(getHost())) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateFlyRollSign(s32 numSteps) {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "FlyRollSignRight");
    }

    if (MR::isStep(this, 10)) {
        MR::startActionSound(getHost(), "BvBossBugRoll");
    }

    startSoundFly(0);

    mMoveSpeed = mCruiseSpeed;
    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, mRailHeight, mRailHeightOffset);

    if (MR::isGreaterStep(this, numSteps)) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateFlyTurnSign() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(getHost(), "BvBossBugTurn");
        MR::startAction(getHost(), "FlyTurnSign");
    }

    startSoundFly(0);

    mMoveSpeed = mCruiseSpeed;
    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, mRailHeight, mRailHeightOffset);

    if (MR::isActionEnd(getHost())) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateFlyTurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "FlyTurn");
        mTurnStartFront = getHost()->mFront;
        mWasRailGoingToEnd = MR::isRailGoingToEnd(getHost());
    }

    startSoundFly(1);

    mMoveSpeed = 0.0f;
    BossStinkBugFunction::turnRail(getHost(), mTurnStartFront, 0.0f, MR::calcNerveRate(this, 120), mCruiseSpeed, mRailHeight, mRailHeightOffset, mWasRailGoingToEnd);

    if (MR::isGreaterStep(this, 120)) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateFall() {
    if (MR::isFirstStep(this)) {
        getHost()->startEventCamera("Fall");
        MR::startAction(getHost(), "Fall");
        MR::onBind(getHost());

        mMoveOffset = getHost()->mPosition;
        mMoveSpeed = 0.0f;
        getHost()->disposeBomb();
    }

    MR::startActionSound(getHost(), "BmLvBossBugFlyAdd", 600, 126);

    BossStinkBugFunction::setFallVelocity(getHost(), mMoveOffset, MR::calcNerveRate(this, 80), 0.3f, 1100.0f);

    if (MR::isBinded(getHost())) {
        MR::offBind(getHost());
        MR::zeroVelocity(getHost());
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateFallDamage() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(getHost(), "BmBossBugPressed");
        MR::startAction(getHost(), "FallDamage");
        MR::startAction(getHost()->getWingModel(), "FallDamage");
        MR::tryRumblePadStrong(getHost(), WPAD_CHAN0);
        MR::shakeCameraStrong();
        MR::offBind(getHost());

        mMoveSpeed = 0.0f;
        MR::offPlayerControl();
        getHost()->disposeBomb();
    }

    if (MR::isStep(this, 9)) {
        MR::startActionSound(getHost(), "BvBossBugDamage");
    }

    if (MR::isActionEnd(getHost())) {
        MR::onPlayerControl(true);
        getHost()->endEventCamera("Fall");
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateToGround() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(getHost(), "BvBossBugAngryLand");
        MR::startAction(getHost()->getWingModel(), "Ground");
        MR::tryStartBck(getHost(), "TurnDown");
        getHost()->validateCollisionGround();
        getHost()->onBindLeg();
        getHost()->mPoseBlendRate = 0.0f;
        mTurnStartFront = getHost()->mFront;

        mFallHeight = mRailHeight + mRailHeightOffset;
        mFallVelocity = 0.0f;
    }

    startSoundFly(0);
    mFallHeight += mFallVelocity;

    if (MR::isLessEqualStep(this, 40)) {
        mFallVelocity += 1.2f;
    }

    mFallVelocity -= 1.0f;
    mFallVelocity *= 0.99f;

    if (mFallHeight < 0.0f) {
        mFallHeight = 0.0f;
    }

    f32 nerveRate = MR::calcNerveRate(this, 60);
    getHost()->mPoseBlendRate = nerveRate;
    MR::rotateVecDegree(&getHost()->mFront, MR::getRailDirection(getHost()), getHost()->mGravity, MR::calcNerveValue(this, 60, 0.0f, 180.0f));

    mMoveSpeed *= 0.9f;
    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, mFallHeight, 0.0f);

    if (mFallHeight <= 0.0f) {
        MR::reverseRailDirection(getHost());
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateLand() {
    if (MR::isFirstStep(this)) {
        MR::tryStartBck(getHost(), "Land");
        getHost()->mPoseBlendRate = 1.0f;
    }

    mMoveSpeed *= 0.99f;
    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, 0.0f, 0.0f);

    if (MR::isGreaterStep(this, 20)) {
        return true;
    } else {
        return false;
    }
}

bool BossStinkBugActionBase::updateToFly() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(getHost(), "BvBossBugAngryTakeoff");
        MR::startAction(getHost()->getWingModel(), "Fly");
        MR::tryStartBck(getHost(), "TakeOff");
        mTakeoffStartSpeed = mMoveSpeed;
        getHost()->validateCollisionHitOnly();
        getHost()->offBindLeg();
    }

    if (MR::isLessStep(this, 0)) {
        startSoundFly(0);
    } else {
        startSoundFly(2);
    }

    if (MR::isLessStep(this, 60)) {
        mMoveSpeed = MR::calcNerveEaseInOutValue(this, 60, mTakeoffStartSpeed, 40.0f);
    } else {
        mMoveSpeed = MR::calcNerveEaseInOutValue(this, 60, 120, 40.0f, mCruiseSpeed);
    }

    f32 nerveEaseInOutValue = MR::calcNerveEaseInOutValue(this, 120, 0.0f, mRailHeight + mRailHeightOffset);
    BossStinkBugFunction::moveRail(getHost(), mMoveSpeed, nerveEaseInOutValue, 0.0f);

    if (MR::isGreaterStep(this, 120)) {
        getHost()->validateCollisionFly();
        return true;
    } else {
        return false;
    }
}

void BossStinkBugActionBase::startSoundFly(s32 sound) {
    s32 flySoundVol = calcFlySoundVol();
    s32 iIRCCutoff = calcIIRCutOff();

    switch (sound) {
    case 0:
        if (isAngry()) {
            MR::startActionSound(getHost(), "BmLvBossBugFlyBase2", flySoundVol, iIRCCutoff);
        } else {
            MR::startActionSound(getHost(), "BmLvBossBugFlyBase", flySoundVol, iIRCCutoff);
        }

        MR::startActionSound(getHost(), "BmLvBossBugFlyAdd", flySoundVol, iIRCCutoff);
        MR::startActionSound(getHost(), "BmLvBossBugFlyAlarm", flySoundVol, iIRCCutoff);
        break;
    case 1:
        MR::startActionSound(getHost(), "BmLvBossBugFlyRotate", flySoundVol, iIRCCutoff);
        MR::startActionSound(getHost(), "BmLvBossBugFlyRotate2");
        MR::startActionSound(getHost(), "BmLvBossBugFlyAdd", flySoundVol, iIRCCutoff);
        MR::startActionSound(getHost(), "BmLvBossBugFlyAlarm", flySoundVol, iIRCCutoff);
        break;
    case 2:
        MR::startActionSound(getHost(), "BmLvBossBugFlyRoll", flySoundVol, iIRCCutoff);
        MR::startActionSound(getHost(), "BmLvBossBugFlyAdd", flySoundVol, iIRCCutoff);
        MR::startActionSound(getHost(), "BmLvBossBugFlyAlarm", flySoundVol, iIRCCutoff);
        break;
    case 3:
        if (isAngry()) {
            MR::startActionSound(getHost(), "BmLvBossBugFlyShake2", flySoundVol, iIRCCutoff);
        } else {
            MR::startActionSound(getHost(), "BmLvBossBugFlyShake", flySoundVol, iIRCCutoff);
        }

        MR::startActionSound(getHost(), "BmLvBossBugFlyAdd", flySoundVol, iIRCCutoff);
        MR::startActionSound(getHost(), "BmLvBossBugFlyAlarm", flySoundVol, iIRCCutoff);
    }
}
