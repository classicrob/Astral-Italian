#include "Game/Enemy/Togezo.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, const char*, const TVec3f&);
};  // namespace MR

namespace YoshiFunction {
    void tryYoshiProjectile(YoshiLockOnTarget*);
    void tryYoshiEat(YoshiLockOnTarget*);
};  // namespace YoshiFunction

namespace MR {
    bool isMsgYoshiStartTongue(u32);
    bool isMsgYoshiLoopTongue(u32);
    bool isMsgYoshiStartEaten(u32);
    bool isMsgYoshiCancelEaten(u32);
    bool isMsgYoshiReleaseEaten(u32);
    bool isMsgEnemyAttackElectric(u32);
    bool isMsgExplosionAttack(u32);
    bool isMsgToEnemyAttackBlow(u32);
    bool isMsgToEnemyAttackTrampleDeath(u32);
    bool isMsgInhaleBlackHole(u32);
};  // namespace MR

// Unnamed SMG2 helpers outside this unit.
extern "C" void fn_8022BA10(LiveActor*);
extern "C" void fn_80012B90(LiveActor*, f32, f32, HitSensor*, HitSensor*, s32);
extern "C" void fn_800137F0(LiveActor*, TVec3f*, const TVec3f*, f32, f32, f32, f32, f32);
extern "C" void fn_80422350(TPos3f*);
extern "C" void fn_8000E960(TVec3f*, const LiveActor*, const TVec3f&);

namespace {
    static const MR::ActorMoveParam hBoundParam = {0.65f, 20.0f, 0.3f, 0.0f};
    static const MR::ActorMoveParam hRollParam = {0.0f, 2.0f, 0.9f, 0.0f};
    static const MR::ActorMoveParam hStopParam = {0.0f, 2.0f, 0.6f, 0.0f};
    static const MR::ActorMoveParam hRunParam = {0.1f, 0.1f, 0.95f, 0.5f};
    static const MR::ActorMoveParam hWalkParam = {0.2f, 2.0f, 0.8f, 2.0f};
    static const MR::ActorMoveParam hAttackParam = {0.8f, 2.0f, 0.9f, 0.8f};
    static const MR::ActorMoveParam hTurnParam = {0.0f, 2.0f, 0.85f, 4.0f};
    static const MR::ActorMoveParam hTurnBackParam = {0.0f, 2.0f, 0.85f, 2.0f};
};  // namespace

// Nerve helpers whose bodies are inlined into the nerve (see NOTES: defined before NEW_NERVE).
void Togezo::exeTongue() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
    }
}

void Togezo::exeTongueHold() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
    }
}

void Togezo::exeEaten() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
    }
    calcYoshiDirection();
}

void Togezo::exeCancelEaten() {
    MR::validateClipping(this);
    mPipo->kill();
    kill();
}

namespace NrvTogezo {
    NEW_NERVE(TogezoNrvHold, Togezo, Hold);
    NEW_NERVE(TogezoNrvBound, Togezo, Bound);
    NEW_NERVE(TogezoNrvAppearDirect, Togezo, AppearDirect);
    NEW_NERVE(TogezoNrvAppear, Togezo, Appear);
    NEW_NERVE(TogezoNrvWalkOnRail, Togezo, WalkOnRail);
    NEW_NERVE(TogezoNrvTurnBack, Togezo, TurnBack);
    NEW_NERVE(TogezoNrvWait, Togezo, Wait);
    NEW_NERVE(TogezoNrvWalk, Togezo, Walk);
    NEW_NERVE(TogezoNrvTurn, Togezo, Turn);
    NEW_NERVE(TogezoNrvSearch, Togezo, Search);
    NEW_NERVE(TogezoNrvRun, Togezo, Run);
    NEW_NERVE(TogezoNrvAttack, Togezo, Attack);
    NEW_NERVE(TogezoNrvAttackEnd, Togezo, AttackEnd);
    NEW_NERVE(TogezoNrvDown, Togezo, Down);
    NEW_NERVE(TogezoNrvDeath, Togezo, Death);
    NEW_NERVE(TogezoNrvSpinReaction, Togezo, SpinReaction);
    NEW_NERVE(TogezoNrvBindStarPointer, Togezo, BindStarPointer);
    NEW_NERVE(TogezoNrvShellStart, Togezo, ShellStart);
    NEW_NERVE(TogezoNrvShellStartLand, Togezo, ShellStartLand);
    NEW_NERVE(TogezoNrvShellWait, Togezo, ShellWait);
    NEW_NERVE(TogezoNrvTongue, Togezo, Tongue);
    NEW_NERVE(TogezoNrvTongueHold, Togezo, TongueHold);
    NEW_NERVE(TogezoNrvEaten, Togezo, Eaten);
    NEW_NERVE(TogezoNrvShot, Togezo, Shot);
    NEW_NERVE(TogezoNrvCancelEaten, Togezo, CancelEaten);
};  // namespace NrvTogezo

// Groups of nerves that appear inlined (as a flag variable) in several functions.
#define NRV_GROUP_GROUNDED(res)                                                                                                                      \
    bool res = false;                                                                                                                                \
    if (isNerve(&NrvTogezo::TogezoNrvBound::sInstance) || isNerve(&NrvTogezo::TogezoNrvHold::sInstance) ||                                           \
        isNerve(&NrvTogezo::TogezoNrvAppearDirect::sInstance) || isNerve(&NrvTogezo::TogezoNrvAppear::sInstance)) {                                  \
        res = true;                                                                                                                                  \
    }

#define NRV_GROUP_SHELL(res)                                                                                                                         \
    bool res = false;                                                                                                                                \
    if (isNerve(&NrvTogezo::TogezoNrvShellStart::sInstance) || isNerve(&NrvTogezo::TogezoNrvShellStartLand::sInstance) ||                            \
        isNerve(&NrvTogezo::TogezoNrvShellWait::sInstance)) {                                                                                        \
        res = true;                                                                                                                                  \
    }

Togezo::Togezo(const char* pName)
    : LiveActor(pName), mPipo(nullptr), mFrontVec(0.0f, 0.0f, 1.0f), mHomePos(0.0f, 0.0f, 0.0f), mWalkDir(0.0f, 0.0f, 1.0f), mGroundCounter(0), mTimer(0), mHasAttacked(false),
      mYoshiLockOn(nullptr), mHoldMtx(nullptr), mIsYoshi(false) {
}

void Togezo::init(const JMapInfoIter& rIter) {
    initActorInfo(rIter);
    mPipo = MR::createModelObjMapObjDecoration("\x83\x67\x83\x51\x83\x5D\x81\x5B\x83\x70\x83\x43\x83\x7C", "TogezoPaipo", getBaseMtx());
    mPipo->makeActorDead();
    MR::calcFrontVec(&mFrontVec, this);
    mHomePos.set(mPosition);
    mWalkDir.set(mFrontVec);
    MR::initUseStageSwitchWriteDead(this, rIter);
    MR::useStageSwitchAwake(this, rIter);
    MR::onCalcGravity(this);
    if (MR::isConnectedWithRail(rIter)) {
        initRailRider(rIter);
        initNerve(&NrvTogezo::TogezoNrvWalkOnRail::sInstance, 1);
    } else {
        initNerve(&NrvTogezo::TogezoNrvWait::sInstance, 1);
    }
    mScaleController = new AnimScaleController(nullptr);
    mStateBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mStateBindStarPointer, &NrvTogezo::TogezoNrvBindStarPointer::sInstance, "SupportFreeze");
    TVec3f zero;
    zero.setAll< f32 >(0.0f);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, "Center", zero);
    makeActorAppeared();
}

void Togezo::initAfterPlacement() {
    MR::trySetMoveLimitCollision(this);
}

void Togezo::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::validateExCollisionParts(this);
}

void Togezo::makeActorDead() {
    mPipo->kill();
    ModelObj* pPipo = mPipo;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    pPipo->mScale.x = 1.0f;
    pPipo->mScale.y = 1.0f;
    pPipo->mScale.z = 1.0f;
    LiveActor::makeActorDead();
    mHoldMtx = nullptr;
}

void Togezo::kill() {
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
    LiveActor::kill();
}

void Togezo::appearBound(const TVec3f& rPos, const TVec3f& rVel) {
    MR::resetPosition(this, rPos);
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    mVelocity.set(rVel);
    ModelObj* pPipo = mPipo;
    pPipo->mScale.x = 1.0f;
    pPipo->mScale.y = 1.0f;
    pPipo->mScale.z = 1.0f;
    appear();
    setNerve(&NrvTogezo::TogezoNrvBound::sInstance);
    MR::invalidateClipping(this);
    mHoldMtx = nullptr;
}

void Togezo::appearHold(MtxPtr pMtx) {
    mHoldMtx = pMtx;
    TVec3f trans;
    trans.setAll< f32 >(0.0f);
    MR::extractMtxTrans(pMtx, &trans);
    MR::resetPosition(this, trans);
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    MR::zeroVelocity(this);
    ModelObj* pPipo = mPipo;
    pPipo->mScale.x = 1.0f;
    pPipo->mScale.y = 1.0f;
    pPipo->mScale.z = 1.0f;
    MR::resetPosition(mPipo, trans);
    appear();
    setNerve(&NrvTogezo::TogezoNrvHold::sInstance);
    MR::invalidateClipping(this);
}

void Togezo::releaseHold(const TVec3f& rVel) {
    if (isNerve(&NrvTogezo::TogezoNrvHold::sInstance)) {
        if (mHoldMtx == nullptr) {
            kill();
            return;
        }
        TVec3f trans;
        trans.setAll< f32 >(0.0f);
        MR::extractMtxTrans(mHoldMtx, &trans);
        MR::resetPosition(this, trans);
        MR::extractMtxZDir(mHoldMtx, &mFrontVec);
        mHoldMtx = nullptr;
        mVelocity.set(rVel);
        setNerve(&NrvTogezo::TogezoNrvBound::sInstance);
    }
}

void Togezo::setTimer(s32 timer) {
    mTimer = timer;
}

void Togezo::updateTimer() {
    if (--mTimer <= 0 && isAttackable()) {
        NRV_GROUP_GROUNDED(grounded);
        if (!grounded) {
            NRV_GROUP_SHELL(shell);
            if (!shell) {
                MR::tryHitReactionDead(this);
                kill();
            }
        }
    }
}

bool Togezo::isYoshiState() const {
    bool result = false;
    if (isNerve(&NrvTogezo::TogezoNrvTongue::sInstance) || isNerve(&NrvTogezo::TogezoNrvTongueHold::sInstance) ||
        isNerve(&NrvTogezo::TogezoNrvEaten::sInstance) || isNerve(&NrvTogezo::TogezoNrvShot::sInstance)) {
        result = true;
    }
    return result;
}

bool Togezo::isShot() const {
    return isNerve(&NrvTogezo::TogezoNrvShot::sInstance);
}

void Togezo::exeHold() {
    if (MR::isFirstStep(this)) {
        startRoll();
        MR::startAction(mPipo, "Hold");
    }
}

void Togezo::exeBound() {
    if (MR::isFirstStep(this)) {
        startRoll();
    }
    if (!MR::isOnGround(this)) {
        MR::addVelocityToGravity(this, 0.5f);
        MR::attenuateVelocity(this, 0.995f);
    } else {
        MR::attenuateVelocity(this, 0.9f);
    }
    MR::turnDirectionDegree(this, &mFrontVec, mVelocity, 1.0f);
    bool rebounded = MR::reboundVelocityFromCollision(this, hBoundParam._0, hBoundParam._4, hBoundParam._8);
    f32 speed = -MR::calcGravitySpeed(this);
    if (rebounded) {
        MR::startActionSound(this, "Bound", (s32)(100.0f * speed), -1, -1);
    }
    if (MR::isBindedGround(this) && speed < hBoundParam._4) {
        setNerve(&NrvTogezo::TogezoNrvAppearDirect::sInstance);
    }
}

void Togezo::exeAppearDirect() {
    TVec3f dir;
    dir.setAll< f32 >(0.0f);
    MR::calcVecToPlayerH(&dir, this, nullptr);
    MR::turnDirectionDegree(this, &mFrontVec, dir, 30.0f);
    if (MR::isFirstStep(this)) {
        mPipo->kill();
        MR::showModel(this);
        MR::startAction(this, "AppearDirect");
        MR::invalidateClipping(this);
    }
    TVec3f toPlayer;
    toPlayer.sub(*MR::getPlayerPos(), mPosition);
    MR::normalize(&toPlayer);
    moveAndTurn(hStopParam, toPlayer);
    if (MR::isActionEnd(this)) {
        setNerve(&NrvTogezo::TogezoNrvSearch::sInstance);
    }
}

void Togezo::exeAppear() {
    if (MR::isFirstStep(this)) {
        mPipo->kill();
        MR::showModel(this);
        MR::startAction(this, "Appear");
        MR::invalidateClipping(this);
    }
    TVec3f toPlayer;
    toPlayer.sub(*MR::getPlayerPos(), mPosition);
    MR::normalize(&toPlayer);
    moveAndTurn(hStopParam, toPlayer);
    if (MR::isActionEnd(this)) {
        setNerve(&NrvTogezo::TogezoNrvTurn::sInstance);
    }
}

void Togezo::exeWalkOnRail() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
    }
    fn_800137F0(this, &mFrontVec, nullptr, 600.0f, hRunParam._0, hRunParam._4, hRunParam._8, hRunParam._C);
    tryAttack();
}

void Togezo::exeTurnBack() {
    TVec3f walkSide;
    TVec3f frontSide;
    TVec3f toPlayer;
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Turn");
        mWalkDir.set(mFrontVec);
        MR::rotateVecDegree(&mWalkDir, mGravity, 60.0f);
    }
    toPlayer.sub(*MR::getPlayerPos(), mPosition);
    MR::normalize(&toPlayer);
    moveAndTurn(hStopParam, toPlayer);
    moveAndTurn(hTurnBackParam, mWalkDir);
    MR::vecKillElement(mWalkDir, mGravity, &walkSide);
    MR::vecKillElement(mFrontVec, mGravity, &frontSide);
    if (MR::isSameDirection(walkSide, frontSide, 0.1f)) {
        if (MR::isFallNextMove(mPosition, mFrontVec, mGravity, 200.0f, 100.0f, 60.0f, nullptr)) {
            MR::rotateVecDegree(&mWalkDir, mGravity, 60.0f);
            return;
        }
        setNerve(&NrvTogezo::TogezoNrvWait::sInstance);
    }
}

void Togezo::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "Wait");
    }
    TVec3f toPlayer;
    toPlayer.sub(*MR::getPlayerPos(), mPosition);
    MR::normalize(&toPlayer);
    moveAndTurn(hStopParam, toPlayer);
    if (MR::isGreaterStep(this, 60)) {
        tryAttack();
    }
    if (MR::isStep(this, 120)) {
        if (MR::isFallNextMove(mPosition, mWalkDir, mGravity, 200.0f, 100.0f, 60.0f, nullptr)) {
            setNerve(&NrvTogezo::TogezoNrvTurnBack::sInstance);
            return;
        }
        setNerve(&NrvTogezo::TogezoNrvWalk::sInstance);
    }
}

void Togezo::exeWalk() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "Walk");
        mWalkDir.set(mFrontVec);
        MR::rotateVecDegree(&mWalkDir, mGravity, (f32)MR::getRandom((s32)-30, (s32)30));
    }
    if (MR::isGreaterStep(this, 180)) {
        setNerve(&NrvTogezo::TogezoNrvWait::sInstance);
        return;
    }
    if (MR::isFallNextMove(mPosition, mWalkDir, mGravity, 200.0f, 100.0f, 60.0f, nullptr)) {
        setNerve(&NrvTogezo::TogezoNrvWait::sInstance);
        return;
    }
    bool far = !MR::isNear(this, mHomePos, 500.0f);
    if (far) {
        mWalkDir.sub(mHomePos, mPosition);
        MR::normalize(&mWalkDir);
    }
    moveAndTurn(hRunParam, mWalkDir);
    tryAttack();
}

void Togezo::exeTurn() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "Turn");
    }
    TVec3f toPlayer;
    toPlayer.sub(*MR::getPlayerPos(), mPosition);
    MR::normalize(&toPlayer);
    moveAndTurn(hTurnParam, toPlayer);
    if (MR::isFaceToPlayerHorizontalDegree(this, mFrontVec, 10.0f)) {
        setNerve(&NrvTogezo::TogezoNrvSearch::sInstance);
    }
}

void Togezo::exeSearch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Search");
        MR::validateClipping(this);
    }
    TVec3f toPlayer;
    toPlayer.sub(*MR::getPlayerPos(), mPosition);
    MR::normalize(&toPlayer);
    moveAndTurn(hStopParam, toPlayer);
    if (MR::isActionEnd(this)) {
        setNerve(&NrvTogezo::TogezoNrvRun::sInstance);
    }
}

void Togezo::exeRun() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Run");
        MR::validateClipping(this);
    }
    if (MR::isLessEqualStep(this, 20)) {
        TVec3f toPlayer;
        toPlayer.sub(*MR::getPlayerPos(), mPosition);
        MR::normalize(&toPlayer);
        moveAndTurn(hWalkParam, toPlayer);
    } else {
        TVec3f toPlayer;
        toPlayer.sub(*MR::getPlayerPos(), mPosition);
        MR::normalize(&toPlayer);
        moveAndTurn(hAttackParam, toPlayer);
    }
    tryTurnBackOrRail();
}

void Togezo::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::validateClipping(this);
    }
    TVec3f toPlayer;
    toPlayer.sub(*MR::getPlayerPos(), mPosition);
    MR::normalize(&toPlayer);
    moveAndTurn(hStopParam, toPlayer);
    if (MR::isActionEnd(this)) {
        setNerve(&NrvTogezo::TogezoNrvAttackEnd::sInstance);
    }
}

void Togezo::exeAttackEnd() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "AttackEnd");
    }
    TVec3f toPlayer;
    toPlayer.sub(*MR::getPlayerPos(), mPosition);
    MR::normalize(&toPlayer);
    moveAndTurn(hStopParam, toPlayer);
    if (MR::isActionEnd(this)) {
        setNerve(&NrvTogezo::TogezoNrvWait::sInstance);
    }
}

void Togezo::exeDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Down");
        MR::startActionSound(this, "Down", -1, -1, -1);
    }
    TVec3f negVel = -mVelocity;
    MR::turnDirectionDegree(this, &mFrontVec, negVel, 30.0f);
    if (MR::isGreaterStep(this, 30)) {
        MR::tryHitReactionDead(this);
        fn_8022BA10(this);
        kill();
    }
}

void Togezo::exeDeath() {
    if (MR::isFirstStep(this)) {
        MR::tryHitReactionDead(this);
        fn_8022BA10(this);
        kill();
    }
}

void Togezo::exeSpinReaction() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "SpinReaction");
    }
    TVec3f toPlayer;
    toPlayer.sub(*MR::getPlayerPos(), mPosition);
    MR::normalize(&toPlayer);
    moveAndTurn(hStopParam, toPlayer);
    if (MR::isActionEnd(this)) {
        setNerve(&NrvTogezo::TogezoNrvWalk::sInstance);
    }
}

void Togezo::exeBindStarPointer() {
    if (MR::updateActorState(this)) {
        if (MR::isExistRail(this)) {
            setNerve(&NrvTogezo::TogezoNrvWalkOnRail::sInstance);
            return;
        }
        mHomePos.set(mPosition);
        setNerve(&NrvTogezo::TogezoNrvWait::sInstance);
    }
}

void Togezo::exeShellStart() {
    if (MR::isFirstStep(this)) {
        mPipo->makeActorAppeared();
        MR::startAction(mPipo, "ShellStart");
    }
    if (MR::isStep(this, 1)) {
        MR::startAction(this, "ShellStart");
    }
    if (MR::isOnGround(this)) {
        setNerve(&NrvTogezo::TogezoNrvShellStartLand::sInstance);
    }
}

void Togezo::exeShellStartLand() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mPipo, "ShellStartLand");
        MR::startAction(this, "ShellStartLand");
    }
    if (MR::isGreaterEqualStep(this, 24)) {
        TVec3f toPlayer;
        toPlayer.sub(*MR::getPlayerPos(), mPosition);
        MR::normalize(&toPlayer);
        moveAndTurn(hStopParam, toPlayer);
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvTogezo::TogezoNrvShellWait::sInstance);
    }
}

void Togezo::exeShellWait() {
    MR::isFirstStep(this);
    if (MR::isStep(this, 90)) {
        MR::startAction(mPipo, "AppearSign");
    }
    TVec3f toPlayer;
    toPlayer.sub(*MR::getPlayerPos(), mPosition);
    MR::normalize(&toPlayer);
    moveAndTurn(hStopParam, toPlayer);
    if (MR::isGreaterEqualStep(this, 180)) {
        setNerve(&NrvTogezo::TogezoNrvAppear::sInstance);
    }
}

void Togezo::exeShot() {
    if (MR::isFirstStep(this)) {
        MR::invalidateExCollisionParts(this);
        MR::invalidateClipping(this);
        MR::tryRumblePadStrong(this, 0);
        MR::deleteEffectAll(this);
        calcYoshiDirection();
        calcThrowVelocity();
        startRoll();
        calcThrowDirection();
        ModelObj* pPipo = mPipo;
        pPipo->mScale.x = 1.0f;
        pPipo->mScale.y = 1.0f;
        pPipo->mScale.z = 1.0f;
        mHasAttacked = false;
        MR::startAction(mPipo, "Shot");
    }
    MR::startActionSound(mPipo, "Shot", -1, -1, 8);
    if (!MR::isBckPlaying(mPipo, "ShotRoll")) {
        if (MR::isActionEnd(mPipo)) {
            MR::startAction(mPipo, "ShotRoll");
        }
    }
    if (MR::isBinded(this) && MR::isGreaterEqualStep(this, 0)) {
        MR::sendMsgToBindedSensor(8, getSensor("attack"));
        mHasAttacked = true;
    }
    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        MR::tryHitReactionDead(this);
        kill();
        return;
    }
    if (isShotEnd()) {
        MR::emitEffect(mPipo, "Explosion");
        MR::startActionSound(this, "Break", -1, -1, -1);
        MR::validateClipping(this);
        mPipo->kill();
        kill();
    }
}

void Togezo::initActorInfo(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Togezo", false);
}

void Togezo::calcAndSetBaseMtx() {
    MR::updateBaseScale(this, mScaleController);
    if (mHoldMtx == nullptr || !isNerve(&NrvTogezo::TogezoNrvHold::sInstance)) {
        TPos3f mtx;
        MR::calcMtxFromGravityAndZAxis(&mtx, this, mGravity, mFrontVec);
        MR::setBaseTRMtx(this, mtx);
        return;
    }
    MR::extractMtxTrans(mHoldMtx, &mPosition);
    MR::setBaseTRMtx(this, mHoldMtx);
}

void Togezo::control() {
    mScaleController->updateNerve();
    if (!isNerve(&NrvTogezo::TogezoNrvShot::sInstance)) {
        MR::setScale(mPipo, mScale);
        if (!isNerve(&NrvTogezo::TogezoNrvBound::sInstance)) {
            if (isNerve(&NrvTogezo::TogezoNrvDown::sInstance) || isNerve(&NrvTogezo::TogezoNrvShellStart::sInstance)) {
                if (!MR::isOnGround(this)) {
                    MR::addVelocityToGravity(this, 1.5f);
                }
                MR::attenuateVelocity(this, MR::isOnGround(this) ? 0.93f : 0.99f);
            } else {
                if (!MR::isOnGround(this)) {
                    MR::addVelocityToGravity(this, 1.0f);
                }
                MR::restrictVelocity(this, 20.0f);
            }
        }
        if (isAttackable()) {
            if (mIsYoshi) {
                YoshiFunction::tryYoshiEat(mYoshiLockOn);
            } else {
                YoshiFunction::tryYoshiProjectile(mYoshiLockOn);
            }
        }
        if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f))) {
            MR::tryHitReactionDead(this);
            kill();
            return;
        }
        if (isWalking() && mStateBindStarPointer->tryStartPointBind()) {
            setNerve(&NrvTogezo::TogezoNrvBindStarPointer::sInstance);
        }
    }
}

void Togezo::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvTogezo::TogezoNrvShot::sInstance) && !mHasAttacked &&
        (MR::isSensorEnemy(pReceiver) || MR::isSensorMapObj(pReceiver) || MR::isSensorNpc(pReceiver))) {
        if (MR::sendArbitraryMsg(8, pReceiver, pSender)) {
            mHasAttacked = true;
        }
    } else {
        bool yoshi = false;
        if (isNerve(&NrvTogezo::TogezoNrvTongue::sInstance) || isNerve(&NrvTogezo::TogezoNrvTongueHold::sInstance) ||
            isNerve(&NrvTogezo::TogezoNrvEaten::sInstance) || isNerve(&NrvTogezo::TogezoNrvShot::sInstance)) {
            yoshi = true;
        }
        if (!yoshi) {
            if (MR::isSensorPlayerOrRide(pReceiver) && MR::isSensorEnemyAttack(pSender)) {
                if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                    NRV_GROUP_GROUNDED(grounded);
                    if (!grounded) {
                        NRV_GROUP_SHELL(shell);
                        if (!shell) {
                            setNerve(&NrvTogezo::TogezoNrvAttack::sInstance);
                            return;
                        }
                    }
                } else {
                    MR::sendMsgPush(pReceiver, pSender);
                }
            }
            if (!MR::isSensorPlayerOrRide(pReceiver) && MR::isSensorEnemy(pSender)) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    }
}

bool Togezo::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pSender) || MR::isSensorMapObj(pSender)) {
        if (isNerve(&NrvTogezo::TogezoNrvHold::sInstance)) {
            return false;
        }
        if (isNerve(&NrvTogezo::TogezoNrvBound::sInstance)) {
            return false;
        }
        TVec3f push;
        LiveActor* pHost = MR::getSensorHost(pSender);
        push.sub(mPosition, pHost->mPosition);
        MR::normalizeOrZero(&push);
        TVec3f killed;
        f32 len = MR::vecKillElement(push, mGravity, &killed);
        MR::vecScaleAdd(&killed, &push, 0.2f * len);
        MR::normalizeOrZero(&killed);
        JMAVECScaleAdd(&killed, &mVelocity, &mVelocity, 0.6f);
        return true;
    }
    return false;
}

bool Togezo::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvTogezo::TogezoNrvHold::sInstance)) {
        return false;
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }
    if (MR::isMsgSupportTicoSpin(msg)) {
        NRV_GROUP_GROUNDED(grounded);
        if (grounded) {
            return false;
        }
        NRV_GROUP_SHELL(shell);
        if (shell) {
            return false;
        }
        if (isAttackable()) {
            MR::stopSceneForDefaultHit(4);
            TVec3f vel = mPosition - *MR::getPlayerPos();
            MR::setVelocitySeparateHV(this, vel, 0.0f, 30.0f);
            setNerve(&NrvTogezo::TogezoNrvShellStart::sInstance);
            return true;
        }
    }
    if (MR::isMsgPlayerHitProjectiles(msg)) {
        if (isAttackable()) {
            MR::deleteEffectAll(this);
            fn_80012B90(this, 36.0f, 30.0f, pSender, pReceiver, 4);
            setNerve(&NrvTogezo::TogezoNrvDown::sInstance);
            MR::tryHitReactionBetweenSensors(this, "\x90\x81\x82\xab\x94\xf2\x82\xce\x82\xb3\x82\xea", pSender, pReceiver);
            return true;
        }
    }
    return false;
}

bool Togezo::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isAttackable()) {
        return false;
    }
    if (MR::isMsgExplosionAttack(msg)) {
        MR::deleteEffectAll(this);
        fn_80012B90(this, 36.0f, 30.0f, pSender, pReceiver, 4);
        setNerve(&NrvTogezo::TogezoNrvDown::sInstance);
        MR::tryHitReactionBetweenSensors(this, "\x90\x81\x82\xab\x94\xf2\x82\xce\x82\xb3\x82\xea", pSender, pReceiver);
        return true;
    }
    if (MR::isMsgEnemyAttackElectric(msg)) {
        MR::deleteEffectAll(this);
        setNerve(&NrvTogezo::TogezoNrvDeath::sInstance);
        MR::tryHitReactionBetweenSensors(this, "\x93\x64\x8c\x82", pSender, pReceiver);
        return true;
    }
    if (MR::isMsgToEnemyAttackBlow(msg)) {
        MR::deleteEffectAll(this);
        fn_80012B90(this, 36.0f, 30.0f, pSender, pReceiver, 4);
        setNerve(&NrvTogezo::TogezoNrvDown::sInstance);
        MR::tryHitReactionBetweenSensors(this, "\x90\x81\x82\xab\x94\xf2\x82\xce\x82\xb3\x82\xea", pSender, pReceiver);
        return true;
    }
    if (MR::isMsgToEnemyAttackTrampleDeath(msg)) {
        MR::stopSceneForDefaultHit(8);
        MR::tryHitReactionDead(this);
        kill();
        return true;
    }
    return false;
}

bool Togezo::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isDead(this)) {
        return false;
    }
    if (isNerve(&NrvTogezo::TogezoNrvHold::sInstance)) {
        return false;
    }
    if (MR::isMsgInhaleBlackHole(msg)) {
        MR::tryHitReactionDead(this);
        kill();
        return true;
    }
    if (MR::isSensorEnemyAttack(pReceiver)) {
        return false;
    }
    if (receiveYoshiMsg(msg, pSender, pReceiver)) {
        return true;
    }
    NRV_GROUP_GROUNDED(grounded);
    if (grounded) {
        return false;
    }
    NRV_GROUP_SHELL(shell);
    if (shell) {
        return false;
    }
    if (MR::isMsgSpinStormRange(msg)) {
        if (!isAttackable()) {
            return false;
        }
        if (isNerve(&NrvTogezo::TogezoNrvSpinReaction::sInstance)) {
            return false;
        }
        if (isNerve(&NrvTogezo::TogezoNrvBindStarPointer::sInstance)) {
            return false;
        }
        setNerve(&NrvTogezo::TogezoNrvSpinReaction::sInstance);
        return true;
    }
    return false;
}

bool Togezo::receiveYoshiMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mIsYoshi) {
        if (MR::isMsgYoshiStartTongue(msg)) {
            return tryStartTongue();
        }
        if (MR::isMsgYoshiLoopTongue(msg)) {
            return true;
        }
        if (MR::isMsgYoshiReleaseEaten(msg)) {
            kill();
            return true;
        }
        if (MR::isMsgYoshiCancelEaten(msg)) {
            setNerve(&NrvTogezo::TogezoNrvCancelEaten::sInstance);
            return true;
        }
    } else {
        if (MR::isMsgYoshiStartTongue(msg)) {
            return tryStartTongue();
        }
        if (MR::isMsgYoshiLoopTongue(msg)) {
            return true;
        }
        if (MR::isMsgYoshiStartEaten(msg)) {
            setNerve(&NrvTogezo::TogezoNrvEaten::sInstance);
            MR::hideModel(this);
            mPipo->kill();
            return true;
        }
        if (MR::isMsgYoshiReleaseEaten(msg)) {
            setNerve(&NrvTogezo::TogezoNrvShot::sInstance);
            return true;
        }
        if (MR::isMsgYoshiCancelEaten(msg)) {
            if (!isNerve(&NrvTogezo::TogezoNrvShot::sInstance)) {
                setNerve(&NrvTogezo::TogezoNrvCancelEaten::sInstance);
            }
            return true;
        }
    }
    return false;
}

void Togezo::moveAndTurn(const MR::ActorMoveParam& rParam, const TVec3f& rDir) {
    if (MR::isBindedGround(this)) {
        mGroundCounter = 0;
    } else {
        mGroundCounter++;
    }
    if (mGroundCounter < 4) {
        MR::moveAndTurnToDirection(this, &mFrontVec, rDir, rParam._0, rParam._4, rParam._8, rParam._C);
        return;
    }
    MR::moveAndTurnToDirection(this, &mFrontVec, rDir, hRollParam._0, hRollParam._4, hRollParam._8, hRollParam._C);
}

void Togezo::calcYoshiDirection() {
    TPos3f mtx;
    fn_80422350(&mtx);
    f32 tz = mtx.mMtx[2][3];
    f32 ty = mtx.mMtx[1][3];
    f32 tx = mtx.mMtx[0][3];
    mPosition.set< f32 >(tx, ty, tz);
    mtx.getEuler(mRotation);
    mRotation.x *= 57.29578f;
    mRotation.y *= 57.29578f;
    mRotation.z *= 57.29578f;
    mtx.getZDir(mFrontVec);
    TVec3f offs(mFrontVec);
    offs.scale(20.0f);
    mPosition -= offs;
    calcAndSetBaseMtx();
}

void Togezo::startRoll() {
    MR::invalidateClipping(this);
    MR::hideModelAndOnCalcAnim(this);
    mPipo->appear();
    MR::startAction(mPipo, "Roll");
}

void Togezo::calcThrowVelocity() {
    MR::getPlayerThrowVec(&mVelocity);
    mVelocity.x *= 40.0f;
    mVelocity.y *= 40.0f;
    mVelocity.z *= 40.0f;
}

void Togezo::calcThrowDirection() {
    TRot3f rot;
    TVec3f throwVec;
    TVec3f side;
    TVec3f up;
    MR::getPlayerThrowVec(&throwVec);
    TVec3f negGravity = -*MR::getPlayerGravity();
    PSVECCrossProduct(&negGravity, &throwVec, &side);
    if (!MR::normalizeOrZero(&side)) {
        PSVECCrossProduct(&throwVec, &side, &up);
        rot.identity();
        rot.setXDir(side);
        rot.setYDir(up);
        rot.setZDir(throwVec);
        rot.getEuler(mRotation);
        mRotation.x *= 57.29578f;
        mRotation.y *= 57.29578f;
        mRotation.z *= 57.29578f;
        rot.getZDir(mFrontVec);
    }
}

bool Togezo::tryTurnBackOrRail() {
    if (MR::isFallNextMove(mPosition, mFrontVec, mGravity, 200.0f, 100.0f, 60.0f, nullptr)) {
        setNerve(&NrvTogezo::TogezoNrvTurnBack::sInstance);
        return true;
    }
    if (MR::isNearPlayer(this, 1500.0f) && MR::isLessStep(this, 300)) {
        return false;
    }
    if (MR::isExistRail(this)) {
        setNerve(&NrvTogezo::TogezoNrvWalkOnRail::sInstance);
    } else {
        mHomePos.set(mPosition);
        setNerve(&NrvTogezo::TogezoNrvWait::sInstance);
    }
    return true;
}

// NONMATCHING: order of the two independent addi (r3/r4) before the TVec3 copy-ctor calls
bool Togezo::tryAttack() {
    TVec3f dir;
    const TVec3f* pPlayerPos = MR::getPlayerPos();
    TVec3f diff(*getPosition());
    diff -= *pPlayerPos;
    if (MR::isPlayerElementModeYoshi()) {
        TVec3f up(*getGravity());
        up.scale(50.0f);
        diff.add(up);
    }
    f32 vert = MR::vecKillElement(diff, mGravity, &dir);
    if (__fabsf(vert) > 100.0f) {
        return false;
    }
    bool far = !MR::isNearPlayer(this, 1000.0f);
    if (far) {
        return false;
    }
    dir.sub(*MR::getPlayerPos(), mPosition);
    MR::normalize(&dir);
    if (MR::isFallNextMove(mPosition, dir, mGravity, 200.0f, 100.0f, 60.0f, nullptr)) {
        return false;
    }
    setNerve(&NrvTogezo::TogezoNrvTurn::sInstance);
    return true;
}

bool Togezo::tryStartTongue() {
    if (isNerve(&NrvTogezo::TogezoNrvHold::sInstance)) {
        return false;
    }
    if (isNerve(&NrvTogezo::TogezoNrvBound::sInstance)) {
        MR::invalidateClipping(this);
        setNerve(&NrvTogezo::TogezoNrvTongue::sInstance);
        MR::stopBck(this);
        mVelocity.zero();
        return true;
    }
    if (isNerve(&NrvTogezo::TogezoNrvShellStart::sInstance) || isNerve(&NrvTogezo::TogezoNrvShellStartLand::sInstance) ||
        isNerve(&NrvTogezo::TogezoNrvShellWait::sInstance)) {
        MR::invalidateClipping(this);
        setNerve(&NrvTogezo::TogezoNrvTongue::sInstance);
        MR::stopBck(this);
        mVelocity.zero();
        return true;
    }
    MR::stopSceneForDefaultHit(8);
    TVec3f vel = mPosition - *MR::getPlayerPos();
    MR::setVelocitySeparateHV(this, vel, 0.0f, 30.0f);
    setNerve(&NrvTogezo::TogezoNrvShellStart::sInstance);
    return false;
}

bool Togezo::isWalking() const {
    bool result = false;
    if (isNerve(&NrvTogezo::TogezoNrvTurnBack::sInstance) || isNerve(&NrvTogezo::TogezoNrvWait::sInstance) ||
        isNerve(&NrvTogezo::TogezoNrvWalk::sInstance) || isNerve(&NrvTogezo::TogezoNrvTurn::sInstance) ||
        isNerve(&NrvTogezo::TogezoNrvSearch::sInstance) || isNerve(&NrvTogezo::TogezoNrvRun::sInstance) ||
        isNerve(&NrvTogezo::TogezoNrvAttack::sInstance) || isNerve(&NrvTogezo::TogezoNrvAttackEnd::sInstance)) {
        result = true;
    }
    return result;
}

bool Togezo::isAttackable() const {
    bool result = false;
    if (!isNerve(&NrvTogezo::TogezoNrvHold::sInstance) && !isNerve(&NrvTogezo::TogezoNrvDown::sInstance) &&
        !isNerve(&NrvTogezo::TogezoNrvTongue::sInstance) && !isNerve(&NrvTogezo::TogezoNrvTongueHold::sInstance) &&
        !isNerve(&NrvTogezo::TogezoNrvEaten::sInstance) && !isNerve(&NrvTogezo::TogezoNrvShot::sInstance) &&
        !isNerve(&NrvTogezo::TogezoNrvCancelEaten::sInstance)) {
        result = true;
    }
    return result;
}

bool Togezo::isShotEnd() const {
    MR::isGreaterEqualStep(this, 300);
    bool result = false;
    if (mHasAttacked == 1 || MR::isGreaterEqualStep(this, 300)) {
        result = true;
    }
    return result;
}

Togezo::~Togezo() {
}

#pragma dont_inline on
TogezoMgr::TogezoMgr(int num, const TVec3f& rPos) : DeriveActorGroup< Togezo >("\x83\x67\x83\x51\x83\x5D\x81\x5B\x8A\xC7\x97\x9D", num) {
    for (int i = 0; i < num; i++) {
        Togezo* pTogezo = new Togezo("\x83\x67\x83\x51\x83\x5D\x81\x5B");
        pTogezo->mPosition.x = rPos.x;
        pTogezo->mPosition.y = rPos.y;
        pTogezo->mPosition.z = rPos.z;
        pTogezo->initWithoutIter();
        pTogezo->makeActorDead();
        registerActor(pTogezo);
    }
}

#pragma dont_inline reset
void TogezoMgr::updateAll() {
    int num = getObjNum();
    for (int i = 0; i < num; i++) {
        Togezo* pTogezo = getMember(i);
        if (!MR::isDead(pTogezo)) {
            pTogezo->updateTimer();
        }
    }
}

s32 TogezoMgr::getActiveNum() const {
    s32 count = 0;
    int num = getObjNum();
    for (int i = 0; i < num; i++) {
        Togezo* pTogezo = getMember(i);
        if (!MR::isDead(pTogezo) && !pTogezo->isYoshiState()) {
            count++;
        }
    }
    return count;
}

TogezoMgr::~TogezoMgr() {
}

// fn_80214610: direction helper for rail-following Togezo (called from outside this unit).
extern "C" void fn_80214610(TVec3f* pDst, const LiveActor* pActor) {
    TVec3f camPos = MR::getCamPos();
    TVec3f toCam(0.0f);
    TVec3f railDir(0.0f);
    TVec3f up(0.0f, 1.0f, 0.0f);
    fn_8000E960(&toCam, pActor, camPos);
    MR::calcRailDirection(&railDir, pActor);
    if (!MR::isRailGoingToEnd(pActor)) {
        TVec3f neg = -railDir;
        railDir.set(neg);
    }
    MR::calcUpVec(&up, pActor);
    pDst->set(railDir);
    MR::rotateVecDegree(pDst, up, 45.0f);
    if (toCam.dot(*pDst) < 0.0f) {
        pDst->set(railDir);
        TVec3f negUp = -up;
        MR::rotateVecDegree(pDst, negUp, 45.0f);
    }
}
