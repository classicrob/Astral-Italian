#include "Game/NPC/EventDirector.hpp"
#include "Game/NPC/CometEventKeeper.hpp"
#include "Game/NPC/PowerStarEventKeeper.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/PowerStarHolder.hpp"
#include "Game/Scene/SceneObjHolder.hpp"

EventDirector::EventDirector()
    : NameObj("イベント指揮"), mPowerStarEventKeeper(), mPowerStarHolder(), mCometEventKeeper() {
}

void EventDirector::init(const JMapInfoIter& rIter) {
    mPowerStarEventKeeper = new PowerStarEventKeeper();
    mPowerStarHolder = new PowerStarHolder("パワースター保持");
    mPowerStarHolder->initWithoutIter();
    mCometEventKeeper = new CometEventKeeper();
    mCometEventKeeper->init();
}

namespace {
    EventDirector* getEventDirector() {
        return MR::getSceneObj< EventDirector >(SceneObj_EventDirector);
    }
}  // namespace

PowerStarEventKeeper* EventFunction::getPowerStarEventKeeper() {
    return getEventDirector()->mPowerStarEventKeeper;
}

PowerStarHolder* EventFunction::getPowerStarHolder() {
    return getEventDirector()->mPowerStarHolder;
}

CometEventKeeper* EventFunction::getCometEventKeeper() {
    return getEventDirector()->mCometEventKeeper;
}

bool EventFunction::isStartCometEvent(const char* pParam1) {
    return getCometEventKeeper()->isStartEvent(pParam1);
}

void EventFunction::startCometEvent() {
    getCometEventKeeper()->startCometEventIfExist();
}

void EventFunction::stopCometEvent() {
    getCometEventKeeper()->stopCometEvent();
}

void EventFunction::endCometEvent() {
    getCometEventKeeper()->endCometEvent();
}

bool EventFunction::isCometEvent() {
    return getCometEventKeeper()->mExecutor != nullptr;
}

namespace MR {
    void initEventSystemAfterPlacement() {
        EventFunction::getPowerStarEventKeeper()->initStarInfoTableAfterPlacement();
    }

    void declareEventPowerStar(const NameObj* pParam1, s32 param2) {
        const char* pVar1 = pParam1->mName;

        EventFunction::getPowerStarEventKeeper()->declareStar(pVar1, nullptr, param2);
    }

    void declareEventPowerStar(const char* pParam1, s32 param2) {
        EventFunction::getPowerStarEventKeeper()->declareStar(pParam1, "ゾーン無し", param2);
    }

    bool isSuccessEventPowerStar(const char* pParam1, s32 param2) {
        return EventFunction::getPowerStarEventKeeper()->isSuccess(pParam1, param2);
    }

    bool isGreenEventPowerStar(const char* pParam1, s32 param2) {
        return EventFunction::getPowerStarEventKeeper()->isGreen(pParam1, param2);
    }

    bool isBronzeEventPowerStar(const char* pParam1, s32 param2) {
        return EventFunction::getPowerStarEventKeeper()->isBronze(pParam1, param2);
    }

    bool isGrandEventPowerStar(const char* pParam1, s32 param2) {
        return EventFunction::getPowerStarEventKeeper()->isGrand(pParam1, param2);
    }

    void appearEventPowerStar(const char* pParam1, s32 param2, const TVec3f* pParam3, bool param4, bool param5, bool fromBelow) {
        EventFunction::getPowerStarEventKeeper()->requestAppearPowerStar(pParam1, param2, pParam3, param4, param5, fromBelow);
    }

    bool isEndEventPowerStarAppearDemo(const char* pParam1) {
        int starId = EventFunction::getPowerStarEventKeeper()->findStarID(pParam1);

        return PowerStarFunction::isEndPowerStarAppearDemo(starId);
    }

    s32 getEventPowerStarID(const char* pParam1) {
        return EventFunction::getPowerStarEventKeeper()->findStarID(pParam1);
    }

    void pauseCometTimer() {
        EventFunction::getCometEventKeeper()->pauseCometTimer();
    }

    void resumeCometTimer() {
        EventFunction::getCometEventKeeper()->resumeCometTimer();
    }

    void showCometTimer() {
        EventFunction::getCometEventKeeper()->showCometTimer();
    }

    void hideCometTimer() {
        EventFunction::getCometEventKeeper()->hideCometTimer();
    }

    void addCometTimer(s32 time) {
        EventFunction::getCometEventKeeper()->addCometTimer(time);
    }

    void decCometTimer(s32 time) {
        EventFunction::getCometEventKeeper()->decCometTimer(time);
    }
};  // namespace MR
