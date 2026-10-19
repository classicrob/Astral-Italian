#include "Game/AreaObj/SwitchArea.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

SwitchArea::SwitchArea(const char* pName) : AreaObj(pName) {
    mSoundType = -1;
    mIsSoundPlayed = false;
}

void SwitchArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::getJMapInfoArg4NoInit(rIter, &mSoundType);
    MR::connectToSceneAreaObj(this);
}

void SwitchArea::movement() {
    if (!isUpdate()) {
        return;
    }

    bool isSwitchBOn = isValidSwitchB() && isOnSwitchB();

    if (isSwitchBOn && isInVolume(*MR::getPlayerPos())) {
        if (mObjArg1 == -1) {
            onSwitchA();
        } else {
            offSwitchA();
            if (mObjArg0 == 1) {
                invalidate();
            }
        }
    } else if (mObjArg0 == 0) {
        offSwitchA();
    }
}

void SwitchArea::onSwitchA() {
    AreaObj::onSwitchA();

    if (mIsSoundPlayed) {
        return;
    }

    switch (mSoundType) {
    case 0:
        MR::startSystemSE("SE_SY_READ_RIDDLE_S", -1, -1);
        break;
    case 1:
        MR::startSystemSE("SE_SY_READ_RIDDLE_SS", -1, -1);
        break;
    }

    mIsSoundPlayed = true;
}

bool SwitchArea::isUpdate() const {
    if (mObjArg2 != -1 && !MR::isOnGroundPlayer()) {
        return false;
    }

    if (!checkPlayerDrillRush()) {
        return false;
    }

    if (mObjArg0 == 0) {
        return true;
    }

    if (mObjArg1 == -1) {
        return !isOnSwitchA();
    }

    return isOnSwitchA();
}

bool SwitchArea::checkPlayerDrillRush() const {
    if (mObjArg5 == 0) {
        return MR::isCurrentRushItemDrill();
    }

    return true;
}

const char* SwitchArea::getManagerName() const {
    return "SwitchArea";
}
