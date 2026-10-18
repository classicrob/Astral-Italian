#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class BossJugemPlanetCloud : public LiveActor {
public:
    BossJugemPlanetCloud(const char*);
    virtual ~BossJugemPlanetCloud();
    virtual void init(const JMapInfoIter&);
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    void exeWhiteToGray();
    void exeWaitGray();
    void exeGrayToWhite();
};
