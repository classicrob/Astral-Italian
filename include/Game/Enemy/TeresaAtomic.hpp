#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class PartsModel;

class TeresaAtomic : public LiveActor {
public:
    // SMG2-only functions (original names unknown; named by address)
    bool fn_8020F940();
    bool fn_8020FB50();
    TeresaAtomic(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void kill();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual void control();
    virtual void calcAndSetBaseMtx();

    bool isEnableAttack() const;
    bool isEnablePush() const;
    bool isEnableStarPieceAttack() const;
    bool tryDisappear();
    bool tryDead();
    void exeAppear();
    void exeWait();
    void exeChase();
    void exeShay();
    void exeAggressive();
    void exeLaugh();
    void exeDead();
    void exeDisappear();

    /* 0x90 */ LiveActor* mClipAreaDrop;
    /* 0x94 */ TVec3f mFrontVec;
    /* 0xA0 */ f32 mSpeed;
    /* 0xA4 */ s32 mPowerStarIndex;
    /* 0xA8 */ PartsModel** mChildParts;
    /* 0xAC */ PartsModel* mPartsHost;
    /* 0xB0 */ s32 mChildCount;
};
