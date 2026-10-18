#include "Game/MapObj/Togepin.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

class GameDataFunction {
public:
    s32 getBestScoreAttackCurrentStage();
    void setBestScoreAttackCurrentStage();
};

class GameSetLayout : public NameObj {
public:
    GameSetLayout();
    virtual void unk2C();
    /* 0x14 */ u8 _14[0x18];
};

class ScoreCounterLayout : public NameObj {
public:
    ScoreCounterLayout();
    /* 0x14 */ u8 _14[0x18];
    /* 0x2C */ s32 _2C;
    /* 0x30 */ s32 _30;
    /* 0x34 */ s32 _34;
    /* 0x38 */ u8 _38;
    /* 0x39 */ u8 _39[0xB];
};

class TogepinScorerSub {
public:
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
};

class TogepinStateObj {
public:
    /* 0x00 */ u8 _00[0x28];
};

extern "C" LiveActor* fn_8026DE50(TogepinStateObj*, TogepinScorer*, const char*, s32);
extern "C" void fn_804904E0(ScoreCounterLayout*, s32);
extern "C" void fn_80490540(ScoreCounterLayout*);
extern "C" void fn_80490580(ScoreCounterLayout*);
extern "C" void fn_80490550(ScoreCounterLayout*, s32, s32);
extern "C" void fn_804905D0(ScoreCounterLayout*);
extern "C" void fn_80490620(ScoreCounterLayout*);
extern "C" void fn_80490D40(s32, s32);
extern "C" void fn_80471DC0(GameSetLayout*);
extern "C" void fn_8005AD00(s32);
extern "C" void fn_80490CD0(void*);

namespace {
    class TogepinScorerNrvIdle : public Nerve {
    public:
        virtual void execute(Spine*) const {}
    };
    class TogepinScorerNrvStart : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class TogepinScorerNrvPlay : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class TogepinScorerNrvEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };

    TogepinScorerNrvIdle sNrvIdle;
    TogepinScorerNrvStart sNrvStart;
    TogepinScorerNrvPlay sNrvPlay;
    TogepinScorerNrvEnd sNrvEnd;
}  // namespace

extern "C" void fn_803249A0(TogepinScorer*);
extern "C" void fn_80324A40(TogepinScorer*);
extern "C" void fn_80324B20(TogepinScorer*);
extern "C" bool fn_80324BC0(TogepinScorer*);

void TogepinScorerNrvStart::execute(Spine* pSpine) const {
    fn_803249A0(reinterpret_cast<TogepinScorer*>(pSpine->mExecutor));
}

void TogepinScorerNrvPlay::execute(Spine* pSpine) const {
    fn_80324A40(reinterpret_cast<TogepinScorer*>(pSpine->mExecutor));
}

void TogepinScorerNrvEnd::execute(Spine* pSpine) const {
    fn_80324B20(reinterpret_cast<TogepinScorer*>(pSpine->mExecutor));
}

// TogepinScorer::TogepinScorer (char const *)
TogepinScorer::TogepinScorer(const char* pName) : LiveActor(pName) {
    mScoreCounter = nullptr;
    mSetLayout = nullptr;
    mTogepinGroup = nullptr;
    _9C = 0;
    mCounter = 0;
    mIsClear = false;
    fn_80490CD0(*(void**)this);
}

// TogepinScorer::init (JMapInfoIter const &)
void TogepinScorer::init(const JMapInfoIter& rIter) {
    MR::connectToSceneNpcMovement(this);
    MR::invalidateClipping(this);
    s32 numTogepin = MR::getChildObjNum(rIter);
    if (numTogepin > 0) {
        mTogepinGroup = new TogepinGroup("\x83\x67\x83\x51\x83\x73\x83\x93\x83\x8A\x83\x58\x83\x67", numTogepin);
    }
    for (s32 i = 0; i < numTogepin; i++) {
        Togepin* pTogepin = new Togepin("\x83\x67\x83\x51\x83\x73\x83\x93", this);
        MR::initChildObj(pTogepin, rIter, i);
        pTogepin->makeActorDead();
        mTogepinGroup->registerActor(pTogepin);
    }
    initSound(3, "TogepinScorer", nullptr, TVec3f(0.0f));
    initNerve(&sNrvIdle, 1);
    TogepinStateObj* pState = new TogepinStateObj;
    LiveActor* pStateActor = (LiveActor*)pState;
    if (pState != nullptr) {
        pStateActor = fn_8026DE50(pState, this, "\x83\x7D\x83\x8A\x83\x49\x88\xCA\x92\x75", 1);
    }
    MR::initActorState(this, (ActorStateBaseInterface*)pStateActor, (Nerve*)&sNrvEnd, "ResetPlayer");
    MR::tryRegisterNamePosLinkObj(this, rIter, "\x83\x7D\x83\x8A\x83\x49\x88\xCA\x92\x75");
    mSetLayout = new GameSetLayout;
    mSetLayout->initWithoutIter();
    MR::registerDemoSimpleCastAll((LayoutActor*)mSetLayout);
    mScoreCounter = new ScoreCounterLayout;
    mScoreCounter->initWithoutIter();
    fn_80490550(static_cast<ScoreCounterLayout*>(mScoreCounter), static_cast<GameDataFunction*>(nullptr)->getBestScoreAttackCurrentStage(), 0x1388);
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::initUseStageSwitchWriteB(this, rIter);
    makeActorAppeared();
}

// fn_803245E0
extern "C" void fn_803245E0(LiveActor* pScorer, Togepin* pTogepin) {
    fn_804904E0(static_cast<ScoreCounterLayout*>(static_cast<TogepinScorer*>(pScorer)->mScoreCounter), 0x64);
    if (pTogepin != nullptr) {
        fn_80490D40(0x64, (s32)pTogepin);
    }
}

// fn_80324630
extern "C" void fn_80324630(TogepinScorer* pScorer) {
    LiveActorGroup* pGroup = pScorer->mTogepinGroup;
    if (pGroup != nullptr) {
        s32 num = pGroup->getObjectCount();
        for (s32 i = 0; i < num; i++) {
            static_cast<Togepin*>(pScorer->mTogepinGroup->getActor(i))->reappear();
        }
    }
}

// fn_803246A0
extern "C" void fn_803246A0(TogepinScorer* pScorer) {
    LiveActorGroup* pGroup = pScorer->mTogepinGroup;
    if (pGroup != nullptr) {
        s32 num = pGroup->getObjectCount();
        for (s32 i = 0; i < num; i++) {
            static_cast<Togepin*>(pScorer->mTogepinGroup->getActor(i))->tryHide();
        }
    }
}

// fn_80324710
extern "C" void fn_80324710(TogepinScorer* pScorer) {
    LiveActorGroup* pGroup = pScorer->mTogepinGroup;
    if (pGroup != nullptr) {
        s32 num = pGroup->getObjectCount();
        for (s32 i = 0; i < num; i++) {
            static_cast<Togepin*>(pScorer->mTogepinGroup->getActor(i))->tryAppear();
        }
    }
}

// fn_80324780
extern "C" void fn_80324780(TogepinScorer* pScorer) {
    if (pScorer->isNerve(&sNrvIdle)) {
        fn_80490540(static_cast<ScoreCounterLayout*>(pScorer->mScoreCounter));
        fn_80490580(static_cast<ScoreCounterLayout*>(pScorer->mScoreCounter));
        fn_80324630(pScorer);
        pScorer->setNerve(&sNrvStart);
        pScorer->mCounter = 0;
        if (MR::isValidSwitchA(pScorer)) {
            MR::onSwitchA(pScorer);
        }
    }
}

// fn_80324800
extern "C" bool fn_80324800(TogepinScorer* pScorer) {
    return pScorer->isNerve(&sNrvIdle) == false;
}

// fn_80324830
extern "C" bool fn_80324830(TogepinScorer* pScorer) {
    return pScorer->mIsClear;
}

// fn_80324840
extern "C" u8 fn_80324840(TogepinScorer* pScorer) {
    return static_cast<ScoreCounterLayout*>(pScorer->mScoreCounter)->_38;
}

// fn_80324850
extern "C" s32 fn_80324850(TogepinScorer* pScorer) {
    return static_cast<ScoreCounterLayout*>(pScorer->mScoreCounter)->_2C;
}

// fn_80324860
extern "C" s32 fn_80324860(TogepinScorer* pScorer) {
    return static_cast<ScoreCounterLayout*>(pScorer->mScoreCounter)->_30;
}

// fn_80324870
extern "C" s32 fn_80324870(TogepinScorer* pScorer) {
    return static_cast<ScoreCounterLayout*>(pScorer->mScoreCounter)->_34;
}

// fn_80324880
extern "C" void fn_80324880(TogepinScorer* pScorer) {
    bool wasEnd = pScorer->mIsEnd;
    bool isEnd = MR::isCurrentRushRockMario();
    if (!isEnd) {
        if (MR::isOnGroundPlayer()) {
            isEnd = MR::isInAreaObj("\x83\x67\x83\x51\x83\x73\x83\x93\x83\x8A\x83\x58\x83\x67", *MR::getPlayerPos());
            pScorer->mIsEnd = isEnd;
        }
    }
    if (wasEnd != pScorer->mIsEnd) {
        if (pScorer->mIsEnd) {
            if (MR::isValidSwitchB(pScorer)) {
                MR::onSwitchB(pScorer);
                MR::startActionSound(pScorer, "NGTrig", -1, -1, -1);
            }
        } else {
            if (MR::isValidSwitchB(pScorer)) {
                MR::offSwitchB(pScorer);
            }
        }
    }
    if (pScorer->mIsEnd) {
        fn_803246A0(pScorer);
        MR::startActionSound(pScorer, "NGLevel", -1, -1, -1);
        pScorer->mCounter += 1;
        return;
    }
    fn_80324710(pScorer);
    pScorer->mCounter = 0;
}

// fn_803249A0
extern "C" void fn_803249A0(TogepinScorer* pScorer) {
    if (MR::isFirstStep(pScorer)) {
        MR::startStageBGM("MBGM_MINI_GAME", false);
        pScorer->mIsEnd = false;
    }
    fn_80324880(pScorer);
    if (fn_80324BC0(pScorer)) {
        ScoreCounterLayout* pCounter = static_cast<ScoreCounterLayout*>(pScorer->mScoreCounter);
        if (pCounter->_2C >= pCounter->_34) {
            pScorer->mIsClear = true;
        }
        MR::curePlayerElementMode();
        MR::offPlayerControl();
        MR::deactivateDefaultGameLayout();
        pScorer->setNerve(&sNrvPlay);
    }
}

// fn_80324A40
extern "C" void fn_80324A40(TogepinScorer* pScorer) {
    if (MR::isFirstStep(pScorer)) {
        fn_804905D0(static_cast<ScoreCounterLayout*>(pScorer->mScoreCounter));
        static_cast<GameSetLayout*>(pScorer->mSetLayout)->unk2C();
        ScoreCounterLayout* pCounter = static_cast<ScoreCounterLayout*>(pScorer->mScoreCounter);
        if (pCounter->_38 != 0) {
            reinterpret_cast<GameDataFunction*>(pCounter->_30)->setBestScoreAttackCurrentStage();
        }
        fn_8005AD00(0x5A);
        if (pScorer->mIsClear) {
            MR::startSubBGM("BGM_RACE_WIN", false);
        } else {
            MR::startSubBGM("BGM_RACE_LOSE", false);
        }
    }
    if (MR::isGreaterStep(pScorer, 0xB4)) {
        fn_80471DC0(static_cast<GameSetLayout*>(pScorer->mSetLayout));
        fn_80490620(static_cast<ScoreCounterLayout*>(pScorer->mScoreCounter));
        pScorer->setNerve(&sNrvEnd);
        MR::onPlayerControl(true);
    }
}

// fn_80324B20
extern "C" void fn_80324B20(TogepinScorer* pScorer) {
    if (MR::isFirstStep(pScorer)) {
        pScorer->mTogepinGroup->makeAllDead();
        MR::startLastStageBGM();
    }
    if (MR::updateActorState(pScorer)) {
        if (pScorer->_9C != nullptr) {
            pScorer->_9C->slot0();
        }
        MR::offPlayerControl();
        MR::deactivateDefaultGameLayout();
        if (MR::isValidSwitchA(pScorer)) {
            MR::offSwitchA(pScorer);
        }
        pScorer->setNerve(&sNrvIdle);
    }
}

// fn_80324BC0
extern "C" bool fn_80324BC0(TogepinScorer* pScorer) {
    if (MR::isValidSwitchA(pScorer) && !MR::isOnSwitchA(pScorer)) {
        return true;
    }
    return false;
}

// TogepinScorer::~TogepinScorer (void)
TogepinScorer::~TogepinScorer() {
}

// TogepinGroup::~TogepinGroup (void)
TogepinGroup::~TogepinGroup() {
}
