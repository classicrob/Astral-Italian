#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JMapInfo.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class ActorJointCtrl;
class AnimScaleController;
class BrosProjectileBase;
class MapObjConnector;
class YoshiLockOnTarget;

/// @brief Common base of HammerBros and BoomerangBros (SMG2).
class BrosBase : public LiveActor {
public:
    BrosBase(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();
    // pure virtuals (vtable +0x70, +0x74, +0x78)
    virtual const TVec3f& getHoldTrans() const = 0;
    virtual const TVec3f& getHoldRotate() const = 0;
    virtual void throwProjectile(BrosProjectileBase*) = 0;

    void calcHoldMtx(MtxPtr);                // fn_80186590
    bool isExistActiveProjectile() const;    // fn_80186630
    bool isExistProjectileNear(f32) const;   // fn_801866D0
    void tryHoldProjectile();                // fn_80186760
    void killHeldProjectile();               // fn_801867F0
    void tryThrowProjectile();               // fn_80186840
    void startFaceCtrl();                    // fn_801868F0
    void endFaceCtrl();                      // fn_80186900
    void updateVelocity();                   // fn_80186910
    void initByArg(const JMapInfoIter&);     // fn_80186A20
    void appearStarPiece();                  // fn_80186A80
    void appearCoin();                       // fn_80186AB0
    void fn_80186AD0();                      // tail-calls fn_8022BAE0
    void killProjectiles();                  // fn_80186AE0
    void attachToUnder();                    // fn_80186BA0

    /* 0x90 */ BrosProjectileBase* mProjectiles[5];
    /* 0xA4 */ BrosProjectileBase* mHeld;
    /* 0xA8 */ s32 mProjectileIdx;
    /* 0xAC */ TVec3f mFront;
    /* 0xB8 */ bool _B8;
    /* 0xB9 */ bool _B9;
    /* 0xBC */ s32 mItemType;
    /* 0xC0 */ AnimScaleController* mScaleCtrl;
    /* 0xC4 */ ActorJointCtrl* mJointCtrl;
    /* 0xC8 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xCC */ s32 _CC;
    /* 0xD0 */ MapObjConnector* mConnector;
    /* 0xD4 */ bool mConnected;
};
