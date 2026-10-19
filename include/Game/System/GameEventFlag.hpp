#pragma once

#include <revolution/types.h>

// SMG2: the flag table is just an array of flag names (stride 4); flag type/star data moved elsewhere.
struct GameEventFlag {
    /* 0x00 */ const char* mName;
};

class GameEventFlagAccessor {
public:
    GameEventFlagAccessor(const GameEventFlag*);

    const char* getName() const;

    /* 0x00 */ const GameEventFlag* mFlag;
};
