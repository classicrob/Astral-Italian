#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"

class MapPartsRotator;

/// @brief UnbabaSlim (SMG2 only, no SMG1 source). Rotating asteroid-like body that eats things near the player (volume-drawn range).
class UnbabaSlim : public LiveActor {
public:
    UnbabaSlim(const char* pName);
    virtual ~UnbabaSlim();

    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void initRotator(const JMapInfoIter& rIter);
    void startRotator();
    void endRotator();
    void exeWait();
    void exeEatStart();
    void exeEat();
    void exeEatEnd();

    // Unnamed SMG2 helpers of this unit, named by address (members of UnbabaSlim).
    bool probeGroundAt(const TVec3f* pOffset);
    void updateRotatingBaseMtx();
    void calcHeadDrawMtx();

    /* 0x90 */ MapPartsRotator* mRotator;
    /* 0x94 */ f32 mRotateSpeed;
    /* 0x98 */ VolumeDrawer* mVolumeDrawer;
    /* 0x9C */ TPos3f mMtx;
    /* 0xCC */ s32 mEatCount;
};

#include "Game/LiveActor/Nerve.hpp"
namespace NrvUnbabaSlim {
    class UnbabaSlimNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static UnbabaSlimNrvWait sInstance;
    };
    class UnbabaSlimNrvEatStart : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static UnbabaSlimNrvEatStart sInstance;
    };
    class UnbabaSlimNrvEat : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static UnbabaSlimNrvEat sInstance;
    };
    class UnbabaSlimNrvEatEnd : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static UnbabaSlimNrvEatEnd sInstance;
    };
}
