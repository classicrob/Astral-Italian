#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/Functor.hpp"

class Coin;
class ActorCameraInfo;

// SMG2-only unit (written from the target asm). Offsets are real SMG2 offsets.
// Placement children are CircleCoinGroup (PlacementCircleObj, 0xB0 bytes) and RailCoin (PlacementRailObj, 0x94 bytes).
class PlacementCircleObj : public NameObj {
public:
    PlacementCircleObj();
};

class PlacementRailObj : public NameObj {
public:
    PlacementRailObj();
};

// Group of coins placed by circle/rail coin objects; appears when its switch/demo fires.
class CoinLinkGroup : public LiveActor {
public:
    CoinLinkGroup(const char*);

    virtual ~CoinLinkGroup();
    virtual void init(const JMapInfoIter&);

    void initCoins(const JMapInfoIter&);
    void createCoins();
    void appearCoins();
    void appearCoinsOnSwitch();
    void appearCoinsFixTimer();
    void killCoins();

    void exeDemo();
    void exeMain();

    /* 0x90 */ ActorCameraInfo* mCameraInfo;
    /* 0x94 */ NameObj** mChildren;
    /* 0x98 */ s32 mChildCount;
    /* 0x9C */ Coin** mCoins;
    /* 0xA0 */ s32 mCoinCount;
    /* 0xA4 */ s32 mTime;
};
