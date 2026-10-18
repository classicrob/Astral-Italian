#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class MapObjConnector;
class HitSensor;

/// @brief One of the parts (feathers/bits) that a SandBird is made of. SMG2 only.
class SandBirdParts : public LiveActor {
public:
    /// @brief Creates a new `SandBirdParts`.
    /// @param pName A pointer to the null-terminated name of the object.
    /// @param pParent The SandBird that owns this part.
    /// @param pSensor The sensor of the parent, used as the collision body sensor.
    SandBirdParts(const char* pName, LiveActor* pParent, HitSensor* pSensor);

    virtual ~SandBirdParts();
    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    bool updateAppear();
    bool updateShrink();

    void exeWait();
    void exeMoveSign();
    void exeMove();
    void exeBreak();
    void exeHide();
    void exeRevive();
    void exeShrink();
    void exeShrinkHide();
    void exeAppear();

    /* 0x90 */ LiveActor* mParent;
    /* 0x94 */ MapObjConnector* mConnector;
    /* 0x98 */ HitSensor* mBodySensor;
    /* 0x9C */ TPos3f mCollisionMtx;
    /* 0xCC */ TVec3f mBasePos;
    /* 0xD8 */ s32 mStepLimit;
};

// Helpers the parent SandBird calls for each of its parts (SMG2 unnamed fn_802F9600, fn_802F9640,
// fn_802F9680, fn_802F96F0, fn_802F95F0). The part is passed as a LiveActor from the parent's group.
void setPartsMoveSign(LiveActor* pPart);
void setPartsMove(LiveActor* pPart);
void setPartsShrink(LiveActor* pPart);
void setPartsAppear(LiveActor* pPart);
bool isPartsWait(LiveActor* pPart);
