#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class TubeSlider;
class AnimScaleController;
class WalkerStateBindStarPointer;
class TubeSliderEnemy : public LiveActor {
public:
    TubeSliderEnemy(TubeSlider*);
    virtual ~TubeSliderEnemy();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    void exeWait();
    void exeWalk();
    void exeSupportFreeze();
    TubeSlider* mSlider;
    TVec3f mFront;
    f32 mRailCoord;
    f32 mAngle;
    f32 mSpeed;
    AnimScaleController* mScaleController;
    WalkerStateBindStarPointer* mBindState;
    bool mPlayerPassed;
    f32 mPrevMicDistance;
};
