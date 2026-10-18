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
