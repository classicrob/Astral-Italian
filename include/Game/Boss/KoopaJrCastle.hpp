#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/NameObj/NameObj.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include <JSystem/JGeometry.hpp>

namespace MR {
    f32 calcNerveEaseOutValue(const NerveExecutor*, s32, f32, f32);
}

class ActorCameraInfo;
class FluffWindEffect;
class BreakModel;
class PartsModelLodCtrl;
class KoopaJrCastle;
class KoopaJrCastleWind;
class KoopaJrCastleCannon;
class KoopaJrCastleFireBar;
class KoopaJrCastleRingBeam;
class KoopaJrCastleElectric;
class KoopaJrCastleDominate;
class KoopaJrCastleBody;
class KoopaJrCastleHead;
class KoopaJrCastleCapsule;
class KoopaJrCastleStateOpening;
class KoopaJrCastleStateBattle;

// SMG2-only unit. Layout from the ctor stores and init accesses (offsets are SMG2 real).
class KoopaJrCastle : public LiveActor {
public:
    KoopaJrCastle(const char*);
    virtual ~KoopaJrCastle();

    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual MtxPtr getBaseMtx() const;
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeBattle();

    /* 0x90 */ KoopaJrCastleWind* mWind;
    /* 0x94 */ KoopaJrCastleCannon* mCannon;
    /* 0x98 */ KoopaJrCastleFireBar* mFireBar;
    /* 0x9C */ KoopaJrCastleRingBeam* mRingBeam;
    /* 0xA0 */ KoopaJrCastleElectric* mElectric;
    /* 0xA4 */ KoopaJrCastleDominate* mDominate;
    /* 0xA8 */ KoopaJrCastleBody* mBody;
    /* 0xAC */ KoopaJrCastleHead* mHead;
    /* 0xB0 */ KoopaJrCastleCapsule* mCapsule;
    /* 0xB4 */ PartsModelLodCtrl* mKoopaJr;
    /* 0xB8 */ BreakModel* mBreakModel;
    /* 0xBC */ KoopaJrCastleStateOpening* mStateOpening;
    /* 0xC0 */ KoopaJrCastleStateBattle* mStateBattle;
    /* 0xC4 */ ActorCameraInfo* mCameraInfo;
    /* 0xC8 */ TVec3f mFrontVec;
    /* 0xD4 */ TPos3f mBaseMtx;
    /* 0x104 */ TVec3f mStartPos;
    /* 0x110 */ TVec3f _110;
    /* 0x11C */ TVec3f mCameraPos;
    /* 0x128 */ s32 mCameraRegId;
};

class JointController;
class CollisionParts;
class ModelObj;
struct JointControllerInfo;
class KoopaJrCastleStateBodyMove;
class KoopaJrCastleCannonShock;
class KoopaJrCastleCannonBall;

class KoopaJrCastleStateBodyMove : public ActorStateBase< KoopaJrCastleBody > {
public:
    KoopaJrCastleStateBodyMove(KoopaJrCastleBody*);
    virtual ~KoopaJrCastleStateBodyMove();
    virtual void init();
    virtual void appear();
    virtual void kill();

    NO_INLINE void setMode0();
    NO_INLINE void setMode1();
    NO_INLINE void setMode2();
    NO_INLINE bool isMoving() const;
    NO_INLINE u8 getFlag() const;
    void exe800CF2A0();
    void exe800CF3B0();
    void exe800CF530();
    void exe800CF5E0();
    void exe800CF8D0();
    void exe800CFA70();
    void exe800CFC40();
    void exe800CF700();
    void exe800CFDD0();
    void calcTargetDir(TVec3f*);
    bool isTurnEnd(f32);
    bool startBtkKeepFrame(const char*);
    bool isOutsideCenter();
    void setMoveRate(f32);

    /* 0x10 */ TVec3f mDirection;
    /* 0x1C */ f32 _1C;
    /* 0x20 */ s32 mMode;
    /* 0x24 */ f32 _24;
    /* 0x28 */ s32 mMoveStep;
    /* 0x2C */ u8 _2C;
};

class KoopaJrCastleBody : public PartsModel {
public:
    KoopaJrCastleBody(LiveActor*, const char*);
    virtual ~KoopaJrCastleBody();

    virtual void init(const JMapInfoIter&);
    virtual void calcAnim();
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeFall();
    void exeDead();

    NO_INLINE void setNerveDummy1();
    NO_INLINE void setNerveDummy2();
    NO_INLINE void setNerveFall();
    NO_INLINE void setNerveDead();
    NO_INLINE void setNerveWait();
    NO_INLINE void setNerveStop();
    NO_INLINE void startMove0();
    NO_INLINE void startMove1();
    NO_INLINE void startMove2();
    NO_INLINE void validateCollisionA();
    NO_INLINE void validateCollisionB();
    NO_INLINE void onFlagF4();
    NO_INLINE void offFlagF4();
    NO_INLINE void onFlagF5();
    NO_INLINE void offFlagF5();
    NO_INLINE void onFlagF6();
    NO_INLINE void resetF0();
    NO_INLINE bool isMove() const;
    NO_INLINE bool isMoveState() const;
    void clearStateMoveCount();
    bool calcBodyGear1(TPos3f*, const JointControllerInfo&);
    bool calcBodyGear2(TPos3f*, const JointControllerInfo&);
    bool calcBodyGear3(TPos3f*, const JointControllerInfo&);
    bool calcPropeller(TPos3f*, const JointControllerInfo&);
    bool calcWindmill(TPos3f*, const JointControllerInfo&);
    bool calcGearL(TPos3f*, const JointControllerInfo&);
    bool calcGearR(TPos3f*, const JointControllerInfo&);
    void calcGearMtx();
    void startMoveSound();

    /* 0xA0 */ ModelObj* mLowModel;
    /* 0xA4 */ JointController** mGearCtrl;
    /* 0xA8 */ JointController** mGearCtrlLR;
    /* 0xAC */ CollisionParts** mCollision;
    /* 0xB0 */ KoopaJrCastleStateBodyMove* mState;
    /* 0xB4 */ bool _B4;
    /* 0xB8 */ TPos3f* mGearMtx;
    /* 0xBC */ s32* mJointIdx;
    /* 0xC0 */ Mtx mMtx;
    /* 0xF0 */ f32 _F0;
    /* 0xF4 */ bool _F4;
    /* 0xF5 */ bool _F5;
    /* 0xF6 */ bool _F6;
    /* 0xF8 */ f32 _F8;
    /* 0xFC */ f32 _FC;
    /* 0x100 */ f32 mRate;
    /* 0x104 */ s32 mGearStep;
    /* 0x108 */ s32 mPropStep;
    /* 0x10C */ s32 mWindStep;
};

class KoopaJrCastleHead : public PartsModel {
public:
    KoopaJrCastleHead(LiveActor*, const char*);
    virtual ~KoopaJrCastleHead();
    NO_INLINE void setNerveA();
    NO_INLINE void setNerveB();
    NO_INLINE void setNerveC();
    NO_INLINE void setNerveD();
    NO_INLINE void fn_800D57D0();
    NO_INLINE bool fn_800D5830();
    NO_INLINE bool fn_800D5880();
    NO_INLINE bool fn_800D58D0() const;
    bool fn_800D5E90();
    void exe800D5960();
    void exe800D59F0();
    void exe800D5A80();
    void exe800D5B10();
    void exe800D5BF0();
    void exe800D5D20();
    /* 0xA0 */ void* _A0;
    /* 0xA4 */ Mtx _A4;
    /* 0xD4 */ bool _D4;
};

class KoopaJrCastleCapsule : public PartsModel {
public:
    KoopaJrCastleCapsule(LiveActor*, const char*);
    virtual ~KoopaJrCastleCapsule();
    virtual void init(const JMapInfoIter&);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
};

class KoopaJrCastleWind : public LiveActor {
public:
    void exe800D9CF0();
    void exe800D9D60();
    void exe800D9E20();
    void exe800D9EC0();
    virtual ~KoopaJrCastleWind();
    KoopaJrCastleWind(KoopaJrCastle*, const char*);
    void fn_800D9B40();
    bool fn_800D9C40();
    u8 _pad[0x8];
};

class KoopaJrCastleCannon : public LiveActor {
public:
    KoopaJrCastleCannon(KoopaJrCastle*, const char*);
    virtual ~KoopaJrCastleCannon();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void makeActorDead();

    void exe800D0640();
    void exe800D0720();
    void exe800D07E0();
    void exe800D09B0();
    void exe800D0A10();
    NO_INLINE void fn_800D0540();
    NO_INLINE void fn_800D0580();
    NO_INLINE void fn_800D05C0();
    KoopaJrCastleCannonBall* findDeadBall();
    void killAllBalls();
    bool shoot();
    void updateGauge();

    /* 0x90 */ KoopaJrCastle* mHost;
    /* 0x94 */ KoopaJrCastleCannonBall* mBall[12];
    /* 0xC4 */ KoopaJrCastleCannonShock* mShock;
    /* 0xC8 */ s32 _C8;
    /* 0xCC */ s32 _CC;
    /* 0xD0 */ s32 _D0;
    /* 0xD4 */ bool _D4;
    /* 0xD8 */ f32 _D8;
};

class KoopaJrCastleFireBar : public LiveActor {
public:
    void exe800D2B50();
    void exe800D2BF0();
    virtual ~KoopaJrCastleFireBar();
    KoopaJrCastleFireBar(const char*);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void attackSensor(HitSensor*, HitSensor*);
    void setup(LiveActor*, const TVec3f&, const TVec3f&, const char*);
    void fn_800D2C50();
    void fn_800D2CC0();
    /* 0x90 */ LiveActor* mBody;
    /* 0x94 */ char** mNames;
    /* 0x98 */ TPos3f* mMtxs;
    /* 0x9C */ LiveActor** mChildren;
    /* 0xA0 */ s32 mChildCount;
    /* 0xA4 */ f32 _A4;
    /* 0xA8 */ f32 _A8;
    /* 0xAC */ f32 _AC;
    /* 0xB0 */ f32 _B0;
    /* 0xB4 */ f32 _B4;
    /* 0xB8 */ s32 _B8;
    /* 0xBC */ f32 _BC;
    /* 0xC0 */ f32 _C0;
    /* 0xC4 */ f32 _C4;
    /* 0xC8 */ f32 _C8;
    /* 0xCC */ bool _CC;
};

class KoopaJrCastleRingBeam : public LiveActor {
public:
    KoopaJrCastleRingBeam(const char*);
    virtual ~KoopaJrCastleRingBeam();
    void setup(LiveActor*, const TVec3f&, const TVec3f&, const char*);
    void exe800D7C00();
    /* 0x90 */ void* _90;
    /* 0x94 */ f32 _94;
};

class KoopaJrCastleElectric : public LiveActor {
public:
    KoopaJrCastleElectric(const char*);
    virtual ~KoopaJrCastleElectric();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void makeActorDead();
    bool fn_800D2250();
    void exe800D20F0();
    void exe800D21A0();
};

class KoopaJrCastleDominate : public LiveActor {
public:
    KoopaJrCastleDominate(const char*);
    virtual ~KoopaJrCastleDominate();
    void exe800D7570();
    /* 0x90 */ void* _90;
    /* 0x94 */ f32 _94;
    /* 0x98 */ f32 _98;
    /* 0x9C */ f32 _9C;
    /* 0xA0 */ f32 _A0;
    /* 0xA4 */ f32 _A4;
};

class KoopaJrCastleStateOpening : public ActorStateBase< KoopaJrCastle > {
public:
    void exe800D6D80();
    void exe800D6E00();
    void exe800D6E90();
    KoopaJrCastleStateOpening(KoopaJrCastle*);
    virtual ~KoopaJrCastleStateOpening();
};

class KoopaJrCastleStateBattle : public ActorStateBase< KoopaJrCastle > {
public:
    void exe800D68C0();
    void exe800D6910();
    void exe800D6970();
    void exe800D69D0();
    void exe800D6AD0();
    KoopaJrCastleStateBattle(KoopaJrCastle*);
    virtual ~KoopaJrCastleStateBattle();
    bool isDemo() const;
    void attackSensor(HitSensor*, HitSensor*);
    bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    /* 0x10 */ ActorStateBaseInterface* mPhaseState[4];
    /* 0x20 */ s32 mPhase;
};

class KoopaJrCastleStateBattle1 : public ActorStateBase< KoopaJrCastle > {
public:
    void exe800D7F60();
    void exe800D7FE0();
    void exe800D8060();
    KoopaJrCastleStateBattle1(KoopaJrCastle*);
    virtual ~KoopaJrCastleStateBattle1();
};

class KoopaJrCastleStateBattle2 : public ActorStateBase< KoopaJrCastle > {
public:
    void exe800D82B0();
    void exe800D8330();
    void exe800D83B0();
    void exe800D8490();
    void exe800D8500();
    void exe800D8550();
    KoopaJrCastleStateBattle2(KoopaJrCastle*);
    virtual ~KoopaJrCastleStateBattle2();
};

class KoopaJrCastleStateBattle3 : public ActorStateBase< KoopaJrCastle > {
public:
    void exe800D8840();
    void exe800D88B0();
    void exe800D8920();
    void exe800D8990();
    void exe800D8A00();
    void exe800D8AC0();
    void exe800D8BA0();
    void exe800D8C90();
    void exe800D8CF0();
    void exe800D8D50();
    void exe800D8DA0();
    void exe800D8E30();
    void exe800D8E80();
    KoopaJrCastleStateBattle3(KoopaJrCastle*);
    virtual ~KoopaJrCastleStateBattle3();
    u8 _pad[0x4];
};

class KoopaJrCastleStateBattle4 : public ActorStateBase< KoopaJrCastle > {
public:
    void exe800D93F0();
    void exe800D9450();
    KoopaJrCastleStateBattle4(KoopaJrCastle*);
    virtual ~KoopaJrCastleStateBattle4();
};

class KoopaJrCastleCannonShock : public LiveActor {
public:
    KoopaJrCastleCannonShock(const char*);
    virtual ~KoopaJrCastleCannonShock();
    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    void shoot(MtxPtr, const f32&);
    void exe800D0FA0();
    void exe800D10A0();
    void exe800D1220();
    /* 0x90 */ TVec3f mDir;
    /* 0x9C */ f32 _9C;
    /* 0xA0 */ f32 _A0;
};

class KoopaJrCastleCannonBall : public LiveActor {
public:
    KoopaJrCastleCannonBall(const char*);
    virtual ~KoopaJrCastleCannonBall();
    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    void exe800D1830();
    /* 0x90 */ TVec3f mDir;
    /* 0x9C */ f32 _9C;
};

class KoopaJrCastleWindUp : public LiveActor {
public:
    KoopaJrCastleWindUp(const char*);
    virtual ~KoopaJrCastleWindUp();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    void exe800DA640();
    void exe800DA6F0();

    /* 0x90 */ PartsModel* mModel;
    /* 0x94 */ s32 mEffectNum;
    /* 0x98 */ FluffWindEffect** mEffect;
    /* 0x9C */ f32 mLength;
    /* 0xA0 */ Mtx mMtx;
};

class KoopaJrCastleWindHolder : public LiveActorGroup {
public:
    KoopaJrCastleWindHolder();
    virtual ~KoopaJrCastleWindHolder();
};

class KoopaJrCastleAccessor : public NameObj {
public:
    KoopaJrCastleAccessor();
    virtual ~KoopaJrCastleAccessor();

    /* 0x14 */ KoopaJrCastle* mCastle;
};

namespace KoopaJrCastleFunction {
    NO_INLINE KoopaJrCastle* getCastle();
    s32 getBattlePhase();
    const TVec3f& getStartPos();
    bool isCapsuleSensorHost(const KoopaJrCastle*, const HitSensor*);
    bool isPlayerOnCastle();
    bool isPlayerOnCapsule();
    bool isPlayerOnHead();
    bool isElectricActive();
    void windFn_800D4290();
    bool windIsDead();
    bool windFn_800D4310();
    void bodyStartMove0();
    void bodyStartMove1();
    void bodyStartMove2();
    void bodyTryStop();
    bool bodyIsMoveState();
    void fn_800D4B40();
    u8 bodyGetStateFlag();
    bool bodyIsNotMove();
    void cannonFn_800D4590();
    void cannonFn_800D45D0();
    void cannonFn_800D4610();
    void killCannon();
    bool isCannonDead();
    bool isHeadFn_800D4790();
    void appearFireBar();
    void killFireBar();
    void fireBarOff();
    void fireBarOn();
    void appearFireBarAndStart();
    f32 getFireBarB0();
    void killRingBeam();
    void appearElectric();
    void killElectric();
    bool isElectricDead();
};  // namespace KoopaJrCastleFunction
