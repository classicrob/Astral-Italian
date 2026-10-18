#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class FlashingCtrl;
class MapPartsRailMover;
class PartsModel;

class ChipBase : public LiveActor {
public:
    enum Type { Type_Blue = 0, Type_Yellow = 1 };

    ChipBase(const char*, s32, const char*);

    virtual ~ChipBase() {
    }

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void initModel(const JMapInfoIter&);
    void initSensor();
    void initShadow(const JMapInfoIter&);
    void initJMapParam(const JMapInfoIter&);
    void deactive();
    void setGroupID(s32);
    void setHost(LiveActor*);
    void appearWait();
    void appearFlashing(s32);
    bool requestGet(HitSensor*, HitSensor*);
    bool requestShow();
    bool requestHide();
    bool requestStartControl();
    bool requestEndControl();
    bool requestNigeroDrop();
    void returnToWait();
    void startFall();
    void exeDeactive();
    void exeWait();
    void exeControled();
    void exeFall();
    void exeFlashing();
    void exeHide();
    bool hasAirBubble() const {
        return mAirBubble != nullptr;
    }
    void exeGot();
    bool isGettable() const;
    bool isNigeroTakeable() const;
    static bool isNeedBubble(const JMapInfoIter&);

    /* 0x90 */ FlashingCtrl* mFlashingCtrl;
    /* 0x94 */ MapPartsRailMover* mRailMover;
    /* 0x98 */ PartsModel* mAirBubble;
    /* 0x9C */ const char* mChipName;
    /* 0xA0 */ LiveActor* mHost;
    /* 0xA4 */ TVec3f mClippingRange;
    /* 0xB0 */ s32 mGroupID;
    /* 0xB4 */ s32 mChipType;
    /* 0xB8 */ s32 mCounter;
    /* 0xBC */ u8 mPad_BC;
    /* 0xBD */ bool mIsCalcShadow;
};
