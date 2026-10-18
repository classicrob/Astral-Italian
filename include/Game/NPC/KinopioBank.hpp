#pragma once

#include "Game/NPC/NPCActor.hpp"

class StarPieceBank;
class TalkMessageCtrl;
class LayoutActor;
class NameObj;
class LiveActor;

class KinopioBank : public NPCActor {
public:
    /// @brief Creates a new `KinopioBank`.
    /// @param pName A pointer to the null-terminated name of the object.
    KinopioBank(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void control();

    s32 branchFunc(u32);
    s32 eventFunc(u32);

    void exeStarPieceWait();
    void exeCountUp();
    void exeEnd();
    void exeWaitBank();
    void exeEvent();

    void createGoodsTables();
    u32 getBankData(LiveActor*** ppGoodsA, LiveActor*** ppGoodsB, s32* pIdx);
    void updateBank(s32);

    /* 0x15C */ s32 _15C;
    /* 0x160 */ s32 _160;
    /* 0x164 */ StarPieceBank* mStarPieceBank;
    /* 0x168 */ s32 mBankIndex;
    /* 0x16C */ NameObj* mChildObj;
    /* 0x170 */ LiveActor** mGoodsA;
    /* 0x174 */ LiveActor** mGoodsB;
    /* 0x178 */ bool mIsFirstTime;
};
