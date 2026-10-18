#pragma once
#include "Game/NPC/NPCActor.hpp"
class ActorCameraInfo;
class TicoStateEatCoin;
class TicoFatCoin : public NPCActor {
public:
    TicoFatCoin(const char*);
    virtual ~TicoFatCoin();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();
    bool branchFunc(u32);
    bool eventFunc(u32);
    void exeWait();
    void exeMetamorphosis();
    void exeMove();
    void exeTransform();
    void exeTransformEnd();
    void exeWipe();
    void exeInformation();
    void exeEnd();
    void appearPlanet(bool) NO_INLINE;
    s32 mRequiredCoins;
    ActorCameraInfo* mCameraInfo;
    TicoStateEatCoin* mEatState;
    TPos3f mEffectMtx;
    TPos3f mSmokeMtx;
};
