#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include <JSystem/JGeometry.hpp>

class LiveActorGroup;
class AnimScaleController;
class AnimScaleParam;
class PlayerRocketSmokeMoveParticleCallBack;
class PlayerRocketSmokeWaitParticleCallBack;

/// Rocket the player rides on the world map (0xEC bytes, ctor 0x804E4530).
class MiniPlayerRocket : public LiveActor {
public:
    MiniPlayerRocket();

    virtual ~MiniPlayerRocket();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void updateHitSensor(HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void disablePlayerBind();
    void enablePlayerBind();
    void resetAction();
    bool changeAction(const char* pName);
    void setMoveDirection(const TVec3f& rDir);
    void setMoveDirectionInstant(const TVec3f& rDir);
    void startBounce();
    void setPlayerMtx(const f32 (*pMtx)[4]);

private:
    bool emitChimneySmoke(const char* pName);
    void updateChimneySmoke();
    void updateAction();
    void updateMove();
    void updateRotation(const TVec3f& rDir, f32 blendRate);
    bool updateStarPieceScatter(s32 count);

public:
    /* 0x90 */ s32 mDirState;                  // 1: moving right, 2: moving left, 0: none
    /* 0x94 */ f32 mMoveSpeed;                 // rotation blend rate, set by setMoveDirection
    /* 0x98 */ s32 mWaitCount;                 // frames before the move update resumes
    /* 0x9C */ LiveActor* mBindHost;           // sensor host while the player is bound
    /* 0xA0 */ TPos3f mPlayerMtx;              // base matrix of the rocket's player
    /* 0xD0 */ LiveActorGroup* mStarPieces;    // scattered star pieces (10)
    /* 0xD4 */ AnimScaleController* mScaleCtrl;
    /* 0xD8 */ AnimScaleParam* mScaleParam;
    /* 0xDC */ s32 mActionTimer;
    /* 0xE0 */ PlayerRocketSmokeMoveParticleCallBack* mSmokeMoveCb;
    /* 0xE4 */ PlayerRocketSmokeWaitParticleCallBack* mSmokeWaitCb;
    /* 0xE8 */ s32 mStarPieceNum;
};

namespace NrvMiniPlayerRocket {
    class NrvBind : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static NrvBind sInstance;
    };

    class NrvNone : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static NrvNone sInstance;
    };
}
