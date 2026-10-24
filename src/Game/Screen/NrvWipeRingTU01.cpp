#include "Game/Screen/MapGalaxyLayouts.hpp"
#include "Game/Screen/CoinCounter.hpp"
#include "Game/Screen/StarPieceCounter.hpp"
#include "Game/Screen/PlayerLeft.hpp"
#include "Game/Screen/StarCounter.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Util.hpp"
namespace NrvMapGalaxyCounters {
    class MapGalaxyCountersNrvWait : public Nerve { public: virtual void execute(Spine*) const {} static MapGalaxyCountersNrvWait sInstance; };
    MapGalaxyCountersNrvWait MapGalaxyCountersNrvWait::sInstance;
}
MapGalaxyCounters::MapGalaxyCounters() : LayoutActor("ワールドマップカウンタ系レイアウト制御", true), mCoinCounter(0), mStarPieceCounter(0), mPlayerLeft(0), mStarCounter(0), mCoinLifeKeepFrames(0), mStarKeepFrames(0), mStarPieceKeepFrames(0), mLastStarPieceCount(0) {}
void MapGalaxyCounters::init(const JMapInfoIter&) {
    MR::connectToSceneLayout(this);
    initNerve(&NrvMapGalaxyCounters::MapGalaxyCountersNrvWait::sInstance);
    mCoinCounter = new CoinCounter("コインカウンタ");
    mCoinCounter->initWithoutIter();
    mStarPieceCounter = new StarPieceCounter("スターピースカウンタ");
    mStarPieceCounter->initWithoutIter();
    mPlayerLeft = new PlayerLeft("プレイヤー残機表示");
    mPlayerLeft->initWithoutIter();
    mStarCounter = new StarCounter();
    mStarCounter->initWithoutIter();
    MR::registerDemoSimpleCastAll(this);
    MR::registerDemoSimpleCastAll(mCoinCounter);
    MR::registerDemoSimpleCastAll(mStarPieceCounter);
    MR::registerDemoSimpleCastAll(mPlayerLeft);
    MR::registerDemoSimpleCastAll(mStarCounter);
    appear();
}
void MapGalaxyCounters::appear() {
    LayoutActor::appear();
    mStarCounter->kill();
    mCoinCounter->kill();
    mStarPieceCounter->kill();
    mPlayerLeft->kill();
    mLastStarPieceCount = GameDataFunction::getStockedStarPieceNum();
}
void MapGalaxyCounters::kill() {
    LayoutActor::kill();
    if (mCoinLifeKeepFrames) { mPlayerLeft->disappear(); mCoinCounter->disappear(false); }
    if (mStarPieceKeepFrames) mStarPieceCounter->forceDisappear();
    if (mStarKeepFrames) mStarCounter->disappear();
    mStarCounter->kill();
    mCoinCounter->kill();
    mStarPieceCounter->kill();
    mPlayerLeft->kill();
    mCoinLifeKeepFrames = 0;
    mStarPieceKeepFrames = 0;
    mStarKeepFrames = 0;
}
void MapGalaxyCounters::control() {
    if (mCoinCounter->isWait() && mCoinLifeKeepFrames == 0) mCoinCounter->disappear(false);
    if (mPlayerLeft->isWait() && mCoinLifeKeepFrames == 0) mPlayerLeft->disappear();
    if (mStarCounter->isWait() && mStarKeepFrames == 0) mStarCounter->disappear();
    s32 count = GameDataFunction::getStockedStarPieceNum();
    if (mLastStarPieceCount != count) { mStarPieceKeepFrames = 120; mStarPieceCounter->forceAppear(false); }
    else if (mStarPieceKeepFrames == 0) {
        if (mStarPieceCounter->isWait()) mStarPieceCounter->forceDisappear();
        else if (!MR::isDead(mStarPieceCounter) && MR::isHiddenLayout(mStarPieceCounter)) mStarPieceCounter->kill();
    }
    mLastStarPieceCount = count;
    mCoinLifeKeepFrames = MR::max(mCoinLifeKeepFrames - 1, 0);
    mStarKeepFrames = MR::max(mStarKeepFrames - 1, 0);
    mStarPieceKeepFrames = MR::max(mStarPieceKeepFrames - 1, 0);
}
void MapGalaxyCounters::requestAppearCoinsAndLives() {
    bool show;
    mCoinLifeKeepFrames = 4;
    if (MR::isDead(this)) LayoutActor::appear();
    {
    show = false;
    CoinCounter* coin = mCoinCounter;
    if (MR::isDead(coin) || MR::isHiddenLayout(coin)) show = true;
    if (show) mCoinCounter->forceAppear(false);
    }
    {
    show = false;
    PlayerLeft* left = mPlayerLeft;
    if (MR::isDead(left) || MR::isHiddenLayout(left)) show = true;
    if (show) mPlayerLeft->forceAppear();
    }
    requestAppearStars();
    requestAppearStarPieces();
}
void MapGalaxyCounters::requestAppearStars() {
    mStarKeepFrames = 4;
    if (MR::isDead(this)) LayoutActor::appear();
    bool show = false;
    StarCounter* star = mStarCounter;
    if (MR::isDead(star) || MR::isHiddenLayout(star)) show = true;
    if (show) mStarCounter->appear();
}
void MapGalaxyCounters::requestAppearStarPieces() {
    mStarPieceKeepFrames = 4;
    if (MR::isDead(this)) LayoutActor::appear();
    bool show = false;
    StarPieceCounter* piece = mStarPieceCounter;
    if (MR::isDead(piece) || MR::isHiddenLayout(piece)) show = true;
    if (show) mStarPieceCounter->forceAppear(false);
}
MapGalaxyCounters::~MapGalaxyCounters() {}
