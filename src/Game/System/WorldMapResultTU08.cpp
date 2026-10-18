#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/Screen/GalaxyConfirmLayout.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/System/WorldMapResultStateEnterGalaxy.hpp"

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" {
    extern const char* lbl_807D0550;
    extern const char* lbl_807D0554;
    extern const char* lbl_807D0558;
    extern const char* lbl_807D055C;
    extern const char* lbl_807D0560;
    extern const char* lbl_807D0564;
    extern const char* lbl_807D0568;
    extern const char* lbl_807D056C;
}

extern f32 lbl_807E1D00;
extern f32 lbl_807E1D08;
extern f32 lbl_807E1D0C;
extern s32 lbl_807E1D10;
extern s32 lbl_807E1D14;
extern f32 lbl_807E1D18;
extern s32 lbl_807E1D1C;
extern s32 lbl_807E1D20;
extern f32 lbl_807E1D24;
extern s32 lbl_807E1D28;
extern s32 lbl_807E1D2C;
extern f32 lbl_807E1D30;
extern s32 lbl_807E1D34;
extern s32 lbl_807E1D38;

/// Camera vectors of the galaxy entry (4 entries of 3 vectors each).
struct WorldMapResultEnterGalaxyTable {
    /* 0x00 */ TVec3f mA;
    /* 0x0C */ TVec3f mB;
    /* 0x18 */ TVec3f mC;
    /* 0x24 */ TVec3f mD;
};
extern WorldMapResultEnterGalaxyTable lbl_8072BA38;

namespace MR {
    void setDepthOfFieldBlurDistance(s32, s32);
    void initAnimCamera(const LiveActor*, const ActorCameraInfo*, const char*);
    void startAnimCameraTargetPlayer(const LiveActor*, const ActorCameraInfo*, const char*, s32, bool, f32);
    void createDepthOfFieldBlur();
    void setDepthOfFieldBlurIntensity(f32);
    void closeSystemWipeWhiteFade(s32);
    bool isUseScenarioSelectMusicOneStar();
    void stopStageBGM(u32);
    void startStarPointerModeDemo(void*);
    void setImageEffectControlAuto();
    TVec3f getCamZdir();
    TVec3f getCamYdir();
    bool isLessEqualStep(const LiveActor*, s32);
    f32 calcNerveEaseInRate(const NerveExecutor*, s32);
    f32 calcNerveEaseInOutRate(const NerveExecutor*, s32);
}

WorldMapResultStateEnterGalaxy::WorldMapResultStateEnterGalaxy(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[ギャラクシーに入る]", pHost) {
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mTargetPos = NULL;
    mCameraName = NULL;
    mAnimName = NULL;
    mConfirmLayout = NULL;
    mBlurNear = NULL;
    mBlurFar = NULL;
    mRate = 0.0f;
    mFlag = false;
}

void WorldMapResultStateEnterGalaxy::init() {
    mHost->declareCamera(lbl_807D0560);
    mHost->declareCamera(lbl_807D0564);
    mHost->declareCamera(lbl_807D0568);
    mHost->declareCamera(lbl_807D056C);
    mConfirmLayout = new GalaxyConfirmLayout(0);
    mConfirmLayout->initWithoutIter();
    initNerve(GET_NERVE(WorldMapResultStateEnterGalaxy, WorldMapResultStateEnterGalaxyZoomIn), 0);
    mCamInfo = new ActorCameraInfo(-1, 0);
    MR::initAnimCamera(MR::getPlayerDemoActor(), mCamInfo, lbl_807D0550);
    MR::initAnimCamera(MR::getPlayerDemoActor(), mCamInfo, lbl_807D0554);
    MR::initAnimCamera(MR::getPlayerDemoActor(), mCamInfo, lbl_807D0558);
    MR::initAnimCamera(MR::getPlayerDemoActor(), mCamInfo, lbl_807D055C);
    MR::createDepthOfFieldBlur();
    mIsDead = true;
}

void WorldMapResultStateEnterGalaxy::appear() {
    mIsDead = false;
    if (mHost->isWorldDataType3AtCurrent()) {
        mTargetPos = &lbl_8072BA38.mB;
        mCameraName = lbl_807D0564;
        mAnimName = lbl_807D0554;
        mBlurNear = &lbl_807E1D1C;
        mBlurFar = &lbl_807E1D20;
        mRate = lbl_807E1D18;
    } else if (mHost->isWorldDataType4AtCurrent()) {
        mTargetPos = &lbl_8072BA38.mC;
        mCameraName = lbl_807D0568;
        mAnimName = lbl_807D0558;
        mBlurNear = &lbl_807E1D28;
        mBlurFar = &lbl_807E1D2C;
        mRate = lbl_807E1D24;
    } else if (mHost->isWorldDataType5AtCurrent()) {
        mTargetPos = &lbl_8072BA38.mD;
        mCameraName = lbl_807D056C;
        mAnimName = lbl_807D055C;
        mBlurNear = &lbl_807E1D34;
        mBlurFar = &lbl_807E1D38;
        mRate = lbl_807E1D30;
    } else {
        mTargetPos = &lbl_8072BA38.mA;
        mCameraName = lbl_807D0560;
        mAnimName = lbl_807D0550;
        mBlurNear = &lbl_807E1D10;
        mBlurFar = &lbl_807E1D14;
        mRate = lbl_807E1D0C;
    }
    mFlag = mHost->isCurrentGalaxyInfoShown();
    mHost->startCameraAtCurrent(mCameraName, -1);
    mHost->changeRocketAction("Wait");
    mPos = *mHost->getRocketPosition();
    setNerve(GET_NERVE(WorldMapResultStateEnterGalaxy, WorldMapResultStateEnterGalaxyZoomIn));
}

void WorldMapResultStateEnterGalaxy::kill() {
    mIsDead = true;
}

void WorldMapResultStateEnterGalaxy::control() {
    mHost->updateParts();
    MR::setDepthOfFieldBlurIntensity(lbl_807E1D08);
    MR::setDepthOfFieldBlurDistance(*mBlurNear, *mBlurFar);
}

WorldMapResultStateEnterGalaxy::~WorldMapResultStateEnterGalaxy() {
}

void WorldMapResultStateEnterGalaxy::exeZoomIn() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(mHost->getRocket(), "ZoomInGalaxy", -1, -1, -1);
    }
    if (mFlag) {
        mHost->showGalaxyInfo(mHost->getCurrentGalaxyName());
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 30);
    TVec3f sum = mPos + *mTargetPos;
    TVec3f scaled = sum * rate;
    TVec3f rest = mPos * (lbl_807E1D00 - rate);
    TVec3f merged = rest + scaled;
    mHost->setRocketPosWithTarget(&merged);
    f32 ease = MR::calcNerveEaseInRate(this, 30);
    TVec3f uniform((lbl_807E1D00 - mRate) + ((lbl_807E1D00 - ease) * mRate));
    mHost->setRocketScale(&uniform);
    if (MR::isStep(this, 16)) {
        MR::turnOnDepthOfField(false);
    }
    if (MR::isGreaterEqualStep(this, 30)) {
        setNerve(GET_NERVE(WorldMapResultStateEnterGalaxy, WorldMapResultStateEnterGalaxyWait));
    }
}

void WorldMapResultStateEnterGalaxy::exeWait() {
    if (MR::isFirstStep(this)) {
        mConfirmLayout->appear();
    }
    mHost->showGalaxyInfo(mHost->getCurrentGalaxyName());
    if (mConfirmLayout->isSelectionComplete() != 0 && mConfirmLayout->isSelectedYes() != 0) {
        setNerve(GET_NERVE(WorldMapResultStateEnterGalaxy, WorldMapResultStateEnterGalaxyDemo));
        return;
    }
    if (mConfirmLayout->isSelectionComplete() != 0 && mConfirmLayout->isSelectedYes() == 0) {
        setNerve(GET_NERVE(WorldMapResultStateEnterGalaxy, WorldMapResultStateEnterGalaxyZoomOut));
    }
}

void WorldMapResultStateEnterGalaxy::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::showPlayer();
        mHost->startDemo();
        MR::startAction(MR::getPlayerDemoActor(), mAnimName);
        mHost->changeRocketAction(lbl_807D0550);
        MR::startAnimCameraTargetPlayer(MR::getPlayerDemoActor(), mCamInfo, mAnimName, -1, false, 1.0f);
        TVec3f camZ = MR::getCamZdir();
        TVec3f negCamZ(-camZ.x, -camZ.y, -camZ.z);
        const TVec3f* pRocketPos = mHost->getRocketPos();
        TVec3f camY = MR::getCamYdir();
        mHost->setRocketFrontUpPos(negCamZ, camY, *pRocketPos);
    }
    MR::isLessEqualStep(this, 170);
    if (MR::isStep(this, 170)) {
        MR::closeSystemWipeWhiteFade(-1);
        if (!MR::isUseScenarioSelectMusicOneStar()) {
            MR::stopStageBGM(0x50);
        }
    }
    if (MR::isBckStoppedPlayer()) {
        setNerve(GET_NERVE(WorldMapResultStateEnterGalaxy, WorldMapResultStateEnterGalaxyZoomOut));
        mHost->enterCurrentGalaxy();
        MR::startStarPointerModeDemo(mHost);
    }
}

void WorldMapResultStateEnterGalaxy::exeZoomOut() {
    if (MR::isFirstStep(this)) {
        MR::setImageEffectControlAuto();
        mHost->startNodeCamera(-1, -1);
        MR::startActionSound(mHost->getRocket(), "ZoomOutGalaxy", -1, -1, -1);
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 30);
    TVec3f scaled = mPos * rate;
    TVec3f sum = mPos + *mTargetPos;
    TVec3f rest = sum * (lbl_807E1D00 - rate);
    TVec3f merged = rest + scaled;
    mHost->setRocketPosWithTarget(&merged);
    TVec3f uniform((lbl_807E1D00 - mRate) + (rate * mRate));
    mHost->setRocketScale(&uniform);
    mHost->showGalaxyInfo(mHost->getCurrentGalaxyName());
    if (MR::isGreaterEqualStep(this, 30)) {
        MR::showPlayer();
        kill();
    }
}

namespace NrvWorldMapResultStateEnterGalaxy {
    WMR_NERVE_DEF(WorldMapResultStateEnterGalaxyZoomOut, WorldMapResultStateEnterGalaxy, ZoomOut);
    WMR_NERVE_DEF(WorldMapResultStateEnterGalaxyDemo, WorldMapResultStateEnterGalaxy, Demo);
    WMR_NERVE_DEF(WorldMapResultStateEnterGalaxyWait, WorldMapResultStateEnterGalaxy, Wait);
    WMR_NERVE_DEF(WorldMapResultStateEnterGalaxyZoomIn, WorldMapResultStateEnterGalaxy, ZoomIn);
};  // namespace NrvWorldMapResultStateEnterGalaxy
