#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class MirrorReflectionModel;
class ProjmapEffectMtxSetter;
class SpaceInner;

class Sky : public LiveActor {
public:
    /// @brief Creates a new `Sky`.
    /// @param pName A pointer to the null-terminated name of the object.
    Sky(const char* pName);
    Sky(const char* pName, const char* pModelName);

    virtual void init(const JMapInfoIter&);
    virtual void calcAnim();
    virtual void control();
    virtual void initModel(const char*);

    void appearSpaceInner();
    void disappearSpaceInner();

    void exeWait();
    void exeChange();

    // PLACEHOLDER(core): SMG2 Sky 0x90 field, zeroed in ctor, not touched by any Sky fn; resolve when identified
    /* 0x90 */ void* _90;
    /* 0x94 */ SpaceInner* mSpaceInner;
    /* 0x98 */ MirrorReflectionModel* mReflectionModel;
};

class ProjectionMapSky : public Sky {
public:
    ProjectionMapSky(const char*);

    virtual void calcAndSetBaseMtx();
    virtual void initModel(const char*);

    /* 0x9C */ ProjmapEffectMtxSetter* mProjmapEffectMtxSetter;
};
