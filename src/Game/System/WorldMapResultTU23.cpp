#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Screen/WorldMapArrow.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util/TalkUtil.hpp"
#include "Game/System/WorldMapResultStateWorldNews.hpp"

class TalkMessageCtrl;

extern const char* lbl_807039D8[12];
extern TVec2f lbl_8072BB30;
extern char lbl_807038E0[];

template <>
class TalkMessageFuncM<WorldMapResultStateWorldNews*, bool (WorldMapResultStateWorldNews::*)(u32)> : public TalkMessageFuncBase {
public:
    typedef bool (WorldMapResultStateWorldNews::*Callback)(u32);
    TalkMessageFuncM(WorldMapResultStateWorldNews* caller, Callback callee) : mCaller(caller), mCallee(callee) {}
    TalkMessageFuncM(const TalkMessageFuncM&) NO_INLINE;
    virtual bool operator()(u32 arg) const { return (mCaller->*mCallee)(arg); }
    virtual TalkMessageFuncM* clone() const { return new TalkMessageFuncM(*this); }
    virtual TalkMessageFuncM* clone(JKRHeap* heap) const { return new (heap, 0) TalkMessageFuncM(*this); }
    WorldMapResultStateWorldNews* mCaller;
    Callback mCallee;
};
typedef TalkMessageFuncM<WorldMapResultStateWorldNews*, bool (WorldMapResultStateWorldNews::*)(u32)> WorldMapResultNewsTalkFunc;

WorldMapResultStateWorldNews::WorldMapResultStateWorldNews(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[ワールドニュース]", pHost) {
    mIndex = -1;
    mTalkCtrls = NULL;
}

void WorldMapResultStateWorldNews::init() {
    initNerve(GET_NERVE(WorldMapResultStateWorldNews, WorldMapResultStateWorldNewsStart), 0);
    mTalkCtrls = new TalkMessageCtrl*[12];
    for (s32 i = 0; i < 12; i++) {
        mTalkCtrls[i] = mHost->createMeisterTalk(lbl_807039D8[i]);
        MR::registerEventFunc(mTalkCtrls[i], WorldMapResultNewsTalkFunc(this, &WorldMapResultStateWorldNews::eventFunc));
    }
    mPanel = new WorldMapArrow();
    mPanel->kill();
    mIsDead = true;
}

void WorldMapResultStateWorldNews::appear() {
    mIsDead = false;
    mPanel->kill();
    mHost->startDemo();
    setNerve(GET_NERVE(WorldMapResultStateWorldNews, WorldMapResultStateWorldNewsWait));
}

void WorldMapResultStateWorldNews::kill() {
    mHost->endDemo();
    mPanel->disappear();
    mIsDead = true;
}

// NONMATCHING: regalloc; equivalent signed range comparison lowering (FP order/grouping checked: no FP instructions).
bool WorldMapResultStateWorldNews::isIndexInRange() const {
    u32 inRange = 0;
    if (mIndex >= 1 && mIndex <= 5) {
        inRange = 1;
    }
    return inRange != 0;
}

bool WorldMapResultStateWorldNews::eventFunc(u32) {
    mPanel->start(mHost->findGalaxyByName("YosshiHomeGalaxy"), 60);
    return true;
}

bool WorldMapResultStateWorldNews::checkStep() {
    const char* flags = lbl_807038E0;
    s32 stage = mHost->getWorldId();
    if (mHost->isWorldEventFlagOff()) {
        if (stage == 1) {
            mIndex = 0;
            return true;
        }
        if (stage == 2) {
            mIndex = 1;
            return true;
        }
        if (stage == 3) {
            mIndex = 2;
            return true;
        }
        if (stage == 4) {
            mIndex = 3;
            return true;
        }
        if (stage == 5) {
            mIndex = 4;
            return true;
        }
        if (stage == 6) {
            mIndex = 5;
            return true;
        }
        if (stage == 7) {
            mIndex = 6;
            return true;
        }
    }
    if (mHost->isAllNeededForGrandGalaxy()) {
        if (stage == 2 && !GameDataFunction::isOnGameEventFlag(flags + 0x134)) {
            mIndex = 7;
            return true;
        }
        if (stage == 3 && !GameDataFunction::isOnGameEventFlag(flags + 0x150)) {
            mIndex = 8;
            return true;
        }
        if (stage == 4 && !GameDataFunction::isOnGameEventFlag(flags + 0x16C)) {
            mIndex = 9;
            return true;
        }
        if (stage == 5 && !GameDataFunction::isOnGameEventFlag(flags + 0x188)) {
            mIndex = 10;
            return true;
        }
        if (stage == 6 && !GameDataFunction::isOnGameEventFlag(flags + 0x1A4)) {
            mIndex = 11;
            return true;
        }
    }
    return false;
}

void WorldMapResultStateWorldNews::setIndexFlag() {
    const char* flags = lbl_807038E0;
    if (mIndex == 0) {
        GameDataFunction::onGameEventFlag(flags + 0x1C0);
    }
    if (mIndex == 1) {
        GameDataFunction::onGameEventFlag(flags + 0x1D4);
    }
    if (mIndex == 2) {
        GameDataFunction::onGameEventFlag(flags + 0x1E8);
    }
    if (mIndex == 3) {
        GameDataFunction::onGameEventFlag(flags + 0x1FC);
    }
    if (mIndex == 4) {
        GameDataFunction::onGameEventFlag(flags + 0x210);
    }
    if (mIndex == 5) {
        GameDataFunction::onGameEventFlag(flags + 0x224);
    }
    if (mIndex == 6) {
        GameDataFunction::onGameEventFlag(flags + 0x238);
    }
    if (mIndex == 7) {
        GameDataFunction::onGameEventFlag(flags + 0x134);
    }
    if (mIndex == 8) {
        GameDataFunction::onGameEventFlag(flags + 0x150);
    }
    if (mIndex == 9) {
        GameDataFunction::onGameEventFlag(flags + 0x16C);
    }
    if (mIndex == 10) {
        GameDataFunction::onGameEventFlag(flags + 0x188);
    }
    if (mIndex == 11) {
        GameDataFunction::onGameEventFlag(flags + 0x1A4);
    }
}

// NONMATCHING: regalloc; equivalent range branch folding (FP order/grouping checked: no FP instructions).
void WorldMapResultStateWorldNews::nextTalkNerve() {
    if (mIndex == 0) {
        setNerve(GET_NERVE(WorldMapResultStateWorldNews, WorldMapResultStateWorldNewsTalk));
        return;
    }
    if (mIndex >= 0 && mIndex <= 6) {
        setNerve(GET_NERVE(WorldMapResultStateWorldNews, WorldMapResultStateWorldNewsTalk));
        return;
    }
    if (mIndex >= 7 && mIndex <= 11) {
        setNerve(GET_NERVE(WorldMapResultStateWorldNews, WorldMapResultStateWorldNewsTalk));
        return;
    }
    kill();
}

void WorldMapResultStateWorldNews::startNextTalk() {
    setIndexFlag();
    if (checkStep()) {
        nextTalkNerve();
        return;
    }
    setNerve(GET_NERVE(WorldMapResultStateWorldNews, WorldMapResultStateWorldNewsEnd));
}

void WorldMapResultStateWorldNews::exeStart() {
    if (MR::isFirstStep(this)) {
        if (mIndex == 0) {
            mHost->appearMeisterAtScreenPos(lbl_8072BB30);
        } else {
            mHost->appearMeister();
        }
    }
    if (!mHost->isMeisterTalkStarting()) {
        setNerve(GET_NERVE(WorldMapResultStateWorldNews, WorldMapResultStateWorldNewsTalk));
    }
}

void WorldMapResultStateWorldNews::exeTalk() {
    if (MR::isFirstStep(this)) {
        mHost->setMeisterTalk();
    }
    if (MR::tryTalkForceWithoutDemoAtEnd(mTalkCtrls[mIndex])) {
        startNextTalk();
    }
}

void WorldMapResultStateWorldNews::exeEnd() {
    if (MR::isFirstStep(this)) {
        mHost->setMeisterTalkEndWait();
    }
    if (!mHost->isMeisterTalkEnding()) {
        if (isIndexInRange()) {
            setNerve(GET_NERVE(WorldMapResultStateWorldNews, WorldMapResultStateWorldNewsSave));
            return;
        }
        kill();
    }
}

void WorldMapResultStateWorldNews::exeSave() {
    if (MR::isFirstStep(this)) {
        GameSequenceFunction::startGameDataSaveSequence(true, false, false);
    }
    if (!GameSequenceFunction::isActiveSaveDataHandleSequence()) {
        kill();
    }
}

WorldMapResultNewsTalkFunc::TalkMessageFuncM(const WorldMapResultNewsTalkFunc& other)
    : mCaller(other.mCaller), mCallee(other.mCallee) {}

WorldMapResultStateWorldNews::~WorldMapResultStateWorldNews() {
}

namespace NrvWorldMapResultStateWorldNews {
    void WorldMapResultStateWorldNewsWait::execute(Spine* pSpine) const {
        WorldMapResultStateWorldNews* pState = reinterpret_cast< WorldMapResultStateWorldNews* >(pSpine->mExecutor);
        if (MR::isGreaterEqualStep(pState, 60)) {
            pState->setNerve(GET_NERVE(WorldMapResultStateWorldNews, WorldMapResultStateWorldNewsStart));
        }
    }
    WorldMapResultStateWorldNewsWait WorldMapResultStateWorldNewsWait::sInstance ATTRIBUTE_WEAK;
    WMR_NERVE_DEF(WorldMapResultStateWorldNewsStart, WorldMapResultStateWorldNews, Start);
    WMR_NERVE_DEF(WorldMapResultStateWorldNewsTalk, WorldMapResultStateWorldNews, Talk);
    WMR_NERVE_DEF(WorldMapResultStateWorldNewsEnd, WorldMapResultStateWorldNews, End);
    WMR_NERVE_DEF(WorldMapResultStateWorldNewsSave, WorldMapResultStateWorldNews, Save);
};  // namespace NrvWorldMapResultStateWorldNews

TVec2f lbl_8072BB30(82.0f, 48.0f);
