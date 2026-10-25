#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"

class EatDemoCoin : public LiveActor {
public:
    EatDemoCoin(const char*);
    virtual ~EatDemoCoin();
    virtual void calcAndSetBaseMtx();
    void shoot(const TVec3f&, const TVec3f*);
    void exeShoot();

    TVec3f mStartPosition;
    const TVec3f* mTargetPosition;
    TQuat4f mQuat;
};
