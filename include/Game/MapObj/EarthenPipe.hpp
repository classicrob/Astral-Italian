#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class PartsModel;

class EarthenPipe : public LiveActor {
public:
    EarthenPipe(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void calcAnim() {};
    virtual void makeActorAppeared();
    virtual MtxPtr getBaseMtx() const {
        return mTopJointMtx;
    }
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    bool tryShowUp();
    bool tryHideDown();
    bool isNerveShowUp() const;
    void exeWait();
    void exeReady();
    void exePlayerIn();
    void exeTargetPipeShowUp();
    void exePlayerOut();
    void exeLaunch();
    void launchPlayer(bool);
    bool isLaunchMode() const;
    void exeInvalid();
    void exeHide();
    void exeShow();
    void exeWaitToHideDown();
    void exeWaitToShowUp();
    void exeShowUp();
    void exeHideDown();
    void calcTrans(f32);

    /* 0x090 */ TVec3f mInitialPos;
    /* 0x09C */ TVec3f mUp;
    /* 0x0A8 */ f32 mMoveDistance;
    /* 0x0AC */ bool mIsIgnoreGravity;
    /* 0x0AD */ bool _A9;
    /* 0x0AE */ bool _AA;
    /* 0x0AF */ bool _AB;
    /* 0x0B0 */ s32 mPipeMode;
    /* 0x0B4 */ EarthenPipe* mTargetPipe;
    /* 0x0B8 */ MtxPtr mTopJointMtx;
    /* 0x0BC */ MtxPtr mBottomJointMtx;
    /* 0x0C0 */ TPos3f mEffectMtx;
    /* 0x0F0 */ LiveActor* mHostActor;
    /* 0x0F4 */ TPos3f mPlayerMtx;
    /* 0x124 */ TMtx34f mPlayerStartMtx;
    /* 0x154 */ TPos3f mPlayerEndMtx;
    /* 0x184 */ f32 _180;
    /* 0x188 */ f32 mHorizExitForce;
    /* 0x18C */ f32 mVertExitForce;
    /* 0x190 */ s32 mMusicChangeIdx;
    /* 0x194 */ s32 mMusicState;
    /* 0x198 */ const char* mSavedBgmId;
    /* 0x19C */ s32 mSavedBgmState;
    /* 0x1A0 */ bool _194;
    /* 0x1A4 */ PartsModel* mPipeStreamModel;
    /* 0x1A8 */ bool mIsPipeStreamShown;
    /* 0x1AC */ ActorCameraInfo* mCameraInfo;
};

class EarthenPipeMediator : public NameObj {
public:
    EarthenPipeMediator();

    struct Entry {
        /* 0x0 */ EarthenPipe* _0;
        /* 0x4 */ EarthenPipe* _4;
        /* 0x8 */ s32 mPipeID;
    };

    void entry(EarthenPipe*, const JMapInfoIter&);

    /* 0x0C */ s32 mNumEntries;
    /* 0x10 */ Entry* mPipeEntries;
};
