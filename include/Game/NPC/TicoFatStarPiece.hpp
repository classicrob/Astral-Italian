#pragma once
#include "Game/NPC/NPCActor.hpp"
class FullnessMeter;
class TicoFatStarPieceTransformation;

class TicoFatStarPiece : public NPCActor {
public:
    TicoFatStarPiece(const char*);
    virtual ~TicoFatStarPiece();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    bool eventFunc(u32);
    s32 calcEatSoundPitch() NO_INLINE;
    void startEatSound();
    void startEatCompleteSound();
    bool tryEndEventCamera();
    const char* getActionName(const char*);
    void selectEatMessage(s32);
    void exeReaction();
    void exeWait();
    void exePoint();
    void exeEatWait();
    void exeEat();
    void exeChem();
    void receiveStarPiece(s32);
    void updateStarPieceGift();
    void updatePointing();
    void updateEventCamera();
    f32 calcScale();
    void updateScale(f32);
    FullnessMeter* mFullnessMeter;
    TalkMessageCtrl* mEatTalkCtrl;
    s32 mStarPieceCount;
    s32 mRemainingStarPieceCount;
    s32 mPendingStarPieceCount;
    s32 mNearPlayerTimer;
    s32 mPointingTimer;
    bool mEventRequested;
    s32 mTicoFatId;
    s32 mEatEndSoundTimer;
    s32 mEatSoundTimer;
    TicoFatStarPieceTransformation* mTransformation;
};
