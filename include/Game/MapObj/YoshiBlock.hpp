#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/MapObj/DynamicCollisionObj.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class YoshiBlock;
class DiceMover;
class ResourceHolder;

class YoshiBlockCollision : public DynamicCollisionObj {
public:
    YoshiBlockCollision(YoshiBlock*);

    virtual ~YoshiBlockCollision();

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    // SMG2: DynamicCollisionObj is 0x124 bytes here (header is 0x120), the owner pointer sits at 0x128.
    u8 _120[0x8];
    /* 0x128 */ YoshiBlock* mYoshiBlock;
};

class YoshiBlock : public LiveActor {
public:
    YoshiBlock(const char*);

    virtual ~YoshiBlock();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void draw() const;
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeDashed();
    void doYoshiDashed(const TVec3f&, const TVec3f&);
    void updateCollisionMtx();

    /* 0x90 */ ResourceHolder* mResource;
    /* 0x94 */ TPos3f mMtx;
    /* 0xC4 */ DiceMover* mCollision;
    /* 0xC8 */ YoshiBlockCollision* mCollisionActor;
    /* 0xCC */ TVec3f mBasePos;
    /* 0xD8 */ TVec3f mBaseRot;
    /* 0xE4 */ TVec3f mSize;
};
