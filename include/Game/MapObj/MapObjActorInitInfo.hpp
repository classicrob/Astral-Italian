#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class Nerve;

// SMG2 layout (0x9C bytes), from MapObjActorInitInfo::MapObjActorInitInfo and the setup* setters.
class MapObjActorInitInfo {
public:
    MapObjActorInitInfo();

    void setupHioNode(const char*);
    void setupDefaultPos();
    void setupModelName(const char*);
    void setupConnectToScene();
    void setupBinder(f32, f32);
    void setupEffect(const char* = nullptr, s32 = 0);
    void setupSound(s32, const char* = nullptr);
    void setupSoundPos(TVec3f*);
    void setupHitSensor();
    void setupHitSensorParam(u16, f32, const TVec3f&);
    void setupNerve(const Nerve*);
    void setupState(s32);
    void setupShadow(const char* = nullptr);
    void setupGroupClipping(s32);
    void setupClippingRadius(f32);
    void setupFarClipping(f32 = -1.0f);
    void setupProjmapMtx(bool, bool) NO_INLINE;
    void setupMultiFur(s32);
    void setupRailMover();
    void setupRotator();
    void setupRailRotator();
    void setupRailPosture();
    void setupBaseMtxFollowTarget();
    void setupAffectedScale();
    void setupSeAppear();
    void setupLightCtrl();
    void setupNoUseLodCtrl();

    // Setters that exist in Petari but are inlined in SMG2.
    inline void setupNoAppearRiddleSE() {
        mIsAppearRiddleSE = false;
    }

    inline void setupHitSensorCallBack() {
        mHasHitSensorCB = true;
    }

    inline void setupProjmapMtx(bool a1) {
        setupProjmapMtx(a1, false);
    }

    inline void setupSeesaw1AxisRotator(const char* pName, f32 radius) {
        _C = true;
        mSeesaw1AxisRotatorName = pName;
        mSeesaw1AxisRotatorRadius = radius;
    }

    inline void setupMirrorReflection(bool a1) {
        mUseMirrorReflection = true;
        mMirrorReflectionArg = a1;
    }

    inline void setupPrepareChangeDummyTexture(const char* pName) {
        mDummyChangeTexture = pName;
    }

    /* 0x00 */ bool mSetDefaultPosition;
    /* 0x01 */ bool mConnectToScene;
    /* 0x02 */ bool mInitBinder;
    /* 0x03 */ bool mHasEffect;
    /* 0x04 */ bool mHasSensors;
    /* 0x05 */ bool mHasShadows;
    /* 0x06 */ bool mCalcGravity;
    /* 0x07 */ bool mUseProjectMapMtx;
    /* 0x08 */ bool _8;
    /* 0x09 */ bool mHasRailMover;
    /* 0x0A */ bool mHasRotator;
    /* 0x0B */ bool mHasRailRotator;
    /* 0x0C */ bool _C;  // seesaw rotate axis 0
    /* 0x0D */ bool _D;  // seesaw rotate axis 1
    /* 0x0E */ bool _E;  // seesaw rotate axis 2
    /* 0x0F */ bool mUseRailPosture;
    /* 0x10 */ bool mUseBaseMtxFollowTarget;
    /* 0x11 */ bool mIsAffectedByScale;
    /* 0x12 */ bool mUseMirrorReflection;
    /* 0x14 */ const char* mModelName;
    /* 0x18 */ const char* mHioNode;
    /* 0x1C */ u16 _1C;
    /* 0x20 */ f32 mSensorRadius;
    /* 0x24 */ TVec3f mSensorPosition;
    /* 0x30 */ bool mHasHitSensorCB;
    /* 0x34 */ f32 mBinderRadius;
    /* 0x38 */ f32 mBinderCenterY;
    /* 0x3C */ const char* mEffectName;
    /* 0x40 */ s32 mEffectNum;
    /* 0x44 */ s32 mSound;
    /* 0x48 */ const char* mSoundName;
    /* 0x4C */ TVec3f* mSoundPos;
    /* 0x50 */ bool mProjmapArg2;
    /* 0x51 */ bool mProjmapArg1;  // Petari _48
    /* 0x54 */ const Nerve* mNerve;
    /* 0x58 */ s32 mState;
    /* 0x5C */ s32 mGroupClipping;
    /* 0x60 */ f32 mClippingRadius;
    /* 0x64 */ f32 mFarClipping;
    /* 0x68 */ s32 mMultiFur;          // Petari _5C
    /* 0x6C */ s32 mColorChangeArg;    // Obj_arg0
    /* 0x70 */ s32 mTextureChangeArg;  // Obj_arg1
    /* 0x74 */ bool mIsAppearRiddleSE;
    /* 0x78 */ const char* mSeesaw1AxisRotatorName;
    /* 0x7C */ f32 mSeesaw1AxisRotatorRadius;
    /* 0x80 */ bool mMirrorReflectionArg;  // Petari _74
    /* 0x84 */ const char* mShadowName;
    /* 0x88 */ f32 mShadowLength;          // Obj_arg2
    /* 0x8C */ const char* mBreakModelName;  // Petari _80
    /* 0x90 */ const char* mDummyChangeTexture;
    /* 0x94 */ s32 _94;  // Petari _88
    /* 0x98 */ bool mUseLightCtrl;  // Petari _8C
    /* 0x99 */ bool mNoUseLOD;
};
