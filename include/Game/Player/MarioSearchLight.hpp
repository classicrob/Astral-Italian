#pragma once

#include "Game/LiveActor/PartsModel.hpp"

class MarioSearchLight : public PartsModel {
public:
    MarioSearchLight(LiveActor*);
    virtual ~MarioSearchLight();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void updateHitSensor(HitSensor*);
    virtual void attackSensor(HitSensor*, HitSensor*);

    void fn_803F1E80(HitSensor*) NO_INLINE;
    void fn_803F2160() NO_INLINE;
    void fn_803F21E0() NO_INLINE;
    void fn_803F2280() NO_INLINE;

    /* 0xA0 */ u16 _A0;
    /* 0xA4 */ HitSensor* _A4;
    /* 0xA8 */ bool _A8;
    /* 0xAC */ TVec3f _AC;
    /* 0xB8 */ bool _B8;
    /* 0xBC */ f32 _BC;
    /* 0xC0 */ f32 _C0;
    /* 0xC4 */ u32 _C4;
};
