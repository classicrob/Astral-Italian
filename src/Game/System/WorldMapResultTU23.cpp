#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util/TalkUtil.hpp"

class TalkMessageCtrl;

extern "C" {
    TalkMessageCtrl* fn_804F8000(WorldMapResultCommand*, const char*);
    void* fn_804A88E0();
    void fn_804A89F0(void*, const TVec2f*, s32);
    void fn_804A8A40(void*);
    TVec2f* fn_804F71A0(WorldMapResultCommand*, const char*);
    s32 fn_804E8950(WorldMapResultCommand*);
    bool fn_804E8CA0(WorldMapResultCommand*);
    bool fn_804F6A80(WorldMapResultCommand*);
    void fn_804F7FB0(WorldMapResultCommand*, const TVec2f*);
    void fn_804F7F90(WorldMapResultCommand*);
    bool fn_804F7FE0(WorldMapResultCommand*);
    void fn_804F7FC0(WorldMapResultCommand*);
    void fn_804F7FD0(WorldMapResultCommand*);
    bool fn_804F7FF0(WorldMapResultCommand*);
    void fn_804E8780(WorldMapResultCommand*);
    void fn_804E8850(WorldMapResultCommand*);
}

extern const char* lbl_807039D8[12];
extern TVec2f lbl_8072BB30;
extern char lbl_807038E0[];

/// World-news state ("State[ワールドニュース]"): talk controls for the stage news, one per entry.
WMR_STATE_DECL(WorldMapResultStateWorldNews, virtual void control(); bool eventFunc(u32); bool checkStep(); void setIndexFlag(); bool isIndexInRange() const; void nextTalkNerve(); void exeStart(); void exeTalk(); void exeEnd(); void exeSave(); void startNextTalk();
               /* 0x10 */ s32 mIndex; /* 0x14 */ TalkMessageCtrl** mTalkCtrls; /* 0x18 */ u8 _18[0xC]; /* 0x24 */ void* mPanel;);

namespace NrvWorldMapResultStateWorldNews {
    WMR_NERVE_DECL(WorldMapResultStateWorldNewsWait);
    WMR_NERVE_DECL(WorldMapResultStateWorldNewsStart);
    WMR_NERVE_DECL(WorldMapResultStateWorldNewsTalk);
    WMR_NERVE_DECL(WorldMapResultStateWorldNewsEnd);
    WMR_NERVE_DECL(WorldMapResultStateWorldNewsSave);
};  // namespace NrvWorldMapResultStateWorldNews

/// Talk-callback object of the world-news state: the state's member function, as registered with the talk ctrls.
typedef TalkMessageFuncM< WorldMapResultStateWorldNews*, bool (WorldMapResultStateWorldNews::*)(u32) > WorldMapResultNewsTalkFunc;

WorldMapResultStateWorldNews::WorldMapResultStateWorldNews(WorldMapResultCommand* pHost) : ActorStateBase< WorldMapResultCommand >("State[ワールドニュース]", pHost) {
    mIndex = -1;
    mTalkCtrls = NULL;
}

void WorldMapResultStateWorldNews::init() {
    initNerve(GET_NERVE(WorldMapResultStateWorldNews, WorldMapResultStateWorldNewsStart), 0);
    mTalkCtrls = new TalkMessageCtrl*[12];
    for (s32 i = 0; i < 12; i++) {
        mTalkCtrls[i] = fn_804F8000(mHost, lbl_807039D8[i]);
        MR::registerEventFunc(mTalkCtrls[i], WorldMapResultNewsTalkFunc(this, &WorldMapResultStateWorldNews::eventFunc));
    }
    mPanel = fn_804A88E0();
    void** pVtable = *reinterpret_cast< void*** >(mPanel);
    reinterpret_cast< void (*)(void*) >(pVtable[0x30 / 4])(mPanel);
    mIsDead = true;
}

void WorldMapResultStateWorldNews::appear() {
    mIsDead = false;
    void** pVtable = *reinterpret_cast< void*** >(mPanel);
    reinterpret_cast< void (*)(void*) >(pVtable[0x30 / 4])(mPanel);
    fn_804E8780(mHost);
    setNerve(GET_NERVE(WorldMapResultStateWorldNews, WorldMapResultStateWorldNewsWait));
}

void WorldMapResultStateWorldNews::kill() {
    fn_804E8850(mHost);
    fn_804A8A40(mPanel);
    mIsDead = true;
}

bool WorldMapResultStateWorldNews::isIndexInRange() const {
    if (mIndex >= 1 && mIndex <= 5) {
        return true;
    }
    return false;
}

void WorldMapResultStateWorldNews::control() {
}

bool WorldMapResultStateWorldNews::eventFunc(u32) {
    fn_804A89F0(mPanel, fn_804F71A0(mHost, "YosshiHomeGalaxy"), 60);
    return true;
}

bool WorldMapResultStateWorldNews::checkStep() {
    s32 stage = fn_804E8950(mHost);
    if (fn_804E8CA0(mHost) != 0) {
        switch (stage) {
        case 1:
            mIndex = 0;
            return true;
        case 2:
            mIndex = 1;
            return true;
        case 3:
            mIndex = 2;
            return true;
        case 4:
            mIndex = 3;
            return true;
        case 5:
            mIndex = 4;
            return true;
        case 6:
            mIndex = 5;
            return true;
        case 7:
            mIndex = 6;
            return true;
        }
        return false;
    }
    if (fn_804F6A80(mHost) != 0) {
        switch (stage) {
        case 2:
            if (!GameDataFunction::isOnGameEventFlag(lbl_807038E0 + 0x134)) {
                mIndex = 7;
                return true;
            }
            break;
        case 3:
            if (!GameDataFunction::isOnGameEventFlag(lbl_807038E0 + 0x150)) {
                mIndex = 8;
                return true;
            }
            break;
        case 4:
        case 5:
        case 6:
            if (!GameDataFunction::isOnGameEventFlag(lbl_807038E0 + 0x16C)) {
                mIndex = 9;
                return true;
            }
            if (stage == 5 && !GameDataFunction::isOnGameEventFlag(lbl_807038E0 + 0x188)) {
                mIndex = 10;
                return true;
            }
            if (stage == 6 && !GameDataFunction::isOnGameEventFlag(lbl_807038E0 + 0x1A4)) {
                mIndex = 11;
                return true;
            }
            break;
        }
    }
    return false;
}

void WorldMapResultStateWorldNews::setIndexFlag() {
    switch (mIndex) {
    case 0:
        GameDataFunction::onGameEventFlag(lbl_807038E0 + 0x1C0);
        break;
    case 1:
        GameDataFunction::onGameEventFlag(lbl_807038E0 + 0x1D4);
        break;
    case 2:
        GameDataFunction::onGameEventFlag(lbl_807038E0 + 0x1E8);
        break;
    case 3:
        GameDataFunction::onGameEventFlag(lbl_807038E0 + 0x1FC);
        break;
    case 4:
        GameDataFunction::onGameEventFlag(lbl_807038E0 + 0x210);
        break;
    case 5:
        GameDataFunction::onGameEventFlag(lbl_807038E0 + 0x224);
        break;
    case 6:
        GameDataFunction::onGameEventFlag(lbl_807038E0 + 0x238);
        break;
    case 7:
        GameDataFunction::onGameEventFlag(lbl_807038E0 + 0x134);
        break;
    case 8:
        GameDataFunction::onGameEventFlag(lbl_807038E0 + 0x150);
        break;
    case 9:
        GameDataFunction::onGameEventFlag(lbl_807038E0 + 0x16C);
        break;
    case 10:
        GameDataFunction::onGameEventFlag(lbl_807038E0 + 0x188);
        break;
    case 11:
        GameDataFunction::onGameEventFlag(lbl_807038E0 + 0x1A4);
        break;
    }
}

void WorldMapResultStateWorldNews::nextTalkNerve() {
    if (mIndex >= 0 && mIndex <= 11) {
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
            fn_804F7FB0(mHost, &lbl_8072BB30);
        } else {
            fn_804F7F90(mHost);
        }
    }
    if (!fn_804F7FE0(mHost)) {
        setNerve(GET_NERVE(WorldMapResultStateWorldNews, WorldMapResultStateWorldNewsTalk));
    }
}

void WorldMapResultStateWorldNews::exeTalk() {
    if (MR::isFirstStep(this)) {
        fn_804F7FC0(mHost);
    }
    if (MR::tryTalkForceWithoutDemoAtEnd(mTalkCtrls[mIndex])) {
        startNextTalk();
    }
}

void WorldMapResultStateWorldNews::exeEnd() {
    if (MR::isFirstStep(this)) {
        fn_804F7FD0(mHost);
    }
    if (!fn_804F7FF0(mHost)) {
        if (isIndexInRange()) {
            setNerve(GET_NERVE(WorldMapResultStateWorldNews, WorldMapResultStateWorldNewsSave));
            return;
        }
        kill();
    }
}

void WorldMapResultStateWorldNews::exeSave() {
    if (MR::isFirstStep(this)) {
        GameSequenceFunction::startGameDataSaveSequence(false, false, false);
    }
    if (!GameSequenceFunction::isActiveSaveDataHandleSequence()) {
        kill();
    }
}

WorldMapResultStateWorldNews::~WorldMapResultStateWorldNews() {
}

namespace NrvWorldMapResultStateWorldNews {
    WMR_NERVE_DEF(WorldMapResultStateWorldNewsSave, WorldMapResultStateWorldNews, Save);
    WMR_NERVE_DEF(WorldMapResultStateWorldNewsEnd, WorldMapResultStateWorldNews, End);
    WMR_NERVE_DEF(WorldMapResultStateWorldNewsTalk, WorldMapResultStateWorldNews, Talk);
    WMR_NERVE_DEF(WorldMapResultStateWorldNewsStart, WorldMapResultStateWorldNews, Start);
    void WorldMapResultStateWorldNewsWait::execute(Spine* pSpine) const {
        WorldMapResultStateWorldNews* pState = reinterpret_cast< WorldMapResultStateWorldNews* >(pSpine->mExecutor);
        if (MR::isGreaterEqualStep(pState, 60)) {
            pState->setNerve(GET_NERVE(WorldMapResultStateWorldNews, WorldMapResultStateWorldNewsStart));
        }
    }
    WorldMapResultStateWorldNewsWait WorldMapResultStateWorldNewsWait::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvWorldMapResultStateWorldNews
