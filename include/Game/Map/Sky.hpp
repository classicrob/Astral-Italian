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
    virtual void getModelName(const char**, const JMapInfoIter&);  // 0x70
    virtual void initModel(const char*);                           // 0x74
    virtual void updateWait() {                                    // 0x78
    }

    void appearSpaceInner();
    void disappearSpaceInner();

    void exeWait();
    void exeChange();

    /* 0x90 */ const char* mModelName;  ///< Optional model name override; if null the object name from the JMap is used.
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
