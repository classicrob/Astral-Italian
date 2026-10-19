#include "Game/Boss/DodoryuStateLv1.hpp"
#include "Game/Util.hpp"
#include "Game/Boss/Dodoryu.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    static const s32 sStopSceneFrame = 10;
    static const f32 sKnockSpeed = 10.0f;
    static const f32 sSpeed = 8.0f;
    static const f32 sAppearSpeed = 4.0f;
    static const f32 sRailAccel = 0.1f;
    static const s32 sHideTermMin = 150;
    static const s32 sHideTermMax = 200;
    static const s32 sAppearStartAttackFrame = 16;
    static const s32 sAppearTermMin = 15;
    static const s32 sAppearTermMax = 30;
    static const s32 sSwoonTerm = 180;
    static const s32 sDiveShadowOffFrame = 50;
    static const s32 sSpinOutEndFrame = 80;
    static const s32 sAppearSignFrame = 30;
    static const s32 sDiveAttackInvalidFrame = 20;
    // static const f32 sPopSpeed = _;
};  // namespace

namespace {
    NEW_NERVE_ONEND(DodoryuStateLv1NrvHide, DodoryuStateLv1, Hide, Hide);
    NEW_NERVE(DodoryuStateLv1NrvAppearSign, DodoryuStateLv1, AppearSign);
    NEW_NERVE_ONEND(DodoryuStateLv1NrvAppearStart, DodoryuStateLv1, AppearStart, AppearStart);
    NEW_NERVE_ONEND(DodoryuStateLv1NrvAppear, DodoryuStateLv1, Appear, Appear);
    NEW_NERVE(DodoryuStateLv1NrvAppearEnd, DodoryuStateLv1, AppearEnd);
    NEW_NERVE(DodoryuStateLv1NrvJumpOut, DodoryuStateLv1, JumpOut);
    NEW_NERVE(DodoryuStateLv1NrvDamage, DodoryuStateLv1, Damage);
    NEW_NERVE(DodoryuStateLv1NrvDive, DodoryuStateLv1, Dive);
    NEW_NERVE(DodoryuStateLv1NrvSpinOut, DodoryuStateLv1, SpinOut);
};  // namespace

DodoryuStateLv1::DodoryuStateLv1(Dodoryu* pHost, const char* pName)
    : DodoryuStateBase(pHost, pName), mHideStep(), mAppearStep(), _A4(), mIsValidPop(), mPopAccel(), mPopSpeed(), mRailSpeed(::sSpeed),
      mRailSpeedMax(::sSpeed) {
    initWithoutIter();
}

void DodoryuStateLv1::init(const JMapInfoIter& rIter) {
    MR::invalidateClipping(this);
    initNerve(GET_NERVE_ANON(DodoryuStateLv1NrvHide));
    makeActorAppeared();
}

void DodoryuStateLv1::control() {
    if (_A4 && !MR::isPlayerStaggering()) {
        _A4 = false;
    }

    if (mRailSpeedMax > mRailSpeed) {
        mRailSpeed += ::sRailAccel;

        if (mRailSpeed > mRailSpeedMax) {
            mRailSpeed = mRailSpeedMax;
        }
    }

    mHost->setMoveRailSpeed(mRailSpeed);
}

void DodoryuStateLv1::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::invalidateShadowAll(mHost);
        mHost->startHill();
        setRailSpeed(::sSpeed);
        mHost->shiftMoveStateRail(mRailSpeed);

        mHideStep = MR::getRandom(::sHideTermMin, ::sHideTermMax);

        MR::shakeCameraInfinity(this, 0.2f, 2.0f);
        MR::startBva(mHost, "Normal");
    }

    if (MR::isBckOneTimeAndStopped(mHost)) {
        startAnimEffectWalk();
        MR::startBva(mHost, "Normal");
    }

    mHost->tryRumblePad();
    MR::startActionSound(mHost, "BmLvDodoryuMoveUShallow", -1, -1, -1);

    if (MR::isGreaterEqualStep(this, mHideStep)) {
        setNerve(GET_NERVE_ANON(DodoryuStateLv1NrvAppearSign));
    }
}

void DodoryuStateLv1::endHide() {
    MR::stopShakingCamera(this);
}

void DodoryuStateLv1::exeAppearSign() {
    if (MR::isFirstStep(this)) {
        startAnimEffectWalkHide();
        mHost->stopHill();
    }

    if (MR::isGreaterEqualStep(this, ::sAppearSignFrame)) {
        setNerve(GET_NERVE_ANON(DodoryuStateLv1NrvAppearStart));
    }
}

void DodoryuStateLv1::exeAppearStart() {
    if (MR::isFirstStep(this)) {
        startAnimAppear();
        MR::shakeCameraNormal();
        setRailSpeed(::sAppearSpeed);
        mHost->shiftMoveStateRail(mRailSpeed);
        mHost->startHill();
        mHost->validateStarPieceSensor();
    }

    MR::startActionSound(mHost, "BmLvDodoryuMoveUShallow", -1, -1, -1);

    if (MR::isActionEnd(mHost)) {
        setNerve(GET_NERVE_ANON(DodoryuStateLv1NrvAppear));
    }
}

void DodoryuStateLv1::endAppearStart() {
    mHost->invalidateStarPieceSensor();
}

void DodoryuStateLv1::exeAppear() {
    if (MR::isFirstStep(this)) {
        startAnimWalk();

        mAppearStep = MR::getRandom(::sAppearTermMin, ::sAppearTermMax);

        mHost->validateStarPieceSensor();
    }

    MR::startActionSound(mHost, "BmLvDodoryuMoveGround", -1, -1, -1);

    if (MR::isGreaterEqualStep(this, mAppearStep)) {
        setNerve(GET_NERVE_ANON(DodoryuStateLv1NrvAppearEnd));
    }
}

void DodoryuStateLv1::endAppear() {
    mHost->invalidateStarPieceSensor();
}

void DodoryuStateLv1::exeAppearEnd() {
    if (MR::isFirstStep(this)) {
        startAnimHide();
        MR::startBva(mHost, "Normal");
    }

    if (MR::isActionEnd(mHost)) {
        startAnimEffectWalkAppear();
        setNerve(GET_NERVE_ANON(DodoryuStateLv1NrvHide));
    }
}

void DodoryuStateLv1::exeJumpOut() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "SwoonStart");
        MR::startActionSound(mHost, "BmDodoryuJumpOut", -1, -1, -1);
        MR::startActionSound(mHost, "BvDodoryuHipdrpDamage", -1, -1, -1);
        MR::validateShadowAll(mHost);
        mHost->stopHillForce();
        mHost->killLeadHill();
        mHost->shiftMoveStateNull();
        MR::startBva(mHost, "Cry");
    }

    if (MR::isActionEnd(mHost)) {
        mPopAccel = 0.0f;
        mPopSpeed = 0.0f;

        setNerve(GET_NERVE_ANON(DodoryuStateLv1NrvDamage));
    }
}

void DodoryuStateLv1::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Swoon");
        mHost->stopHillForce();
    }

    updatePop();
    MR::startActionSound(mHost, "BmLvDodoryuSwoon", -1, -1, -1);
    MR::startActionSound(mHost, "BvLvDodoryuSwoon", -1, -1, -1);

    if (MR::isGreaterEqualStep(this, ::sSwoonTerm)) {
        setNerve(GET_NERVE_ANON(DodoryuStateLv1NrvDive));
    }
}

void DodoryuStateLv1::exeDive() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "SwoonEnd");
        MR::startActionSound(mHost, "BvDodoryuWakeup", -1, -1, -1);
        MR::startBva(mHost, "Normal");
    }

    if (MR::isStep(this, ::sDiveShadowOffFrame)) {
        MR::invalidateShadowAll(mHost);
    }

    updatePop();

    if (MR::isActionEnd(mHost)) {
        startAnimEffectWalkAppear();
        setNerve(GET_NERVE_ANON(DodoryuStateLv1NrvHide));
    }
}

void DodoryuStateLv1::exeSpinOut() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "PunchDamage");
        MR::startBlowHitSound(mHost);
        mHost->stopHillForce();
        MR::startBva(mHost, "Cry");
        mHost->startSpinOutCamera();
        MR::startActionSound(mHost, "BvDodoryuSpinDmg1", -1, -1, -1);
        mHost->invalidateEyeSensor();
    }

    TPos3f mtx = mHost->mBaseMtx;
    TVec3f trans;
    mtx.getTrans(trans);
    trans.add(_98 * ::sKnockSpeed);
    mtx.setTrans(trans);

    mHost->setMtx(mtx);
    mHost->keepOffFromClosedArea(nullptr);
    mHost->snapToGround();
    calcVerticalizedDir(&_98, _98);

    if (MR::isStep(this, 60)) {
        mHost->validateEyeSensor();
    }

    if (MR::isGreaterEqualStep(this, ::sSpinOutEndFrame)) {
        mHost->endSpinOutCamera();
        mHost->nextState();
        mHost->validateEyeSensor();
    }
}

void DodoryuStateLv1::start() {
    mHost->shiftMoveStateRail(10.0f);
    mHost->endPlayerCenterCamera();
    setNerve(GET_NERVE_ANON(DodoryuStateLv1NrvHide));
    mHost->invalidateStarPieceSensor();
}

void DodoryuStateLv1::catchAttackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorPlayerOrRide(pReceiver)) {
        return;
    }

    if (mHost->isHeadNeedle(pSender, pReceiver)) {
        if (!MR::sendMsgEnemyAttack(pReceiver, pSender)) {
            MR::sendMsgPush(pReceiver, pSender);
        }

        return;
    }

    if (!isAttackable() || _A4) {
        MR::sendMsgPush(pReceiver, pSender);
        return;
    }

    TVec3f dir;
    calcAttackDir(&dir, pSender->mPosition, pReceiver->mPosition);

    if (MR::sendMsgEnemyAttackStrongToDir(pReceiver, pSender, dir)) {
        MR::shakeCameraNormal();
        return;
    }

    MR::sendMsgPush(pReceiver, pSender);
}

void DodoryuStateLv1::catchHipDrop() {
    setRailSpeed(0.0f);

    bool isNerves = isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvHide)) || isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvAppearSign));

    if (isNerves) {
        executeHideJump();
        setNerve(GET_NERVE_ANON(DodoryuStateLv1NrvHide));
        return;
    }

    if (isHipDropReactionJumpOut()) {
        mHost->reactJumpOutCommon();
        setNerve(GET_NERVE_ANON(DodoryuStateLv1NrvJumpOut));
        return;
    }

    if (isHipDropReactionPop()) {
        startPop();
        return;
    }

    if (isHipDropReactionNothing()) {
        return;
    }
}

bool DodoryuStateLv1::catchPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    bool isAppear = isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvAppearStart)) || isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvAppear)) ||
                    isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvAppearEnd));

    if (isAppear && MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        if (MR::sendMsgEnemyAttackCounterSpin(pSender, pReceiver)) {
            _A4 = true;

            return true;
        }

        return false;
    }

    bool isNerves = isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvJumpOut)) || isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvDamage));

    if (!isNerves) {
        return false;
    }

    if (mHost->isHeadNeedle(pReceiver, pSender)) {
        return false;
    }

    if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
        return true;
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        _98.set(pReceiver->mPosition - pSender->mPosition);

        if (!calcVerticalizedDir(&_98, _98)) {
            mHost->mBaseMtx.getZDir(_98);
            _98.negate();
        }

        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        MR::stopScene(::sStopSceneFrame);
        setNerve(GET_NERVE_ANON(DodoryuStateLv1NrvSpinOut));

        return true;
    }

    return false;
}

void DodoryuStateLv1::catchStartClipped() {
}

bool DodoryuStateLv1::isAttackable() const {
    return isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvHide)) ||
           (isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvAppearStart)) && MR::isGreaterStep(this, ::sAppearStartAttackFrame)) ||
           isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvAppear)) || isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvAppearEnd)) ||
           (isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvDive)) && MR::isGreaterStep(this, ::sDiveAttackInvalidFrame));
}

void DodoryuStateLv1::executeHideJump() {
    mHost->appearBank();
    MR::startAction(mHost, "Jump");
    mHost->startLeadHillBck("Jump");
    MR::startBva(mHost, "Cry");
    MR::startActionSound(mHost, "BmDodoryuHipdrpReaction", -1, -1, -1);
}

void DodoryuStateLv1::startPop() {
    mIsValidPop = true;
    mPopAccel = 30.0f;
    mPopSpeed = 0.0f;
}

void DodoryuStateLv1::updatePop() {
    if (!mIsValidPop) {
        return;
    }

    TPos3f mtx = mHost->mBaseMtx;
    TVec3f trans;
    mtx.getTrans(trans);
    trans.add(-mHost->mGravity * mPopAccel);

    mPopSpeed += mPopAccel;
    mPopAccel -= 2.5f;

    if (mPopSpeed < 0.0f) {
        trans.add(mHost->mGravity * mPopSpeed);

        MR::shakeCameraNormal();

        mIsValidPop = false;

        MR::emitEffect(mHost, "SmallLandSmoke");
    }

    mtx.setTrans(trans);
    mHost->setMtx(mtx);
}

bool DodoryuStateLv1::isHipDropReactionJumpOut() const {
    f32 dotP = MR::getPlayerGravity()->dot(mHost->mGravity);

    if (dotP < 0.0f) {
        return false;
    }

    return isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvAppearStart)) || isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvAppear)) ||
           isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvAppearEnd));
}

bool DodoryuStateLv1::isHipDropReactionPop() const {
    f32 dotP = MR::getPlayerGravity()->dot(mHost->mGravity);

    if (dotP < 0.0f) {
        return false;
    }

    return isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvDamage));
}

bool DodoryuStateLv1::isHipDropReactionNothing() const {
    f32 dotP = MR::getPlayerGravity()->dot(mHost->mGravity);

    if (dotP < 0.0f) {
        return true;
    }

    return isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvJumpOut)) || isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvDamage)) ||
           isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvDive)) || isNerve(GET_NERVE_ANON(DodoryuStateLv1NrvSpinOut));
}

void DodoryuStateLv1::setRailSpeed(f32 railSpeed) {
    mRailSpeedMax = railSpeed;

    if (mRailSpeed > railSpeed) {
        mRailSpeed = railSpeed;
    }
}

void DodoryuStateLv1::startAnimAppear() {
    MR::startAction(mHost, "OpeningAppear");
    MR::startBtk(mHost, "OpeningAppear");
    mHost->startLeadHillBck("Appear");
    MR::startActionSound(mHost, "BmDodoryuAppearHead", -1, -1, -1);
    MR::startActionSound(mHost, "BvDodoryuAppear1", -1, -1, -1);
}

void DodoryuStateLv1::startAnimWalk() {
    MR::startAction(mHost, "OpeningWalk");
    MR::startBtk(mHost, "OpeningWalk");
    mHost->startLeadHillBck("Walk");
}

void DodoryuStateLv1::startAnimHide() {
    MR::startAction(mHost, "Hide");
    mHost->startLeadHillBck("Hide");
}

void DodoryuStateLv1::startAnimEffectWalkAppear() {
    MR::startAction(mHost, "EffectWalkAppear");
    mHost->startLeadHillBck("EffectWalkAppear");
}

void DodoryuStateLv1::startAnimEffectWalk() {
    MR::startAction(mHost, "EffectWalk");
    mHost->startLeadHillBck("EffectWalk");
}

void DodoryuStateLv1::startAnimEffectWalkHide() {
    MR::startAction(mHost, "EffectWalkHide");
    mHost->startLeadHillBck("EffectWalkHide");
}
