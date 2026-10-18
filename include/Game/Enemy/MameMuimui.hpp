#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class MameMuimuiListener;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;

// SMG2-only unit (MameMuimui, the bean-spitting plant-like walker). LiveActor = 0x90 bytes.
// The MameMuimuiScorer (MameMuimuiTU01) keeps a MameMuimuiListener sub-object at its 0x90 and
// hands its address to every MameMuimui it spawns (fn_801BF570).
class MameMuimui : public LiveActor {
public:
    MameMuimui(const char* pName);

    virtual ~MameMuimui();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void setTypeGreen();
    void setTypeYellow();
    void setTypeSpike();

    // SMG2-only functions (original names unknown; named by address)
    void fn_801BF570(MameMuimuiListener* pListener);
    void fn_801BF580();
    void fn_801BF610(const TVec3f& rPos, s32 waitTime);
    void fn_801BF6A0();
    bool fn_801BF700(HitSensor* pSender, HitSensor* pReceiver);
    bool fn_801BF790();
    bool fn_801C0180() const NO_INLINE;
    bool fn_801C01E0() const NO_INLINE;
    bool fn_801C0250() const NO_INLINE;
    bool fn_801C02D0() const NO_INLINE;
    bool fn_801C0370() const NO_INLINE;

    void exeAppearSign();
    void exeAppear();
    void exeWait();
    void exeHide();
    void exePress();
    void exeBlow();
    void exeEaten();

public:
    /* 0x90 */ MameMuimuiListener* mListener;
    /* 0x94 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x98 */ WalkerStateBindStarPointer* mStarPointBind;
    /* 0x9C */ TQuat4f mQuat;
    /* 0xAC */ TVec3f mFront;
    /* 0xB8 */ s32 mType;
    /* 0xBC */ s32 mWaitTime;
    /* 0xC0 */ s32 mPoints;
    /* 0xC4 */ YoshiLockOnTarget* mYoshiLockOn;
};
