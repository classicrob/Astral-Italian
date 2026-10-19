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
    void startSpreadBck(const char*);
    void setRingBckFrame(f32);
    void setColorFrame(f32);
    void setStartRadius(f32);
    void setMaxRadius(f64);
    void setSpeed(f32 speed);
    void setLife(s32 life);
    void setColor(f32 color);
    void setRadius(f32 radius);
    void initPos(const LiveActor* actor);
    void startBrk(const char*);
    void exeSpread();

    /* 0x90 */ LiveActor* mHost;
    /* 0x94 */ ModelObj* mBloomModel;
    /* 0x98 */ const char* mBeamName;
    /* 0x9C */ f32 mSpeed;
    /* 0xA0 */ s32 mLife;
    /* 0xA4 */ bool mFollowGravity;
    /* 0xA5 */ bool mUseShadowModel;
    /* 0xA6 */ bool mUseBloomModel;
    /* 0xA7 */ bool mUseColorBtp;
    /* 0xA8 */ RingBeamShadowDrawer* mShadowDrawer;
    /* 0xAC */ ModelObj* mShadowModel;
    ActorStateParamScale* mParamScale;
    /* 0xB4 */ TVec3f mFront;
    /* 0xC0 */ TVec3f mGravityDirection;
    /* 0xCC */ TVec3f mBeamPos;
    /* 0xD8 */ TVec3f mSoundPos;
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

    /* 0x24 */ const LiveActor* mHost;
    /* 0x20 */ f32 mRadius;
};
