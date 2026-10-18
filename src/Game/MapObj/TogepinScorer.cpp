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


void TogepinScorerNrvStart::execute(Spine* pSpine) const {
    reinterpret_cast<TogepinScorer*>(pSpine->mExecutor)->exeStart();
}

void TogepinScorerNrvPlay::execute(Spine* pSpine) const {
    reinterpret_cast<TogepinScorer*>(pSpine->mExecutor)->exePlay();
}

void TogepinScorerNrvEnd::execute(Spine* pSpine) const {
    reinterpret_cast<TogepinScorer*>(pSpine->mExecutor)->exeEnd();
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

void TogepinScorer::onTogepinDown(Togepin* pTogepin) {
    fn_804904E0(static_cast<ScoreCounterLayout*>(mScoreCounter), 0x64);
    if (pTogepin != nullptr) {
        fn_80490D40(0x64, (s32)pTogepin);
    }
}

void TogepinScorer::reappearAll() {
    LiveActorGroup* pGroup = mTogepinGroup;
    if (pGroup != nullptr) {
        s32 num = pGroup->getObjectCount();
        for (s32 i = 0; i < num; i++) {
            static_cast<Togepin*>(mTogepinGroup->getActor(i))->reappear();
        }
    }
}

void TogepinScorer::hideAll() {
    LiveActorGroup* pGroup = mTogepinGroup;
    if (pGroup != nullptr) {
        s32 num = pGroup->getObjectCount();
        for (s32 i = 0; i < num; i++) {
            static_cast<Togepin*>(mTogepinGroup->getActor(i))->tryHide();
        }
    }
}

void TogepinScorer::appearAll() {
    LiveActorGroup* pGroup = mTogepinGroup;
    if (pGroup != nullptr) {
        s32 num = pGroup->getObjectCount();
        for (s32 i = 0; i < num; i++) {
            static_cast<Togepin*>(mTogepinGroup->getActor(i))->tryAppear();
        }
    }
}

void TogepinScorer::start() {
    if (isNerve(&sNrvIdle)) {
        fn_80490540(static_cast<ScoreCounterLayout*>(mScoreCounter));
        fn_80490580(static_cast<ScoreCounterLayout*>(mScoreCounter));
        reappearAll();
        setNerve(&sNrvStart);
        mCounter = 0;
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
    }
}

bool TogepinScorer::isPlaying() const {
    return isNerve(&sNrvIdle) == false;
}

bool TogepinScorer::isClear() const {
    return mIsClear;
}

bool TogepinScorer::isNewRecord() const {
    return static_cast<ScoreCounterLayout*>(mScoreCounter)->_38;
}

s32 TogepinScorer::getScore() const {
    return static_cast<ScoreCounterLayout*>(mScoreCounter)->_2C;
}

s32 TogepinScorer::getBestScore() const {
    return static_cast<ScoreCounterLayout*>(mScoreCounter)->_30;
}

s32 TogepinScorer::getTargetScore() const {
    return static_cast<ScoreCounterLayout*>(mScoreCounter)->_34;
}

void TogepinScorer::updateNgArea() {
    bool wasEnd = mIsEnd;
    bool isEnd = MR::isCurrentRushRockMario();
    if (!isEnd) {
        if (MR::isOnGroundPlayer()) {
            isEnd = MR::isInAreaObj("\x83\x67\x83\x51\x83\x73\x83\x93\x83\x8A\x83\x58\x83\x67", *MR::getPlayerPos());
            mIsEnd = isEnd;
        }
    }
    if (wasEnd != mIsEnd) {
        if (mIsEnd) {
            if (MR::isValidSwitchB(this)) {
                MR::onSwitchB(this);
                MR::startActionSound(this, "NGTrig", -1, -1, -1);
            }
        } else {
            if (MR::isValidSwitchB(this)) {
                MR::offSwitchB(this);
            }
        }
    }
    if (mIsEnd) {
        hideAll();
        MR::startActionSound(this, "NGLevel", -1, -1, -1);
        mCounter += 1;
        return;
    }
    appearAll();
    mCounter = 0;
}

void TogepinScorer::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startStageBGM("MBGM_MINI_GAME", false);
        mIsEnd = false;
    }
    updateNgArea();
    if (isSwitchOff()) {
        ScoreCounterLayout* pCounter = static_cast<ScoreCounterLayout*>(mScoreCounter);
        if (pCounter->_2C >= pCounter->_34) {
            mIsClear = true;
        }
        MR::curePlayerElementMode();
        MR::offPlayerControl();
        MR::deactivateDefaultGameLayout();
        setNerve(&sNrvPlay);
    }
}

void TogepinScorer::exePlay() {
    if (MR::isFirstStep(this)) {
        fn_804905D0(static_cast<ScoreCounterLayout*>(mScoreCounter));
        static_cast<GameSetLayout*>(mSetLayout)->unk2C();
        ScoreCounterLayout* pCounter = static_cast<ScoreCounterLayout*>(mScoreCounter);
        if (pCounter->_38 != 0) {
            reinterpret_cast<GameDataFunction*>(pCounter->_30)->setBestScoreAttackCurrentStage();
        }
        fn_8005AD00(0x5A);
        if (mIsClear) {
            MR::startSubBGM("BGM_RACE_WIN", false);
        } else {
            MR::startSubBGM("BGM_RACE_LOSE", false);
        }
    }
    if (MR::isGreaterStep(this, 0xB4)) {
        fn_80471DC0(static_cast<GameSetLayout*>(mSetLayout));
        fn_80490620(static_cast<ScoreCounterLayout*>(mScoreCounter));
        setNerve(&sNrvEnd);
        MR::onPlayerControl(true);
    }
}

void TogepinScorer::exeEnd() {
    if (MR::isFirstStep(this)) {
        mTogepinGroup->makeAllDead();
        MR::startLastStageBGM();
    }
    if (MR::updateActorState(this)) {
        if (_9C != nullptr) {
            _9C->slot0();
        }
        MR::offPlayerControl();
        MR::deactivateDefaultGameLayout();
        if (MR::isValidSwitchA(this)) {
            MR::offSwitchA(this);
        }
        setNerve(&sNrvIdle);
    }
}

bool TogepinScorer::isSwitchOff() const {
    if (MR::isValidSwitchA(this) && !MR::isOnSwitchA(this)) {
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
