#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/MapObj/MoveBlockFunction.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class CollisionParts;
class HipDropMoveBlock;
class HipDropMoveBlockSwitch;

/// Per-switch CSV placement (rows "Offset" and "Rotate").
struct HipDropMoveBlockSwitchParam {
    /* 0x00 */ TVec3f mOffset;
    /* 0x0C */ TVec3f mRotate;
};

class HipDropMoveBlock : public LiveActor {
public:
    HipDropMoveBlock(const char*);

    virtual ~HipDropMoveBlock();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    bool tryInitSwitch();
    bool isValidSensorHost(HitSensor*);

    void exeWaitSwitch();
    void exeMove();

    /* 0x90 */ MoveBlockFunction* mMoveFunction;
    /* 0x94 */ bool mFlag94;
    /* 0x95 */ bool mFlag95;
    /* 0x96 */ bool mFlag96;
    /* 0x97 */ bool mFlag97;
    /* 0x98 */ s32 mSwitchCount;
    /* 0x9C */ HipDropMoveBlockSwitch** mSwitches;
    /* 0xA0 */ HipDropMoveBlockSwitchParam** mSwitchParams;
    /* 0xA4 */ HipDropMoveBlockSwitch* mActiveSwitch;
    /* 0xA8 */ u8 _A8[0xC];  // SMG2 allocation extends to 0xB4; no accesses recovered yet.
};

class HipDropMoveBlockSwitch : public LiveActor {
public:
    HipDropMoveBlockSwitch(const char*, HipDropMoveBlock*, MtxPtr, const TVec3f&, const TVec3f&);

    virtual ~HipDropMoveBlockSwitch();
    virtual void init(const JMapInfoIter&);
    virtual void calcAnim();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    bool tryOff();

    void exeOff();
    void exeSwitchDown();

    /* 0x90 */ HipDropMoveBlock* mMoveBlock;
    /* 0x94 */ TVec3f mOffset;
    /* 0xA0 */ TVec3f mRotate;
    /* 0xAC */ TPos3f mBaseMtx;
    /* 0xDC */ CollisionParts* mCollisionParts;
};
