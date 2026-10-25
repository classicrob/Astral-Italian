#pragma once

#include "Game/MapObj/MapObjActor.hpp"

class DemoCtrlBase;
class PartsModel;

class RailMoveObj : public MapObjActor {
public:
    RailMoveObj(const char*);
    RailMoveObj(const char*, const char*);

    virtual void init(const JMapInfoIter&);
    virtual void startClipped();
    virtual void endClipped();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual void initCaseUseSwitchB(const MapObjActorInitInfo&);
    virtual void initCaseNoUseSwitchB(const MapObjActorInitInfo&);
    virtual void initDemoAction();
    virtual void setupInitInfo(const JMapInfoIter&, MapObjActorInitInfo*);
    virtual bool tryStartMove();
    virtual void startMoveInner();
    virtual void move();
    virtual void doAtEndPoint();
    virtual bool endMove();
    virtual TVec3f* getSoundCalcPos();

    bool isMoving() const;
    void exeWait();
    void exeMove();
    void exeDemoMove();
    void setNerveMove();
    void setNerveWait();

    /* 0xD0 */ TVec3f _D0;
    /* 0xDC */ u8 mWasRailMoverWorking;
};

class RailMoveObjPress : public RailMoveObj {
public:
    RailMoveObjPress(const char*);

    virtual void init(const JMapInfoIter&);
};

class RailMoveObjSwitchWarpEnd : public RailMoveObj {
public:
    inline RailMoveObjSwitchWarpEnd(const char* pName) : RailMoveObj(pName) {
    }

    virtual bool tryStartMove();
    virtual void startMoveInner();
    virtual void move();
    virtual void doAtEndPoint();

    /* 0xDD */ u8 mIsWarpEffectStarted;
};

class RailMoveObjBreakAtEnd : public RailMoveObj {
public:
    RailMoveObjBreakAtEnd(const char*);

    virtual void doAtEndPoint();
};

class RailMoveObjShakeCamera : public RailMoveObj {
public:
    RailMoveObjShakeCamera(const char*);

    virtual void doAtEndPoint();
};

class RailMoveIndirectObj : public RailMoveObj {
public:
    inline RailMoveIndirectObj(const char* pName) : RailMoveObj(pName) {
    }

    virtual void connectToScene(const MapObjActorInitInfo&);
};

class RailMoveWithIndirectModelObj : public RailMoveObj {
public:
    RailMoveWithIndirectModelObj(const char*);

    virtual void init(const JMapInfoIter&);

    /* 0xE0 */ PartsModel* mIndirectModel;
};

class RailMoveObjSwitchShadow : public RailMoveObj {
public:
    inline RailMoveObjSwitchShadow(const char* pName) : RailMoveObj(pName) {
    }

    virtual void init(const JMapInfoIter&);
    virtual void initCaseUseSwitchA(const MapObjActorInitInfo&);

    void startOnShadow();
};

class RailDemoMoveObj : public RailMoveObj {
public:
    RailDemoMoveObj(const char*);

    virtual void init(const JMapInfoIter&);
    virtual bool tryStartMove();
    virtual void startMoveInner();
    virtual void move();
    virtual bool endMove();

    /* 0xE0 */ DemoCtrlBase* mDemoControl;
};

class RailAndRotateMoveObj : public RailMoveObj {
public:
    RailAndRotateMoveObj(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void setupInitInfo(const JMapInfoIter&, MapObjActorInitInfo*);
};

class RailRotateMoveObj : public RailMoveObj {
public:
    RailRotateMoveObj(const char*);

    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual void initCaseUseSwitchB(const MapObjActorInitInfo&);
    virtual void setupInitInfo(const JMapInfoIter&, MapObjActorInitInfo*);

    bool tryStartRotateAtPoint();
    bool tryStartRotateBetweenPoints();
};

class RailSeesawMoveObj : public RailMoveObj {
public:
    inline RailSeesawMoveObj(const char* pName) : RailMoveObj(pName) {
    }

    virtual void setupInitInfo(const JMapInfoIter&, MapObjActorInitInfo*);
};

class RailMoveDemoActionObj : public RailMoveObj {
public:
    RailMoveDemoActionObj(const char*);

    virtual void initDemoAction();
};

class RailMoveShadowDropYObj : public RailMoveObj {
public:
    RailMoveShadowDropYObj(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void control();

    /* 0xE0 */ TVec3f mShadowDropDir;
};

class RailMoveWithReverseAnimObj : public RailMoveObj {
public:
    inline RailMoveWithReverseAnimObj(const char* pName) : RailMoveObj(pName) {
    }

    virtual void startMoveInner();
    virtual void doAtEndPoint();
};

class RailMoveLavaProjmapObj : public RailMoveObj {
public:
    RailMoveLavaProjmapObj(const char*);

    virtual void initAfterPlacement();
    virtual void updateProjmapMtx();

    /* 0xE0 */ TVec3f mGroundPos;
    /* 0xEC */ f32 mGroundDistance;
    /* 0xF0 */ f32 _F0;
};
