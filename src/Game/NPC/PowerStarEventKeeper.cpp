#include "Game/NPC/PowerStarEventKeeper.hpp"
#include "Game/Util.hpp"
#include "Game/MapObj/PowerStarHolder.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/NPC/EventDirector.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"

PowerStarEventKeeper::PowerStarEventKeeper() : mStartScenario(1), mPowerStarNum(1), mInfoTbl() {
    mPowerStarNum = MR::getCurrentStagePowerStarNum();
    initStarInfoTable();
}

s32 PowerStarEventKeeper::findStarID(const char* pName) const {
    if (MR::getAppearPowerStarObjName(MR::getCurrentScenarioNo()) != nullptr &&
        MR::isEqualString(pName, MR::getAppearPowerStarObjName(MR::getCurrentScenarioNo()))) {
        return MR::getCurrentScenarioNo();
    }

    for (s32 starID = mStartScenario; starID <= mPowerStarNum; starID++) {
        if (starID == MR::getCurrentScenarioNo()) {
            continue;
        }

        if (!MR::makeCurrentGalaxyStatusAccessor().isExistPowerStarAtScenario(MR::getCurrentScenarioNo(), starID)) {
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

void PowerStarEventKeeper::declareStar(const char* pName, const char*, s32 starID) {
    if (starID < 0) {
        starID = findStarID(pName);
    }
    mInfoTbl[u32(starID - 1)].mDemoName = pName;
}

void PowerStarEventKeeper::initStarInfoTable() {
    mInfoTbl = new PowerStarInfo[mPowerStarNum];
    for (s32 i = 0; i < mPowerStarNum; i++) {
        PowerStarInfo* info = &mInfoTbl[i + mStartScenario - 1];
        info->mIsExistInGalaxy = false;
        info->mHasNoAppearObj = false;
        info->mIsRequestedAppear = false;
        info->mDemoName = nullptr;
    }
    for (s32 starID = mStartScenario; starID <= mPowerStarNum; starID++) {
        if (MR::makeCurrentGalaxyStatusAccessor().isExistPowerStarAtScenario(MR::getCurrentScenarioNo(), starID)) {
            mInfoTbl[u32(starID - 1)].mIsExistInGalaxy = true;
        }
        if (MR::getAppearPowerStarObjName(starID) == nullptr) {
            mInfoTbl[u32(starID - 1)].mHasNoAppearObj = true;
        }
    }
}

void PowerStarEventKeeper::initStarInfoTableAfterPlacement() {
    for (s32 i = 0; i < mPowerStarNum; i++) {
        s32 starID = i + mStartScenario;
        PowerStarInfo* info = &mInfoTbl[u32(starID - 1)];
        if (info->mIsExistInGalaxy) {
            if (info->mDemoName) {
                PowerStar* star = PowerStarFunction::findPowerStar(starID);
                star->mIsInDemo = true;
                star->kill();
            } else if (info->mHasNoAppearObj) {
                PowerStar* star = PowerStarFunction::findPowerStar(starID);
                if (star->mIsUseAwakeSwitch) {
                    star->kill();
                }
                if (star->mIsGreenStar) {
                    bool isGreenStage;
                    if (GameDataFunction::isOnGameEventFlagEnding120Stars() == false) {
                        isGreenStage = false;
                    } else {
                        isGreenStage = MR::isPowerStarGreenInCurrentStage();
                    }
                    if (!isGreenStage) {
                        star->kill();
                    }
                }
            }
        }
    }
}

void PowerStarEventKeeper::requestAppearPowerStar(const char* pName, s32 starID, const TVec3f* pPos, bool forceAppear, bool withoutDemo, bool fromBelow) {
    GameSequenceFunction::setResultPowerStarAppeared();
    if (starID < 0) {
        starID = findStarID(pName);
    }
    mInfoTbl[u32(starID - 1)].mIsRequestedAppear = true;
    if (withoutDemo) {
        EventFunction::getPowerStarHolder()->appearPowerStarWithoutDemo(starID);
    } else {
        EventFunction::getPowerStarHolder()->requestAppearPowerStar(starID, pPos, forceAppear, fromBelow);
    }
}
