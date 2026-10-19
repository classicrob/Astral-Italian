#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class WalkerStateBindStarPointer;

class NokonokoLandBig : public LiveActor {
public:
    // SMG2-only functions (original names unknown; named by address)
    void updateRailPoseOnSwitch();
    bool isMovingOrBindNerve();
    bool tryPushEnemyAttack(HitSensor* pSender, HitSensor* pReceiver);
    NokonokoLandBig(const char*);

    virtual ~NokonokoLandBig();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    // Vtable slots 28/29: sensor-driven reactions (drill attack, body touch).
    virtual bool tryDrillFall(HitSensor*, HitSensor*);
    virtual bool tryBodyTouch(HitSensor*, HitSensor*);

    bool isWalkNerve() const;
    bool isWalkOrBindNerve() const;
    bool isAlignedToRail() const;
    void exeWalk();
    void exeTurnStart();
    void exeTurnLoop();
    void exeTurnEnd();
    void exeAttack();
    void exeFall();
    void exeDead();

    /* 0x90 */ s32 mArg0;
    /* 0x94 */ AnimScaleController* mAnimScaleController;
    /* 0x98 */ TQuat4f mQuat;
    /* 0xA8 */ TVec3f mUpVec;
    /* 0xB4 */ TQuat4f mBaseQuat;
    /* 0xC4 */ f32 mRailCoord;
    /* 0xC8 */ WalkerStateBindStarPointer* mBindStarPointerState;
};
