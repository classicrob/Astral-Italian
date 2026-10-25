#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Util/SpringValue.hpp"

class SpinCloudBlock : public LiveActor {
public:
    SpinCloudBlock(const char*);

    virtual ~SpinCloudBlock();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait();
    bool exeTransform();
    bool exeMove();
    void exeDisperse();
    void exeDisappear();
    void placeAt(const TVec3f&);
    void spawnAt(const TVec3f&);
    bool tryStartTransform(const TVec3f&) NO_INLINE;
    void setAnchorPos(const TVec3f&) NO_INLINE;
    void calcWind();
    void adjustOnPlayer();
    bool updateScaleEase(s32) NO_INLINE;

    /* 0x90 */ SpringValue* mSpring;
    /* 0x94 */ TQuat4f mQuat;
    /* 0xA4 */ TVec3f mSavedPos;
    /* 0xB0 */ TVec3f mAnchorPos;
    /* 0xBC */ TVec3f mSwayVel;
    /* 0xC8 */ s32 mUnkC8;
    /* 0xCC */ f32 mPushBack;
    /* 0xD0 */ s32 mHitTimer;
};

class SpinCloudBlockGroup : public DeriveActorGroup<SpinCloudBlock> {
public:
    SpinCloudBlockGroup();

    virtual ~SpinCloudBlockGroup();
    virtual void movement();

    void updateBlockScale();
    bool spawnBlocks();
    LiveActor* getDeadBlock();
    void resetBlocks();

    /* 0x20 */ LiveActor** mBlocks;
    /* 0x24 */ s32 mBlockNum;
    /* 0x28 */ s32 mRespawnStep;
    /* 0x2C */ f32 mSpinSpeed;
};

namespace MR {
    void createSpinCloudBlockGroup();
    bool spawnSpinCloudBlocks();
    void resetSpinCloudBlocks();
    bool isSpinCloudBlockGroupEmpty();
    f32 getSpinCloudBlockSpinSpeed();
};  // namespace MR
