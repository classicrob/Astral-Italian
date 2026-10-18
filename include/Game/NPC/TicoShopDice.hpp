#pragma once
#include "Game/NPC/TicoComet.hpp"
class LuckyDice;
class TicoStateEatCoin;
class TicoShopDice : public TicoEat {
public:
    TicoShopDice(const char*);
    virtual ~TicoShopDice();
    virtual void init(const JMapInfoIter&);
    virtual bool isReactionNerve() const;
    bool branchFunc(u32);
    bool eventFunc(u32);
    void exeDisappear();
    void exeDiceAppear();
    void exeRecover();
    bool mUseCoins;
    s32 mPrice;
    s32 mDiceCount;
    LuckyDice** mDice;
    TicoStateEatCoin* mEatCoinState;
};
