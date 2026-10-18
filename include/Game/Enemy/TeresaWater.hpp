#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class WalkerStateBindStarPointer;

class TeresaWater : public LiveActor {
public:
    /// @brief Creates a new `TeresaWater`.
    /// @param pName A pointer to the null-terminated name of the object.
    TeresaWater(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void exeMove();
    void exeHit();
    void exeStun();
    void exeDPDSwoon();
    void endDPDSwoon();
    bool tryDPDSwoon();

    /* 0x90 */ AnimScaleController* _8C;
    /* 0x94 */ WalkerStateBindStarPointer* _90;
    /* 0x98 */ TVec3f _94;
    /* 0xA4 */ TVec3f _A0;
    /* 0xB0 */ TVec3f _AC;
    /* 0xBC */ TVec3f* _B8;
    /* 0xC0 */ bool _C0;
};
