#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class CollisionParts;

class ChangeRoom : public LiveActor {
public:
    ChangeRoom(const char*);

    virtual ~ChangeRoom();
    virtual void init(const JMapInfoIter&) override;
    virtual void makeActorDead() override;
    virtual void calcAnim() override;
    virtual void attackSensor(HitSensor*, HitSensor*) override;
    void exeWait();
    void exeOpen();
    void exeOpenWait();
    void exeEnterDemo();
    void startEnterSoundAndPrepareReset();
    void resetCollisionPartsMtx();

    /* 0x90 */ CollisionParts** mCollisionParts;
};
