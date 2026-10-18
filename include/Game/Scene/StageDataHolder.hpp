#pragma once

#include "Game/NameObj/NameObj.hpp"
#include "Game/Util/Array.hpp"
#include "Game/Util/BothDirList.hpp"
#include <JSystem/JGeometry.hpp>

class JKRArchive;
class JMapIdInfo;
class JMapInfo;
class JMapInfoIter;
class PlacementInfoOrderedHolder;

class StageDataArchiveHolder;

class StageDataHolder : public NameObj {
public:
    StageDataHolder(const char* pStageName, int zoneID, bool loadCommon);

    virtual ~StageDataHolder();
    virtual void init(const JMapInfoIter& rIter);

    void initAfterScenarioSelected();
    static void requestFileLoadCommon();     // SMG2 0x8045ACA0 (body not written yet: unowned gap before StageDataArchiveHolder)
    static void requestFileLoadScenario();   // SMG2 0x8045ACE0
    void initFileLoadPostCreation();
    void initPlacement();
    JMapInfoIter getCommonPathPointInfo(const JMapInfo** ppInfo, int index) const;
    JMapInfoIter getCommonPathPointInfoFromRailDataIndex(const JMapInfo** ppInfo, int index) const;
    s32 getCommonPathInfoElementNum() const;
    s32 getStartPosNum() const;
    s32 getCurrentStartZoneId() const;
    s32 getCurrentStartCameraId() const;
    void getStartCameraIdInfoFromStartDataIndex(JMapIdInfo* pInfo, int index) const;
    s32 getGeneralPosNum() const;
    JMapInfoIter getGeneralPosInfoFromDataIndex(int index) const;
    s32 getChildObjNum(const JMapInfoIter& rIter) const;
    JMapInfoIter getChildObjInfoFromDataIndex(const JMapInfoIter& rIter, int index) const;
    const StageDataHolder* findPlacedStageDataHolder(const JMapInfoIter& rIter) const;
    const StageDataHolder* getStageDataHolderFromZoneId(int zoneID) const;
    StageDataHolder* getStageDataHolderFromZoneId(int zoneID);
    bool isPlacedZone(int zoneID) const;
    const char* getJapaneseObjectName(const char* pName) const;
    void* getStageArchiveResource(const char* pName);
    s32 getStageArchiveResourceSize(void* pData);
    void* getUseResourceArchiveResource(const char* pName);
    JKRArchive* getGhostArchive();
    void initPlacementMario();
    JMapInfoIter makeMarioJMapInfoIter(const JMapIdInfo& rInfo) const;
    JMapInfoIter makeCurrentMarioJMapInfoIter() const;
    void initAllLayerJmpInfo(MR::BothDirList< JMapInfo >* pList, const char* pDir);
    void initTableData();
    void initPlacementInfoOrderedCommon();
    void initPlacementInfoOrderedScenario();
    void attachCommonToPlacementInfoOrdered(PlacementInfoOrderedHolder* pOrdered) const;
    void attachScenarioToPlacementInfoOrdered(PlacementInfoOrderedHolder* pOrdered) const;
    const JMapInfo* findJmpInfoFromArray(const MR::BothDirList< JMapInfo >* pList, const char* pName) const;
    JMapInfoIter getStartJMapInfoIterFromStartDataIndex(int index) const;
    void calcDataAddress();
    void updateDataAddress(const MR::BothDirList< JMapInfo >* pList);
    void createLocalStageDataHolder(const MR::BothDirList< JMapInfo >& rList, bool loadCommon);
    void calcPlacementMtx(const JMapInfoIter& rIter);

    /* 0x14 */ JMapInfo* mObjNameTbl;
    /* 0x18 */ s32 mStageDataHolderCount;
    /* 0x1C */ StageDataHolder* mStageDataArray[0x18];
    /* 0x7C */ const char* mStageName;
    /* 0x80 */ TPos3f mPlacementMtx;
    /* 0xB0 */ s32 mZoneID;
    /* 0xB4 */ bool mLoadCommon;
    /* 0xB8 */ u32 mDataStart;
    /* 0xBC */ u32 mDataEnd;
    /* 0xC0 */ PlacementInfoOrderedHolder* mPlacementInfoOrdered;
    /* 0xC4 */ StageDataArchiveHolder* mArchiveHolder;
    /* 0xC8 */ MR::BothDirList< JMapInfo > mPlacementObjs;
    /* 0xD4 */ MR::BothDirList< JMapInfo > mStartObjs;
    /* 0xE0 */ MR::BothDirList< JMapInfo > mGeneralPosObjs;
    /* 0xEC */ MR::BothDirList< JMapInfo > mPathObjs;
};

namespace MR {
    StageDataHolder* getStageDataHolder();
};  // namespace MR
