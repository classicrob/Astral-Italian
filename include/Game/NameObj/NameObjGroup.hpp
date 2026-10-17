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
        return mObjectCount;
    }

    // Petari-compatible accessors
    NameObj* getObj(int index) const {
        return mObjects[index];
    }

    s32 getObjNum() const {
        return mObjectCount;
    }

    template < typename T >
    T getObj(s32 idx) const {
        return reinterpret_cast< T >(mObjects[idx]);
    }

    // Anonymous unions keep Petari's field names (mObjNumMax/mObjNum/mObjArray) usable.
    union {
        s32 mMaxCount;  // 0x14
        s32 mObjNumMax;
    };
    union {
        s32 mObjectCount;  // 0x18
        s32 mObjNum;
    };
    union {
        NameObj** mObjects;  // 0x1C
        NameObj** mObjArray;
    };
};
