#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class CollisionParts;
class YoshiDashStep : public LiveActor {
public:
    YoshiDashStep(const char* = "YoshiDashStep");
    virtual ~YoshiDashStep();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual MtxPtr getBaseMtx() const;
    void fn_80415B90(MtxPtr);
    TPos3f mMatrix;
    CollisionParts* mCollision;
};
class YoshiHistory15C50 {
public:
    YoshiHistory15C50();
    void fn_80415CF0(MtxPtr);
    void fn_80415DA0();
    YoshiDashStep* mEntries;
    s32 mIndex;
};
