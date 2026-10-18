#pragma once
#include "Game/NPC/NPCActor.hpp"
class MonteThrow;
class JointRumbler;
class Monte : public NPCActor {
public:
    Monte(const char*);
    virtual ~Monte();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    bool tryStartThrow() NO_INLINE;
    void exeWait();
    void exeThrow();
    s32 mType;
    s32 mColor;
    MonteThrow* mThrow;
    JointRumbler* mRumbler;
};
