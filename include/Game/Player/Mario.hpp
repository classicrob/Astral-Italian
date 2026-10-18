#pragma once

#include "Game/Player/MarioModule.hpp"
#include "Game/Util/TriangleFilter.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class AreaObj;
class CubeCameraArea;
class FloorCode;
class GravityInfo;
class HashSortTable;
class HitInfo;
class HitSensor;
class MarioAbyssDamage;
class MarioActor;
class MarioBlown;
class MarioBump;
class MarioBury;
class MarioClimb;
class MarioCrush;
class MarioDamage;
class MarioDarkDamage;
class MarioFaint;
class MarioFireDamage;
class MarioFireDance;
class MarioFireRun;
class MarioFlip;
class MarioFlow;
class MarioFoo;
class MarioFpView;
class MarioFreeze;
class MarioFrontStep;
class MarioHang;
class MarioMagic;
class MarioModuleTask;
class MarioMove;
class MarioParalyze;
class MarioRabbit;
class MarioRecovery;
class MarioSideStep;
class MarioSkate;
class MarioSlider;
class MarioState;
class MarioStep;
class MarioStick;
class MarioStun;
class MarioSukekiyo;
class MarioSwim;
class MarioTalk;
class MarioTeresa;
class MarioWait;
class MarioWall;
class MarioWarp;
class Triangle;
struct SoundList;

class Mario : public MarioModule {
public:
    enum SeVersion {
        SeVersion_NORMAL = 0,
        SeVersion_DARK = 1,
    };

    typedef bool (Mario::*Task)(u32);
    Mario(MarioActor*);

    virtual bool postureCtrl(MtxPtr);

    void initMember();

    void updateAndClearStrideParameter();
    void checkKeyLock();
    void updateTimers();
    void updateMorphResetTimer();
    void doExtraServices();
    bool isEnableCheckGround() NO_INLINE;
    void setGroundNorm(const TVec3f&);
    void checkForceGrounding();
    void updateGroundInfo();
    void fixHeadFrontVecByGravity();
    bool isNonFixHeadVec() const;
    void createMtxDir(MtxPtr, const TVec3f&, const TVec3f&, const TVec3f&);
    void createDirectionMtx(MtxPtr);
    bool createCorrectionMtx(MtxPtr, TVec3f*);
    void createAngleMtx(MtxPtr, bool);
    void slopeTiltHead(TVec3f*);
    void fixFrontVecByGravity();
    void fixFrontVecFromUpSide();
    void fixSideVecFromFrontUp();
    void setSideVec(const TVec3f&);
    void setHeadVec(const TVec3f&);
    void setFrontVec(const TVec3f&);
    void setFrontVecKeepUp(const TVec3f&, f32);
    void setFrontVecKeepUp(const TVec3f&, u32);
    void setFrontVecKeepUp(const TVec3f&);
    void setFrontVecKeepUpAngle(const TVec3f&, f32);
    void setFrontVecKeepSide(const TVec3f&);
    void setHeadAndFrontVecFromRotate(const TVec3f&);
    void forceSetHeadVecKeepSide(const TVec3f&);
    void setGravityVec(const TVec3f&);
    void draw() const;
    void addTrans(const TVec3f&, const char*);
    void setTrans(const TVec3f&, const char*);
    bool isEnableRush() const;
    bool isForceStopRush() const;
    bool isInvincible() const;
    bool isCeiling() const;
    void inputStick();
    void updateSoundCode();
    const TVec3f& getShadowNorm() const;
    const TVec3f& getAirGravityVec() const;
    const TVec3f getAirFrontVec() const;
    const TVec3f* getGravityVec() const;
    void initAfterConst();
    void writeBackPhyisicalVector();
    void update();
    void updateLookOfs();
    void actionMain();
    void touchWater();

    bool isBlendWaitGround() const;
    void checkSpecialWaitAnimation();
    void resetSleepTimer();

    void connectToClimb();

    bool isIgnoreTriangle(const Triangle*);
    void checkBaseTransBall();
    void createAtField(bool, f32);
    void doSwimmingHitCheck(const HitInfo*, u32);
    void doSpinPunchAroundPolygons();
    void checkMap();
    f32 calcDistToCeil(bool);
    f32 calcDistToCeilOnPress();
    f32 calcDistToCeilHead();
    void fixTransBetweenWall(const TVec3f&, const TVec3f&);
    f32 calcDistWidth();
    void updateCameraPolygon();
    void setCameraPolygon(const Triangle*);
    void checkAllWall(const TVec3f&, f32);
    void calcFrontFloor();
    const TVec3f& getWallNorm() const;
    const TVec3f& getSideWallNorm() const;
    const TVec3f& getFrontWallNorm() const;
    const TVec3f& getBackWallNorm() const;
    const TVec3f& getWallPos() const;
    const Triangle* getWallPolygon() const;
    const Triangle* getGroundPolygon() const;
    void updateFloorCode();
    void updateWallFloorCode();
    void saveLastSafetyTrans();
    void setNotSafetyTimer();
    const TVec3f* getLastSafetyTrans(TVec3f*) const;
    bool checkCurrentFloorCodeSevere(u32) const;
    bool isCurrentFloorSink() const;
    bool isCurrentFloorSand() const;
    bool isCurrentShadowFloorDangerAction() const;
    void checkBaseTransPoint();
    void checkHeadPoint();
    void calcShadowPos();
    bool updateBinderInfo();
    bool isThroughWall(const Triangle*) const;
    bool checkGround();
    CubeCameraArea* getCameraCubeCode() const;
    void updateCubeCode();

    bool isDamaging() const;
    bool damageLarge(const TVec3f&);
    void decDamageAfterTimer();
    bool checkDamage();
    u16 getDamageAfterTimer() const;
    bool damageFloorCheck();
    bool damageWallCheck();
    bool damagePolygonCheck(const Triangle*);
    bool flipLarge(const TVec3f&);
    bool isEnableAddDamage() const;
    bool damage(const TVec3f&);
    bool doAbyssDamage();
    void connectToFireRun();
    bool doFireDanceWithInitialDamage(u8);
    bool doFireObjHitWithInitialDamage();
    bool doNeedleWithInitialDamage(u8);
    bool doNeedleWithInitialDamage(const Triangle*);
    bool doNeedle(const Triangle*);
    bool doFireDance();
    void checkKarikariDamage();
    bool doDarkDamage();
    bool doParalyze();
    bool doFreeze();
    bool requestCrush();
    bool tryCrush();

    bool doFlipWeak(const TVec3f&);
    bool faint(const TVec3f&);

    bool doFlipJump(const TVec3f&);
    bool doFlipBackRoll(const TVec3f&);
    bool doFlipLarge(const TVec3f&);
    bool doFlipRot(const TVec3f&);

    bool doFrontStep();

    void beeMarioOnGround();
    bool beeMarioOnAir();

    bool blown(const TVec3f&);

    bool taskOnEffectCheck(u32);

    bool doFlow();

    void tryStartFoo();

    bool isRising() const;
    void checkWallRiseAndSlipFront();
    void tryJump();
    void tryTurnJump();
    void trySquatJump();
    void tryBackJump();
    void tryTornadoJump();
    void startTornadoCentering(HitSensor*);
    bool taskOnTornadoCentering(u32);
    void trySpinJump(u8);
    void tryForceJumpDelay(const TVec3f&);
    void tryFreeJumpDelay(const TVec3f&);
    void tryForceJump(const TVec3f&, bool);
    void tryForceFreeJump(const TVec3f&);
    void tryForcePowerJump(const TVec3f&, bool);
    void tryFreeJump(const TVec3f&, bool);
    void tryWallJump(const TVec3f&, bool);
    void tryStickJump(const TVec3f&);
    void trySlipUpJump();
    void tryHangSlipUp();
    void tryDrop();
    bool isDigitalJump() const NO_INLINE;
    void initActiveJumpVec();
    void initJumpParam();
    bool isEnableFutureJump() const;
    void procJump(bool);
    void checkWallRising() NO_INLINE;
    void checkWallJumpHit();
    void decideSlipUp();
    void moveWallSlide(f32);
    bool jumpToHipDrop();
    void procHipDrop();
    void doAirWalk();
    void stopJump();
    void cancelTornadoJump();
    void setRocketBooster(const TVec3f&, f32, u16);
    void procRocketBooster();
    bool isSoftLandingFloor() const;
    void checkAndTryForceJump();
    void doLanding();
    void startSlidingTask(u32, f32, u16);
    bool taskOnSlide(u32);
    bool taskOnWallRising(u32);
    void incAirWalkTimer();

    void mainMove();
    bool isEnableTurn();
    void recordTurnSlipAngle();
    f32 decideInertia(f32);
    f32 decideInertiaOnIce(f32);
    f32 decideInertiaOnSlip(f32);
    void calcShadowDir(const TVec3f&, TVec3f*);
    bool retainMoveDir(f32, f32, TVec3f*);
    void calcMoveDir(f32, f32, TVec3f*, bool);
    bool checkLockOnHoming();
    void doLockOnHoming();
    void fixPositionInTower();

    void check2DMode();
    void calcMoveDir2D(f32, f32, TVec3f*);
    void calcShadowDir2D(const TVec3f&, TVec3f*);
    void stick2Dadjust(f32&, f32&);
    void set2Dmode(bool);
    void beforeJumping2D();
    void afterLanding2D();
    void clear2DStick();

    void set25Dmode(const AreaObj*);
    void update25Dmode();
    void updateAxisFromMode(u8);
    void calcMoveDir25D(f32, f32, TVec3f*);

    bool checkPressDamage();
    bool checkVerticalPress(bool);
    bool checkSidePressPre();
    bool checkSidePress();

    void startRabbitMode();
    void endRabbitMode();

    bool checkWallJumpCode();
    bool doSideStep();

    bool isSkatableFloor() const;
    bool doSkate();

    u32 initSoundTable(SoundList* list, u32);
    void initSound();
    void playSoundJ(const char*, s32);
    void stopSoundJ(const char*, u32);
    void startBas(const char*, bool, f32, f32);
    bool isRunningBas(const char*) const;
    void skipBas(f32);
    void playSoundTeresaFlying();
    void playSoundTrampleCombo(u8);
    void setSeVersion(u32);

    void checkOnimasu(const HitSensor*);
    bool isDossun(const Triangle*) const;
    bool isStageCameraRotate2D() const;
    bool isNoWalkFallOnDossun() const;
    bool isNotReflectGlassGround() const;
    bool isUseAnotherMovingPolygon() const;
    bool isUseFooSpecialGravity(const TVec3f&, TVec3f*) const;
    void updateOnimasu();
    bool isHeadPushEnableArea() const;
    bool isOnimasuBinderPressSkip() const;

    void checkTornado();
    void resetTornado();
    void calcTornadoTilt();
    void reflectWallOnSpinning(const TVec3f&, u16);
    void forceStopTornado();
    void startRotationTask(u32);
    void doSpinWallEffect();
    bool taskOnRotation(u32);

    void sendStateMsg(u32);
    bool updatePosture(MtxPtr);
    void changeStatus(MarioState*);
    void closeStatus(MarioState*);
    u32 getCurrentStatus() const;
    bool isStatusActive(u32) const;

    void checkBeeStick();
    void tryBeeStick(const HitSensor*);

    void startTalk(const LiveActor*);
    void endTalk();
    void setLastNonFixMoveVec(const TVec3f&);

    bool isWalling() const;
    void checkWallStick();
    u8 checkStickWallSide();
    s32 checkStickFrontBack();
    bool fixWallingPosition(bool);
    void fixWallingDir(bool);
    bool fixWallingTop();
    bool checkWallFloorCode(u16) const;
    bool checkWallCode(const char*, bool) const;
    bool checkWallCodeNorm(u16, TVec3f*, bool) const;
    void setWallCancel();
    void keepDistFrontWall();
    bool isEnableStickWall();
    bool fixWallingDist();
    bool isInhibitWall() const;
    void tryWallPunch();

    bool isHanging() const;
    bool fixHangDir(const TVec3f&, TVec3f*);
    bool isEnableBackHang();
    bool isEnableSideHang();
    void checkHang();
    bool isEnableHang();

    bool isSwimming() const;
    bool forceStartSwimAndShoot(const TVec3f&);
    bool forceExitSwim();
    bool checkStartSwim();
    void startSwim();

    bool checkWaterDamage();

    void clearSlope();
    bool isEnableSlopeMove() const;
    void moveSlopeSlide();
    void slopeMove();
    bool taskOnSlipTurn(u32);

    void lockGroundCheck(void*, bool);
    void unlockGroundCheck(void*);
    bool isUseSimpleGroundCheck() const;
    bool checkGroundOnSlope();

    void stopWalk();
    void cancelSquatMode();
    f32 getTargetWalkSpeed() const;
    void decideSquatWalkAnimation();
    void decideWalkSpeed();
    void decideWalkAnimation();
    void doBrakingAnimation();
    void checkWallPush();
    void updateBrakeAnimation();
    void updateWalkSpeed();
    void decideOnIceAnimation();
    void updateOnSand();
    void updateOnPoison();
    void updateOnWater();

    bool doRecovery();

    bool isSliderFloor() const NO_INLINE;
    bool checkSliderMode() const;
    void startSlider();

    void checkStep();
    void startStep(const TVec3f&);

    void checkBump();
    void startBump(const TVec3f&);

    void checkEnforceMove();
    void checkEnforceMoveInner();
    void recordRelativePosition();
    void invalidateRelativePosition();
    TPos3f* getMoveBaseMtx() const;
    u32 moveRelativePosition(u32);
    u32 moveRelativePositionWall();
    void recordLastGround();
    void getLastGroundPos(TVec3f*) const;
    const TVec3f* getLastGroundEdgeNrm(u32) const;
    u32 getLastGroundEdgeIndex(const TVec3f&, const TVec3f&) const;
    void pushedByReaction();
    void addReaction(const TVec3f&);
    void tryPushToVelocity();
    void push(const TVec3f&);
    void push2(const TVec3f&);
    void powerAreaMove();
    void powerRailMove();
    void recordJumpEnforceMove();
    void doEnforceJump(f32);
    void pushedByWind();

    void delTask(MarioModuleTask*);
    void execTask();
    void drawTask() const;
    void initTask();
    bool isActiveTask(Task);
    bool isActiveTaskID(u32);
    bool pushTask(Task, u32);
    void popTask(Task);
    void callExtraTasks(u32);
    void startHandy();
    bool taskOnHipDropBlurHopper(u32);
    bool taskOnHipDropBlur(u32);
    bool taskOnHipDropSlide(u32);
    bool taskOnFreezeEnd(u32);
    void startFreezeEnd();
    bool taskOnHandy(u32);
    void startHipDropBlur();
    void startHipDropSlide(const HitSensor*);
    void startJumpDropSlide(const HitSensor*);

    void startTeresaMode();
    bool getHitWallNorm(TVec3f*);
    void resetTeresaMode();
    void doTeresaReflection(const TVec3f&, bool);
    void startTeresaDisappear();

    void stopPunch();
    void startMagic();

    bool doObjWarp(LiveActor*);
    bool doPointWarp(const TVec3f&, const TVec3f&, s32);
    bool isVisibleRecoveryWarpBubble() const;
    void doCubeWarp();
    bool doPointWarpRecovery(const TVec3f&, const TVec3f&);

    bool isDisableStayHere() const;
    bool isDisableFpViewMode() const;
    void tryFpViewMode();

    void stick2DadjustGround(f32&, f32&);
    void calcDir2D(f32, f32, TVec3f*);

    // instruction comments to make it easier to identify each bit
    struct MovementStates {
        /* 0x00 */
        unsigned jumping : 1;  // _0 (srwi rX, rX, 31)
        unsigned _1 : 1;       // extrwi rX, rX, 1, 1
        unsigned _2 : 1;
        unsigned turning : 1;  // _3
        unsigned _4 : 1;
        unsigned _5 : 1;
        unsigned _6 : 1;
        unsigned _7 : 1;
        unsigned _8 : 1;
        unsigned _9 : 1;
        unsigned _A : 1;  // extrwi rX, rX, 1, 10
        unsigned _B : 1;
        unsigned _C : 1;
        unsigned _D : 1;
        unsigned _E : 1;
        unsigned _F : 1;
        unsigned _10 : 1;  // rlwinm rX, rX, 0, 17, 15
        unsigned _11 : 1;
        unsigned _12 : 1;
        unsigned _13 : 1;
        unsigned _14 : 1;
        unsigned _15 : 1;
        unsigned debugMode : 1;  // _16
        unsigned _17 : 1;        // extrwi rX, rX, 1, 23
        unsigned _18 : 1;        // rlwinm rX, rX, 0, 25, 23
        unsigned _19 : 1;        // extrwi rX, rX, 1, 25
        unsigned _1A : 1;        // extrwi rX, rX, 1, 26
        unsigned _1B : 1;
        unsigned _1C : 1;
        unsigned _1D : 1;
        unsigned digitalJump : 1;  // _1E
        unsigned _1F : 1;
        /* 0x04 */
        unsigned _20 : 1;
        unsigned _21 : 1;
        unsigned _22 : 1;
        unsigned _23 : 1;  // extrwi rX, rX, 1, 3
        unsigned _24 : 1;
        unsigned _25 : 1;
        unsigned _26 : 1;
        unsigned _27 : 1;
        unsigned _28 : 1;
        unsigned _29 : 1;
        unsigned _2A : 1;
        unsigned _2B : 1;
        unsigned _2C : 1;
        unsigned _2D : 1;
        unsigned _2E : 1;
        unsigned _2F : 1;
        unsigned _30 : 1;
        unsigned _31 : 1;
        unsigned _32 : 1;
        unsigned _33 : 1;
        unsigned _34 : 1;
        unsigned _35 : 1;
        unsigned _36 : 1;
        unsigned _37 : 1;
        unsigned _38 : 1;  // rlwimi rX, rX, 7, 24, 24
        unsigned _39 : 1;
        unsigned _3A : 1;
        unsigned _3B : 1;
        unsigned _3C : 1;
        unsigned _3D : 1;
        unsigned _3E : 2;  // clrrwi rX, rX, 2
    };
    struct DrawStates {
        unsigned _0 : 1;
        unsigned _1 : 1;
        unsigned _2 : 1;
        unsigned _3 : 1;
        unsigned _4 : 1;
        unsigned _5 : 1;
        unsigned _6 : 1;
        unsigned _7 : 1;
        unsigned _8 : 1;
        unsigned _9 : 1;
        unsigned _A : 1;
        unsigned _B : 1;
        unsigned _C : 1;
        unsigned _D : 1;
        unsigned _E : 1;
        unsigned _F : 1;
        unsigned _10 : 1;
        unsigned _11 : 1;
        unsigned mIsUnderwater : 1;
        unsigned _13 : 1;
        unsigned _14 : 1;
        unsigned _15 : 1;
        unsigned _16 : 1;
        unsigned _17 : 1;
        unsigned _18 : 1;
        unsigned _19 : 1;
        unsigned _1A : 1;
        unsigned _1B : 1;
        unsigned _1C : 1;
        unsigned _1D : 1;
        unsigned _1E : 1;
        unsigned _1F : 1;
    };

    inline const MovementStates& getMovementStates() const {
        return mMovementStates;
    }

    inline const DrawStates& getDrawStates() const {
        return mDrawStates;
    }

    inline const DrawStates& getPrevDrawStates() const {
        return mPrevDrawStates;
    }

    const TVec3f& getUpVec() const {
        return _1FC;
    }

    const TVec3f& getFrontVec() const {
        return mFrontVec;
    }

    const TVec3f& getJumpVec() const {
        return mJumpVec;
    }

    inline void setJumpVec(const TVec3f& rVec) {
        mJumpVec = rVec;
    }

    inline void resetInline() {
        _74C = 0.0f;
        _750 = 0;
        _754 = 0;
    }

    inline void set3BC(u16 val) {
        _3BC = val;
    }

    inline void set41E(u16 val) {
        _41E = val;
    }

    /* 0x008 */ u8 _pad8[0x4];
    union {
        /* 0x00C */ MovementStates mMovementStates;
        struct {
            /* 0x00C */ u32 mMovementStates_LOW_WORD;
            /* 0x010 */ u32 mMovementStates_HIGH_WORD;
        };
    };
    union {
        /* 0x014 */ MovementStates _10;
        struct {
            /* 0x014 */ u32 _10_LOW_WORD;
            /* 0x018 */ u32 _10_HIGH_WORD;
        };
    };
    union {
        /* 0x01C */ DrawStates mDrawStates;
        /* 0x01C */ u32 mDrawStates_WORD;
    };
    union {
        /* 0x020 */ DrawStates _1C;
        /* 0x020 */ u32 _1C_WORD;
    };
    union {
        /* 0x024 */ MovementStates _20;
        struct {
            /* 0x024 */ u32 _20_LOW_WORD;
            /* 0x028 */ u32 _20_HIGH_WORD;
        };
    };
    /* 0x02C */ u32 _28;
    /* 0x030 */ u32 _2C;
    /* 0x034 */ DrawStates mPrevDrawStates;
    /* 0x038 */ u32 _34;
    /* 0x03C */ f32 _38;
    /* 0x040 */ f32 _3C;  // SMG2 offset guess
    /* 0x044 */ MarioTeresa* mTeresa;
    /* 0x048 */ TVec3f _40;  // SMG2 offset guess-gap
    /* 0x054 */ TVec3f _4C;  // SMG2 offset guess-gap
    /* 0x060 */ TVec3f _58;  // SMG2 offset guess-gap
    /* 0x06C */ TMtx34f _64;  // SMG2 offset guess-gap
    /* 0x09C */ TMtx34f _94;  // SMG2 offset guess-gap
    /* 0x0CC */ TMtx34f _C4;  // SMG2 offset guess-gap
    /* 0x0FC */ TMtx34f _F4;  // SMG2 offset guess-gap
    /* 0x12C */ f32 _124;  // SMG2 offset guess-gap
    /* 0x130 */ f32 _128;  // SMG2 offset guess-gap
    /* 0x134 */ u32 _12C;  // SMG2 offset guess-gap
    /* 0x138 */ TVec3f mPosition;  // SMG2 offset guess-gap
    /* 0x144 */ TVec3f _13C;  // SMG2 offset guess-gap
    /* 0x150 */ TVec3f _148;  // SMG2 offset guess-gap
    /* 0x15C */ TVec3f _154;  // SMG2 offset guess-gap
    /* 0x168 */ TVec3f mVelocity;  // SMG2 offset guess-gap
    /* 0x174 */ TVec3f _16C;  // SMG2 offset guess-gap
    /* 0x180 */ TVec3f mVelocityAfter;  // SMG2 offset guess-gap
    /* 0x18C */ TVec3f _184;  // SMG2 offset guess-gap
    /* 0x198 */ TVec3f _190;  // SMG2 offset guess-gap
    /* 0x1A4 */ TVec3f _19C;  // SMG2 offset guess-gap
    /* 0x1B0 */ TVec3f _1A8;  // SMG2 offset guess-gap
    /* 0x1BC */ u8 _pad1BC[0x4];
    /* 0x1C0 */ TVec3f mStickPos;
    /* 0x1CC */ TVec3f mWorldPadDir;  // SMG2 offset inh
    /* 0x1D8 */ TVec3f mAirGravityVec;  // SMG2 offset inh
    /* 0x1E4 */ TVec3f _1E4;  // SMG2 offset inh
    /* 0x1F0 */ TVec3f mHeadVec;  // SMG2 offset inh
    /* 0x1FC */ TVec3f _1FC;  // SMG2 offset inh
    /* 0x208 */ TVec3f mFrontVec;  // SMG2 offset inh
    /* 0x214 */ TVec3f _214;  // SMG2 offset inh
    /* 0x220 */ TVec3f _220;  // SMG2 offset inh
    /* 0x22C */ TVec3f _22C;  // SMG2 offset inh
    /* 0x238 */ TVec3f _238;  // SMG2 offset inh
    /* 0x244 */ TVec3f _244;  // SMG2 offset inh
    /* 0x250 */ TVec3f _250;  // SMG2 offset inh
    /* 0x25C */ TVec3f _25C;  // SMG2 offset inh
    /* 0x268 */ TVec3f _268;  // SMG2 offset inh
    /* 0x274 */ u8 _274;
    /* 0x275 */ u8 _pad275[0x3];
    /* 0x278 */ f32 mWalkSpeed;
    /* 0x27C */ f32 mPrevAnimFrame;  // SMG2 offset inh
    /* 0x280 */ f32 _280;
    /* 0x284 */ TVec3f _284;  // SMG2 offset inh
    /* 0x290 */ TVec3f _290;  // SMG2 offset inh
    /* 0x29C */ TVec3f _29C;  // SMG2 offset inh
    /* 0x2A8 */ TVec3f _2A8;  // SMG2 offset inh
    /* 0x2B4 */ f32 _2B4;  // SMG2 offset inh
    /* 0x2B8 */ TVec3f _2B8;  // SMG2 offset inh
    /* 0x2C4 */ TVec3f _2C4;  // SMG2 offset inh
    /* 0x2D0 */ f32 _2D0;
    /* 0x2D4 */ TVec3f mJumpVec;  // SMG2 offset inh
    /* 0x2E0 */ TVec3f _2E0;  // SMG2 offset inh
    /* 0x2EC */ TVec3f _2EC;  // SMG2 offset inh
    /* 0x2F8 */ TVec3f _2F8;  // SMG2 offset inh
    /* 0x304 */ TVec3f _304;  // SMG2 offset inh
    /* 0x310 */ TVec3f mSideVec;  // SMG2 offset inh
    /* 0x31C */ TVec3f mShadowPos;  // SMG2 offset inh
    /* 0x328 */ TVec3f _328;  // SMG2 offset inh
    /* 0x334 */ TVec3f _334;  // SMG2 offset inh
    /* 0x340 */ f32 _340;
    /* 0x344 */ TVec3f _344;  // left/right?  // SMG2 offset inh
    /* 0x350 */ TVec3f _350;  // SMG2 offset inh
    /* 0x35C */ TVec3f _35C;  // SMG2 offset inh
    /* 0x368 */ TVec3f _368;  // SMG2 offset inh
    /* 0x374 */ TVec3f _374;  // SMG2 offset inh
    /* 0x380 */ TVec3f _380;  // SMG2 offset inh
    /* 0x38C */ TVec3f _38C;  // SMG2 offset inh
    /* 0x398 */ TVec3f _398;  // SMG2 offset inh
    /* 0x3A4 */ TVec3f _3A4;  // SMG2 offset inh
    /* 0x3B0 */ TVec3f _3B0;  // SMG2 offset inh
    /* 0x3BC */ u16 _3BC;
    /* 0x3BE */ u16 _3BE;
    /* 0x3C0 */ u16 _3C0;  // SMG2 offset inh
    /* 0x3C2 */ u16 _3C2;
    /* 0x3C4 */ u16 _3C4;
    /* 0x3C6 */ u16 _3C6;
    /* 0x3C8 */ u16 _3C8;  // SMG2 offset inh
    /* 0x3CA */ u16 _3CA;
    /* 0x3CC */ u16 _3CC;
    /* 0x3CE */ u16 _3CE;  // a timer
    /* 0x3D0 */ u16 _3D0;
    /* 0x3D2 */ u16 _3D2;
    /* 0x3D4 */ u16 _3D4;
    /* 0x3D6 */ u16 _412;  // SMG2 offset guess-gap
    /* 0x3D8 */ TVec3f _3D8;  // SMG2 offset inh
    /* 0x3E4 */ TVec3f _3E4;  // SMG2 offset inh
    /* 0x3F0 */ f32 _3F0;
    /* 0x3F4 */ f32 _3F4;
    /* 0x3F8 */ u16 _3F8;
    /* 0x3FA */ u16 _3FA;  // SMG2 offset inh
    /* 0x3FC */ u16 _3FC;  // SMG2 offset inh
    /* 0x3FE */ u16 _3FE;  // SMG2 offset inh
    /* 0x400 */ u16 _400;  // SMG2 offset inh
    /* 0x402 */ u16 _402;
    /* 0x404 */ u16 _404;  // SMG2 offset guess
    /* 0x406 */ u16 _408;
    /* 0x408 */ u16 _40A;
    /* 0x40A */ u16 _40C;  // SMG2 offset inh
    /* 0x40C */ u16 _40E;
    /* 0x40E */ u16 _406;  // SMG2 offset guess-gap
    /* 0x410 */ u16 _414;  // a timer
    /* 0x412 */ u16 mMorphResetTimer;
    /* 0x414 */ u16 _418;
    /* 0x416 */ u16 _41A;  // SMG2 offset inh
    /* 0x418 */ u16 _41C;
    /* 0x41A */ u16 _41E;
    /* 0x41C */ u16 _420;
    /* 0x41E */ u16 _422;
    /* 0x420 */ u16 _424;
    /* 0x422 */ u16 _426;
    /* 0x424 */ u16 _428;
    /* 0x426 */ u16 _42A;
    /* 0x428 */ u16 _42C;
    /* 0x42A */ u16 _410;  // SMG2 offset guess-gap
    /* 0x42C */ u32 _430;
    /* 0x430 */ u16 _434;  // a timer
    /* 0x432 */ u16 _436;  // SMG2 offset inh
    /* 0x434 */ u16 _438;
    /* 0x436 */ u16 _43A;
    /* 0x438 */ TVec3f _43C;  // front?  // SMG2 offset inh
    /* 0x444 */ TVec3f _448;  // SMG2 offset inh
    /* 0x450 */ f32 _454;
    /* 0x454 */ TriangleFilterDelegator< Mario >* _458;  // SMG2 offset inh
    /* 0x458 */ Triangle* _45C;
    /* 0x45C */ Triangle* _460;
    /* 0x460 */ Triangle* mGroundPolygon;
    /* 0x464 */ Triangle* _468;  // SMG2 offset inh
    /* 0x468 */ Triangle* _46C;
    /* 0x46C */ Triangle* _470;  // SMG2 offset inh
    /* 0x470 */ Triangle* _474;
    /* 0x474 */ TVec3f _1B4;  // SMG2 offset guess-gap
    /* 0x480 */ Triangle* _484;  // SMG2 offset vote-gap
    /* 0x484 */ f32 mVerticalSpeed;
    /* 0x488 */ TVec3f mGroundPos;  // SMG2 offset inh
    /* 0x494 */ TVec3f _498;  // SMG2 offset inh
    /* 0x4A0 */ TVec3f _4A4;  // SMG2 offset inh
    /* 0x4AC */ TVec3f _4B0;  // SMG2 offset inh
    /* 0x4B8 */ TVec3f _4BC;  // SMG2 offset inh
    /* 0x4C4 */ Triangle* _4C8;
    /* 0x4C8 */ Triangle* mFrontWallTriangle;
    /* 0x4CC */ Triangle* mBackWallTriangle;
    /* 0x4D0 */ Triangle* mSideWallTriangle;
    /* 0x4D4 */ Triangle* _4D8;  // SMG2 offset inh
    /* 0x4D8 */ Triangle* _4DC;
    /* 0x4DC */ f32 _4E0;  // SMG2 offset inh
    /* 0x4E0 */ f32 _4E4;  // SMG2 offset inh
    /* 0x4E4 */ TVec3f _4E8;  // SMG2 offset inh
    /* 0x4F0 */ TVec3f _4F4;  // SMG2 offset inh
    /* 0x4FC */ TVec3f _500;  // SMG2 offset inh
    /* 0x508 */ TVec3f _50C;  // SMG2 offset inh
    /* 0x514 */ TVec3f _518;  // SMG2 offset inh
    /* 0x520 */ f32 _524;
    /* 0x524 */ f32 _528;
    /* 0x528 */ f32 _52C;  // SMG2 offset inh
    /* 0x52C */ f32 _530;
    /* 0x530 */ u16 _534;
    /* 0x532 */ u8 _pad532[0x2];
    /* 0x534 */ f32 _538;
    /* 0x538 */ f32 _53C;  // SMG2 offset inh
    /* 0x53C */ f32 mYAngleOffset;
    /* 0x540 */ u16 _544;
    /* 0x542 */ u8 _pad542[0x2];
    /* 0x544 */ f32 _548;  // SMG2 offset inh
    /* 0x548 */ TVec3f _54C;  // SMG2 offset inh
    /* 0x554 */ u32 _558;
    /* 0x558 */ Triangle* mTmpPolygon;  // SMG2 offset guess-gap
    /* 0x55C */ Triangle* _47C;  // SMG2 offset guess-gap
    /* 0x560 */ Triangle* _480;  // SMG2 offset guess-gap
    /* 0x564 */ u32 _55C;  // SMG2 offset guess-gap
    /* 0x568 */ u32 _560;  // SMG2 offset guess-gap
    /* 0x56C */ void* _574;
    /* 0x570 */ s32 _564;  // SMG2 offset guess-gap
    /* 0x574 */ CubeCameraArea* _568;  // SMG2 offset guess-gap
    /* 0x578 */ AreaObj* _56C;  // SMG2 offset guess-gap
    /* 0x57C */ u8 _570;  // SMG2 offset guess-gap
    /* 0x57D */ u8 _pad57D[0x3];
    /* 0x580 */ u32 _578;  // SMG2 offset guess-gap
    /* 0x584 */ TVec3f _A8C[9];  // SMG2 offset guess-gap
    /* 0x5F0 */ TVec3f _6A0;  // SMG2 offset guess-gap
    /* 0x5FC */ const HitSensor* _5FC;
    /* 0x600 */ TVec3f _600;  // SMG2 offset inh
    /* 0x60C */ u8 _60C;
    /* 0x60D */ bool _60D;  // SMG2 offset inh
    /* 0x60E */ u8 _60E;
    /* 0x60F */ u8 _60F;
    /* 0x610 */ u8 _610;
    /* 0x611 */ u8 _611;
    /* 0x612 */ u8 _pad612[0x2];
    /* 0x614 */ f32 _614;
    /* 0x618 */ f32 _618;  // SMG2 offset vote-gap
    /* 0x61C */ f32 _620;
    /* 0x620 */ u8 _pad620[0x4];
    /* 0x624 */ bool _624;
    /* 0x625 */ u8 _pad625[0x3];
    /* 0x628 */ f32 _61C;
    /* 0x62C */ u8 _638;
    /* 0x62D */ u8 _pad62D[0x3];
    /* 0x630 */ TVec3f _628;  // SMG2 offset guess-gap
    /* 0x63C */ f32 _634;  // SMG2 offset guess-gap
    /* 0x640 */ TVec3f mSide2D;  // SMG2 offset guess-gap
    /* 0x64C */ TVec3f mUp2D;  // SMG2 offset guess-gap
    /* 0x658 */ TVec3f mNormal2D;  // SMG2 offset guess-gap
    /* 0x664 */ TVec3f _660;  // SMG2 offset guess-gap
    /* 0x670 */ bool _66C;  // SMG2 offset guess-gap
    /* 0x671 */ u8 _pad671[0x3];
    /* 0x674 */ TVec3f _670;  // SMG2 offset guess-gap
    /* 0x680 */ TVec3f _67C;  // SMG2 offset guess-gap
    /* 0x68C */ TVec3f _688;  // SMG2 offset guess-gap
    /* 0x698 */ TVec3f _694;  // SMG2 offset guess-gap
    /* 0x6A4 */ u8 _6AC;
    /* 0x6A5 */ u8 _6AD;
    /* 0x6A6 */ u8 _pad6A6[0x2];
    /* 0x6A8 */ TVec3f _6B0;  // SMG2 offset inh
    /* 0x6B4 */ TVec3f _6BC;  // SMG2 offset inh
    /* 0x6C0 */ TVec2f _6C8;
    /* 0x6C8 */ TVec3f _6D0;  // SMG2 offset inh
    /* 0x6D4 */ TVec3f _6DC;  // SMG2 offset inh
    /* 0x6E0 */ TVec3f _6E8;  // SMG2 offset inh
    /* 0x6EC */ TVec3f _6F4;  // SMG2 offset inh
    /* 0x6F8 */ TVec3f _700;  // SMG2 offset inh
    /* 0x704 */ TVec3f _70C;  // SMG2 offset inh
    /* 0x710 */ f32 _718;
    /* 0x714 */ u8 mTargetWalkSpeedIndex;
    /* 0x715 */ u8 _71D;
    /* 0x716 */ u8 _71E;
    /* 0x717 */ u8 _71F;
    /* 0x718 */ const char* _720;
    /* 0x71C */ const char* _724;
    /* 0x720 */ const char* _728;
    /* 0x724 */ f32 _72C;
    /* 0x728 */ HitSensor* _730;  // SMG2 offset inh
    /* 0x72C */ u8 mIceAnimFoot;
    /* 0x72D */ u8 mSinkTimer;
    /* 0x72E */ u8 _pad72E[0x2];
    /* 0x730 */ f32 _738;
    /* 0x734 */ TVec3f _73C;  // SMG2 offset inh
    /* 0x740 */ u8 mPoisonTimer;
    /* 0x741 */ u8 _pad741[0x3];
    /* 0x744 */ f32 _74C;
    /* 0x748 */ u32 _750;
    /* 0x74C */ u32 _754;
    /* 0x750 */ MarioWall* mWall;
    /* 0x754 */ TVec3f _75C;  // SMG2 offset inh
    /* 0x760 */ MarioStick* mStick;
    /* 0x764 */ u16 _76C;
    /* 0x766 */ u8 _pad766[0x2];
    /* 0x768 */ f32 _770;  // SMG2 offset inh
    /* 0x76C */ u16 _774;
    /* 0x76E */ u8 _pad76E[0x2];
    /* 0x770 */ MarioRabbit* mRabbit;
    /* 0x774 */ MarioFoo* mFoo;
    /* 0x778 */ MarioSukekiyo* mSukekiyo;  // SMG2 offset inh
    /* 0x77C */ MarioBury* mBury;  // SMG2 offset inh
    /* 0x780 */ MarioWait* mWait;
    /* 0x784 */ MarioClimb* mClimb;
    /* 0x788 */ TVec3f _790;  // SMG2 offset inh
    /* 0x794 */ MarioHang* mHang;
    /* 0x798 */ MarioRecovery* mRecovery;
    /* 0x79C */ MarioWarp* mWarp;  // SMG2 offset inh
    /* 0x7A0 */ MarioFlip* mFlip;
    /* 0x7A4 */ MarioSideStep* mSideStep;  // SMG2 offset inh
    /* 0x7A8 */ MarioFrontStep* mFrontStep;
    /* 0x7AC */ MarioSkate* mSkate;
    /* 0x7B0 */ MarioTalk* mTalk;
    /* 0x7B4 */ u8 _pad7B4[0x10];
    /* 0x7C4 */ MarioDamage* mDamage;
    /* 0x7C8 */ TVec3f _7C4;  // SMG2 offset inh
    /* 0x7D4 */ u16 _7D0;  // SMG2 offset inh
    /* 0x7D6 */ u8 _pad7D6[0x2];
    /* 0x7D8 */ TVec3f _7D4;  // SMG2 offset inh
    /* 0x7E4 */ Triangle* _7E0;
    /* 0x7E8 */ TMtx34f _7E4;  // SMG2 offset inh
    /* 0x818 */ TVec3f _814;  // SMG2 offset inh
    /* 0x824 */ Triangle* _820;
    /* 0x828 */ TMtx34f _824;  // SMG2 offset inh
    /* 0x858 */ MarioFlow* mFlow;  // SMG2 offset inh
    /* 0x85C */ MarioFireDamage* mFireDamage;
    /* 0x860 */ MarioFireDance* mFireDance;
    /* 0x864 */ MarioFireRun* mFireRun;
    /* 0x868 */ MarioParalyze* mParalyze;
    /* 0x86C */ MarioStun* mStun;  // SMG2 offset inh
    /* 0x870 */ MarioCrush* mCrush;
    /* 0x874 */ MarioFreeze* mFreeze;
    /* 0x878 */ MarioAbyssDamage* mAbyssDamage;
    /* 0x87C */ MarioDarkDamage* mDarkDamage;  // SMG2 offset inh
    /* 0x880 */ MarioFaint* mFaint;
    /* 0x884 */ MarioBlown* mBlown;
    /* 0x888 */ MarioSwim* mSwim;
    /* 0x88C */ MarioSlider* mSlider;
    /* 0x890 */ MarioStep* mStep;  // SMG2 offset inh
    /* 0x894 */ MarioBump* mBump;  // SMG2 offset inh
    /* 0x898 */ MarioMagic* mMagic;
    /* 0x89C */ bool _898;  // SMG2 offset inh
    /* 0x89D */ u8 _pad89D[0x3];
    /* 0x8A0 */ MarioFpView* mFpView;
    /* 0x8A4 */ MarioMove* mMove;  // SMG2 offset inh
    /* 0x8A8 */ TVec3f _8A4;  // SMG2 offset inh
    union {  // SMG2 offset inh
        struct {  // SMG2 offset inh
            /* 0x8B4 */ TVec3f _8B0;  // SMG2 offset inh
            /* 0x8C0 */ TVec3f _8BC;  // SMG2 offset inh
        };  // SMG2 offset inh
        /* 0x8B4 */ TVec3f mRelativeWallPos[2];  // SMG2 offset inh
    };  // SMG2 offset inh
    /* 0x8CC */ Triangle* _8C8;
    /* 0x8D0 */ Triangle* _8CC[2];
    /* 0x8D8 */ HitSensor* _8D4;
    /* 0x8DC */ HitSensor* _8D8;
    /* 0x8E0 */ TVec3f _8DC;  // SMG2 offset inh
    /* 0x8EC */ Triangle* _8E8;
    /* 0x8F0 */ u8 _8EC;
    /* 0x8F1 */ u8 _pad8F1[0x3];
    /* 0x8F4 */ f32 _8F0;
    /* 0x8F8 */ f32 _8F4;
    /* 0x8FC */ TVec3f _8F8;  // SMG2 offset inh
    /* 0x908 */ TVec3f _904;  // SMG2 offset inh
    /* 0x914 */ TVec3f _910;  // SMG2 offset inh
    /* 0x920 */ TVec3f _91C;  // SMG2 offset inh
    /* 0x92C */ TVec3f _928;  // SMG2 offset inh
    /* 0x938 */ f32 _934;
    /* 0x93C */ TVec3f _938;  // SMG2 offset inh
    /* 0x948 */ TVec3f _944;  // SMG2 offset inh
    /* 0x954 */ TVec3f _950;  // SMG2 offset inh
    /* 0x960 */ FloorCode* _95C;
    /* 0x964 */ u16 _960;
    /* 0x966 */ u16 _962;
    /* 0x968 */ u16 _964[3];
    /* 0x96E */ u16 _96A;
    /* 0x970 */ HashSortTable* _96C;  // Sounds  // SMG2 offset inh
    /* 0x974 */ const char* _970;     // Sounds or somthing  // SMG2 offset inh
    /* 0x978 */ MarioModuleTask* _974;
    /* 0x97C */ u32 _978;  // SMG2 offset inh
    /* 0x980 */ MarioState* _97C;
    /* 0x984 */ MarioState* _980;
    /* 0x988 */ Task _984[0xb];  // SMG2 offset inh
    /* 0xA0C */ u32 _A08[11];  // SMG2 offset inh
    /* 0xA38 */ u16 _A34;
    /* 0xA3A */ u8 _padA3A[0x2];
    /* 0xA3C */ u32 _A38;
    /* 0xA40 */ u16 _A3C;
    /* 0xA42 */ u8 _padA42[0x2];
    /* 0xA44 */ TVec3f _A40;  // SMG2 offset inh
    /* 0xA50 */ TVec3f _A4C;  // SMG2 offset inh
    /* 0xA5C */ TVec3f _A58;  // SMG2 offset inh
    /* 0xA68 */ f32 _A64;
    /* 0xA6C */ HitSensor* _A68;  // SMG2 offset guess
    /* 0xA70 */ u8 _A6C[0x20];  // animations  // SMG2 offset guess
    /* 0xA90 */ u8 _padA90[0xc];
    /* 0xA9C */ Triangle* _57C[0x20];  // SMG2 offset guess-TAIL
};
