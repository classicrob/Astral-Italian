#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class JAISoundID;

class SoundEmitter : public LiveActor {
public:
    SoundEmitter(const char*);

    virtual ~SoundEmitter();
    virtual void init(const JMapInfoIter&);
    virtual void control();

    void exePlaySound();
    void exeStandBy();

    /* 0x90 */ s32 _8C;
    /* 0x94 */ s32 _94;
    /* 0x98 */ bool _98;
    /* 0x99 */ bool _99;
    /* 0x9A */ bool _9A;
};
