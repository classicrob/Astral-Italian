#include "Game/Enemy/BombBird.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/BombBirdBomb.hpp"
#include "Game/Enemy/SpinHitController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

namespace {
    const BombBirdParam hFallParam = {0.0f, 1.2f, 0.98f, 0.0f};
    const BombBirdParam hOnGroundParam = {0.0f, 1.2f, 0.8f, 0.0f};

    const f32 hDamp = 0.96f;
    const f32 hGravity = 1.0f;
    const f32 hFlyVel = 0.3f;
    const f32 hToRailDiv = 200.0f;
    const f32 hBombFallUpVel = -10.0f;
    const f32 hTurnLimitRadian = 4.5836625f;
    const f32 hDotEpsilon = 0.001f;
    const f32 hSearchDist = 2000.0f;
    const f32 hSearchKeepHeight = 800.0f;
    const f32 hSearchKeepHeightAccUp = 10.0f;
    const f32 hEscapeVel = 5.0f;
    const f32 hEscapeUpVel = 5.0f;
    const f32 hSpinAttackedVelH = 10.0f;
    const f32 hSpinAttackedVelV = 20.0f;
    const s32 hSwoonTime = 240;
    const f32 hShiftingSandVel = 1.0f;
    const f32 hSwoonEndJump = 10.0f;
    const f32 hSwoonEndUpVel = 5.0f;
    const s32 hSwoonEndFlyStartTime = 30;
    const f32 hBombFallFrontVel = 0.0f;
};  // namespace

namespace NrvBombBird {
    NEW_NERVE(HostTypeNrvFlyOnRail, BombBird, FlyOnRail);
    NEW_NERVE(HostTypeNrvFlyOnRailSearch, BombBird, FlyOnRailSearch);
    NEW_NERVE(HostTypeNrvFlyWithAttack, BombBird, FlyWithAttack);
    NEW_NERVE(HostTypeNrvBlow, BombBird, Blow);
    NEW_NERVE(HostTypeNrvEscape, BombBird, Escape);
    NEW_NERVE(HostTypeNrvDamage, BombBird, Damage);
    NEW_NERVE(HostTypeNrvFall, BombBird, Fall);
    NEW_NERVE(HostTypeNrvSwoonStart, BombBird, SwoonStart);
    NEW_NERVE(HostTypeNrvSwoon, BombBird, Swoon);
    NEW_NERVE(HostTypeNrvSwoonEnd, BombBird, SwoonEnd);
    NEW_NERVE(HostTypeNrvTrample, BombBird, Trample);
    NEW_NERVE(HostTypeNrvSinkDown, BombBird, SinkDown);
    class HostTypeNrvBindStarPointer : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static HostTypeNrvBindStarPointer sInstance;
    };
    HostTypeNrvBindStarPointer HostTypeNrvBindStarPointer::sInstance ATTRIBUTE_WEAK;
    class HostTypeNrvBindStarPointerGround : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static HostTypeNrvBindStarPointerGround sInstance;
    };
    HostTypeNrvBindStarPointerGround HostTypeNrvBindStarPointerGround::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvBombBird

BombBird::~BombBird() {
}

BombBird::BombBird(const char* pName)
    : LiveActor(pName), mHeldBomb(), mFront(0, 0, 1), mSearchPlayer(), mHoldBomb(), mFixedPos(), mSpinHitController(), mScaleController(),
      mStarPointer(), mStarPointer2() {
}

void BombBird::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "BombBird", false);
    s32 arg = -1;
    MR::getJMapInfoArg0NoInit(rIter, &arg);
    if (arg != -1) mSearchPlayer = true;
    else mSearchPlayer = false;
    initNerve(GET_NERVE(BombBird, HostTypeNrvFlyOnRail), 2);
    MR::onCalcGravity(this);
    mFixedPos = new FixedPosition(this, "BombPos", TVec3f(0, 0, 0), TVec3f(0, 0, 0));
    mFixedPos->mNormalizeScale = false;
    mSpinHitController = new SpinHitController(this, 20, 4, 1.5f, 30.0f, 40.0f);
    for (BombBirdBomb** it = mBombs.begin(); it < mBombs.end(); it++) {
        *it = new BombBirdBomb("バクダン鳥のボム");
        (*it)->initWithoutIter();
        (*it)->makeActorDead();
    }
    if (mSearchPlayer) MR::invalidateClipping(this);
    else MR::validateClipping(this);
    mScaleController = new AnimScaleController(nullptr);
    mScaleController->setParamTight();
    mStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mStarPointer, GET_NERVE(BombBird, HostTypeNrvBindStarPointer), "SupportFreezeAir");
    mStarPointer2 = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mStarPointer2, GET_NERVE(BombBird, HostTypeNrvBindStarPointerGround), "SupportFreezeGround");
    MR::initUseStageSwitchReadB(this, rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void BombBird::makeActorDead() {
    LiveActor::makeActorDead();
    killAllBomb();
}

void BombBird::startClipped() {
    if (isNerve(GET_NERVE(BombBird, HostTypeNrvEscape))) {
        kill();
    } else {
        LiveActor::startClipped();
    }
}

void BombBird::control() {
    mScaleController->updateNerve();
    if (isNerve(GET_NERVE(BombBird, HostTypeNrvFlyOnRail)) || isNerve(GET_NERVE(BombBird, HostTypeNrvFlyOnRailSearch)) ||
        isNerve(GET_NERVE(BombBird, HostTypeNrvFlyWithAttack))) {
        if (mStarPointer->tryStartPointBind()) {
            setNerve(GET_NERVE(BombBird, HostTypeNrvBindStarPointer));
            return;
        }
    }

    if ((isNerve(GET_NERVE(BombBird, HostTypeNrvSwoonStart)) || isNerve(GET_NERVE(BombBird, HostTypeNrvSwoon)) ||
        isNerve(GET_NERVE(BombBird, HostTypeNrvSwoonEnd))) && mStarPointer2->tryStartPointBind()) {
        setNerve(GET_NERVE(BombBird, HostTypeNrvBindStarPointerGround));
        return;
    }

    if (!isNerve(GET_NERVE(BombBird, HostTypeNrvBlow)) && !isNerve(GET_NERVE(BombBird, HostTypeNrvTrample)) &&
        !isNerve(GET_NERVE(BombBird, HostTypeNrvSinkDown))) {
        if (MR::isBindedGroundSinkDeath(this)) {
            setNerve(GET_NERVE(BombBird, HostTypeNrvSinkDown));
            return;
        }
        TVec3f offset;
        offset.x = 0.0f;
        offset.y = 0.0f;
        offset.z = 0.0f;
        if (MR::isInDeath(this, offset)) {
            MR::startActionSound(this, "EmExplodeS");
            MR::emitEffect(this, "Death");
            kill();
            return;
        }
    }

    if (!isNerve(GET_NERVE(BombBird, HostTypeNrvTrample)) && MR::isPressedRoofAndGround(this)) {
        setNerve(GET_NERVE(BombBird, HostTypeNrvTrample));
    }
}

void BombBird::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(BombBird, HostTypeNrvBlow)) || isNerve(GET_NERVE(BombBird, HostTypeNrvTrample))) {
        return;
    }

    if (isNerve(GET_NERVE(BombBird, HostTypeNrvEscape)) || isNerve(GET_NERVE(BombBird, HostTypeNrvDamage)) ||
        isNerve(GET_NERVE(BombBird, HostTypeNrvFall)) || isNerve(GET_NERVE(BombBird, HostTypeNrvSwoonStart)) ||
        isNerve(GET_NERVE(BombBird, HostTypeNrvSwoon)) || isNerve(GET_NERVE(BombBird, HostTypeNrvSwoonEnd)) ||
        isNerve(GET_NERVE(BombBird, HostTypeNrvBindStarPointer)) ||
        isNerve(GET_NERVE(BombBird, HostTypeNrvBindStarPointerGround)) ||
        MR::isPlayerExistUp(this, MR::getSensorRadius(this, "Body"), 0.25f) || MR::isPlayerHipDropFalling() || MR::isPlayerHipDropLand()) {
        MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
    } else if (!MR::sendMsgEnemyAttack(pReceiver, pSender)) {
        MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
    }
}

void BombBird::exeFlyOnRail() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        if (!mSearchPlayer) {
            MR::validateClipping(this);
        }
    }

    doFly();
    if (MR::isValidSwitchB(this) && MR::isOnSwitchB(this)) {
        setNerve(GET_NERVE(BombBird, HostTypeNrvEscape));
    } else if (MR::calcDistanceToPlayerH(this) < ::hSearchDist && !MR::isDemoActive() && MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BombBird, HostTypeNrvFlyOnRailSearch));
    } else if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BombBird, HostTypeNrvFlyOnRail));
    }
}

void BombBird::exeFlyOnRailSearch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Born");
        mHoldBomb = false;
        if (!mSearchPlayer) {
            MR::validateClipping(this);
        }
    }

    doFly();
    if (MR::isValidSwitchB(this) && MR::isOnSwitchB(this)) {
        setNerve(GET_NERVE(BombBird, HostTypeNrvEscape));
    } else {
        if (MR::isStep(this, 54)) {
            mHoldBomb = true;
            mHeldBomb = nullptr;
            for (BombBirdBomb** it = mBombs.begin(); it < mBombs.end(); it++) {
                if (MR::isDead(*it)) {
                    mHeldBomb = *it;
                    mHeldBomb->hold(mPosition);
                    break;
                }
            }
        } else if (MR::isGreaterStep(this, 54)) {
            mHoldBomb = true;
            if (mHeldBomb) {
                TVec3f scale;
                MR::copyJointScale(this, "BombPos", &scale);
                MR::setScale(mHeldBomb, scale);
            }
        }

        if (MR::isActionEnd(this) && !MR::isDemoActive()) {
            if (mHeldBomb) {
                MR::setScale(mHeldBomb, 1.0f);
                setNerve(GET_NERVE(BombBird, HostTypeNrvFlyWithAttack));
            } else {
                setNerve(GET_NERVE(BombBird, HostTypeNrvFlyOnRail));
            }
        }
    }
}

void BombBird::doFly() {
    if (mSearchPlayer) {
        MR::flyAndTurnAlongRailSearchingPlayer(this, &mFront, ::hToRailDiv, ::hFlyVel, ::hGravity, ::hDamp, ::hTurnLimitRadian, true);
        addVelocityToUpFromGround(::hSearchKeepHeightAccUp);
    } else {
        MR::flyAndTurnAlongRailSearchingPlayer(this, &mFront, ::hToRailDiv, ::hFlyVel, 0.0f, ::hDamp, ::hTurnLimitRadian, false);
    }
}

void BombBird::addVelocityToUpFromGround(f32 vel) {
    f32 proj = MR::getShadowNearProjectionLength(this);
    if (3.4028235e38f != proj && proj < ::hSearchKeepHeight) {
        vel = (::hSearchKeepHeight - proj) >= vel ? vel : (::hSearchKeepHeight - proj);

        MR::addVelocityLimit(this, mGravity * -vel);
    }
}

void BombBird::killAllBomb() {
    for (BombBirdBomb** it = mBombs.begin(); it < mBombs.end(); it++) {
        if (!MR::isDead(*it)) {
            (*it)->kill();
        }
    }

    mHeldBomb = nullptr;
}

void BombBird::exeFlyWithAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");

        if (!mSearchPlayer) {
            MR::validateClipping(this);
        }
    }

    doFly();

    if (MR::isValidSwitchB(this) && MR::isOnSwitchB(this)) {
        setNerve(GET_NERVE(BombBird, HostTypeNrvEscape));
    } else {
        BombBirdBomb* bomb = mHeldBomb;
        if (bomb != nullptr) {
            bomb->mVelocity.zero();
        }

        if (MR::isStep(this, 130)) {
            MR::startActionSound(this, "EmBombbirdDropBomb");
            mHeldBomb->start(mPosition, mFront * ::hBombFallFrontVel - mGravity * ::hBombFallUpVel);
            mHeldBomb = nullptr;
            mHoldBomb = false;
        }

        if (MR::isActionEnd(this)) {
            if (MR::calcDistanceToPlayerH(this) < ::hSearchDist) {
                setNerve(GET_NERVE(BombBird, HostTypeNrvFlyOnRailSearch));
            } else {
                setNerve(GET_NERVE(BombBird, HostTypeNrvFlyOnRail));
            }
        }
    }
}


void BombBird::exeBlow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "DownSpin");
        MR::startActionSound(this, "EvBombbirdDead");
        MR::startActionSound(this, "EmBombbirdDamage");
        MR::startBlowHitSound(this);
    }

    if (mSpinHitController->execute(this)) {
        MR::emitItemHit(this);
        MR::startActionSound(this, "EmExplodeS");
        MR::emitEffect(this, "Death");
    }
}

void BombBird::exeEscape() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateClipping(this);
    }

    TVec3f escapeDirection(mPosition);
    escapeDirection -= *MR::getPlayerCenterPos();
    MR::vecKillElement(escapeDirection, mGravity, &escapeDirection);
    MR::normalizeOrZero(&escapeDirection);

    if (!MR::isNearZero(escapeDirection, ::hDotEpsilon)) {
        MR::moveAndTurnToDirection(this, &mFront, escapeDirection, ::hFlyVel, ::hGravity, ::hDamp, ::hTurnLimitRadian);
    }

    mVelocity.set(mFront * ::hEscapeVel - mGravity * ::hEscapeUpVel);

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BombBird, HostTypeNrvEscape));
    }
}

void BombBird::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Damage");
        MR::startActionSound(this, "EvBombbirdDamage");
        MR::startActionSound(this, "EmBombbirdDamage");
        MR::invalidateClipping(this);
    }

    const BombBirdParam* param = &::hFallParam;
    MR::moveAndTurnToPlayer(this, &mFront, param->mMoveAccel, param->mGravityAccel, param->mVelocityDamping, param->mTurnMaxDegree);

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BombBird, HostTypeNrvFall));
    }
}

void BombBird::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Fall");
        MR::invalidateClipping(this);
    }

    const BombBirdParam* param = &::hFallParam;
    MR::moveAndTurnToPlayer(this, &mFront, param->mMoveAccel, param->mGravityAccel, param->mVelocityDamping, param->mTurnMaxDegree);

    if (MR::isOnGround(this)) {
        MR::startAction(this, "SwoonStart");
        setNerve(GET_NERVE(BombBird, HostTypeNrvSwoonStart));
    }
}

void BombBird::exeSwoonStart() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
    }

    MR::startActionSound(this, "EmLvSwoonS");
    if (MR::isOnGround(this)) {
        MR::vecKillElement(mVelocity, mGravity, &mVelocity);
    }

    const BombBirdParam* param = &::hOnGroundParam;
    MR::moveAndTurnToPlayer(this, &mFront, param->mMoveAccel, param->mGravityAccel, param->mVelocityDamping, param->mTurnMaxDegree);
    TVec3f area(0, 0, 0);
    if (MR::calcVelocityAreaOrRailMoveOnGround(&area, this)) {
        mVelocity.set(area * ::hShiftingSandVel);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BombBird, HostTypeNrvSwoon));
    }
}

void BombBird::exeSwoon() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Swoon");
        MR::invalidateClipping(this);
    }

    MR::startActionSound(this, "EmLvSwoonS");

    if (MR::isOnGround(this)) {
        MR::vecKillElement(mVelocity, mGravity, &mVelocity);
    }

    MR::moveAndTurnToPlayer(this, &mFront, ::hOnGroundParam.mMoveAccel, ::hOnGroundParam.mGravityAccel, ::hOnGroundParam.mVelocityDamping, ::hOnGroundParam.mTurnMaxDegree);
    TVec3f area(0, 0, 0);
    if (MR::calcVelocityAreaOrRailMoveOnGround(&area, this)) {
        mVelocity.set(area * ::hShiftingSandVel);
    }

    if (MR::isGreaterStep(this, ::hSwoonTime)) {
        setNerve(GET_NERVE(BombBird, HostTypeNrvSwoonEnd));
    }
}

void BombBird::exeSwoonEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonEnd");
        MR::startActionSound(this, "EvBombbirdRecover");
        MR::invalidateClipping(this);
    }

    if (MR::isOnGround(this)) {
        MR::vecKillElement(mVelocity, mGravity, &mVelocity);
    }

    const BombBirdParam* param = &::hOnGroundParam;
    MR::moveAndTurnToPlayer(this, &mFront, param->mMoveAccel, param->mGravityAccel, param->mVelocityDamping, param->mTurnMaxDegree);

    TVec3f area(0, 0, 0);
    if (MR::calcVelocityAreaOrRailMoveOnGround(&area, this)) {
        mVelocity.set(area * ::hShiftingSandVel);
    }

    if (MR::isGreaterEqualStep(this, ::hSwoonEndFlyStartTime)) {
        if (MR::isStep(this, ::hSwoonEndFlyStartTime)) {
            MR::addVelocityJump(this, ::hSwoonEndJump);
        }

        doFly();
        addVelocityToUpFromGround(::hSwoonEndUpVel);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BombBird, HostTypeNrvFlyOnRail));
    }
}

void BombBird::exeTrample() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "DownStep");
        MR::startActionSound(this, "EmStompedS");
        MR::invalidateClipping(this);
    }

    if (MR::isOnGround(this)) {
        MR::vecKillElement(mVelocity, mGravity, &mVelocity);
    }

    const BombBirdParam* param = &::hOnGroundParam;
    MR::moveAndTurnToPlayer(this, &mFront, param->mMoveAccel, param->mGravityAccel, param->mVelocityDamping, param->mTurnMaxDegree);

    if (MR::isStep(this, 36)) {
        TVec3f pos(mPosition - mGravity * MR::getSensorRadius(this, "Body"));
        MR::emitItemStomp(this);
        MR::startActionSound(this, "EmExplodeS");
        MR::emitEffect(this, "Death");
        kill();
    }
}

void BombBird::exeSinkDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SinkDown");
        MR::invalidateShadow(this, nullptr);
        MR::invalidateClipping(this);
    }

    MR::startActionSound(this, "EmLvSink");
    MR::startActionSound(this, "EvLvBombbirdSink");
    const BombBirdParam* param = &::hOnGroundParam;
    MR::moveAndTurnToPlayer(this, &mFront, param->mMoveAccel, param->mGravityAccel, param->mVelocityDamping, param->mTurnMaxDegree);

    if (MR::isActionEnd(this)) {
        MR::emitEffect(this, "SinkDeath");
        kill();
    }
}

// NONMATCHING: The epilogue restores LR and the saved GPR in a different order.
void BombBird::exeBombBirdBindStarPointer() {
    if (MR::isFirstStep(this)) MR::invalidateClipping(this);
    if (mHeldBomb) {
        mHeldBomb->kill();
        mHeldBomb = nullptr;
    }
    MR::updateActorStateAndNextNerve(this, GET_NERVE(BombBird, HostTypeNrvFlyOnRail));
}
// NONMATCHING: The epilogue restores LR and the saved GPR in a different order.
void BombBird::exeBombBirdBindStarPointerGround() {
    if (MR::isFirstStep(this)) MR::invalidateClipping(this);
    if (mHeldBomb) {
        mHeldBomb->kill();
        mHeldBomb = nullptr;
    }
    MR::updateActorStateAndNextNerve(this, GET_NERVE(BombBird, HostTypeNrvSwoonEnd));
}

bool BombBird::receiveMsgPlayerAttack(u32 msg, HitSensor* sender, HitSensor* receiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) return true;
    if (isGroundSwoonNerve()) {
        if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
            setNerve(GET_NERVE(BombBird, HostTypeNrvTrample));
            return true;
        }
        if (MR::isMsgStarPieceAttack(msg)) {
            mScaleController->startHitReaction();
            return true;
        }
        if (MR::isMsgSupportTicoSpin(msg)) {
            MR::zeroVelocity(this);
            MR::addVelocityToGravity(this, -20.0f);
            setNerve(GET_NERVE(BombBird, HostTypeNrvFall));
            return true;
        }
        if (MR::isMsgPlayerHitAll(msg)) {
            startSpinBlow(sender, receiver);
            return true;
        }
        return false;
    }
    if (isFlyingNerve()) {
        if (MR::isMsgPlayerHitProjectiles(msg)) {
            if (mHeldBomb) { mHeldBomb->kill(); mHeldBomb = nullptr; }
            startSpinBlow(sender, receiver);
            return true;
        }
        if (MR::isMsgPlayerHitAll(msg) || MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg) || MR::isMsgStarPieceAttack(msg)) {
            if (mHeldBomb) { mHeldBomb->kill(); mHeldBomb = nullptr; }
            MR::setVelocitySeparateHV(this, sender, receiver, 10.0f, 20.0f);
            setNerve(GET_NERVE(BombBird, HostTypeNrvDamage));
            if (MR::isMsgPlayerHipDrop(msg)) MR::sendMsgAwayJump(sender, receiver);
            return true;
        }
    } else if (MR::isMsgStarPieceAttack(msg)) return true;
    return false;
}
bool BombBird::isFlyingNerve() const {
    if (isNerve(GET_NERVE(BombBird, HostTypeNrvFlyOnRail)) || isNerve(GET_NERVE(BombBird, HostTypeNrvFlyOnRailSearch)) ||
        isNerve(GET_NERVE(BombBird, HostTypeNrvFlyWithAttack)) || isNerve(GET_NERVE(BombBird, HostTypeNrvBindStarPointer)) ||
        (isNerve(GET_NERVE(BombBird, HostTypeNrvSwoonEnd)) && MR::isGreaterEqualStep(this, 30))) return true;
    return false;
}
bool BombBird::isGroundSwoonNerve() const {
    if (isNerve(GET_NERVE(BombBird, HostTypeNrvSwoonStart)) || isNerve(GET_NERVE(BombBird, HostTypeNrvSwoon)) ||
        (isNerve(GET_NERVE(BombBird, HostTypeNrvSwoonEnd)) && MR::isLessStep(this, 30)) ||
        isNerve(GET_NERVE(BombBird, HostTypeNrvBindStarPointerGround))) return true;
    return false;
}
void BombBird::startSpinBlow(HitSensor* sender, HitSensor* receiver) {
    mSpinHitController->start(this, *MR::getSensorPos(sender), *MR::getSensorPos(receiver));
    MR::calcSensorHorizonNormalize(&mFront, mGravity, receiver, sender);
    MR::invalidateClipping(this);
    setNerve(GET_NERVE(BombBird, HostTypeNrvBlow));
}
bool BombBird::receiveOtherMsg(u32 msg, HitSensor* sender, HitSensor* receiver) {
    if (MR::isMsgPlayerKick(msg) && (isNerve(GET_NERVE(BombBird, HostTypeNrvSwoonStart)) ||
        isNerve(GET_NERVE(BombBird, HostTypeNrvSwoon)) || isNerve(GET_NERVE(BombBird, HostTypeNrvBindStarPointerGround)))) {
        startSpinBlow(sender, receiver);
        return true;
    }
    if (MR::isMsgInhaleBlackHole(msg) && !isNerve(GET_NERVE(BombBird, HostTypeNrvBlow)) &&
        !isNerve(GET_NERVE(BombBird, HostTypeNrvTrample)) && !isNerve(GET_NERVE(BombBird, HostTypeNrvSinkDown))) {
        MR::startActionSound(this, "EmExplodeS");
        MR::emitEffect(this, "Death");
        kill();
        return true;
    }
    return false;
}

void BombBird::calcAndSetBaseMtx() {
    TPos3f mtx;
    MR::calcMtxFromGravityAndZAxis(&mtx, this, mGravity, mFront);
    MR::setBaseTRMtx(this, mtx);
    MR::updateBaseScale(this, mScaleController);
    if (mHoldBomb && mHeldBomb) {
        mFixedPos->calc();
        mFixedPos->copyTrans(&mHeldBomb->mPosition);
        TVec3f scale;
        mFixedPos->mMtx.getScale(scale);
        MR::setBaseScale(mHeldBomb, scale);
    }
}

void NrvBombBird::HostTypeNrvBindStarPointer::execute(Spine* spine) const {
    static_cast<BombBird*>(spine->mExecutor)->exeBombBirdBindStarPointer();
}
void NrvBombBird::HostTypeNrvBindStarPointerGround::execute(Spine* spine) const {
    static_cast<BombBird*>(spine->mExecutor)->exeBombBirdBindStarPointerGround();
}
