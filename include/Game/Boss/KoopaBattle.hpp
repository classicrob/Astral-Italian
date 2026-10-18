#pragma once

#include "Game/Boss/KoopaStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry.hpp>

class ActorCameraInfo;
class ActorJointCtrl;
class CollisionObj;
class PartsModel;
class Triangle;
class VolumeDrawer;

// SMG2-only final Bowser battle actor (KoopaVs1 / KoopaLv2 / KoopaVs2 / KoopaLv3 / KoopaVs3).
class KoopaBattle : public LiveActor {
public:
    KoopaBattle(const char*);

    virtual ~KoopaBattle() {}
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void appearDefenseForm();                             // fn_800C8E40
    PartsModel* getDefenseForm() const;                   // fn_800C8F40
    CollisionObj* getDefenseCollision() const;            // fn_800C8F50
    void setPose(const TQuat4f&, const TVec3f&);          // fn_800C8F60
    bool isInvalidTriangle(const Triangle*);              // fn_800C9140
    void startBattle();
    void exeBattle();

    /* 0x90 */ KoopaStateBase* mSequence;
    /* 0x94 */ ActorJointCtrl* mJointCtrl;
    /* 0x98 */ ActorCameraInfo* mCameraInfo;
    /* 0x9C */ PartsModel* mShell;
    /* 0xA0 */ PartsModel* mDefenseForm;
    /* 0xA4 */ CollisionObj* mDefenseCollision;
    /* 0xA8 */ VolumeDrawer* mVolumeDrawer;
    /* 0xAC */ TPos3f mBaseMtx;
    /* 0xDC */ TQuat4f mQuatA;
    /* 0xEC */ TQuat4f mQuatB;
    /* 0xFC */ TVec3f mPos;
    /* 0x108 */ s32 mBattleNo;
    /* 0x10C */ bool _10C;
};
