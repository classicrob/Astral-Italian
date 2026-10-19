#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/TriangleFilter.hpp"
#include <JSystem/J3DGraphBase/J3DStruct.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class HitSensor;
// Shared angle-wrap helper used by the floating/throw states.
namespace MR {
    void repeatDegree(f32*);
};  // namespace MR

// Per-actor gift quota, including gifts still travelling toward the receiver.
struct StarPieceReceiverInfo {
public:
    inline StarPieceReceiverInfo(NameObj* pNameObj, s32 requested, s32 pending, s32 received) : mObj(pNameObj), mRequestedCount(requested), mPendingGiftCount(pending), mGotCount(received) {
    }
    NameObj* mObj;
    u32 mRequestedCount;
    u32 mPendingGiftCount;
    u32 mGotCount;
};

// Runtime emission budget for one declaring actor; active pieces return to this budget when killed.
struct StarPieceHostInfo {
public:
    inline StarPieceHostInfo(NameObj* pNameObj, s32 declared, s32 active, s32 collected) : mObj(pNameObj), mDeclaredCount(declared), mActiveCount(active), mCollectedCount(collected) {
    }
    // A declared piece may be emitted only if it is neither active nor already collected.
    bool isAppearable() NO_INLINE {
        return mCollectedCount < mDeclaredCount && mActiveCount < mDeclaredCount - mCollectedCount;
    }

    NameObj* mObj;
    s32 mDeclaredCount;
    s32 mActiveCount;
    s32 mCollectedCount;
};

class StarPiece : public LiveActor {
public:
    // These enums retain the original numeric values; group membership does not imply collection eligibility.
    enum ColorIndex { Color_Purple, Color_Gold, Color_Green, Color_Blue, Color_Red, Color_Silver, Color_Count };
    enum ShadowMode { ShadowMode_Default = -1, ShadowMode_Dynamic = 0, ShadowMode_Disabled = 1 };
    enum GroupType { GroupType_None, GroupType_Floating, GroupType_RailMove };

    StarPiece(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void makeActorDead();
    virtual void startClipped();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    // Group switch A controls collection independently of visibility.
    inline void setCollectionEnabled(bool enabled) { mFlags._1 = enabled; }
    inline bool isCollectionEnabled() const { return mFlags._1; }

    void initAndSetFloatingFromGroup(const JMapInfoIter&);
    void initAndSetRailMoveFromGroup(const JMapInfoIter&);

    void setColor(s32);
    static s32 getNumColor();
    void appearFromGroup();
    void changeScale(f32);
    void exeFloating();
    void appearHop(const TVec3f&, const TVec3f&);
    void exeHop();
    void exeFall();
    bool isTouchToTarget(TVec3f*, f32);
    bool isEffectLight();
    void exeToTarget();
    void exeSpinDrained();
    void exeToPlayerEnd();
    void exeThrow();
    void endThrow();
    void exeThrowFall();
    void exeFollowPlayer();
    void exeRailMove();
    void setFollowPlayerAndAppear();
    bool tryCalcGravity();
    void trySetGravityAndFront(const TVec3f&);
    bool throwToTarget(HitSensor*, const TVec3f&, const TVec3f&, f32);
    bool throwToTargetCore(const TVec3f&, const TVec3f&, const TVec3f&, f32, bool);
    void giftToTarget(StarPieceReceiverInfo*, u32, HitSensor*, const TVec3f&);
    void goToPlayer(TVec3f);
    void launch(const TVec3f& launchPos, f32 randomSpeed, f32 verticalSpeed, bool ignoreWater, bool lightEffect, bool quickCollect = false);
    void launch(const TVec3f& launchPos, const TVec3f& launchDirection, f32 randomSpeed, f32 speed, bool ignoreWater, bool lightEffect, bool quickCollect = false);
    void launch(const TVec3f& launchPos, const TVec3f& launchVelocity, bool ignoreWater, bool lightEffect, bool quickCollect = false);
    f32 calcEffectScale(f32, f32, bool);
    f32 calcNearCameraScale();
    f32 calcDistToCamera();
    void emitTouchEffect(s32, f32);
    void emitGettableEffect(f32);
    void tryGotJudge();
    bool isFollowing();
    bool setFall();
    bool isOnRailMove();
    bool isFloat();
    void setReflect(const TVec3f&, const TVec3f&);
    bool isIgnoreTriOnThrow(const Triangle*);
    bool touchPlayer();
    void setHostInfo(StarPieceHostInfo*);
    bool isNerveToPlayerEnd();

    /* 0x90 */ TVec3f mFront;

    /* 0x9C */ f32 mTargetTurnRate;
    /* 0xA0 */ f32 mTargetSpeedRate;
    /* 0xA4 */ f32 mParabolaA;
    /* 0xA8 */ f32 mParabolaB;
    /* 0xAC */ TVec3f mThrowStartPos;
    /* 0xB8 */ TVec3f mThrowEndPos;  // Reused as the player-relative offset while spin-drained.

    /* 0xC4 */ TriangleFilterDelegator< StarPiece >* mThrowTriangleFilter;
    /* 0xC8 */ HitSensor* mTargetSensor;
    /* 0xCC */ s32 mGettableEffectTimer;
    /* 0xD0 */ s32 mGettableDelayCounter;
    /* 0xD4 */ s32 mPointerCollectDelay;  // new in SMG2, init -1
    /* 0xD8 */ s32 mFallKillTimer;

    /* 0xDC */ J3DGXColor mColor;

    /* 0xE0 */ GroupType mGroupType;
    /* 0xE4 */ StarPieceHostInfo* mHostInfo;
    /* 0xE8 */ StarPieceReceiverInfo* mReceiverInfo;
    /* 0xEC */ s32 mNumGift;

    /* 0xF0 */ struct {
        bool isGoToPlayer : 1;
        // Collection enabled by group switch A. Legacy name retained for StarPieceMother.
        bool _1 : 1;
        // flags related to enabling/disabling shadows
        bool isDynamicShadow : 1;
        bool isShadowDisabled : 1;

        bool isGroup : 1;
        bool isInWater : 1;
        // used in IsEffectLight
        bool isLightEffect : 1;
        bool isLaunched : 1;
    } mFlags;
};
