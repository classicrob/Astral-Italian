#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class SeaGullGroup;

class SeaGull : public LiveActor {
public:
    SeaGull(SeaGullGroup* pGroup);

    virtual ~SeaGull();
    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual void calcAndSetBaseMtx();

    void exeHoverFront();
    void exeHoverLeft();
    void exeHoverRight();
    void updateHover();

    /* 0x90 */ bool mSuppressChirp;
    /* 0x94 */ SeaGullGroup* mSeaGullGroup;
    /* 0x98 */ s32 mPointIndex;
    /* 0x9C */ bool mIsReverse;
    /* 0xA0 */ s32 mTargetUpdateTimer;
    /* 0xA4 */ TVec3f* mTargetPosition;
    /* 0xA8 */ TVec3f mUp;
    /* 0xB4 */ TVec3f mFront;
    /* 0xC0 */ TVec3f mBankedUp;
    /* 0xCC */ TVec3f mSide;
    /* 0xD8 */ s32 mHoverTimer;
    /* 0xDC */ f32 mBankAngle;
    /* 0xE0 */ s32 mGlideTimer;
    /* 0xE4 */ s32 mLiftTimer;
    /* 0xE8 */ s32 mChirpTimer;
};

class SeaGullGroup : public LiveActor {
public:
    SeaGullGroup(const char* pName);

    virtual ~SeaGullGroup();
    virtual void init(const JMapInfoIter& rIter);

    virtual void makeActorAppeared();
    virtual void makeActorDead();

    TVec3f* updatePosInfo(s32* pPointIndex, bool isReverse) const;

    /* 0x90 */ s32 mPointCount;
    /* 0x94 */ TVec3f* mPoints;
    /* 0x98 */ s32 mBirdCount;
    /* 0x9C */ SeaGull** mBirds;
    /* 0xA0 */ bool mIsMarioFace;
};
