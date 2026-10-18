#pragma once

#include "Game/LiveActor/ModelObj.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class FireBarBall : public ModelObj {
public:
    FireBarBall(LiveActor*);

    virtual ~FireBarBall();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();

    void controlEmitEffect();

    /* 0x90 */ LiveActor* mFireBarParent;
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
    void exeNerveA();  // PLACEHOLDER(objects): nerve A exe, see FireBar.cpp
    void exeNerveB();  // PLACEHOLDER(objects): nerve B exe, see FireBar.cpp
    void exeNerveC();
    void initFireBarBall(const JMapInfoIter&);
    void fixFireBarBall();
    void setSensorToNearestFireBall(HitSensor*);
    bool isAnyFireBallInNerveB() const;

    /* 0x90 */ FireBarBall** mFireBalls;
    /* 0x94 */ s32 mFireBallCount;
    /* 0x98 */ TPos3f* mUnk98;
    /* 0x9C */ TVec3f mFrontVec;
    /* 0xA8 */ f32 mFireBarSpeed;
    /* 0xAC */ s32 mStickCount;
    /* 0xB0 */ f32 mStickDistance;
    /* 0xB4 */ bool mIsStopped;
    /* 0xB5 */ bool mUnkB5;
    /* 0xB8 */ f32 mUnkB8;
};
