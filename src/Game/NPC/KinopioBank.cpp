#pragma auto_inline off

#include "Game/NPC/KinopioBank.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/Util.hpp"

namespace NrvKinopioBank {
    NEW_NERVE(KinopioBankNrvWait, KinopioBank, WaitBank);
    NEW_NERVE(KinopioBankNrvAppear, KinopioBank, Event);
    NEW_NERVE(KinopioBankNrvStarPieceWait, KinopioBank, StarPieceWait);
    NEW_NERVE(KinopioBankNrvCountUp, KinopioBank, CountUp);
    NEW_NERVE(KinopioBankNrvEnd, KinopioBank, End);
    NEW_NERVE(KinopioBankNrvAction, KinopioBank, Event);
};  // namespace NrvKinopioBank

KinopioBank::KinopioBank(const char* pName) : NPCActor(pName) {
    mStarPieceBank = NULL;
    mBankIndex = 0;
    mChildObj = NULL;
    mGoodsA = NULL;
    mGoodsB = NULL;
    mIsFirstTime = false;
}

/* NONMATCHING: init is only partially transcribed (scale/talk/branch wiring); NPCActor-side 0x100..0x140 writes not yet placed */
void KinopioBank::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps("KinopioBank");
    caps.setDefault();
    initialize(rIter, caps, "KinopioBank", "KinopioBank", false);
}

void KinopioBank::control() {
    NPCActor::control();

    if (mIsFirstTime) {
        MR::startActionSound(this, "KinopioBank", -1, -1, -1);
    }
    if (isPointingSe()) {
        MR::startDPDHitSound();
        MR::startActionSound(this, "KinopioBank", -1, -1, -1);
    }
}

/* NONMATCHING: body not yet transcribed from m2c fn_8034E700 */
void KinopioBank::exeStarPieceWait() {
}

/* NONMATCHING: body not yet transcribed from m2c fn_8034E770 */
void KinopioBank::exeCountUp() {
}

/* NONMATCHING: body not yet transcribed from m2c fn_8034E7F0 */
void KinopioBank::exeEnd() {
}

/* NONMATCHING: body not yet transcribed from m2c fn_8034E850 (goods table creation) */
void KinopioBank::createGoodsTables() {
}

/* NONMATCHING: body not yet transcribed from m2c fn_8034E970 */
u32 KinopioBank::getBankData(LiveActor*** ppGoodsA, LiveActor*** ppGoodsB, s32* pIdx) {
    return 0;
}

/* NONMATCHING: body not yet transcribed from m2c fn_8034EA30 */
void KinopioBank::updateBank(s32) {
}

/* NONMATCHING: body not yet transcribed from m2c KinopioBank::branchFunc */
s32 KinopioBank::branchFunc(u32) {
    return 0;
}

/* NONMATCHING: body not yet transcribed from m2c KinopioBank::eventFunc */
s32 KinopioBank::eventFunc(u32) {
    return 0;
}

/* NONMATCHING: body not yet transcribed from m2c fn_8034EF10 */
void KinopioBank::exeEvent() {
}
