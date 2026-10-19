#include "Game/MapObj/PicketSwitch.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvPicketSwitch {
    NEW_NERVE(PicketSwitchNrvWait, PicketSwitch, Wait);
    NEW_NERVE(PicketSwitchNrvFirstDrop, PicketSwitch, FirstDrop);
    NEW_NERVE(PicketSwitchNrvLastDrop, PicketSwitch, LastDrop);
};  // namespace NrvPicketSwitch

PicketSwitch::PicketSwitch(const char* pName) : LiveActor(pName) {
}

PicketSwitch::~PicketSwitch() {
}

void PicketSwitch::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::needStageSwitchWriteA(this, rIter);
    const char* pName;
    MR::getObjectName(&pName, rIter);
    MR::initActor(this, rIter, pName, false);
    MR::connectToSceneMapObjDecorationStrongLight(this);
    initHitSensor(1);
    MR::addBodyMessageSensor(this, 0x49 /* ATYPE_SWITCH */);
    MR::initCollisionPartsAutoEqualScale(this, pName, getSensor(0), MR::getJointMtx(this, "Picket"));
    initEffectKeeper(0, nullptr, false);
    initSound(4, "PicketSwitch", nullptr, TVec3f(0.0f));
    initNerve(GET_NERVE(PicketSwitch, PicketSwitchNrvWait));
    makeActorAppeared();
}

void PicketSwitch::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startBrk(this, "Second");
        MR::setBrkFrameAndStop(this, 0.0f);
    }
}

void PicketSwitch::exeFirstDrop() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "First");
        MR::tryRumblePadStrong(this, WPAD_CHAN0);
        MR::startActionSound(this, "OjPicketSwitchDrop");
    }
}

void PicketSwitch::exeLastDrop() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Second");
        MR::startBrk(this, "Second");
        MR::tryRumblePadVeryStrong(this, WPAD_CHAN0);
        MR::startActionSound(this, "OjPicketSwitchDrop");
        MR::startActionSound(this, "OjPicketSwitchOn");
    }

    if (MR::isActionEnd(this)) {
        MR::onSwitchA(this);
        MR::invalidateHitSensors(this);
    }
}

bool PicketSwitch::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg)) {
        if (MR::isSensorType(pReceiver, 0x49 /* ATYPE_SWITCH */)) {
            if (isNerve(GET_NERVE(PicketSwitch, PicketSwitchNrvWait))) {
                setNerve(GET_NERVE(PicketSwitch, PicketSwitchNrvLastDrop));
                return true;
            }
            if ((isNerve(GET_NERVE(PicketSwitch, PicketSwitchNrvFirstDrop)))) {
                if (MR::isActionEnd(this)) {
                    setNerve(GET_NERVE(PicketSwitch, PicketSwitchNrvLastDrop));
                    return true;
                }
            }
            return false;
        }
    }
    return false;
}
