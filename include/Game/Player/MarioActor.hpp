#pragma once

#include <cstring>

#include "Game/LiveActor/Nerve.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MatrixControl.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/MapUtil.hpp"

class BlackHole;
class CollisionShadow;
class DLchanger;
class DrawAdaptor;
class FixedPosition;
class FootPrint;
class FurMulti;
class HashSortTable;
class IceStep;
class FireMarioBall;
class J3DAnmTexPattern;
class J3DModel;
class J3DModelData;
class J3DModelX;
class JAIAudible;
class JUTTexture;
class JetTurtleShadow;
class MarioAnimator;
class MarioActorConditionGroup;
class MarioFrontHistory;
class MarioScreenStrip;
class MarioScreenBlur;
class MarioSensorSlots;
class MarioConst;
class MarioEffect;
class CollectCounter;
class MarioActorEffectState;
class MarioMessenger;
class MarioNullBck;
class MarioSearchLight;
class MarioParts;
class ModelHolder;
class ModelObj;
class MultiEmitter;
class RushEndInfo;
class TornadoMario;
class Triangle;
class XanimePlayer;
class XanimeResourceTable;
class XjointTransform;
class BckCtrlData;
class Yoshi;
class YoshiHolder;
class YoshiTongueAction;
class MarioAllModel;
class MarioActionHash;
class MarioTeresaModel;
class MarioActionTable;
class MarioWallShadow;

struct MarioInputConfig {
    MarioInputConfig();
    s16 getButton0() const { return _0; }
    s16 getButton2() const { return _2; }
    s16 getSpinButton() const { return mSpinButton; }
    s16 getButton6() const { return _6; }
    s16 getButton8() const { return _8; }
    s16 _0;
    s16 _2;
    s16 mSpinButton;
    s16 _6;
    s16 _8;
};
class MarioHairAndHatModel;
class CloudMarioHat;
struct DLholder;
struct ResTIMG;
struct SmokeEffectEntry;
class MatrixControl;

enum SPECIAL_STATUS_FOR_CAMERA { CASE_0, CASE_1 };

enum PlayerMode {
    /* 0x0 */ PlayerMode_Normal,
    /* 0x1 */ PlayerMode_Invincible,
    /* 0x2 */ PlayerMode_2,
    /* 0x3 */ PlayerMode_Ice,
    /* 0x4 */ PlayerMode_Bee,
    /* 0x5 */ PlayerMode_Hopper,
    /* 0x6 */ PlayerMode_Teresa,
    /* 0x7 */ PlayerMode_Foo,
    /* 0x8 */ PlayerMode_8,
    /* 0x9 */ PlayerMode_Tornado
};

template < int SIZE, class T, class U >
class AudGenericAudible;
class JAUDopplerAudibleChannel;
class JAUDopplerAudibleAbsPos;
typedef AudGenericAudible< 4, JAUDopplerAudibleChannel, JAUDopplerAudibleAbsPos > AudGeneric;

extern bool gIsLuigi;  // (cc68 - 10000)(r13)

class MarioActor : public LiveActor {
public:
    void applyGravityVec(const TVec3f&, bool);
    bool fn_803CAEE0() const;
    bool fn_803CAF80() const;
    bool fn_803CB010() const;

    // SMG2-only functions (original names unknown; named by address)
    void exeGameOverNoDemo();
    const TVec3f& fn_803B8520();
    u32 fn_803B89E0();
    bool isFireDamageActive();
    void fn_803B9270();
    bool fn_803B93E0();
    void fn_803B94E0();
    void fn_803B9560();
    Yoshi* getYoshi();
    void* fn_803B9800();
    YoshiTongueAction* fn_803B9810();
    void* fn_803B9820();
    YoshiHolder* fn_803B9830();
    bool fn_803B9840();
    MtxPtr fn_803B9850();
    void fn_803B9870();
    void fn_803B98C0();
    void fn_803B98E0();
    s32 fn_803B9900();
    bool fn_803B9920();
    void fn_803B9A10();
    void fn_803B9A30();
    void fn_803B9A50();
    s32 fn_803B9A70();
    f32 fn_803B9A90();
    bool fn_803B9AB0() const;
    void fn_803B9F10();
    void fn_803BA070();
    void fn_803BA220();
    void fn_803BAFE0();
    void fn_803BB4F0();
    void fn_803BB510(const TVec3f& a, const TVec3f& b);
    bool fn_803BB5F0();
    void* fn_803BBA20();
    void fn_803BBAD0();
    void fn_803BBDD0(const TVec3f&, const TVec3f&, const TVec3f&);
    void fn_803BBFD0(const char* name);
    void fn_803BC440(const char* name);
    void fn_803BC5C0();
    void fn_803BDA20(MtxPtr dst, const char* joint);
    bool fn_803BDE10();
    bool fn_803BE0D0();
    void* fn_803BE0C0() const;
    void* fn_803BE0E0() const;
    void fn_803BE650(u32 a);
    void requestMovementOn();
    void fn_803BE6D0();
    void fn_803BE6F0();
    bool fn_803BE800() const;
    void fn_803BE840(const TVec3f&);
    void fn_803BE8D0(const TVec3f&);
    void fn_803BE960();
    void fn_803BE9E0();
    void fn_803BEA60(const char* a, const char* b);
    void fn_803BEA70(const char* a, const BckCtrlData* b);
    bool fn_803BEA80(const char*);
    bool fn_803BEAA0(const char*, const BckCtrlData&);
    void fn_803BEAC0();
    void requestHairAndHatMovementOn();
    void fn_803BEB10(s32 a);
    bool fn_803BEBE0();
    LiveActor* fn_803BECC0();
    bool fn_803BED10();
    void fn_803BF6F0();
    void fn_803BF760(const JMapInfoIter& iter, s32* anim);
    void fn_803BF860();
    void fn_803BF8A0();
    void fn_803BF8F0();
    void fn_803BF950();
    void fn_803BF9D0();
    void fn_803BFAE0();
    void fn_803BFB30();
    void fn_803BFBB0();
    void fn_803BFC10();
    void fn_803BFC90(s32 a);
    void fn_803BFCE0();
    void fn_803BFE50();
    void fn_803C1940();
    void fn_803C1AF0(TVec3f* a, TVec3f* b);
    bool fn_803C1C60();
    bool fn_803C1D30();
    bool fn_803C1E40();
    void fn_803C1E80();
    bool fn_803C1F30() const;
    void fn_803C2010();
    void fn_803C2030();
    void fn_803C2320();
    void fn_803C23B0();
    void fn_803C2520();
    void fn_803C2590();
    void fn_803C2610();
    void fn_803C26A0();
    bool fn_803C2710();
    void fn_803C2900();
    void fn_803C2AA0();
    void fn_803C2B50();
    bool fn_803C3260();
    bool fn_803C33D0();
    void fn_803C34F0();
    void fn_803C38D0();
    bool fn_803C3930() const;
    void fn_803C3C90();
    void fn_803C3D10();
    bool fn_803C3DF0();
    void fn_803C3E40();
    void fn_803C4020();
    void fn_803C4100();
    bool fn_803C4F70();
    bool fn_803C4FB0();
    void fn_803C5090(MtxPtr mtx);
    void fn_803C56F0();
    bool fn_803C5750();
    void fn_803C5820();
    void fn_803C58D0();
    void fn_803C5920() NO_INLINE;
    bool fn_803C5990();
    void fn_803C59C0();
    void fn_803C5AB0(MtxPtr mtx);
    void fn_803C5B30(MtxPtr mtx);
    void fn_803C5E20(MtxPtr mtx);
    void fn_803C5ED0(MtxPtr m1, MtxPtr m2);
    void fn_803C8AA0();
    bool fn_803C9300();
    void fn_803C9920(TVec3f& vec);
    void fn_803CA1F0();
    void fn_803CA270(TVec3f* out, s32 isStatic);
    bool fn_803CA440(TVec3f& a, TVec3f& b);
    void fn_803CA710(TVec3f& a, TVec3f& b);
    void fn_803CA860(TVec3f& v);
    void fn_803CAA00();
    void fn_803CAD10(const char* name, const char* b, const BckCtrlData* d);
    bool fn_803CB1A0(u16 mode);
    void fn_803CB2E0(u16 newMode);
    void fn_803CB510(u16 mode, bool a);
    void fn_803CBA90(bool a, bool b);
    void fn_803CBB30(bool b);
    void fn_803CBC50();
    void fn_803CCE60(HitSensor* a, s16 b);
    void fn_803CCF20(HitSensor* s);
    void fn_803CCF90(HitSensor* sensor);
    bool fn_803CD000();
    bool fn_803CD0A0();
    bool fn_803CD180();
    bool fn_803CD1F0(HitSensor* s);
    f32 fn_803CD250(HitSensor* s);
    bool fn_803CD2C0();
    bool fn_803CD380();
    bool fn_803CD3C0();
    void fn_803CD450(HitSensor* s);
    bool fn_803CD4B0(HitSensor* s, f32 f0, f32 f1);
    bool fn_803CD8A0(TVec3f* v, f32 f0, f32 f1, f32 f2);
    void fn_803CE8A0(HitSensor* s);
    void fn_803CE9D0(HitSensor* s);
    void fn_803CEA80(HitSensor* s, f32 t);
    void fn_803CEBC0();
    bool fn_803CEE50();
    s32 fn_803CEEE0(HitSensor* s);
    void fn_803CEF50(LiveActor*);
    bool fn_803CFCA0(HitSensor* s);
    HitSensor* fn_803D0CE0(s32 a);
    HitSensor* fn_803D0E40(s32 a);
    bool fn_803D0F60(HitSensor* s);
    bool fn_803D11C0();
    f32 fn_803D1470(HitSensor* s);
    void fn_803D1740(HitSensor* s);
    void fn_803D1830(HitSensor* s);
    f32 fn_803D18D0();
    void fn_803D1940();
    void fn_803D26F0();
    void fn_803D32F0();
    void fn_803D5A00(u32 idx);
    void fn_803D82C0(TVec2f* a, TVec2f* b) const;
    void fn_803D8400(TVec2f* a, TVec2f* b) const;
    s32 fn_803D8490() const;
    void fn_803D86B0(TVec2f* a, TVec2f* b) const;
    void fn_803D8A30(s32 a, TVec3f& v);
    void fn_803D8A70(s32 mode);
    bool fn_803D8B30(HitSensor* s) const;
    void fn_803D8BD0(TVec3f* a, TVec3f* b);
    void fn_803D8E30(TVec3f* v);
    void fn_803D9170(TVec3f* v);
    bool fn_803D9370(void* tri, TVec3f* v);
    bool isSilhouetteVisible() const;
    void fn_803D95E0();
    virtual void startMovement();
    virtual void endMovement();
    void fn_803D9710();
    void fn_803D9730();
    void fn_803D9740();
    MarioActor(const char*);

    virtual ~MarioActor();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void movement();
    virtual void draw() const;
    virtual void calcAnim();
    virtual void calcViewAndEntry();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void updateHitSensor(HitSensor*);
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    bool receiveMsgTaken(HitSensor*, HitSensor*);  // SMG2: no longer a LiveActor virtual
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual const TVec3f& getLastMove() const;
    virtual void getLastMove(TVec3f*) const;
    virtual void getFrontVec(TVec3f*) const;

    void initAfterOpeningDemo();
    void calcBaseFrontVec(const TVec3f&);
    void playSound(const char*, s32);
    void getFrontHistoryVec(TVec3f*);
    void changeAnimationNonStop(const char*);
    void changeAnimationUpper(const char*);
    void stopAnimation(const char*);
    bool isAnimationRun(const char*) const;
    void changeNullAnimation(const char*, s8);
    void clearNullAnimation(s8);
    bool isStopNullAnimation() const;
    void changeGameOverAnimation();
    void updateRotationInfo();
    void exeWait();
    // void exeGameOverBlackHole();
    void control2();
    void controlMain();
    void updateBehavior();
    void updateBindRatio();
    bool isInPunchTimerRange() const;
    void updatePunching();
    bool doRush();
    void updateSwingTimer();
    void updateSwingAction();
    bool checkClapCatchStart();
    void updateRealMtx();
    void setUpperRotateY(f32);
    void decLife(u16);
    void decLifeLarge();
    void decLifeMiddle();
    void decLifeSmall();
    void incLife(u32);
    void changeMaxLife(s32);
    bool doPressing();
    void careMovingObject();
    bool doStun();
    void scaleMtx(MtxPtr);
    void updateBaseScaleMtx();
    void getRealMtx(MtxPtr, const char*) const NO_INLINE;
    void getRealPos(const char*, TVec3f*) const;
    MtxPtr getGlobalJointMtx(const char*);
    void calcAnimInMovement();
    void forceSetBaseMtx(MtxPtr);
    void setBlendMtxTimer(u16);
    void getGroundPos(TVec3f* dst) const;
    TVec3f* getShadowPos() const;

    bool isTurning() const;
    bool isJumping() const;
    bool isJumpRising() const;
    bool isPunching() const;
    bool isItemSwinging() const;
    bool isDamaging() const;
    bool isNeedDamageFog() const;
    bool isStaggering() const;
    bool isSleeping() const;
    bool isRefuseTalk() const;
    bool isDebugMode() const;

    void jumpHop();
    void calcCenterPos();
    void calcHeadPos();
    void setPress(u8, s32);
    XanimeResourceTable* getResourceTable() const;

    bool isEnableMoveMario() const;
    bool isEnableNerveChange() const;

    void forceGameOver();
    void forceGameOverAbyss();
    void forceGameOverBlackHole();
    void forceGameOverNonStop();
    void forceGameOverSink();

    void updateCameraInfo();
    bool binderFilter(const Triangle*);

    void setPunchHitTimer(u8);
    void initEffect();
    void initMaterialEffect();
    void initCommonEffect();
    void addSoundObjHolder();

    void initMember();

    void initDrawAndModel();
    void initBeeMario();
    void initTeresaMario();
    void initHopperMario();
    void initIceMario();
    void initInvincibleMario();
    void initTornadoMario();
    void initBoneMario();
    void initBlur();
    void changeDisplayMode();
    bool isAllHidden() const;
    void initFace();
    void updateFace();
    void drawIndirect() const;
    void drawIndirectModel() const;
    void drawReflectModel() const;
    void drawModelBlur() const;
    void drawSearchLight() const;
    void drawPreWipe() const;
    void initSearchLight();
    void initThrowing();
    void drawMarioModel() const;

    // Called by drawMarioModel
    void calcViewSearchLight();

    void changeTeresaAnimation(const char*, s32);

    MultiEmitter* playEffect(const char*);
    void playEffectTrans(const char*, const TVec3f&);
    MultiEmitter* playMaterialEffect(const char*);
    MultiEmitter* playCommonEffect(const char*);
    void stopEffect(const char*);
    void stopMaterialEffect(const char*);
    void stopCommonEffect(const char*);
    bool isCommonEffect(const char*) const NO_INLINE;
    bool isMaterialEffect(const char*) const NO_INLINE;
    s32 getFloorMaterialIndex(u32) const;

    void updateSearchLight();
    void updateThrowing();
    void updateEffect();
    bool checkEffectWaterColumn();

    void bodyClap();
    bool selectWaterInOut(const char*) const;
    void playEffectRT(const char*, const TVec3f&, const TVec3f&);
    void playEffectRTZ(const char*, const TVec3f&, const TVec3f&);
    void playEffectRTW(const char*, const TVec3f&, const TVec3f&);
    void playEffectSRT(const char*, f32, const TVec3f&, const TVec3f&);
    void emitEffectWaterColumn(const TVec3f&, const TVec3f&);
    void endRush(const RushEndInfo*);
    void stopSpinTicoEffect(bool);
    void stopEffectForce(const char*);
    void initBlackHoleOut();  // void ?
    void exeGameOverBlackHole2();
    void initBlink();
    void updateBlink();

    void entryWallWalkMode(const TVec3f&, const TVec3f&);

    HitSensor* getLookTargetSensor() const;
    void copyMaterial(J3DModel*, u16, s32);

    void settingRush();

    void resetCondition();
    bool isFixJumpRushSensor(const HitSensor*) const;
    bool isLandEffectRushSensor(const HitSensor*) const;
    void beginRush(HitSensor*);


    // Defined in MarioActorPunch
    bool isEnableSpinPunch();
    void reactionPunch(HitSensor*);
    bool sendPunch(HitSensor*, const Triangle*, bool);
    bool doFreezeAttack(HitSensor*) NO_INLINE;
    bool trySpinPunch();
    void printHitMark(HitSensor*);
    bool tryPunchAirAfter(HitSensor*);
    bool sendBodyAttack(HitSensor*);
    void doFreezeAttack();

    // Defined in MarioActorCamera
    MtxPtr getMapBaseMtx() const;
    bool isFastRise() const;
    bool isLongDrop() const;
    bool isFastDrop() const;
    u32 getSpecialMode() const;
    bool isCameraStateOn(SPECIAL_STATUS_FOR_CAMERA) const;
    void getSideVec(TVec3f*) const;
    void getUpVec(TVec3f*) const;
    const TVec3f& getTransForCamera() const;
    void updateTransForCamera();
    void updateForCamera();

    // Defined in MarioActorGameOver
    void forceKill(u32);
    void exeGameOver();
    void exeGameOverAbyss();
    void exeGameOverFire();
    void exeGameOverSink();
    void exeGameOverNonStop();
    void exeTimeWait();

    // Defined in MarioActorGravity
    const TVec3f& getGravityVec() const;
    const TVec3f& getGravityVector() const;
    const TVec3f& getAirGravityVec() const;
    void getGravityVector(TVec3f*) const;
    f32 getGravityRatio() const;
    GravityInfo* getGravityInfo() const;
    u8 getGravityLevel() const;
    bool checkBeeWallStick(TVec3f&);
    bool checkBeeFloorStick(TVec3f&);
    void syncJumpBeeStickMode();
    void updateBeeModeGravity(TVec3f&);
    void updateGravityVec(bool, bool);
    bool checkBeeCeilStick(TVec3f&);
    void updateBeeStickMode(TVec3f&);

    // Defined in MarioActorHand
    void initHand();
    void updateHand();
    void changeHandMaterial();
    void updateHandAtMorph();
    void drawHand() const;
    void drawIndirectHand() const;

    // Defined in MarioActorPad
    void resetPadSwing();
    bool isRequestRush() const;
    bool isRequestThrow() const;
    bool isRequestSpin() const;
    bool isRequestJump() const;
    bool isRequestJump2P() const;
    bool isKeepJump() const;
    bool isKeepJump2P() const;
    bool checkButtonType(u16, bool) const;
    void getStickValue(f32*, f32*);
    void updateActionTrigger();
    void updateControllerSwing();
    void updateControllerSwingLeft();
    void lockOnDPD();
    bool isRequestSpinJump2P() const;

    // Defined in MarioActorParts
    void init2D();
    void initParts();
    void updateBeeWingAnimation();
    void updateTornado();
    void updateTakingPosition();
    const HitSensor* getCarrySensor() const;
    void changeSpecialModeAnimation(const char*);
    void updateSpecialModeAnimation();
    void initFireBall();
    void initFireMario();
    void initYoshi();
    void initCloudMario();
    void initRockMario();
    void updateFireSpin();
    void killAllFireBall();
    void calcAnimPlayerMode();
    void calcAnimHorrorBone();
    bool fn_803CB0B0() const;                             // SMG2: Yoshi tongue-action check
    bool fn_803DB380(const char*, const char*) const;     // SMG2-only (by address), same table family as isActionOk
    void shootFireBall();
    void showFreezeModel();
    void hideFreezeModel();
    void updateFairyStar();
    void update2D();
    void updateThrowVector();
    void createIceFloor(const TVec3f&);
    void createIceFloor(const TVec3f&, const TVec3f&);
    void createIceWall(const TVec3f&, const TVec3f&);
    void updateBaseMtxTeresa(MtxPtr);
    bool finalizeFreezeModel();
    void offTakingFlag();

    // Defined in MarioActorSensor
    void setupSensors();
    void doTrampleJump(HitSensor*);
    void trampleJump(f32, f32);
    bool sendMsgToSensor(HitSensor*, u32);
    void resetSensorCount();
    void recordScoutingObject(HitSensor*);
    void updateScouter();
    void initScouter();
    void initForJump();

    // Defined in MarioActorShadow
    void initShadow();
    void calcViewReflectionModel();
    void drawShadow() const;
    void calcViewSilhouetteModel();
    f32 calcCylinderToCenter(const AreaObj*, TVec3f*);
    void calcViewBlurModel();
    void calcViewFootPrint();
    void drawSilhouette() const;

    // Defined in MarioActorSpecialDraw
    void initScreenBox();
    bool isUseScreenBox() const;
    void calcScreenBoxRange();
    void captureScreenBox() const;
    void writeBackScreenBox() const;
    void calc1stPersonView();
    void hideBeeFur();
    void calcFogLighting();
    void resetFog();
    void updateAlphaDL(u8);
    void updateSimpleAlphaDL(u8);
    void updateReflectAlphaDL(u8);
    void updateLightDL(const Color8&, const Color8&, const Color8&, f32);
    void createRainbowDL();
    void drawScreenBlend() const;
    void updateRandomTexture(f32);
    void setRasterScroll(s32, s32, s32);
    void drawMosaic() const;
    void drawLifeUp() const;
    void calcSpinEffect();
    void drawSpinEffect() const;
    void drawSphereMask() const;
    void initDarkMask();
    void updateDarkMask(u16);
    void showBeeFur();
    bool drawDarkMask() const;

    // Defined in MarioActorMatrix
    void initActionMatrix();
    bool isActionOk(const char*) const;
    u8 selectAction(const char*) const;
    bool selectAutoBind(const char*, u8*) const;
    bool selectCustomEffectSpinHitSound(const char*) const;
    bool selectSpinCatchInRush(const char*) const;
    void decreaseLifeByOne();
    bool selectNotHomingSensor(const HitSensor*) const;
    bool selectInvalidMovingCollision(const char*) const;
    bool selectQuickResetBeeWallGravity(const char*) const;
    bool selectJumpRushSensor(const char*) const;
    bool selectDamageFireColor(const char*) const;
    bool selectWaterInOutEffect(const char*) const;
    bool selectOnDamageRush(const char*) const;
    bool selectTiltPress(const HitSensor*) const;
    bool selectHandyRush(const HitSensor*) const;
    bool selectRecoverFlyMeter(const HitSensor*) const;
    bool selectHideFlyMeter(const HitSensor*) const;
    bool selectTeresaThru(const HitSensor*) const;
    bool selectRebindTimer(const HitSensor*) const;
    bool selectHipDropRush(const HitSensor*) const;
    bool selectPushOff(const HitSensor*) const;
    bool selectWaterInOutRush(const HitSensor*) const;
    bool selectLandEffect(const HitSensor*) const;
    bool selectNoShadow(const HitSensor*) const;
    bool selectDamagePop(const HitSensor*) const;
    f32 getFaceLookHeight(const char*) const;

    // Defined in MarioActorMorph
    void setPlayerMode(u16, bool, bool);
    void applyAutoBindPlayerMode(u32);
    void updatePlayerMode();
    void touchSensor(HitSensor*);
    void initMorphStringTable();
    const char* changeMorphString(const char*) const;

    // Defined in MarioActorOffensiveMsg
    void attackOrPushSensor(HitSensor*, f32);
    void attackOrPushSensorInDamage(HitSensor*, f32);
    void attackOrPushSensorInRush(HitSensor*, f32);
    void tryAddClapCoin(HitSensor*);
    bool tryTornadoAttack(HitSensor*);
    bool isUnderTarget(HitSensor*);
    bool tryHipDropAttack(HitSensor*);
    bool checkAndTryTrampleAttack(HitSensor*, f32, bool);
    bool tryTrampleAttack(HitSensor*);
    bool cylinderHorizontalCheck(HitSensor*);
    bool tryJetAttack(HitSensor*);
    bool cylinderPushCheck(HitSensor* pSensor, f32 radius, f32 width, f32 height);
    bool cylinderPushCheck(const TVec3f&, f32, f32, f32);
    void attackOrPushPolygons();
    void sendWallTouch(u32, HitSensor*, HitSensor*);
    bool sendMsgUpperPunch(HitSensor*);

    // Defined in MarioActorDefensiveMsg
    bool tryAttackMsg(u32, const HitSensor*, bool*);
    bool tryVectorAttackMsg(u32, const TVec3f&);
    bool cylinderHitCheck(const HitSensor*, TVec3f*);

    // Defined in MarioActorRushMsg
    void addRushSensor(HitSensor*, bool);
    bool tryStandardRush();
    void checkPriorRushTarget();
    HitSensor* getNearestJumpTarget() const;
    HitSensor* getHighPriorityTarget() const;
    HitSensor* getRescueTarget() const;
    bool tryStartRush(HitSensor*, bool);
    bool tryJumpRush();
    void tryRushInRush();

    // Defined in MarioActorTakeMsg
    void memorizeSensorThrow(HitSensor*);
    bool tryThrow();
    void tryReleaseDirect(const HitSensor*);
    void damageDropThrowMemoSensor();
    void rushDropThrowMemoSensor();
    void trySetLockOnTarget(HitSensor*);
    void tryCoinPull();
    void tryCoinPullInRush();
    void flushCoinPull();
    bool tryCoinPullOne(HitSensor*) NO_INLINE;
    void tryPullTrans(TVec3f*, const TVec3f&);
    bool releaseThrowMemoSensor();
    void tryReleaseWithMsg(u32);
    void tryTornadoPull(HitSensor*);
    void tryReleaseBombTeresa();

    void runTeresaBaseAnimation();
    void updateTeresaAnimation();
    void getThrowVec(TVec3f*) const;

    const MarioConst& getConst() const {
        return *mConst;
    }

    inline u32 getHealth() const {
        return mHealth;
    }

    inline void setB90(bool param1) {
        _B90 = param1;
    }

    inline const Mario::MovementStates& getMovementStates() const {
        return mMario->mMovementStates;
    }

    inline const Mario::DrawStates& getDrawStates() const {
        return mMario->mDrawStates;
    }

    inline const Mario::DrawStates& getPrevDrawStates() const {
        return mMario->mPrevDrawStates;
    }

    // Only used in isSleeping() to make it match
    inline bool IsMarioAnimationRun(const char* pAnimName) const {
        return mMario->isAnimationRun(pAnimName);
    }

    inline bool IsMarioSwimming() const {
        return mMario->isSwimming();
    }

    inline void MarioStopWalk() const {
        mMario->stopWalk();
    }

    inline const bool isBeeMarioOnFur() const {
        return ((mMario->getMovementStates().mLowWord >> 30) & 1U) && strcmp("Fur", MR::getWallCodeString(mMario->getGroundPolygon())) == 0 && mBeeWallWalk == 0;
    }

    inline const bool is481or482On() const {
        return _482 || _481;
    }

    inline Mario* getMario() const {
        return mMario;
    }

    inline bool isBeeWallWalk() const {
        return mBeeWallWalk != 0;
    }

    struct FBO {
        u32 _0;
        u32 _4;
    };

    /* 0x090 */ bool mInitializing90;
    /* 0x091 */ u8 _pad91;
    /* 0x092 */ u8 _8C;  // SMG2 offset guess  // SMG2 offset guess  // SMG2 offset guess
    /* 0x093 */ u8 _pad93[0x1];
    /* 0x094 */ DLchanger* mDLchanger;  // SMG2 offset guess-gap
    /* 0x098 */ DLchanger* mBlurDisplayLists[64];
    /* 0x198 */ u8* mDL[2];
    /* 0x1A0 */ u32 mCurrentDLSize1A0;
    /* 0x1A4 */ u8 mCurrDL;
    /* 0x1A5 */ u8 mFirstPersonDrawHidden1A5;
    /* 0x1A6 */ u8 _pad1A6;
    /* 0x1A7 */ u8 _1A1;  // SMG2 offset guess  // SMG2 offset guess  // SMG2 offset inh
    /* 0x1A8 */ f32 _1A4;  // SMG2 offset guess-gap
    /* 0x1AC */ u32 mMaxHealth;  // SMG2 offset vote-gap
    /* 0x1B0 */ u8 _B68;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x1B1 */ u8 _A6E;  // SMG2 offset guess-gap
    /* 0x1B2 */ u16 _F42;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x1B4 */ Mtx* mBlurMatrices[2][8];
    /* 0x1F4 */ TMtx34f mPreviousViewMatrix;
    /* 0x224 */ TMtx34f mCurrentViewMatrix;
    /* 0x254 */ u16 mBlurBufferIndex;
    /* 0x256 */ u16 mBlurHistoryIndex;
    /* 0x258 */ u16 mBlurJointCount;
    /* 0x25A */ u8 _pad25A[2];
    /* 0x25C */ union {
        u32 mEffectFlagsWord;
        struct {
#ifdef GALAXY_NATIVE
            unsigned : 25;
            unsigned mSpecialSmoke : 1;
            unsigned mBeeWind : 1;
            unsigned mIsStationary : 1;
            unsigned mSlopeSlip : 1;
            unsigned _2 : 1;
            unsigned mSnow : 1;
            unsigned mSmoke : 1;
#else
            unsigned mSmoke : 1;
            unsigned mSnow : 1;
            unsigned _2 : 1;
            unsigned mSlopeSlip : 1;
            unsigned mIsStationary : 1;
            unsigned mBeeWind : 1;
            unsigned mSpecialSmoke : 1;
#endif
        } mEffectFlags;
    };
    /* 0x260 */ u16 _BC4;
    /* 0x262 */ u8 _pad262[0x2];
    /* 0x264 */ TMtx34f mReflectionBaseMtx264;
    /* 0x294 */ TMtx34f mJointTransform294;
    /* 0x2C4 */ Mtx* mJointMatrices2C4;
    /* 0x2C8 */ TMtx34f mHandLMatrix;
    /* 0x2F8 */ TMtx34f mHandRMatrix;
    /* 0x328 */ TMtx34f mHandLJointMatrix;
    /* 0x358 */ TMtx34f mHandRJointMatrix;
    /* 0x388 */ TMtx34f mFaceJointMatrix;
    /* 0x3B8 */ TMtx34f mFootRMatrix;
    /* 0x3E8 */ TMtx34f mSpine1Matrix;
    /* 0x418 */ TMtx34f mCapMatrix;
    /* 0x448 */ TMtx34f mHipMatrix;
    /* 0x478 */ TMtx34f mSpine2Matrix;
    /* 0x4A8 */ TMtx34f mMainJointMatrix4A8;
    /* 0x4D8 */ u16 mBlendMtxTimer;
    /* 0x4DA */ u8 _pad4DA[0x2];
    /* 0x4DC */ f32 _EA0;
    /* 0x4E0 */ bool _EA4;
    /* 0x4E1 */ bool _EA5;
    /* 0x4E2 */ bool _EA6;
    /* 0x4E3 */ u8 _pad4E3;
    /* 0x4E4 */ TMtx34f mRemoteDemoMtx4E4;
    /* 0x514 */ bool mSuperKinokoCollected;
    /* 0x515 */ bool mPowerupCollected;
    /* 0x516 */ bool mTransforming;
    /* 0x517 */ u8 _pad517[0x1];
    /* 0x518 */ HitSensor* _F24;
    /* 0x51C */ u16 _F28;
    /* 0x51E */ bool _F44;
    /* 0x51F */ u8 _pad51F[0x1];
    /* 0x520 */ HitSensor* _F48;  // SMG2 offset guess-gap
    /* 0x524 */ BlackHole* mBlackHole;
    /* 0x528 */ TVec3f mBlackHolePosition528;
    /* 0x534 */ TVec3f mBlackHoleRotateAxis;
    /* 0x540 */ TVec3f mPosRelativeToBlackHole;
    /* 0x54C */ const Nerve* _FB4;  // SMG2 offset guess-gap
    /* 0x550 */ u16 _FB8;  // a timer  // SMG2 offset guess-gap
    /* 0x552 */ u16 _1A8;  // SMG2 offset guess  // SMG2 offset guess  // SMG2 offset inh
    /* 0x554 */ u16 _1AA;
    /* 0x556 */ bool _F74;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0x557 */ u8 _pad557[0x1];
    /* 0x558 */ f32 _1AC;
    /* 0x55C */ Color8 mSwimFogColor;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x560 */ u8 _1B4;
    /* 0x561 */ bool mSwimFogDisabled;
    /* 0x562 */ u16 _1E8;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x564 */ CollectCounter* _1B8;  // SMG2 offset inh  // SMG2 offset guess  // SMG2 offset inh
    /* 0x568 */ MarioMessenger* mMessenger568;
    /* 0x56C */ bool _1C0;
    /* 0x56D */ u8 _A0C;  // SMG2 offset guess-gap  // SMG2 offset vote-gap  // SMG2 offset vote-gap
    /* 0x56E */ bool _1C3;
    /* 0x56F */ bool _1C2;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0x570 */ u16 _1C4;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0x572 */ bool _1E1;
    /* 0x573 */ bool _1E0;
    /* 0x574 */ MarioWallShadow* mWallShadow574;
    /* 0x578 */ AreaObj* mExtraWallArea578;
    /* 0x57C */ u8 mExtraWallAreaKind57C;
    /* 0x57D */ u8 _pad57D;
    /* 0x57E */ bool _3E4;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x57F */ u8 _1D1;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x580 */ CollisionShadow* mCollisionShadow580;
    /* 0x584 */ Mario* mMario;
    /* 0x588 */ MarioAnimator* mMarioAnim;
    /* 0x58C */ MarioEffect* mMarioEffect;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x590 */ MarioConst* mConst;
    /* 0x594 */ TVec3f mAppliedGravityVec;
    /* 0x5A0 */ TVec3f mPreviousGravityVec;
    /* 0x5AC */ TVec3f _258;  // SMG2 offset guess-gap
    /* 0x5B8 */ TVec3f mLastMove;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x5C4 */ TVec3f _1FC;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x5D0 */ TVec3f mBinderMotion5D0;
    /* 0x5DC */ TVec3f mBinderDirection5DC;
    /* 0x5E8 */ TVec3f mPreviousPosition;
    /* 0x5F4 */ TVec3f _240;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x600 */ TVec3f mCapPosition;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x60C */ TVec3f mVector60C;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x618 */ TVec3f _270;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x624 */ TVec3f _27C;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x630 */ TVec3f _288;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x63C */ TVec3f mCameraTrans;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x648 */ TVec3f _2A0;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x654 */ TVec3f _legacy2AC;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x660 */ u16 mCounter660;
    /* 0x662 */ u16 mWaterSurfaceCameraTimer662;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x664 */ u16 _334;
    /* 0x666 */ bool mFirstTransformCameraLatch666;
    /* 0x667 */ u8 _pad667;
    /* 0x668 */ u16 _338;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x66A */ u16 _332;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x66C */ TVec3f _33C;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x678 */ TVec3f _348;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x684 */ TVec3f _354;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x690 */ TVec3f _360;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x69C */ GravityInfo* mGravityInfo;  // SMG2 offset inh
    /* 0x6A0 */ u8 mAxisRotationDisabled6A0;
    /* 0x6A1 */ u8 _reserved6A1[3];
    /* 0x6A4 */ f32 mGravityRatio;  // SMG2 offset inh-gap  // SMG2 offset vote-gap
    /* 0x6A8 */ u16 _378;
    /* 0x6AA */ bool _370;  // SMG2 offset inh  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x6AB */ u8 _pad6AB[0x1];
    /* 0x6AC */ u32 _37C;
    /* 0x6B0 */ u32 mHealth;
    /* 0x6B4 */ u32 _92C;
    /* 0x6B8 */ u16 _38C;
    /* 0x6BA */ u8 _pad6BA[0x2];
    /* 0x6BC */ u32 _390;
    /* 0x6C0 */ u32 _394;  // SMG2 offset vote-TAIL
    /* 0x6C4 */ u32 _398;
    /* 0x6C8 */ s32 mCounter6C8;
    /* 0x6CC */ u8 _39D;
    /* 0x6CD */ u8 _39C;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0x6CE */ u16 _3A8;
    /* 0x6D0 */ u16 _3AA;
    /* 0x6D2 */ u16 _3AC;
    /* 0x6D4 */ f32 _3B0;
    TVec3f mPressNormal6D8;
    /* 0x6E4 */ bool _3C0;
    /* 0x6E5 */ bool _3C1;
    /* 0x6E6 */ u8 _pad6E6[0x2];
    /* 0x6E8 */ TVec3f mRushFront6E8;  // SMG2 offset inh-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0x6F4 */ u16 _3D0;
    /* 0x6F6 */ u16 _3D2;
    /* 0x6F8 */ u16 mPlayerMode;
    /* 0x6FA */ u16 _3D6;
    /* 0x6FC */ u16 _3D8;
    /* 0x6FE */ u16 _3DA;
    /* 0x700 */ u16 _3DC;
    /* 0x702 */ u8 _3DE;
    /* 0x703 */ u8 _3DF;
    /* 0x704 */ bool _3E5;
    /* 0x705 */ bool _3E6;
    /* 0x706 */ bool _3E7;  // SMG2 offset guess  // SMG2 offset inh  // SMG2 offset inh
    /* 0x707 */ bool _3E8;  // SMG2 offset guess  // SMG2 offset inh  // SMG2 offset inh
    /* 0x708 */ u32 _41C;
    /* 0x70C */ HitSensor* _424;
    /* 0x710 */ HitSensor* _428[4];  // SMG2 offset inh-gap
    /* 0x720 */ TVec3f mThrowMemoRotations[4];
    /* 0x750 */ u32 _468;
    /* 0x754 */ HitSensor* _46C;
    /* 0x758 */ HitSensor* _470;
    /* 0x75C */ HitSensor* _474;
    /* 0x760 */ f32 _478;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x764 */ u32 _47C;
    /* 0x768 */ bool _480;
    /* 0x769 */ u8 _481;
    /* 0x76A */ bool _482;
    /* 0x76B */ bool _483;
    /* 0x76C */ TVec3f mThrowProbePosition76C;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0x778 */ f32 mCarryWobblePhase778;
    /* 0x77C */ FixedPosition* _494;  // SMG2 offset guess-gap
    /* 0x780 */ FixedPosition* mCarryFixedPosition780;
    /* 0x784 */ FixedPosition* mTurnipFixedPosition784;
    /* 0x788 */ FixedPosition* mDrillFixedPosition788;
    /* 0x78C */ FixedPosition* mSearchLightThrowPos;  // SMG2 offset guess-gap
    /* 0x790 */ HitSensor* _4A4;  // used in calcAnimInMovement()
    /* 0x794 */ HitSensor* mSensor794;
    /* 0x798 */ HitSensor* mNearestUnderSensor798;
    /* 0x79C */ f32 mNearestUnderAngle79C;
    /* 0x7A0 */ f32 mBodyRadius7A0;
    /* 0x7A4 */ f32 mBodyHeight7A4;
    /* 0x7A8 */ TVec3f mBodyUpDirection;
    /* 0x7B4 */ TVec3f mBodyDownDirection;
    /* 0x7C0 */ HitSensor* mPulledSensors7C0[128];
    /* 0x9C0 */ bool _6D0;
    /* 0x9C1 */ u8 _pad9C1[0x3];
    /* 0x9C4 */ f32 mSpinRadius;
    /* 0x9C8 */ f32 mSpinHeight;
    /* 0x9CC */ f32 mSpinNoise[128];
    /* 0xBCC */ HitSensor* mTornadoSensors[64];
    /* 0xCCC */ u16 _7DC;  // mTornadoSensors count
    /* 0xCCE */ u16 _7DE;
    /* 0xCD0 */ u16 _7E0;  // SMG2 offset guess  // SMG2 offset inh  // SMG2 offset inh
    /* 0xCD2 */ u8 _padCD2[0x2];
    /* 0xCD4 */ HitSensor* _7E4[64];  // SMG2 offset guess  // SMG2 offset inh  // SMG2 offset inh
    /* 0xDD4 */ bool mRushTouchingDD4[64];
    /* 0xE14 */ HitSensor* _924;
    /* 0xE18 */ HitSensor* _928;
    u32 mLastRushEndFrameE1C;
    /* 0xE20 */ u32 _930;
    /* 0xE24 */ bool _934;
    /* 0xE25 */ u8 _7E2;
    /* 0xE26 */ u16 _F16;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xE28 */ s32 mAnimationDelayE28;
    /* 0xE2C */ bool _935;  // SMG2 offset guess
    /* 0xE2D */ u8 _padE2D[0x1];
    /* 0xE2E */ u16 _F14;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xE30 */ TVec3f _938;  // SMG2 offset guess  // SMG2 offset guess  // SMG2 offset inh
    /* 0xE3C */ u8 _944;
    /* 0xE3D */ u8 _945;
    /* 0xE3E */ u16 _946;
    /* 0xE40 */ u16 _94E;
    /* 0xE42 */ u16 _948;
    /* 0xE44 */ u16 _94C;
    /* 0xE46 */ bool _950;
    /* 0xE47 */ bool _F21;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset guess
    /* 0xE48 */ HitSensor* _954[8];
    /* 0xE68 */ u8 _974;  // _954 count
    /* 0xE69 */ u8 _F0D;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xE6A */ u8 _padE6A[0x2];
    /* 0xE6C */ TVec3f _978;  // SMG2 offset inh  // SMG2 offset guess  // SMG2 offset inh
    /* 0xE78 */ f32 _984;
    /* 0xE7C */ u8 _988;  // SMG2 offset inh
    /* 0xE7D */ u8 _989;
    /* 0xE7E */ bool _1B5;  // SMG2 offset vote-gap  // SMG2 offset vote-gap
    /* 0xE7F */ u8 _1D0;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset guess
    /* 0xE80 */ TornadoMario* mTornadoMario;  // SMG2 offset inh  // SMG2 offset guess-gap  // SMG2 offset guess
    /* 0xE84 */ u8 _990;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset guess
    /* 0xE85 */ u8 _padE85[0x3];
    /* 0xE88 */ LiveActor* _994;  // probably some subclass of LiveActor  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset guess
    /* 0xE8C */ MarioSearchLight* mSearchLight;  // SMG2 offset guess-gap
    /* 0xE90 */ u32 mSearchLightTimer;
    /* 0xE94 */ JetTurtleShadow* mJetTurtleShadowE94;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0xE98 */ MarioTeresaModel* mTeresaModel;  // SMG2 offset guess-gap
    /* 0xE9C */ ModelHolder* mModelE9C;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xEA0 */ MarioParts* _9C4;  // SMG2 offset guess-gap
    /* 0xEA4 */ ModelHolder* _9C0;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0xEA8 */ f32 _9CC;
    /* 0xEAC */ f32 _9D0;
    /* 0xEB0 */ HitSensor* _9D4;
    /* 0xEB4 */ TVec3f _9D8;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0xEC0 */ ModelHolder* _9E4;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0xEC4 */ MarioParts* _9E8;  // SMG2 offset inh
    /* 0xEC8 */ FurMulti* _9EC;  // SMG2 offset inh
    /* 0xECC */ u8 _9F0;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0xECD */ u8 mBeeWallWalk;
    /* 0xECE */ u16 _9F2;
    /* 0xED0 */ TVec3f mGravityReferenceED0;
    /* 0xEDC */ ModelHolder* mModelEDC;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0xEE0 */ ModelHolder* mModelEE0;
    /* 0xEE4 */ YoshiHolder* mYoshiHolder;
    /* 0xEE8 */ ModelHolder* mModelEE8;
    /* 0xEEC */ ModelHolder* mModelEEC;
    /* 0xEF0 */ CloudMarioHat* mCloudMarioHatEF0;
    /* 0xEF4 */ ModelHolder* mModelEF4;
    /* 0xEF8 */ bool mWaterReflectionEF8;
    /* 0xEF9 */ bool mSphereReflectionEF9;
    /* 0xEFA */ bool mShadowEnabledEFA;
    /* 0xEFB */ u8 mShadowModeEFB;
    /* 0xEFC */ bool mGroundReflectionEFC;
    /* 0xEFD */ u8 mGroundReflectionAlphaEFD;
    /* 0xEFE */ u8 _padReflectionEFE[2];
    /* 0xF00 */ MarioAllModel* mAllModel;
    /* 0xF04 */ ModelHolder* mSimpleModelActorF04;
    /* 0xF08 */ J3DModelX* mSimpleModelF08;
    /* 0xF0C */ TVec3f _A18;
    /* 0xF18 */ u8 mFlagF18;
    /* 0xF19 */ u8 mFlagF19;
    /* 0xF1A */ u8 _padF1A[2];
    /* 0xF1C */ ModelHolder* mModelF1C;
    /* 0xF20 */ ModelHolder* _A10;
    /* 0xF24 */ ModelHolder* mModelF24;
    /* 0xF28 */ ModelHolder* mModelF28;
    /* 0xF2C */ ModelHolder* mHandModelL;
    /* 0xF30 */ ModelHolder* mHandModelR;
    /* 0xF34 */ u8 mShadowHideHandsF34;
    /* 0xF35 */ u8 mHandFlagF35;
    /* 0xF36 */ u8 mHandFlagF36;
    /* 0xF37 */ u8 mFaceJointCountF37;
    /* 0xF38 */ ModelHolder* mModelF38;
    /* 0xF3C */ u8 mFaceJointIndexF3C;
    /* 0xF3D */ u8 _padFaceF3D[3];
    /* 0xF40 */ MarioActorEffectState* mActorEffectState;
    /* 0xF44 */ TVec3f mReflectionPositionF44;
    /* 0xF50 */ TVec2f mScreenF50;
    /* 0xF58 */ TVec2f mScreenF58;
    /* 0xF60 */ TVec2f mScreenF60;
    /* 0xF68 */ TVec2f mScreenF68;
    /* 0xF70 */ u8* mScreenBuffer;
    /* 0xF74 */ FootPrint* _B48;
    /* 0xF78 */ IceStep** _B4C;  // SMG2 offset guess-gap
    /* 0xF7C */ u16 _B50;  // _B4C count  // SMG2 offset guess-gap
    /* 0xF7E */ u16 _B6A;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xF80 */ FireMarioBall* mFireBalls[5];
    /* 0xF94 */ s32 mFireBallDelay;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0xF98 */ u8 mBlinkF98;
    /* 0xF99 */ u8 _padF99;
    /* 0xF9A */ u16 mEyeTextureFrameF9A;
    /* 0xF9C */ u8 mBlinkF9C;
    /* 0xF9D */ u8 _padF9D;
    /* 0xF9E */ u16 mBlinkF9E;
    /* 0xFA0 */ J3DAnmTexPattern* mEyeRes;
    /* 0xFA4 */ JUTTexture* _B7C;
    JUTTexture* mMaskTextures[2];
    /* 0xFB0 */ u16 mRandomTextureIndex;
    u16 _padFB2;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0xFB4 */ MarioNullBck* mNullAnimation;
    /* 0xFB8 */ bool _B90;
    /* 0xFB9 */ bool _B91;
    /* 0xFBA */ s8 _B92;
    /* 0xFBB */ u8 _B72;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0xFBC */ u16 _B94;
    /* 0xFBE */ u8 _B96;
    /* 0xFBF */ bool _EED;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xFC0 */ u16 mCommonEffectCount;
    /* 0xFC2 */ u16 mTimedCommonEffectCount;
    /* 0xFC4 */ SmokeEffectEntry** mTimedCommonEffects;
    /* 0xFC8 */ HashSortTable* mMaterialEffectsFC8;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xFCC */ ModelObj* mWaterColumn;
    /* 0xFD0 */ TVec3f mEffectTranslationFD0;
    /* 0xFDC */ TVec3f mEffectRotationFDC;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xFE8 */ bool _EEB;
    /* 0xFE9 */ bool mActionButtonHeld;
    /* 0xFEA */ bool mActionTriggered;
    /* 0xFEB */ bool mJumpButtonHeld;
    /* 0xFEC */ u16 _EF0;
    /* 0xFEE */ u16 _EF2;
    /* 0xFF0 */ u16 _EF4;
    /* 0xFF2 */ u16 _EF6;
    /* 0xFF4 */ u32 _EF8;
    /* 0xFF8 */ u32 _EFC;
    /* 0xFFC */ bool _F00;  // SMG2 offset guess-gap
    /* 0xFFD */ u8 _padFFD[0x1];
    /* 0xFFE */ u16 _legacyB10;  // SMG2 offset guess  // SMG2 offset inh-gap  // SMG2 offset inh
    /* 0x1000 */ u32 _F04;  // SMG2 offset guess-gap
    /* 0x1004 */ u32 _F08;  // SMG2 offset guess-gap
    /* 0x1008 */ bool _F0C;  // SMG2 offset guess-gap
    /* 0x1009 */ bool _F20;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x100A */ u16 _F1C;
    /* 0x100C */ u16 _F1E;
    /* 0x100E */ u8 mFlag100E;
    u8 mFlag100F;  // SMG2 offset guess  // SMG2 offset inh-gap  // SMG2 offset inh
    /* 0x1010 */ u8 mSwimSurfaceState1010;
    /* 0x1011 */ u8 _pad1011[3];
    /* 0x1014 */ TVec3f mCameraPosition;
    /* 0x1020 */ TVec3f mCameraBasisX;
    /* 0x102C */ TVec3f mCameraBasisY;
    /* 0x1038 */ TVec3f mCameraBasisZ;
    /* 0x1044 */ TVec3f mPostureReference1044;
    /* 0x1050 */ bool _FCC;
    /* 0x1051 */ bool _FCD;
    /* 0x1052 */ u8 mFlag1052;
    u8 mDashWallHit1053;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset guess
    /* 0x1054 */ u32 mRushAnimationDelay1054;
    /* 0x1058 */ TVec3f mCarryAnimationPosition1058;
    /* 0x1064 */ u8 mFlag1064;
    /* 0x1065 */ u8 mWasInMorphLimitArea1065;
    /* 0x1066 */ bool mReflectionSuppressed;
    /* 0x1067 */ bool mBlinkRequested1067;
    /* 0x1068 */ u8 _pad1068[4];
    /* 0x106C */ const Triangle* mPunchTriangle106C;
    /* 0x1070 */ bool mLifeDecreasedThisFrame;
    /* 0x1071 */ u8 _pad1071[3];
    /* 0x1074 */ MarioScreenBlur* mScreenBlur1074;
    /* 0x1078 */ MarioFrontHistory* mFrontHistory;
    /* 0x107C */ MarioInputConfig* mInputConfig;
    /* 0x1080 */ MarioActionTable* mActionTable1080;
    /* 0x1084 */ MarioScreenStrip* mScreenStrip1084;
    /* 0x1088 */ MarioActorConditionGroup* mConditionGroup;
    /* 0x108C */ MarioSensorSlots* mSensorSlots108C;
    /* 0x1090 */ MarioHairAndHatModel* mHairAndHat1090;
    /* 0x1094 */ MarioActionHash* mActionHash;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset guess
};

// header defined since these actually get defined in CamHeliEffector
// and in sinit of NameObjFactory
// SMG2: only MarioActor.cpp instantiates these (other Player TUs' __sinit do not).
#ifdef MARIO_ACTOR_DEFINE_NERVES
namespace NrvMarioActor {
    NEW_NERVE(MarioActorNrvWait, MarioActor, Wait);
    NEW_NERVE(MarioActorNrvGameOver, MarioActor, GameOver);
    NEW_NERVE(MarioActorNrvGameOverAbyss, MarioActor, GameOverAbyss);
    NEW_NERVE(MarioActorNrvGameOverAbyss2, MarioActor, GameOverAbyss);
    NEW_NERVE(MarioActorNrvGameOverFire, MarioActor, GameOverFire);
    NEW_NERVE(MarioActorNrvGameOverBlackHole, MarioActor, GameOverBlackHole2);
    NEW_NERVE(MarioActorNrvGameOverNonStop, MarioActor, GameOverNonStop);
    NEW_NERVE(MarioActorNrvGameOverSink, MarioActor, GameOverSink);
    NEW_NERVE(MarioActorNrvGameOverNoDemo, MarioActor, GameOverNoDemo);
    NEW_NERVE(MarioActorNrvTimeWait, MarioActor, TimeWait);
    NEW_NERVE(MarioActorNrvNoRush, MarioActor, Wait);
};  // namespace NrvMarioActor
#else
// declarations only (the instances are defined in MarioActor.cpp)
namespace NrvMarioActor {
    class MarioActorNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static MarioActorNrvWait sInstance;
    };
    class MarioActorNrvGameOver : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static MarioActorNrvGameOver sInstance;
    };
    class MarioActorNrvGameOverAbyss : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static MarioActorNrvGameOverAbyss sInstance;
    };
    class MarioActorNrvGameOverAbyss2 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static MarioActorNrvGameOverAbyss2 sInstance;
    };
    class MarioActorNrvGameOverFire : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static MarioActorNrvGameOverFire sInstance;
    };
    class MarioActorNrvGameOverBlackHole : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static MarioActorNrvGameOverBlackHole sInstance;
    };
    class MarioActorNrvGameOverNonStop : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static MarioActorNrvGameOverNonStop sInstance;
    };
    class MarioActorNrvGameOverSink : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static MarioActorNrvGameOverSink sInstance;
    };
    class MarioActorNrvGameOverNoDemo : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static MarioActorNrvGameOverNoDemo sInstance;
    };
    class MarioActorNrvTimeWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static MarioActorNrvTimeWait sInstance;
    };
    class MarioActorNrvNoRush : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static MarioActorNrvNoRush sInstance;
    };
};  // namespace NrvMarioActor
#endif
