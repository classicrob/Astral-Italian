#pragma once

#include "Game/NameObj/NameObj.hpp"

class ChangeObj;

class ChangeObjHolder : public NameObj {
public:
    ChangeObjHolder(const char* pName);

    virtual ~ChangeObjHolder();
    virtual void init(const JMapInfoIter&);

    void registerChangeObj(ChangeObj*, const JMapInfoIter&);

    /* 0x14 */ ChangeObj** mChangeObjs;
    /* 0x18 */ s32 mCapacity;
    /* 0x1C */ s32 mCount;
};

namespace MR {
    void registerChangeObj(ChangeObj*, const JMapInfoIter&);
};
