#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class FloaterFloatingForceTypeNormal;
class ProjmapEffectMtxSetter;
class LodCtrl;

class LavaFloater : public LiveActor {
public:
    LavaFloater(const char*);

    virtual ~LavaFloater();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();

    f32 getCurrentSinkDepth() const;

    void exeFloat();
    void exeSink();

    /* 0x90 */ FloaterFloatingForceTypeNormal* mFloatingForce;
    /* 0x94 */ ProjmapEffectMtxSetter* mMtxSetter;
    /* 0x98 */ LodCtrl* mLodCtrlPlanet;
    /* 0x9C */ const char* mObjectName;
    /* 0xA0 */ TVec3f _A0;
    /* 0xAC */ f32 _AC;
    /* 0xB0 */ bool _B0;
};
