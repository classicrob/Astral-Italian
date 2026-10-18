#include "Game/MapObj/CoinAppearSpot.hpp"
#include "Game/Util.hpp"
CoinAppearSpot::CoinAppearSpot(const char* name) : LiveActor(name), mCoinCount(8) { mMtx.identity(); }
CoinAppearSpot::~CoinAppearSpot() {}
void CoinAppearSpot::init(const JMapInfoIter& iter) {
    MR::initDefaultPos(this, iter);
    MR::makeMtxTR(mMtx, this);
    MR::connectToSceneMapObjMovement(this);
    MR::setGroupClipping(this, iter, 8);
    MR::setClippingTypeSphere(this, 100.0f);
    MR::getJMapInfoArg0NoInit(iter, &mCoinCount);
    MR::declareCoin(this, mCoinCount);
    MR::useStageSwitchSyncAppear(this, iter);
    makeActorDead();
}
void CoinAppearSpot::appear() {
    LiveActor::appear();
    MR::appearCoinPop(this, mPosition, mCoinCount);
}
MtxPtr CoinAppearSpot::getBaseMtx() const { return (MtxPtr)&mMtx; }
