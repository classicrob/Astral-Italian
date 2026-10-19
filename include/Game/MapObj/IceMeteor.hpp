#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class IceMeteorParts;
class IceMeteor : public LiveActor {
public:
    IceMeteor(const char*);
    virtual ~IceMeteor();
    virtual void init(const JMapInfoIter&);
    virtual void attackSensor(HitSensor*, HitSensor*);
    static void initPlayerResource(const JMapInfoIter&);
    void exeWait();
    void exeFall();
    void exeSpread();
    void exeSpreadWait();
    void exeShrink();
    void exeShrinkWait();
    void resetMeteor();
    void updateMeteor();
    bool isPlayerOutsideIce(HitSensor*);
    void spreadIce();
    void shrinkIce();
    bool mIsSpread; // 0x90
    f32 mIceRadius; // 0x94
    TVec3f mFallVelocity; // 0x98
    TVec3f mUp; // 0xA4
    s32 mWaitTime; // 0xB0
    s32 mShrinkWaitTime;
    f32 mFallDistance;
    s32 mFallTime;
    IceMeteorParts* mParts;
};
