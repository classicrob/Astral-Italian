#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class YoshiNest;
class ModelObj;
class ActorCameraInfo;

class YoshiEgg : public LiveActor {
public:
    YoshiEgg(const char*);
    virtual ~YoshiEgg();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void kill();
    virtual void appear();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual void calcAndSetBaseMtx();
    void fn_80416530();
    void fn_804165D0();
    void fn_80416680();
    void fn_80416720();
    void fn_80416880();
    void fn_80416970();
    void fn_80416A00();
    void fn_80416B30();
    void fn_80416C30();
    void fn_80416D30();
    void fn_80416DB0();
    void fn_80416E60();
    void fn_80416EE0();
    void fn_80416F60();
    void fn_80416FC0();
    static void fn_80417040();
    bool fn_804172C0() const;
    void fn_80417370();
    void fn_804173F0();
    bool fn_80417450() const;
    void fn_80417470();
    void meetYoshiDemo();

    YoshiNest* mNest;
    ModelObj* mShell;
    TVec3f mInitialPosition;
    TVec3f mInitialRotation;
    bool mSwitchEnabled;
    ActorCameraInfo* mCameraInfo;
    bool mCameraActive;
    s32 mCameraFrames;
    bool mShouldAppear;
};
