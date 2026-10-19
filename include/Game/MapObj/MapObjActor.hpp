#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class LodCtrl;
class MapObjActorInitInfo;
class MapPartsRailMover;
class MapPartsRotatorBase;
class MapPartsRailRotator;
class MapPartsRailGuideDrawer;
class MapPartsRailPosture;
class ModelObj;
class MapPartsFunction;
class ProjmapEffectMtxSetter;

// SMG2: per-object camera shake / pad rumble parameters read from the "StageEffectCamera"/"StageEffectPad" csv.
struct MapObjStageEffectCamera {
    MapObjStageEffectCamera() : _0(nullptr), _4(0.0f), _8(0.0f), _C(nullptr), _10(0.0f) {
    }

    /* 0x00 */ const char* _0;
    /* 0x04 */ f32 _4;
    /* 0x08 */ f32 _8;
    /* 0x0C */ const char* _C;
    /* 0x10 */ f32 _10;
};

struct MapObjStageEffectPad {
    MapObjStageEffectPad() : _0(nullptr), _4(nullptr), _8(nullptr), _C(0.0f) {
    }

    /* 0x00 */ const char* _0;
    /* 0x04 */ const char* _4;
    /* 0x08 */ const char* _8;
    /* 0x0C */ f32 _C;
};

class MapObjActor : public LiveActor {
public:
    MapObjActor(const char* pName);
    MapObjActor(const char* pName, const char* pObjName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void kill();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void setPlayAppearSe();
    virtual void connectToScene(const MapObjActorInitInfo& rInfo);
    virtual void initCaseUseSwitchA(const MapObjActorInitInfo& rInfo);
    virtual void initCaseNoUseSwitchA(const MapObjActorInitInfo& rInfo);
    virtual void initCaseUseSwitchB(const MapObjActorInitInfo& rInfo);
    virtual void initCaseNoUseSwitchB(const MapObjActorInitInfo& rInfo);
    virtual void makeSubModels(const JMapInfoIter& rIter, const MapObjActorInitInfo& rInfo);
    virtual void updateProjmapMtx();
    virtual bool tryEmitWaitEffect();
    virtual bool tryDeleteWaitEffect();

    void initialize(const JMapInfoIter& rIter, const MapObjActorInitInfo& rInfo);
    bool isObjectName(const char* pName) const;
    bool tryCreateBreakModel(const MapObjActorInitInfo& rInfo);
    bool initStageEffectCamera();
    bool initStageEffectPad();
    void startMapPartsFunctions();
    void endMapPartsFunctions();
    void pauseMapPartsFunctions();
    void resumeMapPartsFunctions();
    void setStateWait();
    void exeWait();
    void exeMove();
    void exeDone();

    /* 0x90 */ const char* mObjectName;
    /* 0x94 */ LodCtrl* mPlanetLodCtrl;
    /* 0x98 */ ModelObj* mBloomModel;
    /* 0x9C */ ModelObj* mModelObj;
    /* 0xA0 */ ProjmapEffectMtxSetter* mMatrixSetter;
    /* 0xA4 */ MapPartsRailMover* mRailMover;
    /* 0xA8 */ MapPartsRotatorBase* mRotator;
    /* 0xAC */ MapPartsRailRotator* mRailRotator;
    /* 0xB0 */ MapPartsRailPosture* mRailPosture;
    /* 0xB4 */ MapPartsRailGuideDrawer* mRailGuideDrawer;
    /* 0xB8 */ u8 _B8;  // Petari _B4
    /* 0xB9 */ u8 _B9;
    /* 0xBA */ u8 _BA;  // Petari _B5
    /* 0xBB */ u8 _BB;  // Petari _B6 (play appear SE)
    /* 0xBC */ Nerve* mWaitNrv;
    /* 0xC0 */ Nerve* mMoveNrv;
    /* 0xC4 */ Nerve* mDoneNrv;
    /* 0xC8 */ MapObjStageEffectCamera* mStageEffectCamera;
    /* 0xCC */ MapObjStageEffectPad* mStageEffectPad;
};

class MapObjActorUtil {
public:
    static void setupProjmapMtx(MapObjActorInitInfo*, bool, bool);
    static void startAllMapPartsFunctions(const MapObjActor* pActor);
    static void endAllMapPartsFunctions(const MapObjActor* pActor);
    static void pauseAllMapPartsFunctions(const MapObjActor* pActor);
    static void resumeAllMapPartsFunctions(const MapObjActor* pActor);
    static bool isRotatorMoving(const MapObjActor* pActor);
    static bool isRailMoverWorking(const MapObjActor* pActor);
    static bool isRailMoverReachedEnd(const MapObjActor* pActor);
    static f32 getSeesaw1AxisAngularSpeed(const MapObjActor* pActor);
    static void forceRotateSeesaw1Axis(const MapObjActor* pActor, f32);

    static void startRotator(const MapObjActor* pActor);
    static void startRailMover(const MapObjActor* pActor);
    static void endRotator(const MapObjActor* pActor);
    static void startRailMoverDirect(const MapObjActor* pActor);
    static void pauseRotator(const MapObjActor* pActor);
    static void resetRailMoverToInitPos(const MapObjActor* pActor);
    static void startBreak(MapObjActor* pActor);
    static bool tryStartBreak(MapObjActor* pActor);
    static bool isBreakStopped(const MapObjActor* pActor);
    static void killBloomModel(MapObjActor* pActor);
    static void appearBloomModel(MapObjActor* pActor);

    static void setupInitInfoTypical(MapObjActorInitInfo* pInfo, const char* pName);
    static void setupInitInfoColorChangeArg0(MapObjActorInitInfo* pInfo, const JMapInfoIter& rIter);
    static void setupInitInfoTextureChangeArg1(MapObjActorInitInfo* pInfo, const JMapInfoIter& rIter);
    static void setupInitInfoShadowLengthArg2(MapObjActorInitInfo* pInfo, const JMapInfoIter& rIter);
    static void setupInitInfoSeesaw(MapObjActorInitInfo* pInfo, const JMapInfoIter& rIter, const char* pName, f32);
    static void setupInitInfoSimpleMapObj(MapObjActorInitInfo* pInfo);
    static void setupInitInfoPlanet(MapObjActorInitInfo* pInfo);
};
