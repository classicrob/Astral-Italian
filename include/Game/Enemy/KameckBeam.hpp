#pragma once

#include "Game/Util/CollisionPartsFilter.hpp"

class KameckBeamEventListener;
class KameckTurtle;
class KameckFireBall;

class KameckBeamProjectile {
public:
    KameckBeamProjectile() NO_INLINE;
    virtual void appearDirection(const TVec3f&, const TVec3f&);
    virtual bool tryStormProjectile();
    virtual void resetBeamProjectile();
    virtual bool isBeamProjectileDead() const;
    void setProjectileEventListener(KameckBeamEventListener*);
    void notifyBeamHit();
    KameckBeamEventListener* mEventListener;
};

class KameckBeam : public LiveActor {
public:
    bool tryFireProjectiles();
    void emitBeamBreakEffect();
    enum BeamType {
        BeamType_None = 0,
        BeamType_FireBall1 = 1,
        BeamType_Turtle = 2,
        BeamType_KuriboMini = 3,
        BeamType_Meramera = 4,
    };

    /// @brief Creates a new `KameckBeam`.
    /// @param pName A pointer to the null-terminated name of the object.
    KameckBeam(const char* pName, s32 kind = BeamType_FireBall1, s32 count = 1);

    virtual void init(const JMapInfoIter& rIter);
    virtual void calcAnim();
    virtual void kill();
    virtual void control();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void setWandLocalPosition(const TVec3f&);
    void setBeamKind(s32);
    void setEventListener(KameckBeamEventListener*);
    void resetBeam();
    bool requestFollowWand(MtxPtr, f32);
    void requestShootToPlayerGround(f32);
    void requestShootToPlayerCenter(f32);
    void requestShoot(const TVec3f&, f32);
    bool requestStorm(HitSensor*, HitSensor*);
    bool tryShootEnd();
    void exeFollowWand();
    void exeShoot();
    void exeExplosion();
    void exeProjectileFlight();
    bool sendBeamAttack(HitSensor*, HitSensor*);
    void startBeamLevelSound();
    void emitBeamReadyEffect();
    void emitBeamEffect();

    /* 0x90 */ KameckBeamEventListener* mEventListener;
    /* 0x94 */ KameckBeamProjectile* mProjectiles[3];
    /* 0xA0 */ MtxPtr mFollowMtx;
    /* 0xA4 */ TVec3f mShootDirection;
    /* 0xB0 */ TVec3f mWandLocalPosition;
    /* 0xBC */ s32 mBeamKind;
    /* 0xC0 */ s32 mProjectileCount;
};

class KameckBeamEventListener {
public:
    /// @brief Creates a new `KameckBeamEventListener`.
    KameckBeamEventListener();

    virtual void hitBeam(){};
};

class KameckBeamCollisionFilter : public CollisionPartsFilterBase {
public:
    KameckBeamCollisionFilter(const TVec3f*) NO_INLINE;

    virtual bool isInvalidParts(const CollisionParts*) const;

    /* 0x04 */ const TVec3f* mBeamPosition;
};

namespace MR {
    void setKameckBeamCollisionFilter(LiveActor*);
};  // namespace MR
