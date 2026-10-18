#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"

class HitSensor;
class KoopaBattle;

// SMG2 final battle (Bowser) state base: host KoopaBattle, three extra pure virtuals (vtable +0x20..+0x28).
class KoopaStateBase : public ActorStateBase< KoopaBattle > {
public:
    KoopaStateBase(const char*, KoopaBattle*);

    virtual ~KoopaStateBase() {}
    virtual void attackSensor(HitSensor*, HitSensor*) = 0;
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) = 0;
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*) = 0;
};
