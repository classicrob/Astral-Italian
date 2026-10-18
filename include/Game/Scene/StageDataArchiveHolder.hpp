#pragma once

#include "Game/Util/Array.hpp"
#include "Game/Util/BothDirList.hpp"

class JKRArchive;
class JMapInfo;

// Per-directory stage archive object returned by StageDataArchiveHolder::mountArchive (0xE8 bytes).
// Only the three name lists used by the StageDataArchiveHolder constructor are laid out so far.
class StageDataArchive {
public:
    StageDataArchive(JKRArchive*, const char*, const char*);   // SMG2 0x8045AE00

    void initJmpInfo(MR::BothDirList< JMapInfo >* pList, const char* pDir);   // SMG2 0x8045AE50
    bool isNameListed(const char* pName) const;   // fn_8045AFA0: true if NameList0 is empty or contains pName
    static JMapInfo* createUseResourceJMapInfo(const char* pName);   // fn_8045AD60

    /* 0x00 */ JKRArchive* mArchive;
    /* 0x04 */ MR::Vector< MR::FixedArray< const char*, 16 > > mNameList0;
    /* 0x48 */ MR::Vector< MR::FixedArray< const char*, 16 > > mNameList1;
    /* 0x8C */ MR::Vector< MR::FixedArray< const char*, 16 > > mNameList2;
    /* 0xD0 */ MR::BothDirList< JMapInfo > mCommonInfos;
    /* 0xDC */ MR::BothDirList< JMapInfo > mScenarioInfos;
};

// SMG2 stage archive holder. Mounts the Map / Sound / Design / ZoneInfo / UseResource / Ghost archives of one stage.
class StageDataArchiveHolder {
public:
    StageDataArchiveHolder(const char*);

    StageDataArchive* mountArchive(const char*, const char*);   // SMG2 0x8045B530 (defined with StageDataHolder)
    void initJmpInfo(MR::BothDirList< JMapInfo >* pList, const char* pDir);
    void initLayerJmpInfo(MR::BothDirList< JMapInfo >* pList, const char* pDir, u32 layerMask);
    void initCommonLayerJmpInfo(const char* pDir, u32 layerMask);
    void initScenarioLayerJmpInfo(const char* pDir, u32 layerMask);

    /* 0x00 */ StageDataArchive* mMapArchive;
    /* 0x04 */ StageDataArchive* mSoundArchive;
    /* 0x08 */ StageDataArchive* mDesignArchive;
    /* 0x0C */ StageDataArchive* mZoneInfoArchive;
    /* 0x10 */ StageDataArchive* mUseResourceArchive;
    /* 0x14 */ StageDataArchive* mGhostArchive;
};
