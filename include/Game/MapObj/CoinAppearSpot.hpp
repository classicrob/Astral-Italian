#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class CoinAppearSpot : public LiveActor {
public:
    CoinAppearSpot(const char*);
    virtual ~CoinAppearSpot();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual MtxPtr getBaseMtx() const;
    TPos3f mMtx;
    s32 mCoinCount;
};
