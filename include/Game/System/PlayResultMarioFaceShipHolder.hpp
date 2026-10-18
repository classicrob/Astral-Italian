#pragma once

#include "Game/NameObj/NameObj.hpp"

class PlayResultMarioFaceShipHolder : public NameObj {
public:
    PlayResultMarioFaceShipHolder();
    virtual ~PlayResultMarioFaceShipHolder();

    void reset();

    /* 0x12 */ bool mUnk12;
    /* 0x13 */ bool mUnk13;
    /* 0x14 */ bool mUnk14;
    /* 0x18 */ s32 mUnk18;
    /* 0x1C */ bool mUnk1C;
};
