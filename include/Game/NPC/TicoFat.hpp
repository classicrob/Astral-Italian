#pragma once

#include "Game/NPC/NPCActor.hpp"

class ActorCameraInfo;
class BenefitItemObj;
class FullnessMeter;
class SpinDriverShootPath;

class TicoFat : public NPCActor {
public:
    TicoFat(const char*);

    virtual ~TicoFat();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual void shootStarPiece();
    virtual void receiveStarPiece(s32);
    virtual void startEat();
    virtual bool enableAppear();
    virtual void initStarPieceSaveData(const JMapInfoIter&);
    virtual void addStarPieceSaveData(s32);
    virtual void disappear(bool);
    virtual void appearInformation() const;

    static void makeArchiveList(NameObjArchiveListCollector*, const JMapInfoIter&);

    const char* getActionName(const char*);
    void setCameraParam();
    bool eventFunc(u32);
    f32 calcScale();
    void setScale(f32);
    void setMessage(s32);
    bool tryMetamorphosis();
    void emitScreenEffect();
    void updateScreenEffect();
    s32 getDanceSeTranspose() const;
    void startAbsorbSound();
    void updateGuidance();
    void updatePointing();
    void updateGoods();
    void exeReaction();
    void exePrep();
    void exeWait();
    void exePoint();
    void exeEat();
    void exeChem();
    void exeTest();
    void exeFullness();
    void exeMeta();
    void exeDemo();
    void exeFly();
    void exeWipeOut();
    void exeWipeIn();
    void exeInfo();
    void exeAfter();

    /* 0x164 */ ActorCameraInfo* mCameraInfo;
    /* 0x168 */ BenefitItemObj* mItem;
    /* 0x16C */ FullnessMeter* mMeter;
    /* 0x170 */ SpinDriverShootPath* mShootPath;
    /* 0x174 */ TalkMessageCtrl* mEatTalkCtrl;
    /* 0x178 */ u8 mIsEventCameraActive;
    /* 0x179 */ bool mStartEat;
    /* 0x17A */ u8 mEatStateReserved0;
    /* 0x17B */ u8 mEatStateReserved1;
    /* 0x17C */ s32 mNearPlayerTimer;
    /* 0x180 */ s32 mPointingTimer;
    /* 0x184 */ TPos3f mScreenEffectMtx;
    /* 0x1B4 */ TPos3f mScreenEffectLightMtx;
    /* 0x1E4 */ s32 mItemType;
    /* 0x1E8 */ s32 mRemainingStarPieces;
    /* 0x1EC */ s32 mRequiredStarPieces;
    /* 0x1F0 */ s32 mCurrentFed;
    /* 0x1F4 */ s32 mSaveDataId;
    /* 0x1F8 */ u32 mShotInterval;
    /* 0x1FC */ s32 mGuidanceId;
    /* 0x200 */ f32 mFlyCoord;
    /* 0x204 */ f32 mFlySpeed;
    /* 0x208 */ s32 mChewSoundTimer;
    /* 0x20C */ s32 mAbsorbSoundCooldown;
};
