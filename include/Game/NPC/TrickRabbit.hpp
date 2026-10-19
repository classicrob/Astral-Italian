#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class FootPrint;
class MultiEventCamera;
class ParabolicPath;
class RabbitStateWaitStart;
class RabbitStateCaught;
class RailGraph;
class RailGraphIter;
class RailGraphNodeSelecter;
class SpotMarkLight;
class TalkMessageCtrl;

class TrickRabbit : public LiveActor {
public:
    /// @brief Creates a new `TrickRabbit`.
    /// @param pName A pointer to the null-terminated name of the object.
    TrickRabbit(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void initModel();
    void initSensor();
    void initBind();
    void initRoute(const JMapInfoIter& rIter);
    void initDemoCamera(const JMapInfoIter& rIter);
    void initState();
    bool receiveMsgTrample();
    bool requestCaught();
    bool tryStartDemoRunnaway();
    void endDemoRunnaway();
    bool tryRunaway();
    bool tryBrakeTurn();
    bool tryBrakeTurnEnd();
    bool tryStop();
    bool tryJumpStart();
    bool tryJump();
    void endJump();
    bool tryJumpEnd();
    bool tryEndFallDown();
    bool tryEndFallDownLand();
    bool tryComebackRouteStart();
    bool tryComebackRoute();
    void endComebackRoute();
    bool tryEndTumble();
    bool tryEndGetUp();
    bool tryDemoJumpStart();
    bool tryDemoJump();
    void endDemoJump();
    bool tryDemoJumpEnd();
    bool tryEndPowerStarDemo();
    void exeWaitStart();
    void exeTryStartDemoRunnaway();
    void exeDemoRunaway();
    void exeDemoJumpStart();
    void exeDemoJump();
    void exeDemoEnd();
    void exeWait();
    void endWait();
    void exeRunaway();
    void exeBrakeTurn();
    void exeJumpStart();
    void exeJump();
    void exeFallDown();
    void exeFallDownLand();
    void exeGetUpFromFallDown();
    void exeComebackRouteStart();
    void exeComebackRoute();
    void exeTumble();
    void exeGetUp();
    void exeCaught();
    void exeWaitPowerStarDemo();
    void exeGiveUp();
    void doWait();
    void doRunaway();
    void doJumpStart();
    void doJump();
    void tearDownWait();
    bool checkDivingThicket(const TVec3f& rVec1, const TVec3f& rVec2);
    bool selectEdgeStartEvent(RailGraphIter* pIter);
    void selectEdgeRunnaway(RailGraphIter* pIter, const TVec3f& rVec, f32 f);
    void startRouteLevelControl(s32 l1, s32 l2) NO_INLINE;
    void controlRouteLevel();
    void addMovingAccel(const TVec3f& rVec, f32 f);
    void addKeepRouteRange(f32 f1, f32 f2, f32 f3);
    void updateFootPrint();
    bool isCaughtable() const;
    bool isTumbable() const;
    bool isNextEdgeJump() const;
    void setUpJumpParam();
    void setUpJumpParamFromJMap();
    s32 calcDefaultJumpTime() const;
    s32 calcJumpStartTime() const;
    s32 calcJumpTime() const;
    f32 calcJumpHeight() const;
    void initTimer();
    void updateTime();

    /* 0x90 */ TQuat4f mOrientation;
    /* 0xA0 */ TVec3f mFrontDirection;
    /* 0xAC */ RabbitStateWaitStart* mRabbitStateWaitStart;
    /* 0xB0 */ RabbitStateCaught* mRabbitStateCaught;
    /* 0xB4 */ MultiEventCamera* mMultiEventCamera;
    /* 0xB8 */ TalkMessageCtrl* mTalkMessageCtrl;
    /* 0xBC */ FootPrint* mFootPrint;
    /* 0xC0 */ RailGraph* mRailGraph;
    /* 0xC4 */ RailGraphIter* mRailGraphIter;
    /* 0xC8 */ ParabolicPath* mParabolicPath;
    /* 0xCC */ SpotMarkLight* mSpotMarkLight;
    /* 0xD0 */ s32 mJumpDuration;
    /* 0xD4 */ f32 mMoveSpeed;
    /* 0xD8 */ s32 mRouteLevelDelay;
    /* 0xDC */ s32 mRouteLevel;
    /* 0xE0 */ s32 mTargetRouteLevel;
    /* 0xE4 */ bool mIsChangingRouteLevel;
    /* 0xE5 */ bool mIsGhost;
    /* 0xE8 */ s32 mTimerFrames;
    /* 0xEC */ bool mTimerEnabled;
    /* 0xED */ bool mHasStartedRunaway;
};
