#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class LodCtrl;
class ModelObj;
class PartsModel;

class PlanetMap : public LiveActor {
public:
    PlanetMap(const char*, const char* = nullptr);

    virtual ~PlanetMap();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual void connectToScene();                                 // 0x70
    virtual f32 getFarClipDistance() const;                        // 0x74
    virtual s32 getLowMovementType() const;                        // 0x78
    virtual void initDemoActionAppear(const JMapInfoIter&);        // 0x7C
    virtual bool useProjmapMtx() const {                           // 0x80
        return false;
    }
    virtual void initProjmapMtx() {                                // 0x84
    }
    virtual void initSwitchBListener(const JMapInfoIter&);         // 0x88
    virtual void hide();                                           // 0x8C
    virtual void show();                                           // 0x90
    virtual void initMyEffect();                                   // 0x94
    virtual bool tryEmitMyEffect();                                // 0x98
    virtual bool tryDeleteMyEffect();                              // 0x9C

    void initClipping(const JMapInfoIter&);
    void initModel(const char*, const JMapInfoIter&);
    void initBloomModel(const char*);

    /* 0x8C */ const char* mModelName;
    /* 0x90 */ TVec3f _90;
    /* 0x9C */ LodCtrl* mLODCtrl;
    /* 0xA0 */ ModelObj* mBloomModel;
    /* 0xA4 */ PartsModel* mWaterModel;
    /* 0xA8 */ PartsModel* mIndirectModel;
    /* 0xB0 */ s32 mEffectNum;  ///< number of registered "<model>%d" effects, see PlanetMapFunction::verifyEffects
};

class FurPlanetMap : public PlanetMap {
public:
    FurPlanetMap(const char*);

    virtual ~FurPlanetMap();
    virtual void init(const JMapInfoIter&);
};

class RailPlanetMap : public PlanetMap {
public:
    RailPlanetMap(const char*);

    virtual ~RailPlanetMap();
    virtual void init(const JMapInfoIter&);
};

class PlanetMapAnimLow : public PlanetMap {
public:
    PlanetMapAnimLow(const char* p1) : PlanetMap(p1) {
    }

    virtual ~PlanetMapAnimLow();
    virtual s32 getLowMovementType() const;
};

class PlanetMapFarClippable : public PlanetMap {
public:
    PlanetMapFarClippable(const char*);

    virtual void init(const JMapInfoIter&);
    virtual f32 getFarClipDistance() const {
        return mFarClipDistance;
    }

    /* 0xB4 */ f32 mFarClipDistance;
};

struct PlanetMapClippingInfo {
    /* 0x00 */ const char* mName;
    /* 0x04 */ f32 mRadius;
    /* 0x08 */ Vec mOffset;
    /* 0x14 */ s32 _14;
};
