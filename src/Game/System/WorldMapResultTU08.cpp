#define JGEOMETRY_VEC3_WMR_OUTLINE
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/Screen/GalaxyConfirmLayout.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/System/WorldMapResultStateEnterGalaxy.hpp"

extern const char* lbl_807D0550;
extern const char* lbl_807D0554;
extern const char* lbl_807D0558;
extern const char* lbl_807D055C;
extern const char* lbl_807D0560;
extern const char* lbl_807D0564;
extern const char* lbl_807D0568;
extern const char* lbl_807D056C;

extern const f32 lbl_807E1D00;
extern const f32 lbl_807E1D08;
extern const f32 lbl_807E1D0C;
extern const s32 lbl_807E1D10;
extern const s32 lbl_807E1D14;
extern const f32 lbl_807E1D18;
extern const s32 lbl_807E1D1C;
extern const s32 lbl_807E1D20;
extern const f32 lbl_807E1D24;
extern const s32 lbl_807E1D28;
extern const s32 lbl_807E1D2C;
extern const f32 lbl_807E1D30;
extern const s32 lbl_807E1D34;
extern const s32 lbl_807E1D38;

namespace {
    extern TVec3f sEnterGalaxyPositionNormal;
    extern TVec3f sEnterGalaxyPositionType3;
    extern TVec3f sEnterGalaxyPositionType4;
    extern TVec3f sEnterGalaxyPositionType5;
}

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

namespace NrvWorldMapResultStateEnterGalaxy {
    void WorldMapResultStateEnterGalaxyZoomOut::execute(Spine* spine) const {
        reinterpret_cast<WorldMapResultStateEnterGalaxy*>(spine->mExecutor)->exeZoomOut();
    }
    void WorldMapResultStateEnterGalaxyDemo::execute(Spine* spine) const {
        reinterpret_cast<WorldMapResultStateEnterGalaxy*>(spine->mExecutor)->exeDemo();
    }
    void WorldMapResultStateEnterGalaxyWait::execute(Spine* spine) const {
        reinterpret_cast<WorldMapResultStateEnterGalaxy*>(spine->mExecutor)->exeWait();
    }
    void WorldMapResultStateEnterGalaxyZoomIn::execute(Spine* spine) const {
        reinterpret_cast<WorldMapResultStateEnterGalaxy*>(spine->mExecutor)->exeZoomIn();
    }
    WorldMapResultStateEnterGalaxyZoomIn WorldMapResultStateEnterGalaxyZoomIn::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateEnterGalaxyWait WorldMapResultStateEnterGalaxyWait::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateEnterGalaxyDemo WorldMapResultStateEnterGalaxyDemo::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateEnterGalaxyZoomOut WorldMapResultStateEnterGalaxyZoomOut::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvWorldMapResultStateEnterGalaxy

namespace {
    TVec3f sEnterGalaxyPositionNormal(-770.0f, -60.0f, 290.0f);
    TVec3f sEnterGalaxyPositionType3(-770.0f, -60.0f, 290.0f);
    TVec3f sEnterGalaxyPositionType4(240.0f, -800.0f, -600.0f);
    TVec3f sEnterGalaxyPositionType5(-770.0f, 220.0f, 290.0f);
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
        mTargetPos = &sEnterGalaxyPositionType3;
        mCameraName = lbl_807D0564;
        mAnimName = lbl_807D0554;
        mBlurNear = &lbl_807E1D1C;
        mBlurFar = &lbl_807E1D20;
        mRate = lbl_807E1D18;
    } else if (mHost->isWorldDataType4AtCurrent()) {
        mTargetPos = &sEnterGalaxyPositionType4;
        mCameraName = lbl_807D0568;
        mAnimName = lbl_807D0558;
        mBlurNear = &lbl_807E1D28;
        mBlurFar = &lbl_807E1D2C;
        mRate = lbl_807E1D24;
    } else if (mHost->isWorldDataType5AtCurrent()) {
        mTargetPos = &sEnterGalaxyPositionType5;
        mCameraName = lbl_807D056C;
        mAnimName = lbl_807D055C;
        mBlurNear = &lbl_807E1D34;
        mBlurFar = &lbl_807E1D38;
        mRate = lbl_807E1D30;
    } else {
        mTargetPos = &sEnterGalaxyPositionNormal;
        mCameraName = lbl_807D0560;
        mAnimName = lbl_807D0550;
        mBlurNear = &lbl_807E1D10;
        mBlurFar = &lbl_807E1D14;
        mRate = lbl_807E1D0C;
    }
    mFlag = mHost->isCurrentGalaxyInfoShown();
    mHost->startCameraAtCurrent(mCameraName, -1);
    mHost->changeRocketAction("Wait");
    const TVec3f* pos = mHost->getRocketPosition();
    mPos.x = pos->x;
    mPos.y = pos->y;
    mPos.z = pos->z;
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

// NONMATCHING: FP operand commutation only (grouping checked); register/stack/branch lowering.
void WorldMapResultStateEnterGalaxy::exeZoomIn() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(mHost->getRocket(), "ZoomInGalaxy", -1, -1, -1);
    }
    if (mFlag) {
        WorldMapOwner* infoHost = mHost;
        infoHost->showGalaxyInfo(infoHost->getCurrentGalaxyName());
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 30);
    TVec3f merged = (mPos * (lbl_807E1D00 - rate)) + ((mPos + *mTargetPos) * rate);
    mHost->setRocketPosWithTarget(&merged);
    f32 ease = MR::calcNerveEaseInRate(this, 30);
    WorldMapOwner* host = mHost;
#ifdef GALAXY_NATIVE
    {
        TVec3f rocketScale((lbl_807E1D00 - mRate) + ((lbl_807E1D00 - ease) * mRate));
        host->setRocketScale(&rocketScale);
    }
#else
    host->setRocketScale(&TVec3f((lbl_807E1D00 - mRate) + ((lbl_807E1D00 - ease) * mRate)));
#endif
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
    WorldMapOwner* infoHost = mHost;
    infoHost->showGalaxyInfo(infoHost->getCurrentGalaxyName());
    if (mConfirmLayout->isSelectionComplete() != 0 && mConfirmLayout->isSelectedYes() != 0) {
        setNerve(GET_NERVE(WorldMapResultStateEnterGalaxy, WorldMapResultStateEnterGalaxyDemo));
        return;
    }
    if (mConfirmLayout->isSelectionComplete() != 0 && mConfirmLayout->isSelectedYes() == 0) {
        setNerve(GET_NERVE(WorldMapResultStateEnterGalaxy, WorldMapResultStateEnterGalaxyZoomOut));
    }
}

// NONMATCHING: FP operand commutation only (grouping checked); register/stack/branch lowering.
void WorldMapResultStateEnterGalaxy::exeZoomOut() {
    if (MR::isFirstStep(this)) {
        MR::setImageEffectControlAuto();
        mHost->startNodeCamera(-1, -1);
        MR::startActionSound(mHost->getRocket(), "ZoomOutGalaxy", -1, -1, -1);
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 30);
    TVec3f merged = ((mPos + *mTargetPos) * (lbl_807E1D00 - rate)) + (mPos * rate);
    mHost->setRocketPosWithTarget(&merged);
    WorldMapOwner* host = mHost;
#ifdef GALAXY_NATIVE
    {
        TVec3f rocketScale((lbl_807E1D00 - mRate) + ((lbl_807E1D00 - (lbl_807E1D00 - rate)) * mRate));
        host->setRocketScale(&rocketScale);
    }
#else
    host->setRocketScale(&TVec3f((lbl_807E1D00 - mRate) + ((lbl_807E1D00 - (lbl_807E1D00 - rate)) * mRate)));
#endif
    WorldMapOwner* infoHost = mHost;
    infoHost->showGalaxyInfo(infoHost->getCurrentGalaxyName());
    if (MR::isGreaterEqualStep(this, 30)) {
        MR::showPlayer();
        kill();
    }
}

void WorldMapResultStateEnterGalaxy::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::showPlayer();
        mHost->startDemo();
        MR::startAction(MR::getPlayerDemoActor(), mAnimName);
        mHost->changeRocketAction(lbl_807D0550);
        MR::startAnimCameraTargetPlayer(MR::getPlayerDemoActor(), mCamInfo, mAnimName, -1, false, 1.0f);
        TVec3f negCamZ = -MR::getCamZdir();
        WorldMapOwner* host = mHost;
        host->setRocketFrontUpPos(negCamZ, MR::getCamYdir(), *host->getRocketPos());
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

WorldMapResultStateEnterGalaxy::~WorldMapResultStateEnterGalaxy() {
}

const f32 lbl_807E1D00 = 1.0f;
const f32 lbl_807E1D08 = 1.0f;
const f32 lbl_807E1D0C = 0.5f;
const s32 lbl_807E1D10 = 8900;
const s32 lbl_807E1D14 = 9200;
const f32 lbl_807E1D18 = 0.1f;
const s32 lbl_807E1D1C = 9350;
const s32 lbl_807E1D20 = 10000;
const f32 lbl_807E1D24 = 0.3f;
const s32 lbl_807E1D28 = 9350;
const s32 lbl_807E1D2C = 10000;
const f32 lbl_807E1D30 = 0.3f;
const s32 lbl_807E1D34 = 9350;
const s32 lbl_807E1D38 = 10000;
