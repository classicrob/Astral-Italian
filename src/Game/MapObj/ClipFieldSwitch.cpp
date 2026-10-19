#include "Game/MapObj/ClipFieldSwitch.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/ClipAreaHolder.hpp"
#include "Game/Util.hpp"

ClipFieldSwitch::ClipFieldSwitch(const char* pName) : LiveActor(pName) {
}

void ClipFieldSwitch::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::initUseStageSwitchWriteB(this, rIter);
    initHitSensor(1);
    MR::addHitSensorEye(this, "Body", 8, 1000.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::invalidateClipping(this);
    appear();
}

void ClipFieldSwitch::control() {
    HitSensor* rushSensor;
    if (MR::isPlayerInRush() && MR::isSensorType(rushSensor = MR::getCurrentRushSensor(), 0x60)) {
        mPosition.set(*MR::getSensorPos(rushSensor));
    } else {
        mPosition.set(*MR::getPlayerPos());
    }

    if (MR::isInClipArea(mPosition)) {
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }

        if (MR::isValidSwitchB(this)) {
            MR::offSwitchB(this);
        }
    } else {
        if (MR::isValidSwitchA(this)) {
            MR::offSwitchA(this);
        }

        if (MR::isValidSwitchB(this)) {
            MR::onSwitchB(this);
        }
    }
}

ClipFieldSwitch::~ClipFieldSwitch() {
}
