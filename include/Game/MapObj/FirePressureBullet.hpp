#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class AnimScaleController;
class WalkerStateBindStarPointer;

class FirePressureBullet : public LiveActor {
public:
    FirePressureBullet(const char*);

    virtual ~FirePressureBullet();
    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);

    void shotFireBullet(LiveActor*, const TPos3f&, const f32&, bool, bool);
    void exeFly();
    void exeBindStarPointer();
    bool isCrash() const;

    /* 0x90 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x94 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0x98 */ TVec3f _8C;
    /* 0xA4 */ LiveActor* mFirePressure;
    /* 0xA8 */ f32 _9C;
    /* 0xAC */ bool _A0;
    /* 0xAD */ bool _A1;
};
