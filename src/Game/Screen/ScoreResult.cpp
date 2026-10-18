#include "Game/Screen/ScoreResult.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"

namespace NrvScoreResult {
    NEW_NERVE(ScoreResultNrvAppear, ScoreResult, Appear);
}

ScoreResult::ScoreResult(const char* name) : LayoutActor(name, true), mStartPosition(0.0f, 0.0f), mScore(0) {}
void ScoreResult::init(const JMapInfoIter&) {
    initLayoutManager("ScoreNumber", 1);
    MR::connectToSceneLayout(this);
    initNerve(&NrvScoreResult::ScoreResultNrvAppear::sInstance);
    kill();
}
void ScoreResult::control() {}
void ScoreResult::exeAppear() {
    f32 offset = MR::calcNerveEaseOutValue(this, 40, -0.0f, -50.0f);
    setTrans(mStartPosition + TVec2f(0.0f, offset));
    if (MR::isGreaterStep(this, 60)) kill();
}
void ScoreResult::appearAt(s32 score, const TVec3f& position) {
    mScore = score;
    MR::calcScreenPosition(&mStartPosition, position);
    setTrans(mStartPosition);
    setNerve(&NrvScoreResult::ScoreResultNrvAppear::sInstance);
    MR::setTextBoxNumberRecursive(this, "ScoreNumber", score);
    appear();
}
ScoreResult::~ScoreResult() {}

ScoreResultKeeper::ScoreResultKeeper() : NameObj("スコア表示保持"), mResults(0), mCapacity(16) {
    mResults = new ScoreResult*[16];
    for (s32 i = 0; i < 16; ++i) {
        ScoreResult* result = new ScoreResult("スコア表示");
        result->initWithoutIter();
        mResults[i] = result;
    }
}
ScoreResult* ScoreResultKeeper::findUnused() {
    for (s32 i = 0; i < 16; ++i) if (MR::isDead(mResults[i])) return mResults[i];
    return 0;
}
void ScoreResultKeeper::killAll() {
    for (s32 i = 0; i < 16; ++i) if (!MR::isDead(mResults[i])) mResults[i]->kill();
}
namespace MR {
    void createScoreResultKeeper() { MR::createSceneObj(SceneObj_ScoreResultKeeper); }
    void appearScoreResult(s32 score, const TVec3f& position) {
        ScoreResult* result = MR::getSceneObj<ScoreResultKeeper>(SceneObj_ScoreResultKeeper)->findUnused();
        if (result) result->appearAt(score, position);
    }
    void appearScoreResult(s32 score, const LiveActor* actor) { appearScoreResult(score, actor->mPosition); }
    void killAllScoreResults() { MR::getSceneObj<ScoreResultKeeper>(SceneObj_ScoreResultKeeper)->killAll(); }
}
ScoreResultKeeper::~ScoreResultKeeper() {}
