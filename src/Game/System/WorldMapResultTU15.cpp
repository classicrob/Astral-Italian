#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/System/MiniPartsActors.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Util/TalkUtil.hpp"
#include "Game/System/WorldMapResultStateGreenStarDemo.hpp"

namespace MR {
    void forceOpenSystemWipeWhiteFade(s32);
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" {
    void fn_804F33F0(TVec3f*, const TVec3f*, const TVec3f*, s32, s32);
}

extern s32 lbl_807E3148;

/// Camera vectors of the demo (8 entries, constructed in declaration order).
static TVec3f sCameraTable[8] = {
    TVec3f(60000.0f, 30000.0f, 35000.0f),
    TVec3f(-65000.0f, 30000.0f, 35000.0f),
    TVec3f(60000.0f, -3000.0f, 8000.0f),
    TVec3f(-65000.0f, -3000.0f, 8000.0f),
    TVec3f(40000.0f, 27627.0f, 33520.0f),
    TVec3f(-50000.0f, 27627.0f, 33520.0f),
    TVec3f(40000.0f, 0.0f, -2000.0f),
    TVec3f(-50000.0f, 0.0f, -2000.0f),
};

/// Camera name of the demo ("グリーンスターデモカメラ").
static const char* sCameraName = "グリーンスターデモカメラ";

WorldMapResultStateGreenStarDemo::WorldMapResultStateGreenStarDemo(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[グリーンスター出現デモ]", pHost) {
    mComet = NULL;
    mShower = NULL;
    mCounts = NULL;
    mTalkCtrl = NULL;
}

void WorldMapResultStateGreenStarDemo::init() {
    initNerve(GET_NERVE(WorldMapResultStateGreenStarDemo, WorldMapResultStateGreenStarDemoStart), 0);
    MR::declareEventCameraProgrammable(sCameraName);
    TVec3f zero(0.0f, 0.0f, 0.0f);
    mTalkCtrl = MR::createTalkCtrlDirect(mHost->getMeister(), "WorldMapNewsStartWorld08", zero, NULL);
    mComet = new MiniGreenComet();
    mComet->initWithoutIter();
    mComet->makeActorDead();
    mShower = new MiniGreenCometShower();
    mShower->initWithoutIter();
    mShower->makeActorDead();
    mCounts = new GalaxyWorldMapInfo*[7];
    for (s32 i = 0; i < 7; i++) {
        mCounts[i] = nullptr;
    }
    mIsDead = true;
}

void WorldMapResultStateGreenStarDemo::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(WorldMapResultStateGreenStarDemo, WorldMapResultStateGreenStarDemoStart));
    mHost->getMainLayout()->activate();
    mHost->getMainLayout()->killRoutes();
    mHost->finishWorld();
    mHost->getMainLayout()->warpRocketToNode(0);
    mHost->changeRocketActionBck("Wait");
    mHost->hideRocket();
    MR::startGlobalEventCameraNoTarget(sCameraName, 0);
    mHost->startDemo();
}

void WorldMapResultStateGreenStarDemo::kill() {
    mShower->kill();
    mHost->endDemo();
    mIsDead = true;
}

void WorldMapResultStateGreenStarDemo::control() {
    if (!isNerve(GET_NERVE(WorldMapResultStateGreenStarDemo, WorldMapResultStateGreenStarDemoStart))) {
        mHost->startAllPointingAnim();
    }
}

void WorldMapResultStateGreenStarDemo::showComets() {
    mShower->appear();
    mHost->collectInfos(mCounts);
    for (s32 i = 0; i < 7; i++) {
        mCounts[i]->endPointingAnim();
    }
}

void WorldMapResultStateGreenStarDemo::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::forceOpenSystemWipeWhiteFade(1);
        MR::openWipeFade(60);
    }
    if (MR::isStep(this, 120)) {
        mComet->appear();
    }
    f32 rate = MR::calcNerveRate(this, 300);
    TVec3f vec2c = sCameraTable[0] * (1.0f - rate);
    TVec3f vec38 = sCameraTable[1] * rate;
    TVec3f vec50 = vec38 + vec2c;
    TVec3f vec14 = sCameraTable[2] * (1.0f - rate);
    TVec3f vec20 = sCameraTable[3] * rate;
    TVec3f vec44 = vec20 + vec14;
    TVec3f vecUp(0.0f, 1.0f, 0.0f);
    MR::setProgrammableCameraParam(sCameraName, vec44, vec50, vecUp, true);
    if (MR::isStep(this, 240)) {
        MR::closeWipeFade(60);
    }
    if (MR::isWipeBlank()) {
        mHost->getMainLayout()->deactivate();
        mHost->prepareWorld();
        mHost->warpRocketToNode(0);
        mHost->showRocket();
        MR::endGlobalEventCamera(sCameraName, -1, true);
        setNerve(GET_NERVE(WorldMapResultStateGreenStarDemo, WorldMapResultStateGreenStarDemoShower));
    }
}

void WorldMapResultStateGreenStarDemo::exeShower() {
    if (MR::isFirstStep(this)) {
        showComets();
        MR::startGlobalEventCameraNoTarget(sCameraName, 0);
        MR::openWipeFade(60);
    }
    s32 steps[7] = {lbl_807E3148, 40, 70, 150, 210, 290, 360};
    for (s32 i = 0; i < 7; i++) {
        if (MR::isStep(this, steps[i])) {
            mCounts[i]->setComet(true);
        }
    }
    TVec3f vec20;
    fn_804F33F0(&vec20, &sCameraTable[4], &sCameraTable[5], getNerveStep(), 500);
    TVec3f vec14;
    fn_804F33F0(&vec14, &sCameraTable[6], &sCameraTable[7], getNerveStep(), 500);
    TVec3f vecUp(0.0f, 1.0f, 0.0f);
    MR::setProgrammableCameraParam(sCameraName, vec14, vec20, vecUp, true);
    MR::setProgrammableCameraParamFovy(sCameraName, 26.0f);
    if (MR::isStep(this, 500)) {
        mShower->kill();
        setNerve(GET_NERVE(WorldMapResultStateGreenStarDemo, WorldMapResultStateGreenStarDemoWait));
    }
}

void WorldMapResultStateGreenStarDemo::exeTalkStart() {
    if (MR::isFirstStep(this)) {
        mHost->appearMeister();
    }
    if (!mHost->isMeisterTalkStarting()) {
        setNerve(GET_NERVE(WorldMapResultStateGreenStarDemo, WorldMapResultStateGreenStarDemoTalk));
    }
}

void WorldMapResultStateGreenStarDemo::exeTalk() {
    if (MR::isFirstStep(this)) {
        mHost->setMeisterTalk();
    }
    if (MR::tryTalkForceWithoutDemoAtEnd(mTalkCtrl)) {
        setNerve(GET_NERVE(WorldMapResultStateGreenStarDemo, WorldMapResultStateGreenStarDemoEnd));
    }
}

void WorldMapResultStateGreenStarDemo::exeEnd() {
    if (MR::isFirstStep(this)) {
        mHost->setMeisterTalkEndWait();
    }
    if (!mHost->isMeisterTalkEnding()) {
        kill();
    }
}

WorldMapResultStateGreenStarDemo::~WorldMapResultStateGreenStarDemo() {
}

namespace NrvWorldMapResultStateGreenStarDemo {
    WMR_NERVE_DEF(WorldMapResultStateGreenStarDemoEnd, WorldMapResultStateGreenStarDemo, End);
    WMR_NERVE_DEF(WorldMapResultStateGreenStarDemoTalk, WorldMapResultStateGreenStarDemo, Talk);
    WMR_NERVE_DEF(WorldMapResultStateGreenStarDemoTalkStart, WorldMapResultStateGreenStarDemo, TalkStart);
    void WorldMapResultStateGreenStarDemoWait::execute(Spine* pSpine) const {
        WorldMapResultStateGreenStarDemo* pState = reinterpret_cast< WorldMapResultStateGreenStarDemo* >(pSpine->mExecutor);
        if (MR::isGreaterEqualStep(pState, 120)) {
            pState->setNerve(GET_NERVE(WorldMapResultStateGreenStarDemo, WorldMapResultStateGreenStarDemoTalkStart));
        }
    }
    WMR_NERVE_DEF(WorldMapResultStateGreenStarDemoShower, WorldMapResultStateGreenStarDemo, Shower);
    WMR_NERVE_DEF(WorldMapResultStateGreenStarDemoStart, WorldMapResultStateGreenStarDemo, Start);
    WorldMapResultStateGreenStarDemoWait WorldMapResultStateGreenStarDemoWait::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvWorldMapResultStateGreenStarDemo
