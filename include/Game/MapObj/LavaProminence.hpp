#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class PartsModel;
class SupportTicoStop;
class ModelObj;

class LavaProminence : public LiveActor {
public:
    /// @brief Creates a new `LavaProminence`.
    /// @param pName A pointer to the null-terminated name of the object.
    LavaProminence(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void kill();
    virtual void startClipped();
    virtual void calcAndSetBaseMtx();
    virtual void control();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);

    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void moveOnRail();
    void setGravityAndMakeMtx();
    void updateEffectClipping(bool);

    void exeWaitSwitch();
    void exeWait();
    void exeSign();
    void exeMoveStartExtra();
    void exeMoveStart();
    void exeMoveLoop();
    void exeMoveEnd();
    void exeMoveEndExtra();

    bool isNrvMove();

    /* 0x090 */ SupportTicoStop* mTicoStop;
    /* 0x094 */ s32 mWaitTime;
    /* 0x098 */ f32 mMoveSpeed;
    /* 0x09C */ f32 mMoveRailCoord;
    /* 0x0A0 */ f32 mMoveRailEndOffset;
    /* 0x0A4 */ TVec3f mRailStartDir;
    /* 0x0B0 */ TVec3f mRailEndDir;
    /* 0x0BC */ TVec3f mRailStartPos;
    /* 0x0C8 */ TVec3f mRailEndPos;
    /* 0x0D4 */ TPos3f mEffectStartMtx;
    /* 0x104 */ TPos3f mEffectEndMtx;
    /* 0x134 */ TVec3f _12C;
    /* 0x140 */ ModelObj* mBloomModel;
    /* 0x144 */ TPos3f mBloomModelMtx;
    /* 0x174 */ TQuat4f mBaseQuat;
    /* 0x184 */ TVec3f mRailDir;
    /* 0x190 */ bool mEmitDropEffect;
    /* 0x194 */ TPos3f _18C;
};
