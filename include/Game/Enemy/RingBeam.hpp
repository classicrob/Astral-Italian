#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ShadowVolumeDrawer.hpp"

class ActorStateParamScale;
class ModelObj;
class RingBeamShadowDrawer;

class RingBeam : public LiveActor {
public:
    RingBeam(const char*, LiveActor*, bool, bool, bool, bool, const char*);

    virtual ~RingBeam() {};
    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void movement();
    virtual void control();
    virtual void makeActorDead();
    virtual void kill();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void initWithParamScale(const JMapInfoIter&, const ActorStateParamScale*);
    void fn_801F6990(const char*);
    void fn_801F69E0(f32);
    void fn_801F6A80(f32);
    void fn_801F6DB0(f32);
    void fn_801F6DC0(f64);
    void setSpeed(f32 speed);
    void setLife(s32 life);
    void setColor(f32 color);
    void setRadius(f32 radius);
    void initPos(const LiveActor* actor);
    void startBrk(const char*);
    void exeSpread();

    /* 0x90 */ LiveActor* _8c;
    /* 0x94 */ ModelObj* mBloomModel;
    /* 0x98 */ const char* mBeamName;
    /* 0x9C */ f32 mSpeed;
    /* 0xA0 */ s32 mLife;
    /* 0xA4 */ bool _9c;
    /* 0xA5 */ bool _9d;
    /* 0xA6 */ bool _9e;
    /* 0xA7 */ bool _9f;
    /* 0xA8 */ RingBeamShadowDrawer* _a0;
    /* 0xAC */ ModelObj* _a4;
    ActorStateParamScale* mParamScale;
    /* 0xB4 */ TVec3f _a8;
    /* 0xBC */ TVec3f _b4;
    /* 0xC8 */ TVec3f _c0;
    /* 0xD4 */ TVec3f _cc;
    f32 mColor;
    f32 mStartRadius;
    f32 mMaxRadius;
};

class RingBeamShadowDrawer : public ShadowVolumeDrawer {
public:
    RingBeamShadowDrawer(const LiveActor*);

    virtual ~RingBeamShadowDrawer() {};
    virtual void loadModelDrawMtx() const;
    virtual void drawShape() const;
    virtual bool isDraw() const;

    const LiveActor* _1c;
    f32 _20;
};
