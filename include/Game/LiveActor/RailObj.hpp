#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class RailObj : public LiveActor {
public:
    RailObj(const char* pName);

    virtual ~RailObj();
    virtual void init(const JMapInfoIter&);

    /* 0x90 */ s32 mArg0;
    /* 0x94 */ s32 mArg1;
    /* 0x98 */ s32 mArg2;
    /* 0x9C */ s32 mArg3;
    /* 0xA0 */ s32 mArg4;
    /* 0xA4 */ s32 mArg5;
    /* 0xA8 */ s32 mArg6;
    /* 0xAC */ s32 mArg7;
};
