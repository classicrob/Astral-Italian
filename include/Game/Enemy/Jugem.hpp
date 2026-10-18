#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util/JointController.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class AnimScaleController;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;
class JointController;
class Togezo;
class JugemCloud;

/// @brief Jugem (lakitu-like cloud enemy that carries and throws Togezo children). SMG2-only.
/// Layout derived from SB4E01 (LiveActor is 0x90 bytes in SMG2).
class Jugem : public LiveActor {
public:
    Jugem(const char*);
    virtual ~Jugem();

    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void initBinder();       // fn_8019EC60
    void initBinderFall();   // fn_8019ECC0
    bool calcRootJoint(TPos3f*, const JointControllerInfo&);
    bool calcWristJoint(TPos3f*, const JointControllerInfo&);

    // Nerve executors (names from the action names they start).
    void exeWait();            // fn_8019B780
    void exeSearch();          // fn_8019B830
    void exeFollow();          // fn_8019B900
    void exeApproach();        // fn_8019B9E0
    void exePickTogezo();      // fn_8019BC00
    void exeMoveUp();          // fn_8019BD50
    void exeThrowSign();       // fn_8019BEC0
    void exeThrow();           // fn_8019BF30
    void exeThrowEnd();        // fn_8019C060
    void exeHitReaction();     // fn_8019C0F0
    void exeStarPieceReaction();  // fn_8019C180
    void exeBlowDown();        // fn_8019C200
    void exePressDown();       // fn_8019C320
    void exeDead();            // fn_8019C3B0
    void exeReBirth();         // fn_8019C5F0
    void exeFallDownEaten();   // fn_8019C6D0
    void exeFallLand();        // fn_8019C800
    void exeFallRecovery();    // fn_8019C8F0
    void exeFallDownTrample(); // fn_8019C9D0
    void exeFallDownHipDrop(); // fn_8019CAC0
    void exeLandPressDown();   // fn_8019CC10
    void exeSupportFreeze();   // fn_8019CC80

    // Unnamed helpers, named by address.
    bool fn_8019D1D0();
    bool fn_8019D280();
    bool fn_8019D2F0();
    bool fn_8019D390();
    bool fn_8019D450();
    bool fn_8019D520();
    bool fn_8019D5B0();
    bool fn_8019D650();
    bool fn_8019D6C0();
    bool fn_8019D7E0(HitSensor*, HitSensor*);
    bool fn_8019D8A0();
    bool fn_8019D940();
    bool fn_8019D9E0();
    bool fn_8019DA80();
    bool fn_8019DB20(HitSensor*, HitSensor*);
    bool fn_8019DBE0();
    void fn_8019DC40(const TVec3f&);
    bool fn_8019DDE0();
    bool fn_8019DF90();
    void fn_8019DFE0(TVec3f*, s32);
    void fn_8019E250(TVec3f*, f32);
    LiveActor* fn_8019E560();
    void fn_8019E5C0(LiveActor*);
    void fn_8019E600();
    void fn_8019E8D0();
    bool fn_8019EB20();
    void fn_8019EB90();

    /* 0x090 */ JointController* mJointCtrlA;
    /* 0x094 */ JointController* mJointCtrlB;
    /* 0x098 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x09C */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0x0A0 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0x0A4 */ YoshiLockOnTarget* mYoshiLockOn2;
    /* 0x0A8 */ TPos3f mBaseMtx;
    /* 0x0D8 */ TPos3f mSubMtx;
    /* 0x108 */ JugemCloud* mCloud;
    /* 0x10C */ Togezo* mHoldTogezo;
    /* 0x110 */ Togezo** mChildObjs;
    /* 0x114 */ LiveActor** mChildren;
    /* 0x118 */ TVec3f* mChildPoses;
    /* 0x11C */ s32* mChildFlags;
    /* 0x120 */ TVec3f mFrontVec;
    /* 0x12C */ TVec3f mBasePos;
    /* 0x138 */ f32 _138;
    /* 0x13C */ s32 mChildNum;
    /* 0x140 */ s32 mChildNumPlusOne;
    /* 0x144 */ s32 mChildIndex;
    /* 0x148 */ s32 mStepCounter;
    /* 0x14C */ s32 mChildCount;
    /* 0x150 */ s32 mWaitTime;
    /* 0x154 */ LiveActor* mGroundActor;
    /* 0x158 */ u8 _158[0xC];
    /* 0x164 */ TVec3f _164;
    /* 0x170 */ s32 _170;
    /* 0x174 */ s32 _174;
    /* 0x178 */ f32 _178;
    /* 0x17C */ s32 mTimer;
    /* 0x180 */ u8 mIsActive;
    /* 0x181 */ u8 mIsHitDone;
};

/// @brief Cloud model that the Jugem stands on.
class JugemCloud : public PartsModel {
public:
    JugemCloud(Jugem* pHost, const char* pName);
    virtual ~JugemCloud();

    virtual void makeActorAppeared();
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void disappear();     // fn_8019EF80
    void tryDisappear();  // fn_8019EFA0

    /* 0xA0 */ Jugem* mJugem;
    /* 0xA4 */ bool mIsAppear;
};
