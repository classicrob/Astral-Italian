#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"

class FallDeath : public LiveActor {
public:
    FallDeath(const char* pName) : LiveActor(pName) {
    }

    FallDeath(LiveActor*, const char*, f32, const f32*);

    virtual ~FallDeath();
    virtual void init(const JMapInfoIter&);

    /* 0x90 */ s32 _90;
    /* 0x94 */ const f32* mFallAccel;
};

namespace NrvFallDeath {
    class FallDeathNrvFall : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;

        static FallDeathNrvFall sInstance;
    };
};  // namespace NrvFallDeath
