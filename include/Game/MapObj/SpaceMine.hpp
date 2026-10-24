#pragma once

#include "Game/MapObj/MapObjActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class AnimScaleController;
class BenefitItemObj;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;

class SpaceMine : public MapObjActor {
public:
    SpaceMine(const char* pName);

    virtual ~SpaceMine();
    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    bool isCalcShadowAlways() const;
    void exeWait();
    void exeAppear();
    void exeReaction();
    void exePause();
    bool appearItem();

    inline bool isShadowValid(s32 val) const {
        return -1 < val && val < 2;
    }

    /* 0xD0 */ f32 mRotateSpeed;
    /* 0xD4 */ TVec3f mClippingRange;
    /* 0xE0 */ s32 mShadowType;
    /* 0xE4 */ s32 mItemType;
    /* 0xE8 */ u8 mIsPlayerNear;
    /* 0xEC */ BenefitItemObj* mBenefitItem;
    /* 0xF0 */ TPos3f mKinokoMtx;
    /* 0x120 */ YoshiLockOnTarget* mLockOnTarget;
    /* 0x124 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x128 */ WalkerStateBindStarPointer* mStarPointerState;
};
