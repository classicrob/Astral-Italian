#pragma once

#include "Game/NameObj/NameObj.hpp"

class ChangeObj;

// Two linked forms of an object in one placed zone.
class ChangeObjPair : public NameObj {
public:
    bool tryRegister(ChangeObj*, s32 zoneId, s32 linkId);

    /* 0x14 */ s32 mZoneId;
    /* 0x18 */ s32 mFirstLinkId;
    /* 0x1C */ s32 mSecondLinkId;
    /* 0x20 */ ChangeObj* mFirst;
    /* 0x24 */ ChangeObj* mSecond;
};

class ChangeObjHolder : public NameObj {
public:
    ChangeObjHolder(const char* pName);

    virtual ~ChangeObjHolder();
    virtual void init(const JMapInfoIter&);

    void registerChangeObj(ChangeObj*, const JMapInfoIter&);

    /* 0x14 */ ChangeObjPair** mChangeObjs;
    /* 0x18 */ s32 mCapacity;
    /* 0x1C */ s32 mCount;
};

namespace MR {
    void registerChangeObj(ChangeObj*, const JMapInfoIter&);
};
