#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ModelObj;
class MogucchiHill;

class Mogucchi : public LiveActor {
public:
    void updateSpinCounterCooldown() NO_INLINE;
    Mogucchi(const char*);
    virtual ~Mogucchi();

    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void makeActorDead();
    virtual void control();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeStroll();
    void endStroll();
    void exeAppearDown();
    void exeDive();
    void exeScatter();
    void exeDie();
    void exeDown();
    void initSensor();
    void createMogucchiHill();
    void createHole();
    void updatePosition();
    void updateReferenceMtx();
    bool checkHipDrop() const;
    void updateStrollSpeed();
    void reflectStarPointer2P();
    void makeEulerRotation();
    void calcAttackDir(TVec3f*, const TVec3f&, const TVec3f&) const;
    bool receiveAttackBySpinSensor(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    bool receiveAttackByBodySensor(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    void calcScatterVec(const TVec3f&, const TVec3f&);

    /* 0x90 */ MogucchiHill* mHill;
    /* 0x94 */ ModelObj* mHole;
    /* 0x98 */ TPos3f mNewHolePos;
    /* 0xC8 */ TVec3f mRailGravity;
    /* 0xD4 */ bool mGrounded;
    /* 0xD8 */ f32 mScatterPropulsionSpeed;
    /* 0xDC */ TVec3f mScatterNormal;
    /* 0xE8 */ f32 mUnusedE8;
    /* 0xEC */ f32 mStrollSpeed;
    /* 0xF0 */ f32 mMaxStrollSpeed;
    /* 0xF4 */ s32 mSpinCounterCooldown;
    /* 0xF8 */ bool mIsStoppedByP2;
};
