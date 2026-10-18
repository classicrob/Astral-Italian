#pragma once

#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/LiveActor.hpp"

class GravityFallBlock : public LiveActor {
public:
    GravityFallBlock(const char*);

    virtual ~GravityFallBlock();
    virtual void init(const JMapInfoIter&) override;
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) override;
    void exeWait();
    void exeFall();
    void exeBreak();

    /* 0x90 */ BreakModel* mBreakModel;
    /* 0x94 */ TVec3f mBasePos;
    /* 0xA0 */ TVec3f mUpDir;
};
