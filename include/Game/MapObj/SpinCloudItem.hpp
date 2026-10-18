#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class SpinCloudItem : public LiveActor {
public:
    SpinCloudItem(const char*);
    virtual ~SpinCloudItem();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    bool updatePlayerPowerUp();
    static void enablePlayerResourceCloud();
    void exeAppear();
    void exeWait();
    void exeHide();
};
