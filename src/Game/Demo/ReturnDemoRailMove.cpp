#define JGEOMETRY_OUTLINE_MATRIX_POSITION
#include "Game/Demo/ReturnDemoRailMove.hpp"
#include "Game/Util.hpp"
#include "Game/MapObj/SpinDriverPathDrawer.hpp"
#include "Game/MapObj/SpinDriverShootPath.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Demo/PrepareKoopaFinalDemoObj.hpp"

ReturnDemoRailMove::ReturnDemoRailMove(LiveActor* pDemoStarter, LiveActor* pPowerStar, const JMapInfoIter& rIter, bool isGrandstar,
                                       TPos3f* pTransform)
    : mDemoStarter(pDemoStarter), mPowerStar(pPowerStar), mIsGrandStar(isGrandstar), mTransform(pTransform), mShootPath(nullptr),
      mPathDrawer(nullptr), mForward(0.0f, 0.0f, 1.0f), mGravity(0.0f, -1.0f, 0.0f) {
    mShootPath = new SpinDriverShootPath();
    mShootPath->init(rIter);

    mPathDrawer = new SpinDriverPathDrawer(mShootPath);
    mPathDrawer->initWithoutIter();
    mPathDrawer->setColorNormal();

    TVec3f endPos;
    calcPathPosDir(&endPos, &mForward, 1.0f);
    MR::calcGravityVector(pDemoStarter, endPos, &mGravity, nullptr, 0);
}

void ReturnDemoRailMove::posToStart() {
    TVec3f position;
    TVec3f forward;
    calcPathPosDir(&position, &forward, 0.0f);

    MR::makeMtxUpFront(getMtx(), forward, mGravity);
    mTransform->setTrans(position);
    MR::setPlayerBaseMtx(*mTransform);
};

void ReturnDemoRailMove::posToEnd() {
    TVec3f position;
    calcPathPosDir(&position, nullptr, 1.0f);

    makeEndMtx(getMtx(), mForward);
    mTransform->setTrans(position);
    MR::setPlayerBaseMtx(*mTransform);
};

void ReturnDemoRailMove::start() {
    const char* pBckName = (mIsGrandStar) ? "ResultFlyGrandStar" : "ResultFly";
    MR::startBckPlayer(pBckName);

    MR::startAction(mPowerStar, pBckName);
    mPathDrawer->_B0 = 0.0f;
    mPathDrawer->appear();
};

void ReturnDemoRailMove::offPathDraw() {
    mPathDrawer->kill();
};

void ReturnDemoRailMove::update(s32 currentStep, s32 maxSteps) {
    int startStepFirstDemo = maxSteps - getDemoFlyBrakeFrame();
    int startStepSecondDemo = maxSteps - ((mIsGrandStar) ? 98 : 34);

    f32 progress = (static_cast< f32 >(currentStep) / maxSteps) - 1.0f;
    f32 t = 1.0f - progress * progress;

    if ((startStepFirstDemo < 0 && MR::isFirstStep(mDemoStarter)) || MR::isStep(mDemoStarter, startStepFirstDemo)) {
        const char* pBckName = (mIsGrandStar) ? "ResultFlyGrandStarEnd" : "ResultFlyEnd";

        MR::startBckPlayer(pBckName);
        MR::startAction(mPowerStar, pBckName);

        if (!mIsGrandStar) {
            MR::startSoundPlayer("SE_PM_S_SPIN_DRV_COOL_DOWN", -1, -1);
        }
    }

    if (MR::isFirstStep(mDemoStarter)) {
        if (startStepFirstDemo < 0) {
            startStepFirstDemo = -startStepFirstDemo;
            MR::setBckFrame(MR::getPlayerDemoActor(), startStepFirstDemo);
            MR::setBckFrame(mPowerStar, startStepFirstDemo);
        } else {
            reinterpret_cast<PrepareKoopaFinalDemoObj*>(MR::getPlayerDemoActor())->setRandomBckFrame(startStepFirstDemo);
            reinterpret_cast<PrepareKoopaFinalDemoObj*>(mPowerStar)->setRandomBckFrame(startStepFirstDemo);
        }
    }

    MR::startActionSound(mPowerStar, "OjLvRetPowStarFly", -1, -1, -1);

    TVec3f position;
    TVec3f direction;
    calcPathPosDir(&position, &direction, t);

    MR::makeMtxUpFront(getMtx(), direction, mGravity);

    if (MR::isGreaterStep(mDemoStarter, startStepSecondDemo)) {
        f32 rate = MR::calcNerveEaseOutRate(mDemoStarter, startStepSecondDemo, maxSteps);

        TPos3f transform;
        makeEndMtx(&transform, mForward);
        MR::blendMtxRotateSlerp(*mTransform, transform, rate, *mTransform);
    }

    mTransform->setTrans(position);
    mPathDrawer->setCoord(t);
    MR::setPlayerBaseMtx(*mTransform);
};

s32 ReturnDemoRailMove::getDemoFlyBrakeFrame() const {
    return (mIsGrandStar) ? 296 : 45;
};

void ReturnDemoRailMove::setupPathDrawForGraneStarReturnDemo() {
    mPathDrawer->setMaskLength(100.0f);
    mPathDrawer->setFadeScale(0.1f);
};

void ReturnDemoRailMove::calcPathPosDir(TVec3f* position, TVec3f* direction, f32 t) const {
    if (position != nullptr) {
        mShootPath->calcPosition(position, t);
    }

    if (direction != nullptr) {
        mShootPath->calcDirection(direction, t, 0.01f);
    }
};

void ReturnDemoRailMove::makeEndMtx(TPos3f* pMtx, const TVec3f& rForward) const {
    TVec3f up;
    up.negate(mGravity);
    MR::makeMtxUpFront(pMtx, up, rForward);
}
