#pragma once

#include <revolution.h>

class ActorHitReactionCtrl;
class ActorAnimKeeper;
class ActorItemGenerator;
class FootPrint;

// PLACEHOLDER(core): only +0x8 (FootPrint*) is known (asm forceClearFootPrint); rename when owner is ported
class ActorFootPrintHolder {
public:
    /* 0x00 */ u8 mUnk00[0x8];
    /* 0x08 */ FootPrint* mFootPrint;
};

class ActorActionKeeper {
public:
    bool startAction(const char*);
    void initFlagCtrl();
    void updateAnimKeeper();
    void update();

    /* 0x00 */ u8 mUnk00[4];
    /* 0x04 */ ActorAnimKeeper* mAnimKeeper;
    /* 0x08 */ u8 mUnk08[8];
    /* 0x10 */ ActorHitReactionCtrl* mHitReactionCtrl;
    /* 0x14 */ ActorItemGenerator* mItemGenerator;
    /* 0x18 */ u8 mUnk18[0x8];
    /* 0x20 */ ActorFootPrintHolder* mFootPrintHolder;
};
