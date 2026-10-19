#pragma once

#include "Game/NPC/NPCActor.hpp"

class FollowCollisionArea;

class SignBoard : public NPCActor {
public:
    /// @brief Creates a new `SignBoard`.
    /// @param pName A pointer to the null-terminated name of the object.
    SignBoard(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void makeActorDead();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait();

    /* 0x164 */ s32 mSupportPlayMode;
    /* 0x168 */ s32 mSupportPlayActive;
};
