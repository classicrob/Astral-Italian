#include <cstdio>
#include "Game/Util.hpp"
#include "Game/Enemy/MameMuimui.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Enemy/MameMuimuiScorer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/ActorStatePlayerPosReset.hpp"
#include "Game/Map/StartCountdownLayout.hpp"
#include "Game/Screen/BombTimerLayout.hpp"
#include "Game/Screen/ScoreCounterLayout.hpp"
#include "Game/Screen/TimeUpLayout.hpp"
#include "Game/Screen/ScoreResult.hpp"
#include "Game/System/GameDataFunction.hpp"

// Note: the MameMuimuiTU01 .data/.rodata/.sdata2 ranges (strings, nerve vtables, literals) are not in the split yet.

MameMuimuiPlacementSet::MameMuimuiPlacementSet(s32 num) {
    mPlacements = nullptr;
    mNum = num;
    if (num <= 0) {
        mNum = 1;
    }
    mPlacements = new MameMuimuiPlacement[mNum];
}

MameMuimuiPlacement::MameMuimuiPlacement() {
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mUnusedC = 0;
}

void MameMuimuiPlacementSet::setPos(s32 idx, const TVec3f& rPos) {
    if (idx < 0 || idx >= mNum) {
        return;
    }
    mPlacements[idx].mPos.set(rPos);
}

void MameMuimuiPlacementSet::getPos(s32 idx, TVec3f* pOut) const {
    if (idx < 0 || idx >= mNum) {
        return;
    }
    pOut->set(mPlacements[idx].mPos);
}

namespace {
    class MameMuimuiScorerNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class MameMuimuiScorerNrvResetStart : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class MameMuimuiScorerNrvCountdown : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class MameMuimuiScorerNrvReady : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class MameMuimuiScorerNrvTimeUp : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class MameMuimuiScorerNrvPlay : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class MameMuimuiScorerNrvResetEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };
    class MameMuimuiScorerNrvEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
    };

    MameMuimuiScorerNrvWait sNrvWait;
    MameMuimuiScorerNrvResetStart sNrvResetStart;
    MameMuimuiScorerNrvCountdown sNrvCountdown;
    MameMuimuiScorerNrvReady sNrvReady;
    MameMuimuiScorerNrvTimeUp sNrvTimeUp;
    MameMuimuiScorerNrvPlay sNrvPlay;
    MameMuimuiScorerNrvResetEnd sNrvResetEnd;
    MameMuimuiScorerNrvEnd sNrvEnd;
}  // namespace

// Hole index order per random pattern (9 holes).
static s32 sHoleOrder[4][9] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8},
    {6, 3, 0, 7, 4, 1, 8, 5, 2},
    {8, 7, 6, 5, 4, 3, 2, 1, 0},
    {2, 5, 8, 1, 4, 7, 0, 3, 6},
};

MameMuimuiScorer::MameMuimuiScorer(const char* pName) : LiveActor(pName) {
    mGroups[0] = nullptr;
    mGroups[1] = nullptr;
    mGroups[2] = nullptr;
    mPlacements = nullptr;
    mTimerLayout = nullptr;
    mCountdownLayout = nullptr;
    mTimeUpLayout = nullptr;
    mScoreLayout = nullptr;
    mEndCallback = nullptr;
    mWaves = nullptr;
    mWaveCount = 0;
    mWaveIndex = 0;
    mTargetScore = 500;
    mScoreMultiplier = 1;
    mLevel = 1;
    mWavePattern = 0;
    mIsClear = false;
    mInitialMtx.identity();
    MR::createScoreResultKeeper();
}

// NONMATCHING: register allocation (resource holder / count / new actor swap r27-r28).
void MameMuimuiScorer::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneEnemyMovement(this);
    MR::getJMapInfoMatrixFromRT(rIter, &mInitialMtx);
    const char* pObjName;
    MR::getObjectName(&pObjName, rIter);
    ResourceHolder* pHolder = MR::createAndAddResourceHolder(pObjName);
    loadPositionList(pHolder);
    loadSequence(pHolder);
    if (MR::isObjectName(rIter, "MameMuimuiScorerLv2")) {
        mLevel = 2;
        mTargetScore = 600;
    } else {
        mLevel = 1;
        mTargetScore = 500;
    }
    const s32 num = mPlacements->mNum;
    mGroups[0] = new MameMuimuiGroup("\227\316\203}\203\201\203\200\203C\203\212\203X\203g", num);
    for (s32 i = 0; i < num; i++) {
        MameMuimui* pMuimui = new MameMuimui("\227\316\203}\203\201\203\200\203C");
        pMuimui->setTypeGreen();
        pMuimui->setMuimuiListener(this);
        pMuimui->initWithoutIter();
        mGroups[0]->registerActor(pMuimui);
    }
    mGroups[1] = new MameMuimuiGroup("\211\251\203}\203\201\203\200\203C\203\212\203X\203g", num);
    for (s32 i = 0; i < num; i++) {
        MameMuimui* pMuimui = new MameMuimui("\211\251\203}\203\201\203\200\203C");
        pMuimui->setTypeYellow();
        pMuimui->setMuimuiListener(this);
        pMuimui->initWithoutIter();
        mGroups[1]->registerActor(pMuimui);
    }
    mGroups[2] = new MameMuimuiGroup("\203g\203Q\203\200\203C\203\212\203X\203g", num);
    for (s32 i = 0; i < num; i++) {
        MameMuimui* pMuimui = new MameMuimui("\203g\203Q\203\200\203C");
        pMuimui->setTypeSpike();
        pMuimui->setMuimuiListener(this);
        pMuimui->initWithoutIter();
        mGroups[2]->registerActor(pMuimui);
    }
    initNerve(&sNrvWait, 4);
    ActorStatePlayerPosReset* pStartState;
    if (MR::tryRegisterNamePosLinkObj(this, rIter, "\203Q\201[\203\200\212J\216n\210\312\222u")) {
        pStartState = new ActorStatePlayerPosReset(this, "\203Q\201[\203\200\212J\216n\210\312\222u", true);
    } else {
        pStartState = new ActorStatePlayerPosReset(this, mInitialMtx.toMtxPtr(), TVec3f(0.0f), true);
    }
    ActorStatePlayerPosReset* pEndState;
    if (MR::tryRegisterNamePosLinkObj(this, rIter, "\203Q\201[\203\200\217I\227\271\210\312\222u")) {
        pEndState = new ActorStatePlayerPosReset(this, "\203Q\201[\203\200\217I\227\271\210\312\222u", true);
    } else {
        pEndState = new ActorStatePlayerPosReset(this, mInitialMtx.toMtxPtr(), TVec3f(0.0f), true);
    }
    MR::initActorState(this, pStartState, &sNrvResetStart, "ResetPlayerStart");
    MR::initActorState(this, pEndState, &sNrvResetEnd, "ResetPlayerEnd");
    initSound(4, "MameMuimuiScorer", nullptr, TVec3f(0.0f));
    MR::invalidateClipping(this);
    mWaveIndex = 0;
    MR::initUseStageSwitchWriteA(this, rIter);
    mTimerLayout = new BombTimerLayout();
    mTimerLayout->initWithoutIter();
    mCountdownLayout = new StartCountdownLayout();
    mCountdownLayout->initWithoutIter();
    mTimeUpLayout = new TimeUpLayout();
    mTimeUpLayout->initWithoutIter();
    mScoreLayout = new ScoreCounterLayout();
    mScoreLayout->initWithoutIter();
    mScoreLayout->initScore(GameDataFunction::getBestScoreAttackCurrentStage(nullptr), mTargetScore);
    makeActorAppeared();
}

MameMuimuiGroup::MameMuimuiGroup(const char* pName, int num) : LiveActorGroup(pName, num) {}

#pragma push
#pragma opt_loop_invariants off
void MameMuimuiScorer::loadPositionList(ResourceHolder* pHolder) {
    JMapInfo* pCsv = MR::createCsvParser(pHolder, "PositionList");
    s32 num = MR::getCsvDataElementNum(pCsv);
    mPlacements = new MameMuimuiPlacementSet(num);
    TVec3f axisX, axisY, axisZ;
    MR::calcActorAxis(&axisX, &axisY, &axisZ, this);
    TVec3f pos;
    for (s32 i = 0; i < num; ++i) {
        pos = mPosition;
        TVec3f offset(0.0f);
        MR::getCsvDataVec(&offset, pCsv, "Position", i);
        MR::vecScaleAdd(&pos, &axisX, offset.x);
        MR::vecScaleAdd(&pos, &axisY, offset.y);
        MR::vecScaleAdd(&pos, &axisZ, offset.z);
        mPlacements->setPos(i, pos);
        ModelObj* pModel = MR::createModelObjMapObjDecoration("\217o\214\273\210\312\222u\203\202\203f\203\213", "MuimuiHole", nullptr);
        MR::setPosition(static_cast< LiveActor* >(pModel), pos);
        MR::setQuatUpNoSupport(static_cast< LiveActor* >(pModel), axisY);
    }
}

#pragma pop

void MameMuimuiScorer::registerEndCallback(const MR::FunctorBase& rFunctor) {
    mEndCallback = rFunctor.clone(nullptr);
}

void MameMuimuiScorer::start() {
    if (isNerve(&sNrvWait)) {
        setNerve(&sNrvResetStart);
    }
}

bool MameMuimuiScorer::isPlaying() const {
    return !isNerve(&sNrvWait);
}

bool MameMuimuiScorer::isClear() const {
    return mIsClear;
}

bool MameMuimuiScorer::isNewRecord() const {
    return mScoreLayout->mIsNewRecord;
}

s32 MameMuimuiScorer::getScore() const {
    return mScoreLayout->mScore;
}

s32 MameMuimuiScorer::getBestScore() const {
    return mScoreLayout->mHighScore;
}

s32 MameMuimuiScorer::getTargetScore() const {
    return mScoreLayout->mTargetScore;
}

void MameMuimuiScorer::loadSequence(ResourceHolder* pHolder) {
    JMapInfo* pCsv = MR::createCsvParser(pHolder, "Sequence");
    mWaveCount = MR::getCsvDataElementNum(pCsv);
    mWaves = new MameMuimuiWave[mWaveCount];
    for (s32 i = 0; i < mWaveCount; i++) {
        for (s32 j = 0; j < 9; j++) {
            char name[0x100];
            snprintf(name, 0x100, "Pos%d", j);
            s32 val = 0;
            MR::getCsvDataS32(&val, pCsv, name, i);
            mWaves[i].mTypes[j] = val;
        }
    }
}

void MameMuimuiScorer::control() {
    if (!MR::isDead(mTimerLayout) && mTimerLayout->isReadyToTimeUp()) {
        retreatAll();
        mTimerLayout->kill();
        setNerve(&sNrvTimeUp);
    }
}

void MameMuimuiScorer::scoreHit(s32 points, MameMuimui* pActor) {
    if (isNerve(&sNrvPlay)) {
        s32 score = points * mScoreMultiplier;
        MR::appearScoreResult(score, pActor);
        if (mScoreMultiplier < 1) {
            mScoreMultiplier++;
        }
        if (pActor->isYellowType()) {
            MR::startActionSound(this, "AddScoreL", mScoreMultiplier, -1, -1);
        } else {
            MR::startActionSound(this, "AddScore", mScoreMultiplier, -1, -1);
        }
        mScoreLayout->addScore(score);
    }
}

void MameMuimuiScorer::breakCombo(MameMuimui*) {
    MR::startActionSound(this, "TogeFailure", -1, -1, -1);
    mScoreMultiplier = 1;
}

void MameMuimuiScorer::exeResetStart() {
    if (MR::isFirstStep(this)) {
        MR::curePlayerElementMode();
    }
    if (MR::updateActorState(this)) {
        MR::offPlayerControl();
        MR::deactivateDefaultGameLayout();
        setNerve(&sNrvCountdown);
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
    }
}

void MameMuimuiScorer::exeMuimuiCountdown() {
    if (MR::isFirstStep(this)) {
        resetGameState();
        MR::stopStageBGM(20);
        mCountdownLayout->appear();
        mScoreLayout->show();
    }
    if (MR::isDead(mCountdownLayout)) {
        MR::onPlayerControl(true);
        setNerve(&sNrvReady);
    }
}

void MameMuimuiScorer::exeMuimuiReady() {
    if (MR::isGreaterStep(this, 30)) {
        mTimerLayout->setTimeLimit(2400);
        mTimerLayout->mDangerTransFrame = 600;
        mTimerLayout->appear();
        spawnNextWave();
    }
}

void MameMuimuiScorer::exeMuimuiTimeUp() {
    if (MR::isFirstStep(this)) {
        mTimerLayout->kill();
        mTimeUpLayout->appear();
        mScoreLayout->timeUp();
        MR::stopStageBGM(10);
        if (mScoreLayout->mScore >= mScoreLayout->mTargetScore) {
            MR::startActionSound(this, "Complete", -1, -1, -1);
            mIsClear = true;
        } else {
            MR::startActionSound(this, "TimeUp", -1, -1, -1);
        }
        if (mScoreLayout->mIsNewRecord) {
            GameDataFunction::setBestScoreAttackCurrentStage(mScoreLayout->mHighScore);
        }
    }
    if (MR::isGreaterStep(this, 150)) {
        if (mScoreLayout->mScore < mScoreLayout->mTargetScore) {
            MR::startLastStageBGM();
        }
        mTimeUpLayout->disappear();
        mScoreLayout->disappear();
        setNerve(&sNrvResetEnd);
    }
}

void MameMuimuiScorer::exeMuimuiPlay() {
    if (mGroups[1]->getLivingActorNum() == 0 && mGroups[0]->getLivingActorNum() == 0) {
        s32 num = mGroups[2]->getObjectCount();
        for (s32 i = 0; i < num; i++) {
            static_cast< MameMuimui* >(mGroups[2]->getActor(i))->tryHideMuimui();
        }
    }
    if (isNoneAlive()) {
        spawnNextWave();
    }
}

void MameMuimuiScorer::exeResetEnd() {
    if (MR::updateActorState(this)) {
        MR::offPlayerControl();
        MR::deactivateDefaultGameLayout();
        if (mEndCallback != nullptr) {
            (*mEndCallback)();
        }
        setNerve(&sNrvWait);
        if (MR::isValidSwitchA(this)) {
            MR::offSwitchA(this);
        }
    }
}

void MameMuimuiScorer::resetGameState() {
    mScoreMultiplier = 1;
    mScoreLayout->resetScore();
    mWaveIndex = 0;
    mWavePattern = MR::getRandom((s32)0, (s32)3);
}

void MameMuimuiScorer::spawnNextWave() {
    if (mWaves[mWaveIndex].mTypes[0] < 0) {
        mWaveIndex = 13;
    }
    s32 time;
    if (mWaveIndex < 5) {
        time = 180;
    } else if (mWaveIndex < 9) {
        time = 150;
    } else {
        time = 60;
        if (mWaveIndex < 13) {
            time = 120;
        }
    }
    for (s32 i = 0; i < 9; i++) {
        s32 type = mWaves[mWaveIndex].mTypes[i];
        if (type != 0) {
            TVec3f pos;
            mPlacements->getPos(sHoleOrder[mWavePattern][i], &pos);
            MameMuimui* pMuimui = nullptr;
            switch (type) {
            case 1:
                pMuimui = mGroups[0]->findDeadActor();
                break;
            case 2:
                pMuimui = mGroups[1]->findDeadActor();
                break;
            case 3:
                pMuimui = mGroups[2]->findDeadActor();
                break;
            }
            if (pMuimui != nullptr) {
                pMuimui->respawnAt(pos, time);
            }
        }
    }
    setNerve(&sNrvPlay);
    mWaveIndex++;
}

MameMuimui* MameMuimuiGroup::findDeadActor() {
    if (getDeadActor() != nullptr) {
        return static_cast< MameMuimui* >(getDeadActor());
    }
    return nullptr;
}

void MameMuimuiScorer::retreatAll() {
    s32 i;
    s32 n;
    n = mGroups[0]->getObjectCount();
    for (i = 0; i < n; i++) {
        static_cast< MameMuimui* >(mGroups[0]->getActor(i))->tryHideMuimui();
    }
    n = mGroups[2]->getObjectCount();
    for (i = 0; i < n; i++) {
        static_cast< MameMuimui* >(mGroups[2]->getActor(i))->tryHideMuimui();
    }
    n = mGroups[1]->getObjectCount();
    for (i = 0; i < n; i++) {
        static_cast< MameMuimui* >(mGroups[1]->getActor(i))->tryHideMuimui();
    }
}

bool MameMuimuiScorer::isNoneAlive() {
    if (mGroups[1]->getLivingActorNum() == 0 && mGroups[0]->getLivingActorNum() == 0 && mGroups[2]->getLivingActorNum() == 0) {
        return true;
    }
    return false;
}

MameMuimuiScorer::~MameMuimuiScorer() {}

MameMuimuiGroup::~MameMuimuiGroup() {}

void MameMuimuiScorerNrvEnd::execute(Spine* pSpine) const {
}

void MameMuimuiScorerNrvResetEnd::execute(Spine* pSpine) const {
    reinterpret_cast< MameMuimuiScorer* >(pSpine->mExecutor)->exeResetEnd();
}

void MameMuimuiScorerNrvPlay::execute(Spine* pSpine) const {
    reinterpret_cast< MameMuimuiScorer* >(pSpine->mExecutor)->exeMuimuiPlay();
}

void MameMuimuiScorerNrvTimeUp::execute(Spine* pSpine) const {
    reinterpret_cast< MameMuimuiScorer* >(pSpine->mExecutor)->exeMuimuiTimeUp();
}

void MameMuimuiScorerNrvReady::execute(Spine* pSpine) const {
    reinterpret_cast< MameMuimuiScorer* >(pSpine->mExecutor)->exeMuimuiReady();
}

void MameMuimuiScorerNrvCountdown::execute(Spine* pSpine) const {
    reinterpret_cast< MameMuimuiScorer* >(pSpine->mExecutor)->exeMuimuiCountdown();
}

void MameMuimuiScorerNrvResetStart::execute(Spine* pSpine) const {
    reinterpret_cast< MameMuimuiScorer* >(pSpine->mExecutor)->exeResetStart();
}

void MameMuimuiScorerNrvWait::execute(Spine* pSpine) const {
}
