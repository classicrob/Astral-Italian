#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class MagmaMeteorParts;
class MagmaMeteor : public LiveActor {
public:
    MagmaMeteor(const char*);
    virtual ~MagmaMeteor();
    virtual void init(const JMapInfoIter&);
    virtual void attackSensor(HitSensor*, HitSensor*);
    void exeFirstWait();
    void exeFall();
    void exeSpread();
    void exeWait();
    void exeShrink();
    void exeInterval();
    void resetMeteor();
    void updateMeteor();
    bool isPlayerInFire(HitSensor*) const;
    void expandFire();
    void shrinkFire();
    bool mFireActive;
    f32 mFireRadius;
    TVec3f mMeteorVelocity;
    TVec3f mUp;
    s32 mFirstWait;
    s32 mInterval;
    f32 mHeight;
    s32 mFallTime;
    MagmaMeteorParts* mMeteor;
};
