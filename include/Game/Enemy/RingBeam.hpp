#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ShadowVolumeDrawer.hpp"

class ModelObj;
class RingBeamShadowDrawer;

class RingBeam : public LiveActor {
public:
    RingBeam(const char*, LiveActor*, bool, bool, bool, bool, const char*);

    virtual ~RingBeam() {};
    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void kill();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

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
    /* 0xB0 */ TVec3f _a8;
    /* 0xBC */ TVec3f _b4;
    /* 0xC8 */ TVec3f _c0;
    /* 0xD4 */ TVec3f _cc;
    /* 0xE0 */ s32 _e0;
    /* 0xE4 */ TVec3f _e4;
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
