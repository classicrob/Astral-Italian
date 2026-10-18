#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class StarPieceTracking;
class GliderBalloon : public LiveActor {
public:
    GliderBalloon(const char*);
    virtual ~GliderBalloon();
    virtual void init(const JMapInfoIter&);
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    void exeWait();
    void exeBurst();
    void exeStarPieceBurst();
    void appearStarPieceTracking();
    s32 mPieceCount;
    StarPieceTracking* mPieces;
    f32 mPieceSpeed;
    f32 mPieceDistance;
    bool mUsePlayerVelocity;
    TVec3f mEffectOffset;
};
