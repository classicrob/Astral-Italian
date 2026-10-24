#include "Game/Demo/DemoPlayerKeeper.hpp"
#include "Game/Util.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Util/SceneUtil.hpp"

DemoPlayerKeeper::DemoPlayerKeeper(const DemoExecutor* pExecutor) : mExecutor(pExecutor), mNumPlayerInfos(), mPlayerInfos(), mIsExecutedPlayer() {
    JMapInfo* map = nullptr;
    mNumPlayerInfos = DemoFunction::createSheetParser(mExecutor, "Player", &map);
    mPlayerInfos = new DemoPlayerInfo[mNumPlayerInfos];
    for (int i = 0; i < mNumPlayerInfos; i++) {
        DemoPlayerInfo* playerinfo = &mPlayerInfos[i];
        MR::getCsvDataStrOrNULL(&playerinfo->mPartName, map, "PartName", i);
        MR::getCsvDataStrOrNULL(&playerinfo->mPosName, map, "PosName", i);
        MR::getCsvDataStrOrNULL(&playerinfo->mBckName, map, "BckName", i);
        if (MR::hasCsvItem(map, "Visible")) {
            MR::getCsvDataS32(&playerinfo->mVisible, map, "Visible", i);
        } else {
            playerinfo->mVisible = -1;
        }
    }
}

DemoPlayerInfo::DemoPlayerInfo() : mPosName(nullptr), mPartName(nullptr), mBckName(nullptr), mVisible(-1) {
}




void DemoPlayerKeeper::start() {
    if (mNumPlayerInfos > 0) {
        for (s32 i = 0; i < mNumPlayerInfos; i++) {
            if (MR::isDemoSubPartActive(mPlayerInfos[i].mPartName)) {
                executePlayer(&mPlayerInfos[i]);
            }
        }
        mIsExecutedPlayer = true;
    }
}

void DemoPlayerKeeper::update() {
    const DemoPlayerInfo* info;
    for (s32 i = 0; i < mNumPlayerInfos; i++) {
        info = &mPlayerInfos[i];
        if (MR::isDemoPartFirstStep(info->mPartName)) {
            executePlayer(info);
        }
    }
    if (mIsExecutedPlayer) mIsExecutedPlayer = false;
}

const DemoPlayerInfo* DemoPlayerKeeper::findPlayerInfo(const char* partName) const {
    const DemoPlayerInfo* info;
    for (s32 i = 0; i < mNumPlayerInfos; i++) {
        info = &mPlayerInfos[i];
        if (MR::isEqualString(info->mPartName, partName)) return info;
    }
    return nullptr;
}

bool DemoPlayerKeeper::isExistPosName() const {
    for (s32 i = 0; i < mNumPlayerInfos; i++) {
        if (mPlayerInfos[i].mPosName != nullptr) return true;
    }
    return false;
}

void DemoPlayerKeeper::executePlayer(const DemoPlayerInfo* info) const {
    if (MR::isStageStoryBook() && MR::isDemoSubPartActive(nullptr) && mIsExecutedPlayer) return;
    if (info->mPosName != nullptr) MR::setPlayerPosOnGroundAndWait(info->mPosName, false);
    if (info->mBckName != nullptr) MR::changePlayerAnimationAndBva(info->mBckName);
    if (info->mVisible == 0) MR::hidePlayer();
    else if (info->mVisible == 1) MR::showPlayer();
}
