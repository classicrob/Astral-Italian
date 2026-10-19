#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class WalkerStateBindStarPointer;

class FireBarBall : public LiveActor {
public:
    FireBarBall(LiveActor*);

    virtual ~FireBarBall();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void movement();
    virtual void makeActorAppeared();
    virtual void startClipped();
    virtual void endClipped();

    void exeNerveA();

    /* 0x90 */ LiveActor* mFireBarParent;
    /* 0x94 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0x98 */ bool mIsClipped;
};

class FireBar : public LiveActor {
public:
    FireBar(const char*);

    virtual ~FireBar();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void updateHitSensor(HitSensor*);
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeNerveC();
    void initFireBarBall(const JMapInfoIter&);
    void fixFireBarBall();
    void setSensorToNearestFireBall(HitSensor*);
    bool isAnyFireBallInNerveB() const;

    /* 0x90 */ FireBarBall** mFireBalls;
    /* 0x94 */ s32 mFireBallCount;
    /* 0x98 */ TPos3f* mChildBaseMtx;
    /* 0x9C */ TVec3f mFrontVec;
    /* 0xA8 */ f32 mFireBarSpeed;
    /* 0xAC */ s32 mStickCount;
    /* 0xB0 */ f32 mStickDistance;
    /* 0xB4 */ bool mIsStopped;
    /* 0xB5 */ bool mIsSpherical;  // object name FireBarSpherical
    /* 0xB8 */ f32 mSphericalSpeed;
};
