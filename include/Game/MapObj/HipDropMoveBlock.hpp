#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class CollisionParts;
class HipDropMoveBlock;
class HipDropMoveBlockSwitch;

/// Move path / state helper owned by the MoveBlockFunction unit. Layout as far as HipDropMoveBlock uses it.
class MoveBlockFunction {
public:
    MoveBlockFunction();
    virtual ~MoveBlockFunction();
    virtual void init(const JMapInfoIter&);

    void setMoveVec(const TVec3f&);
    void setReturnStep(s32);
    void setMoveStep(s32);
    void calcMoveVec(TVec3f*) const;
    void setForward(bool);
    bool update();
    bool isEndStep() const;
    bool isStartStep() const;
    bool canMove(bool) const;

    /* 0x04 */ u8 _04[0x10];
    /* 0x14 */ TQuat4f mQuat;  // (x, y, z) is read as a TVec3f offset by HipDropMoveBlockSwitch::calcAndSetBaseMtx
    /* 0x24 */ u8 _24[0x10];
    /* 0x34 */ TVec3f mPos;
    /* 0x40 */ u8 _40[0x34];
    /* 0x74 */ bool mFlag74;
    /* 0x75 */ u8 _75[0x3];
};

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
