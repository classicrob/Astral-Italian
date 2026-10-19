#pragma once

#include "Game/MapObj/MapObjActor.hpp"

class SimpleClipPartsObj : public MapObjActor {
public:
    SimpleClipPartsObj(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void connectToScene(const MapObjActorInitInfo&);
    virtual void initCaseUseSwitchA(const MapObjActorInitInfo&);
    virtual void initCaseNoUseSwitchA(const MapObjActorInitInfo&);
    virtual void initCaseUseSwitchB(const MapObjActorInitInfo&);
    virtual void initCaseNoUseSwitchB(const MapObjActorInitInfo&);
    virtual void setupInitInfo(const MapObjActorInitInfo&, const JMapInfoIter&);

    void exeWait();

    /* 0xD0 */ u8 _D0;
};

class RailMoveObjClipParts : public SimpleClipPartsObj {
public:
    RailMoveObjClipParts(const char*);

    virtual ~RailMoveObjClipParts();
    virtual void init(const JMapInfoIter&);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual void initCaseUseSwitchB(const MapObjActorInitInfo&);
    virtual void setupInitInfo(const MapObjActorInitInfo&, const JMapInfoIter&);

    /* 0xD4 */ TVec3f mRailClipPos;
};

class RotateMoveObjClipParts : public SimpleClipPartsObj {
public:
    RotateMoveObjClipParts(const char*);

    virtual ~RotateMoveObjClipParts();
    virtual void init(const JMapInfoIter&);
    virtual void setupInitInfo(const MapObjActorInitInfo&, const JMapInfoIter&);
};
