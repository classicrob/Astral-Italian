#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class ModelObj;
class NameObjArchiveListCollector;
class PowerStarAppearPoint;

class PowerStar : public LiveActor {
public:
    PowerStar(const char*);

    virtual ~PowerStar();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void requestAppear();
    void setDemoAppearPos(const TVec3f&);
    bool isEndAppearDemo() const;
    void offAppearDemo();
    // fn_802DF9A0: marks the star as appearing from below (boss demo) and shifts the appear position along gravity
    void setAppearFromBelow();
    static s32 setupColor(LiveActor*, const NameObj*, s32);
    static s32 setupColorAtResultSequence(LiveActor*, bool);
    static void requestPointLight(const LiveActor*, const NameObj*, s32);
    static void requestPointLightAtResultSequence(const LiveActor*);
    static f32 getPowerStarWaitRotateSpeed();
    static s32 getBtpFrameCurrentStage(s32);
    static bool isCurrentStageKoopaVs3();
    static void initShadowPowerStar(LiveActor*, bool, f32 = -1.0f);
    // fn_802DFD00
    static void emitColorEffect(LiveActor*, const char*, s32, bool);
    void initMapToolInfo(const JMapInfoIter&);
    void initModel(const JMapInfoIter&);
    void initSensorGrandStar();
    void initShadow(const JMapInfoIter&);
    void initPosture();
    void endAppearDemo();
    PowerStarAppearPoint* getNearestAppearPoint(const TVec3f&) const;
    LiveActor* getAppearCameraActor();
    ActorCameraInfo* getAppearCameraInfo() const;
    void requestAppearOrWait();
    const char* getStarGetAnimCameraName() const;
    void calcAppearDemoRiseTrans(TVec3f*, f32) const;
    void applyAppearDemoRiseTrans(const TVec3f&, f32, f32);
    void startAppearDemoCommon();  // fn_802E07C0
    void endAppearDemoMove();      // fn_802E0D50
    void processWait(f32);
    void calcStageClearDemoMtx();
    void startExistLevelSound();
    void exeWaitStartAppear();
    void exeAppearDemoRise();
    void exeAppearDemoMove();
    void exeAppearDemoBossFormer();  // fn_802E1250
    void exeAppearDemoBossLatter();  // fn_802E1350
    void exeWait();
    void exeStageClearDemo();

    inline bool isGrandStar() {
        return mIsGrandStar;
    }

    /* 0x090 */ s32 mPowerStarId;
    /* 0x094 */ bool mIsInDemo;
    /* 0x098 */ TVec3f mAppearPosition;
    /* 0x0A4 */ TVec3f mInitRotation;
    /* 0x0B0 */ TVec3f mInitPosition;
    /* 0x0BC */ TPos3f mPostureMtx;
    /* 0x0EC */ TPos3f mBaseMtx;
    /* 0x11C */ ModelObj* mPowerStarModelObj;
    /* 0x120 */ LiveActor* mPeachModel;
    /* 0x124 */ LiveActor* mTicoBabyModel;
    /* 0x128 */ LiveActor* mSupportTicoModel;
    /* 0x12C */ bool _12C;
    /* 0x12D */ bool _12D;
    // this isn't what they wrote but oh well
    union {
        /* 0x130 */ vs32 mColor_v;
        /* 0x130 */ s32 mColor;
    };
    /* 0x134 */ volatile bool mIsGrandStar;
    /* 0x135 */ bool mIsGreenStar;
    /* 0x136 */ bool _136;
    /* 0x137 */ bool _137;
    /* 0x138 */ bool mIsBaseFromAppearPoint;
    /* 0x13C */ ActorCameraInfo* mCameraInfo;
    /* 0x140 */ PowerStarAppearPoint* mAppearPoint;
    /* 0x144 */ TMtx34f _144;
    /* 0x174 */ bool mIsUseAwakeSwitch;
};
