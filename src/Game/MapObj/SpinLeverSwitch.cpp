#include "Game/MapObj/SpinLeverSwitch.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/Util.hpp"
#include "Game/Util/YoshiUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

namespace {
    static const s32 sStepToOnSwitch = 15;
    static const s32 sStepOnSE = 8;
    static const s32 sStepForHitStop = 8;
};  // namespace

namespace NrvSpinLeverSwitch {
    NEW_NERVE(SpinLeverSwitchNrvWait, SpinLeverSwitch, Wait);
    NEW_NERVE(SpinLeverSwitchNrvSwitchOn, SpinLeverSwitch, SwitchOn);
    NEW_NERVE(SpinLeverSwitchNrvEnd, SpinLeverSwitch, End);
};  // namespace NrvSpinLeverSwitch

SpinLeverSwitch::SpinLeverSwitch(const char* pName) : LiveActor(pName), mConnector(), mYoshiTarget(nullptr), mForceAnim(false) {
    mConnector = new MapObjConnector(this);
}

void SpinLeverSwitch::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "SpinLeverSwitch", false);
    mYoshiTarget = MR::createYoshiLockOnTarget(this, MR::getJointMtx(this, "Spin"), TVec3f(0.0f));
    bool isWait = MR::isValidSwitchA(this);
    const char* objectName = nullptr;
    MR::getObjectName(&objectName, rIter);
    mForceAnim = MR::isEqualString(objectName, "SpinLeverSwitchForceAnim");

    if (isWait) {
        initNerve(GET_NERVE(SpinLeverSwitch, SpinLeverSwitchNrvWait));
    } else {
        MR::startAction(this, "On");
        MR::setBckFrameAndStop(this, MR::getBckFrameMax(this));
        MR::startBrk(this, "On");
        MR::setBrkFrameAndStop(this, MR::getBrkFrameMax(this));
        initNerve(GET_NERVE(SpinLeverSwitch, SpinLeverSwitchNrvEnd));
    }

    makeActorAppeared();
}

void SpinLeverSwitch::initAfterPlacement() {
    mConnector->attachToUnder();
}

void SpinLeverSwitch::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    mConnector->connect();
}

void SpinLeverSwitch::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startBrk(this, "On");
        MR::setBrkFrameAndStop(this, 0.0f);
    }
    if (!MR::isValidSwitchB(this) || MR::isOnSwitchB(this) || mForceAnim) {
        MR::attachSupportTicoToTarget(this);
        YoshiFunction::tryYoshiEat(mYoshiTarget);
    }
    if (mForceAnim && MR::isValidSwitchB(this) && MR::isOnSwitchB(this)) {
        MR::startAction(this, "On");
        MR::setBckFrameAndStop(this, MR::getBckFrameMax(this));
        MR::startBrk(this, "On");
        MR::setBrkFrameAndStop(this, MR::getBrkFrameMax(this));
        setNerve(GET_NERVE(SpinLeverSwitch, SpinLeverSwitchNrvEnd));
    }
}

void SpinLeverSwitch::exeSwitchOn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "On");
        MR::startBrk(this, "On");

        if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
            MR::startActionSound(this, "OjSpinLeverSwHitW");
        } else {
            MR::startActionSound(this, "OjSpinLeverSwHit");
        }

        MR::tryRumblePadStrong(this, WPAD_CHAN0);
        MR::shakeCameraNormal();
    }

    if (MR::isStep(this, ::sStepOnSE)) {
        if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
            MR::startActionSound(this, "OjSpinLeverSwOnW");
        } else {
            MR::startActionSound(this, "OjSpinLeverSwOn");
        }
    }

    if (MR::isStep(this, ::sStepToOnSwitch)) {
        MR::onSwitchA(this);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(SpinLeverSwitch, SpinLeverSwitchNrvEnd));
    }
}

void SpinLeverSwitch::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver) && pSender == getSensor("Body")) {
        if (!MR::sendMsgPush(pReceiver, pSender)) return;
    }
    if (MR::isSensorYoshi(pReceiver)) MR::sendMsgPush(pReceiver, pSender);
}

bool SpinLeverSwitch::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(SpinLeverSwitch, SpinLeverSwitchNrvWait))) {
        return false;
    }

    if (pReceiver != getSensor("spin")) {
        return false;
    }

    if (MR::isValidSwitchB(this) && !MR::isOnSwitchB(this) && !mForceAnim) return false;

    if (MR::isMsgPlayerHitAll(msg)) {
        setNerve(GET_NERVE(SpinLeverSwitch, SpinLeverSwitchNrvSwitchOn));
        MR::tryHitReaction(this, "スイッチオン");
        return true;
    }

    return false;
}

bool SpinLeverSwitch::receiveMsgEnemyAttack(u32 msg, HitSensor* sender, HitSensor* receiver) {
    if (!isNerve(GET_NERVE(SpinLeverSwitch, SpinLeverSwitchNrvWait))) return false;
    if (receiver != getSensor("spin")) return false;
    if (MR::isValidSwitchB(this) && !MR::isOnSwitchB(this) && !mForceAnim) return false;
    if (MR::isSensorType(sender, 45) && MR::isMsgExplosionAttack(msg)) {
        setNerve(GET_NERVE(SpinLeverSwitch, SpinLeverSwitchNrvSwitchOn));
        MR::tryHitReaction(this, "スイッチオン");
        return true;
    }
    return false;
}

bool SpinLeverSwitch::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (isNerve(GET_NERVE(SpinLeverSwitch, SpinLeverSwitchNrvWait))) {
            MR::tryHitReaction(this, "スイッチオン（ヨッシー舌）");
            setNerve(GET_NERVE(SpinLeverSwitch, SpinLeverSwitchNrvSwitchOn));
            return false;
        }
        return false;
    }
    if (MR::isMsgYoshiLoopTongue(msg)) return false;
    if (MR::isMsgYoshiReleaseOrCancelEaten(msg)) return false;
    return false;
}

void SpinLeverSwitch::exeEnd() {
}
