#pragma once

#include "Game/Util/BothDirList.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/StringUtil.hpp"

class NameObj;
class StageDataArchive;

typedef NameObj* (*CreationFuncPtr)(const char*);

// SMG2: one ordered list of placement entries (0x18). Entries with the same name/shape id are grouped in a SameIdSet;
// entries owned by a generator (GeneratorID != -1) are kept apart and only get their post-creation call.
class PlacementInfoOrdered {
public:
    class Identifier {
    public:
        Identifier() : mName(), mShapeId(-1) {
        }

        Identifier(const char* pName, s32 shapeId) : mName(pName), mShapeId(shapeId) {
        }

        bool operator==(const Identifier& rOther) const {
            return MR::isEqualString(mName, rOther.mName) && mShapeId == rOther.mShapeId;
        }

        /* 0x00 */ const char* mName;
        /* 0x04 */ s32 mShapeId;
    };

    class Index : public MR::BothDirLink< Index > {
    public:
        Index() : MR::BothDirLink< Index >(this), mInfoIter(JMapInfoIter()) {
        }

        const JMapInfoIter& getInfoIter() const {
            return mInfoIter;
        }

        /* 0x10 */ JMapInfoIter mInfoIter;
    };

    class SameIdSet : public Identifier {
    public:
        SameIdSet() : Identifier(), mList() {
        }

        bool operator<(const SameIdSet& rOther) const {
            if (mPriority == rOther.mPriority) {
                return rOther.mList.mCount < mList.mCount;
            }

            return mPriority < rOther.mPriority;
        }

        /* 0x08 */ s32 mPriority;
        /* 0x0C */ MR::BothDirList< Index > mList;
    };

    PlacementInfoOrdered();

    void attach(const JMapInfo* pInfo, PlacementInfoOrdered* pAfterScenario, const StageDataArchive* pArchive);
    void sort();
    void requestFileLoad();
    void initPlacement();
    void insert(const Identifier&, const JMapInfoIter&);
    SameIdSet* find(const Identifier&) const;
    SameIdSet* createSameIdSet(const Identifier&);
    Index* createIndex(const JMapInfoIter&);
    void addGeneratedIndex(const JMapInfoIter&);   // SMG2 0x804567B0 (outside this unit's split)

    /* 0x00 */ MR::BothDirList< SameIdSet > mSetList;
    /* 0x0C */ MR::BothDirList< JMapInfoIter > mGeneratedList;
};

// SMG2: the five placement lists of a scene (StageDataHolder::mPlacementInfoOrdered).
class PlacementInfoOrderedHolder {
public:
    PlacementInfoOrderedHolder();

    void attachCommon(const StageDataArchive* pArchive);
    void attachScenario(const StageDataArchive* pArchive);
    void sortCommon();
    void sortScenario();
    void requestFileLoad();
    void initPlacementHighPriority();
    void initPlacement();
    void attachInfos(PlacementInfoOrdered* pPriority, PlacementInfoOrdered* pNormal, PlacementInfoOrdered* pAfterScenario,
                     const MR::BothDirList< JMapInfo >* pList, const StageDataArchive* pArchive);

    /* 0x00 */ PlacementInfoOrdered* mCommonPriority;
    /* 0x04 */ PlacementInfoOrdered* mCommon;
    /* 0x08 */ PlacementInfoOrdered* mScenarioPriority;
    /* 0x0C */ PlacementInfoOrdered* mScenario;
    /* 0x10 */ PlacementInfoOrdered* mAfterScenario;
};
