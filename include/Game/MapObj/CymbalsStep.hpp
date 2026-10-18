#pragma once
#include "Game/MapObj/SimpleMapObj.hpp"
class JointController;
class CollisionParts;
struct JointControllerInfo;
class CymbalsStep : public SimpleSeesawObj {
public:
    CymbalsStep(const char*);
    virtual ~CymbalsStep();
    virtual void init(const JMapInfoIter&);
    virtual void calcAnim();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual void initCaseUseSwitchA(const MapObjActorInitInfo&);
    void exeClose();
    bool move(TPos3f*, const JointControllerInfo&);
    JointController* mJointController;
    CollisionParts* mClosedCollision;
    f32 mCloseRate;
    TPos3f mInitialMtx;
};
