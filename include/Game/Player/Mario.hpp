#pragma once

#include "Game/Map/HitInfo.hpp"

#include "Game/Player/MarioModule.hpp"
#include "Game/Player/MarioPosHolder.hpp"
#include "Game/Util/TriangleFilter.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class AreaObj;
class CubeCameraArea;
class FloorCode;
class GravityInfo;
class HashSortTable;
class HitInfo;
class HitSensor;
class MarioMoveInfoA84;
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
class MarioYoshiPull;
class MarioYoshiBalloon;
class MarioYoshiShot;
class MarioYoshiDash;
class CollisionPartsFilterBase;
class TriangleFilterBase;
class HitInfo;
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

/// @brief One collected strike result of Mario::fn_803B4300 (size 0x70). Original name unknown.
struct MarioStrikeEntry {
    /* 0x00 */ Triangle mTriangle;
    /* 0x60 */ TVec3f mHitPos;
    /* 0x6C */ f32 mDepth;
};

/// @brief Three state flags embedded in Mario at +0xA90. Original name unknown.
struct AudSeLifeEntry;

struct MarioFlags3 {
    void fn_8038C890();  // clear all

    /* 0x0 */ bool _0;
    /* 0x1 */ bool _1;
    /* 0x2 */ bool _2;
};

class Mario : public MarioModule {
public:
    // SMG2-only functions (original names unknown; named by address)
    void fn_803880D0();
    void fn_80388150();
    void fn_803882A0();
    void fn_803885F0();
    void fn_803886B0();
    void fn_80388DB0();
    bool fn_803890B0();
    void fn_803896E0();
    void fn_803897A0();
    void fn_80389D90(f32 (*m)[4], const TVec3f* col2, const TVec3f* col1, const TVec3f* col0);
    void fn_8038A560(f32 (*arg1)[4], s32 arg2);
    void fn_8038A600();
    void fn_8038A840(void*, f32 x, f32 y, f32 z);
    s32 fn_8038A8A0(f32 (*arg1)[4], TVec3f* arg2);
    void fn_8038A9D0();
    bool fn_8038AA30();
    void fn_8038AB10();
    void fn_8038AB70();
    void fn_8038B9F0();
    void fn_8038C540();
    void fn_8038C620(s32 arg3);
    bool fn_8038CCF0();
    void fn_8038CE40();
    void fn_8038E1A0(const HitInfo* hit, s32 arg2);
    bool fn_80390210(const Triangle* triangle);
    bool fn_803902B0();
    bool fn_80391260(const Triangle*);
    void fn_803912D0(const HitInfo*, const TVec3f&, const TVec3f&);
    bool fn_80391330(const HitInfo*, const TVec3f&, const TVec3f&);
    void fn_803929A0();
    bool fn_80393C40();
    bool fn_80394550(const TVec3f* arg);
    bool fn_803949E0();
    bool fn_80395310();
    void fn_80395400(u32 arg);
    bool fn_803972D0(const TVec3f* arg1);
    bool fn_80399D30();
    void fn_80399E60(TVec3f* arg1);
    void fn_8039A020();
    void fn_8039A090();
    bool fn_8039A100();
    void fn_8039A1C0();
    void fn_8039A2A0();
    bool fn_8039A3D0();
    bool fn_8039A430();
    void fn_8039A460();
    void fn_8039A480();
    void fn_8039A5B0();
    void fn_8039A600();
    f32 calcJumpGravity();
    f32 fn_8039AA40();
    void fn_8039AB50();
    f32 getMaxDropSpeed();
    void fn_8039ACE0();
    f32 fn_8039ADC0();
    void fn_8039CFD0();
    void fn_803A02F0(s32 arg1, s32 arg2, s32 arg3);
    void fn_803A0490(s32 arg1, s32 arg2);
    void fn_803A0BE0();
    void fn_803A0D90(TVec3f* arg);
    bool fn_803A0E10();
    void fn_803A0ED0(TVec3f* arg1, s32 arg2);
    bool fn_803A0FF0();
    bool fn_803A1050();
    void fn_803A10D0(s32 arg);
    void fn_803A11A0(TVec3f* arg);
    void fn_803A1240(f32* a, f32* b);
    void fn_803A1330(u8* arg1);
    void fn_803A1600(u8* arg1);
    f32 fn_803A1800();
    bool fn_803A1920();
    bool fn_803A19C0();
    void fn_803A1A20(f32* out);
    void fn_803A1A60(TVec3f* arg);
    void fn_803A1AC0();
    void fn_803A1BB0();
    void fn_803A1C90();
    void fn_803A1D60();
    f32 fn_803A1DB0();
    void fn_803A1E40();
    s32 fn_803A1E60(f32 f);
    void fn_803A1F60();
    void fn_803A1F80();
    void fn_803A1FC0();
    void fn_803A1FF0(f32 x, s32 arg);
    void fn_803A20E0(f32 rate, s32 arg);
    void fn_803A2180();
    bool fn_803A2270();
    void fn_803A22D0(f32 x, s32 arg);
    void fn_803A23F0(TVec3f* out);
    void fn_803A2420();
    void fn_803A2450(const TVec3f* arg);
    void fn_803A24C0(TVec3f* arg);
    void fn_803A2530(TVec3f* out);
    void fn_803A2570(f32 x, TVec3f* arg);
    void fn_803A2690(f32* p);
    f32 fn_803A26C0(f32 x);
    void fn_803A2720();
    void fn_803A27A0(TVec3f* arg);
    void fn_803A2860();
    void fn_803A28D0(TVec3f* arg);
    void fn_803A2A70();
    void fn_803A2C40(const TVec3f* arg);
    void fn_803A2CC0();
    void fn_803A2CF0();
    s32 fn_803A2D10();
    void fn_803A2D50();
    s32 fn_803A2D80();
    void fn_803A2EF0();
    void fn_803A2FC0();
    s32 fn_803A3030();
    bool fn_803A3130();
    f32 fn_803A3780(f32 fself);
    bool fn_803A4210();
    void fn_803A42B0();
    void fn_803A56C0();
    void fn_803A6BE0();
    void fn_803A7540();
    bool fn_803A9690(const char* pName);
    void fn_803A96D0(f32);
    bool fn_803AA2C0();
    bool fn_803AA340(const TVec3f*, TVec3f*);
    bool fn_803AC810();
    void fn_803ACC40();
    void fn_803ACCE0();
    void fn_803ACD80();
    bool fn_803ACE10();
    void fn_803ACE60();
    void fn_803ACF70();
    void fn_803AD0B0();
    void fn_803AD120(const Triangle* pTri);
    void fn_803AD4F0();
    void fn_803AD5B0();
    void fn_803AD9C0();
    void fn_803AF2F0(const f32* pSpeedTable);
    void fn_803B0E60();
    void fn_803B0EF0();
    void fn_803B1100();
    void fn_803B1180();
    void fn_803B12B0();
    void fn_803B14E0();
    s32 fn_803B1540();
    void fn_803B1710();
    void fn_803B1720();
    void fn_803B1780();
    void fn_803B17C0();
    void fn_803B1800();
    bool fn_803B1850() const;
    void fn_803B1880();
    bool fn_803B18C0();
    void fn_803B1B20();
    bool fn_803B1B70() const;
    f32 getJumpHeight();
    bool fn_803B1C10();
    f32 fn_803B1CA0();
    f32 fn_803B1D00();
    bool fn_803B1D60();
    bool fn_803B1DE0(TVec3f* arg1);
    void fn_803B1EF0(TVec3f* arg1, f32 fself);
    void fn_803B23E0(const TVec3f*, const TVec3f*, f32);
    bool fn_803B2470();
    s32 fn_803B2530(bool squat);
    void fn_803B2610();
    bool fn_803B26C0();
    bool fn_803B2730();
    bool fn_803B2770(TVec3f*, Triangle*, const TVec3f&, const TVec3f&);
    bool fn_803B27F0(TVec3f*, Triangle*, const TVec3f&, const TVec3f&, const CollisionPartsFilterBase*, const TriangleFilterBase*);
    bool fn_803B2880(const TVec3f&, const TVec3f&);
    bool fn_803B28E0(const TVec3f&, const TVec3f&, const LiveActor*);
    s32 fn_803B2950();
    s32 fn_803B29B0(const TVec3f&, f32) NO_INLINE;
    s32 fn_803B2A20(const TVec3f&, f32, f32) NO_INLINE;
    const HitInfo* fn_803B2AA0(u32);
    bool fn_803B2AF0(const TVec3f&, HitInfo*);
    bool fn_803B2B50(const TVec3f&, const TVec3f&, TVec3f*, Triangle*);
    bool fn_803B2E10(const Triangle*);
    bool isStateHanging() const;
    bool isStateBury() const;
    s32 fn_803B2BD0();
    f32 fn_803B2CC0(TVec3f* arg1);
    s32 fn_803B2E80(TVec3f* arg1);
    void fn_803B2F20(const HitInfo*);
    void fn_803B3010();
    bool fn_803B3040(TVec3f* pOutPos, Triangle* pOutTri);
    void fn_803B3280(const TVec3f& rPos, Triangle* pTri);
    void fn_803B35D0(const TVec3f& rPos, u32 count);
    f32 fn_803B3870();
    void fn_803B38B0(const HitInfo*, const TVec3f*);
    void fn_803B3AE0(const HitInfo*, const TVec3f*);
    void fn_803B3BB0(const HitInfo*, const TVec3f*);
    void fn_803B3CE0();
    bool fn_803B3E90();
    void fn_803B3EE0(TVec3f* pPos, f32* pHeight);
    s32 fn_803B41F0();
    void fn_803B4280();
    s32 fn_803B4300(TVec3f* pOutVec, MarioStrikeEntry* pOut, s32, const TVec3f& rPos, f32 radius);
    void fn_803B44F0(TVec3f* arg1);
    void fn_803B45E0(const char* pName, const char* arg);
    void fn_803B4630(const char* pName);
    void fn_803B4670(const char* pName);
    bool fn_803B4850(const HitSensor* pSensor) const;
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
    void addTrans(const TVec3f&);
    void setTrans(const TVec3f&);
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
    void touchWater();

    bool isBlendWaitGround() const NO_INLINE;
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
    void updateCometCode();
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
    bool tryTurnJump();
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

    bool isEnableTurn();
    void recordTurnSlipAngle();
    f32 decideInertia(f32);
    f32 decideInertiaOnIce(f32);
    f32 decideInertiaOnSlip(f32);
    void calcShadowDir(const TVec3f&, TVec3f*);
    void retainMoveDir(f32, f32, TVec3f*);
    void calcMoveDir(f32, f32, TVec3f*, bool);
    bool checkLockOnHoming();
    void doLockOnHoming();
    void fixPositionInTower();

    void check2DMode();
    void calcMoveDir2D(f32, f32, TVec3f*);
    void calcShadowDir2D(const TVec3f&, TVec3f*);
    void stick2Dadjust(f32&, f32&);
    void set2Dmode(bool);
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
    void push(const TVec3f&) NO_INLINE;
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
    bool taskOnHipDropBlurCloud(u32);
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
        enum Mask {
            Jumping = 0x80000000U,
            Turning = 0x10000000U,
            DebugMode = 0x00000200U,
            DigitalJump = 0x00000002U
        };

        u32 mLowWord;
        u32 mHighWord;
    };

    struct DrawStates {
        enum Mask {
            Underwater = 0x00002000U
        };

        u32 mWord;
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

    /* 0x008 */ bool mDashWallSoundPlayed;
    /* 0x009 */ u8 _pad9[3];
    /* 0x00C */ MovementStates mMovementStates;
    /* 0x014 */ MovementStates mFlags14;
    /* 0x01C */ DrawStates mDrawStates;
    /* 0x020 */ DrawStates mFlags20;
    /* 0x024 */ MovementStates mFlags24;
    /* 0x02C */ u32 _28;
    /* 0x030 */ u32 _2C;
    /* 0x034 */ DrawStates mPrevDrawStates;
    /* 0x038 */ u32 _34;
    /* 0x03C */ f32 _38;
    /* 0x040 */ f32 _3C;  // SMG2 offset guess  // SMG2 offset inh
    /* 0x044 */ MarioTeresa* mTeresa;
    /* 0x048 */ TVec3f _40;  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x054 */ TVec3f _4C;  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x060 */ TVec3f _58;  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x06C */ TMtx34f _64;  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x09C */ TMtx34f _94;  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x0CC */ TMtx34f _C4;  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x0FC */ TMtx34f _F4;  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x12C */ f32 _124;  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x130 */ f32 _128;  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x134 */ u32 _12C;  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x138 */ MarioPosHolder* mPosHolder;  // SMG2: position moved into a holder (pos at +0xC)
    /* 0x13C */ TVec3f _13C;
    /* 0x148 */ TVec3f _148;
    /* 0x154 */ TVec3f _154;
    /* 0x160 */ TVec3f mVelocity;
    /* 0x16C */ TVec3f _16C;
    /* 0x178 */ TVec3f mVelocityAfter;
    /* 0x184 */ TVec3f _184;
    /* 0x190 */ TVec3f _190;
    /* 0x19C */ TVec3f _19C;
    /* 0x1A8 */ TVec3f _1A8;
    /* 0x1B4 */ TVec3f _1B4;
    /* 0x1C0 */ TVec3f mStickPos;
    /* 0x1CC */ TVec3f mWorldPadDir;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x1D8 */ TVec3f mAirGravityVec;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x1E4 */ TVec3f _1E4;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x1F0 */ TVec3f mHeadVec;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x1FC */ TVec3f _1FC;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x208 */ TVec3f mFrontVec;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x214 */ TVec3f _214;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x220 */ TVec3f _220;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x22C */ TVec3f _22C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x238 */ TVec3f _238;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x244 */ TVec3f _244;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x250 */ TVec3f _250;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x25C */ TVec3f _25C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x268 */ TVec3f _268;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x274 */ u8 _274;
    /* 0x275 */ u8 _pad275[0x3];
    /* 0x278 */ f32 mWalkSpeed;
    /* 0x27C */ f32 mPrevAnimFrame;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x280 */ f32 _280;
    /* 0x284 */ TVec3f _284;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x290 */ TVec3f _290;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x29C */ TVec3f _29C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x2A8 */ TVec3f _2A8;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x2B4 */ f32 _2B4;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x2B8 */ TVec3f _2B8;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x2C4 */ TVec3f _2C4;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x2D0 */ f32 _2D0;
    /* 0x2D4 */ TVec3f mJumpVec;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x2E0 */ TVec3f _2E0;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x2EC */ TVec3f _2EC;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x2F8 */ TVec3f _2F8;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x304 */ TVec3f _304;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x310 */ TVec3f mSideVec;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x31C */ TVec3f mShadowPos;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x328 */ TVec3f _328;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x334 */ TVec3f _334;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x340 */ f32 _340;
    /* 0x344 */ TVec3f _344;  // left/right?  // SMG2 offset inh  // SMG2 offset inh
    /* 0x350 */ TVec3f _350;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x35C */ TVec3f _35C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x368 */ TVec3f _368;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x374 */ TVec3f _374;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x380 */ TVec3f _380;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x38C */ TVec3f _38C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x398 */ TVec3f _398;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x3A4 */ TVec3f _3A4;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x3B0 */ TVec3f _3B0;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x3BC */ u16 _3BC;
    /* 0x3BE */ u16 _3BE;
    /* 0x3C0 */ u16 _3C0;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x3C2 */ u16 _3C2;
    /* 0x3C4 */ u16 _3C4;
    /* 0x3C6 */ u16 _3C6;
    /* 0x3C8 */ u16 _3C8;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x3CA */ u16 _3CA;
    /* 0x3CC */ u16 _3CC;
    /* 0x3CE */ u16 _3CE;  // a timer
    /* 0x3D0 */ u16 _3D0;
    /* 0x3D2 */ u16 _3D2;
    /* 0x3D4 */ u16 _3D4;
    /* 0x3D6 */ u16 _412;  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x3D8 */ TVec3f _3D8;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x3E4 */ TVec3f _3E4;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x3F0 */ f32 _3F0;
    /* 0x3F4 */ f32 _3F4;
    /* 0x3F8 */ u16 _3F8;
    /* 0x3FA */ u16 _3FA;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x3FC */ u16 _3FC;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x3FE */ u16 _3FE;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x400 */ u16 _400;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x402 */ u16 _402;
    /* 0x404 */ u16 _404;  // SMG2 offset guess  // SMG2 offset inh
    /* 0x406 */ u16 _408;
    /* 0x408 */ u16 _40A;
    /* 0x40A */ u16 _40C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x40C */ u16 _40E;
    /* 0x40E */ u16 _406;  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x410 */ u16 _414;  // a timer
    /* 0x412 */ u16 mMorphResetTimer;
    /* 0x414 */ u16 _418;
    /* 0x416 */ u16 _41A;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x418 */ u16 _41C;
    /* 0x41A */ u16 _41E;
    /* 0x41C */ u16 _420;
    /* 0x41E */ u16 _422;
    /* 0x420 */ u16 _424;
    /* 0x422 */ u16 _426;
    /* 0x424 */ u16 _428;
    /* 0x426 */ u16 _42A;
    /* 0x428 */ u16 _42C;
    /* 0x42A */ u16 _410;  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x42C */ u32 _430;
    /* 0x430 */ u16 _434;  // a timer
    /* 0x432 */ u16 _436;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x434 */ u16 _438;
    /* 0x436 */ u16 _43A;
    /* 0x438 */ TVec3f _43C;  // front?  // SMG2 offset inh  // SMG2 offset inh
    /* 0x444 */ TVec3f _448;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x450 */ f32 _454;
    /* 0x454 */ TriangleFilterDelegator< Mario >* _458;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x458 */ Triangle* _45C;
    /* 0x45C */ Triangle* _460;
    /* 0x460 */ Triangle* mGroundPolygon;
    /* 0x464 */ Triangle* _468;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x468 */ Triangle* _46C;
    /* 0x46C */ Triangle* _470;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x470 */ Triangle* _474;
    /* 0x474 */ Triangle* mTmpPolygon;
    /* 0x478 */ Triangle* _47C;
    /* 0x47C */ Triangle* _480;
    /* 0x480 */ Triangle* _484;  // SMG2 offset vote-gap  // SMG2 offset vote-gap
    /* 0x484 */ f32 mVerticalSpeed;
    /* 0x488 */ TVec3f mGroundPos;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x494 */ TVec3f _498;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x4A0 */ TVec3f _4A4;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x4AC */ TVec3f _4B0;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x4B8 */ TVec3f _4BC;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x4C4 */ Triangle* _4C8;
    /* 0x4C8 */ Triangle* mFrontWallTriangle;
    /* 0x4CC */ Triangle* mBackWallTriangle;
    /* 0x4D0 */ Triangle* mSideWallTriangle;
    /* 0x4D4 */ Triangle* _4D8;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x4D8 */ Triangle* _4DC;
    /* 0x4DC */ f32 _4E0;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x4E0 */ f32 _4E4;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x4E4 */ TVec3f _4E8;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x4F0 */ TVec3f _4F4;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x4FC */ TVec3f _500;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x508 */ TVec3f _50C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x514 */ TVec3f _518;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x520 */ f32 _524;
    /* 0x524 */ f32 _528;
    /* 0x528 */ f32 _52C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x52C */ f32 _530;
    /* 0x530 */ u16 _534;
    /* 0x532 */ u8 _pad532[0x2];
    /* 0x534 */ f32 _538;
    /* 0x538 */ f32 _53C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x53C */ f32 mYAngleOffset;
    /* 0x540 */ u16 _544;
    /* 0x542 */ u8 _pad542[0x2];
    /* 0x544 */ f32 _548;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x548 */ TVec3f _54C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x554 */ u32 _558;
    /* 0x558 */ u8 _pad558[8];
    /* 0x560 */ s32 _564;
    /* 0x564 */ AreaObj* _56C;
    /* 0x568 */ u8 _570;
    /* 0x569 */ u8 _pad569[3];
    /* 0x56C */ void* _574;
    /* 0x570 */ u32 _578;
    /* 0x574 */ Triangle* _57C[0x20];
    /* 0x5F4 */ u32 mPreviousCometRestrictions;
    /* 0x5F8 */ u32 mCometRestrictions;
    /* 0x5FC */ const HitSensor* _5FC;
    /* 0x600 */ TVec3f _600;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x60C */ u8 _60C;
    /* 0x60D */ bool _60D;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x60E */ u8 _60E;
    /* 0x60F */ u8 _60F;
    /* 0x610 */ u8 _610;
    /* 0x611 */ u8 _611;
    /* 0x612 */ u8 _pad612[0x2];
    /* 0x614 */ f32 _614;
    /* 0x618 */ f32 _618;  // SMG2 offset vote-gap  // SMG2 offset vote-gap
    /* 0x61C */ f32 mPrevious2DStickX;
    /* 0x620 */ f32 mPrevious2DStickY;
    /* 0x624 */ bool _624;
    /* 0x625 */ u8 _pad625[0x3];
    /* 0x628 */ f32 _61C;
    /* 0x62C */ u8 _638;
    /* 0x62D */ u8 _pad62D[0x3];
    /* 0x630 */ TVec3f mSide2D;
    /* 0x63C */ TVec3f mUp2D;
    /* 0x648 */ TVec3f mNormal2D;
    /* 0x654 */ TVec3f _660;
    /* 0x660 */ bool _66C;
    /* 0x661 */ u8 _pad661[0x3];
    /* 0x664 */ TVec3f _670;
    /* 0x670 */ TVec3f _67C;
    /* 0x67C */ TVec3f _688;
    /* 0x688 */ TVec3f _694;
    /* 0x694 */ TVec3f mSwimSideVec694;
    /* 0x6A0 */ u32 mInitValue6A0;
    /* 0x6A4 */ u8 mAxisMode6A4;
    /* 0x6A5 */ u8 mCandidateAxisMode6A5;
    /* 0x6A6 */ u8 _padAxis6A6[2];
    /* 0x6A8 */ TVec3f _6B0;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x6B4 */ TVec3f _6BC;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x6C0 */ TVec2f _6C8;
    /* 0x6C8 */ TVec3f _6D0;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x6D4 */ TVec3f _6DC;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x6E0 */ TVec3f _6E8;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x6EC */ TVec3f _6F4;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x6F8 */ TVec3f _700;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x704 */ TVec3f _70C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x710 */ f32 _718;
    /* 0x714 */ u8 mTargetWalkSpeedIndex;
    /* 0x715 */ u8 _71D;
    /* 0x716 */ u8 _71E;
    /* 0x717 */ u8 _71F;
    /* 0x718 */ const char* _720;
    /* 0x71C */ const char* _724;
    /* 0x720 */ const char* _728;
    /* 0x724 */ f32 _72C;
    /* 0x728 */ HitSensor* _730;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x72C */ u8 mIceAnimFoot;
    /* 0x72D */ u8 mSinkTimer;
    /* 0x72E */ u8 _pad72E[0x2];
    /* 0x730 */ f32 _738;
    /* 0x734 */ TVec3f _73C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x740 */ u8 mPoisonTimer;
    /* 0x741 */ u8 _pad741[0x3];
    /* 0x744 */ f32 _74C;
    /* 0x748 */ u32 _750;
    /* 0x74C */ u32 _754;
    /* 0x750 */ MarioWall* mWall;
    /* 0x754 */ TVec3f _75C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x760 */ MarioStick* mStick;
    /* 0x764 */ u16 _76C;
    /* 0x766 */ u8 _pad766[0x2];
    /* 0x768 */ f32 _770;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x76C */ u16 _774;
    /* 0x76E */ u8 _pad76E[0x2];
    /* 0x770 */ MarioRabbit* mRabbit;
    /* 0x774 */ MarioFoo* mFoo;
    /* 0x778 */ MarioSukekiyo* mSukekiyo;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x77C */ MarioBury* mBury;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x780 */ MarioWait* mWait;
    /* 0x784 */ MarioClimb* mClimb;
    /* 0x788 */ TVec3f _790;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x794 */ MarioHang* mHang;
    /* 0x798 */ MarioRecovery* mRecovery;
    /* 0x79C */ MarioWarp* mWarp;  // SMG2 offset inh
    /* 0x7A0 */ MarioFlip* mFlip;
    /* 0x7A4 */ MarioSideStep* mSideStep;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x7A8 */ MarioFrontStep* mFrontStep;
    /* 0x7AC */ MarioSkate* mSkate;
    /* 0x7B0 */ MarioTalk* mTalk;
    /* 0x7B4 */ MarioYoshiPull* mYoshiPull;
    /* 0x7B8 */ MarioYoshiBalloon* mYoshiBalloon;
    /* 0x7BC */ MarioYoshiShot* mYoshiShot;
    /* 0x7C0 */ MarioYoshiDash* mYoshiDash;
    /* 0x7C4 */ MarioDamage* mDamage;
    /* 0x7C8 */ TVec3f _7C4;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x7D4 */ u16 _7D0;  // SMG2 offset inh
    /* 0x7D6 */ u8 _pad7D6[0x2];
    /* 0x7D8 */ TVec3f _7D4;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x7E4 */ Triangle* _7E0;
    /* 0x7E8 */ TMtx34f _7E4;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x818 */ TVec3f _814;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x824 */ Triangle* _820;
    /* 0x828 */ TMtx34f _824;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x858 */ MarioFlow* mFlow;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x85C */ MarioFireDamage* mFireDamage;
    /* 0x860 */ MarioFireDance* mFireDance;
    /* 0x864 */ MarioFireRun* mFireRun;
    /* 0x868 */ MarioParalyze* mParalyze;
    /* 0x86C */ MarioStun* mStun;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x870 */ MarioCrush* mCrush;
    /* 0x874 */ MarioFreeze* mFreeze;
    /* 0x878 */ MarioAbyssDamage* mAbyssDamage;
    /* 0x87C */ MarioDarkDamage* mDarkDamage;  // SMG2 offset inh
    /* 0x880 */ MarioFaint* mFaint;
    /* 0x884 */ MarioBlown* mBlown;
    /* 0x888 */ MarioSwim* mSwim;
    /* 0x88C */ MarioSlider* mSlider;
    /* 0x890 */ MarioStep* mStep;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x894 */ MarioBump* mBump;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x898 */ MarioMagic* mMagic;
    /* 0x89C */ bool _898;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x89D */ u8 _pad89D[0x3];
    /* 0x8A0 */ MarioFpView* mFpView;
    /* 0x8A4 */ MarioMove* mMove;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x8A8 */ TVec3f _8A4;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x8B4 */ TVec3f mRelativeWallPos[2];
    /* 0x8CC */ Triangle* _8C8;
    /* 0x8D0 */ Triangle* _8CC[2];
    /* 0x8D8 */ HitSensor* _8D4;
    /* 0x8DC */ HitSensor* _8D8;
    /* 0x8E0 */ TVec3f _8DC;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x8EC */ Triangle* _8E8;
    /* 0x8F0 */ u8 _8EC;
    /* 0x8F1 */ u8 _pad8F1[0x3];
    /* 0x8F4 */ f32 _8F0;
    /* 0x8F8 */ f32 _8F4;
    /* 0x8FC */ TVec3f _8F8;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x908 */ TVec3f _904;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x914 */ TVec3f _910;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x920 */ TVec3f _91C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x92C */ TVec3f _928;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x938 */ f32 _934;
    /* 0x93C */ TVec3f _938;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x948 */ TVec3f _944;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x954 */ TVec3f _950;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x960 */ FloorCode* _95C;
    /* 0x964 */ u16 _960;
    /* 0x966 */ u16 _962;
    /* 0x968 */ u16 _964[3];
    /* 0x96E */ u16 _96A;
    /* 0x970 */ HashSortTable* _96C;  // Sounds  // SMG2 offset inh  // SMG2 offset inh
    /* 0x974 */ const char* _970;     // Sounds or somthing  // SMG2 offset inh  // SMG2 offset inh
    /* 0x978 */ MarioModuleTask* _974;
    /* 0x97C */ u32 _978;  // SMG2 offset inh  // SMG2 offset inh
    /* 0x980 */ MarioState* _97C;
    /* 0x984 */ MarioState* _980;
    /* 0x988 */ Task _984[0xb];  // SMG2 offset inh  // SMG2 offset inh
    /* 0xA0C */ u32 _A08[11];  // SMG2 offset inh  // SMG2 offset inh
    /* 0xA38 */ u16 _A34;
    /* 0xA3A */ u8 _padA3A[0x2];
    /* 0xA3C */ HitSensor* mTornadoCenterSensor;
    /* 0xA40 */ u16 _A3C;
    /* 0xA42 */ u8 _padA42[0x2];
    /* 0xA44 */ TVec3f _A40;  // SMG2 offset inh  // SMG2 offset inh
    /* 0xA50 */ TVec3f _A4C;  // SMG2 offset inh  // SMG2 offset inh
    /* 0xA5C */ TVec3f _A58;  // SMG2 offset inh  // SMG2 offset inh
    /* 0xA68 */ f32 _A64;
    /* 0xA6C */ HitSensor* _A68;  // SMG2 offset guess
    /* 0xA70 */ u8 _A70;
    /* 0xA71 */ u8 _A71;
    /* 0xA72 */ u8 _A72;
    /* 0xA73 */ u8 _padA73;
    /* 0xA74 */ u32 mDashWallCooldown;
    /* 0xA78 */ bool mCometNoSpinArea;
    /* 0xA79 */ bool mCometFireDanceArea;
    /* 0xA7A */ bool mCometRainbowArea;
    /* 0xA7B */ bool mCometHopperArea;
    /* 0xA7C */ bool mCometHopperApplied;
    /* 0xA7D */ bool mCometRainbowApplied;
    /* 0xA7E */ u8 _A7E;
    /* 0xA7F */ u8 _A7F;
    /* 0xA80 */ u8 _A80;
    /* 0xA81 */ u8 _padA81[3];
    /* 0xA84 */ MarioMoveInfoA84* mMoveInfoA84;
    /* 0xA88 */ u32 _A88;
    /* 0xA8C */ s32 mJumpFallTimer;
    /* 0xA90 */ MarioFlags3 mStateFlags;
    /* 0xA93 */ u8 _padA93;
    /* 0xA94 */ AudSeLifeEntry* mLifeAlertCounter;
    /* 0xA98 */ s32 _A98;
};

// SMG2-only free helpers of the Mario unit (set one axis column of a 3x4 matrix; original names unknown)
struct MarioRotationMatrix { f32 mMtx[3][3]; };
f32 fn_80386D30(f32, f32, f32);
void fn_803B21D0(const MarioRotationMatrix*, TQuat4f&);
void fn_803B2170(MarioRotationMatrix* pMtx, const TVec3f& rVec);
void fn_803B2190(MarioRotationMatrix* pMtx, const TVec3f& rVec);
void fn_803B21B0(MarioRotationMatrix* pMtx, const TVec3f& rVec);
