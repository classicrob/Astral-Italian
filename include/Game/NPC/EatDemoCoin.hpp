#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"

class EatDemoCoin : public LiveActor {
public:
    EatDemoCoin(const char*);
    virtual void calcAndSetBaseMtx();
    void shoot(const TVec3f&, const TVec3f*);
    void exeShoot();

    TVec3f mStartPosition;
    const TVec3f* mTargetPosition;
    TQuat4f mQuat;
};

namespace NrvEatDemoCoin {
    class EatDemoCoinNrvShoot : public Nerve {
    public:
        EatDemoCoinNrvShoot();
        virtual void execute(Spine*) const;
        static EatDemoCoinNrvShoot sInstance;
    };
}
