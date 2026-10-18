#pragma once
#include "Game/NPC/NPCActor.hpp"
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
    void* _164;
    s32 _168;
    s32 mStarPieceCount;
    s32 mEatenStarPieceCount;
    void* _174;
    void* _178;
    void* _17C;
    bool mEventRequested;
    s32 _184;
    s32 _188;
    s32 mEatSoundTimer;
    s32 _190;
};
