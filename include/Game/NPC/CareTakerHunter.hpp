#pragma once

#include "Game/NPC/NPCActor.hpp"

class ActorCameraInfo;
class LiveActorGroup;
class NameObj;
class TakeOutStar;

class BringEnemy : public LiveActor {
public:
    s32 getEnemyType() const;
};

class CareTakerHunter : public NPCActor {
public:
    /// @brief Creates a new `CareTakerHunter`.
    /// @param pName A pointer to the null-terminated name of the object.
    CareTakerHunter(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    bool branchFunc(u32);
    bool eventFunc(u32);

    LiveActor* updateRotation();
    void setTalkStrings(s32);

    void exeReaction();
    void exeTalk();
    void exeTalkBegin();
    void exeTalkEnd();
    void exeWait();
    void exeGrab();
    void exeGrabWait();
    void exeHide();
    void exeHideWait();
    void exeAppear();
    void exeAppeared();
    void exeWipeIn();
    void exeLeave();
    void exeEnd();

private:
    /* 0x164 */ TakeOutStar* mTakeOutStar;
    /* 0x168 */ s32 mObjArg0;
    /* 0x16C */ s32 mObjArg4;
    /* 0x170 */ s32 mBodyColorFrame;
    /* 0x174 */ ActorCameraInfo* mCameraInfo0;
    /* 0x178 */ ActorCameraInfo* mCameraInfo1;
    /* 0x17C */ ActorCameraInfo* mCameraInfo2;
    /* 0x180 */ ActorCameraInfo* mCameraInfo3;
    /* 0x184 */ BringEnemy* mBringEnemy;
    /* 0x188 */ LiveActor** mChildObjs;
    /* 0x18C */ s32 mChildCount;
    /* 0x190 */ LiveActorGroup* mActorGroup;
    /* 0x194 */ f32 mRotAngle;
    /* 0x198 */ TVec3f mScale;
    /* 0x1A4 */ f32 mTimer;
};
