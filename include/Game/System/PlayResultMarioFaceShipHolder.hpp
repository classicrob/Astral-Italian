#pragma once

#include "Game/NameObj/NameObj.hpp"

class PlayResultMarioFaceShipHolder : public NameObj {
public:
    PlayResultMarioFaceShipHolder();
    virtual ~PlayResultMarioFaceShipHolder();

    void reset();

    /* 0x12 */ bool mIsAfterFileSelect;
    /* 0x13 */ bool mIsRespondGameOver;
    /* 0x14 */ bool mIsSkipBankInfo;
    /* 0x18 */ s32 mClearedScenariosCount;
    /* 0x1C */ bool mFlag1C;
};
