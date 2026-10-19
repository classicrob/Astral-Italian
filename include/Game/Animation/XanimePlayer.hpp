#pragma once

#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/types.h>

class J3DAnmTransform;
class J3DModel;
class XanimeCore;
class XanimeGroupInfo;
class XanimeResourceTable;

class XanimeFrameCtrl : public J3DFrameCtrl {
public:
    /// @brief Creates a new `XanimeFrameCtrl`.
    XanimeFrameCtrl();

   /* 0x14 */ s32 mInterpoleFrames;
};

class XanimePlayer {
public:
    XanimePlayer(J3DModel*, XanimeResourceTable*);
    XanimePlayer(J3DModel*, XanimeResourceTable*, XanimePlayer*);

    f32 getAnimationFrame() const;
    void setAnimationFrame(f32);
    bool isExistAnimation(const char*) const;
    void init();
    void stopAnimation();
    void stopAnimation(const char*);
    void changeTrackAnimation(u8, const char*);
    void changeTrackAnimation(u8, u32);
    void changeAnimationBck(const char*);
    bool changeSimpleBck(const char*);
    void changeAnimation(const char*);
    void changeAnimationByHash(u32);
    void swapFrameCtrl(const XanimeGroupInfo*);
    void changeAnimation(const XanimeGroupInfo*);
    void prepareAnimation(const XanimeGroupInfo*);
    void runNextAnimation();
    void changeAnimationSimple(J3DAnmTransform*);
    void changeSpeed(f32);
    void changeInterpoleFrame(s32);
    bool changeTrackWeight(u32, f32);
    void calcAnm(u16);
    void updateFrameRatio() NO_INLINE;
    f32 getEffectiveAnimationFrame() const NO_INLINE;
    void setModel(J3DModel*);
    void overWriteMtxCalc(u16);
    void clearMtxCalc(u16);
    void clearAnm(u16);
    void updateBeforeMovement();
    void updateAfterMovement();
    void updateInterpoleRatio();
    void getMainAnimationTrans(u32, TVec3f*) const;
    bool isRun(const char*) const;
    bool fn_8040F600() const NO_INLINE;
    bool isTerminate(const char*) const;
    bool isTerminate() const;
    void setDefaultAnimation(const char*);
    void runDefaultAnimation();
    bool isAnimationRunSimple() const;
    const char* getCurrentAnimationName() const;
    const char* getDefaultAnimationName() const;
    const char* getCurrentBckName() const;
    const char* getNameStringPointer(const char*) const;
    bool isCurrentBckActive() const NO_INLINE;
    bool isFrameInRange(f32, f32) const;
    bool checkPass(f32) const;
    f32 tellAnimationFrame() const;
    XanimeGroupInfo* getSimpleGroup() const;
    void duplicateSimpleGroup();
    void changeCurrentAnimation(const XanimeGroupInfo*);
    XanimeFrameCtrl* getFrameCtrl() {
       /* 0x20 */ return mFrameCtrl;
    }

    inline XanimeCore* getCore() {
       /* 0x6C */ return mCore;
    }
    inline f32 getRate() {
        return mFrameCtrl->getRate();
    }

    /* 0x00 */ J3DModel* mModel;
    /* 0x04 */ J3DModelData* mModelData;
   /* 0x08 */ f32 mInterpoleRatio;
   /* 0x0C */ f32 mFrameBlendRatio;
    /* 0x10 */ f32 mWeights[4];
   /* 0x20 */ XanimeFrameCtrl* mFrameCtrl;
   /* 0x24 */ XanimeFrameCtrl mFrameCtrls[2];
   /* 0x54 */ u8 mCurrentFrameCtrlIndex;
   /* 0x55 */ u8 mNextFrameCtrlIndex;
    /* 0x58 */ const char* mCurrentBckName;
    /* 0x5C */ const XanimeGroupInfo* mDefaultAnimation;
    /* 0x60 */ const XanimeGroupInfo* mCurrentAnimation;
    /* 0x64 */ const XanimeGroupInfo* mPrevAnimation;
   /* 0x68 */ const XanimeGroupInfo* mNextAnimation;
    /* 0x6C */ XanimeCore* mCore;
    /* 0x70 */ XanimeResourceTable* mResourceTable;
    /* 0x74 */ XanimeGroupInfo* mSimpleGroup;
   /* 0x78 */ s32 mDefaultAnimationDelay;
   /* 0x7C */ bool mHasAnimation;
   /* 0x7D */ bool mHasDefaultAnimation;
   /* 0x7E */ bool mAutoRunDefaultAnimation;
   /* 0x7F */ bool mNextAnimationReady;
   /* 0x80 */ bool mKeepFrameRatio;
   /* 0x84 */ f32 mCachedFrame;
   /* 0x88 */ bool mHasUpdatedFrame;
   /* 0x89 */ bool mAnimationChanged;
};
