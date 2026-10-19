#pragma once

#include "Game/LiveActor/LiveActor.hpp"

#define JETTURTLETYPE_RED 1
#define JETTURTLETYPE_GOLD 2

class YoshiLockOnTarget;

class JetTurtle : public LiveActor {
public:
    JetTurtle(const char*);

    virtual ~JetTurtle();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    bool receiveMsgThrow(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual void exeThrowing();
    virtual void resetPosition();
    virtual void reset(u32);

    void exeWait();
    void endWait();
    void exeWait2();
    void endWait2();
    void exeThrowWait();
    void exeTakenReserve();
    void exeTakenStart();
    void exeRestart();
    void exeDrop();

    void init2(const JMapInfoIter&, s32);
    void becomeSlowType();
    void becomeFastType();
    void appearAndTryTaken();
    void resetPositionAndVanish();
    void boundDrop();
    void bound();
    bool isWait() const;
    bool isRestart() const;

    inline bool isNerveWait();
    void setDropRotate(const TVec3f&);
    void startThrowing();
    void throwByYoshi(const TVec3f&, const TVec3f&);
    void dropByYoshi(const TVec3f&, const TVec3f&);

    /* 0x90 */ f32 mPlayerThrowSpeedBonus;
    /* 0x94 */ u16 mShellType;  // (0 = green, 1 = red, 2 = gold)
    /* 0x96 */ u16 mMotionProfileIndex;
    /* 0x98 */ HitSensor* _94;
    /* 0x9C */ HitSensor* _98;
    /* 0xA0 */ TVec3f _9C;
    /* 0xAC */ TVec3f _A8;
    /* 0xB8 */ TVec3f _B4;
    /* 0xC4 */ TVec3f _C0;
    /* 0xD0 */ TVec3f _CC;
    /* 0xDC */ u16 _D8;
    /* 0xDE */ u16 _DA;
    /* 0xE0 */ u32 _DC;
    /* 0xE4 */ u8 _E0;
    /* 0xE5 */ u8 _E1;
    /* 0xE6 */ u8 _E2;
    /* 0xE7 */ u8 _E3;
    /* 0xE8 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xEC */ bool _EC;
};

class GoldenTurtle : public JetTurtle {
public:
    GoldenTurtle(const char* pName) : JetTurtle(pName) {
    }

    virtual ~GoldenTurtle();
    virtual void init(const JMapInfoIter&);
    virtual void resetPosition();
    virtual void reset(u32);
    virtual void exeThrowing();
};
