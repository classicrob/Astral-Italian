#include "Game/MapObj/ActorStatePlayerPosReset.hpp"
#include "Game/MapObj/Togepin.hpp"
#include "Game/Screen/ScoreCounterLayout.hpp"
#include "Game/Screen/GameSetLayout.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Screen/ScoreResult.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"




class TogepinScorerSub {
public:
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
};


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
    MR::createScoreResultKeeper();
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
    ActorStatePlayerPosReset* state = new ActorStatePlayerPosReset(this, "\x83\x7D\x83\x8A\x83\x49\x88\xCA\x92\x75", true);
    MR::initActorState(this, state, &sNrvEnd, "ResetPlayer");
    MR::tryRegisterNamePosLinkObj(this, rIter, "\x83\x7D\x83\x8A\x83\x49\x88\xCA\x92\x75");
    mSetLayout = new GameSetLayout;
    mSetLayout->initWithoutIter();
    MR::registerDemoSimpleCastAll((LayoutActor*)mSetLayout);
    mScoreCounter = new ScoreCounterLayout;
    mScoreCounter->initWithoutIter();
    static_cast<ScoreCounterLayout*>(mScoreCounter)->initScore(GameDataFunction::getBestScoreAttackCurrentStage(nullptr), 0x1388);
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::initUseStageSwitchWriteB(this, rIter);
    makeActorAppeared();
}

void TogepinScorer::onTogepinDown(Togepin* pTogepin) {
    static_cast<ScoreCounterLayout*>(mScoreCounter)->addScore(0x64);
    if (pTogepin != nullptr) {
        MR::appearScoreResult(100, pTogepin);
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
        static_cast<ScoreCounterLayout*>(mScoreCounter)->resetScore();
        static_cast<ScoreCounterLayout*>(mScoreCounter)->show();
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
    return static_cast<ScoreCounterLayout*>(mScoreCounter)->mIsNewRecord;
}

s32 TogepinScorer::getScore() const {
    return static_cast<ScoreCounterLayout*>(mScoreCounter)->mScore;
}

s32 TogepinScorer::getBestScore() const {
    return static_cast<ScoreCounterLayout*>(mScoreCounter)->mHighScore;
}

s32 TogepinScorer::getTargetScore() const {
    return static_cast<ScoreCounterLayout*>(mScoreCounter)->mTargetScore;
}

void TogepinScorer::updateNgArea() {
    bool wasEnd = mIsEnd;
    bool isEnd = MR::isCurrentRushRockMario();
    if (!isEnd) {
        if (MR::isOnGroundPlayer()) {
            isEnd = MR::isInAreaObj("TogepinAttackFoulArea", *MR::getPlayerPos());
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
        if (pCounter->mScore >= pCounter->mTargetScore) {
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
        static_cast<ScoreCounterLayout*>(mScoreCounter)->timeUp();
        static_cast<GameSetLayout*>(mSetLayout)->appear();
        ScoreCounterLayout* pCounter = static_cast<ScoreCounterLayout*>(mScoreCounter);
        if (pCounter->mIsNewRecord != 0) {
            GameDataFunction::setBestScoreAttackCurrentStage(pCounter->mHighScore);
        }
        MR::setAudioSystemField9A0(0x5A);
        if (mIsClear) {
            MR::startSubBGM("BGM_RACE_WIN", false);
        } else {
            MR::startSubBGM("BGM_RACE_LOSE", false);
        }
    }
    if (MR::isGreaterStep(this, 0xB4)) {
        static_cast<GameSetLayout*>(mSetLayout)->disappear();
        static_cast<ScoreCounterLayout*>(mScoreCounter)->disappear();
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
