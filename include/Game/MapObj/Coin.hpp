#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <revolution.h>

/* FINISHED */

class FlashingCtrl;
class PartsModel;
class MapObjConnector;

class CoinHostInfo {
public:
    CoinHostInfo() {
        mHostActor = nullptr;
        _4 = 0;
        _8 = 0;
        _C = 0;
    }

    /* 0x0 */ const NameObj* mHostActor;
    /* 0x4 */ s32 _4;
    /* 0x8 */ s32 _8;
    /* 0xC */ u32 _C;
};

class Coin : public LiveActor {
public:
    Coin(const char* pName);

    virtual ~Coin();
    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void initShadow(const JMapInfoIter& rIter);
    void setShadowAndPoseModeFromJMapIter(const JMapInfoIter& rIter);
    void setShadowCylinder();

    void exeNonActive();
    void exeFix();
    void exeFixHide() {
    }
    void exeFixTimer();
    void exeMove();
    void exeControled();
    void exeSpinDrained();
    void exeHop();
    void exeSupportTicoThrow();
    void exeGot() {
    }

    void appearFixInit();
    void appearFix();
    void appearFixTimer(s32 lifeTime, s32 cannotTime);
    void appearMove(const TVec3f& rPosition, const TVec3f& rVelocity, s32 lifeTime, s32 cannotTime);
    void appearHop(const TVec3f& rPosition, const TVec3f& rHopDir);

    bool requestGetCoin();
    bool requestStartControl();
    bool requestEndControl();
    bool requestMove();
    bool requestThrow();
    bool requestShow();
    bool requestHide();
    void noticeGetCoin();

    void setLife(s32 lifeTime);
    void setHostInfo(CoinHostInfo* pInfo);
    void setCalcShadowMode();
    void setCannotTime(s32 cannotTime);

    void calcRebouond();
    void attenuateVelocity();
    bool isNeedBubble(const JMapInfoIter& rIter);
    bool canStartMove() const;

    CoinHostInfo* mCoinHostInfo;     // 0x90
    FlashingCtrl* mFlashingCtrl;     // 0x94
    PartsModel* mAirBubble;          // 0x98
    MapObjConnector* mConnector;     // 0x9C
    TVec3f mShadowDropPos;           // 0xA0
    TVec3f mClippingRange;           // 0xAC
    TVec3f _B8;                      // 0xB8 : unused
    s32 mLifeTime;                   // 0xC4
    s32 mCannotTime;                 // 0xC8
    s32 mShadowType;                 // 0xCC
    bool mInWater;                   // 0xD0
    bool mShadowCalcOn;              // 0xD1
    bool mIgnoreGravity;             // 0xD2
    bool mCalcShadowPrivateGravity;  // 0xD3
    bool mIsPurple;                  // 0xD4
    bool mIsInBubble;                // 0xD5
};

namespace MR {
    Coin* createDirectSetCoin(const char* pName);
    Coin* createCoin(const NameObj* pHost, const char* pName);
    Coin* createDirectSetPurpleCoin(const char* pName);
    Coin* createPurpleCoin(const NameObj* pHost, const char* pName);
};  // namespace MR
