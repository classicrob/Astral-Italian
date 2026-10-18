#include "Game/Demo/DemoPlayerKeeper.hpp"
#include "Game/Util.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"

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



