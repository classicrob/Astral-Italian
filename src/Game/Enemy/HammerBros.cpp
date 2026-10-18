#include "Game/Enemy/HammerBros.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"

extern "C" void fn_8022BC70(LiveActor*);
extern "C" void fn_800345E0(LiveActor*, const char*);
extern "C" void fn_80034630(LiveActor*, const char*, HitSensor*, HitSensor*);
extern "C" void fn_80034910(LiveActor*);
extern "C" void fn_80012B90(LiveActor*, f32, f32, HitSensor*, HitSensor*, s32);
extern "C" void fn_800348C0(LiveActor*);
extern "C" void fn_8022BAE0(LiveActor*);
extern "C" s32 lbl_807E3028;

// Per-state actor states (defined in the second half of the original unit, not ported yet).
class HammerBrosStateJump : public ActorStateBase< HammerBros > {
public:
    HammerBrosStateJump(HammerBros*);
    u8 _pad[88 - sizeof(ActorStateBase< HammerBros >)];
};

class HammerBrosStateAttack : public ActorStateBase< HammerBros > {
public:
    HammerBrosStateAttack(HammerBros*);
    u8 _pad[20 - sizeof(ActorStateBase< HammerBros >)];
};

class HammerBrosStateSwoon : public ActorStateBase< HammerBros > {
public:
    HammerBrosStateSwoon(HammerBros*);
    void fn_8018AFF0();
    bool fn_8018B000() const;
    bool fn_8018B080() const;
    bool fn_8018B0E0() const;
    u8 _pad[24 - sizeof(ActorStateBase< HammerBros >)];
};

HammerBros::HammerBros(const char* pName) : BrosBase(pName), _E0(0.0f), mWaitCount(0), mStateJump(nullptr), mStateAttack(nullptr), mStateSwoon(nullptr) {
    mMtx.identity();
}

namespace NrvHammerBros {
    NEW_NERVE(HammerBrosNrvWait, HammerBros, Wait);
    NEW_NERVE(HammerBrosNrvInterval, HammerBros, Interval);
    NEW_NERVE(HammerBrosNrvJump, HammerBros, Jump);
    NEW_NERVE(HammerBrosNrvAttack, HammerBros, Attack);
    NEW_NERVE(HammerBrosNrvSwoon, HammerBros, Swoon);
    NEW_NERVE(HammerBrosNrvBlowDown, HammerBros, BlowDown);
    NEW_NERVE(HammerBrosNrvSwoonPressDown, HammerBros, SwoonPressDown);
    NEW_NERVE(HammerBrosNrvStandPressDown, HammerBros, StandPressDown);
    NEW_NERVE(HammerBrosNrvGuard, HammerBros, Guard);

    class HammerBrosNrvFreezeStand : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* pActor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(pActor, &HammerBrosNrvWait::sInstance);
        }
        static HammerBrosNrvFreezeStand sInstance;
    };
    HammerBrosNrvFreezeStand HammerBrosNrvFreezeStand::sInstance ATTRIBUTE_WEAK;

    class HammerBrosNrvFreezeSwoon : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            HammerBros* pActor = reinterpret_cast< HammerBros* >(pSpine->mExecutor);
            if (MR::updateActorStateAndNextNerve(pActor, &HammerBrosNrvSwoon::sInstance)) {
                pActor->mStateSwoon->fn_8018AFF0();
            }
        }
        static HammerBrosNrvFreezeSwoon sInstance;
    };
    HammerBrosNrvFreezeSwoon HammerBrosNrvFreezeSwoon::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(HammerBrosNrvEaten, HammerBros, Eaten);
};  // namespace NrvHammerBros

#define NRV(n) (&NrvHammerBros::HammerBrosNrv##n::sInstance)

static TVec3f sHoldTrans(18.0f, 0.0f, -40.0f);
static TVec3f sHoldRotate(-95.0f, 15.0f, -90.0f);
static TVec3f sThrowVel(0.0f, 11.0f, 27.0f);

void HammerBros::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "HammerBros", false);
    BrosBase::init(rIter);
    for (s32 i = 0; i < 5; i++) {
        mProjectiles[i] = new HammerBrosHammer("\x83\x6e\x83\x93\x83\x7d\x81\x5b");
        mProjectiles[i]->initWithoutIter();
    }
    if (MR::isExistRail(this)) {
        MR::moveCoordToRailPoint(this, MR::getRailPointNum(this) - 1);
    }
    MR::calcFrontVec(&mFront, this);
    MR::getJMapInfoArg0NoInit(rIter, &mWaitCount);
    if (mWaitCount > 0) {
        _B9 = true;
    }
    bool flag = false;
    MR::getJMapInfoArg2NoInit(rIter, &flag);
    if (flag) {
        _B8 = false;
    }
    initStates();
    initByArg(rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
    mBindStand = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mBindStand, NRV(FreezeStand), "SupportFreezeStand");
    mBindSwoon = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mBindSwoon, NRV(FreezeSwoon), "SupportFreezeSwoon");
    MR::validateClipping(this);
}

void HammerBros::initStates() {
    initNerve(NRV(Wait), 16);
    mStateJump = new HammerBrosStateJump(this);
    mStateAttack = new HammerBrosStateAttack(this);
    mStateSwoon = new HammerBrosStateSwoon(this);
    MR::initActorState(this, mStateJump, NRV(Jump), "Jump");
    MR::initActorState(this, mStateAttack, NRV(Attack), "Attack");
    MR::initActorState(this, mStateSwoon, NRV(Swoon), "Swoon");
}

void HammerBros::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    setNerve(NRV(Wait));
    mHeld = nullptr;
}

void HammerBros::kill() {
    killProjectiles();
    LiveActor::kill();
}

const TVec3f& HammerBros::getHoldTrans() const {
    return sHoldTrans;
}

const TVec3f& HammerBros::getHoldRotate() const {
    return sHoldRotate;
}

void HammerBros::control() {
    if (!isNerve(NRV(Wait))) {
        mConnected = false;
    }
    mScaleCtrl->updateNerve();
    if (!isNerve(NRV(Eaten))) {
        if (isNormalNerve() || isAliveNerve()) {
            YoshiFunction::tryYoshiEat(mYoshiLockOn);
        }
    }
    mJointCtrl->update();
    killProjectiles();
    if (MR::isBindedGroundDamageFire(this) || MR::isInWater(mPosition) || MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        appearStarPiece();
        fn_80034910(this);
        kill();
    } else {
        tryBindStarPointer();
    }
}

void HammerBros::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        mJointCtrl->startFaceCtrl(10);
        attachToUnder();
    }
    if (MR::isValidSwitchA(this)) {
        if (MR::isOnSwitchA(this) && MR::isBindedGround(this)) {
            MR::invalidateClipping(this);
            setNerve(NRV(Jump));
            return;
        }
    } else if (MR::calcDistanceToPlayerH(this) < 1000.0f && MR::isBindedGround(this)) {
        mWaitCount = 0;
        setNerve(NRV(Jump));
        return;
    }
    updateVelocity();
}

void HammerBros::exeInterval() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
    if (--mWaitCount <= 0) {
        setNerve(NRV(Attack));
    }
}

void HammerBros::exeJump() {
    if (MR::updateActorState(this)) {
        if (mWaitCount <= 0) {
            setNerve(NRV(Attack));
        } else {
            setNerve(NRV(Interval));
        }
    }
}

void HammerBros::exeAttack() {
    if (MR::updateActorState(this)) {
        _B9 = false;
        setNerve(NRV(Jump));
    }
}

void HammerBros::exeSwoon() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
    }
    bool done = MR::updateActorState(this);
    updateVelocity();
    if (done) {
        MR::zeroVelocity(this);
        setNerve(NRV(Jump));
    }
}

void HammerBros::exeBlowDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BlowDown");
        MR::invalidateClipping(this);
        mJointCtrl->endFaceCtrl(10);
    }
    updateVelocity();
    TVec3f dir = -mVelocity;
    MR::turnDirectionDegree(this, &mFront, dir, 30.0f);
    if (MR::isGreaterStep(this, 240) || (MR::isGreaterStep(this, 15) && MR::isBindedGround(this))) {
        appearStarPiece();
        fn_80034910(this);
        kill();
    }
}

void HammerBros::exeSwoonPressDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonPressDown");
        MR::invalidateClipping(this);
        mJointCtrl->endFaceCtrl(10);
        MR::zeroVelocity(this);
    }
    if (MR::isActionEnd(this)) {
        appearCoin();
        fn_80034910(this);
        kill();
    }
}

void HammerBros::exeStandPressDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StandPressDown");
        MR::invalidateClipping(this);
        mJointCtrl->endFaceCtrl(10);
        MR::zeroVelocity(this);
    }
    if (MR::isPlayerHipDropFalling()) {
        const TVec3f* pPlayer = MR::getPlayerPos();
        TVec3f diff(mPosition);
        diff -= *pPlayer;
        MR::vecKillElement(diff, mGravity, &diff);
        TVec3f pos(*MR::getPlayerPos());
        pos += diff;
        MR::setPosition(this, pos);
    }
    if (MR::isActionEnd(this)) {
        appearCoin();
        fn_80034910(this);
        kill();
    }
}

void HammerBros::exeGuard() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Guard");
        mJointCtrl->endFaceCtrl(10);
    }
    updateVelocity();
    if (MR::isActionEnd(this) && MR::isGreaterStep(this, lbl_807E3028)) {
        setNerve(NRV(Wait));
    }
}

bool HammerBros::isVulnerable() const {
    bool r = false;
    if (isNormalNerve() || (isNerve(NRV(Swoon)) && mStateSwoon->fn_8018B000()) || isNerve(NRV(FreezeSwoon))) {
        r = true;
    }
    return r;
}

bool HammerBros::isNormalNerve() const {
    return isNerve(NRV(Wait)) || isNerve(NRV(Interval)) || isNerve(NRV(Jump)) || isNerve(NRV(Attack)) || isNerve(NRV(Guard)) || isNerve(NRV(FreezeStand));
}

bool HammerBros::isGuardable() const {
    bool r;
    if (isNerve(NRV(Swoon))) {
        r = false;
    } else {
        r = false;
        if (isNormalNerve() || isNerve(NRV(Guard))) {
            r = true;
        }
    }
    return r;
}

bool HammerBros::isAliveNerve() const {
    bool r = false;
    if (isNerve(NRV(Wait)) || isNerve(NRV(Interval)) || isNerve(NRV(Jump)) || isNerve(NRV(Attack)) || isNerve(NRV(Swoon)) || isNerve(NRV(Guard)) || isNerve(NRV(FreezeStand)) || isNerve(NRV(FreezeSwoon))) {
        r = true;
    }
    return r;
}

bool HammerBros::isStarPointerBindable() const {
    if (isNerve(NRV(Wait)) || isNerve(NRV(Interval)) || isNerve(NRV(Jump)) || isNerve(NRV(Attack))) {
        return true;
    }
    if (isNerve(NRV(Swoon)) && mStateSwoon->fn_8018B080()) {
        return true;
    }
    if (isNerve(NRV(Guard)) && MR::isGreaterStep(this, 60)) {
        return true;
    }
    return false;
}

bool HammerBros::tryExplosionDamage(HitSensor* pSender, HitSensor* pReceiver) {
    if (isVulnerable()) {
        fn_80012B90(this, 36.0f, 40.0f, pSender, pReceiver, 4);
        setNerve(NRV(BlowDown));
        killHeldProjectile();
        return true;
    }
    return false;
}

bool HammerBros::tryKickDamage(HitSensor* pSender, HitSensor* pReceiver) {
    bool vuln = false;
    if ((isNerve(NRV(Swoon)) && mStateSwoon->fn_8018B000()) || isNerve(NRV(FreezeSwoon))) {
        vuln = true;
    }
    if (vuln) {
        fn_80012B90(this, 36.0f, 40.0f, pSender, pReceiver, 4);
        fn_800345E0(this, "\x90\x81\x82\xc1\x94\xf2\x82\xd1");
        setNerve(NRV(BlowDown));
        killHeldProjectile();
        return true;
    }
    return false;
}

bool HammerBros::tryHitDamage(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNormalNerve()) {
        fn_80012B90(this, 22.0f, 2.0f, pSender, pReceiver, 4);
        fn_800345E0(this, "\x90\x81\x82\xc1\x94\xf2\x82\xd1");
        setNerve(NRV(Swoon));
        killHeldProjectile();
        return true;
    }
    return false;
}

bool HammerBros::trySpinDamage(HitSensor* pSender, HitSensor* pReceiver) {
    bool ok = isNormalNerve();
    if (!ok) {
        if ((isNerve(NRV(Swoon)) && mStateSwoon->fn_8018B000()) || isNerve(NRV(FreezeSwoon))) {
            ok = true;
        }
        if (!ok && !isNerve(NRV(FreezeSwoon))) {
            return false;
        }
    }
    if (isNormalNerve()) {
        fn_80012B90(this, 22.0f, 2.0f, pSender, pReceiver, 4);
    } else {
        fn_80012B90(this, 0.0f, 2.0f, pSender, pReceiver, 4);
    }
    fn_800345E0(this, "\x83\x60\x83\x52\x90\x81\x82\xc1\x94\xf2\x82\xd1");
    setNerve(NRV(Swoon));
    killHeldProjectile();
    return true;
}

bool HammerBros::tryStomp() {
    bool vuln = false;
    if ((isNerve(NRV(Swoon)) && mStateSwoon->fn_8018B000()) || isNerve(NRV(FreezeSwoon))) {
        vuln = true;
    }
    if (vuln) {
        fn_800345E0(this, "\x82\xc2\x82\xd4\x82\xea");
        setNerve(NRV(SwoonPressDown));
        killHeldProjectile();
        return true;
    }
    if (MR::isPlayerHipDropFalling() && isVulnerable()) {
        fn_800345E0(this, "\x82\xc2\x82\xd4\x82\xea");
        setNerve(NRV(StandPressDown));
        killHeldProjectile();
        return true;
    }
    if (isNormalNerve()) {
        fn_800345E0(this, "\x82\xc2\x82\xd4\x82\xea");
        setNerve(NRV(Swoon));
        killHeldProjectile();
        return true;
    }
    return false;
}

bool HammerBros::tryStarPieceReflect() {
    if (isGuardable()) {
        setNerve(NRV(Guard));
        killHeldProjectile();
        MR::zeroVelocity(this);
        return true;
    }
    return false;
}

bool HammerBros::tryYoshiTongue(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNormalNerve()) {
        MR::addVelocityMoveToTargetHorizon(this, *MR::getPlayerPos(), 22.0f);
        MR::addVelocityMoveToTarget(this, *MR::getPlayerPos(), -2.0f);
        fn_800345E0(this, "\x83\x88\x83\x62\x83\x56\x81\x5b\x90\xe3\x83\x71\x83\x62\x83\x67");
        setNerve(NRV(Swoon));
        killHeldProjectile();
        return false;
    }
    if (isAliveNerve()) {
        MR::addVelocityMoveToTarget(this, *MR::getPlayerPos(), -2.0f);
        fn_800345E0(this, "\x83\x88\x83\x62\x83\x56\x81\x5b\x90\xe3\x83\x71\x83\x62\x83\x67");
        setNerve(NRV(Swoon));
        killHeldProjectile();
        return false;
    }
    return false;
}

bool HammerBros::tryBindStarPointer() {
    if (isStarPointerBindable()) {
        if (mBindStand->tryStartPointBind()) {
            setNerve(NRV(FreezeStand));
            killHeldProjectile();
            return true;
        }
        return false;
    }
    bool swoon = false;
    if (isNerve(NRV(Swoon)) && mStateSwoon->fn_8018B0E0()) {
        swoon = true;
    }
    if (swoon) {
        if (mBindSwoon->tryStartPointBind()) {
            setNerve(NRV(FreezeSwoon));
            killHeldProjectile();
            return true;
        }
        return false;
    }
    return false;
}

void HammerBros::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pSender)) {
        if (MR::isSensorPlayerOrRide(pReceiver)) {
            bool attacking = false;
            if (isNerve(NRV(Wait)) || isNerve(NRV(Jump)) || isNerve(NRV(Attack)) || isNerve(NRV(Guard))) {
                attacking = true;
            }
            if (attacking && !MR::isPlayerHipDropFalling()) {
                MR::sendMsgEnemyAttack(pReceiver, pSender);
            } else if (!isNerve(NRV(SwoonPressDown)) && !isNerve(NRV(StandPressDown))) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    } else if (MR::isSensorYoshi(pReceiver)) {
        if (!isNerve(NRV(SwoonPressDown)) && !isNerve(NRV(StandPressDown))) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool HammerBros::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorPlayerOrYoshiOrRide(pSender)) {
        MR::addVelocityFromPush(this, 10.0f, pSender, pReceiver);
        return true;
    }
    return false;
}

bool HammerBros::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }
    if (MR::isMsgDrillAttack(msg)) {
        return tryExplosionDamage(pSender, pReceiver);
    }
    if (MR::isMsgJetTurtleAttack(msg) || MR::isMsgYoshiProjectile(msg) || MR::isMsgYoshiEat(msg) || MR::isMsgExplosionAttack(msg)) {
        if (tryExplosionDamage(pSender, pReceiver)) {
            fn_80034630(this, "\x93\x8a\x82\xb0\x81\x45\x94\x9a\x94\xad\x95\xa8\x83\x5f\x83\x81\x81\x5b\x83\x57", pSender, pReceiver);
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
    if (MR::isMsgPlayerHipDrop(msg)) {
        return tryStomp();
    }
    if (MR::isMsgPlayerTrample(msg)) {
        return tryStomp();
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
    return false;
}

bool HammerBros::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg) && tryExplosionDamage(pSender, pReceiver)) {
        fn_80034630(this, "\x93\x8a\x82\xb0\x81\x45\x94\x9a\x94\xad\x95\xa8\x83\x5f\x83\x81\x81\x5b\x83\x57", pSender, pReceiver);
    }
    return false;
}

bool HammerBros::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        return tryYoshiTongue(pSender, pReceiver);
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return true;
    }
    if (MR::isMsgYoshiReleaseOrCancelEaten(msg)) {
        fn_8022BAE0(this);
        kill();
        return true;
    }
    if (MR::isMsgPlayerKick(msg)) {
        return tryKickDamage(pSender, pReceiver);
    }
    return false;
}

void HammerBros::throwProjectile(BrosProjectileBase* pProj) {
    MtxPtr pHand = MR::getJointMtx(this, "HandR");
    Mtx a, b, c;
    PSMTXIdentity(a);
    PSMTXIdentity(b);
    PSMTXIdentity(c);
    PSMTXInverse(getBaseMtx(), a);
    PSMTXConcat(a, pHand, b);
    b[0][3] += 20.0f;
    b[1][3] = 260.0f;
    b[2][3] = 125.0f;
    PSMTXConcat(b, getBaseMtx(), c);
    TVec3f trans(0.0f);
    MR::extractMtxTrans(c, &trans);
    pProj->v70(trans, TVec3f(mPosition));
}

HammerBros::~HammerBros() {
}

HammerBrosHammer::HammerBrosHammer(const char* pName) : BrosProjectileBase(pName), mBind(nullptr) {
}

namespace NrvHammerBrosHammer {
    NEW_NERVE(HammerBrosHammerNrvFly, HammerBrosHammer, Fly);
    NEW_NERVE(HammerBrosHammerNrvSpin, HammerBrosHammer, Spin);
    NEW_NERVE(HammerBrosHammerNrvTongue, HammerBrosHammer, Tongue);
    NEW_NERVE(HammerBrosHammerNrvEaten, HammerBrosHammer, Eaten);
    NEW_NERVE(HammerBrosHammerNrvThrown, HammerBrosHammer, Thrown);

    class HammerBrosHammerNrvFreeze : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            HammerBrosHammer* pActor = reinterpret_cast< HammerBrosHammer* >(pSpine->mExecutor);
            pActor->_B4 = true;
            MR::updateActorStateAndNextNerve(pActor, &HammerBrosHammerNrvSpin::sInstance);
        }
        static HammerBrosHammerNrvFreeze sInstance;
    };
    HammerBrosHammerNrvFreeze HammerBrosHammerNrvFreeze::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvHammerBrosHammer

#define HNRV(n) (&NrvHammerBrosHammer::HammerBrosHammerNrv##n::sInstance)

void HammerBrosHammer::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "HammerBrosHammer", false);
    BrosProjectileBase::init(rIter);
    initNerve(HNRV(Fly), 1);
    mBind = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mBind, HNRV(Freeze), "SupportFreeze");
    fn_8022BC70(this);
    makeActorDead();
}

void HammerBrosHammer::appear() {
    BrosProjectileBase::appear();
    setNerve(HNRV(Fly));
    MR::startAction(this, "Normal");
    MR::showModel(this);
}

void HammerBrosHammer::kill() {
    LiveActor::kill();
    if (!isNerve(HNRV(Tongue))) {
        fn_800348C0(this);
    }
}

void HammerBrosHammer::control() {
    BrosProjectileBase::control();
    if (isNerve(HNRV(Spin)) && mBind->tryStartPointBind()) {
        setNerve(HNRV(Freeze));
    }
}

void HammerBrosHammer::exeFly() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Normal");
        MR::offBind(this);
    }
    mScale.x = MR::clamp(1.0f + mScale.x, 0.0f, 1.0f);
    MR::setScale(this, TVec3f(mScale.x));
}

void HammerBrosHammer::exeSpin() {
    if (MR::isFirstStep(this)) {
        _B4 = false;
        if (MR::tryStartActionNotPlaying(this, "Spin")) {
            MR::setBckFrame(this, 1.0f);
        }
    }
    if (MR::isGreaterStep(this, 10) && MR::isNoBind(this)) {
        MR::onBind(this);
    }
    mLocalVel.y -= 0.15f;
    mLocalVel *= 1.0f;
    mScale.x = MR::clamp(1.0f + mScale.x, 0.0f, 1.0f);
    MR::setScale(this, TVec3f(mScale.x));
    if (MR::isBinded(this) || MR::isGreaterStep(this, 240)) {
        if (MR::isBinded(this)) {
            MR::sendMsgToBindedSensor(8, getSensor(nullptr));
        }
        killWithState1();
    }
}

void HammerBrosHammer::exeTongue() {
    if (MR::isFirstStep(this)) {
        MR::deleteEffectAll(this);
        MR::startAction(this, "Normal");
    }
    exeEaten();
}

void HammerBrosHammer::exeThrown() {
    if (MR::isFirstStep(this)) {
        fn_800345E0(this, "\x94\xad\x8e\xcb");
        exeEaten();
        _B4 = false;
        if (MR::tryStartActionNotPlaying(this, "Spin")) {
            MR::setBckFrame(this, 1.0f);
        }
    }
    if (MR::isGreaterStep(this, 10) && MR::isNoBind(this)) {
        MR::onBind(this);
    }
    mLocalVel.y -= 0.3f;
    mLocalVel *= 1.0f;
    mScale.x = MR::clamp(1.0f + mScale.x, 0.0f, 1.0f);
    MR::setScale(this, TVec3f(mScale.x));
    if (MR::isBinded(this) || MR::isGreaterStep(this, 240)) {
        if (MR::isBinded(this)) {
            MR::sendMsgToBindedSensor(8, getSensor(nullptr));
        }
        killWithState4();
    }
}

bool HammerBrosHammer::v78() {
    return isNerve(HNRV(Fly));
}

bool HammerBrosHammer::v7C() {
    return isNerve(HNRV(Tongue));
}

bool HammerBrosHammer::v80() {
    return isNerve(HNRV(Eaten));
}

bool HammerBrosHammer::v84() {
    return isNerve(HNRV(Thrown));
}

bool HammerBrosHammer::v88() {
    if (!v78() && !v7C() && !v80() && !isNerve(HNRV(Fly))) {
        setNerve(HNRV(Tongue));
        mLocalVel = TVec3f(0.0f);
        MR::zeroVelocity(this);
        MR::invalidateHitSensors(this);
        return true;
    }
    return false;
}

bool HammerBrosHammer::v8C() {
    if (v7C()) {
        setNerve(HNRV(Eaten));
        MR::invalidateHitSensors(this);
        MR::hideModel(this);
        return true;
    }
    return false;
}

bool HammerBrosHammer::v90() {
    if (v80()) {
        setNerve(HNRV(Thrown));
        MR::getPlayerThrowVec(&mFront);
        mLocalVel = sThrowVel;
        MR::validateHitSensors(this);
        MR::showModel(this);
        return true;
    }
    return false;
}

HammerBrosHammer::~HammerBrosHammer() {
}
