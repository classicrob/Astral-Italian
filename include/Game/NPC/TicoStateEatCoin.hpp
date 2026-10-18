#pragma once

#include "Game/NPC/TicoStateBase.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class EatDemoCoin;
class TicoStateEatCoin : public TicoStateBase {
public:
    TicoStateEatCoin(NPCActor*, s32);
    virtual ~TicoStateEatCoin();
    virtual void appear();
    virtual void control();
    bool isComplete() const;
    void exeTurn();
    void exeEat();
    void exeWait();
    void exeComplete();
    s32 getSoundLevel() const;
    void updateAbsorbSound();

    /* 0x10 */ NPCActor* mActor;
    /* 0x14 */ EatDemoCoin** mCoins;
    /* 0x18 */ s32 mRequiredCoins;
    /* 0x1C */ TVec3f mMouthPosition;
    /* 0x28 */ s32 mGrowthStage;
    /* 0x2C */ s32 mAbsorbSoundCooldown;
};
