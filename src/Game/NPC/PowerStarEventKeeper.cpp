#include "Game/NPC/PowerStarEventKeeper.hpp"
#include "Game/Util.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"

PowerStarEventKeeper::PowerStarEventKeeper() : mStartScenario(1), mPowerStarNum(1), mInfoTbl() {
    mPowerStarNum = MR::getCurrentStagePowerStarNum();
    initStarInfoTable();
}

// NONMATCHING: SMG2 findStarID goes through GalaxyStatusAccessor::isExistPowerStarAtScenario; this is the Petari form.
s32 PowerStarEventKeeper::findStarID(const char* pName) const {
    if (MR::getAppearPowerStarObjName(MR::getCurrentScenarioNo()) != nullptr &&
        MR::isEqualString(pName, MR::getAppearPowerStarObjName(MR::getCurrentScenarioNo()))) {
        return MR::getCurrentScenarioNo();
    }

    for (s32 starID = mStartScenario; starID <= mPowerStarNum; starID++) {
        if (starID == MR::getCurrentScenarioNo()) {
            continue;
        }

        if (MR::getAppearPowerStarObjName(starID) == nullptr) {
            continue;
        }

        if (MR::isEqualString(pName, MR::getAppearPowerStarObjName(starID))) {
            return starID;
        }
    }

    return -1;
}

bool PowerStarEventKeeper::isGreen(const char* pName, s32 starID) const {
    if (starID < 0) {
        starID = findStarID(pName);
    }

    return MR::isPowerStarGreenInCurrentStage(starID);
}

bool PowerStarEventKeeper::isBronze(const char* pName, s32 starID) const {
    if (starID < 0) {
        starID = findStarID(pName);
    }

    return MR::isPowerStarBronzeInCurrentStage(starID);
}

bool PowerStarEventKeeper::isGrand(const char* pName, s32 starID) const {
    if (starID < 0) {
        starID = findStarID(pName);
    }

    return MR::isGrandStarInCurrentStage(starID);
}

bool PowerStarEventKeeper::isSuccess(const char* pName, s32 starNum) const {
    if (starNum < 0) {
        starNum = findStarID(pName);
    }

    return MR::hasPowerStarInCurrentStage(starNum);
}
