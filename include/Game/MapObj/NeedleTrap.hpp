#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class CollisionObj;
class NeedleTrap : public LiveActor {
public:
    NeedleTrap(const char*);
    virtual ~NeedleTrap();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    void exeWaitFirst();
    void exeWait();
    void exeAttackSign();
    void exeAttack();
    void exeAttackEnd();
    void exeFreeze();
    bool isNotNerveFreeze() const NO_INLINE;
    CollisionObj* mNeedleCollision;
    s32 mFirstWait;
};
