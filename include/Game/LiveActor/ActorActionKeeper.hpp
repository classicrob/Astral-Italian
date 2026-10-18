#pragma once

#include <revolution.h>

class ActorItemGenerator;

class ActorActionKeeper {
public:
    void initFlagCtrl();
    void updateAnimKeeper();
    void update();

    /* 0x00 */ u8 mUnk00[0x14];  // layout before 0x14 not yet mapped
    /* 0x14 */ ActorItemGenerator* mItemGenerator;
};
