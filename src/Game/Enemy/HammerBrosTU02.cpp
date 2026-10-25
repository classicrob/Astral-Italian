#include "Game/Util.hpp"
#include "Game/Enemy/BrosState.hpp"
#include "Game/LiveActor/Nerve.hpp"

extern const volatile s32 gBrosAttackReleaseFrame;

// Jump/attack tuning. The target keeps these as named .sdata2 objects (in this order: 6, 48, 0.13f, 8, 0.43f) and
// reloads them at every use; every const spelling tried (plain, late extern definition, volatile) is folded by our
// compiler, so the int uses compile to immediates (NONMATCHING in exeJumpStart; values and behaviour are identical).
namespace {
    const s32 sSeekTargetStep = 6;
    const s32 sJumpFrames = 48;
    const f32 sJumpTurnRate = 0.13f;
    const s32 sHoldProjectileStep = 8;
    const f32 sAttackTurnRate = 0.43f;
}

namespace {
    class NrvBrosStateJumpStart : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class NrvBrosStateJump : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class NrvBrosStateJumpEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class NrvHammerBrosStateAttack : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class NrvHammerBrosStateAttackSecond : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class NrvHammerBrosStateWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class NrvBoomerangBrosStateAttack : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class NrvBoomerangBrosStateAttackSecond : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class NrvBoomerangBrosStateAttackEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class NrvBoomerangBrosStateCatch : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class NrvBrosStateSwoonStart : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class NrvBrosStateSwoon : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class NrvBrosStateSwoonEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };

    NrvBrosStateJumpStart sNrvJumpStart;
    NrvBrosStateJump sNrvJump;
    NrvBrosStateJumpEnd sNrvJumpEnd;
    NrvHammerBrosStateAttack sNrvHammerAttack;
    NrvHammerBrosStateAttackSecond sNrvHammerAttackSecond;
    NrvHammerBrosStateWait sNrvHammerWait;
    NrvBoomerangBrosStateAttack sNrvBoomerangAttack;
    NrvBoomerangBrosStateAttackSecond sNrvBoomerangAttackSecond;
    NrvBoomerangBrosStateAttackEnd sNrvBoomerangAttackEnd;
    NrvBoomerangBrosStateCatch sNrvBoomerangCatch;
    NrvBrosStateSwoonStart sNrvSwoonStart;
    NrvBrosStateSwoon sNrvSwoon;
    NrvBrosStateSwoonEnd sNrvSwoonEnd;
}  // namespace

BrosStateJump::BrosStateJump(BrosBase* pHost)
    : ActorStateBase< BrosBase >("State[\203W\203\203\203\203\203\223\203v]", pHost), mTargetPointNo(0), mCurrentPointNo(0), mHomePos(0.0f, 0.0f, 0.0f),
      mStartPos(0.0f, 0.0f, 0.0f), mTargetPos(0.0f, 0.0f, 0.0f), mFront(0.0f, 0.0f, 0.0f), mJumpVelocity(0.0f, 0.0f, 0.0f) {}

void BrosStateJump::init() {
    initNerve(&sNrvJumpStart, 0);
    mHomePos = mHost->mPosition;
    kill();
}

void BrosStateJump::appear() {
    ActorStateBaseInterface::appear();
    mHost->startFaceCtrl();
    setNerve(&sNrvJumpStart);
    mFront = TVec3f(0.0f, 0.0f, 1.0f);
    mJumpVelocity = TVec3f(0.0f);
}

// NONMATCHING: sJumpFrames/sSeekTargetStep/sHoldProjectileStep are reloaded from .sdata2 in the target (see top);
// stack slot order / scheduling of the int->float conversions.
void BrosStateJump::exeJumpStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "JumpStart");
        mStep = 0;
    }
    if (MR::isStep(this, sHoldProjectileStep)) {
        if (!mHost->mIsInitialAttack) {
            mHost->tryHoldProjectile();
        }
    }
    if (MR::isStep(this, sSeekTargetStep)) {
        mStep = 0;
        selectTarget();
        BrosBase* pHost = mHost;
        TVec3f gravStart(0.0f);
        TVec3f gravEnd(0.0f);
        MR::calcGravityVector(pHost, mStartPos, &gravStart, nullptr, 0);
        MR::calcGravityVector(pHost, mTargetPos, &gravEnd, nullptr, 0);
        TVec3f hitStart(0.0f);
        if (!MR::getFirstPolyOnLineToMap(&hitStart, nullptr, mStartPos - gravStart * 500.0f, gravStart * 1000.0f)) {
            hitStart = mStartPos;
        }
        TVec3f hitTarget(0.0f);
        if (!MR::getFirstPolyOnLineToMap(&hitTarget, nullptr, mTargetPos - gravEnd * 500.0f, gravEnd * 1000.0f)) {
            hitTarget = mTargetPos;
        }
        if (MR::isSameDirection(gravEnd, gravStart, 0.01f)) {
            f32 height = gravStart.dot(mStartPos - hitTarget);
            mJumpVelocity.x = 0.0f;
            mJumpVelocity.z = 0.0f;
            mJumpVelocity.y = height / (f32)sJumpFrames + 0.5f * (f32)sJumpFrames * 1.5f;
            TVec3f horizontal;
            MR::vecKillElement(hitTarget - mStartPos, gravStart, &horizontal);
            mJumpVelocity.z += PSVECMag(&horizontal) / (f32)sJumpFrames;
            mFront = horizontal;
        } else {
            f32 startHeight = gravStart.dot(mStartPos - hitStart);
            f32 targetHeight = gravStart.dot(mTargetPos - hitTarget);
            mJumpVelocity.x = 0.0f;
            mJumpVelocity.z = 0.0f;
            mJumpVelocity.y = (targetHeight - startHeight) / (f32)sJumpFrames + 0.5f * (f32)sJumpFrames * 1.5f;
            TVec3f xDir(1.0f, 0.0f, 0.0f);
            TVec3f diff = hitTarget - mStartPos;
            if (!MR::normalizeOrZero(&diff)) {
                TPos3f mtx;
                PSMTXIdentity(mtx.toMtxPtr());
                MR::makeMtxUpFront(&mtx, -gravStart, diff);
                MR::extractMtxXDir(mtx.toMtxPtr(), &xDir);
                MR::extractMtxZDir(mtx.toMtxPtr(), &mFront);
                f32 angle = MR::diffAngleSigned(gravStart, gravEnd, xDir);
                f32 mag = PSVECMag(&diff);
                f32 dh = targetHeight - startHeight;
                f32 horiz = MR::sqrt< f32 >(mag * mag - dh * dh);
                MR::sin(0.5f * angle);
                mJumpVelocity.z = 2.0f * MR::sqrt< f32 >(0.5f * (horiz * horiz)) / (f32)sJumpFrames;
            }
        }
    }
    if (MR::isGreaterEqualStep(this, sSeekTargetStep)) {
        updateJump();
        if (mStep >= sJumpFrames - 1) {
            setNerve(&sNrvJumpEnd);
            return;
        }
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&sNrvJump);
    }
}

void BrosStateJump::exeJump() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Jump");
    }
    updateJump();
    if (mStep >= sJumpFrames - 1) {
        setNerve(&sNrvJumpEnd);
    }
}

void BrosStateJump::exeJumpEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "JumpEnd");
        updateJump();
        if (MR::isExistRail(mHost)) {
            MR::moveCoordToRailPoint(mHost, mTargetPointNo);
            if (!MR::isLoopRail(mHost)) {
                if (MR::isRailReachedEdge(mHost)) {
                    MR::reverseRailDirection(mHost);
                }
            }
        }
    } else {
        MR::zeroVelocity(mHost);
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

void BrosStateJump::selectTarget() {
    mStartPos = mHost->mPosition;
    if (MR::isExistRail(mHost)) {
        if (mHost->mAllowRailJump && MR::getRailPointNum(mHost) >= 3) {
            mTargetPointNo = mCurrentPointNo;
            f32 maxDist = 0.0f;
            for (s32 i = -2; i < 2; i++) {
                if (i != 0) {
                    s32 idx = i + MR::getCurrentRailPointNo(mHost);
                    s32 num = MR::getRailPointNum(mHost);
                    idx = (num + idx) % num;
                    if (MR::getRailPointNum(mHost) < 4 || idx != mCurrentPointNo) {
                        TVec3f pos(0.0f);
                        MR::calcRailPointPos(&pos, mHost, idx);
                        f32 dist = MR::calcDistanceToPlayer(pos);
                        if (dist > maxDist) {
                            maxDist = dist;
                            mTargetPointNo = idx;
                        }
                    }
                }
            }
            mCurrentPointNo = MR::getCurrentRailPointNo(mHost);
        } else {
            mTargetPointNo = MR::getNextRailPointNo(mHost);
        }
        MR::calcRailPointPos(&mTargetPos, mHost, mTargetPointNo);
        return;
    }
    mTargetPos = mHomePos;
}

// NONMATCHING: stack slots of the dummy TVec3 and the negated-gravity temp are swapped (only addi r1 offsets differ).
namespace {
    inline void turnJumpFrontToPlayer(BrosBase* host, f32 turnRate) {
        TVec3f toPlayer(0.0f);
        MR::calcVecToPlayerH(&toPlayer, host, &host->mGravity);
        MR::vecBlendSphere(&host->mFront, host->mFront, toPlayer, turnRate);
    }
}

void BrosStateJump::updateJump() {
    mJumpVelocity.y -= 1.5f;
    TPos3f mtx;
    PSMTXIdentity(mtx.toMtxPtr());
    MR::makeMtxUpFront(&mtx, -mHost->mGravity, mFront);
    MR::extractMtxZDir(mtx.toMtxPtr(), &mFront);
    TVec3f dummy(0.0f);
    PSMTXMultVecSR(mtx.toMtxPtr(), &mJumpVelocity, &mHost->mVelocity);
    BrosBase* pHost = mHost;
    turnJumpFrontToPlayer(pHost, sJumpTurnRate);
    mStep++;
}

// Hammer attack: throws a hammer, then a second one, then waits.
HammerBrosStateAttack::HammerBrosStateAttack(BrosBase* pHost) : ActorStateBase< BrosBase >("State[\215U\214\202]", pHost), mAttackCount(0) {}

void HammerBrosStateAttack::init() {
    initNerve(&sNrvHammerAttack, 0);
    kill();
}

void HammerBrosStateAttack::appear() {
    mHost->startFaceCtrl();
    ActorStateBaseInterface::appear();
    setNerve(&sNrvHammerAttack);
    mAttackCount = 1;
}

void HammerBrosStateAttack::exeAttack() {
    s32 throwFrame = 28;
    mHost->updateVelocity();
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Attack");
    }
    if (mHost->mIsInitialAttack) {
        if (MR::getBckFrame(mHost) >= (f32)gBrosAttackReleaseFrame) {
            if (MR::getBckFrame(mHost) < (f32)throwFrame) {
                mHost->tryHoldProjectile();
            }
        }
    }
    if (MR::getBckFrame(mHost) < (f32)throwFrame) {
        BrosBase* pHost = mHost;
        TVec3f toPlayer(0.0f);
        MR::calcVecToPlayerH(&toPlayer, pHost, &pHost->mGravity);
        MR::vecBlendSphere(&pHost->mFront, pHost->mFront, toPlayer, sAttackTurnRate);
    }
    if (MR::getBckFrame(mHost) >= (f32)throwFrame) {
        BrosBase* pHost = mHost;
        if (MR::getBckFrame(pHost) < (f32)throwFrame + MR::getBckRate(pHost)) {
            mHost->tryThrowProjectile();
        }
    }
    if (MR::isActionEnd(mHost)) {
        mAttackCount++;
        setNerve(&sNrvHammerAttackSecond);
    }
}

void HammerBrosStateAttack::exeAttackSecond() {
    s32 throwFrame = 28;
    mHost->updateVelocity();
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "AttackSecond");
    }
    if (MR::getBckFrame(mHost) < (f32)throwFrame) {
        BrosBase* pHost = mHost;
        TVec3f toPlayer(0.0f);
        MR::calcVecToPlayerH(&toPlayer, pHost, &pHost->mGravity);
        MR::vecBlendSphere(&pHost->mFront, pHost->mFront, toPlayer, sAttackTurnRate);
    }
    if (MR::getBckFrame(mHost) >= (f32)gBrosAttackReleaseFrame) {
        if (MR::getBckFrame(mHost) < (f32)throwFrame) {
            mHost->tryHoldProjectile();
        }
    }
    if (MR::getBckFrame(mHost) >= (f32)throwFrame) {
        BrosBase* pHost = mHost;
        if (MR::getBckFrame(pHost) < (f32)throwFrame + MR::getBckRate(pHost)) {
            mHost->tryThrowProjectile();
        }
    }
    if (MR::isActionEnd(mHost)) {
        mAttackCount++;
        if (mAttackCount < 2) {
            setNerve(&sNrvHammerAttackSecond);
        } else {
            setNerve(&sNrvHammerWait);
        }
    }
}

void HammerBrosStateAttack::exeWait() {
    mHost->updateVelocity();
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Wait");
    }
    if (MR::isGreaterEqualStep(this, 53)) {
        kill();
    }
}

// Boomerang attack: throws a boomerang, a second one, then waits for the catch.
BoomerangBrosStateAttack::BoomerangBrosStateAttack(BrosBase* pHost) : ActorStateBase< BrosBase >("State[\215U\214\202]", pHost), mAttackCount(0) {}

void BoomerangBrosStateAttack::init() {
    initNerve(&sNrvBoomerangAttack, 0);
    kill();
}

void BoomerangBrosStateAttack::appear() {
    mHost->startFaceCtrl();
    mIsFaceCtrl = true;
    ActorStateBaseInterface::appear();
    setNerve(&sNrvBoomerangAttack);
    mAttackCount = 1;
    mTimer = 0;
}

void BoomerangBrosStateAttack::exeAttack() {
    s32 throwFrame = 28;
    mHost->updateVelocity();
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Attack");
    }
    if (mHost->mIsInitialAttack) {
        if (MR::getBckFrame(mHost) >= (f32)gBrosAttackReleaseFrame) {
            if (MR::getBckFrame(mHost) < (f32)throwFrame) {
                mHost->tryHoldProjectile();
            }
        }
    }
    if (MR::getBckFrame(mHost) < (f32)throwFrame) {
        BrosBase* pHost = mHost;
        TVec3f toPlayer(0.0f);
        MR::calcVecToPlayerH(&toPlayer, pHost, &pHost->mGravity);
        MR::vecBlendSphere(&pHost->mFront, pHost->mFront, toPlayer, sAttackTurnRate);
    }
    if (MR::getBckFrame(mHost) >= (f32)throwFrame) {
        BrosBase* pHost = mHost;
        if (MR::getBckFrame(pHost) < (f32)throwFrame + MR::getBckRate(pHost)) {
            mHost->tryThrowProjectile();
        }
    }
    if (MR::isActionEnd(mHost)) {
        mAttackCount++;
        setNerve(&sNrvBoomerangAttackSecond);
    }
}

void BoomerangBrosStateAttack::exeAttackSecond() {
    s32 throwFrame = 28;
    mHost->updateVelocity();
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "AttackSecond");
    }
    if (MR::getBckFrame(mHost) < (f32)throwFrame) {
        BrosBase* pHost = mHost;
        TVec3f toPlayer(0.0f);
        MR::calcVecToPlayerH(&toPlayer, pHost, &pHost->mGravity);
        MR::vecBlendSphere(&pHost->mFront, pHost->mFront, toPlayer, sAttackTurnRate);
    }
    if (MR::getBckFrame(mHost) >= 0.0f) {
        if (MR::getBckFrame(mHost) < (f32)throwFrame) {
            mHost->tryHoldProjectile();
        }
    }
    if (MR::getBckFrame(mHost) >= (f32)throwFrame) {
        BrosBase* pHost = mHost;
        if (MR::getBckFrame(pHost) < (f32)throwFrame + MR::getBckRate(pHost)) {
            mHost->tryThrowProjectile();
        }
    }
    if (MR::isActionEnd(mHost)) {
        mAttackCount++;
        if (mAttackCount < 2) {
            setNerve(&sNrvBoomerangAttackSecond);
            return;
        }
        mTimer = 240;
        setNerve(&sNrvBoomerangAttackEnd);
    }
}

void BoomerangBrosStateAttack::exeAttackEnd() {
    mHost->updateVelocity();
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "AttackEnd");
    }
    mTimer--;
    bool noProjectile = !mHost->isExistActiveProjectile();
    if (mTimer <= 0 || noProjectile) {
        kill();
        return;
    }
    if (mIsFaceCtrl && mHost->isExistProjectileNear(450.0f)) {
        mIsFaceCtrl = false;
        mHost->endFaceCtrl();
    }
}

void BoomerangBrosStateAttack::exeCatch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Catch");
        mHost->endFaceCtrl();
    }
    s32 timer = mTimer;
    mTimer = timer - 1;
    if (MR::isStep(this, 24)) {
        mHost->killHeldProjectile();
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&sNrvBoomerangAttackEnd);
    }
}

bool BoomerangBrosStateAttack::tryCatch() {
    if (!mIsDead && isNerve(&sNrvBoomerangAttackEnd)) {
        setNerve(&sNrvBoomerangCatch);
        return true;
    }
    return false;
}

bool BoomerangBrosStateAttack::isAttackEnd() const {
    return isNerve(&sNrvBoomerangAttackEnd);
}

// Swoon: lies on the ground, gets up after a while.
BrosStateSwoon::BrosStateSwoon(BrosBase* pHost) : ActorStateBase< BrosBase >("State[\202\320\202\351\202\335]", pHost), mTimer(0), mStartWithRecovery(false) {}

void BrosStateSwoon::init() {
    initNerve(&sNrvSwoonStart, 0);
    kill();
}

void BrosStateSwoon::appear() {
    mHost->endFaceCtrl();
    ActorStateBaseInterface::appear();
    if (mStartWithRecovery) {
        setNerve(&sNrvSwoonEnd);
    } else {
        setNerve(&sNrvSwoonStart);
    }
    mTimer = 0;
    mStartWithRecovery = false;
}

void BrosStateSwoon::control() {
    s32 timer = mTimer + 1;
    mTimer = timer;
    if (timer > 180) {
        if (!isNerve(&sNrvSwoonEnd) && MR::isBindedGround(mHost)) {
            setNerve(&sNrvSwoonEnd);
        }
    }
}

void BrosStateSwoon::requestBrosSwoonRecovery() {
    mStartWithRecovery = true;
}

bool BrosStateSwoon::isBrosSwoonVulnerable() const {
    bool ret = false;
    if ((isNerve(&sNrvSwoonStart) && mTimer > 24) || isNerve(&sNrvSwoon) || isNerve(&sNrvSwoonEnd)) {
        ret = true;
    }
    return ret;
}

bool BrosStateSwoon::isBrosSwoonRecoveryComplete() const {
    bool ret = false;
    if (isNerve(&sNrvSwoonEnd) && MR::isGreaterStep(this, 60)) {
        ret = true;
    }
    return ret;
}

bool BrosStateSwoon::canBrosSwoonBindStarPointer() const {
    bool ret = false;
    if (isNerve(&sNrvSwoon) || (isNerve(&sNrvSwoonEnd) && MR::isLessStep(this, 60))) {
        ret = true;
    }
    return ret;
}

void BrosStateSwoon::exeSwoonStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "SwoonStart");
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&sNrvSwoon);
    }
}

void BrosStateSwoon::exeSwoonEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "SwoonEnd");
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

inline void BrosStateSwoon::exeSwoon() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Swoon");
    }
}

BrosStateJump::~BrosStateJump() {}

HammerBrosStateAttack::~HammerBrosStateAttack() {}

BoomerangBrosStateAttack::~BoomerangBrosStateAttack() {}

BrosStateSwoon::~BrosStateSwoon() {}



void NrvBrosStateSwoonEnd::execute(Spine* pSpine) const {
    reinterpret_cast< BrosStateSwoon* >(pSpine->mExecutor)->exeSwoonEnd();
}
void NrvBrosStateSwoon::execute(Spine* pSpine) const {
    reinterpret_cast< BrosStateSwoon* >(pSpine->mExecutor)->exeSwoon();
}
void NrvBrosStateSwoonStart::execute(Spine* pSpine) const {
    reinterpret_cast< BrosStateSwoon* >(pSpine->mExecutor)->exeSwoonStart();
}
void NrvBoomerangBrosStateCatch::execute(Spine* pSpine) const {
    reinterpret_cast< BoomerangBrosStateAttack* >(pSpine->mExecutor)->exeCatch();
}
void NrvBoomerangBrosStateAttackEnd::execute(Spine* pSpine) const {
    reinterpret_cast< BoomerangBrosStateAttack* >(pSpine->mExecutor)->exeAttackEnd();
}
void NrvBoomerangBrosStateAttackSecond::execute(Spine* pSpine) const {
    reinterpret_cast< BoomerangBrosStateAttack* >(pSpine->mExecutor)->exeAttackSecond();
}
void NrvBoomerangBrosStateAttack::execute(Spine* pSpine) const {
    reinterpret_cast< BoomerangBrosStateAttack* >(pSpine->mExecutor)->exeAttack();
}
void NrvHammerBrosStateWait::execute(Spine* pSpine) const {
    reinterpret_cast< HammerBrosStateAttack* >(pSpine->mExecutor)->exeWait();
}
void NrvHammerBrosStateAttackSecond::execute(Spine* pSpine) const {
    reinterpret_cast< HammerBrosStateAttack* >(pSpine->mExecutor)->exeAttackSecond();
}
void NrvHammerBrosStateAttack::execute(Spine* pSpine) const {
    reinterpret_cast< HammerBrosStateAttack* >(pSpine->mExecutor)->exeAttack();
}
void NrvBrosStateJumpEnd::execute(Spine* pSpine) const {
    reinterpret_cast< BrosStateJump* >(pSpine->mExecutor)->exeJumpEnd();
}
void NrvBrosStateJump::execute(Spine* pSpine) const {
    reinterpret_cast< BrosStateJump* >(pSpine->mExecutor)->exeJump();
}
void NrvBrosStateJumpStart::execute(Spine* pSpine) const {
    reinterpret_cast< BrosStateJump* >(pSpine->mExecutor)->exeJumpStart();
}

extern const volatile s32 gBrosAttackReleaseFrame = 0;
