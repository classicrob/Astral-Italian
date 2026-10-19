#pragma once

#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JMapInfo.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class CollisionParts;

/// Tongari (SMG2-only): a rail walker that turns at rail ends. The "Tongari2D" variant has collision parts.
class Tongari : public LiveActor {
public:
    Tongari(const char*);

    virtual ~Tongari();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeTurn();
    void exeGuard();
    void exeFreeze();
    void rotateOnRail(f32);

    /* 0x90 */ TVec3f mFront;
    /* 0x9C */ TVec3f mUp;
    /* 0xA8 */ f32 mSpeed;
    /* 0xAC */ bool mTurnFlag;
    /* 0xAD */ bool mIs2D;
    /* 0xB0 */ TPos3f mMtx;
    /* 0xE0 */ CollisionParts* mCollisionParts;
    /* 0xE4 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0xE8 */ AnimScaleController* mScaleCtrl;
};
