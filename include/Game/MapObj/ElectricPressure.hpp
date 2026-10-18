#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ElectricPressureBullet;

class ElectricPressure : public LiveActor {
public:
    ElectricPressure(const char*);

    virtual ~ElectricPressure();
    virtual void init(const JMapInfoIter&) override;
    void exeShotStart();
    void exeShot();
    virtual void attackSensor(HitSensor*, HitSensor*) override;
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) override;
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*) override;
    void shoot();

    /* 0x90 */ ElectricPressureBullet** mBullets;  // 5 bullets, allocated in init
    /* 0x94 */ s32 mWaitFrames;                    // !JMap arg1 (isStep count before shot)
    /* 0x98 */ bool mIsBulletFlag;                 // !JMap arg7 (passed to shotElectricBullet)
};
