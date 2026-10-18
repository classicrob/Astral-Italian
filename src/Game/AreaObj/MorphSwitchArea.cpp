#include "Game/AreaObj/MorphSwitchArea.hpp"
#include "Game/Util.hpp"

MorphSwitchArea::MorphSwitchArea(const char* pName) : AreaObj(pName), mMorphType(0) {
}

void MorphSwitchArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);

    s32 morphType = -1;
    MR::getJMapInfoArg0WithInit(rIter, &morphType);

    if (morphType == -1) {
        mMorphType = 0;
    } else {
        mMorphType = morphType;
    }

    MR::connectToSceneAreaObj(this);
}

void MorphSwitchArea::movement() {
    if (checkCondition() && isInVolume(*MR::getPlayerPos())) {
        onSwitchA();
    } else {
        offSwitchA();
    }
}

bool MorphSwitchArea::checkCondition() const {
    switch (mMorphType) {
    case 0:
        return MR::isPlayerElementModeTeresa();
    case 1:
        return MR::getCurrentRushActor() != nullptr;
    case 2:
        return MR::isPlayerElementModeYoshiLight();
    default:
        return false;
    }
}
