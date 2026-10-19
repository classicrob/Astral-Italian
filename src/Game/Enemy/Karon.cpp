#pragma auto_inline off
#include "Game/Enemy/Karon.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/Util/YoshiLockOnUtil.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/TerritoryMover.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/types.h>

namespace {
    static const MR::ActorMoveParam hNoMoveNoTurnParam = {0.0f, 1.2f, 0.8f, 0.0f};
    static const MR::ActorMoveParam hNoMoveNoTurnOnAirParam = {0.0f, 2.0f, 0.95, 0.0f};
    static const MR::ActorMoveParam hWalkParam = {0.12f, 1.2f, 0.95f, 2.0f};
    static const MR::ActorMoveParam hRailTurnParam = {0.15f, 1.2f, 0.9f, 3.0f};
    static const MR::ActorMoveParam hSearchParam = {0.0f, 1.2f, 0.95f, 2.0f};
    static const MR::ActorMoveParam hPursueParam = {0.25f, 1.2f, 0.95f, 2.0f};
    static const MR::ActorMoveParam hHitReactionOnGroundParam = {0.0f, 1.2f, 0.8f, 5.0f};
    static const MR::ActorMoveParam hHitReactionAirParam = {0.0f, 2.0f, 0.95f, 5.0f};
    static const MR::ActorMoveParam hRailWalkParam = {0.15f, 1.2f, 0.95f, 2.0f};
    static const MR::ActorMoveParam hRailPursueParam = {0.3f, 1.2f, 0.95f, 2.0f};
};  // namespace

#define KARON_NERVE(Name, Method) \
    class Name : public Nerve { public: virtual void execute(Spine* spine) const { static_cast<Karon*>(spine->mExecutor)->Method(); } static Name sInstance; }; \
    Name Name::sInstance ATTRIBUTE_WEAK;
#define KARON_NERVE_END(Name, Method) \
    class Name : public Nerve { public: virtual void execute(Spine* spine) const { static_cast<Karon*>(spine->mExecutor)->Method(); } virtual void executeOnEnd(Spine* spine) const { MR::validateClipping(static_cast<Karon*>(spine->mExecutor)); } static Name sInstance; }; \
    Name Name::sInstance ATTRIBUTE_WEAK;

namespace NrvKaron {
    NEW_NERVE(HostTypeNrvFixWait, Karon, FixWait);
    NEW_NERVE(HostTypeNrvWait, Karon, Wait);
    NEW_NERVE(HostTypeNrvStarPieceHitWait, Karon, Wait);
    NEW_NERVE(HostTypeNrvWalk, Karon, Walk);
    NEW_NERVE(HostTypeNrvTurn, Karon, Turn);
    KARON_NERVE(HostTypeNrvRailTurn, exeTurnOnRail);
    NEW_NERVE(HostTypeNrvWalkOnRail, Karon, WalkOnRail);
    NEW_NERVE(HostTypeNrvSearch, Karon, Search);
    NEW_NERVE_ONEND(HostTypeNrvPursue, Karon, Pursue, Pursue);
    KARON_NERVE_END(HostTypeNrvRailPursue, exePursueOnRail);
    NEW_NERVE(HostTypeNrvBroken, Karon, Broken);
    NEW_NERVE(HostTypeNrvStepBroken, Karon, StepBroken);
    NEW_NERVE(HostTypeNrvRecover, Karon, Recover);
    NEW_NERVE(HostTypeNrvHitReaction, Karon, HitReaction);
    NEW_NERVE(HostTypeNrvStarPieceHit, Karon, HitReaction);
    NEW_NERVE(HostTypeNrvDeath, Karon, Death);
    NEW_NERVE(HostTypeNrvSinkDown, Karon, SinkDown);
    NEW_NERVE(HostTypeNrvLavaDown, Karon, LavaDown);
    class HostTypeNrvEaten : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static HostTypeNrvEaten sInstance;
    };
    HostTypeNrvEaten HostTypeNrvEaten::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE_ONEND(HostTypeNrvBindStarPointer, Karon, BindStarPointer, BindStarPointer);
};  // namespace NrvKaron

Karon::Karon(const char* pName) : LiveActor(pName) {
    mTerritoryMover = nullptr;
    mYoshiLockOn = nullptr;
    mStopSwAnim = false;
    mIsRailConstrained = false;
    mSearchAngle = 80.0f;
    mRailDirection.set(0.0f);
    mScaleController = nullptr;
    mStateStarPointer = nullptr;
}

void Karon::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Karon", false);

    s32 arg0 = -1;
    MR::getJMapInfoArg0NoInit(rIter, &arg0);
    if (arg0 == -1) {
        mStopSwAnim = false;
    } else {
        mStopSwAnim = true;
    }

    MR::getJMapInfoArg1NoInit(rIter, &mIsRailConstrained);
    if (mIsRailConstrained) {
        mSearchAngle = 20.0f;
    }
    mTerritoryMover = new TerritoryMover(500.0f);
    mTerritoryMover->mCenter.set(mPosition);

    mScaleController = new AnimScaleController(nullptr);
    mScaleController->setParamTight();
    mStateStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    initNerve(GET_NERVE(Karon, HostTypeNrvWait), 1);
    MR::initActorState(this, mStateStarPointer, GET_NERVE(Karon, HostTypeNrvBindStarPointer), "SupportFreeze");
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, "Center", TVec3f(0.0f));

    if (MR::isValidSwitchA(this)) {
        initNerve(GET_NERVE(Karon, HostTypeNrvFixWait), 1);
    }

    MR::useStageSwitchSyncAppear(this, rIter);
}

void Karon::initAfterPlacement() {
    MR::trySetMoveLimitCollision(this);
}

void Karon::kill() {
    LiveActor::kill();
    MR::emitEffect(this, "Death");
    MR::startActionSound(this, "EmExplodeS", -1, -1, -1);
}

void Karon::exeFixWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        if (!mStopSwAnim) {
            MR::stopBck(this);
        }
    }

    mBinder->_1EC._5 = true;
    MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam._0, ::hNoMoveNoTurnParam._4, ::hNoMoveNoTurnParam._8, ::hNoMoveNoTurnParam._C);

    if (MR::isOnSwitchA(this)) {
        setNerve(GET_NERVE(Karon, HostTypeNrvWait));
    }
}

void Karon::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        if (mIsRailConstrained) {
            TVec3f direction(MR::getRailDirection(this));
            TVec3f front(0.0f);
            MR::calcFrontVec(&front, this);
            if (direction.dot(front) < 0.0f) MR::reverseRailDirection(this);
        }
    }

    mBinder->_1EC._5 = true;

    if (MR::isOnGround(this)) {
        MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam._0, ::hNoMoveNoTurnParam._4, ::hNoMoveNoTurnParam._8, ::hNoMoveNoTurnParam._C);
    } else {
        MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnOnAirParam._0, ::hNoMoveNoTurnOnAirParam._4, ::hNoMoveNoTurnOnAirParam._8,
                                ::hNoMoveNoTurnOnAirParam._C);
    }

    s32 step;
    if (isNerve(GET_NERVE(Karon, HostTypeNrvStarPieceHitWait))) {
        step = 60;
    } else {
        step = 120;
    }

    f32 range = 700.0f;
    if (mIsRailConstrained) range = 1500.0f;
    if (MR::calcDistanceToPlayer(this) < range && MR::isFaceToPlayerDegree(this, mSearchAngle) && isRailPursueAllowed()) {
        setNerve(GET_NERVE(Karon, HostTypeNrvSearch));
    } else if (MR::isGreaterStep(this, step)) {
        if (mRailRider != nullptr) {
            setNerve(GET_NERVE(Karon, HostTypeNrvWalkOnRail));
        } else {
            mTerritoryMover->decideNextTargetPos(this);
            setNerve(GET_NERVE(Karon, HostTypeNrvWalk));
        }
    }
}

void Karon::exeWalk() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
    }

    MR::moveAndTurnToTarget(this, mTerritoryMover->mTarget, ::hWalkParam._0, ::hWalkParam._4, ::hWalkParam._8, ::hWalkParam._C);

    if (MR::isGreaterStep(this, 180) || mTerritoryMover->isReachedTarget(this, 40.0f)) {
        setNerve(GET_NERVE(Karon, HostTypeNrvWait));
    } else if (isFallNextMove(true)) {
        setNerve(GET_NERVE(Karon, HostTypeNrvTurn));
    } else if (MR::calcDistanceToPlayer(this) < 700.0f && MR::isFaceToPlayerDegree(this, mSearchAngle)) {
        setNerve(GET_NERVE(Karon, HostTypeNrvSearch));
    }
}

void Karon::exeTurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
        mTerritoryMover->decideNextTargetPos(this);
    }

    MR::moveAndTurnToTarget(this, mTerritoryMover->mTarget, ::hSearchParam._0, ::hSearchParam._4, ::hSearchParam._8, ::hSearchParam._C);

    if (MR::calcDistanceToPlayer(this) < 700.0f && MR::isFaceToPlayerDegree(this, mSearchAngle)) {
        setNerve(GET_NERVE(Karon, HostTypeNrvPursue));
    } else {
        TVec3f frontVec;
        MR::calcFrontVec(&frontVec, this);
        if (MR::isGreaterStep(this, 10) && MR::isFaceToTargetHorizontalDegree(this, mTerritoryMover->mTarget, frontVec, ::hSearchParam._C * 2.0f)) {
            if (isFallNextMove(false)) {
                setNerve(GET_NERVE(Karon, HostTypeNrvTurn));
            } else {
                setNerve(GET_NERVE(Karon, HostTypeNrvWalk));
            }
        }
    }
}

void Karon::exeTurnOnRail() {
    if (MR::isFirstStep(this)) mRailDirection = MR::getRailDirection(this);
    MR::moveAndTurnAlongRail(this, 600.0f, hRailTurnParam._0, hRailTurnParam._4, hRailTurnParam._8, hRailTurnParam._C, nullptr);
    if (isRailPursueAllowed()) {
        TVec3f front(0.0f);
        MR::calcFrontVec(&front, this);
        if (mRailDirection.dot(front) < -0.9f) setNerve(GET_NERVE(Karon, HostTypeNrvWalkOnRail));
    }
}

void Karon::exeWalkOnRail() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Walk");
    f32 range = 700.0f;
    if (mIsRailConstrained) {
        range = 1500.0f;
        if (MR::isRailReachedNearGoal(this, 50.0f)) {
            setNerve(GET_NERVE(Karon, HostTypeNrvRailTurn));
            return;
        }
        MR::moveAndTurnAlongRail(this, 600.0f, hRailWalkParam._0, hRailWalkParam._4, hRailWalkParam._8, hRailWalkParam._C, nullptr);
    } else {
        MR::moveAndTurnAlongRail(this, 600.0f, hWalkParam._0, hWalkParam._4, hWalkParam._8, hWalkParam._C, nullptr);
    }
    MR::calcDistanceToPlayer(this);
    if (MR::calcDistanceToPlayer(this) < range && MR::isFaceToPlayerDegree(this, mSearchAngle) && isRailPursueAllowed()) setNerve(GET_NERVE(Karon, HostTypeNrvSearch));
}

void Karon::exeSearch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Search");
        MR::startActionSound(this, "EvKaronFind");
    }

    if (mIsRailConstrained) {
        MR::moveAndTurnAlongRail(this, 600.0f, ::hSearchParam._0, ::hSearchParam._4, ::hSearchParam._8, ::hSearchParam._C, nullptr);
    } else {
        MR::moveAndTurnToPlayer(this, ::hSearchParam._0, ::hSearchParam._4, ::hSearchParam._8, ::hSearchParam._C);
    }

    if (MR::isActionEnd(this)) {
        if (mIsRailConstrained) { setNerve(GET_NERVE(Karon, HostTypeNrvRailPursue)); return; }
        setNerve(GET_NERVE(Karon, HostTypeNrvPursue));
    }
}

void Karon::exePursue() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Pursue");
        MR::invalidateClipping(this);
        mVelocity += mGravity * -10.0f;
    }

    MR::moveAndTurnToPlayer(this, ::hPursueParam._0, ::hPursueParam._4, ::hPursueParam._8, ::hPursueParam._C);

    if (1200.0f < MR::calcDistanceToPlayer(this) || !MR::isFaceToPlayerDegree(this, mSearchAngle)) {
        setNerve(GET_NERVE(Karon, HostTypeNrvWait));
    }
}

void Karon::exePursueOnRail() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Pursue");
        MR::invalidateClipping(this);
        mVelocity.add(mGravity * -10.0f);
    }
    MR::moveAndTurnAlongRail(this, 600.0f, hRailPursueParam._0, hRailPursueParam._4, hRailPursueParam._8, hRailPursueParam._C, nullptr);
    if (3200.0f < MR::calcDistanceToPlayer(this) || !MR::isFaceToPlayerDegree(this, mSearchAngle) || !isRailPursueAllowed()) setNerve(GET_NERVE(Karon, HostTypeNrvWait));
}

void Karon::exeBroken() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SpinBreak");
        MR::startActionSound(this, "EmKaronBroken");
        MR::startActionSound(this, "EvKaronDamage");
        MR::offCalcGravity(this);
        MR::forceBindOnGround(this, 0.0f, 1.0f);
        mScaleController->resetScale();
    }

    if (mIsRailConstrained) {
        MR::offBind(this);
        MR::zeroVelocity(this);
    } else {
    TVec3f velVec(0, 0, 0);
    if (MR::calcVelocityAreaOrRailMoveOnGround(&velVec, this)) {
        MR::addVelocityLimit(this, velVec * 1.0f);
        MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam._0, ::hNoMoveNoTurnParam._4, ::hNoMoveNoTurnParam._8, ::hNoMoveNoTurnParam._C);
        MR::calcGravity(this);
    } else if (MR::isBinded(this)) {
        const TVec3f* bindedVec = &MR::getBindedNormal(this);
        mVelocity.scale(bindedVec->dot(mVelocity), *bindedVec);
    } else {
        MR::forceBindOnGround(this, 1.0f, 0.8f);
        TVec3f* gravity = &mGravity;
        mVelocity.scale(gravity->dot(mVelocity), *gravity);
    }

    mBinder->_1EC._5 = true;

    }

    if (MR::isGreaterStep(this, 300)) {
        if (mIsRailConstrained) MR::onBind(this);
        setNerve(GET_NERVE(Karon, HostTypeNrvRecover));
    }
}

void Karon::exeStepBroken() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StepBreak");
        MR::startActionSound(this, "EmKaronBroken");
        MR::startActionSound(this, "EvKaronDamage");
        MR::offCalcGravity(this);
        MR::forceBindOnGround(this, 0.0f, 1.0f);
    }

    if (mIsRailConstrained) {
        MR::offBind(this);
        MR::zeroVelocity(this);
    } else {
    TVec3f velVec(0, 0, 0);
    if (MR::calcVelocityAreaOrRailMoveOnGround(&velVec, this)) {
        MR::addVelocityLimit(this, velVec * 1.0f);
        MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam._0, ::hNoMoveNoTurnParam._4, ::hNoMoveNoTurnParam._8, ::hNoMoveNoTurnParam._C);
        MR::calcGravity(this);
    } else if (MR::isBinded(this)) {
        const TVec3f* bindedVec = &MR::getBindedNormal(this);
        mVelocity.scale(bindedVec->dot(mVelocity), *bindedVec);
    } else {
        TVec3f* gravity = &mGravity;
        mVelocity.scale(gravity->dot(mVelocity), *gravity);
        MR::forceBindOnGround(this, 1.0f, 0.8f);
    }

    mBinder->_1EC._5 = true;

    }

    if (MR::isGreaterStep(this, 300)) {
        if (mIsRailConstrained) MR::onBind(this);
        setNerve(GET_NERVE(Karon, HostTypeNrvRecover));
    }
}

void Karon::exeRecover() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Recovery");
        MR::startActionSound(this, "EmKaronRecover");
    }

    if (MR::isOnGround(this)) {
        TVec3f* gravity = &mGravity;
        mVelocity.scale(gravity->dot(mVelocity), *gravity);
    }

    MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam._0, ::hNoMoveNoTurnParam._4, ::hNoMoveNoTurnParam._8, ::hNoMoveNoTurnParam._C);
    mBinder->_1EC._5 = true;

    if (MR::isActionEnd(this)) {
        MR::onCalcGravity(this);
        setNerve(GET_NERVE(Karon, HostTypeNrvWait));
    }
}

void Karon::exeHitReaction() {
    if (MR::isFirstStep(this)) {
        if (isNerve(GET_NERVE(Karon, HostTypeNrvStarPieceHit))) {
            MR::startAction(this, "StarPiece");
        } else {
            MR::startAction(this, "AttackHit");
        }
    }

    MR::startActionSound(this, "EvLvKaronAtkSuccess");

    if (isNerve(GET_NERVE(Karon, HostTypeNrvStarPieceHit))) {
        if (MR::isOnGround(this)) {
            MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam._0, ::hNoMoveNoTurnParam._4, ::hNoMoveNoTurnParam._8, ::hNoMoveNoTurnParam._C);
        } else {
            MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnOnAirParam._0, ::hNoMoveNoTurnOnAirParam._4, ::hNoMoveNoTurnOnAirParam._8,
                                    ::hNoMoveNoTurnOnAirParam._C);
        }
    } else if (MR::isOnGround(this)) {
        MR::moveAndTurnToPlayer(this, ::hHitReactionOnGroundParam._0, ::hHitReactionOnGroundParam._4, ::hHitReactionOnGroundParam._8,
                                ::hHitReactionOnGroundParam._C);
    } else {
        MR::moveAndTurnToPlayer(this, ::hHitReactionAirParam._0, ::hHitReactionAirParam._4, ::hHitReactionAirParam._8, ::hHitReactionAirParam._C);
    }

    if (MR::isOnGround(this) && MR::isActionEnd(this)) {
        if (isNerve(GET_NERVE(Karon, HostTypeNrvStarPieceHit))) {
            setNerve(GET_NERVE(Karon, HostTypeNrvStarPieceHitWait));
        } else {
            setNerve(GET_NERVE(Karon, HostTypeNrvWait));
        }
    }
}

void Karon::exeDeath() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Down");
        MR::startActionSound(this, "EmKaronBroken");
        MR::startActionSound(this, "EvKaronDamage");
    }

    MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam._0, ::hNoMoveNoTurnParam._4, ::hNoMoveNoTurnParam._8, ::hNoMoveNoTurnParam._C);

    if (MR::isActionEnd(this)) {
        kill();
    }
}

void Karon::exeSinkDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SinkDown");
        MR::invalidateShadow(this, nullptr);
    }

    if (MR::isOnGround(this)) {
        TVec3f* gravity = &mGravity;
        mVelocity.scale(gravity->dot(mVelocity), *gravity);
    }

    MR::startActionSound(this, "EvLvKaronSink");
    MR::startActionSound(this, "EmLvKaronSink");
    MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam._0, ::hNoMoveNoTurnParam._4, ::hNoMoveNoTurnParam._8, ::hNoMoveNoTurnParam._C);

    if (MR::isActionEnd(this)) {
        kill();
    }
}

void Karon::exeLavaDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "LavaDown");
        MR::invalidateShadow(this, nullptr);
    }

    if (MR::isOnGround(this)) {
        TVec3f* gravity = &mGravity;
        mVelocity.scale(gravity->dot(mVelocity), *gravity);
    }

    MR::startActionSound(this, "EvLvKaronSink");
    MR::startActionSound(this, "EmLvKaronSink");
    MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam._0, ::hNoMoveNoTurnParam._4, ::hNoMoveNoTurnParam._8, ::hNoMoveNoTurnParam._C);

    if (MR::isActionEnd(this)) {
        kill();
    }
}

void Karon::exeBindStarPointer() {
    mBinder->_1EC._5 = true;
    const Nerve* nerve = GET_NERVE(Karon, HostTypeNrvWait);
    if (MR::updateActorStateAndNextNerve(this, nerve)) return;
}

void Karon::control() {
    mScaleController->updateNerve();
    if (isKaronYoshiEatEnabled()) YoshiFunction::tryYoshiEat(mYoshiLockOn);

    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f)) || MR::isInWater(mPosition) || MR::isInDarkMatter(mPosition)) {
        kill();
        return;
    }

    if (MR::isPressedRoofAndGround(this)) { kill(); return; }
    MR::fn_800145C0(this);

    if (!isNerve(GET_NERVE(Karon, HostTypeNrvStepBroken)) && !isNerve(GET_NERVE(Karon, HostTypeNrvBroken))) {
        TVec3f velVec(0, 0, 0);
        if (MR::calcVelocityAreaOrRailMoveOnGround(&velVec, this)) {
            MR::addVelocityLimit(this, velVec * 1.0f);
        }
    }

    if ((isNerve(GET_NERVE(Karon, HostTypeNrvWait)) || isNerve(GET_NERVE(Karon, HostTypeNrvStarPieceHitWait)) ||
         isNerve(GET_NERVE(Karon, HostTypeNrvWalk)) || isNerve(GET_NERVE(Karon, HostTypeNrvWalkOnRail)) ||
         isNerve(GET_NERVE(Karon, HostTypeNrvSearch)) || isNerve(GET_NERVE(Karon, HostTypeNrvPursue))) &&
        mStateStarPointer->tryStartPointBind()) {
        setNerve(GET_NERVE(Karon, HostTypeNrvBindStarPointer));
        return;
    }

    if (!isNerve(GET_NERVE(Karon, HostTypeNrvBroken)) && !isNerve(GET_NERVE(Karon, HostTypeNrvStepBroken)) &&
        !isNerve(GET_NERVE(Karon, HostTypeNrvRecover)) && !isNerve(GET_NERVE(Karon, HostTypeNrvDeath)) &&
        !isNerve(GET_NERVE(Karon, HostTypeNrvSinkDown)) && !isNerve(GET_NERVE(Karon, HostTypeNrvLavaDown)) && MR::isBindedGroundSinkDeath(this)) {
        setNerve(GET_NERVE(Karon, HostTypeNrvSinkDown));
        return;
    }
    if (!isNerve(GET_NERVE(Karon, HostTypeNrvBroken)) && !isNerve(GET_NERVE(Karon, HostTypeNrvStepBroken)) &&
        !isNerve(GET_NERVE(Karon, HostTypeNrvRecover)) && !isNerve(GET_NERVE(Karon, HostTypeNrvDeath)) &&
        !isNerve(GET_NERVE(Karon, HostTypeNrvSinkDown)) && !isNerve(GET_NERVE(Karon, HostTypeNrvLavaDown)) && MR::isBindedGroundDamageFire(this)) {
        setNerve(GET_NERVE(Karon, HostTypeNrvLavaDown));
    }
}

bool Karon::isFallNextMove(bool a1) {
    if (a1) {
        return MR::isFallOrDangerNextMove(this, 150.0f, 140.0f, 150.0f);
    }

    TVec3f frontVec;
    MR::calcFrontVec(&frontVec, this);
    return MR::isFallOrDangerNextMove(mPosition, frontVec, mGravity, 150.0f, 140.0f, 150.0f);
}

bool Karon::isRailPursueAllowed() const {
    if (!mIsRailConstrained) return true;
    if (!MR::isExistRail(this)) return true;
    if (MR::isLoopRail(this)) return true;
    return !MR::isRailReachedNearGoal(this, 50.0f);
}

bool Karon::isKaronYoshiEatEnabled() const {
    if (isNerve(GET_NERVE(Karon, HostTypeNrvBroken)) || isNerve(GET_NERVE(Karon, HostTypeNrvStepBroken)) ||
        isNerve(GET_NERVE(Karon, HostTypeNrvRecover)) || isNerve(GET_NERVE(Karon, HostTypeNrvDeath)) ||
        isNerve(GET_NERVE(Karon, HostTypeNrvSinkDown)) || isNerve(GET_NERVE(Karon, HostTypeNrvLavaDown)) ||
        isNerve(GET_NERVE(Karon, HostTypeNrvEaten))) return false;
    return true;
}

void Karon::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(Karon, HostTypeNrvBroken)) || isNerve(GET_NERVE(Karon, HostTypeNrvStepBroken)) ||
        isNerve(GET_NERVE(Karon, HostTypeNrvRecover)) || isNerve(GET_NERVE(Karon, HostTypeNrvDeath))) {
        return;
    }

    if (getSensor("Body") == pSender) {
        if (MR::isSensorPlayer(pReceiver)) {
            if (MR::isPlayerExistUp(this, MR::getSensorRadius(this, "Body"), 0.25f) && MR::isOnGroundPlayer()) {
                if (MR::sendMsgTouchJump(pReceiver, pSender)) {
                    setNerve(GET_NERVE(Karon, HostTypeNrvStepBroken));
                }
            } else if (!isNerve(GET_NERVE(Karon, HostTypeNrvHitReaction)) && !isNerve(GET_NERVE(Karon, HostTypeNrvFixWait)) && !isNerve(GET_NERVE(Karon, HostTypeNrvBindStarPointer)) &&
                       !MR::isPlayerExistUp(this, MR::getSensorRadius(this, "Body"), 0.25f) && MR::isOnGroundPlayer() && !MR::isPlayerHipDropFalling() &&
                       !MR::isPlayerHipDropLand() && MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                TVec3f v18(*MR::getSensorPos(pSender));
                v18 -= *MR::getSensorPos(pReceiver);
                MR::vecKillElement(v18, mGravity, &v18);
                MR::normalizeOrZero(&v18);
                mVelocity.add(v18 * 10.0f);
                mVelocity.add(mGravity * -20.0f);
                setNerve(GET_NERVE(Karon, HostTypeNrvHitReaction));
            } else {
                MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
            }
        } else if (MR::isSensorEnemy(pReceiver) || MR::isSensorYoshi(pReceiver)) {
            MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
        }
    }
}

bool Karon::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(Karon, HostTypeNrvDeath)) || isNerve(GET_NERVE(Karon, HostTypeNrvSinkDown)) || isNerve(GET_NERVE(Karon, HostTypeNrvLavaDown))) return false;
    if (isNerve(GET_NERVE(Karon, HostTypeNrvBroken)) || isNerve(GET_NERVE(Karon, HostTypeNrvStepBroken))) {
        if (MR::isMsgInvincibleAttack(msg)) {
            setNerve(GET_NERVE(Karon, HostTypeNrvDeath));
            MR::emitItemHit(this);
            MR::startBlowHitSound(this);
            return true;
        }
        return false;
    }

    if (MR::isMsgPlayerTrample(msg)) {
        if (MR::isPlayerElementModeInvincible()) {
            MR::emitItemHit(this);
            setNerve(GET_NERVE(Karon, HostTypeNrvDeath));
        } else {
            setNerve(GET_NERVE(Karon, HostTypeNrvStepBroken));
        }
        return true;
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        setNerve(GET_NERVE(Karon, HostTypeNrvStepBroken));
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        setNerve(GET_NERVE(Karon, HostTypeNrvStarPieceHit));
        return true;
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) || MR::isMsgJetTurtleAttack(msg)) {
        MR::startBlowHitSound(this);
        setNerve(GET_NERVE(Karon, HostTypeNrvBroken));
        return true;
    }

    if (MR::isMsgFireBallAttack(msg)) {
        mScaleController->startHitReaction();
        setNerve(GET_NERVE(Karon, HostTypeNrvStarPieceHit));
        return true;
    }

    if (MR::isMsgInvincibleAttack(msg) || MR::isMsgYoshiProjectileOrEat(msg) || MR::isMsgDrillAttack(msg)) {
        setNerve(GET_NERVE(Karon, HostTypeNrvDeath));
        MR::emitItemHit(this);
        MR::startBlowHitSound(this);
        return true;
    }

    return false;
}

bool Karon::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg)) {
        kill();
        MR::emitItemHit(this);
        return true;
    }

    return false;
}

bool Karon::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pSender)) {
        return false;
    }

    if (isNerve(GET_NERVE(Karon, HostTypeNrvBroken)) || isNerve(GET_NERVE(Karon, HostTypeNrvStepBroken)) ||
        isNerve(GET_NERVE(Karon, HostTypeNrvRecover)) || isNerve(GET_NERVE(Karon, HostTypeNrvDeath))) {
        return false;
    }

    MR::addVelocityFromPush(this, 2.0f, pSender, pReceiver);
    return true;
}

bool Karon::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgInhaleBlackHole(msg)) {
        kill();
        return true;
    }

    if (MR::isMsgStartPowerStarGet(msg)) {
        kill();
        return true;
    }

    if (MR::isMsgYoshiStartTongue(msg) && isKaronYoshiEatEnabled()) {
        MR::stopBck(this);
        MR::invalidateHitSensors(this);
        setNerve(GET_NERVE(Karon, HostTypeNrvEaten));
        return true;
    }
    if (MR::isMsgYoshiLoopTongue(msg)) return true;
    if (MR::isMsgYoshiReleaseEaten(msg)) { makeActorDead(); return true; }
    if (MR::isMsgYoshiCancelEaten(msg)) { makeActorDead(); return true; }
    return false;
}

void Karon::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    MR::updateBaseScale(this, mScaleController);
}

Karon::~Karon() {
}

void Karon::endPursue() {
    MR::validateClipping(this);
}

void Karon::endBindStarPointer() {
    mStateStarPointer->kill();
}
