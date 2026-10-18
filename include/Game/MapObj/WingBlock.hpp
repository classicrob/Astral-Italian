#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class WingBlock : public LiveActor {
public:
    WingBlock(const char*);

    virtual ~WingBlock();
    virtual void init(const JMapInfoIter&) override;
    virtual void control() override;
    virtual void calcAndSetBaseMtx() override;
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) override;
    void initItem(const JMapInfoIter&);
    void appearItem();
    void exeFly();
    void exeHit();

    /* 0x90 */ TQuat4f mQuat;
    /* 0xA0 */ TVec3f mRailPos;
    /* 0xAC */ f32 mFlySpeed;
    /* 0xB0 */ s32 mItemType;
};
