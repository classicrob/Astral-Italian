#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class SpringValue;
class MapObjConnector;
class CollisionParts;

class HipDropTimerSwitch : public LiveActor {
public:
    HipDropTimerSwitch(const char*);

    virtual ~HipDropTimerSwitch();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void calcAnim();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void updateTimerSE();
    void listenSwitchOnA();
    void listenSwitchOffA();
    void exeOff();
    void exeSwitchDown();
    void exeOn();
    void exeSwitchUp();
    void exeSyncOff();

    /* 0x090 */ u32 _90;
    /* 0x094 */ SpringValue* mSpring;
    /* 0x098 */ MapObjConnector* mMapObjConnector;
    /* 0x09C */ CollisionParts* mCollisionParts;
    /* 0x0A0 */ TPos3f _A0;
    /* 0x0D0 */ s32 mTimerMax;      // In frames
    /* 0x0D4 */ s32 mTimerSFXType;  // 0: A ticking, 1: B ticking
    /* 0x0D8 */ u8 _D8;
    /* 0x0D9 */ bool mWasLightPressed;
    /* 0x0DA */ bool mIsLightPressed;
    /* 0x0DC */ s32 mScreenEffectType;
};
