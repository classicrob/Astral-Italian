#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ActorCameraInfo;
class Coin;

class CoinGroup : public LiveActor {
public:
    CoinGroup(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void appear();

    virtual void initCoinCount(const JMapInfoIter&);
    virtual void initCoinArray(const JMapInfoIter&) = 0;
    virtual void placementCoin();
    virtual const char* getCoinName() const;

    void killCoinAll();
    void appearCoinAll();
    void appearCoinFix();
    void appearCoinAllTimer();
    void setCoinTrans(s32, const TVec3f&);
    void exeAppear();
    void exeTryStartDemo();
    void exeWait();
    void exeDemoAppear();
    void exeKill();

    /* 0x90 */ Coin** mCoinArray;
    /* 0x94 */ ActorCameraInfo* mCameraInfo;
    /* 0x98 */ u32 mCoinCount;
    /* 0x9C */ s32 mTimeLimit;
    /* 0xA0 */ bool mIsPurpleCoinGroup;
};
