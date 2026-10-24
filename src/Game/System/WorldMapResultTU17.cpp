#pragma auto_inline off
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/WorldMapResultStatePointEvent.hpp"
#include "Game/Util/TalkUtil.hpp"

s32 sPointEventRouteSetupStep = 0;

/// Camera name of the point-event demo ("ワールドマップカメラ").
static const char* sCameraName = "ワールドマップカメラ";

/// Talk messages of the star checkpoint (the third slot is empty).
static const char* sTalkNames[3] = {"WorldMapEventStarCheckPointClose", "WorldMapEventStarCheckPointCloseAndAdvice", NULL};

/// Offset of the point-event camera path (defined below the nerves, constructed last in the sinit).
extern TVec3f sPointEventOffset;

namespace NrvWorldMapResultStatePointEvent {
    WMR_NERVE_DECL(WorldMapResultStatePointEventStart);
    WMR_NERVE_DECL(WorldMapResultStatePointEventMove);
    WMR_NERVE_DECL(WorldMapResultStatePointEventWait);
    WMR_NERVE_DECL(WorldMapResultStatePointEventTalkStart);
    WMR_NERVE_DECL(WorldMapResultStatePointEventTalk);
    WMR_NERVE_DECL(WorldMapResultStatePointEventTalkEnd);
    WMR_NERVE_DECL(WorldMapResultStatePointEventWaitFade);
    WMR_NERVE_DECL(WorldMapResultStatePointEventFade);
    WMR_NERVE_DECL(WorldMapResultStatePointEventEaseIn);
    WMR_NERVE_DECL(WorldMapResultStatePointEventEvent);
    WMR_NERVE_DECL(WorldMapResultStatePointEventExtend);
    WMR_NERVE_DECL(WorldMapResultStatePointEventWaitExtend);
    WMR_NERVE_DECL(WorldMapResultStatePointEventMessage);
    WMR_NERVE_DECL(WorldMapResultStatePointEventEnd);
};  // namespace NrvWorldMapResultStatePointEvent

// NONMATCHING: register/stack or instruction scheduling (FP order/grouping proven by default rdclass c-checked).
WorldMapResultStatePointEvent::WorldMapResultStatePointEvent(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[ポイントイベント]", pHost) {
    mIsDead = true;
    mRoute = NULL;
    mCount = 0;
    mIndex = -1;
    mSubIndex = -1;
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mOffset.x = 0.0f;
    mOffset.y = 0.0f;
    mOffset.z = 0.0f;
    mTalkCtrls = NULL;
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void WorldMapResultStatePointEvent::init() {
    initNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventStart), 0);
    mHost->declareCamera("イベント（スターチェックポイント：スター足りない）");
    MR::declareEventCameraProgrammable(sCameraName);
    mTalkCtrls = new TalkMessageCtrl*[3];
    for (s32 i = 0; i < 3; i++) {
        if (sTalkNames[i] != NULL) {
            WorldMapOwner* owner = mHost;
            TVec3f zero(0.0f, 0.0f, 0.0f);
            mTalkCtrls[i] = MR::createTalkCtrlDirect(owner->getMeister(), sTalkNames[i], zero, NULL);
        }
    }
    mRoute = new WorldMapResultRoute();
    mIsDead = true;
}

void WorldMapResultStatePointEvent::appear() {
    mIsDead = false;
    mCount = 0;
    mHost->changeRocketAction("Wait");
    mPos.set(*mHost->getRocketPosition());
    mHost->startDemo();
    MR::hidePlayer();
    if ((u32)mIndex <= 1U) {
        mHost->startCamera("イベント（スターチェックポイント：スター足りない）", -1);
        setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventStart));
    } else if (mIndex == 2) {
        setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventEaseIn));
        const TVec3f* galaxyOffset = WorldMapFunction::getRocketOffsetGalaxy();
        TVec3f sp8(mPos);
        sp8 -= *galaxyOffset;
        TVec3f sp14 = sp8 + *WorldMapFunction::getRocketOffsetPoint();
        mOffset.set(sp14);
    }
    GameSequenceFunction::setWorldMapCurrentNode(mHost->getCurrentNode());
}

void WorldMapResultStatePointEvent::kill() {
    mIsDead = true;
    MR::showPlayer();
    mHost->endDemo();
    mIndex = -1;
}

void WorldMapResultStatePointEvent::control() {
    mHost->updateParts();
    if (isNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventEaseIn)) || isNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventEvent)) ||
        isNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventExtend)) ||
        isNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventWaitExtend)) ||
        isNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventMessage)) ||
        isNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventEnd))) {
        mHost->startAllPointingAnim();
        return;
    }
    MR::setDepthOfFieldBlurIntensity(1.0f);
    MR::setDepthOfFieldBlurDistance(0x2486, 0x2710);
}

void WorldMapResultStatePointEvent::setIndex(s32 index, s32 subIndex) {
    mIndex = index;
    mSubIndex = subIndex;
}

void WorldMapResultStatePointEvent::exeStart() {
    if (MR::isStep(this, 8)) {
        MR::turnOnDepthOfField(false);
    }
    if (MR::isGreaterEqualStep(this, 0x16)) {
        setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventMove));
    }
}

void WorldMapResultStatePointEvent::exeMove() {
    moveCamera(MR::getLinerValue(MR::calcNerveRate(this, 30), 0.0f, 1.0f, 1.0f));
    if (MR::isGreaterEqualStep(this, 0x1E)) {
        setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventWait));
    }
}

void WorldMapResultStatePointEvent::exeTalkStart() {
    if (MR::isFirstStep(this)) {
        mHost->appearMeister();
    }
    if (!mHost->isMeisterTalkStarting()) {
        setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventTalk));
    }
}

void WorldMapResultStatePointEvent::exeTalk() {
    if (MR::isFirstStep(this)) {
        mHost->setMeisterTalk();
    }
    if (MR::tryTalkForceWithoutDemoAtEnd(mTalkCtrls[mIndex])) {
        setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventTalkEnd));
    }
}

void WorldMapResultStatePointEvent::exeTalkEnd() {
    if (MR::isFirstStep(this)) {
        mHost->setMeisterTalkEndWait();
    }
    if (!mHost->isMeisterTalkEnding()) {
        setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventWaitFade));
    }
}

void WorldMapResultStatePointEvent::exeFade() {
    if (MR::isFirstStep(this)) {
        MR::setImageEffectControlAuto();
    }
    moveCamera(MR::getLinerValue(1.0f - MR::calcNerveRate(this, 30), 0.0f, 1.0f, 1.0f));
    if (MR::isGreaterEqualStep(this, 30)) {
        kill();
    }
}

void WorldMapResultStatePointEvent::exeEaseIn() {
    moveCamera(MR::getEaseInValue(MR::calcNerveRate(this, 30), 0.0f, 1.0f, 1.0f));
    if (MR::isGreaterEqualStep(this, 30)) {
        setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventEvent));
    }
}

void WorldMapResultStatePointEvent::exeEvent() {
    if (MR::isStep(this, sPointEventRouteSetupStep)) {
        mHost->setupRouteOpen(mRoute);
    }
    if (MR::isGreaterEqualStep(this, 0xF0)) {
        if (mRoute->mCount == 0) {
            setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventEnd));
            return;
        }
        mHost->startRouteCamera(mRoute, sCameraName);
        setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventExtend));
    }
}

void WorldMapResultStatePointEvent::exeExtend() {
    bool result = mHost->updateRoute(mRoute, mCount, MR::calcNerveRate(this, 30));
    MR::startActionSound(mHost->getRocket(), "RoadExtend", -1, -1, -1);
    if (MR::isGreaterEqualStep(this, 30)) {
        if (result != 0) {
            setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventWaitExtend));
            return;
        }
        mCount++;
        setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventExtend));
    }
}

void WorldMapResultStatePointEvent::exeWaitExtend() {
    if (mHost->isRouteNotHeld(mRoute)) {
        setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventMessage));
    }
}

void WorldMapResultStatePointEvent::exeMessage() {
    if (MR::isStep(this, 12)) {
        MR::appearInformationMessageDown("System_ResultWorldMapRouteOpen", true);
        MR::startSubBGM("BGM_JG_NEW_GALAXY", false);
    }
    if (!MR::isLessEqualStep(this, 12)) {
        if (MR::isDeadInformationMessage()) {
            setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventEnd));
        }
    }
}

void WorldMapResultStatePointEvent::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::endGlobalEventCamera(sCameraName, -1, true);
        mHost->startNodeCamera(-1, -1);
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 30);
    TVec3f sp2C = mOffset * rate + (mPos + sPointEventOffset) * (1.0f - rate);
    mHost->setRocketPosWithTarget(&sp2C);
    if (MR::isGreaterEqualStep(this, 30)) {
        kill();
    }
}

void WorldMapResultStatePointEvent::moveCamera(f32 rate) {
    rate = MR::getEaseInOutValue(rate, 0.0f, 1.0f, 1.0f);
    TVec3f sp2C = mPos * (1.0f - rate) + (mPos + sPointEventOffset) * rate;
    mHost->setRocketPosWithTarget(&sp2C);
}

WorldMapResultStatePointEvent::~WorldMapResultStatePointEvent() {
}

namespace NrvWorldMapResultStatePointEvent {
    WMR_NERVE_DEF(WorldMapResultStatePointEventStart, WorldMapResultStatePointEvent, Start);
    WMR_NERVE_DEF(WorldMapResultStatePointEventMove, WorldMapResultStatePointEvent, Move);
    void WorldMapResultStatePointEventWait::execute(Spine* pSpine) const {
        WorldMapResultStatePointEvent* pState = reinterpret_cast< WorldMapResultStatePointEvent* >(pSpine->mExecutor);
        if (MR::isGreaterEqualStep(pState, 5)) {
            pState->setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventTalkStart));
        }
    }
    WorldMapResultStatePointEventWait WorldMapResultStatePointEventWait::sInstance ATTRIBUTE_WEAK;
    WMR_NERVE_DEF(WorldMapResultStatePointEventTalkStart, WorldMapResultStatePointEvent, TalkStart);
    WMR_NERVE_DEF(WorldMapResultStatePointEventTalk, WorldMapResultStatePointEvent, Talk);
    WMR_NERVE_DEF(WorldMapResultStatePointEventTalkEnd, WorldMapResultStatePointEvent, TalkEnd);
    void WorldMapResultStatePointEventWaitFade::execute(Spine* pSpine) const {
        WorldMapResultStatePointEvent* pState = reinterpret_cast< WorldMapResultStatePointEvent* >(pSpine->mExecutor);
        if (MR::isGreaterEqualStep(pState, 10)) {
            pState->setNerve(GET_NERVE(WorldMapResultStatePointEvent, WorldMapResultStatePointEventFade));
        }
    }
    WorldMapResultStatePointEventWaitFade WorldMapResultStatePointEventWaitFade::sInstance ATTRIBUTE_WEAK;
    WMR_NERVE_DEF(WorldMapResultStatePointEventFade, WorldMapResultStatePointEvent, Fade);
    WMR_NERVE_DEF(WorldMapResultStatePointEventEaseIn, WorldMapResultStatePointEvent, EaseIn);
    WMR_NERVE_DEF(WorldMapResultStatePointEventEvent, WorldMapResultStatePointEvent, Event);
    WMR_NERVE_DEF(WorldMapResultStatePointEventExtend, WorldMapResultStatePointEvent, Extend);
    WMR_NERVE_DEF(WorldMapResultStatePointEventWaitExtend, WorldMapResultStatePointEvent, WaitExtend);
    WMR_NERVE_DEF(WorldMapResultStatePointEventMessage, WorldMapResultStatePointEvent, Message);
    WMR_NERVE_DEF(WorldMapResultStatePointEventEnd, WorldMapResultStatePointEvent, End);
};  // namespace NrvWorldMapResultStatePointEvent

TVec3f sPointEventOffset(-800.0f, -1000.0f, 1000.0f);
