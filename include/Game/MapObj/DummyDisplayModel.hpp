#pragma once

#include "Game/LiveActor/PartsModel.hpp"

class LodCtrl;
class NameObjArchiveListCollector;

#define ITEM_TYPE_COIN 0
#define ITEM_TYPE_KINOPIO 1
#define ITEM_TYPE_STARPIECE 4
#define ITEM_TYPE_POWER_STAR 7
#define ITEM_TYPE_GRAND_STAR 13

struct DummyDisplayModelInfo {
    /* 0x00 */ const char* mName;
    /* 0x04 */ Vec _4;
    /* 0x10 */ u32 _10;
    /* 0x14 */ const char* mAnim;
    /* 0x18 */ bool mHasColorChange;
};

class DummyDisplayModel : public PartsModel {
public:
    DummyDisplayModel(LiveActor* pHost, const DummyDisplayModelInfo* pInfo, int, s32, s32);

    virtual ~DummyDisplayModel();
    virtual void init(const JMapInfoIter& rIter);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();

    /* 0xA0 */ const DummyDisplayModelInfo* mModelInfo;
    /* 0xA4 */ s32 mItemType;
    /* 0xA8 */ s32 _A4;
    /* 0xAC */ LodCtrl* _A8;
    /* 0xB0 */ bool _AC;
};

namespace MR {
    DummyDisplayModel* tryCreateDummyModel(LiveActor* pHost, const JMapInfoIter& rIter, s32, int);
    DummyDisplayModel* createDummyDisplayModel(LiveActor* pHost, const JMapInfoIter& rIter, s32, const TVec3f&, const TVec3f&);
    DummyDisplayModel* createDummyDisplayModel(LiveActor* pHost, const JMapInfoIter& rIter, const char*, s32, const TVec3f&, const TVec3f&);
    DummyDisplayModel* createDummyDisplayModel(LiveActor* pHost, s32, const char*, const TVec3f&, const TVec3f&);
    DummyDisplayModel* createDummyDisplayModelCrystalItem(LiveActor* pHost, const JMapInfoIter& rIter, const TVec3f&, const TVec3f&);
    DummyDisplayModel* createDummyDisplayModelCrystalItem(LiveActor* pHost, s32, const TVec3f&, const TVec3f&);
    s32 getDummyDisplayModelId(const JMapInfoIter&, s32);
    s32 getDummyDisplayModelId(const LiveActor*);
}  // namespace MR

// Merged from Petari (core lane): declarations missing here
class NameObjArchiveListCollector;
class JMapInfoIter;

namespace MR {
    void makeArchiveListDummyDisplayModel(NameObjArchiveListCollector*, const JMapInfoIter&);
};  // namespace MR
