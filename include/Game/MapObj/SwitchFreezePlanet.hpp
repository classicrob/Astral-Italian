#pragma once

#include "Game/MapObj/MapObjActor.hpp"

class PartsModel;
class CollisionParts;

class SwitchFreezePlanet : public MapObjActor {
public:
    SwitchFreezePlanet(const char*);
    virtual ~SwitchFreezePlanet();
    virtual void init(const JMapInfoIter&);
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual void connectToScene(const MapObjActorInitInfo&);
    virtual void initCaseUseSwitchA(const MapObjActorInitInfo&);
    virtual void initCaseUseSwitchB(const MapObjActorInitInfo&);

    void startFreeze();
    void swapCollisionFreeze();
    void swapCollisionNormal();
    void exeWait();
    void exeFreeze();
    void exeFreezeWait();
    void exeFreezeImmediately();
    void exeThaw();

    /* 0xD0 */ PartsModel* mWaterModel;
    /* 0xD4 */ PartsModel* mIceModel;
    /* 0xD8 */ PartsModel* mIndirectModel;
    /* 0xDC */ CollisionParts* mIceCollision;
    /* 0xE0 */ bool mReplaceBody;
    /* 0xE4 */ s32 mEffectCount;
};
