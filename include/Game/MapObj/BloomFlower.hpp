#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class StarPiece;
class BloomFlower : public LiveActor {
public:
    BloomFlower(const char*);
    virtual ~BloomFlower();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    void exeGrow();
    void exeWaitGrow();
    void exeWither();
    void exeWaitWither();
    bool tryGrow();
    bool tryWither();
    StarPiece* mStarPiece;
};
