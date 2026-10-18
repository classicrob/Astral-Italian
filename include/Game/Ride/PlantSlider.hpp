#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/MapObj/PlantPoint.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class CameraTargetMtx;
class JUTTexture;
class LiveActorGroup;
class PlantRailInfo;

class PlantSliderPoint : public PlantPoint {
public:
    PlantSliderPoint(const TVec3f&, const TVec3f&, const TVec3f&, f32);

    void updateSpring(f32, f32);
    void updateSprings(f32, f32, f32, f32);

    /* 0x38 */ TVec3f mBasePos;
    /* 0x44 */ TVec3f mVelocity;
    /* 0x50 */ TVec3f mTargetPos;
};

class PlantSlider : public LiveActor {
public:
    PlantSlider(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual void updateHitSensor(HitSensor*);
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual void draw() const;
    virtual ~PlantSlider();

    void exeWait();
    void exeCooldown();
    void exeStart();
    void exeSlide();

    bool tryReachGoal();
    void updateSlide();
    static TVec3f* getPointUp(PlantRailInfo*, s32);
    void updatePoints();
    void updateSpringPoints();
    void initDrawSetting() const;

    /* 0x90 */ s32 mNumPoints;
    /* 0x94 */ PlantSliderPoint** mSliderPoints;
    /* 0x98 */ PlantRailInfo* mRailInfo;
    /* 0x9C */ LiveActor* mRider;
    /* 0xA0 */ f32 mRailCoord;
    /* 0xA4 */ TVec3f mRailDirection;
    /* 0xB0 */ TVec3f mUp;
    /* 0xBC */ TVec3f mSide;
    /* 0xC8 */ f32 mSlideSpeed;
    /* 0xCC */ f32 mRideSpeed;
    /* 0xD0 */ f32 mRideAccel;
    /* 0xD4 */ f32 mStartDelay;
    /* 0xD8 */ f32 mAppearSpeed;
    /* 0xDC */ ActorCameraInfo* mCameraInfo;
    /* 0xE0 */ CameraTargetMtx* mCameraTargetMtx;
    /* 0xE4 */ bool mPlayAppearDemo;
    /* 0xE8 */ f32 mSlideSpeedFactor;
    /* 0xEC */ s32 mNumFlowers;
    /* 0xF0 */ LiveActorGroup* mFlowerGroup;
    /* 0xF4 */ s32 mNumLeaves;
    /* 0xF8 */ LiveActorGroup* mLeafGroup;
    /* 0xFC */ TPos3f* mLeafMtxs;
    /* 0x100 */ JUTTexture* mTexture;
    /* 0x104 */ TVec3f mClippingCenter;
    /* 0x110 */ bool mIsNoClipping;
};
