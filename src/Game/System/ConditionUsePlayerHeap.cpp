#include "Game/Util/Functor.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/GameSystemFunction.hpp"
#include "Game/System/GameSystemStationedArchiveLoader.hpp"
#include "Game/System/HeapMemoryWatcher.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/System/StationedFileInfo.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <JSystem/JKernel/JKRExpHeap.hpp>

// These condition virtuals lie in the adjacent original text split.
bool ConditionIfIsNotPlayer1st::isExecute(const MR::StationedFileInfo* info) const {
    switch (info->mLoadType) {
    case 2:
    case 3:
    case 4:
    case 6:
    case 8:
        return false;
    default:
        return true;
    }
}

JKRHeap* ConditionIfIsNotPlayer1st::getProperHeap(const MR::StationedFileInfo*) const {
    return nullptr;
}

bool ConditionIfIsNotPlayer::isExecute(const MR::StationedFileInfo* info) const {
    switch (info->mLoadType) {
    case 0:
    case 1:
    case 3:
    case 4:
    case 5:
    case 7:
        return false;
    default:
        return true;
    }
}

JKRHeap* ConditionIfIsNotPlayer::getProperHeap(const MR::StationedFileInfo*) const {
    return nullptr;
}

ConditionUsePlayerHeap::ConditionUsePlayerHeap() : mNapaHeap(), mGDDRHeap(), mIsDataMario(true) {
}

bool ConditionUsePlayerHeap::isExecute(const MR::StationedFileInfo* pInfo) const {
    MR::StationedFileInfo::LoadType type = MR::StationedFileInfo::LOAD_TYPE_MOUNT_RESOURCE_LUIGI;

    if (mIsDataMario) {
        type = MR::StationedFileInfo::LOAD_TYPE_MOUNT_RESOURCE_MARIO;
    }

    return pInfo->mLoadType == type;
}

JKRHeap* ConditionUsePlayerHeap::getProperHeap(const MR::StationedFileInfo* pInfo) const {
    switch (pInfo->mHeapType) {
    case MR::StationedFileInfo::HEAP_TYPE_NAPA:
        return mNapaHeap;
    case MR::StationedFileInfo::HEAP_TYPE_GDDR:
        return mGDDRHeap;
    case MR::StationedFileInfo::HEAP_TYPE_NONE:
        return nullptr;
    default:
        return nullptr;
    }
}
