#pragma once
#include "Game/Player/InfernoMarioStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
class ManekuriModel;

class InfernoMarioManekuri : public InfernoMarioStateBase {
public:
    InfernoMarioManekuri(InfernoMario*);
    virtual ~InfernoMarioManekuri();
    virtual void appear();
    virtual void kill();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual bool startFrame(s32);
    virtual void fn_80381750();
    virtual void fn_80381780();
    virtual void control();
    bool trample(u32, HitSensor*) NO_INLINE;
    bool blowDownPlayer(HitSensor*, HitSensor*) NO_INLINE;
    bool blowDownEnemy(HitSensor*, HitSensor*) NO_INLINE;
    bool isFollowing() const NO_INLINE;
    bool isNerveBlowDownPlayer() const NO_INLINE;
    bool isJumping() const NO_INLINE;
    bool canDamage() const NO_INLINE;
    void exeWait();
    void exeFind();
    void exeFollowStart();
    void exeJump();
    void exeFollow();
    void exeTrampleDown();
    void exeBlowDownPlayer();
    void exeBlowDownEnemy();
    bool tryFind() NO_INLINE;
    ManekuriModel* mModel;
    TVec3f mFront;
    TVec3f mStartPos;
    TVec3f mEndPos;
    s32 mFrame;
    u32 mCombo;
};
class ManekuriModel : public LiveActor {
public:
    ManekuriModel(const char*, MtxPtr);
    virtual ~ManekuriModel();
    virtual void calcAndSetBaseMtx();
    MtxPtr mParentMtx;
};
