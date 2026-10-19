#pragma once

#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class J3DModelData;
class J3DAnmTransform;

class XtransformInfo {
public:
    XtransformInfo();

   /* 0x00 */ TVec3f mScale;
   /* 0x0C */ TVec3f mTranslation;
    /* 0x18 */ Quaternion mRotation;
};

class XjointTransform {
public:
    XjointTransform();

    void setLocalScale(f32 scale)
#ifdef XJOINT_TRANSFORM_SCALE_NOINLINE
        NO_INLINE
#endif
    {
#ifdef XJOINT_TRANSFORM_SCALE_NOINLINE
        mScale.x = scale;
        mScale.y = scale;
        mScale.z = scale;
#else
        mScale.set(scale);
#endif
    }

   /* 0x00 */ J3DJoint* mJoint;
   /* 0x04 */ u16 mParentJointIndex;
   /* 0x08 */ TVec3f mScale;
   /* 0x14 */ TVec3f mScaleModifier;
   /* 0x20 */ TVec3f mOutputTranslationOffset;
   /* 0x2C */ TVec3f mLocalTranslationOffset;
   /* 0x38 */ TVec3f mWorldTranslationOffset;
   /* 0x44 */ J3DTransformInfo mBindTransform;
   /* 0x64 */ MtxPtr mLocalRotationMtx;
   /* 0x68 */ MtxPtr mPostTransform;
   /* 0x6C */ MtxPtr mWorldRotationMtx;
};

class XjointInfo {
public:
    XjointInfo();
    XjointInfo& operator=(const XjointInfo&) NO_INLINE;

   /* 0x00 */ XtransformInfo mFrozenTransform;
   /* 0x28 */ XtransformInfo mCurrentTransform;
   /* 0x50 */ TVec3f mUnblendedTranslation;
  /* 0x5C */ f32 mInterpoleRatio;
   /* 0x60 */ f32 mInterpoleStep;
};

class XanimeTrack {
public:
    XanimeTrack() {
    }

    void init();
    void setFrame(f32 frame) {
        mFrameRatio = frame;
        mHasFrameRatio = 1;
    }

    f32 getWeight() const {
       /* 0x04 */ return mWeight;
    }

   /* 0x00 */ J3DAnmTransform* mAnimation;
    /* 0x04 */ f32 mWeight;
   /* 0x08 */ f32 mFrameRatio;
   /* 0x0C */ u8 mHasFrameRatio;
};

class XanimeCore : public J3DMtxCalc {
public:
    XanimeCore(u32, u32, u8);
    XanimeCore(u32, XanimeCore*);

    virtual ~XanimeCore();
    virtual void setWeight(u8, f32);
    virtual void init(const Vec&, const Mtx&);
    virtual void calc();

    void shareJointTransform(const XanimeCore*);
    void doFreeze();
    void setBck(u32, J3DAnmTransform*);
    void calcBlend(TVec3f*, TVec3f*);
    void calcSingle(TVec3f*, TVec3f*);
    void calcBlendSpecial();
    void calcScaleBlendMaya(const TVec3f&, const TVec3f&);
    void calcScaleBlendMayaNoTransform(const TVec3f&, const TVec3f&);
    void calcScaleBlendSI(const TVec3f&, const TVec3f&);
    void calcScaleBlendBasic(const TVec3f&, const TVec3f&);
    void calcScaleBlendSpecial();
    void freezeCopy(J3DModelData*, XanimeCore*, u32, u32);
    void initT(J3DModelData*);
    void reconfigJointTransform(J3DModelData*);
    void updateFrame();
    void fixT(TVec3f*);

    void enableJointTransform(J3DModelData*);

    void initMember(u32);

    XjointTransform* getJointTransform(u32 index) {
        if (mTransformList == nullptr) {
            return nullptr;
        }

       /* 0x14 */ return &mTransformList[index];
    }

   /* 0x04 */ u8 mScaleBlendMode;
    /* 0x05 */ u8 mTrackCount;
   /* 0x06 */ u8 mCalcMode;
    /* 0x08 */ u32 mJointCount;
   /* 0x0C */ u16 mTranslationJointIndex;
    /* 0x10 */ XjointInfo* mJointList;
    /* 0x14 */ XjointTransform* mTransformList;
    /* 0x18 */ XanimeTrack* mTrackList;
  /* 0x1C */ f32 mInterpoleRatio;
   /* 0x20 */ f32 mFrameBlendRatio;
   /* 0x24 */ f32 mFrameRatio;
   /* 0x28 */ u8 mFreezeRequested;
   /* 0x29 */ u8 mUpdateFrozenTransform;
};
