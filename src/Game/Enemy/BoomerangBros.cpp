#include "Game/Enemy/BoomerangBros.hpp"
#include "Game/Enemy/BrosState.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace {
    extern const f32 sTongueAccel;
}


void BoomerangBros::exeEaten() {
    endFaceCtrl();
}

namespace NrvBoomerangBros {
    NEW_NERVE(BoomerangBrosNrvWait, BoomerangBros, Wait);
    NEW_NERVE(BoomerangBrosNrvInterval, BoomerangBros, Interval);
    NEW_NERVE(BoomerangBrosNrvJump, BoomerangBros, Jump);
    NEW_NERVE(BoomerangBrosNrvAttack, BoomerangBros, Attack);
    NEW_NERVE(BoomerangBrosNrvSwoon, BoomerangBros, Swoon);
    NEW_NERVE(BoomerangBrosNrvBlowDown, BoomerangBros, BlowDown);
    NEW_NERVE(BoomerangBrosNrvSwoonPressDown, BoomerangBros, SwoonPressDown);
    NEW_NERVE(BoomerangBrosNrvStandPressDown, BoomerangBros, StandPressDown);
    NEW_NERVE(BoomerangBrosNrvGuard, BoomerangBros, Guard);
    NEW_NERVE(BoomerangBrosNrvEaten, BoomerangBros, Eaten);

    class BoomerangBrosNrvFreezeStand : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* pActor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(pActor, &BoomerangBrosNrvWait::sInstance);
        }
        static BoomerangBrosNrvFreezeStand sInstance;
    };
    BoomerangBrosNrvFreezeStand BoomerangBrosNrvFreezeStand::sInstance ATTRIBUTE_WEAK;

    class BoomerangBrosNrvFreezeSwoon : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            BoomerangBros* pActor = reinterpret_cast< BoomerangBros* >(pSpine->mExecutor);
            if (MR::updateActorStateAndNextNerve(pActor, &BoomerangBrosNrvSwoon::sInstance)) {
                pActor->mStateSwoon->requestBrosSwoonRecovery();
            }
        }
        static BoomerangBrosNrvFreezeSwoon sInstance;
    };
    BoomerangBrosNrvFreezeSwoon BoomerangBrosNrvFreezeSwoon::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvBoomerangBros

#define BNRV(n) (&NrvBoomerangBros::BoomerangBrosNrv##n::sInstance)

static TVec3f sHoldTrans(52.0f, -15.0f, -70.0f);
static TVec3f sHoldRotate(-53.0f, -78.0f, 200.0f);

BoomerangBros::BoomerangBros(const char* pName) : BrosBase(pName), mBindStand(nullptr), mBindSwoon(nullptr), mUnusedE0(0.0f), mWaitCount(0), mStateJump(nullptr), mStateAttack(nullptr), mStateSwoon(nullptr) {
}

void BoomerangBros::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "BoomerangBros", false);
    BrosBase::init(rIter);
    for (s32 i = 0; i < 5; i++) {
        mProjectiles[i] = new BoomerangBrosBoomerang("\x83\x75\x81\x5b\x83\x81\x83\x89\x83\x93");
        mProjectiles[i]->initWithoutIter();
    }
    MR::moveCoordToRailPoint(this, MR::getRailPointNum(this) - 1);
    MR::calcFrontVec(&mFront, this);
    MR::getJMapInfoArg0NoInit(rIter, &mWaitCount);
    if (mWaitCount > 0) {
        mIsInitialAttack = true;
    }
    bool flag = false;
    MR::getJMapInfoArg2NoInit(rIter, &flag);
    if (flag) {
        mAllowRailJump = false;
    }
    initStates();
    initByArg(rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
    mBindStand = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mBindStand, BNRV(FreezeStand), "SupportFreezeStand");
    mBindSwoon = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mBindSwoon, BNRV(FreezeSwoon), "SupportFreezeSwoon");
    MR::validateClipping(this);
}

void BoomerangBros::initStates() {
    initNerve(BNRV(Wait), 16);
    mStateJump = new BrosStateJump(this);
    mStateAttack = new BoomerangBrosStateAttack(this);
    mStateSwoon = new BrosStateSwoon(this);
    MR::initActorState(this, mStateJump, BNRV(Jump), "Jump");
    MR::initActorState(this, mStateAttack, BNRV(Attack), "Attack");
    MR::initActorState(this, mStateSwoon, BNRV(Swoon), "Swoon");
}

void BoomerangBros::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    setNerve(BNRV(Wait));
    mHeld = nullptr;
}

void BoomerangBros::kill() {
    killHeldProjectile();
    LiveActor::kill();
}

void BoomerangBros::control() {
    mScaleCtrl->updateNerve();
    if (!isNerve(BNRV(Eaten))) {
        if (isNormalNerve() || isAliveNerve()) {
            YoshiFunction::tryYoshiEat(mYoshiLockOn);
        }
    }
    mJointCtrl->update();
    killProjectiles();
    if (MR::isBindedGroundDamageFire(this) || MR::isInWater(mPosition)) {
        appearStarPiece();
        MR::tryHitReactionDead(this);
        kill();
    }
    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        appearStarPiece();
        MR::tryHitReactionDead(this);
        kill();
    } else {
        tryBindStarPointer();
    }
}

const TVec3f& BoomerangBros::getHoldTrans() const {
    return sHoldTrans;
}

const TVec3f& BoomerangBros::getHoldRotate() const {
    return sHoldRotate;
}

void BoomerangBros::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateClipping(this);
        startFaceCtrl();
    }
    if (MR::isValidSwitchA(this)) {
        if (MR::isOnSwitchA(this) && MR::isBindedGround(this)) {
            setNerve(BNRV(Jump));
            return;
        }
    } else if (MR::calcDistanceToPlayerH(this) < 1000.0f && MR::isBindedGround(this)) {
        mWaitCount = 0;
        MR::invalidateClipping(this);
        setNerve(BNRV(Jump));
        return;
    }
    updateVelocity2();
}

void BoomerangBros::exeInterval() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        startFaceCtrl();
    }
    if (--mWaitCount <= 0) {
        setNerve(BNRV(Attack));
    }
}

void BoomerangBros::exeJump() {
    if (MR::updateActorState(this)) {
        if (mWaitCount <= 0) {
            setNerve(BNRV(Attack));
        } else {
            setNerve(BNRV(Interval));
        }
    }
}

void BoomerangBros::exeAttack() {
    if (MR::updateActorState(this)) {
        mIsInitialAttack = false;
        setNerve(BNRV(Jump));
    }
}

void BoomerangBros::exeSwoon() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
    }
    bool done = MR::updateActorState(this);
    updateVelocity2();
    if (done) {
        MR::zeroVelocity(this);
        setNerve(BNRV(Jump));
    }
}

void BoomerangBros::exeBlowDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BlowDown");
        MR::invalidateClipping(this);
        endFaceCtrl();
    }
    updateVelocity2();
    TVec3f dir = -mVelocity;
    MR::turnDirectionDegree(this, &mFront, dir, 30.0f);
    if (MR::isGreaterStep(this, 240) || (MR::isGreaterStep(this, 15) && MR::isBindedGround(this))) {
        appearStarPiece();
        MR::tryHitReactionDead(this);
        kill();
    }
}

void BoomerangBros::exeSwoonPressDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonPressDown");
        MR::invalidateClipping(this);
        endFaceCtrl();
        MR::zeroVelocity(this);
    }
    if (MR::isActionEnd(this)) {
        appearCoin();
        MR::tryHitReactionDead(this);
        kill();
    }
}

// NONMATCHING: register allocation / operand load order of the TVec3f temporaries.
void BoomerangBros::exeStandPressDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StandPressDown");
        MR::invalidateClipping(this);
        endFaceCtrl();
        MR::zeroVelocity(this);
    }
    if (MR::isPlayerHipDropFalling()) {
        TVec3f diff = mPosition - *MR::getPlayerPos();
        MR::vecKillElement(diff, mGravity, &diff);
        MR::setPosition(this, *MR::getPlayerPos() + diff);
    }
    if (MR::isActionEnd(this)) {
        appearCoin();
        MR::tryHitReactionDead(this);
        kill();
    }
}

void BoomerangBros::exeGuard() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Guard");
        endFaceCtrl();
    }
    updateVelocity2();
    if (MR::isActionEnd(this)) {
        setNerve(BNRV(Wait));
    }
}

void BoomerangBros::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pSender)) {
        if (MR::isSensorPlayerOrRide(pReceiver)) {
            bool attacking = false;
            if (isNerve(BNRV(Wait)) || isNerve(BNRV(Jump)) || isNerve(BNRV(Attack)) || isNerve(BNRV(Guard))) {
                attacking = true;
            }
            if (attacking && !MR::isPlayerHipDropFalling()) {
                if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                    return;
                } else {
                    return;
                }
            } else if (!isNerve(BNRV(SwoonPressDown)) && !isNerve(BNRV(StandPressDown))) {
                if (MR::sendMsgPush(pReceiver, pSender)) {
                    return;
                } else {
                    return;
                }
                return;
            }
            return;
        }
        return;
    } else if (MR::isSensorYoshi(pReceiver)) {
        if (!isNerve(BNRV(SwoonPressDown)) && !isNerve(BNRV(StandPressDown))) {
            if (MR::sendMsgPush(pReceiver, pSender)) {
                return;
            }
        }
    }
}

bool BoomerangBros::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pReceiver)) {
        if (!MR::isSensorPlayerOrYoshiOrRide(pSender)) {
            MR::addVelocityFromPush(this, 10.0f, pSender, pReceiver);
            return true;
        }
        return false;
    }
    return false;
}

bool BoomerangBros::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        return tryStarPieceReflect();
    }
    if (MR::isMsgStarPieceAttack(msg)) {
        if (!isGuardable()) {
            trySpinDamage(pSender, pReceiver);
            return true;
        }
        return true;
    }
    if (MR::isMsgJetTurtleAttack(msg) || MR::isMsgYoshiProjectile(msg) || MR::isMsgYoshiEat(msg) || MR::isMsgExplosionAttack(msg)) {
        if (tryExplosionDamage(pSender, pReceiver)) {
            MR::tryHitReactionBetweenSensors(this, "\x93\x8a\x82\xb0\x81\x45\x94\x9a\x94\xad\x95\xa8\x83\x5f\x83\x81\x81\x5b\x83\x57", pSender, pReceiver);
            return true;
        }
        return false;
    }
    if (MR::isMsgSupportTicoSpin(msg)) {
        return trySpinDamage(pSender, pReceiver);
    }
    if (MR::isMsgPlayerHitAll(msg)) {
        if (tryKickDamage(pSender, pReceiver)) {
            return true;
        }
        return tryHitDamage(pSender, pReceiver);
    }
    if (MR::isMsgPlayerTrample(msg)) {
        return tryStomp();
    }
    if (MR::isMsgPlayerHipDrop(msg)) {
        return tryStomp();
    }
    return false;
}

bool BoomerangBros::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }
    if (MR::isMsgEnemyAttack(msg)) {
        for (s32 i = 0; i < 5; i++) {
            if (MR::getSensorHost(pSender) == mProjectiles[i] && MR::getSensorHost(pSender) != mHeld) {
                if (isCatchNerve()) {
                    if (mStateAttack->tryCatch() && static_cast<BoomerangBrosBoomerang*>(mProjectiles[i])->isReturning()) {
                        mHeld = mProjectiles[i];
                        mHeld->attachToBros(this);
                        MR::startActionSound(this, "CatchBoomerang", -1, -1, -1);
                    }
                    return true;
                }
                if ((isNerve(BNRV(FreezeStand)) || isNerve(BNRV(FreezeSwoon))) && static_cast<BoomerangBrosBoomerang*>(mProjectiles[i])->isReturning()) {
                    mProjectiles[i]->killWithState3();
                    return true;
                }
            }
        }
    }
    return false;
}

bool BoomerangBros::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 244) {
        if (tryExplosionDamage(pSender, pReceiver)) {
            MR::tryHitReactionBetweenSensors(this, "\x91\xe5\x96\x43", pSender, pReceiver);
            MR::setVelocityBlowAttack(this, pSender, pReceiver, 36.0f, 40.0f, 4);
            return true;
        }
        return false;
    }
    if (MR::isMsgYoshiStartTongue(msg)) {
        return tryYoshiTongue(pSender, pReceiver);
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return true;
    }
    if (MR::isMsgYoshiReleaseOrCancelEaten(msg)) {
        emitBrosYoshiEatItem();
        kill();
        return true;
    }
    if (MR::isMsgPlayerKick(msg)) {
        return tryKickDamage(pSender, pReceiver);
    }
    return false;
}

void BoomerangBros::updateVelocity2() {
    if (!MR::isOnGround(this)) {
        MR::addVelocityToGravity(this, 1.5f);
    }
    MR::attenuateVelocity(this, MR::isBindedGround(this) ? 0.94f : 0.98f);
    if (MR::isBindedWall(this)) {
        MR::calcReboundVelocity(&mVelocity, MR::getWallNormal(this), 0.2f, 0.7f);
    }
}

bool BoomerangBros::isVulnerable() const {
    bool r = false;
    if (isNormalNerve() || (isNerve(BNRV(Swoon)) && mStateSwoon->isBrosSwoonVulnerable()) || isNerve(BNRV(FreezeSwoon))) {
        r = true;
    }
    return r;
}

bool BoomerangBros::isNormalNerve() const {
    bool r = false;
    if (isNerve(BNRV(Wait)) || isNerve(BNRV(Interval)) || isNerve(BNRV(Jump)) || isNerve(BNRV(Attack)) || isNerve(BNRV(Guard)) || isNerve(BNRV(FreezeStand))) {
        r = true;
    }
    return r;
}

bool BoomerangBros::isGuardable() const {
    bool r;
    if (isNerve(BNRV(Swoon))) {
        r = false;
    } else {
        r = false;
        if (isNormalNerve() || isNerve(BNRV(Guard))) {
            r = true;
        }
    }
    return r;
}

bool BoomerangBros::isCatchNerve() const {
    bool r = false;
    if (isNerve(BNRV(Wait)) || isNerve(BNRV(Interval)) || isNerve(BNRV(Jump)) || (isNerve(BNRV(Attack)) && mStateAttack->isAttackEnd()) || isNerve(BNRV(Guard))) {
        r = true;
    }
    return r;
}

bool BoomerangBros::isAliveNerve() const {
    bool r = false;
    if (isNerve(BNRV(Wait)) || isNerve(BNRV(Interval)) || isNerve(BNRV(Jump)) || isNerve(BNRV(Attack)) || isNerve(BNRV(Swoon)) || isNerve(BNRV(Guard))) {
        r = true;
    }
    return r;
}

bool BoomerangBros::isStarPointerBindable() const {
    if (isNerve(BNRV(Wait)) || isNerve(BNRV(Interval)) || isNerve(BNRV(Jump)) || isNerve(BNRV(Attack)) ||
        (isNerve(BNRV(Swoon)) && mStateSwoon->isBrosSwoonRecoveryComplete())) {
        return true;
    }
    if (isNerve(BNRV(Guard)) && MR::isGreaterStep(this, 60)) {
        return true;
    }
    return false;
}

bool BoomerangBros::tryExplosionDamage(HitSensor* pSender, HitSensor* pReceiver) {
    if (isVulnerable()) {
        MR::setVelocityBlowAttack(this, pSender, pReceiver, 36.0f, 40.0f, 4);
        setNerve(BNRV(BlowDown));
        killHeldProjectile();
        return true;
    }
    return false;
}

bool BoomerangBros::tryKickDamage(HitSensor* pSender, HitSensor* pReceiver) {
    bool vuln = false;
    if ((isNerve(BNRV(Swoon)) && mStateSwoon->isBrosSwoonVulnerable()) || isNerve(BNRV(FreezeSwoon))) {
        vuln = true;
    }
    if (vuln) {
        MR::setVelocityBlowAttack(this, pSender, pReceiver, 36.0f, 40.0f, 4);
        MR::tryHitReaction(this, "\x90\x81\x82\xc1\x94\xf2\x82\xd1");
        setNerve(BNRV(BlowDown));
        killHeldProjectile();
        return true;
    }
    return false;
}

bool BoomerangBros::tryHitDamage(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNormalNerve()) {
        MR::setVelocityBlowAttack(this, pSender, pReceiver, 22.0f, sTongueAccel, 4);
        MR::tryHitReaction(this, "\x90\x81\x82\xc1\x94\xf2\x82\xd1");
        setNerve(BNRV(Swoon));
        killHeldProjectile();
        return true;
    }
    return false;
}


bool BoomerangBros::trySpinDamage(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNormalNerve()) {
        bool ok = false;
        if ((isNerve(BNRV(Swoon)) && mStateSwoon->isBrosSwoonVulnerable()) || isNerve(BNRV(FreezeSwoon))) {
            ok = true;
        }
        if (!ok && !isNerve(BNRV(FreezeSwoon))) {
            goto fail;
        }
    }
    if (isNormalNerve()) {
        MR::setVelocityBlowAttack(this, pSender, pReceiver, 22.0f, sTongueAccel, 4);
    } else {
        MR::setVelocityBlowAttack(this, pSender, pReceiver, 0.0f, sTongueAccel, 4);
    }
    MR::tryHitReaction(this, "\x83\x60\x83\x52\x90\x81\x82\xc1\x94\xf2\x82\xd1");
    setNerve(BNRV(Swoon));
    killHeldProjectile();
    return true;
fail:
    return false;
}

bool BoomerangBros::tryStomp() {
    bool vuln = false;
    if ((isNerve(BNRV(Swoon)) && mStateSwoon->isBrosSwoonVulnerable()) || isNerve(BNRV(FreezeSwoon))) {
        vuln = true;
    }
    if (vuln) {
        setNerve(BNRV(SwoonPressDown));
        MR::tryHitReaction(this, "\x82\xc2\x82\xd4\x82\xea");
        killHeldProjectile();
        return true;
    }
    if (MR::isPlayerHipDropFalling() && isVulnerable()) {
        MR::tryHitReaction(this, "\x82\xc2\x82\xd4\x82\xea");
        setNerve(BNRV(StandPressDown));
        killHeldProjectile();
        return true;
    }
    if (isNormalNerve()) {
        setNerve(BNRV(Swoon));
        MR::tryHitReaction(this, "\x82\xc2\x82\xd4\x82\xea");
        killHeldProjectile();
        return true;
    }
    return false;
}

bool BoomerangBros::tryStarPieceReflect() {
    if (isGuardable()) {
        setNerve(BNRV(Guard));
        killHeldProjectile();
        MR::zeroVelocity(this);
        return true;
    }
    return false;
}

bool BoomerangBros::tryYoshiTongue(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNormalNerve()) {
        MR::addVelocityMoveToTargetHorizon(this, *MR::getPlayerPos(), 22.0f);
        MR::addVelocityMoveToTarget(this, *MR::getPlayerPos(), -sTongueAccel);
        MR::tryHitReaction(this, "\x83\x88\x83\x62\x83\x56\x81\x5b\x90\xe3\x83\x71\x83\x62\x83\x67");
        setNerve(BNRV(Swoon));
        killHeldProjectile();
        return false;
    }
    if (isAliveNerve()) {
        MR::addVelocityMoveToTarget(this, *MR::getPlayerPos(), -sTongueAccel);
        MR::tryHitReaction(this, "\x83\x88\x83\x62\x83\x56\x81\x5b\x90\xe3\x83\x71\x83\x62\x83\x67");
        setNerve(BNRV(Swoon));
        killHeldProjectile();
        return false;
    }
    return false;
}

bool BoomerangBros::tryBindStarPointer() {
    if (isStarPointerBindable()) {
        if (mBindStand->tryStartPointBind()) {
            setNerve(BNRV(FreezeStand));
            killHeldProjectile();
            return true;
        }
        return false;
    }
    bool swoon = false;
    if (isNerve(BNRV(Swoon)) && mStateSwoon->canBrosSwoonBindStarPointer()) {
        swoon = true;
    }
    if (swoon) {
        if (mBindSwoon->tryStartPointBind()) {
            setNerve(BNRV(FreezeSwoon));
            killHeldProjectile();
            return true;
        }
        return false;
    }
    return false;
}

void BoomerangBros::throwProjectile(BrosProjectileBase* pProj) {
    MtxPtr pHand = MR::getJointMtx(this, "HandR");
    Mtx a, b, c;
    PSMTXIdentity(a);
    PSMTXIdentity(b);
    PSMTXIdentity(c);
    PSMTXInverse(getBaseMtx(), a);
    PSMTXConcat(a, pHand, b);
    b[1][3] = 130.0f;
    PSMTXConcat(getBaseMtx(), b, c);
    TVec3f trans(0.0f);
    MR::extractMtxTrans(c, &trans);
    pProj->v70(trans, TVec3f(mPosition));
}

BoomerangBros::~BoomerangBros() {
}

namespace {
    const f32 sTongueAccel = 2.0f;
}
