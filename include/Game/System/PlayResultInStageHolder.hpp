#pragma once

#include "Game/NameObj/NameObj.hpp"

class JMapIdInfo;

// 8-byte entry used by both lists (value + flag).
struct PlayResultStageEntry {
    PlayResultStageEntry();  // fn_804C8D40

    /* 0x0 */ u32 mValue;
    /* 0x4 */ bool mFlag;
};

// 3-word list header (base, capacity, count) embedded in the holder.
struct PlayResultStageList {
    PlayResultStageList();   // fn_804C8D50
    ~PlayResultStageList();  // fn_804C9410
    void add(const PlayResultStageEntry&);  // fn_804C93E0

    /* 0x0 */ PlayResultStageEntry* mBase;
    /* 0x4 */ s32 mCapacity;
    /* 0x8 */ s32 mCount;
};

class PlayResultInStageHolder : public NameObj {
public:
    PlayResultInStageHolder();
    virtual ~PlayResultInStageHolder();

    void reset();
    void setStageName(const char*);
    const char* getStageName() const;
    void setClearedPowerStarNum(s32);
    s32 getClearedPowerStarNum() const;
    bool addStarPieceNum(s32);
    s32 getClearedStarPieceNum() const;
    void addCoinNum(s32);
    s32 getClearedCoinNum() const;
    bool incPurpleCoinNum();
    s32 getPurpleCoinNum() const;
    void setCollectTicoCoinSaved();
    void setRestartMarioNo(const JMapIdInfo&);
    void clearAfterMiss();
    s32 getPlayerHealth() const;
    void setPadModeDreamer();
    void setPadModeSuperDreamer();
    void clearPadMode();
    void clearStageResult();               // fn_804C8E90
    void setupStageResult();              // fn_804C8ED0
    void updateNewlyOpenedGalaxies();     // fn_804C9170
    bool isNewlyOpenedGalaxy(const char*) const;  // fn_804C9200
    void clearForRetry() NO_INLINE;                 // fn_804C92A0
    void setCoinOneUpNum(s32);            // fn_804C9320

    /* 0x12 */ char mStageName[0x30];
    /* 0x42 */ bool mIsStageResult;
    /* 0x43 */ bool mHasPowerStar;
    /* 0x44 */ bool mIsNotGreenStar;
    /* 0x48 */ s32 mClearedPowerStarNum;
    /* 0x4C */ s32 mClearedStarPieceNum;
    /* 0x50 */ s32 mStarPieceAdded;
    /* 0x54 */ s32 mClearedCoinNum;
    /* 0x58 */ s32 mCoinAdded;
    /* 0x5C */ s32 mPurpleCoinNum;
    /* 0x60 */ bool mIsPurpleCoinFull;
    /* 0x61 */ bool mIsCollectTicoCoinSaved;
    /* 0x62 */ bool mIsTicoCoinSaved;
    /* 0x64 */ s32 mPlayTime;
    /* 0x68 */ bool mIsNewBestTime;
    /* 0x6C */ JMapIdInfo* mRestartMarioNo;
    /* 0x70 */ s32 mMissCount;
    /* 0x74 */ bool mHasRetryGalaxy;
    /* 0x75 */ bool mIsPowerStarAppeared;
    /* 0x76 */ bool mIsLuigiIntruding;  // snapshot of the 'Luigi intruding' game event flag
    /* 0x78 */ s32 mPlayerHealth;
    /* 0x7C */ s32 mCoinOneUpNum;
    /* 0x80 */ bool mIsUnlockGhostLuigi;
    /* 0x84 */ s32 mPadMode;  // non-zero => star pointer mode
    /* 0x88 */ bool mIsSuperDreamer;
    /* 0x8C */ PlayResultStageList mNewStageList;
    /* 0x98 */ PlayResultStageList mResultStageList;
};
