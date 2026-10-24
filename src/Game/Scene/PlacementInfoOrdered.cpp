#define JMAPINFOITER_COPY_NOINLINE
#include "Game/Scene/PlacementInfoOrdered.hpp"
#include "Game/LiveActor/AllLiveActorGroup.hpp"
#include "Game/Map/PlanetMapCreator.hpp"
#include "Game/MapObj/PlantObjFactory.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Scene/StageDataArchiveHolder.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SceneUtil.hpp"

namespace {
    static bool isPrioPlacementObjInfo(const char* pName) {
        return MR::isEqualStringCase(pName, "AreaObjInfo") || MR::isEqualStringCase(pName, "PlanetObjInfo") ||
               MR::isEqualStringCase(pName, "DemoObjInfo") || MR::isEqualStringCase(pName, "CameraCubeInfo");
    }
};  // namespace

PlacementInfoOrderedHolder::PlacementInfoOrderedHolder()
    : mCommonPriority(nullptr), mCommon(nullptr), mScenarioPriority(nullptr), mScenario(nullptr), mAfterScenario(nullptr) {
    mAfterScenario = new PlacementInfoOrdered();
    mCommonPriority = new PlacementInfoOrdered();
    mCommon = new PlacementInfoOrdered();
    mScenarioPriority = new PlacementInfoOrdered();
    mScenario = new PlacementInfoOrdered();
}

void PlacementInfoOrderedHolder::attachCommon(const StageDataArchive* pArchive) {
    if (pArchive == nullptr) {
        return;
    }

    attachInfos(mCommonPriority, mCommon, mAfterScenario, &pArchive->mCommonInfos, pArchive);
}

void PlacementInfoOrderedHolder::attachScenario(const StageDataArchive* pArchive) {
    if (pArchive == nullptr) {
        return;
    }

    attachInfos(mScenarioPriority, mScenario, nullptr, &pArchive->mScenarioInfos, pArchive);
}

void PlacementInfoOrderedHolder::sortCommon() {
    mCommonPriority->sort();
    mCommon->sort();
}

void PlacementInfoOrderedHolder::sortScenario() {
    mScenarioPriority->sort();
    mScenario->sort();
    mAfterScenario->sort();
}

void PlacementInfoOrderedHolder::requestFileLoad() {
    mCommonPriority->requestFileLoad();
    mCommon->requestFileLoad();
    mScenarioPriority->requestFileLoad();
    mScenario->requestFileLoad();
    mAfterScenario->requestFileLoad();
}

void PlacementInfoOrderedHolder::initPlacementHighPriority() {
    mCommonPriority->initPlacement();
    mScenarioPriority->initPlacement();
}

void PlacementInfoOrderedHolder::initPlacement() {
    mCommon->initPlacement();
    mScenario->initPlacement();
    mAfterScenario->initPlacement();
}

void PlacementInfoOrderedHolder::attachInfos(PlacementInfoOrdered* pPriority, PlacementInfoOrdered* pNormal, PlacementInfoOrdered* pAfterScenario,
                                             const MR::BothDirList< JMapInfo >* pList, const StageDataArchive* pArchive) {
    for (MR::BothDirList< JMapInfo >::iterator it = pList->begin(); !it.isEnd(); it++) {
        if (::isPrioPlacementObjInfo(it->getName())) {
            pPriority->attach(&*it, nullptr, pArchive);
        } else {
            pNormal->attach(&*it, pAfterScenario, pArchive);
        }
    }
}

PlacementInfoOrdered::PlacementInfoOrdered() : mSetList(), mGeneratedList() {
}

void PlacementInfoOrdered::attach(const JMapInfo* pInfo, PlacementInfoOrdered* pAfterScenario, const StageDataArchive* pArchive) {
    s32 count = pInfo->getNumEntries();
    for (s32 i = 0; i < count; i++) {
        JMapInfoIter iter(pInfo, i);
        s32 generatorID = -1;
        if (MR::getGeneratorID(iter, &generatorID) && generatorID != -1) {
            addGeneratedIndex(iter);
        } else {
            const char* pName = "";
            MR::getObjectName(&pName, iter);
            s32 shapeId = -1;
            MR::getJMapInfoShapeIdWithInit(iter, &shapeId);
            Identifier identifier(pName, shapeId);
            if (pAfterScenario != nullptr && shapeId == -1 && PlanetMapCreatorFunction::isLoadArchiveAfterScenarioSelected(pName)) {
                pAfterScenario->insert(identifier, iter);
            } else {
                insert(identifier, iter);
            }
        }
    }
}

void PlacementInfoOrdered::sort() {
    SameIdSet* sets[0x800];
    u32 count = mSetList.mCount;
    s32 i = 0;

    for (MR::BothDirList< SameIdSet >::iterator it = mSetList.begin(); !it.isEnd(); it++) {
        it->mPriority = 1;
        sets[i++] = &*it;
    }

    s32 gap = 13;
    while (gap < count) {
        gap = gap * 3 + 1;
    }

    gap /= 9;
    for (; gap > 0; gap /= 3) {
        s32 h = gap;
        for (s32 j = h; j < count; j++) {
            SameIdSet* pSet = sets[j];
            s32 k = j - h;
            while (k >= 0 && *pSet < *sets[k]) {
                sets[k + h] = sets[k];
                k -= h;
            }

            sets[k + h] = pSet;
        }
    }

    i = 0;
    for (MR::BothDirPtrLink* pLink = mSetList.mHead; pLink != nullptr; pLink = pLink->mNextLink) {
        pLink->mValue = sets[i++];
    }
}

void PlacementInfoOrdered::requestFileLoad() {
    for (MR::BothDirList< SameIdSet >::iterator it = mSetList.begin(); !it.isEnd(); it++) {
        for (MR::BothDirList< Index >::iterator idx = it->mList.begin(); !idx.isEnd(); idx++) {
            NameObjFactory::callPostCreation(it->mName, idx->mInfoIter);
        }
    }

    for (MR::BothDirList< JMapInfoIter >::iterator it = mGeneratedList.begin(); !it.isEnd(); it++) {
        const char* pName = "";
        MR::getObjectName(&pName, *it);
        NameObjFactory::callPostCreation(pName, *it);
    }
}

void PlacementInfoOrdered::initPlacement() {
    MR::startInitLiveActorSystemInfo();

    for (MR::BothDirList< SameIdSet >::iterator it = mSetList.begin(); !it.isEnd(); it++) {
        CreationFuncPtr creator;
        if (it->mShapeId != -1) {
            creator = PlantObjFactory::getCreator(it->mName);
        } else {
            creator = NameObjFactory::getCreator(it->mName);
        }

        if (creator != nullptr) {
            const char* pName = it->mName;
            const char* pJapaneseName = MR::getJapaneseObjectNameOrNull(pName);
            if (pJapaneseName == nullptr) {
                pJapaneseName = pName;
            }

            for (MR::BothDirList< Index >::iterator idx = it->mList.begin(); !idx.isEnd(); idx++) {
                const JMapInfoIter& rIter = idx->mInfoIter;
                MR::setCurrentPlacementZoneId(MR::getPlacedZoneId(rIter));
                MR::initialize(creator(pJapaneseName), rIter);
                MR::clearCurrentPlacementZoneId();
            }
        }
    }
}

void PlacementInfoOrdered::insert(const Identifier& rIdentifier, const JMapInfoIter& rIter) {
    SameIdSet* pSet = find(rIdentifier);
    if (pSet == nullptr) {
        pSet = createSameIdSet(rIdentifier);
    }

    pSet->mList.append(createIndex(rIter));
}

PlacementInfoOrdered::SameIdSet* PlacementInfoOrdered::find(const Identifier& rIdentifier) const {
    for (MR::BothDirList< SameIdSet >::iterator it = mSetList.begin(); !it.isEnd(); it++) {
        if (*it == rIdentifier) {
            return &*it;
        }
    }

    return nullptr;
}

PlacementInfoOrdered::SameIdSet* PlacementInfoOrdered::createSameIdSet(const Identifier& rIdentifier) {
    SameIdSet* pSet = new SameIdSet();
    mSetList.append(new MR::BothDirLink< SameIdSet >(pSet));
    *static_cast< Identifier* >(pSet) = rIdentifier;
    return pSet;
}

PlacementInfoOrdered::Index* PlacementInfoOrdered::createIndex(const JMapInfoIter& rIter) {
    Index* pIndex = new Index();
    pIndex->mInfoIter = rIter;
    return pIndex;
}

void PlacementInfoOrdered::addGeneratedIndex(const JMapInfoIter& rIter) {
    JMapInfoIter* pIter = new JMapInfoIter(rIter);
    mGeneratedList.append(new MR::BothDirLink< JMapInfoIter >(pIter));
}
