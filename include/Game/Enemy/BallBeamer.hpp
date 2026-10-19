#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class NameObjArchiveListCollector;
class RingBeam;

class BallBeamer : public LiveActor {
public:
    BallBeamer(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    static void makeArchiveList(NameObjArchiveListCollector*, const JMapInfoIter&);
    void syncSwitchOnA();
    void syncSwitchOffA();
    void syncSwitchOnB();
    void setupAttack();
    bool tryAttack();
    void exeWait();
    void exeAttack();
    void exeInter();

    RingBeam** mBeams;
    /* 0x94 */ bool mIsAttackEnabled;
    /* 0x95 */ u8 mReserved95[4];
    /* 0x9C */ TMtx34f mHeadMtx;
};
