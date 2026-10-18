#pragma once

#include "Game/NPC/NPCActor.hpp"

class CollisionParts;
class ModelObj;

class HoneyQueen : public NPCActor {
public:
    /// @brief Creates a new `HoneyQueen`.
    /// @param pName A pointer to the null-terminated name of the object.
    HoneyQueen(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void calcAnim();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeReady();

    /* 0x15C */ ModelObj* mWing;
    /* 0x168 */ CollisionParts* mCollisionParts[10];
};
