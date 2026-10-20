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

PlayerHeapHolder::PlayerHeapHolder() : mCondition(), mNapaHeap(), mGDDRHeap(), mIsDataMario(true) {
    ConditionUsePlayerHeap* condition = new ConditionUsePlayerHeap();
#ifdef GALAXY_NATIVE
    // Natively the player's J3D models and animations also get host-layout copies on these heaps
    // (MacioGalaxy measured 3.23 MiB of native player J3D loading for SMG1); adjust() trims the tails.
    mNapaHeap = PlayerHeapHolder::createHeap(0x200000 + 0x800000, MR::getStationedHeapNapa());
    JKRExpHeap* gddr = PlayerHeapHolder::createHeap(0x500000 + 0x800000, MR::getStationedHeapGDDR3());
#else
    mNapaHeap = PlayerHeapHolder::createHeap(0x200000, MR::getStationedHeapNapa());
    JKRExpHeap* gddr = PlayerHeapHolder::createHeap(0x500000, MR::getStationedHeapGDDR3());
#endif
    mGDDRHeap = gddr;
    condition->mNapaHeap = mNapaHeap;
    condition->mGDDRHeap = gddr;
    mCondition = condition;
    condition->mIsDataMario = mIsDataMario;
}

ConditionUsePlayerHeap* PlayerHeapHolder::getCondition() const {
    return mCondition;
}

void PlayerHeapHolder::adjust() {
    JKRExpHeap* napa = mNapaHeap;
    if (!napa->isEmpty()) {
#ifdef GALAXY_NATIVE
        napa->alloc(0x10000, 0);
#else
        napa->alloc(0x10000, nullptr);
#endif
    }

    JKRExpHeap* gddr = mGDDRHeap;
    if (!gddr->isEmpty()) {
#ifdef GALAXY_NATIVE
        gddr->alloc(0x10000, 0);
#else
        gddr->alloc(0x10000, nullptr);
#endif
    }

    MR::adjustHeapSize(mNapaHeap);
    MR::adjustHeapSize(mGDDRHeap);
}

void PlayerHeapHolder::dispose() {
    MR::removeResourceAndFileHolderIfIsEqualHeap(mNapaHeap);
    MR::removeResourceAndFileHolderIfIsEqualHeap(mGDDRHeap);

    if (mNapaHeap->mSize != 0) {
        mNapaHeap->freeAll();
    }

    if (mGDDRHeap->mSize != 0) {
        mGDDRHeap->freeAll();
    }
}

void PlayerHeapHolder::setIsDataMario(bool isDataMario) NO_INLINE {
    mIsDataMario = isDataMario;
    mCondition->mIsDataMario = isDataMario;
}

JKRExpHeap* PlayerHeapHolder::createHeap(u32 size, JKRHeap* pParent) {
    if (pParent->getMaxAllocatableSize(0x10) < size) {
        size = -1;
    }

    return JKRExpHeap::create(size, pParent, true);
}

ConditionUsePlayerHeap::~ConditionUsePlayerHeap() {}
