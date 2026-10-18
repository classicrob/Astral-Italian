#pragma once
#include "Game/LiveActor/ActorStateBase.hpp"
class NPCActor;
class LiveActor;
class EatDemoCoin;
class TicoStateEatCoin : public ActorStateBase<NPCActor> {
public:
    TicoStateEatCoin(NPCActor*, s32);
    bool isComplete() const;
    LiveActor* mActor;
    EatDemoCoin** mCoins;
    s32 mRequiredCoins;
    f32 _1C;
    f32 _20;
    f32 _24;
    s32 _28;
    s32 _2C;
};
