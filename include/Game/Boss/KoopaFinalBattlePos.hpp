#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry.hpp>

// SMG2-only: position marker of the final Bowser battle (pose B).
class KoopaFinalBattlePos : public LiveActor {
public:
    KoopaFinalBattlePos(const char*);

    virtual ~KoopaFinalBattlePos();
    virtual void init(const JMapInfoIter&);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
};

// SMG2-only: fire ball spat by the final Bowser.
class KoopaFireBall : public LiveActor {
public:
    KoopaFireBall(const char*);

    virtual ~KoopaFireBall();
    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);

    void shot(s32);
    bool isShotOrLand() const;
    void exeShot();
    void exeLand();
    void exeBreak();

    /* 0x90 */ TQuat4f mQuat;
    /* 0xA0 */ s32 mCount;
};
