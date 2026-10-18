#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class LiveActorGroup;

class ExterminationSwitch : public LiveActor {
public:
    ExterminationSwitch(const char*);

    virtual ~ExterminationSwitch();
    virtual void init(const JMapInfoIter&) override;
    void exeWait();
    void exeCheck();
    void exeAppear();
    void exeOn();

    /* 0x90 */ LiveActorGroup* mGroup;
    /* 0x94 */ s32 mCheckerType;  // 1 = "ExterminationGeneralChecker" star variant, 0 = plain
    /* 0x98 */ s32 mStep;
    /* 0x9C */ bool mIsShowCount;
};
