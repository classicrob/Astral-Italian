#pragma auto_inline off
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/ScoreAttackMan.hpp"
#include "Game/NPC/TalkDirector.hpp"
#include "Game/Screen/ScoreCounterLayout.hpp"
#include "Game/Screen/BombTimerLayout.hpp"
#include "Game/Screen/ScoreResult.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Util.hpp"

namespace {
    s32 sTrampleScore[] = {100, 200, 300, 300, 300, 300, 300};
    s32 sSpinScore[] = {100, 200, 300};
    s32 sComboScore[] = {100, 200, 300, 300, 300, 300, 300};
    ScoreAttackCounter* getScoreAttackCounter() NO_INLINE {
        return static_cast<ScoreAttackAccessor*>(MR::getSceneObjHolder()->getObj(SceneObj_ScoreAttackAccessor))->mCounter;
    }
}
namespace NrvScoreAttackCounter {
    class Wait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static Wait sInstance;
    };
    class Counting : public Nerve {
    public:
        virtual void execute(Spine* s) const { static_cast<ScoreAttackCounter*>(s->mExecutor)->exeCounting(); }
        static Counting sInstance;
    };
    class TimeUp : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static TimeUp sInstance;
    };
    class End : public Nerve {
    public:
        virtual void execute(Spine* s) const {
            if (MR::isFirstStep(static_cast<ScoreAttackCounter*>(s->mExecutor))) {
                MR::startScoreAttackResult();
                MR::validateTalkDirector();
            }
        }
        static End sInstance;
    };
    Wait Wait::sInstance;
    Counting Counting::sInstance;
    TimeUp TimeUp::sInstance;
    End End::sInstance;
    inline void TimeUp::execute(Spine* s) const {
        ScoreAttackCounter* host = static_cast<ScoreAttackCounter*>(s->mExecutor);
        if (MR::isStep(host, 60)) host->setNerve(&End::sInstance);
    }
}
ScoreAttackCounter::ScoreAttackCounter(const char* name) : LiveActor(name), mScoreLayout(nullptr), mTimer(nullptr), mTrampleCount(0), mSpinCount(0), mTimeLimit(180) {
    MR::createScoreResultKeeper();
    static_cast<ScoreAttackAccessor*>(MR::getSceneObjHolder()->getObj(SceneObj_ScoreAttackAccessor))->mCounter = this;
}
void ScoreAttackCounter::init(const JMapInfoIter& iter) {
    MR::connectToSceneNpcMovement(this);
    MR::invalidateClipping(this);
    initNerve(&NrvScoreAttackCounter::Wait::sInstance, 1);
    initSound(4, "ScoreAttackCounter", nullptr, TVec3f(0.0f));
    s32 best = GameDataFunction::getBestScoreAttackCurrentStage(nullptr);
    s32 target = 5000;
    MR::getJMapInfoArg0NoInit(iter, &target);
    mScoreLayout = new ScoreCounterLayout();
    mScoreLayout->initWithoutIter();
    mScoreLayout->initScore(best, target);
    mTimeLimit = 180;
    MR::getJMapInfoArg1NoInit(iter, &mTimeLimit);
    mTimer = new BombTimerLayout();
    mTimer->initWithoutIter();
    makeActorAppeared();
}
void ScoreAttackCounter::start() {
    mScoreLayout->resetScore();
    mScoreLayout->show();
    mTimer->setTimeLimit(mTimeLimit * 60);
    mTimer->mDangerTransFrame = 600;
    mTimer->appear();
    setNerve(&NrvScoreAttackCounter::Counting::sInstance);
}
void ScoreAttackCounter::saveBestScore() {
    if (isNewRecord()) {
        GameDataFunction::setBestScoreAttackCurrentStage(mScoreLayout->mHighScore);
    }
}
void ScoreAttackCounter::scoreTrample(const LiveActor* actor) {
    if (mTrampleCount < 7) addScore(sTrampleScore[mTrampleCount], actor);
    else { addScore(sTrampleScore[6], actor); MR::addPlayerLeft(1); }
    if ((s32)mTrampleCount > 0) MR::startActionSound(this, "TrampleCombo", mTrampleCount, -1, -1);
    mTrampleCount++;
}
void ScoreAttackCounter::scoreSpin(const LiveActor* actor) {
    if (mSpinCount < 3) addScore(sSpinScore[mSpinCount], actor);
    else addScore(sSpinScore[2], actor);
    mSpinCount++;
}
void ScoreAttackCounter::scoreCombo(const LiveActor* actor, u32 count) {
    if (count < 7) addScore(sComboScore[count], actor);
    else { addScore(sComboScore[6], actor); MR::addPlayerLeft(1); }
}
void ScoreAttackCounter::control() {
    if (MR::isOnGroundPlayer() || MR::isPlayerSwimming()) mTrampleCount = 0;
    if (!MR::isPlayerSkating()) mSpinCount = 0;
}
bool ScoreAttackCounter::isClear() const { return mScoreLayout->mScore >= mScoreLayout->mTargetScore; }
bool ScoreAttackCounter::isNewRecord() const {
    u32 score = mScoreLayout->mScore;
    return GameDataFunction::getBestScoreAttackCurrentStage(nullptr) < score;
}
void ScoreAttackCounter::addScore(s32 score, const LiveActor* actor) {
    if (!isNerve(&NrvScoreAttackCounter::Wait::sInstance) && !isNerve(&NrvScoreAttackCounter::End::sInstance)) {
        mScoreLayout->addScore(score);
        MR::startActionSound(this, "ScoreUpNormal", -1, -1, -1);
        if (actor) MR::appearScoreResult(score, actor);
    }
}
void ScoreAttackCounter::exeCounting() {
    if (isReadyToFinish()) {
        if (mScoreLayout->mScore >= mScoreLayout->mTargetScore) MR::startActionSound(this, "ScoreSuccess", -1, -1, -1);
        else MR::startActionSound(this, "TimeUp", -1, -1, -1);
        MR::invalidateTalkDirector();
        setNerve(&NrvScoreAttackCounter::TimeUp::sInstance);
    }
}
bool ScoreAttackCounter::isReadyToFinish() const {
    if (MR::isPlayerPowerupTransforming()) return false;
    return mTimer->isReadyToTimeUp();
}
ScoreAttackAccessor::ScoreAttackAccessor() : NameObj("スコアアタックアクセサ"), mCounter(nullptr), mMan(nullptr) {}
namespace MR {
    void startScoreAttack() { getScoreAttackCounter()->start(); }
    void endScoreAttack() { getScoreAttackCounter()->saveBestScore(); }
}
namespace ScoreAttackFunction {
    void scoreByAttack(const LiveActor* actor, u32 msg, const HitSensor* sensor) {
        if (MR::isExistSceneObj(SceneObj_ScoreAttackAccessor) && (!sensor || MR::isSensorPlayerOrRide(sensor))) {
        if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) getScoreAttackCounter()->scoreTrample(actor);
        else if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) getScoreAttackCounter()->scoreSpin(actor);
        else if (MR::isMsgRockAttack(msg)) getScoreAttackCounter()->addScore(100, actor);
        else if (MR::isMsgPlayerHitAll(msg) || MR::isMsgPlayerKick(msg) || MR::isMsgSearchlightAttack(msg)) getScoreAttackCounter()->addScore(100, actor);
        }
    }
    void scoreTrample(const LiveActor* actor) { if (MR::isExistSceneObj(SceneObj_ScoreAttackAccessor)) getScoreAttackCounter()->scoreTrample(actor); }
    void score100Points(const LiveActor* actor) { if (MR::isExistSceneObj(SceneObj_ScoreAttackAccessor)) getScoreAttackCounter()->addScore(100, actor); }
    void score100PointsMultiply(const LiveActor* actor, s32 count) {
        if (MR::isExistSceneObj(SceneObj_ScoreAttackAccessor)) for (s32 i=0;i<count;i++) getScoreAttackCounter()->addScore(100, actor);
    }
    void score10PointsMultiply(s32 count) {
        if (MR::isExistSceneObj(SceneObj_ScoreAttackAccessor)) for (s32 i=0;i<count;i++) getScoreAttackCounter()->addScore(10, nullptr);
    }
    void score1000Points(const LiveActor* actor) { if (MR::isExistSceneObj(SceneObj_ScoreAttackAccessor)) getScoreAttackCounter()->addScore(1000, actor); }
    void score300Points(const LiveActor* actor) { if (MR::isExistSceneObj(SceneObj_ScoreAttackAccessor)) getScoreAttackCounter()->addScore(300, actor); }
    void scoreCombo(const LiveActor* actor, u32 count) { if (MR::isExistSceneObj(SceneObj_ScoreAttackAccessor)) getScoreAttackCounter()->scoreCombo(actor, count); }
}
namespace MR {
    s32 getScoreAttackScore() {
        if (isExistSceneObj(SceneObj_ScoreAttackAccessor) == false) return -1;
        return getScoreAttackCounter()->mScoreLayout->mScore;
    }
    s32 getScoreAttackTargetScore() {
        if (isExistSceneObj(SceneObj_ScoreAttackAccessor) == false) return -1;
        return getScoreAttackCounter()->mScoreLayout->mTargetScore;
    }
    bool isScoreAttackFinished() { return getScoreAttackCounter()->isNerve(&NrvScoreAttackCounter::End::sInstance); }
    bool isScoreAttackClear() { return getScoreAttackCounter()->isClear(); }
    bool isScoreAttackFirstClear() { ScoreAttackCounter* counter=getScoreAttackCounter(); return !hasScoreAttackDeclaredPowerStar() && counter->isClear(); }
    bool isScoreAttackNewRecord() { return getScoreAttackCounter()->isNewRecord(); }
    void registerScoreAttackMan(ScoreAttackMan* man) { static_cast<ScoreAttackAccessor*>(getSceneObjHolder()->getObj(SceneObj_ScoreAttackAccessor))->mMan=man; }
    void startScoreAttackResult() { static_cast<ScoreAttackAccessor*>(getSceneObjHolder()->getObj(SceneObj_ScoreAttackAccessor))->mMan->startResult(); }
    bool hasScoreAttackDeclaredPowerStar() { return static_cast<ScoreAttackAccessor*>(getSceneObjHolder()->getObj(SceneObj_ScoreAttackAccessor))->mMan->hasDeclaredPowerStar(); }
    void killScoreAttackLayouts() { ScoreAttackCounter* counter=getScoreAttackCounter(); counter->mTimer->kill();counter->mScoreLayout->kill(); }
    void hideScoreAttackLayouts() { ScoreAttackCounter* counter=getScoreAttackCounter(); hideLayout(counter->mScoreLayout);hideLayout(counter->mTimer); }
    void showScoreAttackLayouts() { ScoreAttackCounter* counter=getScoreAttackCounter(); showLayout(counter->mScoreLayout);showLayout(counter->mTimer); }
}
ScoreAttackCounter::~ScoreAttackCounter() {}
ScoreAttackAccessor::~ScoreAttackAccessor() {}
