#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class HanachanBig;

class HanachanBigParts : public LiveActor {
public:
    enum PartsType {
        PartsType_Head,
        PartsType_Body,
        PartsType_BodyS,
    };

    HanachanBigParts(HanachanBig*, s32, const char*);

    virtual ~HanachanBigParts();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void exeWalk();
    void becomeAngry();

    /* 0x90 */ HanachanBig* mHost;
    /* 0x94 */ AnimScaleController* mScaleController;
    /* 0x98 */ TVec3f mRailDirection;
    /* 0xA4 */ s32 mPartsType;
    /* 0xA8 */ s32 mPartsIndex;
    /* 0xAC */ bool mIsAngry;
};

class HanachanBig : public LiveActor {
public:
    HanachanBig(const char*);

    virtual ~HanachanBig();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeHit();
    void exeStandby();
    void moveAlongRail(f32);
    bool isStarPointerPointing();

    /* 0x90 */ AnimScaleController* mScaleController;
    /* 0x94 */ HanachanBigParts* mParts[13];
    /* 0xC8 */ s32 mPartsCount;
    /* 0xCC */ f32 mRange;
    /* 0xD0 */ s32 mHitCount;
    /* 0xD4 */ bool mIsAngry;
};
