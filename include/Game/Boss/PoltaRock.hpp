#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class Polta;
class WalkerStateBindStarPointer;

class PoltaRock : public LiveActor {
public:
    PoltaRock(const char*);

    virtual ~PoltaRock();
    virtual void init(const JMapInfoIter& rIter);
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void start(Polta*, const TVec3f&, const TVec3f&);
    void setColorWhite();
    void setColorBlack();
    void setColorYellow();
    void exeFloat();
    void exeSign();
    void exeMove();
    void generateKill();
    bool tryStartPointBind(bool);
    void exeBindStarPointer();
    bool isEnableAttack() const NO_INLINE;
    bool isEnableExplosion() const;

    inline Polta* getOwner() {
        return mOwner;
    }  // required to match instruction order in generateKill

    /* 0x8C */ Polta* mOwner;
    WalkerStateBindStarPointer* mPointerBindState;
    TQuat4f mPoseQuat;
    TVec3f mChaseDirection;
    TVec3f mOwnerLocalPos;
    /* 0xC0 */ s32 mType;
};
