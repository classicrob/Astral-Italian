#pragma once

#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Enemy/KameckBeam.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class KameckBeam;
class KameckBeamEventListener;
class KameckFireBall;
class KameckTurtle;
class KuriboMini;
class Meramera;

class KameckBeamHolder : public DeriveActorGroup< KameckBeam > {
public:
    /// @brief Creates a new `KameckBeamHolder`.
    KameckBeamHolder();
    KameckBeamHolder(s32, s32, s32);
    virtual ~KameckBeamHolder();
    KameckBeam* fn_801A7CF0(f32, MtxPtr, const TVec3f&, KameckBeamEventListener*);  // unnamed, by address
};

class KameckFireBallHolder : public DeriveActorGroup< KameckFireBall > {
public:
    /// @brief Creates a new `KameckFireBallHolder`.
    KameckFireBallHolder();
};

class KameckBeamTurtleHolder : public DeriveActorGroup< KameckTurtle > {
public:
    /// @brief Creates a new `KameckBeamTurtleHolder`.
    KameckBeamTurtleHolder();
};

namespace MR {
    void createKameckBeamHolder();
    void createKameckFireBallHolder();
    void createKameckBeamTurtleHolder();
    KameckBeam* startFollowKameckBeam(s32, MtxPtr, f32, const TVec3f&, KameckBeamEventListener*);
    KameckBeam* getKameckBeam();
    KameckFireBall* getKameckFireBall();
    KameckTurtle* getKameckBeamTurtle();
};  // namespace MR

class KameckBeamKuriboMini : public LiveActor, public KameckBeamProjectile {
public:
    KameckBeamKuriboMini();
    virtual ~KameckBeamKuriboMini();
    virtual void control();
    virtual void appearDirection(const TVec3f&, const TVec3f&);
    virtual void resetBeamProjectile();
    virtual bool isBeamProjectileDead() const;
    KuriboMini* mEnemy;
    s32 mStep;
};
class KameckBeamMeramera : public LiveActor, public KameckBeamProjectile {
public:
    KameckBeamMeramera();
    virtual ~KameckBeamMeramera();
    virtual void control();
    virtual void appearDirection(const TVec3f&, const TVec3f&);
    virtual void resetBeamProjectile();
    virtual bool isBeamProjectileDead() const;
    Meramera* mEnemy;
    s32 mStep;
};
