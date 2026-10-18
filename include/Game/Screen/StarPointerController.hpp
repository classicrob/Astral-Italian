#pragma once

#include "Game/Screen/StarPointerInput.hpp"

class DpdInfo {
public:
    DpdInfo();

    /* 0x00 */ TVec2f mPos;
    /* 0x08 */ bool mInScreen;
    /* 0x09 */ bool mDrawReady;
    /* 0x0C */ u32 mZDepth;
    /* 0x10 */ f32 mViewDistZ;
};

// Reconstructed empty base: its original name is unknown; the inheritance shape is verified against the target ABI.
class StarPointerControllerBase {};

class StarPointerController : public StarPointerControllerBase, public StarPointerInput {
public:
    StarPointerController();

    void initAndSetPort(s32);
    void movement(const f32*, const f32*);
    void storeDataFromCallback(const f32*, const f32*);
    void storePastPointingData();
    void calcPastPointingPosOnScreen(TVec2f*, s32) const;
    void updateDpdInfo();
    void updateAdditionalInfo();

    void forcePointerOut();

    f32 calcViewDistanceZ(const TVec3f&, MtxPtr);

    virtual const TVec2f& getScreenPosition() const { return mPastInfo.mPos; }
    virtual const TVec2f& getScreenVelocity() const { return mScreenVel; }
    virtual const TVec3f& getWorldVelocity() const { return mWorldVel; }
    virtual f32 getScreenSpeed() const { return mScreenSpeed; }
    virtual f32 getViewDistanceZ() const { return mPastInfo.mViewDistZ; }
    virtual s32 getPastPointCount() const { return mPastPointNum; }
    virtual s32 getNextPastPointCount() const { return mNextPastPointNum; }
    virtual const TVec2f& getPastPosition(s32 index) const { return mPastPosition[index]; }
    virtual const TVec2f& getNextPastPosition(s32 index) const { return mNextPastPosition[index]; }
    virtual const TVec3f& getWorldPosition() const { return mWorldPos; }
    virtual bool isInScreen() const { return mPastInfo.mInScreen; }
    virtual bool isOutScreenLong() const;

    f32 getViewDistZ() const { return mPastInfo.mViewDistZ; }
    const TVec2f& getPastPos() const { return mPastInfo.mPos; }

    /* 0x08 */ s32 mPadChannel;
    /* 0x0C */ DpdInfo mPastInfo;
    /* 0x20 */ DpdInfo mInfo;
    /* 0x34 */ TVec2f mScreenPos;
    /* 0x3C */ s32 mOutScreenTime;
    /* 0x40 */ TVec2f* mPastPosition;
    /* 0x44 */ s32 mPastPointNum;
    /* 0x48 */ TVec2f* mNextPastPosition;
    /* 0x4C */ s32 mNextPastPointNum;
    /* 0x50 */ f32 mScreenDist;
    /* 0x54 */ f32 mScreenSpeed;
    /* 0x58 */ TVec2f mScreenVel;
    /* 0x60 */ TVec3f mWorldVel;
    /* 0x6C */ TVec3f mWorldPos;
    /* 0x78 */ bool mNotInScreen;
};
