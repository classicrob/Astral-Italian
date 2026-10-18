#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ActorPoseTQ;
class MapPartsRailMover;
class WalkerStateBindStarPointer;

/// @brief Garigari enemy (SMG2 only, no SMG1 source). Rotates on a rail as a sign, then falls when released by a switch or star pointer.
class Garigari : public LiveActor {
public:
    Garigari(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);

    void fn_80182140();
    void fn_801821A0();
    void exeWait();
    void exeRotateSign();
    void exeRotate();
    void exeFall();
    void exeSupportFreeze();

    /* 0x14 */ TVec3f mUnk14;
    /* 0x20 */ u8 _20[0x24];
    /* 0x44 */ TVec3f mUnk44;
    /* 0x50 */ u8 _50[0x40];
    /* 0x90 */ ActorPoseTQ* mPose;
    /* 0x94 */ MapPartsRailMover* mRailMover;
    /* 0x98 */ WalkerStateBindStarPointer* mBindState;
    /* 0x9C */ TVec3f mUnk9C;
    /* 0xA8 */ s32 mType;
};
