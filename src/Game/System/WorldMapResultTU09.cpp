#include "Game/System/WorldMapFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/Util/ActorCameraUtil.hpp"

#include "Game/Screen/GalaxyConfirmLayout.hpp"
#include "Game/System/WorldMapResultStateEnterHome.hpp"



namespace MR {
    void setImageEffectControlAuto();
    void startAnimCameraTargetSelf(const LiveActor*, const ActorCameraInfo*, const char*, s32, bool, f32);
    void initAnimCamera(const LiveActor*, const ActorCameraInfo*, const char*, ResourceHolder*);
    bool isAnimCameraEnd(const LiveActor*, const ActorCameraInfo*, const char*);
    void setScale(LiveActor*, f32);
    void setDepthOfFieldBlurDistance(s32, s32);
    f32 calcNerveEaseInOutRate(const NerveExecutor*, s32);
    void onDestroySceneKeepAllSound();
}

extern const char* lbl_807D0570;
extern f32 lbl_807E3140;

static void setEnterHomeCamera(WorldMapOwner* pHost) {
    pHost->setRocketSpeed(0.1f);
    TVec3f pos;
    pos.x = 0.0f;
    pos.y = 0.0f;
    pos.z = -1.0f;
    pHost->faceRocket(&pos);
}

WorldMapResultStateEnterHome::WorldMapResultStateEnterHome(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[ホームに入る]", pHost) {
    mLayout = nullptr;
}

void WorldMapResultStateEnterHome::init() {
    mHost->declareCamera("接近（ホーム）");
    mHost->declareCamera("突入（ホーム）");
    mLayout = new GalaxyConfirmLayout(2);
    mLayout->initWithoutIter();
    MR::createCenterScreenBlur();
    initNerve(GET_NERVE(WorldMapResultStateEnterHome, WorldMapResultStateEnterHomeApproach), 0);
    mCamInfo = new ActorCameraInfo(-1, 0);
    LiveActor* pActor;
    WorldMapOwner* host = mHost;
    pActor = host->getCamera()->mCameraTarget;
    MR::initAnimCamera(pActor, mCamInfo, lbl_807D0570, MR::getResourceHolder(host->getRocket()));
    MR::createDepthOfFieldBlur();
    mIsDead = true;
}

void WorldMapResultStateEnterHome::appear() {
    mIsDead = false;
    mHost->startCamera("接近（ホーム）", -1);
    mHost->changeRocketAction("Wait");
    setNerve(GET_NERVE(WorldMapResultStateEnterHome, WorldMapResultStateEnterHomeApproach));
}

void WorldMapResultStateEnterHome::kill() {
    mHost->endCamera("接近（ホーム）", false, -1);
    mIsDead = true;
}

void WorldMapResultStateEnterHome::control() {
    mHost->updateParts();
    MR::setDepthOfFieldBlurIntensity(1.0f);
    MR::setDepthOfFieldBlurDistance(0x22C4, 0x23F0);
}

inline f32 calcHomeApproachScale(f32 inverseRate, f32 range) {
    return (1.0f - range) + inverseRate * range;
}

void WorldMapResultStateEnterHome::exeApproach() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(mHost->getRocket(), "ZoomInShip", -1, -1, -1);
    }

    f32 rate = MR::calcNerveEaseInRate(this, 30);
    f32 scale = calcHomeApproachScale(1.0f - rate, lbl_807E3140);
    MR::setScale(mHost->getRocket(), scale);

    if (MR::isStep(this, 16)) {
        MR::turnOnDepthOfField(false);
    }

    if (MR::isGreaterEqualStep(this, 30)) {
        setNerve(GET_NERVE(WorldMapResultStateEnterHome, WorldMapResultStateEnterHomeWait));
    }
}

void WorldMapResultStateEnterHome::exeWait() {
    if (MR::isFirstStep(this)) {
        mLayout->appear();
    }

    if (mLayout->isSelectionComplete() && mLayout->isSelectedYes()) {
        setNerve(GET_NERVE(WorldMapResultStateEnterHome, WorldMapResultStateEnterHomeEnter));
    } else if (mLayout->isSelectionComplete() && !mLayout->isSelectedYes()) {
        setNerve(GET_NERVE(WorldMapResultStateEnterHome, WorldMapResultStateEnterHomeExit));
    } else if (mLayout->isSelected() && mLayout->isSelectedYes()) {
        setEnterHomeCamera(mHost);
    }
}

void WorldMapResultStateEnterHome::exeEnter() {
    if (MR::isFirstStep(this)) {
        MR::setImageEffectControlAuto();
        LiveActor* pActor = mHost->getCamera()->mCameraTarget;
        MR::startAnimCameraTargetSelf(pActor, mCamInfo, lbl_807D0570, -1, false, 1.0f);
        MR::stopStageAndSubBGM(30);
        mHost->startDemo();
    }

    if (MR::isStep(this, 15)) {
        MR::startSystemSE("SE_SY_WMAP_TO_SHIP", -1, -1);
        MR::onDestroySceneKeepAllSound();
    }

    setEnterHomeCamera(mHost);

    if (MR::isStep(this, 38)) {
        if (WorldMapFunction::isWorldNoMid(mHost->getWorldId())) {
            MR::startSpaceWipeForward();
        } else {
            MR::startCloudWipeForward();
        }
    }

    LiveActor* pActor = mHost->getCamera()->mCameraTarget;
    if (MR::isAnimCameraEnd(pActor, mCamInfo, lbl_807D0570)) {
        mHost->enterCurrentGalaxy();
    }
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
inline f32 calcHomeExitScale(f32 inverseRate, f32 range) {
    return (1.0f - range) + (1.0f - inverseRate) * range;
}

void WorldMapResultStateEnterHome::exeExit() {
    if (MR::isFirstStep(this)) {
        MR::setImageEffectControlAuto();
        mHost->endCamera("接近（ホーム）", false, -1);
        mHost->startNodeCamera(-1, -1);
        MR::startActionSound(mHost->getRocket(), "ZoomOutShip", -1, -1, -1);
    }

    f32 rate = MR::calcNerveEaseInOutRate(this, 30);
    f32 scale = calcHomeExitScale(1.0f - rate, lbl_807E3140);
    MR::setScale(mHost->getRocket(), scale);

    if (MR::isGreaterEqualStep(this, 30)) {
        kill();
    }
}

WorldMapResultStateEnterHome::~WorldMapResultStateEnterHome() {
}

namespace NrvWorldMapResultStateEnterHome {
    WMR_NERVE_DEF(WorldMapResultStateEnterHomeApproach, WorldMapResultStateEnterHome, Approach);
    WMR_NERVE_DEF(WorldMapResultStateEnterHomeWait, WorldMapResultStateEnterHome, Wait);
    WMR_NERVE_DEF(WorldMapResultStateEnterHomeEnter, WorldMapResultStateEnterHome, Enter);
    WMR_NERVE_DEF(WorldMapResultStateEnterHomeExit, WorldMapResultStateEnterHome, Exit);
};  
