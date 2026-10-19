#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/TalkUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class AnimScaleController;
class JointController;
class JointControllerInfo;
class LodCtrl;
class NameObjArchiveListCollector;
class NPCActorItem;
class PartsModel;
class Triangle;
class TalkMessageCtrl;


extern const char cNPCReactionDefault[];
extern const char cNPCPointingDefault[];
extern const char cNPCTrampledDefault[];
extern const char cNPCSpinDefault[];
extern const char cNPCTurnDefault[];
extern const char cNPCWaitDefault[];

class NPCActorCaps {
public:
    NPCActorCaps(const char*);

    void setDefault();
    void setIndirect();

    /* 0x00 */ const char* mName;
    /* 0x04 */ bool mModel;
    /* 0x08 */ const char* mObjectName;
    /* 0x0C */ bool mMakeActor;
    /* 0x0D */ bool mHostIO;
    /* 0x0E */ bool mMessage;
    /* 0x0F */ bool mUseDirectMessage;
    /* 0x10 */ const char* mMessageId;
    /* 0x14 */ TVec3f mMessageOffset;
    /* 0x20 */ MtxPtr mTalkMtx;
    /* 0x24 */ const char* mTalkJointName;
    /* 0x28 */ bool mInterpole;
    /* 0x29 */ bool mConnectTo;
    /* 0x2A */ bool mLightCtrl;
    /* 0x2B */ bool mEffect;
    /* 0x2C */ bool mSound;
    /* 0x30 */ const char* mSoundName;  // new in SMG2
    /* 0x34 */ s32 mSoundSize;
    /* 0x38 */ bool mAttribute;
    /* 0x39 */ bool mPosition;
    /* 0x3A */ bool mLodCtrl;
    /* 0x3B */ bool mNerve;
    /* 0x3C */ s32 mNerveStep;  // new in SMG2 (second arg of initNerve)
    /* 0x40 */ bool mBinder;
    /* 0x44 */ f32 mBinderSize;
    /* 0x48 */ bool mSensor;
    /* 0x4C */ const char* mSensorJoint;
    /* 0x50 */ f32 mSensorSize;
    /* 0x54 */ TVec3f mSensorOffset;
    /* 0x60 */ s32 mSensorMax;
    /* 0x64 */ bool mShadow;
    /* 0x65 */ bool mUseShadow;
    /* 0x68 */ f32 mShadowSize;
    /* 0x6C */ bool mRailRider;
    /* 0x6D */ bool mSwitchDead;
    /* 0x6E */ bool mSwitchAppear;
    /* 0x6F */ bool mStartDead;
    /* 0x70 */ bool mPointer;
    /* 0x74 */ const char* mStarPointerJointName;
    /* 0x78 */ const char* mScaleJointName;
    /* 0x7C */ bool mUseAlternateScaleParam;  // new in SMG2
    /* 0x80 */ TVec3f mStarPointerOffs;
    /* 0x8C */ f32 mPointerSize;
    /* 0x90 */ s32 mSceneConnectionType;
    /* 0x94 */ bool mDropItemOnSupportTicoSpin;  // new in SMG2
    /* 0x95 */ bool mYoshiInteraction;  // new in SMG2 (setDefault sets it)
    /* 0x98 */ Nerve* mWaitNerve;
    /* 0x9C */ Nerve* mTalkNerve;
    /* 0xA0 */ Nerve* mReactionNerve;
};

class YoshiLockOnTarget;

class NPCActor : public LiveActor {
public:
    NPCActor(const char*);
    virtual ~NPCActor();

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    static void makeArchiveList(NameObjArchiveListCollector*, const JMapInfoIter&);
    static void addArchive(NameObjArchiveListCollector*, const NPCActorItem&);
    static void makeArchiveListDefault(NameObjArchiveListCollector*, const JMapInfoIter&);

    bool initTalkCtrl(const JMapInfoIter&, const char*, const TVec3f&, MtxPtr);
    bool initTalkCtrlDirect(const JMapInfoIter&, const char*, const TVec3f&, MtxPtr);
    bool calcJointScale(TPos3f*, const JointControllerInfo&);
    bool turnToPlayer();
    bool turnToPlayer(f32);
    bool turnToPlayer(f32, f32, f32);
    bool turnToDefault(f32);
    void setToDefault();
    void pushNerve(const Nerve*);
    const Nerve* popAndPushNerve(const Nerve*);
    const Nerve* popNerve();
    bool tryPullNullNerve();
    bool isEmptyNerve() const;
    virtual bool isReactionNerve() const;
    virtual bool isSensorSpinCloudBlock(const Triangle*);
    bool isScaleAnim() const;
    bool isPointingSe() const;
    void updateReaction();
    void updateScaleCtrl();

    bool isTrampledStart() const {
    /* 0xE6 */ return !mWasTrampled && mTrampled;
    }

    bool isReactionStart() const {
    /* 0xE9 */ return !mWasReacting && mReacting;
    }

    bool isSpinAttackedStart() const {
    /* 0xE7 */ return !mWasSpinAttacked && mSpinAttacked;
    }

    bool isPointingStart() const {
    /* 0xE8 */ return !mWasPointed && mPointed;
    }

    bool tryPushNullNerve();

    void exeReaction();
    void exeWait();
    void exeTalk();
    void exeNull();

    void setInitPose();

    void initialize(const JMapInfoIter&, const NPCActorCaps&, const char*, const char*, const char*);

    void equipment(const NPCActorItem&, bool);

    void setBaseMtx(const TPos3f&);
    void setBaseMtx(MtxPtr);

    inline void setDefaults() {
        mSpinAction = cNPCSpinDefault;
        mTrampledAction = cNPCTrampledDefault;
        mPointingAction = cNPCPointingDefault;
        mReactionAction = cNPCReactionDefault;
    }

    inline void setDefaults(const char* pReaction, const char* pPointing, const char* pTrampled, const char* pSpin) {
        mSpinAction = pSpin;
        mTrampledAction = pTrampled;
        mPointingAction = pPointing;
        mReactionAction = pReaction;
    }

    inline void setDefaults2(const char* pReaction, const char* pPointing, const char* pSpin, const char* pTrampled) {
        mTrampledAction = pTrampled;
        mSpinAction = pSpin;
        mPointingAction = pPointing;
        mReactionAction = pReaction;
    }

    inline void setDefaults3(const char* pReaction, const char* pPointing, const char* pSpin, const char* pTrampled) {
        mTrampledAction = pTrampled;
        mSpinAction = pSpin;
        mReactionAction = pPointing;
        mPointingAction = pReaction;
    }

    inline void setDefaults4(const char* a1, const char* a2, const char* a3, const char* a4) {
        mSpinAction = a4;
        mReactionAction = a2;
        mTrampledAction = a3;
        mPointingAction = a1;
    }

    inline void setDefaultsParam() {
        mParam._14 = "Wait";
        mParam._18 = "Turn";
        mParam._1C = "Talk";
        mParam._20 = "TalkTurn";
        mSpinAction = cNPCSpinDefault;
        mTrampledAction = cNPCTrampledDefault;
        mPointingAction = cNPCPointingDefault;
        mReactionAction = cNPCReactionDefault;
    }

    inline void setDefaults2() {
        mSpinAction = cNPCSpinDefault;
        mTrampledAction = cNPCTrampledDefault;
        mPointingAction = cNPCPointingDefault;
    }

    inline void setTalkAction(const char* pActionName) {
        mRailMoveAction = pActionName;
        mRailTalkAction = pActionName;
    }

    inline void setTalkAction(const char* pActionName, const char* pActionName2) {
        mRailMoveAction = pActionName;
        mRailTalkAction = pActionName2;
    }

    TalkMessageCtrl* getMsgCtrl() const {
        return mMsgCtrl;
    }

    /* 0x90 */ LodCtrl* mLodCtrl;
    /* 0x94 */ TalkMessageCtrl* mMsgCtrl;
    /* 0x98 */ PartsModel* mGoods0;
    /* 0x9C */ PartsModel* mGoods1;
    /* 0xA0 */ s32 mTrampleCooldown;
    /* 0xA4 */ TQuat4f mOrientationQuat;
    /* 0xB4 */ TQuat4f mInitOrientationQuat;
    /* 0xC4 */ TVec3f mInitPosition;
    /* 0xD0 */ TVec3f mPreviousRotation;
    /* 0xDC */ u8 mTrampledStart;
    /* 0xDD */ u8 mSpinAttackedStart;
    /* 0xDE */ u8 mPointingStart;
    /* 0xDF */ u8 mReactionStart;
    /* 0xE0 */ u8 mJetTurtleHitStart;
    /* 0xE1 */ u8 mWasTrampled;
    /* 0xE2 */ u8 mWasSpinAttacked;
    /* 0xE3 */ u8 mWasPointed;
    /* 0xE4 */ u8 mWasReacting;
    /* 0xE5 */ u8 mWasHitByJetTurtle;
    /* 0xE6 */ u8 mTrampled;
    /* 0xE7 */ u8 mSpinAttacked;
    /* 0xE8 */ u8 mPointed;
    /* 0xE9 */ u8 mReacting;
    /* 0xEA */ u8 mHitByJetTurtle;
    /* 0xEB */ u8 mDropItemOnSupportTicoSpin;
    /* 0xEC */ MR::ActorTalkParam mParam;
    /* 0x110 */ f32 mRailMoveSpeed;
    /* 0x114 */ f32 mRailMoveAccel;
    /* 0x118 */ f32 mRailTurnRate;
    /* 0x11C */ f32 mRailTalkAnimRate;
    /* 0x120 */ const char* mRailMoveAction;
    /* 0x124 */ const char* mRailTalkAction;
    /* 0x128 */ u8 mFollowRailOnGround;  // follow rail on ground
    /* 0x129 */ u8 mRailFollowReserved0;
    /* 0x12A */ u8 mRailFollowReserved1;
    /* 0x12B */ u8 mRailFollowReserved2;
    /* 0x12C */ u8 mUseReactionActions;
    /* 0x130 */ f32 mSpinAttackRange;
    /* 0x134 */ const char* mSpinAction;
    /* 0x138 */ const char* mTrampledAction;
    /* 0x13C */ const char* mPointingAction;
    /* 0x140 */ const char* mReactionAction;
    /* 0x144 */ AnimScaleController* mScaleController;
    /* 0x148 */ JointController* mDelegator;
    /* 0x14C */ YoshiLockOnTarget* mYoshiTarget;  // new in SMG2
    /* 0x150 */ const Nerve* mCurNerve;
    /* 0x154 */ Nerve* mWaitNerve;
    /* 0x158 */ Nerve* mTalkNerve;
    /* 0x15C */ Nerve* mReactionNerve;
    /* 0x160 */ s32 mFramesSincePointingSound;
};
