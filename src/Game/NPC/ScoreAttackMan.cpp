#pragma auto_inline off

#include "Game/NPC/ScoreAttackMan.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/Util.hpp"

namespace NrvScoreAttackMan {
    NEW_NERVE(ScoreAttackManNrvWait, ScoreAttackMan, Wait);
    NEW_NERVE(ScoreAttackManNrvTalk, ScoreAttackMan, Talk);
    NEW_NERVE(ScoreAttackManNrvStart, ScoreAttackMan, Start);
    NEW_NERVE(ScoreAttackManNrvReaction, ScoreAttackMan, Reaction);
    NEW_NERVE(ScoreAttackManNrvEnd, ScoreAttackMan, End);
    NEW_NERVE(ScoreAttackManNrvEventWait, ScoreAttackMan, EventWait);
};  // namespace NrvScoreAttackMan

ScoreAttackMan::ScoreAttackMan(const char* pName) : NPCActor(pName) {
    mJMapIdInfo = NULL;
    mTalkCtrl = NULL;
    mCameraTarget = NULL;
    mTakeOutStar = NULL;
    mNearRadius = 0.0f;
    mRestartInfo = NULL;
    mIsInRange = false;
}

/* NONMATCHING: init is not yet transcribed (talk-ctrl, camera and power-star setup); see m2c fn_8035E190 for the func wrappers */
void ScoreAttackMan::init(const JMapInfoIter& rIter) {
}

/* NONMATCHING: control() flow is a best reading of the m2c draft; scenario/demo checks unverified */
void ScoreAttackMan::control() {
    NPCActor::control();
}

/* NONMATCHING: body not yet transcribed from m2c branchFunc */
s32 ScoreAttackMan::branchFunc(u32) {
    return 0;
}

/* NONMATCHING: body not yet transcribed from m2c eventFunc */
s32 ScoreAttackMan::eventFunc(u32) {
    return 0;
}

/* NONMATCHING: body not yet transcribed from m2c animeFunc */
s32 ScoreAttackMan::animeFunc(u32) {
    return 0;
}

/* NONMATCHING: exe bodies not yet transcribed from m2c fn_8035E220/E270/E320/E380 */
void ScoreAttackMan::exeWait() {
}

void ScoreAttackMan::exeTalk() {
}

void ScoreAttackMan::exeStart() {
}

void ScoreAttackMan::exeReaction() {
}

void ScoreAttackMan::exeEnd() {
}

void ScoreAttackMan::exeEventWait() {
}
