#pragma once

#include "Game/NPC/NPCActor.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/NPCUtil.hpp"

class TicoDemoGetPower;
class ActorCameraInfo;
class DemoObj;

/// @brief Luma
class Tico : public NPCActor {
public:
    /// @brief Creates a new `Tico`.
    /// @param pName A pointer to the null-terminated name of the object.
    Tico(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    virtual void initAfterPlacement();
    virtual bool isReactionNerve() const;

    void initBase(const JMapInfoIter&, s32);
    void initMessage(const JMapInfoIter&, const char*);
    void initMessage(const char*);
    void initRail(const JMapInfoIter&);
    bool tryReaction();
    bool killFunc(u32);
    bool eventFunc(u32);
    bool branchFunc(u32);

    void exeReaction();
    void exeDelight();
    void exeReactionTalk();
    void exeAppearRail();
    void exeAppear();
    void exeNoReaction();
    void exeTogglePlayerControl();
    void exeWait();
    void exeTalk();
    void exeMeta();
    void exeWaitSwitchABeforeRail();
    void exeMoveRail();
    void exeWaitAfterRail();
    void exeWaitBeforeRail();
    void exeMoveRailForceTalk();
    void exeFall();
    void exeFallRecover();
    void exeDemoWait() NO_INLINE;

    inline bool isRailType() const {
        return mBehavior == 6 || mBehavior == 8;
    }

    static const f32 sFloatSeMinVolume;

    /* 0x164 */ s32 mBehavior;
    /* 0x168 */ TVec3f mFloatBasePosition;
    /* 0x174 */ f32 mFloatOffset;
    /* 0x178 */ TicoDemoGetPower* mDemoGetPower;
    /* 0x17C */ TalkMessageCtrl* mStarPieceTalkCtrl;
    /* 0x180 */ TalkMessageCtrl* mReactionTalkCtrl;
    /* 0x184 */ MtxPtr mBodyJointMtx;
    /* 0x188 */ Color8 mPointLightColor;
    /* 0x18C */ const Nerve* mMetaNerve;
    /* 0x190 */ DemoStarter mDemoStarter;
    /* 0x19C */ bool mWasHitByStarPiece;
    /* 0x19D */ bool mIsReactionTalkEnabled;
    /* 0x19E */ bool mFollowGravity;
    /* 0x1A0 */ f32 mRailSpeed;
    /* 0x1A4 */ ActorCameraInfo* mCameraInfo;
    /* 0x1A8 */ DemoObj* mDemoObj;
};
