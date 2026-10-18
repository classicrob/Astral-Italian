#pragma auto_inline off
#include "Game/NPC/Meister.hpp"
#include "Game/Util.hpp"
TalkMessageCtrl* MeisterTalkHolder::getTalkCtrl() {
    TalkMessageCtrl* ctrl = mMainTalk;
    if (!ctrl) ctrl = getEventTalk();
    if (!ctrl) ctrl = getGeneralTalk();
    if (!ctrl) ctrl = mDefaultTalk;
    return ctrl;
}
// NONMATCHING: commutative add operands are reversed.
void MeisterTalkHolder::nextTalk() { mTalkIndex = (mEventTalkCount + mTalkIndex + 1) % mEventTalkCount; }
bool MeisterTalkHolder::branchFunc(u32 arg) {
    switch (arg) { case 0: return mStarCount >= 5; case 1: return _38; case 2: return MR::isConnectedWPad(1); }
    return false;
}
TalkMessageCtrl* MeisterTalkHolder::getEventTalk() {
    if (mEventTalkCount > 0) return mEventTalks[mTalkIndex];
    return 0;
}
// NONMATCHING: register allocation and argument evaluation scheduling differ.
TalkMessageCtrl* MeisterTalkHolder::getGeneralTalk() {
    s32 index = -1;
    for (s32 i = 0; i < 15; i++) {
        if (mGeneralTalkFlags[i] && isGeneralTalkEnabled(i)) { index = i; break; }
    }
    if (index != -1) {
        if (index == 6) { TalkMessageCtrl** ctrls = mGeneralTalks; s32 count = MR::getStarPieceNum(); MR::setMessageArg(ctrls[index], count); }
        return mGeneralTalks[index];
    }
    return 0;
}
TalkMessageCtrl* MeisterTalkHolder::getReactionTalk() { return mReactionTalk; }
