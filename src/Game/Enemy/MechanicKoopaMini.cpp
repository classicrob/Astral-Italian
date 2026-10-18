#include "Game/Enemy/MechanicKoopaMini.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/MapObj/KeySwitch.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/types.h>

namespace NrvMechanicKoopaMini {
    NEW_NERVE(HostTypeNrvWait, MechanicKoopaMini, Wait);
    NEW_NERVE(HostTypeNrvWalkOnRail, MechanicKoopaMini, WalkOnRail);
    NEW_NERVE(HostTypeNrvTurn, MechanicKoopaMini, Turn);
    NEW_NERVE(HostTypeNrvJumpStart, MechanicKoopaMini, JumpStart);
    NEW_NERVE(HostTypeNrvJumpEnd, MechanicKoopaMini, JumpEnd);
    NEW_NERVE(HostTypeNrvFind, MechanicKoopaMini, Find);
    NEW_NERVE(HostTypeNrvPursue, MechanicKoopaMini, Pursue);
    NEW_NERVE(HostTypeNrvPreFireAttack, MechanicKoopaMini, PreFireAttack);
    NEW_NERVE_ONEND(HostTypeNrvFireAttack, MechanicKoopaMini, FireAttack, FireAttack);
    NEW_NERVE(HostTypeNrvFireAttackEnd, MechanicKoopaMini, FireAttackEnd);
    NEW_NERVE(HostTypeNrvAttackHit, MechanicKoopaMini, AttackHit);
    NEW_NERVE(HostTypeNrvSpinHit, MechanicKoopaMini, SpinHit);
    NEW_NERVE(HostTypeNrvTrample, MechanicKoopaMini, Trample);
    NEW_NERVE(HostTypeNrvHipDropped, MechanicKoopaMini, HipDropped);
    NEW_NERVE(HostTypeWaterBreak, MechanicKoopaMini, WaterBreak);
    NEW_NERVE(HostTypeNrvBindStarPointer, MechanicKoopaMini, BindStarPointer);
    NEW_NERVE(HostTypeNrvPreFireAttackAfterBind, MechanicKoopaMini, PreFireAttackAfterBind);
};  // namespace NrvMechanicKoopaMini

namespace {
    static const MR::ActorMoveParam hNoMoveNoTurnParam = {0.0f, 2.0f, 0.8f, 0.0f};
    static const MR::ActorMoveParam hOnAirParam = {0.0f, 2.0f, 0.98f, 0.0f};
    static const MR::ActorMoveParam hOnAirTurnParam = {0.0f, 2.0f, 0.98f, 2.0f};
    static const MR::ActorMoveParam hOnAirMoveParam = {0.2f, 2.0f, 0.98f, 2.0f};
    static const MR::ActorMoveParam hWalkParam = {0.4f, 2.0f, 0.8f, 2.0f};
    static const MR::ActorMoveParam hPursueParam = {1.3f, 2.0f, 0.8f, 2.0f};
    static const MR::ActorMoveParam hPreFireAttackParam = {0.0f, 2.0f, 0.8f, 0.0f};
    static const MR::ActorMoveParam hFireAttackParam = {0.0f, 2.0f, 0.8f, 1.0f};
    static const MR::ActorMoveParam hHitReactionOnGroundParam = {0.0f, 1.2f, 0.8f, 5.0f};
    static const MR::ActorMoveParam hHitReactionAirParam = {0.0f, 2.0f, 0.95f, 5.0f};
    static const MR::ActorMoveParam hJumpOnGroundParam = {0.0f, 1.2f, 0.8f, 0.0f};
    static const MR::ActorMoveParam hJumpAirParam = {0.5f, 2.0f, 0.95f, 0.0f};
    static const MR::ActorMoveParam hTurnParam = {0.0f, 2.0f, 0.8f, 2.0f};
    static const TVec3f hBinderOffset(0.0f, 110.0f, 20.0f);
    static const f32 hTrampleUpRangeSub = 80;
    static const f32 hPushUpRangeSub = 130;
};  // namespace

MechanicKoopaMini::MechanicKoopaMini(const char* pName) : LiveActor(pName) {
    mIsAppearJumping = false;
    mIsForbidPause = false;
    _8E = false;
    mKeySwitch = nullptr;
    mScaleController = nullptr;
    mBindStarPointer = nullptr;
}

void MechanicKoopaMini::onAppearSwitchOn() {
    if (mIsAppearJumping) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvJumpStart));
    } else {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvWait));
    }
}

// NONMATCHING: MWCC elides repeated member-function-pointer copies, reducing stack size.
void MechanicKoopaMini::init(const JMapInfoIter& rIter) {
    initNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvWalkOnRail), 1);
    MR::initActor(this, rIter, "MechanicKoopaMini", false);
    if (MR::isValidSwitchAppear(this)) {
        MR::listenStageSwitchOnAppear(this, MR::Functor_Inline(this, &MechanicKoopaMini::onAppearSwitchOn));
    }
    MR::getJMapInfoArg0NoInit(rIter, &mIsAppearJumping);
    MR::getJMapInfoArg1NoInit(rIter, &mIsForbidPause);
    MR::setBinderOffsetVec(this, &hBinderOffset, true);
    bool useUp = false;
    MR::getJMapInfoArg2NoInit(rIter, &useUp);
    if (useUp) {
        TVec3f up(0.0f, 1.0f, 0.0f);
        MR::calcUpVec(&up, this);
        mGravity = -up;
    } else {
        MR::onCalcGravity(this);
    }
    s32 count = MR::getChildObjNum(rIter);
    for (s32 i = 0; i < count; i++) {
        mKeySwitch = static_cast<KeySwitch*>(NameObjFactory::initChildObj(rIter, i));
        mKeySwitch->prepareDemoStart();
        MR::disableAllItemGenerator(this);
    }
    mScaleController = new AnimScaleController(nullptr);
    mScaleController->setParamTight();
    MR::tryRegisterDemoCast(this, rIter);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mBindStarPointer, GET_NERVE(MechanicKoopaMini, HostTypeNrvBindStarPointer), "SupportFreeze");
    MR::useStageSwitchSyncAppear(this, rIter);
}

void MechanicKoopaMini::initAfterPlacement() {
    MR::trySetMoveLimitCollision(this);
}

void MechanicKoopaMini::makeActorAppeared() {
    LiveActor::makeActorAppeared();
}

void MechanicKoopaMini::kill() {
    LiveActor::kill();
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }

    MR::emitEffect(this, "Death");
    MR::startActionSound(this, "EmExplodeS");

    if (mKeySwitch != nullptr) {
        mKeySwitch->appearKeySwitch(mPosition);
    }
}

void MechanicKoopaMini::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam._0, ::hNoMoveNoTurnParam._4, ::hNoMoveNoTurnParam._8, ::hNoMoveNoTurnParam._C);

    if (!fn_801C34F0() && MR::isGreaterStep(this, 120)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvWalkOnRail));
    }
}

void MechanicKoopaMini::exeWalkOnRail() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
    }

    bool canTurn = false;
    if (MR::isOnGround(this)) {
        MR::moveAndTurnAlongRail(this, 600.0f, ::hWalkParam._0, ::hWalkParam._4, ::hWalkParam._8, ::hWalkParam._C, &canTurn);
        TVec3f frontVec;
        MR::calcFrontVec(&frontVec, this);
        if (fn_801C3590()) return;
        if (fn_801C34F0()) return;
    } else {
        MR::moveAndTurnAlongRail(this, 600.0f, ::hOnAirMoveParam._0, ::hOnAirMoveParam._4, ::hOnAirMoveParam._8, ::hOnAirMoveParam._C, &canTurn);
    }

    if (canTurn) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvTurn));
    } else if (!mIsForbidPause && MR::isGreaterStep(this, 120)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvWait));
    }
}

void MechanicKoopaMini::exeTurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Turn");
    }

    TVec3f railDir(MR::getRailDirection(this));
    if (MR::isOnGround(this)) {
        MR::moveAndTurnToDirection(this, railDir, ::hTurnParam._0, ::hTurnParam._4, ::hTurnParam._8, ::hTurnParam._C);
    } else {
        MR::moveAndTurnToDirection(this, railDir, ::hOnAirParam._0, ::hOnAirParam._4, ::hOnAirParam._8, ::hOnAirParam._C);
    }

    TVec3f vec(mPosition);
    vec.add(railDir);
    if (MR::isFaceToTargetDegree(this, vec, 5.0f)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvWalkOnRail));
    }
}

void MechanicKoopaMini::exeJumpStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "JumpStart");
    }

    if (MR::isOnGround(this)) {
        MR::moveAndTurnToPlayer(this, ::hJumpOnGroundParam._0, ::hJumpOnGroundParam._4, ::hJumpOnGroundParam._8, ::hJumpOnGroundParam._C);
    } else {
        MR::moveAndTurnToPlayer(this, ::hJumpAirParam._0, ::hJumpAirParam._4, ::hJumpAirParam._8, ::hJumpAirParam._C);
    }

    if (MR::isStep(this, (s32)MR::getBckFrameMax(this) - 3)) {
        MR::addVelocityJump(this, 45.0f);
        MR::startActionSound(this, "EmMkoopaminiJump");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvJumpEnd));
    }
}

void MechanicKoopaMini::exeJumpEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "JumpEnd");
    }

    if (MR::isOnGround(this)) {
        MR::moveAndTurnToPlayer(this, ::hJumpOnGroundParam._0, ::hJumpOnGroundParam._4, ::hJumpOnGroundParam._8, ::hJumpOnGroundParam._C);
    } else {
        MR::moveAndTurnToPlayer(this, ::hJumpAirParam._0, ::hJumpAirParam._4, ::hJumpAirParam._8, ::hJumpAirParam._C);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvWalkOnRail));
    }
}

void MechanicKoopaMini::exeFind() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Find");
        MR::addVelocityJump(this, 26.0f);
        MR::startActionSound(this, "EmMkoopaminiJump");
    }

    if (MR::isOnGround(this)) {
        MR::moveAndTurnToPlayer(this, ::hTurnParam._0, ::hTurnParam._4, ::hTurnParam._8, ::hTurnParam._C);
    } else {
        MR::moveAndTurnToPlayer(this, ::hOnAirTurnParam._0, ::hOnAirTurnParam._4, ::hOnAirTurnParam._8, ::hOnAirTurnParam._C);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvPursue));
    }
}

void MechanicKoopaMini::exePursue() {
    TVec3f frontVec;
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Pursue");
        MR::invalidateClipping(this);
        mVelocity.set(mGravity * -10.0f);
    }

    if (MR::isOnGround(this)) {
        MR::moveAndTurnToPlayer(this, ::hPursueParam._0, ::hPursueParam._4, ::hPursueParam._8, ::hPursueParam._C);
        if (fn_801C3590()) return;
    } else {
        MR::moveAndTurnToPlayer(this, hOnAirMoveParam._0, hOnAirMoveParam._4, hOnAirMoveParam._8, hOnAirMoveParam._C);
    }

    f32 distance = MR::calcDistanceToPlayer(this);
    if (!(1100.0f < distance)) {
        bool enabled = false;
        if (!MR::isValidSwitchA(this) || MR::isOnSwitchA(this)) enabled = true;
        if (enabled) goto canAttack;
    }
    setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvWalkOnRail));
    return;
canAttack:
    if (distance < 300.0f) {
        MR::calcFrontVec(&frontVec, this);
        if (MR::isFaceToPlayerHorizontalDegree(this, frontVec, 5.729578f) && MR::isOnGround(this)) {
            setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvPreFireAttack));
        }
    }
}

void MechanicKoopaMini::exePreFireAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "PreFire");
        MR::invalidateClipping(this);
        mVelocity.set(mGravity * -10.0f);
    }

    if (MR::isLessEqualStep(this, 60)) {
        MR::startActionSound(this, "EmLvMkoopaminiPreFire");
    }

    if (MR::isOnGround(this)) {
        MR::moveAndTurnToPlayer(this, ::hPreFireAttackParam._0, ::hPreFireAttackParam._4, ::hPreFireAttackParam._8, ::hPreFireAttackParam._C);
    } else {
        MR::moveAndTurnToPlayer(this, ::hOnAirParam._0, ::hOnAirParam._4, ::hOnAirParam._8, ::hOnAirParam._C);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvFireAttack));
    }
}

void MechanicKoopaMini::exePreFireAttackAfterBind() {
    if (MR::getBckFrame(this) >= 60.0f) {
        MR::startActionSound(this, "EmLvMkoopaminiPreFire", -1, -1, -1);
    }

    if (MR::isOnGround(this)) {
        MR::moveAndTurnToPlayer(this, ::hPreFireAttackParam._0, ::hPreFireAttackParam._4, ::hPreFireAttackParam._8, ::hPreFireAttackParam._C);
    } else {
        MR::moveAndTurnToPlayer(this, ::hOnAirParam._0, ::hOnAirParam._4, ::hOnAirParam._8, ::hOnAirParam._C);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvFireAttack));
    }

}

void MechanicKoopaMini::exeFireAttack() {
    if (MR::isFirstStep(this)) {
            MR::validateHitSensor(this, "fire");
        MR::startAction(this, "Fire");
    }

    if (MR::isOnGround(this)) {
        MR::moveAndTurnToPlayer(this, ::hFireAttackParam._0, ::hFireAttackParam._4, ::hFireAttackParam._8, ::hFireAttackParam._C);
    } else {
        MR::moveAndTurnToPlayer(this, ::hOnAirParam._0, ::hOnAirParam._4, ::hOnAirParam._8, ::hOnAirParam._C);
    }

    if (MR::isGreaterStep(this, 120)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvFireAttackEnd));
        return;
    }
    if (MR::isLessEqualStep(this, 90)) {
        MR::startActionSound(this, "EmLvMkoopaminiFire");
    }
}

void MechanicKoopaMini::endFireAttack() {
    MR::invalidateHitSensor(this, "fire");
}

void MechanicKoopaMini::exeFireAttackEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FireEnd");
    }

    if (MR::isOnGround(this)) {
        MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam._0, ::hNoMoveNoTurnParam._4, ::hNoMoveNoTurnParam._8, ::hNoMoveNoTurnParam._C);
    } else {
        MR::moveAndTurnToPlayer(this, ::hOnAirParam._0, ::hOnAirParam._4, ::hOnAirParam._8, ::hOnAirParam._C);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvWalkOnRail));
    }
}

void MechanicKoopaMini::exeAttackHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackHit");
    }

    if (MR::isOnGround(this)) {
        MR::moveAndTurnToPlayer(this, ::hHitReactionOnGroundParam._0, ::hHitReactionOnGroundParam._4, ::hHitReactionOnGroundParam._8,
                                ::hHitReactionOnGroundParam._C);
    } else {
        MR::moveAndTurnToPlayer(this, ::hHitReactionAirParam._0, ::hHitReactionAirParam._4, ::hHitReactionAirParam._8, ::hHitReactionAirParam._C);
    }

    if (MR::isOnGround(this) && MR::isActionEnd(this)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvWait));
    }
}

void MechanicKoopaMini::exeSpinHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SpinHit");
        MR::startBlowHitSound(this);
        MR::startActionSound(this, "EmMkoopaminiSpinHit");
    }

    if (MR::isGreaterEqualStep(this, 60)) {
        MR::startActionSound(this, "EmLvMkoopaminiPreFire");
    }

    if (MR::isOnGround(this)) {
        MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam._0, ::hNoMoveNoTurnParam._4, ::hNoMoveNoTurnParam._8, ::hNoMoveNoTurnParam._C);
    } else {
        MR::moveAndTurnToPlayer(this, ::hOnAirParam._0, ::hOnAirParam._4, ::hOnAirParam._8, ::hOnAirParam._C);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvFireAttack));
    }
}

void MechanicKoopaMini::exeTrample() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Trample");
    }

    if (MR::isGreaterEqualStep(this, 60)) {
        MR::startActionSound(this, "EmLvMkoopaminiPreFire");
    }

    if (MR::isOnGround(this)) {
        MR::moveAndTurnToPlayer(this, ::hNoMoveNoTurnParam._0, ::hNoMoveNoTurnParam._4, ::hNoMoveNoTurnParam._8, ::hNoMoveNoTurnParam._C);
    } else {
        MR::moveAndTurnToPlayer(this, ::hOnAirParam._0, ::hOnAirParam._4, ::hOnAirParam._8, ::hOnAirParam._C);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvFireAttack));
    }
}

void MechanicKoopaMini::exeHipDropped() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Break");
        MR::invalidateHitSensors(this);
        MR::startActionSound(this, "EmMkoopaminiBreak");
    }

    if (MR::isActionEnd(this)) {
        kill();
        MR::emitItemStomp(this);
    }
}

void MechanicKoopaMini::exeBindStarPointer() {
    if (MR::updateActorState(this)) {
        if (_8E) {
            setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvPreFireAttackAfterBind));
        } else {
            setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvWalkOnRail));
        }
    }
}



void MechanicKoopaMini::control() {
    mScaleController->updateNerve();

    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f)) || MR::isInWater(mPosition)) {
        kill();
        return;
    }

    if ((isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvPreFireAttack)) ||
        isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvWalkOnRail)) ||
        isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvPursue)) ||
        isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvTrample)) ||
        isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvFireAttackEnd))) && mBindStarPointer->tryStartPointBind()) {
        _8E = false;
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvBindStarPointer));
        return;
    }

    if (isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvFireAttack)) && mBindStarPointer->tryStartPointBind()) {
        _8E = false;
        MR::startAction(this, "FireEnd");
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvBindStarPointer));
        return;
    }

    if (((isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvSpinHit)) && MR::isGreaterStep(this, 60)) ||
        isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvPreFireAttackAfterBind))) && mBindStarPointer->tryStartPointBind()) {
        _8E = true;
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvBindStarPointer));
    }
}

void MechanicKoopaMini::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvHipDropped))) {
        if (getSensor("fire") == pSender) {
            if (MR::isSensorPlayer(pReceiver)) {
                MR::sendMsgEnemyAttackFire(pReceiver, pSender);
            }
        } else if (MR::isSensorPlayer(pReceiver)) {
            if (!MR::isPlayerHipDropFalling()) {
                TVec3f pos(*MR::getSensorPos(pReceiver));
                pos -= *MR::getSensorPos(pSender);
                f32 dot = pos.dot(-mGravity);
                f32 SenderRad = MR::getSensorRadius(pSender);
                f32 ReceiverRad = MR::getSensorRadius(pReceiver);
                if (!(((ReceiverRad + SenderRad) - ::hPushUpRangeSub) < dot)) {
                    MR::sendMsgPush(pReceiver, pSender);
                }
            }
        } else if (MR::isSensorEnemy(pReceiver) || MR::isSensorYoshi(pReceiver)) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool MechanicKoopaMini::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (getSensor("fire") == pReceiver) {
        return false;
    }

    if (isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvHipDropped))) {
        return false;
    }

    if (isNerve(GET_NERVE(MechanicKoopaMini, HostTypeWaterBreak))) return false;

    if (MR::isMsgStarPieceAttack(msg)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvSpinHit));
        return true;
    }

    if (MR::isMsgDrillAttack(msg)) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeWaterBreak));
        return true;
    }

    if (MR::isMsgPlayerTrample(msg)) {
        TVec3f v16(*MR::getSensorPos(pSender));
        v16 -= *MR::getSensorPos(pReceiver);
        f32 dot = v16.dot(-mGravity);
        f32 ReceiverRad = MR::getSensorRadius(pReceiver);
        f32 SenderRad = MR::getSensorRadius(pSender);
        if (((SenderRad + ReceiverRad) - ::hTrampleUpRangeSub) < dot) {
            return false;
        } else {
            MR::startActionSound(this, "EmMkoopaminiTrample");
            if (isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvSpinHit)) ||
                (isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvBindStarPointer)) && _8E) ||
                isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvPreFireAttackAfterBind))) {
                return true;
            }

            if (!isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvTrample))) {
                setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvTrample));
            }
            return true;
        }
    }

    if (MR::isMsgPlayerHipDrop(msg) && !isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvHipDropped))) {
        setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvHipDropped));
        return true;
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) || MR::isMsgJetTurtleAttack(msg)) {
        if ((!isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvSpinHit)) || !MR::isLessStep(this, 120)) &&
            (!isNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvPreFireAttackAfterBind)) || !(MR::getBckFrame(this) < 120.0f))) {
            setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvSpinHit));
            return true;
        }
        return false;
    }
    return false;
}

bool MechanicKoopaMini::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg)) {
        kill();
        MR::emitItemHit(this);
        return true;
    }

    return false;
}

bool MechanicKoopaMini::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgInhaleBlackHole(msg)) {
        kill();
        return true;
    }

    return false;
}

bool MechanicKoopaMini::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorPlayer(pSender)) {
        MR::addVelocityFromPush(this, 3.0f, pSender, pReceiver);
        return true;
    }

    return false;
}

// NONMATCHING: MWCC folds an empty clamp arm into a single conditional branch.
void MechanicKoopaMini::updateHitSensor(HitSensor* sensor) {
    TVec3f up, front;
    MR::calcUpVec(&up, this);
    MR::calcFrontVec(&front, this);
    TVec3f toPlayer(*MR::getPlayerCenterPos());
    toPlayer -= mPosition;
    f32 frontDist = front.dot(toPlayer);
    f32 upDist = up.dot(toPlayer);
    f32 offset = 0.0f;
    if (upDist - MR::getSensorRadius(this, "Body") < 0.0f) offset = -10.0f;
    f32 rate = MR::calcNerveRate(this, 5, 15);
    f32 maximum = 350.0f * rate;
    f32 distance = 0.0f;
    if (frontDist < distance) {}
    else if (frontDist > maximum) distance = maximum;
    else distance = frontDist;
    MR::getSensorPos2(sensor)->set(mPosition);
    MR::getSensorPos2(sensor)->add(up * (offset + MR::getSensorRadius(this, "Body")));
    MR::getSensorPos2(sensor)->add(front * distance);
}

void MechanicKoopaMini::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    MR::updateBaseScale(this, mScaleController);
}

void MechanicKoopaMini::exeWaterBreak() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Break");
        MR::invalidateHitSensors(this);
        MR::startActionSound(this, "EmMkoopaminiBreak");
    }
    if (MR::isActionEnd(this)) {
        kill();
        MR::emitItemStomp(this);
    }
}
bool MechanicKoopaMini::fn_801C34F0() {
    if (MR::calcDistanceToPlayer(this) < 900.0f && MR::isFaceToPlayerDegree(this, 120.0f)) {
        bool enabled = false;
        if (!MR::isValidSwitchA(this) || MR::isOnSwitchA(this)) enabled = true;
        if (enabled) {
            setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvFind));
            return true;
        }
    }
    return false;
}
bool MechanicKoopaMini::fn_801C3590() {
    TVec3f front;
    MR::calcFrontVec(&front, this);
    if (MR::isBindedWall(this)) {
        TVec3f normal(MR::getWallNormal(this));
        if (front.dot(normal) < -0.99f) {
            setNerve(GET_NERVE(MechanicKoopaMini, HostTypeNrvJumpStart));
            return true;
        }
    }
    return false;
}
