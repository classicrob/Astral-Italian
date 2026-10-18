#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class StarPieceCluster : public LiveActor {
public:
    StarPieceCluster(const char*);
    virtual ~StarPieceCluster();
    virtual void init(const JMapInfoIter&);
    virtual void attackSensor(HitSensor*, HitSensor*);
    void exeBreak();
    LiveActor* mChild;
};
