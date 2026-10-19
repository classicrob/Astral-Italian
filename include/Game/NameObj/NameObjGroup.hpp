#pragma once

#include "Game/NameObj/NameObj.hpp"

class NameObjGroup : public NameObj {
public:
    NameObjGroup(const char*, int);

    virtual ~NameObjGroup();

    void registerObj(NameObj*);
    void clear();
    bool isFull() const;
    void pauseOffAll() const;
    void initObjArray(int);

    s32 getObjectCount() const {
        return mObjNum;
    }

    // Petari-compatible accessors
    NameObj* getObj(int index) const {
        return mObjArray[index];
    }

    s32 getObjNum() const {
        return mObjNum;
    }

    template < typename T >
    T getObj(s32 idx) const {
        return reinterpret_cast< T >(mObjArray[idx]);
    }

    /* 0x14 */ s32 mObjNumMax;
    /* 0x18 */ s32 mObjNum;
    /* 0x1C */ NameObj** mObjArray;
};
