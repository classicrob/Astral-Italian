#define JGEOMETRY_VEC3_MULASSIGN_NOINLINE
#include "Game/MapObj/AirBubble.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

// PLACEHOLDER(objects): forces the leading .sdata2 pool order [1.0f, 0.0f, MR::pi(), 2.0f] that the target has but plain first-use order does not give
// (the target pools these ahead of the TU's own literals: probably the .sdata2 of header-inline bodies the original TU compiled whose text the linker deduped;
// function order already matches the target); resolve when those inline bodies are identified
void AirBubble_FORCE_MATCH_SDATA2() {
    (void)1.0f;
    (void)0.0f;
    (void)MR::pi();
    (void)2.0f;
}

namespace MR {
    // PLACEHOLDER(objects): not declared in any Util header yet (attachSupportTicoToTarget__2MRFPC9LiveActor)
    void attachSupportTicoToTarget(const LiveActor*);
};  // namespace MR

namespace {
    const f32 cCorioriRotSpeed = 1.5f;
    const f32 cMoveStartAccel = 7.0f;
    const f32 cSwingRange = 250.0f;
    const s32 cDefaultLife = 360;
};  // namespace

namespace NrvAirBubble {
    NEW_NERVE(AirBubbleNrvWait, AirBubble, Wait);
    NEW_NERVE(AirBubbleNrvMove, AirBubble, Move);
    NEW_NERVE(AirBubbleNrvVanish, AirBubble, Vanish);
    NEW_NERVE(AirBubbleNrvGather, AirBubble, Gather);
    NEW_NERVE(AirBubbleNrvBreak, AirBubble, Break);
    NEW_NERVE(AirBubbleNrvKillWait, AirBubble, KillWait);
};  // namespace NrvAirBubble

AirBubble::AirBubble(const char* pName) : LiveActor(pName), mAnchorPos(0.0f, 0.0f, 0.0f), _9C(0.0f, 0.0f, 1.0f), _A8(0.0f), mLife(::cDefaultLife) {
}

void AirBubble::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "AirBubble", false);
    initNerve(GET_NERVE(AirBubble, AirBubbleNrvWait));
    makeActorAppeared();
    MR::setClippingFar100m(this);
    MR::startAction(this, "Move");
}

void AirBubble::initAfterPlacement() {
    mAnchorPos = mPosition;
}

void AirBubble::control() {
}

void AirBubble::appearMove(const TVec3f& rTrans, s32 life) {
    mAnchorPos.set(rTrans);
    mPosition.set(rTrans);
    appear();
    MR::showModel(this);
    MR::validateHitSensors(this);
    setNerve(GET_NERVE(AirBubble, AirBubbleNrvMove));
    MR::invalidateClipping(this);

    mLife = life > 0 ? life : ::cDefaultLife;
}

void AirBubble::exeWait() {
    _A8 = MR::sin(MR::calcNerveRate(this, 120) * MR::pi() * 2.0f);
    mPosition = mAnchorPos + TVec3f(0.0f, 1.0f, 0.0f) * _A8 * 1.0f;
    MR::attachSupportTicoToTarget(this);
}

void AirBubble::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::onCalcGravity(this);

        mVelocity.set((-mGravity).multInLine2(::cMoveStartAccel));
    }

    MR::rotateVecDegree(&_9C, mGravity, ::cCorioriRotSpeed);
    _9C.killElement(_9C, mGravity);

    if (MR::isNearZero(_9C)) {
        MR::getRandomVector(&_9C, 1.0f);
    }

    MR::normalizeOrZero(&_9C);
    mVelocity += _9C * 0.1f;
    mVelocity -= mGravity * 0.3f;
    mVelocity.mult(0.85f);

    if (MR::isGreaterStep(this, mLife)) {
        MR::hideModel(this);
        MR::startActionSound(this, "OjAirBubbleBreak");
        MR::emitEffect(this, "RecoveryBubbleBreak");
        MR::offCalcGravity(this);
        setNerve(GET_NERVE(AirBubble, AirBubbleNrvKillWait));
    } else {
        MR::attachSupportTicoToTarget(this);
    }
}

void AirBubble::exeVanish() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
    }

    if (!MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        MR::hideModel(this);
        MR::startActionSound(this, "OjAirBubbleBreak");
        MR::emitEffect(this, "RecoveryBubbleBreak");
        kill();
    }

    MR::zeroVelocity(this);
}

void AirBubble::exeGather() {
    MR::vecBlend(&mPosition, mPosition, *MR::getPlayerPos(), MR::calcNerveRate(this, 15));

    if (MR::isGreaterStep(this, 15)) {
        setNerve(GET_NERVE(AirBubble, AirBubbleNrvBreak));
    }
}

void AirBubble::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OjAirBubbleBreak");
        MR::emitEffect(this, "RecoveryBubbleBreak");
        MR::incPlayerOxygen(8);
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        kill();
    }
}

void AirBubble::exeKillWait() {
    if (MR::isGreaterStep(this, 90)) {
        kill();
    }
}

bool AirBubble::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (!canSpinGet()) {
        return false;
    }

    if (MR::isSensorPlayer(pSender)) {
        if (isNerve(GET_NERVE(AirBubble, AirBubbleNrvKillWait))) {
            MR::incPlayerOxygen(8);
            kill();

            return true;
        }

        setNerve(GET_NERVE(AirBubble, AirBubbleNrvBreak));

        return true;
    }

    return false;
}

bool AirBubble::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgItemShow(msg)) {
        return MR::receiveItemShowMsg(msg, pSender, pReceiver);
    }

    if (MR::isMsgItemHide(msg)) {
        return MR::receiveItemHideMsg(msg, pSender, pReceiver);
    }

    if (MR::isMsgItemStartMove(msg)) {
        return startVanish();
    }

    if (MR::isMsgItemSupportTicoDrop(msg)) {
        return tryStartGatherFromVanish();
    }

    if (MR::isMsgItemEndMoveOrDrop(msg)) {
        mAnchorPos = mPosition;
        return tryStartGather();
    }

    if (MR::isMsgSpinStormRange(msg) && canSpinGet()) {
        if ((*MR::getSensorPos(pSender) - mPosition).length() < ::cSwingRange) {
            setNerve(GET_NERVE(AirBubble, AirBubbleNrvBreak));
            return true;
        }
    }

    return false;
}

bool AirBubble::startVanish() {
    if (canSpinGet()) {
        setNerve(GET_NERVE(AirBubble, AirBubbleNrvVanish));
        return true;
    }

    return false;
}

bool AirBubble::tryStartGatherFromVanish() {
    if (isNerve(GET_NERVE(AirBubble, AirBubbleNrvVanish))) {
        setNerve(GET_NERVE(AirBubble, AirBubbleNrvGather));
        return true;
    }

    return false;
}

bool AirBubble::tryStartGather() {
    if (canSpinGet()) {
        setNerve(GET_NERVE(AirBubble, AirBubbleNrvGather));
        return true;
    }

    return false;
}

bool AirBubble::canSpinGet() const {
    if (isNerve(GET_NERVE(AirBubble, AirBubbleNrvWait)) || isNerve(GET_NERVE(AirBubble, AirBubbleNrvMove))) {
        return true;
    }

    return false;
}

// PLACEHOLDER(objects): forces .sdata2/.data pool order that the target gets from an unidentified earlier consumer in the TU;
// resolve when the real cause (header inline / earlier function) is found
