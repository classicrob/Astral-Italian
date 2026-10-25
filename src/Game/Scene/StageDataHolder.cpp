#include "Game/Scene/StageDataHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Array.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Scene/PlacementInfoOrdered.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Scene/StageDataArchiveHolder.hpp"
#include "Game/Scene/StageResourceLoader.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JKernel/JKRFileFinder.hpp>
#include <JSystem/JKernel/JKRMemArchive.hpp>
#include <cstdio>

template JMapInfoIter JMapInfo::findElement< s32 >(const char*, s32, int) const;

namespace {
    inline void initSelectedStageLayers(u32 scenarioLayers, StageDataArchiveHolder* const& archive,
                                       MR::BothDirList<JMapInfo>* list, const char* directory, const u32& commonLayers) {
        archive->initLayerJmpInfo(list, directory, commonLayers | scenarioLayers);
    }

    static const char* cLayerDirName[0x11] = {"Common", "LayerA", "LayerB", "LayerC", "LayerD", "LayerE", "LayerF", "LayerG", "LayerH",
                                              "LayerI", "LayerJ", "LayerK", "LayerL", "LayerM", "LayerN", "LayerO", "LayerP"};
};  // namespace

void StageDataArchiveHolder::initLayerJmpInfo(MR::BothDirList< JMapInfo >* pList, const char* pDir, u32 layerMask) {
    char path[64];

    for (u32 i = 0; i < 0x11; i++) {
        if (layerMask & (1 << i)) {
            snprintf(path, sizeof(path), "%s/%s", pDir, ::cLayerDirName[i]);
            initJmpInfo(pList, path);
        }
    }
}

void StageDataArchiveHolder::initCommonLayerJmpInfo(const char* pDir, u32 layerMask) {
    char path[64];

    for (u32 i = 0; i < 0x11; i++) {
        if (layerMask & (1 << i)) {
            snprintf(path, sizeof(path), "%s/%s", pDir, ::cLayerDirName[i]);
            mMapArchive->initJmpInfo(&mMapArchive->mCommonInfos, path);
            if (mSoundArchive != nullptr) {
                mSoundArchive->initJmpInfo(&mSoundArchive->mCommonInfos, path);
            }
            if (mDesignArchive != nullptr) {
                mDesignArchive->initJmpInfo(&mDesignArchive->mCommonInfos, path);
            }
        }
    }
}

void StageDataArchiveHolder::initScenarioLayerJmpInfo(const char* pDir, u32 layerMask) {
    char path[64];

    for (u32 i = 0; i < 0x11; i++) {
        if (layerMask & (1 << i)) {
            snprintf(path, sizeof(path), "%s/%s", pDir, ::cLayerDirName[i]);
            mMapArchive->initJmpInfo(&mMapArchive->mScenarioInfos, path);
            if (mSoundArchive != nullptr) {
                mSoundArchive->initJmpInfo(&mSoundArchive->mScenarioInfos, path);
            }
            if (mDesignArchive != nullptr) {
                mDesignArchive->initJmpInfo(&mDesignArchive->mScenarioInfos, path);
            }
        }
    }
}

StageDataArchive* StageDataArchiveHolder::mountArchive(const char* pStageName, const char* pDir) {
    char path[256];
    snprintf(path, sizeof(path), "/StageData/%s/%s%s.arc", pStageName, pStageName, pDir);
    JKRArchive* pArchive = MR::receiveArchive(path);

    if (pArchive == nullptr) {
        return nullptr;
    }

    return new StageDataArchive(pArchive, pStageName, pDir);
}

StageDataHolder::StageDataHolder(const char* pStageName, int zoneID, bool loadCommon)
    : NameObj("StageDataHolder"), mObjNameTbl(nullptr), mStageDataHolderCount(0), mStageName(pStageName), mZoneID(zoneID), mLoadCommon(loadCommon),
      mDataStart(0), mDataEnd(0), mPlacementInfoOrdered(nullptr), mArchiveHolder(nullptr) {
    MR::zeroMemory(mStageDataArray, sizeof(mStageDataArray));
    mPlacementMtx.identity();
    mArchiveHolder = new StageDataArchiveHolder(pStageName);

    if (mZoneID == 0) {
        mPlacementInfoOrdered = new PlacementInfoOrderedHolder();
    }
}

void StageDataHolder::init(const JMapInfoIter& rIter) {
    if (mZoneID == 0) {
        initTableData();
    }

    if (mLoadCommon) {
        u32 commonLayers = ScenarioDataFunction::getCurrentCommonLayers(mStageName);
        mArchiveHolder->initCommonLayerJmpInfo("/jmp/Placement", commonLayers);
        mArchiveHolder->initCommonLayerJmpInfo("/jmp/MapParts", commonLayers);
    }

    createLocalStageDataHolder(mArchiveHolder->mMapArchive->mCommonInfos, true);

    if (mZoneID == 0) {
        initPlacementInfoOrderedCommon();
    }
}

void StageDataHolder::initAfterScenarioSelected() {
    u32 layers = ScenarioDataFunction::getCurrentScenarioLayers(mStageName, MR::getCurrentScenarioNo());

    if (!mLoadCommon) {
        layers |= ScenarioDataFunction::getCurrentCommonLayers(mStageName);
    }

    mArchiveHolder->initScenarioLayerJmpInfo("/jmp/Placement", layers);
    mArchiveHolder->initScenarioLayerJmpInfo("/jmp/MapParts", layers);
    createLocalStageDataHolder(mArchiveHolder->mMapArchive->mScenarioInfos, false);

    for (s32 i = 0; i < mStageDataHolderCount; i++) {
        mStageDataArray[i]->initAfterScenarioSelected();
    }

    if (mZoneID == 0) {
        initPlacementInfoOrderedScenario();
    }

    initAllLayerJmpInfo(&mPlacementObjs, "/jmp/Placement");
    initAllLayerJmpInfo(&mPlacementObjs, "/jmp/MapParts");
    initAllLayerJmpInfo(&mStartObjs, "/jmp/Start");
    initAllLayerJmpInfo(&mGeneralPosObjs, "/jmp/GeneralPos");
    mArchiveHolder->initJmpInfo(&mPathObjs, "/jmp/Path");
    calcDataAddress();
}

void StageDataHolder::initFileLoadPostCreation() {
    mPlacementInfoOrdered->requestFileLoad();
}

void StageDataHolder::initPlacement() {
    MR::setInitializeStatePlacementPlayer();
    initPlacementMario();
    MR::setInitializeStatePlacementHighPriority();
    mPlacementInfoOrdered->initPlacementHighPriority();
    MR::setInitializeStatePlacement();
    mPlacementInfoOrdered->initPlacement();
}

JMapInfoIter StageDataHolder::getCommonPathPointInfo(const JMapInfo** ppOut, int idx) const {
    const JMapInfo* pInfo = findJmpInfoFromArray(&mPathObjs, "CommonPathInfo");
    JMapInfoIter pathIter = pInfo->findElement< s32 >("l_id", idx, 0);
    return getCommonPathPointInfoFromRailDataIndex(ppOut, pathIter.mIndex);
}

JMapInfoIter StageDataHolder::getCommonPathPointInfoFromRailDataIndex(const JMapInfo** ppInfo, int idx) const {
    const JMapInfo* pInfo = findJmpInfoFromArray(&mPathObjs, "CommonPathInfo");
    char buf[128];
    snprintf(buf, sizeof(buf), "CommonPathPointInfo.%d", idx);
    *ppInfo = findJmpInfoFromArray(&mPathObjs, buf);
    return JMapInfoIter(pInfo, idx);
}

s32 StageDataHolder::getCommonPathInfoElementNum() const {
    return findJmpInfoFromArray(&mPathObjs, "CommonPathInfo")->getNumEntries();
}

s32 StageDataHolder::getStartPosNum() const {
    s32 cur = 0;

    for (MR::BothDirList< JMapInfo >::iterator it = mStartObjs.begin(); !it.isEnd(); it++) {
        cur += it->getNumEntries();
    }

    for (s32 i = 0; i < mStageDataHolderCount; i++) {
        cur += mStageDataArray[i]->getStartPosNum();
    }

    return cur;
}

s32 StageDataHolder::getCurrentStartZoneId() const {
    return findPlacedStageDataHolder(makeCurrentMarioJMapInfoIter())->mZoneID;
}

s32 StageDataHolder::getCurrentStartCameraId() const {
    JMapInfoIter marioIter = makeCurrentMarioJMapInfoIter();
    s32 cameraID;
    bool ret = marioIter.getValue< s32 >("Camera_id", &cameraID);

    if (ret) {
        return cameraID;
    }

    return -1;
}

void StageDataHolder::getStartCameraIdInfoFromStartDataIndex(JMapIdInfo* pInfo, int startDataIdx) const {
    JMapInfoIter startIter = getStartJMapInfoIterFromStartDataIndex(startDataIdx);
    s32 cameraID;
    startIter.getValue< s32 >("Camera_id", &cameraID);
    pInfo->initialize(cameraID, startIter);
}

s32 StageDataHolder::getGeneralPosNum() const {
    s32 count = 0;

    if (mGeneralPosObjs.mCount != 0) {
        for (MR::BothDirList< JMapInfo >::iterator it = mGeneralPosObjs.begin(); !it.isEnd(); it++) {
            count += it->getNumEntries();
        }
    }

    for (s32 i = 0; i < mStageDataHolderCount; i++) {
        count += mStageDataArray[i]->getGeneralPosNum();
    }

    return count;
}

JMapInfoIter StageDataHolder::getGeneralPosInfoFromDataIndex(int idx_) const {
    int idx = idx_;

    for (MR::BothDirList< JMapInfo >::iterator it = mGeneralPosObjs.begin(); !it.isEnd(); it++) {
        const JMapInfo* pInfo = &*it;
        const JMapData* curData = pInfo->mData;
        int curIdx = curData ? curData->mNumEntries : 0;

        if (idx < curIdx) {
            return JMapInfoIter(pInfo, idx);
        }

        curIdx = curData ? curData->mNumEntries : 0;

        idx -= curIdx;
    }

    for (s32 i = 0; i < mStageDataHolderCount; i++) {
        StageDataHolder* pHolder = mStageDataArray[i];
        int startPosNum = pHolder->getGeneralPosNum();

        if (idx < startPosNum) {
            return pHolder->getGeneralPosInfoFromDataIndex(idx);
        }

        idx -= startPosNum;
    }

    return JMapInfoIter();
}

s32 StageDataHolder::getChildObjNum(const JMapInfoIter& rIter) const {
    s32 count = 0;
    s32 linkID;
#ifdef GALAXY_NATIVE
    if (!MR::getJMapInfoLinkID(rIter, &linkID)) {
        return 0;
    }
#else
    MR::getJMapInfoLinkID(rIter, &linkID);
#endif

    if (mPlacementObjs.mCount != 0) {
        for (MR::BothDirList< JMapInfo >::iterator it = mPlacementObjs.begin(); !it.isEnd(); it++) {
            for (s32 i = 0; i < it->getNumEntries(); i++) {
                s32 generatorID = -1;
                if (MR::getGeneratorID(JMapInfoIter(&*it, i), &generatorID) && generatorID == linkID) {
                    count++;
                }
            }
        }
    }

    return count;
}

JMapInfoIter StageDataHolder::getChildObjInfoFromDataIndex(const JMapInfoIter& rIter, int index) const {
    s32 count = 0;
    s32 linkID;
#ifdef GALAXY_NATIVE
    if (!MR::getJMapInfoLinkID(rIter, &linkID)) {
        return JMapInfoIter();
    }
#else
    MR::getJMapInfoLinkID(rIter, &linkID);
#endif

    for (MR::BothDirList< JMapInfo >::iterator it = mPlacementObjs.begin(); !it.isEnd(); it++) {
        for (s32 i = 0; i < it->getNumEntries(); i++) {
            s32 generatorID = -1;
            JMapInfoIter iter(&*it, i);
            if (MR::getGeneratorID(iter, &generatorID) && generatorID == linkID) {
                if (count == index) {
                    return iter;
                }

                count++;
            }
        }
    }

    return JMapInfoIter();
}

const StageDataHolder* StageDataHolder::findPlacedStageDataHolder(const JMapInfoIter& rIter) const {
#ifdef GALAXY_NATIVE
    uintptr_t data = reinterpret_cast<uintptr_t>(rIter.mInfo->mData) + rIter.mInfo->mData->mDataOffset +
                     rIter.mInfo->mData->mEntrySize * rIter.mIndex;
#else
    u32 data = (s32)rIter.mInfo->mData + rIter.mInfo->mData->mDataOffset + rIter.mInfo->mData->mEntrySize * rIter.mIndex;
#endif

    if (mDataStart <= data && data < mDataEnd) {
        return this;
    }

    for (s32 i = 0; i < mStageDataHolderCount; i++) {
        const StageDataHolder* pHolder = mStageDataArray[i]->findPlacedStageDataHolder(rIter);

        if (pHolder != nullptr) {
            return pHolder;
        }
    }

    return nullptr;
}

const StageDataHolder* StageDataHolder::getStageDataHolderFromZoneId(int zoneID) const {
    if (zoneID == 0) {
        return this;
    }

    for (s32 i = 0; i < mStageDataHolderCount; i++) {
        StageDataHolder* pHolder = mStageDataArray[i];
        s32 curZoneID = pHolder->mZoneID;

        if (zoneID == curZoneID) {
            return pHolder;
        }
    }

    return nullptr;
}

StageDataHolder* StageDataHolder::getStageDataHolderFromZoneId(int zoneID) {
    return const_cast< StageDataHolder* >(static_cast< const StageDataHolder* >(this)->getStageDataHolderFromZoneId(zoneID));
}

bool StageDataHolder::isPlacedZone(int zoneID) const {
    if (!zoneID) {
        return true;
    }

    for (s32 i = 0; i < mStageDataHolderCount; i++) {
        if (zoneID == mStageDataArray[i]->mZoneID) {
            return true;
        }
    }

    return false;
}

const char* StageDataHolder::getJapaneseObjectName(const char* pName) const {
    const JMapInfoIter englishName = mObjNameTbl->findElement< const char* >("en_name", pName, 0);

    if (englishName == mObjNameTbl->end()) {
        return nullptr;
    }

    const char* japaneseName;
    englishName.getValue< const char* >("jp_name", &japaneseName);
    return japaneseName;
}

void* StageDataHolder::getStageArchiveResource(const char* pName) {
    return mArchiveHolder->mMapArchive->mArchive->getResource('????', pName);
}

s32 StageDataHolder::getStageArchiveResourceSize(void* pData) {
    return mArchiveHolder->mMapArchive->mArchive->getResSize(pData);
}

void* StageDataHolder::getUseResourceArchiveResource(const char* pName) {
    StageDataArchive* pArchive = mArchiveHolder->mUseResourceArchive;

    if (pArchive == nullptr) {
        return nullptr;
    }

    if (pArchive->mArchive == nullptr) {
        return nullptr;
    }

    return pArchive->mArchive->getResource('????', pName);
}

JKRArchive* StageDataHolder::getGhostArchive() {
    StageDataArchive* pArchive = mArchiveHolder->mGhostArchive;

    if (pArchive == nullptr) {
        return nullptr;
    }

    return pArchive->mArchive;
}

void StageDataHolder::initPlacementMario() {
    JMapInfoIter iter = makeCurrentMarioJMapInfoIter();
    MR::setCurrentPlacementZoneId(MR::getPlacedZoneId(iter));
    const char* objName = "";
    MR::getObjectName(&objName, iter);
    CreationFuncPtr funcPtr = NameObjFactory::getCreator(objName);

    NameObj* obj = funcPtr("マリオアクター");
    MR::initialize(obj, iter);
    MR::clearCurrentPlacementZoneId();
}

JMapInfoIter StageDataHolder::makeMarioJMapInfoIter(const JMapIdInfo& rInfo) const {
    const StageDataHolder* pHolder = getStageDataHolderFromZoneId(rInfo.mZoneID);
    s32 id = rInfo._0;

    for (MR::BothDirList< JMapInfo >::iterator it = pHolder->mStartObjs.begin(); !it.isEnd(); it++) {
        JMapInfoIter iter = it->findElement< s32 >("MarioNo", id, 0);
        bool res = !(iter == it->end());

        if (res) {
            return iter;
        }
    }

    return JMapInfoIter();
}

JMapInfoIter StageDataHolder::makeCurrentMarioJMapInfoIter() const {
    JMapIdInfo startID = MR::getCurrentMarioStartIdInfo();
    JMapInfoIter iter = makeMarioJMapInfoIter(startID);

    if (iter.isValid()) {
        return iter;
    }

    return JMapInfoIter();
}

void StageDataHolder::initAllLayerJmpInfo(MR::BothDirList< JMapInfo >* pList, const char* pDir) {
    const char* pStageName;
    s32 scenarioNo = MR::getCurrentScenarioNo();
    pStageName = mStageName;
    u32 commonLayers = ScenarioDataFunction::getCurrentCommonLayers(pStageName);
    initSelectedStageLayers(ScenarioDataFunction::getCurrentScenarioLayers(pStageName, scenarioNo), mArchiveHolder,
                            pList, pDir, commonLayers);
}

void StageDataHolder::initTableData() {
    void* tableFile = MR::receiveArchive("/SystemData/ObjNameTable.arc")->getResource('????', "ObjNameTable.tbl");

    mObjNameTbl = new JMapInfo();
    mObjNameTbl->attach(tableFile);
}

void StageDataHolder::initPlacementInfoOrderedCommon() {
    attachCommonToPlacementInfoOrdered(mPlacementInfoOrdered);

    for (s32 i = 0; i < mStageDataHolderCount; i++) {
        mStageDataArray[i]->attachCommonToPlacementInfoOrdered(mPlacementInfoOrdered);
    }

    mPlacementInfoOrdered->sortCommon();
}

void StageDataHolder::attachCommonToPlacementInfoOrdered(PlacementInfoOrderedHolder* pOrdered) const {
    pOrdered->attachCommon(mArchiveHolder->mMapArchive);
    pOrdered->attachCommon(mArchiveHolder->mSoundArchive);
    pOrdered->attachCommon(mArchiveHolder->mDesignArchive);
}

void StageDataHolder::initPlacementInfoOrderedScenario() {
    attachScenarioToPlacementInfoOrdered(mPlacementInfoOrdered);

    for (s32 i = 0; i < mStageDataHolderCount; i++) {
        mStageDataArray[i]->attachScenarioToPlacementInfoOrdered(mPlacementInfoOrdered);
    }

    mPlacementInfoOrdered->sortScenario();
}

void StageDataHolder::attachScenarioToPlacementInfoOrdered(PlacementInfoOrderedHolder* pOrdered) const {
    pOrdered->attachScenario(mArchiveHolder->mMapArchive);
    pOrdered->attachScenario(mArchiveHolder->mSoundArchive);
    pOrdered->attachScenario(mArchiveHolder->mDesignArchive);
}

const JMapInfo* StageDataHolder::findJmpInfoFromArray(const MR::BothDirList< JMapInfo >* pList, const char* pName) const {
    for (MR::BothDirList< JMapInfo >::iterator it = pList->begin(); !it.isEnd(); it++) {
        if (MR::isEqualStringCase(it->getName(), pName)) {
            return &*it;
        }
    }

    return nullptr;
}

JMapInfoIter StageDataHolder::getStartJMapInfoIterFromStartDataIndex(int idx_) const {
    int idx = idx_;

    for (MR::BothDirList< JMapInfo >::iterator it = mStartObjs.begin(); !it.isEnd(); it++) {
        const JMapInfo* pInfo = &*it;
        const JMapData* curData = pInfo->mData;
        int curIdx = curData ? curData->mNumEntries : 0;

        if (idx < curIdx) {
            return JMapInfoIter(pInfo, idx);
        }

        curIdx = curData ? curData->mNumEntries : 0;

        idx -= curIdx;
    }

    for (s32 i = 0; i < mStageDataHolderCount; i++) {
        StageDataHolder* pHolder = mStageDataArray[i];
        int startPosNum = pHolder->getStartPosNum();

        if (idx < startPosNum) {
            return pHolder->getStartJMapInfoIterFromStartDataIndex(idx);
        }

        idx -= startPosNum;
    }

    return JMapInfoIter();
}

void StageDataHolder::calcDataAddress() {
    mDataStart = -1;
    mDataEnd = 0;
    updateDataAddress(&mPlacementObjs);
    updateDataAddress(&mStartObjs);
    updateDataAddress(&mGeneralPosObjs);
    updateDataAddress(&mPathObjs);
}

void StageDataHolder::updateDataAddress(const MR::BothDirList< JMapInfo >* pList) {
    for (MR::BothDirList< JMapInfo >::iterator it = pList->begin(); !it.isEnd(); it++) {
#ifdef GALAXY_NATIVE
        const uintptr_t start = reinterpret_cast<uintptr_t>(it->mData);
        if (start < mDataStart) {
            mDataStart = start;
        }
        uintptr_t addr = (it->mData->mEntrySize * it->mData->mNumEntries) + (start + it->mData->mDataOffset);
#else
        if ((u32)it->mData < mDataStart) {
            mDataStart = (u32)it->mData;
        }

        u32 addr = (it->mData->mEntrySize * it->mData->mNumEntries) + ((s32)it->mData + it->mData->mDataOffset);
#endif

        if (mDataEnd < addr) {
            mDataEnd = addr;
        }
    }
}

void StageDataHolder::createLocalStageDataHolder(const MR::BothDirList< JMapInfo >& rList, bool loadCommon) {
    for (MR::BothDirList< JMapInfo >::iterator it = rList.begin(); !it.isEnd(); it++) {
        if (MR::isEqualStringCase(it->getName(), "StageObjInfo")) {
            for (s32 i = 0; i < it->getNumEntries(); i++) {
                JMapInfoIter iter(&*it, i);
                const char* pName = "";
                MR::getObjectName(&pName, iter);

                if (MR::isValidZoneName(pName)) {
                    s32 zoneId = MR::makeCurrentGalaxyStatusAccessor().getZoneId(pName);
                    mStageDataArray[mStageDataHolderCount] = new StageDataHolder(pName, zoneId, loadCommon);
                    mStageDataArray[mStageDataHolderCount]->initWithoutIter();
                    mStageDataArray[mStageDataHolderCount]->calcPlacementMtx(iter);
                    mStageDataHolderCount++;
                }
            }
        }
    }
}

void StageDataHolder::calcPlacementMtx(const JMapInfoIter& rIter) {
    TVec3f pos;
    rIter.getValue< f32 >("pos_x", &pos.x);
    rIter.getValue< f32 >("pos_y", &pos.y);
    rIter.getValue< f32 >("pos_z", &pos.z);

    TVec3f rot;
    rIter.getValue< f32 >("dir_x", &rot.x);
    rIter.getValue< f32 >("dir_y", &rot.y);
    rIter.getValue< f32 >("dir_z", &rot.z);

    MR::makeMtxTR(mPlacementMtx, pos, rot);
}

namespace MR {
    StageDataHolder* getStageDataHolder() {
        return getSceneObj< StageDataHolder >(SceneObj_StageDataHolder);
    }
};  // namespace MR

StageDataHolder::~StageDataHolder() {
}
template void MR::Vector< MR::FixedArray< const char*, 16 > >::push_back(const char* const&);
