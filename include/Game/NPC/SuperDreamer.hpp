#pragma once

#include "Game/NPC/NPCActor.hpp"

class DreamerButtonGuidance;
class ModelObj;

class SuperDreamer : public NPCActor {
public:
    SuperDreamer(const char*);
    virtual ~SuperDreamer();

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    bool eventFunc(u32);
    bool killFunc(u32);

    void exeAppear();
    void exeWait();
    void exeDemoTransform();
    void exeInHelp();
    void exeEndHelp();
    void exeEndTalk();
    void exeEnd();
    void exeKill();

    void initForPlaying(const JMapInfoIter&);
    void initForNpc(const JMapInfoIter&);
    void initHelperWitchBall();
    void initHelperWitchEffect();
    void initButtonGuidance();
    bool isMakeAppear() const;
    TalkMessageCtrl* createTalkCtrl(const JMapInfoIter&, const char*, const char*);
    void attemptResetPadMode() const;
    void updatePlayerBaseMtx() const;
    void updateHelperWitchEffectMtx() const;
    void setupPadMode();

    /* 0x164 */ DreamerButtonGuidance* mButtonGuidance;
    /* 0x168 */ s32 _168;
    /* 0x16C */ s32 mStartId;
    /* 0x170 */ s32 mMissNum;
    /* 0x174 */ TalkMessageCtrl* mTalkCtrl;
    /* 0x178 */ bool mIsPadMode;
    /* 0x17C */ TPos3f mPlayerJointMtx;
    /* 0x1AC */ ModelObj* mHelperWitchBall;
    /* 0x1B0 */ ModelObj* mHelperWitchEffect;
    /* 0x1B4 */ mutable TPos3f mHelperEffectMtx;
};
