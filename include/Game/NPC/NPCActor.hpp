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

    /* 0x00 */ const char* _0;
    /* 0x04 */ bool mModel;
    /* 0x08 */ const char* mObjectName;
    /* 0x0C */ bool mMakeActor;
    /* 0x0D */ bool mHostIO;
    /* 0x0E */ bool mMessage;
    /* 0x0F */ bool _F;
    /* 0x10 */ const char* _10;
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
    /* 0x6F */ bool _67;
    /* 0x70 */ bool mPointer;
    /* 0x74 */ const char* _6C;
    /* 0x78 */ const char* _70;
    /* 0x7C */ bool _7C;  // new in SMG2
    /* 0x80 */ TVec3f mStarPointerOffs;
    /* 0x8C */ f32 mPointerSize;
    /* 0x90 */ s32 mSceneConnectionType;
    /* 0x94 */ bool _94;  // new in SMG2
    /* 0x95 */ bool _95;  // new in SMG2 (setDefault sets it)
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
        return !_DD && _E2;
    }

    bool isReactionStart() const {
        return !_E0 && _E5;
    }

    bool isSpinAttackedStart() const {
        return !_DE && _E3;
    }

    bool isPointingStart() const {
        return !_DF && _E4;
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
        _130 = cNPCSpinDefault;
        _134 = cNPCTrampledDefault;
        _138 = cNPCPointingDefault;
        _13C = cNPCReactionDefault;
    }

    inline void setDefaults(const char* pReaction, const char* pPointing, const char* pTrampled, const char* pSpin) {
        _130 = pSpin;
        _134 = pTrampled;
        _138 = pPointing;
        _13C = pReaction;
    }

    inline void setDefaults2(const char* pReaction, const char* pPointing, const char* pSpin, const char* pTrampled) {
        _134 = pTrampled;
        _130 = pSpin;
        _138 = pPointing;
        _13C = pReaction;
    }

    inline void setDefaults3(const char* pReaction, const char* pPointing, const char* pSpin, const char* pTrampled) {
        _134 = pTrampled;
        _130 = pSpin;
        _13C = pPointing;
        _138 = pReaction;
    }

    inline void setDefaults4(const char* a1, const char* a2, const char* a3, const char* a4) {
        _130 = a4;
        _13C = a2;
        _134 = a3;
        _138 = a1;
    }

    inline void setDefaultsParam() {
        mParam._14 = "Wait";
        mParam._18 = "Turn";
        mParam._1C = "Talk";
        mParam._20 = "TalkTurn";
        _130 = cNPCSpinDefault;
        _134 = cNPCTrampledDefault;
        _138 = cNPCPointingDefault;
        _13C = cNPCReactionDefault;
    }

    inline void setDefaults2() {
        _130 = cNPCSpinDefault;
        _134 = cNPCTrampledDefault;
        _138 = cNPCPointingDefault;
    }

    inline void setTalkAction(const char* pActionName) {
        _11C = pActionName;
        _120 = pActionName;
    }

    inline void setTalkAction(const char* pActionName, const char* pActionName2) {
        _11C = pActionName;
        _120 = pActionName2;
    }

    TalkMessageCtrl* getMsgCtrl() const {
        return mMsgCtrl;
    }

    /* 0x8C */ LodCtrl* mLodCtrl;
    /* 0x90 */ TalkMessageCtrl* mMsgCtrl;
    PartsModel* _94;
    PartsModel* _98;
    s32 _9C;
    TQuat4f _A0;
    TQuat4f _B0;
    TVec3f _C0;
    TVec3f _CC;
    u8 _D8;
    u8 _D9;
    u8 _DA;
    u8 _DB;
    u8 _DC;
    u8 _DD;
    u8 _DE;
    u8 _DF;
    u8 _E0;
    u8 _E1;
    u8 _E2;
    u8 _E3;
    u8 _E4;
    u8 _E5;
    u8 _E6;
    u8 _E7;
    /* 0xE8 */ MR::ActorTalkParam mParam;
    f32 _10C;
    f32 _110;
    f32 _114;
    f32 _118;
    const char* _11C;
    const char* _120;
    u8 _124;  // follow rail on ground
    u8 _125;
    u8 _126;
    u8 _127;
    u8 _128;
    f32 _12C;
    const char* _130;
    const char* _134;
    const char* _138;
    const char* _13C;
    /* 0x140 */ AnimScaleController* mScaleController;
    /* 0x144 */ JointController* mDelegator;
    YoshiLockOnTarget* mYoshiTarget;  // 0x14C, new in SMG2 (YoshiLockOnTarget*)
    /* 0x150 */ const Nerve* mCurNerve;
    /* 0x14C */ Nerve* mWaitNerve;
    /* 0x150 */ Nerve* mTalkNerve;
    /* 0x154 */ Nerve* mReactionNerve;
    s32 _158;
};
