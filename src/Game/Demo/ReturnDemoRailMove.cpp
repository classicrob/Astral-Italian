#include "Game/Demo/ReturnDemoRailMove.hpp"
#include "Game/Util.hpp"
#include "Game/MapObj/SpinDriverPathDrawer.hpp"
#include "Game/MapObj/SpinDriverShootPath.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    void setResultFlyStartFrame(LiveActor* liveActor, s32 frame) NO_INLINE {
        int maxFrames = MR::getBckFrameMax(liveActor);
        MR::setBckFrame(liveActor, maxFrames - frame % maxFrames);
    }
};  // namespace

ReturnDemoRailMove::ReturnDemoRailMove(LiveActor* pDemoStarter, LiveActor* pPowerStar, const JMapInfoIter& rIter, bool isGrandstar,
                                       TPos3f* pTransform)
    : mDemoStarter(pDemoStarter), mPowerStar(pPowerStar), mIsGrandStar(isGrandstar), mTransform(pTransform), mShootPath(nullptr),
      mPathDrawer(nullptr), mForward(0.0f, 0.0f, 1.0f) {
    mShootPath = new SpinDriverShootPath();
    mShootPath->init(rIter);

    mPathDrawer = new SpinDriverPathDrawer(mShootPath);
    mPathDrawer->initWithoutIter();
    mPathDrawer->setColorNormal();

    calcPathPosDir(nullptr, &mForward, 1.0f);
}

void ReturnDemoRailMove::posToStart() {
    TVec3f position;
    TVec3f forward;
    calcPathPosDir(&position, &forward, 0.0f);

    MR::makeMtxUpFront(getMtx(), forward, TVec3f(0.0f, -1.0f, 0.0f));
    mTransform->setTrans(position);
    MR::setPlayerBaseMtx(*mTransform);
};

void ReturnDemoRailMove::posToEnd() {
    TVec3f position;
    calcPathPosDir(&position, nullptr, 1.0f);

    MR::makeMtxUpFront(getMtx(), TVec3f(0.0f, 1.0f, 0.0f), mForward);
    mTransform->setTrans(position);
    MR::setPlayerBaseMtx(*mTransform);
};

void ReturnDemoRailMove::offPathDraw() {
    mPathDrawer->kill();
};

s32 ReturnDemoRailMove::getDemoFlyBrakeFrame() const {
    return (mIsGrandStar) ? 296 : 45;
};

void ReturnDemoRailMove::calcPathPosDir(TVec3f* position, TVec3f* direction, f32 t) const {
    if (position != nullptr) {
        mShootPath->calcPosition(position, t);
    }

    if (direction != nullptr) {
        mShootPath->calcDirection(direction, t, 0.01f);
    }
};

void ReturnDemoRailMove::setupPathDrawForGraneStarReturnDemo() {
    mPathDrawer->setMaskLength(100.0f);
    mPathDrawer->setFadeScale(0.1f);
};

void ReturnDemoRailMove::start() {
    const char* pBckName = (mIsGrandStar) ? "ResultFlyGrandStar" : "ResultFly";
    MR::startBckPlayer(pBckName);

    MR::startAction(mPowerStar, pBckName);
    mPathDrawer->_B0 = 0.0f;
    mPathDrawer->appear();
};

