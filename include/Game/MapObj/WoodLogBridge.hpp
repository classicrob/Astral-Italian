#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class LiveActorGroup;
class WoodLogBridge;
class JUTTexture;

class WoodLogBridgeParts : public LiveActor {
public:
    WoodLogBridgeParts(WoodLogBridge*, const char*);

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void updateRandomKick() NO_INLINE;

    /* 0x90 */ WoodLogBridge* mHost;
    /* 0x94 */ const char* mName;
    /* 0x98 */ TVec3f mRestPos;
    /* 0xA4 */ s32 mKickTimer;
};

class WoodLogBridge : public LiveActor {
public:
    WoodLogBridge(const char*);

    virtual ~WoodLogBridge();
    virtual void init(const JMapInfoIter&);
    virtual void draw() const;
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual MtxPtr getBaseMtx() const;

    /* 0x90 */ LiveActorGroup* mParts;
    /* 0x94 */ bool mIsShort;
    /* 0x98 */ TVec3f mDirZ;
    /* 0xA4 */ TVec3f mDirX;
    /* 0xB0 */ TPos3f mBaseMtx;
    /* 0xE0 */ s32 mCooldown;
    /* 0xE4 */ const f32* mPtrE4;
    /* 0xE8 */ const s32* mPtrE8;
    /* 0xEC */ JUTTexture* mTex;
    /* 0xF0 */ TVec3f mClipPos;
};
