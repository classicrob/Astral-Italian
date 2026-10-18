#pragma once

#include "Game/Screen/LayoutActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class LiveActor;
class StarPointerCommandStream;
class StarPointerDirector;
class StarPointerBlur;
class StarPointerNumber;

// TODO: what are the missing values?
enum StarPointerKind {
    StarPointerKind_NULL = -1,
    StarPointerKind_StarPointer = 0,
    StarPointerKind_HandPointer = 1,
    StarPointerKind_2 = 2,
    StarPointerKind_HandPointerReactionWithCrossCursor = 3,
    StarPointerKind_FingerPointer = 4,
    StarPointerKind_5 = 5,
    StarPointerKind_StarPointerNozzle = 6,
    StarPointerKind_YoshiPointer = 7,
    StarPointerKind_YoshiPointerNoBlur = 8,
};

enum AnimType {
    AnimType_NULL = -1,
    AnimType_Finger = 0,
    AnimType_HandGuu = 1,
    AnimType_HandPaa = 2,
    AnimType_StarPointer = 3,
    AnimType_YoshiPointer = 4,
};

class StarPointerLayout : public LayoutActor {
public:
    StarPointerLayout(const char* pName = "スターポインタレイアウト");

    virtual void draw() const;
    virtual void calcAnim();
    virtual void appear();
    virtual void kill();
    virtual void control();

    void initWithPort(s32);
    bool startCommandStream(const LiveActor*, const TVec3f*, bool);
    void endCommandStream(const LiveActor*);
    void forceEndCommandStream();
    bool isCommandStream(const LiveActor*);

    void exeWait();
    void exeOnReaction();
    void exeGrip();
    void exeHold();
    void endHold();
    void exeRelease();
    void exeOutScreen();
    void exeNotConnected();
    void endNotConnected();
    void exeHide();
    void endHide();

    void tearDownHold();
    void updateTouch();
    bool isTouch();
    void hideAll();
    void tearDownHide();
    void drawDecoration() const;
    void setPosition();
    void rotateTest();
    void calcPoseNozzle(const TVec2f&, const TVec2f&);
    void changeLayout(StarPointerKind);
    bool isChanceToRumble() const;
    void hideBlur();
    void resetSameNerve();
    void changeToStarPointer();
    void changeToHandPointer();
    void updatePlayerPane() NO_INLINE;
    void showYoshiPointer();
    void startAnimYoshiPointer(const char*, u32);
    void startAnimStarPointer(const char*);
    void startAnimHandPaa();
    void startAnimHandGuu();
    void startAnimHandFinger();
    void updateDecoration();

    void show() {
        if (!mIsAppear) {
            mAppearTime = -3;
        }
        mIsAppear = true;
    }

    void hide() {
        mIsAppear = false;
    }

    void validate() {
        mIsPointerValid = true;
    }

    void invalidate() {
        mIsPointerValid = false;
    }

    bool isAppear() const {
        return mIsAppear && mAppearTime >= 0;
    }

    AnimType getAnimType() const {
        return mAnimType;
    }

    bool isSingleTouch() {
        return mSingleTouch || mStartSingleTouch;
    }

    f32 getRadius() const {
        return mRadius;
    }

    /* 0x2C */ TVec2f mPosition;
    /* 0x34 */ StarPointerKind mPointerKind;
    /* 0x38 */ AnimType mAnimType;
    /* 0x3C */ s32 mTouchTimer;
    /* 0x40 */ s32 mPadChannel;
    /* 0x44 */ f32 mRadius;
    /* 0x48 */ f32 _48;
    /* 0x4C */ StarPointerDirector* mDirector;
    /* 0x50 */ StarPointerBlur* mBlur;
    /* 0x54 */ StarPointerNumber* mNumber;
    /* 0x58 */ u64 mNewTouchedID;  // holds a unique identifier of the last touched object (pointer-compare with extra steps)
    /* 0x60 */ u64 mTouchedID;
    /* 0x68 */ bool mIsNewTouch;
    /* 0x69 */ bool mStartTouch;
    /* 0x6A */ bool mStartDisableShoot;
    /* 0x6B */ bool mSingleTouch;
    /* 0x6C */ bool mStartSingleTouch;
    /* 0x6D */ bool mShootDisabled;
    /* 0x6E */ bool mIsPointerValid;
    /* 0x6F */ bool mIsAppear;
    /* 0x70 */ s32 mAppearTime;
    /* 0x74 */ const LiveActor* mActor;
};

class StarPointerNumber : public LayoutActor {
public:
    StarPointerNumber(StarPointerLayout*, s32, const char*);

    virtual void control();

    bool isOffState() const;
    void startAnimPosition(u32) NO_INLINE;

    /* 0x2C */ StarPointerLayout* mParent;
    /* 0x30 */ s32 mPadChannel;
    /* 0x34 */ AnimType mAnimType;
    /* 0x38 */ bool mIsAppear;
    /* 0x3C */ s32 mAppearTime;
};
