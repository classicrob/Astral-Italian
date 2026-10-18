#pragma once

// Galaxy markers of the world map (SMG2-only): the marker actor, its parts and the per-world holder.

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/DemoObj.hpp"
#include "Game/Screen/GalaxyStatusIcon.hpp"
#include "Game/Screen/MapGalaxyLayouts.hpp"
#include <cmath>

class GalaxyNamePlate;
class WorldMapResultWorld;

/// Wraps an angle in degrees into [base, base + 360) (SMG2 keeps the base in a register across fmod).
inline void wrapAngle(f32& rAngle, f32 base) ALWAYS_INLINE {
    rAngle = base + (f32)fmod(360.0f + (rAngle - base), 360.0);
}

/// PartsModel attached to a world-map object (0xA0 bytes: PartsModel plus its own vtable).
class MiniPartsModel : public PartsModel {
public:
    MiniPartsModel(LiveActor* pHost, const char* pName, const char* pModelName, const TVec3f& rPos);
    MiniPartsModel(LiveActor* pHost, const char* pName, const char* pModelName, const char* pArcName, const TVec3f& rPos);
    virtual ~MiniPartsModel();
    virtual void endClipped();

    void setupCommon(LiveActor* pHost);
};


/// Shadow model of the mini planets.
class MiniGalaxyShadow : public DemoObj {
public:
    MiniGalaxyShadow(LiveActor* pHost, const char* pName, const char* pModelName);

    virtual ~MiniGalaxyShadow();
};

class GalaxyWorldMapInfo : public LiveActor {
public:
    GalaxyWorldMapInfo(WorldMapResultWorld* pWorld, const char* pGalaxyName, const char* pModelName, const char* pType, s32 pointIndex);

    virtual ~GalaxyWorldMapInfo();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();

    bool tryNew() NO_INLINE;
    bool tryNewBoss() NO_INLINE;
    bool tryOpen() NO_INLINE;
    bool isNerveOpened() const NO_INLINE;
    bool isNerveNew() const NO_INLINE;
    bool isNerveNewOpen() const NO_INLINE;
    bool isNerveHide() const NO_INLINE;
    bool startOpenSound(s32 kind) NO_INLINE;
    bool isPointing() NO_INLINE;
    bool updatePointing() NO_INLINE;
    void showNamePlate(bool) NO_INLINE;
    void startPointingAnim() NO_INLINE;
    void endPointingAnim() NO_INLINE;
    void setComet(bool) NO_INLINE;
    void setAllEffectDrawOrder(s32) NO_INLINE;
    void exeNewOpen() NO_INLINE;
    void updateFlag() NO_INLINE;
    bool isBossGalaxy() const NO_INLINE;
    bool hasFlag() const NO_INLINE;
    void setScaleRange(f32 scaleMin, f32 scaleMax) ALWAYS_INLINE {
        mScaleMin = scaleMin;
        mScaleMax = scaleMax;
    }
    bool isMiniOrHide() const ALWAYS_INLINE {
        bool ret = true;
        if (mKind != 1 && mKind != 2) {
            ret = false;
        }
        return ret;
    }
    void updateStatusIcon() NO_INLINE;

    /* 0x90 */ WorldMapResultWorld* mWorld;
    /* 0x94 */ const char* mGalaxyName;
    /* 0x98 */ const char* mType;
    /* 0x9C */ const char* mModelName;
    /* 0xA0 */ s32 mPointIndex;
    /* 0xA4 */ s32 mKind;
    /* 0xA8 */ MiniPartsModel* mShadow;
    /* 0xAC */ MiniPartsModel* mBack;
    /* 0xB0 */ MiniPartsModel* mFlag;
    /* 0xB4 */ GalaxyNamePlate* mNamePlate;
    /* 0xB8 */ GalaxyStatusIcon* mStatusIcon;
    /* 0xBC */ bool mShowEffect;
    /* 0xC0 */ s32 mCometTimer;
    /* 0xC4 */ f32 mScaleMin;
    /* 0xC8 */ f32 mScaleMax;
    /* 0xCC */ TVec3f mNamePlateOffset;
    /* 0xD8 */ TVec3f mPosOffset;
    /* 0xE4 */ f32 mRotateSpeed;
    /* 0xE8 */ TVec3f mBasePos;
};

/// The galaxy markers of one world (WorldMapResultWorld::mWorldData).
class GalaxyWorldMapInfoHolder {
public:
    GalaxyWorldMapInfoHolder(WorldMapResultWorld* pWorld);

    void init();
    bool tryOpen(s32 pointIndex);
    void tryNewBossAll();
    void tryNewAll();
    bool isOpened(s32 pointIndex);
    bool isNewOpen(s32 pointIndex);
    const char* getGalaxyName(s32 pointIndex);
    s32 getPointingPoint();
    bool updatePointing(s32 pointIndex);
    GalaxyWorldMapInfo* find(s32 pointIndex);
    GalaxyWorldMapInfo* findByName(const char* pName);
    void showNamePlate(s32 pointIndex);
    void startAllPointingAnim();
    void appearAll();
    void killAll();
    bool isAllCompleted();
    bool startOpenSoundAll();
    void setAllEffectDrawOrder(s32 order);

    WorldMapResultWorld* mWorld;  // 0x00
    s32 mNum;                     // 0x04
    GalaxyWorldMapInfo** mInfos;  // 0x08
};

/// Layouts shared by the whole world map (WorldMapModel::mLayouts).
class WorldMapLayouts {
public:
    WorldMapLayouts();

    void init();
    void initInfo();
    void killCounters();

    MapGalaxyWorld* mWorldLayout;         // 0x00
    MapGalaxyInfo* mInfoLayout;           // 0x04
    MapGalaxyZoomButton* mZoomButton;     // 0x08
    MapGalaxyCounters* mCounters;         // 0x0C
};
