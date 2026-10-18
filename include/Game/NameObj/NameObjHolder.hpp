#pragma once
#include <revolution.h>
#include "Game/Util/Array.hpp"

class NameObj;

typedef void (NameObj::*func)(void);
typedef void (NameObj::*func_const)(void) const;
typedef void (NameObj::*NameObjMethod)(void);
typedef void (NameObj::*NameObjMethodConst)(void) const;

class NameObjHolder {
public:
    NameObjHolder(int);

    void add(NameObj*);
    void suspendAllObj();
    void resumeAllObj();
    void syncWithFlags();
    void callMethodAllObj(func);
    void clearArray();

    NameObj* find(const char*);

    NameObj** mObjs;  // 0x00
    u32 mMaxObjs;     // 0x04
    s32 mObjCount;    // 0x08
    MR::Vector< MR::FixedArray< NameObj*, 16 > > mObjArray2;  // 0x0C
};
