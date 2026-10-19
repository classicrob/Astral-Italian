#pragma once

#include "Game/System/GameEventFlag.hpp"
#include <revolution/types.h>

class GameEventFlagIter {
public:
    bool isEnd() const;
    void goNext();
    const GameEventFlag* getFlag() const;
    bool isValid() const;

    /* 0x00 */ s32 mIter;
};

class GameEventFlagTableInstance {
public:
    struct Key {
        u16 mHashCode;
        const GameEventFlag* mFlag;
    };

    GameEventFlagTableInstance();

    void initSortTable();
    const GameEventFlag* findFlag(const char*);

    /* 0x00 */ Key* mSortTable;
    /* 0x04 */ s32 mLength;
};

namespace GameEventFlagTable {
    GameEventFlagIter getBeginIter();
    GameEventFlagAccessor makeAccessor(const char*);
    s32 getTableSize();
    const GameEventFlag* getFlag(int);
    const GameEventFlag* findFlag(const char*);
    bool isExist(const char*);
    int getIndex(const GameEventFlag*);
    int getIndexFromHashCode(u16);
};  // namespace GameEventFlagTable
