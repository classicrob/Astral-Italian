#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class WalkerStateBindStarPointer;

class Shellfish : public LiveActor {
public:
    Shellfish(const char*);

    virtual ~Shellfish();
    virtual void init(const JMapInfoIter&) override;
    virtual void initAfterPlacement() override;
    virtual void calcAndSetBaseMtx() override;
    virtual void control() override;
    virtual void attackSensor(HitSensor*, HitSensor*) override;
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) override;

    void exeWait();
    void exeOpen();
    void exeOpenWait();
    void exeCloseSignal();
    void exeClose();
    void exeShock();
    void exeDPDOpen();
    void initItem(const JMapInfoIter&);
    void initCoin(const JMapInfoIter&) NO_INLINE;
    void initYellowChip(const JMapInfoIter&);
    void initKinokoOneUp(const JMapInfoIter&);
    void startBindItem();
    void endBindItem();
    void putItem();

    /* 0x90 */ AnimScaleController* mScaleController;
    /* 0x94 */ WalkerStateBindStarPointer* mStarPointerState;
    /* 0x98 */ LiveActor* mItem;
    /* 0x9C */ s32 mItemType;
    /* 0xA0 */ bool mIsItemBound;
    /* 0xA4 */ CollisionParts* mLowCollision;
};
