#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class LiveActorGroup;

/// @brief A bird made of SandBirdParts that flies along a rail (SMG2 only).
class SandBird : public LiveActor {
public:
    /// @brief Creates a new `SandBird`.
    /// @param pName A pointer to the null-terminated name of the object.
    SandBird(const char* pName);

    virtual ~SandBird();
    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual MtxPtr getBaseMtx() const;
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void startAllPartsMoveSign();
    bool isAllPartsWait();
    void setAllPartsMove();
    void setAllPartsShrink();
    void setAllPartsAppear();

    void exeWait();
    void exeStart();
    void exeCruise();
    void exeStop();
    void exeReturn();

    /* 0x90 */ LiveActorGroup* mPartsGroup;
    /* 0x94 */ s32 _94;
    /* 0x98 */ TPos3f mBaseMtx;
    /* 0xC8 */ TQuat4f mQuat2;
    /* 0xD8 */ TQuat4f mQuat;
    /* 0xE8 */ TVec3f mHomePos;
    /* 0xF4 */ TVec3f mUp;
    /* 0x100 */ TVec3f mPlayerPos;
    /* 0x10C */ TVec3f mZDir;
    /* 0x118 */ s32 mTimer;
    /* 0x11C */ f32 mRailCoordPrev;
    /* 0x120 */ f32 mRailCoord;
    /* 0x124 */ bool mHitFlag;
    /* 0x125 */ bool _125;
    /* 0x126 */ bool mIsPlayerOn;
};
