#pragma once

#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Util/SpringValue.hpp"
#include <JSystem/JGeometry/TQuat.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class CollisionObj;

/// Flower Mimic (SMG2-only): a flower that closes on the player, binds and eats it.
class FlowerMimic : public LiveActor {
public:
    FlowerMimic(const char*);

    virtual ~FlowerMimic();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    bool fn_80180DE0(TPos3f*, const JointControllerInfo&);
    bool fn_80181060() const;
    void fn_801810D0();
    void fn_80181150();
    void fn_801811C0();
    void fn_80181250();
    void fn_801812C0();
    void fn_80181320();

    /* 0x90 */ TQuat4f mQuat;
    /* 0xA0 */ TVec3f mOffset;
    /* 0xAC */ TVec3f mPrevOffset;
    /* 0xB8 */ TVec3f mInitPos;
    /* 0xC4 */ AnimScaleController* mScaleCtrl;
    /* 0xC8 */ SpringValue* mSpring;
    /* 0xCC */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0xD0 */ JointControlDelegator< FlowerMimic >* mJointCtrl;
    /* 0xD4 */ CollisionObj* mCollisionObj;
    /* 0xD8 */ s32 mTimer;
    /* 0xDC */ f32 mAngle;
    /* 0xE0 */ bool mFloorTouchFlag;
};
