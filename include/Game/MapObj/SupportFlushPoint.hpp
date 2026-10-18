#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class SupportFlushPoint : public LiveActor {
public:
    SupportFlushPoint(const char*);
    virtual ~SupportFlushPoint();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    void exeAppear();
    void exeHide();
    void exeWait();
    void exeGenerateItem();
    s32 mItemType;
};
