#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ActorCameraInfo;
class CameraTargetMtx;
class FixedPosition;
class GhostPacket;
class HitSensor;
class JMapInfoIter;
class JetTurtleShadow;
class NameObjArchiveListCollector;
class RaceManagerLayout;
class XanimePlayer;

f32 getShiftRatio(s8 shiftValue);

namespace MR {
    void convToFloat(TVec3s&, s8, TVec3f*);
    void convToFloat(TVec3Sc&, s8, TVec3f*);
    void convToFloat(s16, s8, f32*);
    void convToFloat(s8, s8, f32*);
};  // namespace MR

class GhostPlayer : public LiveActor {
public:
    static void makeArchiveList(NameObjArchiveListCollector*, const JMapInfoIter&);

    GhostPlayer(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void draw() const;
    virtual void appear();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait();
    void warpPosition(const char*);
    void exePreStartDemo0();
    bool isRequestSkipDemo() const;
    void exePreStartDemo1();
    void exePreStartDemo2();
    void exeWinDemo();
    void exeLostDemo();
    void initAnimation();
    void setAnimation(const char*);
    void setAnimationWeight(const f32*);
    u32 receiveGhostPacket(GhostPacket*);

    /* 0x08C */ u32 _8C;
    /* 0x090 */ u32 _90;
    /* 0x094 */ ActorCameraInfo* mCameraInfo;
    /* 0x098 */ XanimePlayer* mXanimePlayer;
    /* 0x09C */ XanimePlayer* mXanimePlayerUpper;
    /* 0x0A0 */ Mtx mTargetRotationMtx;
    /* 0x0D0 */ f32 mAnimTrackWeights[4];
    /* 0x0E0 */ TVec3f mStartPos;
    /* 0x0EC */ u32 _EC[3];  // unused
    /* 0x0F8 */ TVec3f mTargetRotation;
    /* 0x104 */ TVec3f mInitialDirection;
    /* 0x110 */ bool mIsHidden;
    /* 0x111 */ bool mWaitingToStart;
    /* 0x112 */ bool _112;
    /* 0x113 */ bool mKilledByStar;
    /* 0x114 */ bool mIsDemoCameraActive;
    /* 0x115 */ bool mHasJetTurtle;
    /* 0x118 */ LiveActor* mPowerStarTarget;
    /* 0x11C */ JetTurtleShadow* mJetTurtleShadow;
    /* 0x120 */ FixedPosition* mHandRPos;
    /* 0x124 */ u16 mAppearStarPieceCooldown;
    /* 0x126 */ u16 mPlayerTrampleCooldown;
    /* 0x128 */ GhostPacket* mCurrentPacket;
    /* 0x12C */ RaceManagerLayout* mRaceManagerLayout;
    /* 0x130 */ CameraTargetMtx* mCameraTargetMtx;
};

namespace MR {
    bool fn_8037F5B0(const TVec3f&, s8, TVec3s*);
    bool fn_8037F680(f32, s8, s16*);
    bool fn_8037F6F0(const TVec3f&, s8, TVec3Sc*);
    bool fn_8037F7D0(f32, s8, s8*);
    void convToFloatRotation(f32*, s8);
    void convToFloatBckRate(f32*, s8);
    void convToFloatTrackWeight(f32*, s8);
}

class XanimeResourceTable;
namespace MR {
    void fn_8037F4B0(f32*);
    bool fn_8037FA30(TVec3s*, MtxPtr);
    void fn_8037FA80(TVec3Sc*, const TVec3f&);
    void fn_8037FAF0(TVec3Sc*, MtxPtr);
    void fn_8037FC30(TVec3Sc*, const TVec3f&);
    void fn_8037FC40(TVec3Sc*, const TVec3f&);
    void fn_8037FC50(s16*, f32);
    void fn_8037FC60(s8*, f32);
    void fn_8037FC70(s8*, f32);
    u32 fn_8037FE20(u32);
    bool fn_8037FEE0(const char*);
    XanimePlayer* fn_8037FF80(LiveActor*, XanimeResourceTable*);
    XanimePlayer* fn_80380030(LiveActor*, XanimePlayer*, XanimeResourceTable*);
    void fn_803800C0(XanimePlayer*, const char*);
    void fn_80380130(XanimePlayer*, XanimePlayer*, LiveActor*, u32, bool*);
}
