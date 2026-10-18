#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JointController.hpp"

class Yoshi;
class FixedPosition;

class YoshiTongueCurve {
public:
    YoshiTongueCurve();
    void fn_8041D750(const TVec3f&, const TVec3f&, const TVec3f&, const TVec3f&);
    void fn_8041DA20(TVec3f*, f32) const;
    TVec3f mControl[4];
    bool mInitialized;
    TVec3f mSamples[7];
    f32 mDistances[7];
};

class YoshiTongueJoint {
public:
    YoshiTongueJoint(LiveActor*, const char*, f32);
    bool fn_8041D5B0(TPos3f*, const JointControllerInfo&);
    void fn_8041D6E0(const TVec3f&, const TVec3f&, f32) NO_INLINE;
    JointController* mController;
    TVec3f mPosition;
    TVec3f mDirection;
    f32 mLength;
    f32 mWeight;
    u8 _24[12];
};

class YoshiTongue : public LiveActor {
public:
    YoshiTongue(Yoshi*, const char*);
    virtual ~YoshiTongue();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void calcAndSetBaseMtx();
    void exeState0();
    void exeState1();
    void exeState3();
    void exeState7();
    void fn_8041CB30(bool);
    bool fn_8041CB50() const;
    void fn_8041CB70(bool);
    bool fn_8041CB80() const;
    bool fn_8041CB90() const;
    void fn_8041CBA0();
    bool fn_8041CC00() const;
    void fn_8041CC60();
    void fn_8041CCB0(const TVec3f&);
    void fn_8041CCF0();
    bool fn_8041CD00() const;
    void fn_8041CD10(TPos3f*) const;
    f32 fn_8041CEC0() const;
    void fn_8041CEE0();
    void fn_8041D180(YoshiTongueCurve*);
    f32 fn_8041D290(f32, f32) const;
    void fn_8041D2B0();
    bool fn_8041D3C0() const;
    void fn_8041D460();

    Yoshi* mYoshi;
    FixedPosition* mFixedPosition;
    MtxPtr mMouthMtx;
    TVec3f mTipPosition;
    TVec3f mTipDirection;
    f32 mProgress;
    bool mUseMouthDirection;
    bool mHitMap;
    f32 mCollisionProgress;
    TVec3f mCollisionPosition;
    YoshiTongueCurve* mCurve;
    s32 mExtendFrames;
    s32 mRetractFrames;
    TVec3f mRetractOffset;
    f32 mRetractProgress;
    YoshiTongueJoint* mJoints[4];
};
