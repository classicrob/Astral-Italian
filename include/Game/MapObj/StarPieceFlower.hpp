#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;

class StarPieceFlower : public LiveActor {
public:
    StarPieceFlower(const char*);

    virtual ~StarPieceFlower();
    virtual void init(const JMapInfoIter&) override;
    virtual void control() override;
    virtual void calcAndSetBaseMtx() override;
    virtual void attackSensor(HitSensor*, HitSensor*) override;
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) override;

    void exeCloseWait();
    void exeOpen();
    void exeSpinReaction();

    /* 0x90 */ TVec3f mUp;
    /* 0x9C */ f32 mAngle;                       // Y rotation in degrees (control)
    /* 0xA0 */ const TVec3f* mTargetPos;         // set from outside (StarPieceFlowerController); null = own transform
    /* 0xA4 */ AnimScaleController* mAnimScaleCtrl;
};
