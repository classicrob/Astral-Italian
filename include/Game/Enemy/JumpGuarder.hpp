#pragma once

#include "Game/Enemy/BegomanBaby.hpp"
#include "Game/Enemy/JumpEmitter.hpp"

class ActorCameraInfo;
class PartsModel;
class MultiEventCamera;
class JumpGuarderBaby;

class JumpGuarder : public JumpEmitter {
public:
    JumpGuarder(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    /// @brief Filters out their own babies.
    bool isHit(const LiveActor*) const;

    bool enableAttack();
    void exeHide();
    void exeUp();
    void exeWait();
    void exeDown();
    void exeHopStart();
    void exeHopWait();
    void exeHopJump();
    void exeHopEnd();
    void exePreOpen();
    void addBaby(JumpGuarderBaby*);
    void exeOpen();
    void exeClose();
    void exeInter();

    /// @brief Allocated array of `mNumBabies` babies.
    /* 0x0E4 */ JumpGuarderBaby* mBabies;

    /* 0x0E8 */ s32 mNumBabies;
    /* 0x0EC */ s32 mBumpCooldown;
    /* 0x0F0 */ JumpGuarderBaby* mActiveBabies[4];
    /* 0x100 */ s32 mNumActiveBabies;
    /* 0x104 */ s32 mNumCoins;
    /* 0x108 */ f32 mBabyVelocity;
};

class JumpGuarderBaby : public BegomanBaby {
public:
    JumpGuarderBaby();
};
