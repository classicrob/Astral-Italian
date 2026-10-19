#pragma once

#include "Game/MapObj/MapObjActor.hpp"

class ActorCameraInfo;
class PartsModel;

class CapsuleCage : public MapObjActor {
public:
    /// @brief Creates a new `CapsuleCage`.
    /// @param pName A pointer to the null-terminated name of the object.
    CapsuleCage(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void connectToScene(const MapObjActorInitInfo&);
    virtual void initCaseUseSwitchA(const MapObjActorInitInfo&) {};
    virtual void initCaseNoUseSwitchA(const MapObjActorInitInfo&) {};
    virtual void initCaseUseSwitchB(const MapObjActorInitInfo&);
    virtual void initCaseNoUseSwitchB(const MapObjActorInitInfo&) {};

    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    void startOpen();

    void exeWait();
    void exeStartCamera();
    void exeOpen();
    void exeEndCamera();

    /* 0xD0 */ ActorCameraInfo* mCameraInfo;
    bool mNoKeyHole;
    bool mRestartBgm;
    bool mRockCover;
    PartsModel* mFrame;
    bool mPlayRiddle;
};
