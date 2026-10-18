#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class StarPieceTracking;
class StarPieceClusterRock : public LiveActor {
public:
    StarPieceClusterRock(const char*);
    virtual ~StarPieceClusterRock();
    virtual void init(const JMapInfoIter&);
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    void calcStarPieceDirection();
    void exeWait();
    void exeBreak();
    s32 mPieceCount;
    StarPieceTracking* mPieces;
    f32 mSpeed;
    s32 mItemType;
};
