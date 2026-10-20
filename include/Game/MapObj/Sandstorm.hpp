#pragma once

#include "Game/LiveActor/ModelObj.hpp"

class ActorCameraInfo;
class NameObjArchiveListCollector;

class SunakazeKunTrap : public ModelObj {
public:
    SunakazeKunTrap(LiveActor*, const TVec3f&);

    virtual void init(const JMapInfoIter& rIter);
    virtual void attackSensor(HitSensor*, HitSensor*);

    void exeTrap();

private:
    /* 0x90 */ LiveActor* mHost;
    /* 0x94 */ TVec3f mTrapOffset;
};

class Sandstorm : public LiveActor {
public:
    enum Type {
        /* 0x00 */ Type_TwisterSan,
        /* 0x01 */ Type_SunakazeKun,
        /* 0x02 */ Type_TwisterSanSlim,
    };

    /// @brief Creates a new `Sandstorm`.
    /// @param pName A pointer to the null-terminated name of the object.
    Sandstorm(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void updateHitSensor(HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    static void makeArchiveList(NameObjArchiveListCollector*, const JMapInfoIter&);

    static void enablePlayerTornado();
    void initSandstormParam();
    void initTrapModel();
    void updateBindActorMtx();
    void endBind();
    void updateSpiral();
    bool tryStartBind(HitSensor*);
    bool tryTornadoJumpPlayer();
    bool tryNonActive();
    bool isSunakazeKun() const;

    void exeNonActive();
    void exeWait();
    void exeStorm();
    void exeStormAdjust();
    void exeStormFix();
    void exeStormEnd();

private:
    /* 0x90 */ ActorCameraInfo* mCameraInfo;
    /* 0x94 */ LiveActor* mBindActor;
    /* 0x98 */ TVec3f mFrontDir;
    /* 0xA4 */ TVec3f mRailClipCenter;
    /* 0xB0 */ s32 mType;
    /* 0xB4 */ f32 mSpiralRot;
    /* 0xB8 */ f32 mSpiralRadius;
    /* 0xBC */ f32 mSpiralAngle;
    /* 0xC0 */ f32 mCurRadius;
    /* 0xC4 */ f32 mAdjustStartRadius;
    /* 0xC8 */ f32 mCurHeight;
    /* 0xCC */ s32 mStepCount;
    /* 0xD0 */ f32 mBindStepOffset;
    /* 0xD4 */ f32 mSensorHeight;
    /* 0xD8 */ f32 mSizeUpRate;
    /* 0xDC */ f32 mSensorMin;
    /* 0xE0 */ s32 mStepStorm;
    /* 0xE4 */ s32 mStepAdjust;
    /* 0xE8 */ s32 mStepFix;
    /* 0xEC */ s32 mStepEnd;
    /* 0xF0 */ f32 mBindHeight;
    /* 0xF4 */ SunakazeKunTrap** mTrapModel;
};
