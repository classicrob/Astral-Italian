#include "Game/MapObj/PunchingKinoko.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/Util.hpp"
#include "JSystem/JGeometry/TVec.hpp"
#include <revolution.h>

void PunchingKinoko_FORCE_EMIT_SCALE() {
    TVec3f vec;
    vec.scale(0.0f);
}

namespace NrvPunchingKinoko {
    NEW_NERVE(PunchingKinokoNrvCrushedShrink, PunchingKinoko, CrushedShrink);
    NEW_NERVE(PunchingKinokoNrvCrushedFlat, PunchingKinoko, CrushedFlat);
    NEW_NERVE(PunchingKinokoNrvCrushedRecover, PunchingKinoko, CrushedRecover);
    NEW_NERVE(PunchingKinokoNrvWait, PunchingKinoko, Wait);
    NEW_NERVE(PunchingKinokoNrvSwing, PunchingKinoko, Swing);
    NEW_NERVE(PunchingKinokoNrvPunched, PunchingKinoko, Punched);
    NEW_NERVE(PunchingKinokoNrvPunchedBrake, PunchingKinoko, PunchedBrake);
    NEW_NERVE(PunchingKinokoNrvHitted, PunchingKinoko, Hitted);
    NEW_NERVE(PunchingKinokoNrvPointSnaped, PunchingKinoko, PointSnaped);
    NEW_NERVE(PunchingKinokoNrvCrushed, PunchingKinoko, Crushed);
    NEW_NERVE(PunchingKinokoNrvCrushedEnd, PunchingKinoko, CrushedEnd);
};  // namespace NrvPunchingKinoko

PunchingKinoko::PunchingKinoko(const char* pName)
    : LiveActor(pName), mGroundChecker(nullptr), mScaleController(nullptr), mDelegator(nullptr), mCameraNo(-1), mCameraPos(0, 0, 0), mUpDir(0, 1, 0) {
    mStarPointerHitCoolDown = 0;
    _C0 = true;
    mInvincibleHitCoolDown = -1;
}

void PunchingKinoko::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "PunchingKinoko", false);
    MR::calcGravity(this);
    mCameraPos.set(mPosition);

    mGroundChecker = new GroundChecker("頭コリジョン", 70.0f, 0.0f);
    MR::calcPositionUpOffset(&mGroundChecker->mPosition, this, 130.0f);
    MR::resetPosition(mGroundChecker);
    MR::onCalcGravity(mGroundChecker);

    MR::setShadowDropPositionPtr(this, "頭", &mGroundChecker->mPosition);
    MR::setShadowDropDirectionPtr(this, "頭", &mGroundChecker->mGravity);
    MR::tryRegisterStarLightReceiver(getSensor("Body"));

    MR::getJMapInfoArg7WithInit(rIter, &mCameraNo);
    if (mCameraNo != -1) {
        MR::declareCameraRegisterVec(this, mCameraNo, &mCameraPos);
    }

    mDelegator = MR::createJointDelegatorWithNullChildFunc(this, &PunchingKinoko::ballMtxCallBack, "Ball");

    mScaleController = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    initNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvWait), 1);
    MR::initActorState(this, mBindStarPointer, GET_NERVE(PunchingKinoko, PunchingKinokoNrvPointSnaped), "SupportFreeze");

    if (MR::useStageSwitchReadB(this, rIter)) {
        MR::listenStageSwitchOnB(this, MR::Functor_Inline(this, &PunchingKinoko::kill));
    }

    makeActorAppeared();
}

bool PunchingKinoko::ballMtxCallBack(TPos3f* pMtx, const JointControllerInfo& joint) {
    TVec3f stack_8;
    stack_8.sub(mGroundChecker->mPosition, mPosition);
    MR::orthogonalize(pMtx);
    if (!MR::normalizeOrZero(&stack_8)) {
        MR::turnMtxToYDirRate(pMtx, stack_8, 1.0f);
    }

    TPos3f stack_14;
    stack_14.identity();

    MR::scaleMtxToDir(&stack_14, mUpDir, mScaleController->mScale);
    pMtx->concat(stack_14, *pMtx);
    pMtx->setTrans(mGroundChecker->mPosition);

    return true;
}

void PunchingKinoko::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    mGroundChecker->makeActorAppeared();
}

void PunchingKinoko::kill() {
    LiveActor::kill();
}

void PunchingKinoko::makeActorDead() {
    LiveActor::makeActorDead();
    mGroundChecker->makeActorDead();
}

void PunchingKinoko::control() {
    if (mStarPointerHitCoolDown > 0) {
        mStarPointerHitCoolDown -= 1;
    }
    mScaleController->update();
    mGroundChecker->movement();
    MR::reboundVelocityFromCollision(mGroundChecker, 0.0f, 0.0f, 1.0f);
    if (mInvincibleHitCoolDown >= 0) {
        mInvincibleHitCoolDown -= 1;
    }
}

void PunchingKinoko::calcAndSetBaseMtx() {
    TPos3f stack_14;
    TVec3f stack_8;
    stack_8.negate(mGravity);
    MR::makeMtxUpNoSupportPos(&stack_14, stack_8, mPosition);
    MR::setBaseTRMtx(this, stack_14);
    mDelegator->registerCallBack();
}

void PunchingKinoko::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isCrushed()) {
        if (pSender == getSensor("Body")) {
            if (isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvWait))) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else if (!isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedEnd)) || !MR::isSensorPlayer(pReceiver) ||
                   !MR::sendMsgEnemyAttackFlipWeakJump(pReceiver, pSender)) {
            TVec3f stack_3C;
            TVec3f stack_30;
            f32 stack_8;
            MR::separateScalarAndDirection(&stack_8, &stack_3C, mGroundChecker->mVelocity);
            bool hit = false;
            if (MR::isSensorPlayer(pReceiver)) {
                if (isEnableHitPlayer()) {
                    if (stack_8 >= 30.0f) {
                        if (stack_8 >= 45.0f) {
                            hit = MR::sendMsgEnemyAttackFlipMaximumToDir(pReceiver, pSender, stack_3C.multInLine(70.0f));
                        } else {
                            hit = MR::sendMsgEnemyAttackFlipToDir(pReceiver, pSender, stack_3C * 70.0f);
                        }
                        if (hit) {
                            MR::startSoundPlayer("SE_PM_WALL_HIT_BODY", -1, -1);
                            MR::scatterStarPiecePlayer(10);
                        }
                    } else {
                        if (stack_8 >= 15.0f) {
                            hit = MR::sendMsgEnemyAttackFlipWeak(pReceiver, pSender);
                        } else {
                            MR::sendMsgPush(pReceiver, pSender);
                        }
                    }
                } else {
                    MR::sendMsgPush(pReceiver, pSender);
                }
            } else {
                if (isEnableEnemyAttack()) {
                    hit = MR::sendMsgToEnemyAttackBlow(pReceiver, pSender);
                }
                if (!hit) {
                    MR::sendMsgPush(pReceiver, pSender);
                }
            }
            if (hit) {
                MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, "Hit");
                MR::calcSensorDirectionNormalize(&stack_30, pReceiver, pSender);
                f32 dot = mGroundChecker->mVelocity.dot(stack_30) * 1.6f;
                mGroundChecker->mVelocity -= stack_30 * dot;
                mGroundChecker->mVelocity.mult(0.3f);
                setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvWait));
            }
        }
    }
}

bool PunchingKinoko::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isCrushed()) {
        return false;
    }

    if (pReceiver == getSensor("Body")) {
        return false;
    }

    if (MR::isMsgInvincibleAttack(msg) || MR::isMsgRockAttack(msg)) {
        if (mInvincibleHitCoolDown < 0 && isEnableBlowed()) {
            MR::startBlowHitSound(this);
            mInvincibleHitCoolDown = 10;
        }

        return requestEnemyBlow(pSender, pReceiver);
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        return requestCrush();
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        TVec3f stack_14;
        MR::calcSensorDirectionNormalize(&stack_14, pSender, pReceiver);
        f32 dot = mGroundChecker->mVelocity.dot(stack_14);

        if (dot < 15.0f) {
            mGroundChecker->mVelocity.add(stack_14 * 15.0f);
        }

        MR::startActionSound(this, "OjPncKinokoBound");
        return true;
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        return requestPunch(pSender, pReceiver);
    }

    if (MR::isMsgPlayerTrample(msg)) {
        return requestTrample(pSender, pReceiver);
    }

    return false;
}

bool PunchingKinoko::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isCrushed()) {
        return false;
    }

    if (pReceiver == getSensor("Body")) {
        return false;
    }

    if (MR::isMsgToEnemyAttackTrample(msg)) {
        return requestCrush();
    }

    if (MR::isMsgToEnemyAttackBlow(msg)) {
        return requestEnemyBlow(pSender, pReceiver);
    }

    return false;
}

bool PunchingKinoko::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (isCrushedOrPointSnaped()) {
        return false;
    }

    if (pReceiver == getSensor("Body")) {
        return false;
    }

    TVec3f stack_14;
    MR::calcSensorHorizonNormalize(&stack_14, mGravity, pSender, pReceiver);
    f32 fVar1 = stack_14.dot(mGroundChecker->mVelocity);
    if (fVar1 < 0.0f) {
        fVar1 = 3.0f;
    } else {
        fVar1 = 3.0f - fVar1;
    }

    if (fVar1 > 0.0f) {
        mGroundChecker->mVelocity.add(stack_14 * fVar1);
        return true;
    }

    return false;
}

bool PunchingKinoko::requestPunch(HitSensor* pOtherSensor, HitSensor* pMySensor) {
    if (isCrushed()) {
        return false;
    }

    if (pMySensor == getSensor("Body")) {
        return false;
    }

    if (isEnablePunched()) {
        MR::invalidateClipping(this);
        TVec3f stack_14;
        MR::calcSensorHorizonNormalize(&stack_14, mGravity, pOtherSensor, pMySensor);
        mGroundChecker->mVelocity.add(stack_14 * 50.0f);
        MR::startSpinHitSound(this);
        MR::startBlowHitSound(this);
        MR::tryRumblePadStrong(this, WPAD_CHAN0);
        MR::stopScene(5);
        setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvPunched));

        return true;
    }

    return false;
}

bool PunchingKinoko::requestEnemyBlow(HitSensor* pOtherSensor, HitSensor* pMySensor) {
    if (!isEnableBlowed()) {
        return false;
    }

    MR::invalidateClipping(this);
    TVec3f stack_14;
    MR::calcSensorHorizonNormalize(&stack_14, mGravity, pOtherSensor, pMySensor);
    mGroundChecker->mVelocity.add(stack_14 * 25.0f);
    if (stack_14.length() >= 0.5f) {
        MR::startActionSound(this, "OjPncKinokoHitSelf");
        setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvWait));
    }

    return true;
}

bool PunchingKinoko::requestTrample(HitSensor* pOtherSensor, HitSensor* pMySensor) {
    if (isEnableTrample()) {
        TVec3f stack_14;
        MR::calcSensorDirectionNormalize(&stack_14, pOtherSensor, pMySensor);

        mGroundChecker->mVelocity.add(stack_14 * 5.0f);
        return true;
    }
    return false;
}

bool PunchingKinoko::requestCrush() {
    if (isEnableCrushed()) {
        MR::invalidateClipping(this);
        setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushed));
        return true;
    }
    return false;
}

bool PunchingKinoko::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarLightGrow(msg)) {
        return requestCrushedRecover();
    }

    if (MR::isMsgStarLightShrink(msg)) {
        return requestCrushedShrink();
    }

    return false;
}

bool PunchingKinoko::requestCrushedShrink() {
    if (isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedFlat)) || isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedShrink))) {
        return false;
    }

    MR::invalidateClipping(this);
    setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedShrink));
    return true;
}

bool PunchingKinoko::requestCrushedRecover() {
    if (isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedFlat)) || isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedShrink))) {
        MR::invalidateClipping(this);
        setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedRecover));
        return true;
    }

    return false;
}

void PunchingKinoko::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startBrk(this, "Normal");
        MR::validateClipping(this);
        _C0 = true;
    }

    addVelocityToUpPos(130.0f, 0.008f, 0.94f);
    MR::vecBlend(&mCameraPos, mCameraPos, mPosition, 0.05f);

    if (mBindStarPointer->tryStartPointBind()) {
        MR::zeroVelocity(mGroundChecker);
        setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvPointSnaped));
    }
}

void PunchingKinoko::exeSwing() {
    f32 var2 = MR::calcVelocityLength(mGroundChecker);
    TVec3f stack_20;
    MR::calcPositionUpOffset(&stack_20, this, 130.0f);

    TVec3f stack_14 = stack_20 - mGroundChecker->mPosition;
    MR::addVelocity(mGroundChecker, stack_14 * 0.008f);
    MR::attenuateVelocity(mGroundChecker, 0.97f);
    f32 var3 = MR::calcVelocityLength(mGroundChecker);

    const f32 f0 = 20.0f;
    if (var2 < f0 && var3 >= f0) {
        MR::startActionSound(this, "OjPncKinokoBound");
    }

    MR::vecBlend(mCameraPos, mPosition, &mCameraPos, 0.05f);

    if (var3 < 40.0f && stack_14.length() < 50.0f) {
        setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvWait));
    }
}

void PunchingKinoko::exePointSnaped() {
    MR::zeroVelocity(mGroundChecker);
    MR::updateActorStateAndNextNerve(this, GET_NERVE(PunchingKinoko, PunchingKinokoNrvWait));
}

void PunchingKinoko::exePunched() {
    if (MR::isFirstStep(this)) {
    }

    MR::startActionSound(this, "OjLvPncKinokoPunched");
    addVelocityKeepHeight();
    MR::attenuateVelocity(mGroundChecker, 0.99f);
    HitSensor* sensor = getSensor("Head");
    MR::sendMsgEnemyAttackToBindedSensor(mGroundChecker, sensor);
    mCameraPos.set(mGroundChecker->mPosition);

    if (MR::isGreaterStep(this, 5)) {
        setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvPunchedBrake));
    }
}

void PunchingKinoko::exePunchedBrake() {
    addVelocityKeepHeight();
    MR::attenuateVelocity(mGroundChecker, 0.9f);
    MR::startActionSound(this, "OjLvPncKinokoPunched");
    mCameraPos.set(mGroundChecker->mPosition);
    if (!MR::isGreaterStep(this, 40)) {
        HitSensor* sensor = getSensor("Head");
        if (!MR::sendMsgEnemyAttackToBindedSensor(mGroundChecker, sensor)) {
            return;
        }
    }
    MR::startActionSound(this, "OjPncKinokoReturn");
    setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvSwing));
}

void PunchingKinoko::exeHitted() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OjPncKinokoHitSelf");
    }
    addVelocityKeepHeight();
    MR::attenuateVelocity(mGroundChecker, 0.99f);
    if (MR::isGreaterStep(this, 5)) {
        setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvWait));
    }
}

void PunchingKinoko::exeCrushed() {
    if (MR::isFirstStep(this)) {
        MR::startBrk(this, "Press");

        if (MR::isShadowProjected(this, "頭")) {
            MR::getShadowProjectionNormal(this, "頭", &mUpDir);
            MR::getShadowProjectionPos(this, "頭", &mGroundChecker->mPosition);
            mGroundChecker->mPosition.add(mUpDir * 20.0f);
        } else {
            mUpDir.set(mGravity);
        }

        MR::offBind(mGroundChecker);
        mScaleController->startCrush();
        MR::startActionSound(this, "OjPncKinokoCrash");
    }

    MR::zeroVelocity(mGroundChecker);

    if (MR::isGreaterStep(this, 180)) {
        setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedEnd));
    }
}

void PunchingKinoko::exeCrushedEnd() {
    if (MR::isFirstStep(this)) {
    }

    if (MR::isStep(this, 10)) {
        MR::startBrk(this, "Revival");
        mScaleController->startAnim();
        MR::startActionSound(this, "OjPncKinokoRecover");
    }

    addVelocityToUpPos(130.0f, 0.008f, 0.94f);

    if (MR::isGreaterStep(this, 60)) {
        setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvWait));
        MR::onBind(mGroundChecker);
    }
}

void PunchingKinoko::addVelocityToUpPos(f32 height, f32 gain, f32 attenuate) {
    TVec3f up;
    MR::calcPositionUpOffset(&up, this, height);
    TVec3f* pGroundPos = &mGroundChecker->mPosition;
    TVec3f diff(up);
    diff.sub(*pGroundPos);
    TVec3f vel(diff);
    vel.scale(gain);
    MR::addVelocity(mGroundChecker, vel);
    MR::attenuateVelocity(mGroundChecker, attenuate);
}

void PunchingKinoko::exeCrushedShrink() {
    if (MR::isFirstStep(this)) {
        MR::offBind(mGroundChecker);
    }

    f32 ease = MR::calcNerveEaseInOutValue(this, 40, 1.0f, 0.4f);
    mScaleController->stop(TVec3f(ease, ease, ease));
    MR::setShadowVolumeSphereRadius(this, "頭", 70.0f * ease);
    addVelocityToUpPos(40.0f, 0.005f, 0.9f);

    if (MR::isGreaterStep(this, 40)) {
        setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedFlat));
    }
}

void PunchingKinoko::exeCrushedFlat() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        mScaleController->stop(TVec3f(0.4f, 0.4f, 0.4f));
        MR::setShadowVolumeSphereRadius(this, "頭", 28.0f);
    }

    addVelocityToUpPos(40.0f, 0.005f, 0.9f);
}

void PunchingKinoko::exeCrushedRecover() {
    f32 ease = MR::calcNerveEaseOutValue(this, 20, 0.4f, 1.0f);
    mScaleController->stop(TVec3f(ease, ease, ease));
    MR::setShadowVolumeSphereRadius(this, "頭", 70.0f * ease);
    addVelocityToUpPos(130.0f, 0.008f, 0.94f);

    if (MR::isGreaterStep(this, 20)) {
        MR::onBind(mGroundChecker);
        setNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvWait));
    }
}

void PunchingKinoko::addVelocityKeepHeight() {
    if (MR::isShadowProjected(this, "頭")) {
        TVec3f stack_8;
        MR::getShadowProjectionPos(this, "頭", &stack_8);
        MR::addVelocityKeepHeight(mGroundChecker, stack_8, 130.0f, 0.5f, 60.0f);
    }
}

bool PunchingKinoko::isEnablePunched() const {
    if (isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvWait)) || isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvPointSnaped)) ||
        isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvSwing))) {
        return true;
    }
    return false;
}

bool PunchingKinoko::isEnableHitPlayer() const {
    return isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvSwing));
}

bool PunchingKinoko::isEnableEnemyAttack() const {
    if (isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvSwing)) || isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvPunched)) ||
        isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvPunchedBrake))) {
        return true;
    }

    return false;
}

bool PunchingKinoko::isEnableCrushed() const {
    if (isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvWait)) || isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvPointSnaped)) ||
        isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedEnd))) {
        return true;
    }

    return false;
}

bool PunchingKinoko::isEnableTrample() const {
    if (isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushed)) || isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedEnd))) {
        return false;
    }

    return true;
}

bool PunchingKinoko::isEnableBlowed() const {
    return isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvWait));
}

bool PunchingKinoko::isCrushedOrPointSnaped() const {
    if (isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushed)) || isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedShrink)) ||
        isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedFlat)) || isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvPointSnaped)) ||
        isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedRecover))) {
        return true;
    }

    return false;
}

bool PunchingKinoko::isCrushed() const {
    if (isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushed)) || isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedShrink)) ||
        isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedFlat)) || isNerve(GET_NERVE(PunchingKinoko, PunchingKinokoNrvCrushedRecover))) {
        return true;
    }

    return false;
}

PunchingKinoko::~PunchingKinoko() {
}
