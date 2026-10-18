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

    bool branchFunc(u32);
    bool eventFunc(u32);

    void exeStarPieceWait();
    void exeCountUp();
    void exeEnd();
    void awardBankLives();
    bool needsGoodsUpdate();

    void createGoodsTables();
    u32 getBankData(PartsModel** ppGoodsA, PartsModel** ppGoodsB);
    void updateBank(s32);

    /* 0x164 */ StarPieceBank* mStarPieceBank;
    /* 0x168 */ s32 mBankIndex;
    /* 0x16C */ LiveActor* mChildObj;
    /* 0x170 */ PartsModel** mGoodsA;
    /* 0x174 */ PartsModel** mGoodsB;
    /* 0x178 */ bool mIsFirstTime;
};
