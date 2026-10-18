#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/MapObj/PlacementCircleObj.hpp"
#include "Game/MapObj/PlacementRailObj.hpp"

class Coin;
class ActorCameraInfo;

// SMG2-only unit (written from the target asm). Offsets are real SMG2 offsets.
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
    /* 0x94 */ PlacementObj** mChildren;
    /* 0x98 */ s32 mChildCount;
    /* 0x9C */ Coin** mCoins;
    /* 0xA0 */ s32 mCoinCount;
    /* 0xA4 */ s32 mTime;
};
