#include "Game/Enemy/SamboHead.hpp"
void fn_801F9370(LiveActor* pActor);
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/SpinHitController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include "Game/LiveActor/ActorItemGenerator.hpp"


namespace NrvSamboHead {
    NEW_NERVE(HostTypeNrvWaitUnderGround, SamboHead, WaitUnderGround);
    NEW_NERVE(HostTypeNrvHide, SamboHead, Hide);
    NEW_NERVE(HostTypeNrvAppear, SamboHead, Appear);
    NEW_NERVE(HostTypeNrvAppearImmediately, SamboHead, Appear);
    NEW_NERVE(HostTypeNrvWait, SamboHead, Wait);
    NEW_NERVE(HostTypeNrvPrePursue, SamboHead, PrePursue);
    NEW_NERVE(HostTypeNrvPursue, SamboHead, Pursue);
    NEW_NERVE(HostTypeNrvPursueEnd, SamboHead, PursueEnd);
    NEW_NERVE(HostTypeNrvHitToPlayer, SamboHead, HitToPlayer);
    NEW_NERVE(HostTypeNrvStarPieceHit, SamboHead, StarPieceHit);
    NEW_NERVE_ONEND(HostTypeNrvStampFall, SamboHead, StampFall, StampFall);
    NEW_NERVE_ONEND(HostTypeNrvStampDeath, SamboHead, StampDeath, StampDeath);
    NEW_NERVE_ONEND(HostTypeNrvHitBlow, SamboHead, HitBlow, HitBlow);
    NEW_NERVE(HostTypeNrvBuryDeath, SamboHead, BuryDeath);
    NEW_NERVE_ONEND(HostTypeNrvBindStarPointer, SamboHead, BindStarPointer, BindStarPointer);
    NEW_NERVE(HostTypeNrvBindStarPointerEnd, SamboHead, BindStarPointerEnd);
};  // namespace NrvSamboHead

SamboHead::SamboHead(const char* pName) : LiveActor(pName), mSpinCtrl(), mScaleController(), mBindStarPointer(), _98(0, 0, 1), _A4(0, 0, 0) {
    _B0 = 0;
}

void SamboHead::init(const JMapInfoIter& rIter) {
    if (MR::isValidInfo(rIter)) {
        s32 arg0 = -1;
        MR::getJMapInfoArg0NoInit(rIter, &arg0);
        if (arg0 == -1) {
            _B0 = false;
        } else {
            _B0 = true;
        }
    }

    MR::initActor(this, rIter, "SamboHead", false);
    _A4.set(mPosition);
    TPos3f mtx = getBaseMtx();
    mtx.getZDir(_98);
    initNerve(GET_NERVE(SamboHead, HostTypeNrvWaitUnderGround), 1);
    MR::onCalcGravity(this);

    mScaleController = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mBindStarPointer, GET_NERVE(SamboHead, HostTypeNrvBindStarPointer), "SupportFreeze");

    mSpinCtrl = new SpinHitController(this);

    MR::initUseStageSwitchWriteDead(this, rIter);
    MR::useStageSwitchReadA(this, rIter);
    MR::useStageSwitchAwake(this, rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void SamboHead::appear() {
    LiveActor::appear();
    setNerve(GET_NERVE(SamboHead, HostTypeNrvAppearImmediately));
}

void SamboHead::kill() {
    LiveActor::kill();
    MR::emitEffect(this, "Death");
    MR::startActionSound(this, "EmExplodeS");

    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
}

void SamboHead::control() {
    mScaleController->updateNerve();

    if (isNerveTypeDead() && MR::isPressedBetweenWalls(this)) {
        MR::emitItemHit(this);
        kill();
        return;
    }

    if (isNerveTypeDead() && MR::isBindedGroundSinkDeath(this)) {
        setNerve(GET_NERVE(SamboHead, HostTypeNrvBuryDeath));
        return;
    }

    if (isStarPointerBindable() && mBindStarPointer->tryStartPointBind()) {
        setNerve(GET_NERVE(SamboHead, HostTypeNrvBindStarPointer));
    }
}

bool SamboHead::isNerveTypeDead() const {
    if (isNerve(GET_NERVE(SamboHead, HostTypeNrvStampFall)) || isNerve(GET_NERVE(SamboHead, HostTypeNrvStampDeath)) ||
        isNerve(GET_NERVE(SamboHead, HostTypeNrvHitBlow)) || isNerve(GET_NERVE(SamboHead, HostTypeNrvBuryDeath))) {
        return false;
    }

    return true;
}

bool SamboHead::isStarPointerBindable() const {
    if (isNerve(GET_NERVE(SamboHead, HostTypeNrvWait)) || isNerve(GET_NERVE(SamboHead, HostTypeNrvPrePursue)) ||
        isNerve(GET_NERVE(SamboHead, HostTypeNrvPursue)) || isNerve(GET_NERVE(SamboHead, HostTypeNrvPursueEnd)) ||
        isNerve(GET_NERVE(SamboHead, HostTypeNrvHitToPlayer))) {
        return true;
    }

    if (isNerve(GET_NERVE(SamboHead, HostTypeNrvStarPieceHit))) {
        return MR::isGreaterStep(this, 20);
    }

    return false;
}

void SamboHead::exeWaitUnderGround() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AppearMini");
        MR::setPosition(this, _A4);
        MR::invalidateShadow(this, nullptr);
        MR::invalidateHitSensor(this, "Body");
    }

    if (!MR::isValidSwitchA(this) || MR::isOnSwitchA(this)) {
        mVelocity.zero();
        f32 dist = MR::calcDistanceToPlayer(this);

        if (MR::isGreaterStep(this, 120)) {
            if (dist < 800.0f) {
                MR::validateHitSensor(this, "Body");
                setNerve(GET_NERVE(SamboHead, HostTypeNrvAppear));
            }
        }
    }
}

void SamboHead::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        mVelocity.zero();
    }

    mVelocity.zero();

    if (MR::isStep(this, 30)) {
        MR::invalidateShadow(this, nullptr);
    }

    if (MR::isGreaterStep(this, 120)) {
        setNerve(GET_NERVE(SamboHead, HostTypeNrvWaitUnderGround));
    }
}

void SamboHead::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");

        if (isNerve(GET_NERVE(SamboHead, HostTypeNrvAppearImmediately))) {
            MR::setBckFrame(this, 45.0f);
        }
    }

    mVelocity.zero();

    if (MR::isStep(this, 63)) {
        MR::validateShadow(this, nullptr);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(SamboHead, HostTypeNrvPrePursue));
    }
}

void SamboHead::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    fn_801F9370(this);
    f32 dist = MR::calcDistanceToPlayer(this);

    if (800.0f < dist && !_B0) {
        if (MR::isGreaterStep(this, 120)) {
            setNerve(GET_NERVE(SamboHead, HostTypeNrvHide));
        }
    } else {
        if (dist < 500.0f || _B0) {
            TVec3f v4;
            MR::calcVecToPlayerH(&v4, this, nullptr);

            if (MR::turnVecToVecRadian(&_98, _98, v4, 0.05f, mGravity) < 0.015f) {
                if (_B0) {
                    setNerve(GET_NERVE(SamboHead, HostTypeNrvPursue));
                } else {
                    setNerve(GET_NERVE(SamboHead, HostTypeNrvPrePursue));
                }
            }
        } else {
            TVec3f v3;
            MR::calcVecToPlayerH(&v3, this, nullptr);
            MR::turnVecToVecRadian(&_98, _98, v3, 0.05f, mGravity);
        }
    }
}

void SamboHead::exePrePursue() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Find");
    }

    TVec3f v2;
    MR::calcVecToPlayerH(&v2, this, nullptr);
    MR::turnVecToVecRadian(&_98, _98, v2, 0.05f, mGravity);

    if (MR::isActionEnd(this) && MR::isOnGround(this)) {
        setNerve(GET_NERVE(SamboHead, HostTypeNrvPursue));
        MR::startActionSound(this, "EmSamboheadHopLand");
    } else {
        fn_801F9370(this);
    }
}

void SamboHead::exePursue() {
    TVec3f v3;
    MR::calcVecToPlayerH(&v3, this, nullptr);

    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Run");
        mVelocity.set(_98 * 7.0f);
        MR::addVelocityJump(this, 25.0f);
    }

    MR::applyVelocityDampAndGravity(this, 1.5f, 0.80f, 0.98f, 0.98f, 1.0f);
    if (MR::isBindedGround(this)) {
        MR::killVelocityOnGroundH(this);
    }
    MR::turnVecToVecRadian(&_98, _98, v3, 0.05f, mGravity);

    if (MR::isOnGround(this)) {
        setNerve(GET_NERVE(SamboHead, HostTypeNrvPursueEnd));
        MR::startActionSound(this, "EmSamboheadHopLand");
    }
}

void SamboHead::exePursueEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Land");
    }

    mVelocity.zero();

    if (MR::isActionEnd(this)) {
        if (1000.0f < MR::calcDistanceToPlayer(this)) {
            setNerve(GET_NERVE(SamboHead, HostTypeNrvWait));
        } else {
            setNerve(GET_NERVE(SamboHead, HostTypeNrvPursue));
        }
    }
}

void SamboHead::exeHitToPlayer() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hit");
        MR::startActionSound(this, "EmSamboheadAttack");
    }

    fn_801F9370(this);

    if (MR::isGreaterStep(this, 58)) {
        TVec3f v2;
        MR::calcVecToPlayerH(&v2, this, nullptr);
        MR::turnVecToVecRadian(&_98, _98, v2, 0.05f, mGravity);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(SamboHead, HostTypeNrvWait));
    }
}

void SamboHead::exeStarPieceHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StarPieceHit");
    }

    MR::startActionSound(this, "EmLvSwoonS");
    fn_801F9370(this);

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(SamboHead, HostTypeNrvWait));
    }
}

void SamboHead::exeStampFall() {
    if (MR::isFirstStep(this)) {
        TVec3f velocity(mGravity);
        velocity *= 30.0f;
        mVelocity.set(velocity);
        MR::invalidateClipping(this);
    }

    if (MR::isOnGround(this)) {
        setNerve(GET_NERVE(SamboHead, HostTypeNrvStampDeath));
    }
}

void SamboHead::exeStampDeath() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Press");
        MR::invalidateClipping(this);
    }

    MR::killVelocityOnGroundH(this);

    if (MR::isGreaterStep(this, 35)) {
        MR::emitItemStomp(this);
        kill();
    }
}

void SamboHead::exeHitBlow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Blow");
        MR::startBlowHitSound(this);
        MR::invalidateHitSensor(this, "Body");
    }

    if (mSpinCtrl->execute(this)) {
        MR::emitItemHit(this);
        MR::startActionSound(this, "EmExplodeS");
        MR::validateHitSensor(this, "Body");
    }
}

void SamboHead::exeBuryDeath() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Sink");
        mVelocity.zero();
    }

    mVelocity.zero();

    if (MR::isActionEnd(this)) {
        kill();
    }
}

void SamboHead::exeBindStarPointer() {
    if (MR::isFirstStep(this)) {
        MR::startDPDHitSound();
    }

    MR::startDPDFreezeLevelSound(this);
    MR::updateActorStateAndNextNerve(this, GET_NERVE(SamboHead, HostTypeNrvBindStarPointerEnd));
}

void SamboHead::exeBindStarPointerEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    fn_801F9370(this);

    if (MR::isOnGround(this)) {
        setNerve(GET_NERVE(SamboHead, HostTypeNrvWait));
    }
}

// SMG2 helper (unnamed in symbols): rebound, bind-ground gravity, damp the velocity.
void fn_801F9370(LiveActor* pActor) {
    MR::reboundVelocityFromCollision(pActor, 0.0f, 0.0f, 1.0f);
    if (!MR::isBindedGround(pActor)) {
        MR::addVelocityToGravity(pActor, 1.5f);
    } else {
        MR::killVelocityVertical(pActor);
    }

    TVec3f* pVelocity = &pActor->mVelocity;
    TVec3f* pGravity = &pActor->mGravity;
    TVec3f damp;
    TVec3f horizontal;
    JMAVECScaleAdd(pGravity, pVelocity, &horizontal, -pGravity->dot(*pVelocity));
    damp.scale(pGravity->dot(*pVelocity), *pGravity);
    if (MR::isOnGround(pActor)) {
        horizontal *= 0.80f;
    } else {
        horizontal *= 0.98f;
    }

    if (damp.dot(*pGravity) < 0.0f) {
        damp *= 0.98f;
    }

    horizontal += damp;
    pVelocity->set(horizontal);
}

// this was laid out in some other way
void SamboHead::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (pSender != getSensor("Body")) {
        return;
    }

    if (MR::isSensorPlayer(pReceiver)) {
        if (MR::isPlayerHipDropFalling() || MR::isPlayerHipDropLand()) {
            return;
        }

        if (isNerve(GET_NERVE(SamboHead, HostTypeNrvPursue)) || isNerve(GET_NERVE(SamboHead, HostTypeNrvPrePursue)) ||
            isNerve(GET_NERVE(SamboHead, HostTypeNrvPursueEnd)) || isNerve(GET_NERVE(SamboHead, HostTypeNrvWait))) {
            if (MR::isPlayerExistUp(this, MR::getSensorRadius(this, "Body"), 0.25f)) {
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }

            if (MR::sendMsgEnemyAttack(pReceiver, pSender) == 1) {
                setNerve(GET_NERVE(SamboHead, HostTypeNrvHitToPlayer));
                return;
            }

            MR::sendMsgPush(pReceiver, pSender);
            return;
        }

        if (isNerve(GET_NERVE(SamboHead, HostTypeNrvAppear)) && MR::isGreaterStep(this, 45)) {
            if (MR::isPlayerExistUp(this, MR::getSensorRadius(this, "Body"), 0.25f)) {
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }

            if (MR::sendMsgEnemyAttack(pReceiver, pSender) != 1) {
                MR::sendMsgPush(pReceiver, pSender);
            }

            return;
        }

        if (!isNerve(GET_NERVE(SamboHead, HostTypeNrvWaitUnderGround)) && !isNerve(GET_NERVE(SamboHead, HostTypeNrvHide)) &&
            !isNerve(GET_NERVE(SamboHead, HostTypeNrvStampFall)) && !isNerve(GET_NERVE(SamboHead, HostTypeNrvStampDeath)) &&
            !isNerve(GET_NERVE(SamboHead, HostTypeNrvHitBlow))) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    } else if (MR::isSensorEnemy(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool SamboHead::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(SamboHead, HostTypeNrvHitBlow)) || isNerve(GET_NERVE(SamboHead, HostTypeNrvStampDeath)) ||
        isNerve(GET_NERVE(SamboHead, HostTypeNrvStampFall)) || isNerve(GET_NERVE(SamboHead, HostTypeNrvWaitUnderGround)) ||
        isNerve(GET_NERVE(SamboHead, HostTypeNrvHide))) {
        return false;
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        if (isNerve(GET_NERVE(SamboHead, HostTypeNrvStarPieceHit))) {
            mScaleController->startHitReaction();
        } else {
            setNerve(GET_NERVE(SamboHead, HostTypeNrvStarPieceHit));
        }

        return true;
    }

    if (MR::isMsgPlayerHipDrop(msg) || MR::isMsgPlayerTrample(msg)) {
        MR::startActionSound(this, "EmStompedS");
        if (MR::isOnGround(this)) {
            setNerve(GET_NERVE(SamboHead, HostTypeNrvStampDeath));
        } else {
            setNerve(GET_NERVE(SamboHead, HostTypeNrvStampFall));
        }

        return true;
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        mSpinCtrl->start(this, *MR::getSensorPos(pSender), *MR::getSensorPos(pReceiver));
        setNerve(GET_NERVE(SamboHead, HostTypeNrvHitBlow));
        return true;
    }

    return false;
}

bool SamboHead::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemy(pSender) && !MR::isSensorMapObj(pSender)) {
        return false;
    }
    if (isNerve(GET_NERVE(SamboHead, HostTypeNrvWaitUnderGround)) || isNerve(GET_NERVE(SamboHead, HostTypeNrvHide)) ||
        isNerve(GET_NERVE(SamboHead, HostTypeNrvStampFall)) || isNerve(GET_NERVE(SamboHead, HostTypeNrvStampDeath)) ||
        isNerve(GET_NERVE(SamboHead, HostTypeNrvHitBlow))) {
        return false;
    }
    MR::addVelocityFromPush(this, 2.0f, pSender, pReceiver);
    return true;
}

bool SamboHead::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(SamboHead, HostTypeNrvHitBlow)) || isNerve(GET_NERVE(SamboHead, HostTypeNrvStampDeath)) ||
        isNerve(GET_NERVE(SamboHead, HostTypeNrvStampFall)) || isNerve(GET_NERVE(SamboHead, HostTypeNrvWaitUnderGround)) ||
        isNerve(GET_NERVE(SamboHead, HostTypeNrvHide))) {
        return false;
    }
    if (MR::isMsgInhaleBlackHole(msg)) {
        kill();
        return true;
    }
    if (MR::isMsgPlayerKick(msg)) {
        if (!MR::isPlayerExistSide(this, MR::getSensorRadius(this, "Body"), 0.25f)) {
            return false;
        }
        if (isNerve(GET_NERVE(SamboHead, HostTypeNrvStarPieceHit))) {
            mSpinCtrl->start(this, *MR::getSensorPos(pSender), *MR::getSensorPos(pReceiver));
            setNerve(GET_NERVE(SamboHead, HostTypeNrvHitBlow));
            return true;
        }
    }

    return false;
}

void SamboHead::calcAndSetBaseMtx() {
    TPos3f mtx;
    MR::calcMtxFromGravityAndZAxis(&mtx, this, mGravity, _98);
    MR::setBaseTRMtx(this, mtx);
    MR::updateBaseScale(this, mScaleController);
}

void SamboHead::endBindStarPointer() {
    mBindStarPointer->kill();
}

void SamboHead::endHitBlow() {
    MR::validateClipping(this);
}

void SamboHead::endStampDeath() {
    MR::validateClipping(this);
}

void SamboHead::endStampFall() {
    MR::validateClipping(this);
}

SamboHead::~SamboHead() {
}
