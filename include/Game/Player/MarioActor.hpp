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
struct MarioInputConfig {
    MarioInputConfig();
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
    void fn_803B8A80();
    s32 fn_803B8DE0();
    u8* fn_803B8FE0();
    u8* fn_803B9020();
    void fn_803B9270();
    bool fn_803B93E0();
    void fn_803B94E0();
    void fn_803B9560();
    Yoshi* fn_803B97F0();
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
    bool fn_803B9AB0();
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
    void fn_803BE670();
    void fn_803BE6D0();
    void fn_803BE6F0();
    bool fn_803BE800();
    void fn_803BE840(const TVec3f&);
    void fn_803BE8D0(const TVec3f&);
    void fn_803BE960();
    void fn_803BE9E0();
    void fn_803BEA60(const char* a, const char* b);
    void fn_803BEA70(const char* a, const BckCtrlData* b);
    bool fn_803BEA80(const char*);
    bool fn_803BEAA0(const char*, const BckCtrlData&);
    void fn_803BEAC0();
    void fn_803BEAF0();
    void fn_803BEB10(s32 a);
    bool fn_803BEBE0();
    void* fn_803BECC0();
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
    void fn_803C0E60();
    void fn_803C0EF0();
    void fn_803C1940();
    void fn_803C1AF0(TVec3f* a, TVec3f* b);
    bool fn_803C1C60();
    bool fn_803C1D30();
    bool fn_803C1E40();
    void fn_803C1E80();
    bool fn_803C1F30();
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
    bool fn_803C3930();
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
    void fn_803C8E40(void* pk, u16 idx, s32 which);
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
    void fn_803CF130(MtxPtr mtx);
    bool fn_803CFCA0(HitSensor* s);
    s32 fn_803CFE30(HitSensor* s);
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
    void fn_803D8200();
    void fn_803D82C0(TVec2f* a, TVec2f* b);
    void fn_803D8400(TVec2f* a, TVec2f* b);
    s32 fn_803D8490();
    void fn_803D86B0(TVec2f* a, TVec2f* b);
    void fn_803D8A30(s32 a, TVec3f& v);
    void fn_803D8A70(s32 mode);
    bool fn_803D8B30(HitSensor* s);
    void fn_803D8BD0(TVec3f* a, TVec3f* b);
    void fn_803D8E30(TVec3f* v);
    void fn_803D9170(TVec3f* v);
    bool fn_803D9370(void* tri, TVec3f* v);
    bool fn_803D9430();
    void fn_803D95E0();
    void fn_803D9670();
    void fn_803D96C0();
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

    void init2(const TVec3f&, const TVec3f&, s32);
    void initAfterOpeningDemo();
    void calcBaseFrontVec(const TVec3f&);
    void playSound(const char*, s32);
    void changeAnimation(const char*, const char* = nullptr);
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
    void resetWaterLife();
    void updateLife();
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
    void initTeresaMarioAnimation();
    void initTornadoMario();
    void initBoneMario();
    void swapTextureInit();
    void initBlur();
    void changeDisplayMode();
    bool isAllHidden() const;
    void calcViewMainModel();
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
    void setBlink(const char*);

    void entryWallWalkMode(const TVec3f&, const TVec3f&);

    HitSensor* getLookTargetSensor() const;
    J3DModelX* getJ3DModel() const;
    J3DModelData* getModelData() const;
    J3DModelX* getSimpleModel() const;
    void createTextureDL(DLholder*, u16, u16);
    void swapTexture(const char*, u8) const;
    void copyMaterial(J3DModel*, u16, s32);

    void settingRush();

    void resetCondition();
    bool isFixJumpRushSensor(const HitSensor*) const;
    bool isLandEffectRushSensor(const HitSensor*) const;
    void beginRush(HitSensor*);

    bool takeSensor(HitSensor*);

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
    bool isInZeroGravitySpot() const;
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
    bool isRequestHipDrop() const;
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
    bool fn_803CB0B0();                                   // SMG2: Yoshi tongue-action check (symbol is misnamed resetPlayerModeOnDamage)
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
    void calcViewWallShadowModel();
    void drawShadow() const;
    void decideShadowMode();
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
    void drawWallShade(const TVec3f&, const TVec3f&, f32) const;
    void drawSpinInhibit() const;
    void drawColdWaterDamage() const;
    void setRasterScroll(s32, s32, s32);
    void updateRasterScroll();
    void drawRasterScroll(f32, s16, f32) const;
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
    void tryCounterJetAttack(HitSensor*);
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
    HitSensor* getNearestRushTarget(bool) const;
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
        return mMario->getMovementStates()._1 && strcmp("Fur", MR::getWallCodeString(mMario->getGroundPolygon())) == 0 && mBeeWallWalk == 0;
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

    /* 0x090 */ union { u8 _pad90[0x2]; bool mInitializing90; };
    /* 0x092 */ u8 _8C;  // SMG2 offset guess  // SMG2 offset guess  // SMG2 offset guess
    /* 0x093 */ u8 _pad93[0x1];
    /* 0x094 */ DLchanger* mDLchanger;  // SMG2 offset guess-gap
    union {
        struct {
    /* 0x098 */ TMtx34f _legacyC8C;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0x0C8 */ TMtx34f _legacyCBC;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0x0F8 */ u8 _padF8[0x1c];
    /* 0x114 */ TMtx34f _E0C;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0x144 */ TMtx34f _legacyDDC;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    union {
        struct { u8 _pad174[0x24]; u8* mDL[2]; u32 _pad1A0; };
        /* 0x174 */ TMtx34f _legacyDAC;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    };
        };
        DLchanger* mBlurDisplayLists[64];
        DLchanger* _94[64];
    };
    union { u16 _F40; struct { u8 mCurrDL; u8 _pad1A5; }; };
    /* 0x1A6 */ u8 _pad1A6;
    /* 0x1A7 */ u8 _1A1;  // SMG2 offset guess  // SMG2 offset guess  // SMG2 offset inh
    /* 0x1A8 */ f32 _1A4;  // SMG2 offset guess-gap
    /* 0x1AC */ u32 mMaxHealth;  // SMG2 offset vote-gap
    /* 0x1B0 */ u8 _B68;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x1B1 */ u8 _A6E;  // SMG2 offset guess-gap
    /* 0x1B2 */ u16 _F42;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    union {
        struct {
    union {
        struct {
    /* 0x1B4 */ TMtx34f _legacyD7C;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x1E4 */ TMtx34f _legacyD4C;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x214 */ TMtx34f _legacyD1C;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
        };
        Mtx* mBlurMatrices[2][8];
    };
    union {  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
        /* 0x244 */ u32 _B98;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
        struct {  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
            unsigned mSmoke : 1;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
            unsigned mSnow : 1;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
            unsigned _2 : 1;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
            unsigned mSlopeSlip : 1;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
            unsigned mIsStationary : 1;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
            unsigned mBeeWind : 1;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
        } mLegacyEffectFlags;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    };  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x248 */ u16 _B9C;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x24A */ u16 _B9E;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x24C */ SmokeEffectEntry** _BA0;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x250 */ HashSortTable* _BA4;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
        };
        struct { u8 _padBlur1B4To1F4[0x40]; union { TMtx34f mPreviousViewMatrix; TMtx34f _AB0; }; union { TMtx34f mCurrentViewMatrix; TMtx34f _AE0; }; };
    };
    /* 0x254 */ union { ModelObj* _legacyBA8; struct { union { u16 mBlurBufferIndex; u16 _B10; }; union { u16 mBlurHistoryIndex; u16 _B12; }; }; };  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x258 */ union {
        u8 _pad258[0x8];
        struct { union { u16 mBlurJointCount; u16 _B14; }; u8 _padBlur25A[6]; };
        struct {
            u8 _padEffect258[4];
            union {
                u32 mEffectFlagsWord;
                struct {
                    unsigned mSmoke : 1;
                    unsigned mSnow : 1;
                    unsigned _2 : 1;
                    unsigned mSlopeSlip : 1;
                    unsigned mIsStationary : 1;
                    unsigned mBeeWind : 1;
                    unsigned mSpecialSmoke : 1;
                } mEffectFlags;
            };
        };
    };
    /* 0x260 */ u16 _BC4;
    /* 0x262 */ u8 _pad262[0x2];
    union { struct {
    /* 0x264 */ TVec3f _BAC;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x270 */ TVec3f _BB8;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    union {
        struct {
    union {
        struct {
    union {
        struct {
    /* 0x27C */ TMtx34f _BC8;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x2AC */ TMtx34f _BF8;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
        };
        struct { u8 _padJoint27C[0x18]; TMtx34f mJointTransform294; };
    };
    /* 0x2DC */ Mtx* _C28;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x2E0 */ TMtx34f _legacyC2C;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x310 */ TMtx34f _legacyC5C;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x340 */ TVec3f mBlackHoleRotateAxis;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x34C */ DLchanger* _legacy94[0x40];  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x44C */ u8 _pad44C[0x50];
    /* 0x49C */ TMtx34f _legacyCEC;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
        };
        struct {
            u8 _padJoint27CTo2C8[0x4C];
            union { TMtx34f mHandLMatrix; TMtx34f _C2C; };
            union { TMtx34f mHandRMatrix; TMtx34f _C5C; };
            union { TMtx34f mHandLJointMatrix; TMtx34f _C8C; };
            union { TMtx34f mHandRJointMatrix; TMtx34f _CBC; };
            union { TMtx34f mFaceJointMatrix; TMtx34f _CEC; };
            union { TMtx34f mFootRMatrix; TMtx34f _D1C; };
            union { TMtx34f mSpine1Matrix; TMtx34f _D4C; };
            union { TMtx34f mCapMatrix; TMtx34f _D7C; };
            union { TMtx34f mHipMatrix; TMtx34f _DAC; };
            union { TMtx34f mSpine2Matrix; TMtx34f _DDC; };
        };
    };
    /* 0x4CC */ TVec3f mBlackHolePosition;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
        };
        struct { u8 _padJointMatrix27C[0x22C]; TMtx34f mMainJointMatrix4A8; };
        struct { u8 _padJointCache27C[0x48]; Mtx* mJointMatrices2C4; };
    };
    }; TMtx34f mReflectionBaseMtx264; };
    /* 0x4D8 */ u16 mBlendMtxTimer;
    /* 0x4DA */ u8 _pad4DA[0x2];
    /* 0x4DC */ f32 _EA0;
    union { struct {
    /* 0x4E0 */ bool _EA4;
    /* 0x4E1 */ bool _EA5;
    /* 0x4E2 */ bool _EA6;  // SMG2 offset guess  // SMG2 offset guess  // SMG2 offset inh
    /* 0x4E3 */ u8 _pad4E3[0x9];
    union {  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
        /* 0x4EC */ AudGeneric* _F3C;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
        /* 0x4EC */ TVec3f* _F3CVec;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    };  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0x4F0 */  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    union {  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
        /* 0x4F0 */ u32 _F2C;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
        /* 0x4F0 */ TVec3f _F2CVec;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    };  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0x4FC */ u8 _pad4FC[0xc];
    /* 0x508 */ TVec3f _2B8;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    }; struct { u8 _padRemote4E0[4]; TMtx34f mRemoteDemoMtx4E4; }; };
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
    union { TVec3f mBlackHolePosition528; struct {
    /* 0x528 */ const char* _EE4;  // SMG2 offset guess  // SMG2 offset inh-gap  // SMG2 offset inh
    /* 0x52C */ f32 _1E4;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x530 */ u8 _pad530[0x2];
    /* 0x532 */ u8 _1E2;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x533 */ u8 _F38;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    }; };
    /* 0x534 */ TVec3f _ED8;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0x540 */ TVec3f _2C4;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0x54C */ const Nerve* _FB4;  // SMG2 offset guess-gap
    /* 0x550 */ u16 _FB8;  // a timer  // SMG2 offset guess-gap
    /* 0x552 */ u16 _1A8;  // SMG2 offset guess  // SMG2 offset guess  // SMG2 offset inh
    /* 0x554 */ u16 _1AA;
    /* 0x556 */ bool _F74;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0x557 */ u8 _pad557[0x1];
    /* 0x558 */ f32 _1AC;
    /* 0x55C */ union { Color8 mSwimFogColor; u32 mDLSize; };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x560 */ u8 _1B4;
    /* 0x561 */ bool mSwimFogDisabled;
    /* 0x562 */ u16 _1E8;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x564 */ CollectCounter* _1B8;  // SMG2 offset inh  // SMG2 offset guess  // SMG2 offset inh
    union { struct { u8 _pad568[3]; bool _1C1; }; MarioMessenger* mMessenger568; };  // SMG2 offset guess  // SMG2 offset inh-gap  // SMG2 offset inh
    /* 0x56C */ bool _1C0;
    /* 0x56D */ u8 _A0C;  // SMG2 offset guess-gap  // SMG2 offset vote-gap  // SMG2 offset vote-gap
    /* 0x56E */ bool _1C3;
    /* 0x56F */ bool _1C2;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0x570 */ u16 _1C4;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0x572 */ bool _1E1;
    /* 0x573 */ bool _1E0;
    /* 0x574 */ u8 _pad574[8];  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x57C */ u16 _1C6;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x57E */ bool _3E4;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x57F */ u8 _1D1;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x580 */ union { f32 _1C8; CollisionShadow* mCollisionShadow580; };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x584 */ Mario* mMario;
    /* 0x588 */ MarioAnimator* mMarioAnim;
    /* 0x58C */ MarioEffect* mMarioEffect;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x590 */ MarioConst* mConst;
    union {
        struct {
    union {
        /* 0x594 */ TVec3f mAppliedGravityVec;
        struct {
    /* 0x594 */ f32 _1CC;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x598 */ f32 _1D4;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    union {  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
        /* 0x59C */ FBO* mRasterBuffers[2];  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
        struct {  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
            /* 0x59C */ FBO* _1D8;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
            /* 0x5A0 */ FBO* _1DC;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
        };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
        };
    };
    /* 0x5A4 */ f32 _1EC;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x5A8 */ f32 _208;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
        };
        struct { u8 _padGravity594[0xC]; TVec3f mPreviousGravityVec; };
    };
    /* 0x5AC */ TVec3f _258;  // SMG2 offset guess-gap
    /* 0x5B8 */ union { TVec3f _1F0; TVec3f mRushVelocityBackup5B8; };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x5C4 */ TVec3f _1FC;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    union {
        struct {
    /* 0x5D0 */ const AreaObj* _20C;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x5D4 */ u8 _210;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x5D5 */ u8 _211;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x5D6 */ u16 _330;  // a timer  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x5D8 */ CollisionShadow* _214;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
        };
        TVec3f mBinderMotion5D0;
    };
    union {
        struct {
    /* 0x5DC */ DrawAdaptor* _218;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x5E0 */ DrawAdaptor* _21C;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x5E4 */ DrawAdaptor* _220;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
        };
        TVec3f mBinderDirection5DC;
    };
    /* 0x5E8 */ TVec3f mPreviousPosition;
    /* 0x5F4 */ TVec3f _240;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x600 */ union { TVec3f _24C; TVec3f mCapPosition; TVec3f _2AC; };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x60C */ TVec3f mLastMove;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x618 */ TVec3f _270;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x624 */ TVec3f _27C;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x630 */ TVec3f _288;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x63C */ TVec3f _294;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x648 */ TVec3f _2A0;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x654 */ TVec3f _legacy2AC;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x660 */ MatrixControl* _FBC;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x664 */ u16 _334;
    /* 0x666 */ u16 _336;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x668 */ u16 _338;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x66A */ u16 _332;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x66C */ TVec3f _33C;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x678 */ TVec3f _348;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x684 */ TVec3f _354;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x690 */ TVec3f _360;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x69C */ GravityInfo* mGravityInfo;  // SMG2 offset inh
    /* 0x6A0 */ union { u32 _420; u8 mAxisRotationDisabled6A0; };  // SMG2 offset vote-gap  // SMG2 offset vote-gap  // SMG2 offset vote-gap
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
    /* 0x6C8 */ u8 _pad6C8[0x4];
    /* 0x6CC */ u8 _39D;
    /* 0x6CD */ u8 _39C;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0x6CE */ u16 _3A8;
    /* 0x6D0 */ u16 _3AA;
    /* 0x6D2 */ u16 _3AC;
    /* 0x6D4 */ f32 _3B0;
    union { TVec3f mPressNormal6D8; struct {
    /* 0x6D8 */ u32 _388;  // SMG2 offset inh-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0x6DC */ u8 _39E;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0x6DD */ u8 _pad6DD[0x3];
    /* 0x6E0 */ f32 _490;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    }; };
    /* 0x6E4 */ bool _3C0;
    /* 0x6E5 */ bool _3C1;
    /* 0x6E6 */ u8 _pad6E6[0x2];
    /* 0x6E8 */ union { TVec3f _3B4; TVec3f mRushFront6E8; };  // SMG2 offset inh-gap  // SMG2 offset inh  // SMG2 offset inh
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
    /* 0x720 */ TVec3f _3C4;  // SMG2 offset inh-gap  // SMG2 offset guess-gap  // SMG2 offset guess
    /* 0x72C */ TVec3f mUpVec;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset guess
    /* 0x738 */ TVec3f _2DC;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset guess
    /* 0x744 */ HitSensor* _4A8;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset guess
    /* 0x748 */ FixedPosition* _49C;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset guess
    /* 0x74C */ u32 mWaterLife;  // SMG2 offset vote-gap  // SMG2 offset vote-gap
    union {
        struct {
            /* 0x750 */ u32 _468;
            /* 0x754 */ HitSensor* _46C;
            /* 0x758 */ HitSensor* _470;
        };

        /* 0x750 */ TVec3f _468Vec;
    };
    /* 0x75C */ HitSensor* _474;
    /* 0x760 */ f32 _478;  // SMG2 offset inh  // SMG2 offset inh  // SMG2 offset inh
    /* 0x764 */ u32 _47C;
    /* 0x768 */ bool _480;
    /* 0x769 */ u8 _481;
    /* 0x76A */ bool _482;
    /* 0x76B */ bool _483;
    /* 0x76C */ TVec3f mCameraTrans;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0x778 */ FixedPosition* _498;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x77C */ FixedPosition* _494;  // SMG2 offset guess-gap
    /* 0x780 */ union { TVec3f _300; struct { FixedPosition* mCarryFixedPosition780; FixedPosition* mTurnipFixedPosition784; FixedPosition* mDrillFixedPosition788; }; };  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset inh
    /* 0x78C */ FixedPosition* mSearchLightThrowPos;  // SMG2 offset guess-gap
    /* 0x790 */ HitSensor* _4A4;  // used in calcAnimInMovement()
    /* 0x794 */ u32 _3A0;
    union { struct {
    union {
        struct {
    /* 0x798 */ TVec3f _30C;  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset inh
    /* 0x7A4 */ TVec3f _2E8;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x7B0 */ TVec3f _438[4];  // SMG2 offset inh-gap  // SMG2 offset guess-gap  // SMG2 offset inh
        };
        struct { HitSensor* mNearestUnderSensor798; f32 mNearestUnderAngle79C; f32 mBodyRadius7A0; f32 mBodyHeight7A4; };
        struct { u8 _padBody798To7A8[0x10]; union { TVec3f mBodyUpDirection; TVec3f _4B8; }; union { TVec3f mBodyDownDirection; TVec3f _4C4; }; };
    };
    /* 0x7E0 */ TVec3f _318;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x7EC */ TVec3f _324;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x7F8 */ u32 _3A4;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x7FC */ TMtx34f _3EC;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x82C */ TVec3f _484;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x838 */ f32 _4AC;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x83C */ f32 _4B0;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x840 */ f32 _4B4;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x844 */ TVec3f _legacy4B8;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x850 */ TVec3f _legacy4C4;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x85C */ TMtx34f _E3C;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x88C */ TMtx34f _E6C;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x8BC */ TMtx34f _EA8;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x8EC */ TVec3f mPosRelativeToBlackHole;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x8F8 */ TVec3f mCamPos;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x904 */ TVec3f mCamDirX;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x910 */ TVec3f mCamDirY;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x91C */ TVec3f mCamDirZ;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x928 */ TVec3f _FA8;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x934 */ MatrixControl* _FC0;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x938 */ MatrixControl* _FC4;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x93C */ MatrixValueGetter* _FC8;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x940 */ u8 _pad940[0x48];
    /* 0x988 */ TMtx34f _legacyAB0;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x9B8 */ u8 _pad9B8[8];
    }; struct { u8 _padPull798[0x28]; HitSensor* mPulledSensors7C0[128]; }; };
    /* 0x9C0 */ bool _6D0;
    /* 0x9C1 */ u8 _pad9C1[0x3];
    /* 0x9C4 */ f32 _6D4;
    /* 0x9C8 */ HitSensor* _4D0[128];  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0xBC8 */ f32 _6D8;  // SMG2 offset guess  // SMG2 offset inh  // SMG2 offset inh
    /* 0xBCC */ HitSensor* _6DC[64];  // SMG2 offset guess  // SMG2 offset inh  // SMG2 offset inh
    /* 0xCCC */ u16 _7DC;  // _6DC count
    /* 0xCCE */ u16 _7DE;
    /* 0xCD0 */ u16 _7E0;  // SMG2 offset guess  // SMG2 offset inh  // SMG2 offset inh
    /* 0xCD2 */ u8 _padCD2[0x2];
    /* 0xCD4 */ HitSensor* _7E4[64];  // SMG2 offset guess  // SMG2 offset inh  // SMG2 offset inh
    union {
        struct {
    /* 0xDD4 */ u8 _padDD4[0x4];
    /* 0xDD8 */ FireMarioBall* _B54[3];  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0xDE4 */ TMtx34f _legacyAE0;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
        };
        bool mRushTouchingDD4[64];
    };
    /* 0xE14 */ HitSensor* _924;
    /* 0xE18 */ HitSensor* _928;
    union { u32 mLastRushEndFrameE1C; struct {
    /* 0xE1C */ u16 _F1A;
    /* 0xE1E */ u16 _F18;
    }; };
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
    union {
        HitSensor* _954[8];
        struct {
    /* 0xE48 */ u16 _94A;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset guess
    /* 0xE4A */ u16 _F12;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset guess
    /* 0xE4C */ u8 _padE4C[0x5];
    /* 0xE51 */ u8 _A24;  // SMG2 offset guess  // SMG2 offset guess  // SMG2 offset guess
    /* 0xE52 */ u8 _A25;  // SMG2 offset guess  // SMG2 offset guess  // SMG2 offset guess
    /* 0xE53 */ u8 _padE53[0x5];
    /* 0xE58 */ TVec3f _9F4;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset guess
    /* 0xE64 */ u8 _padE64[0x2];
    /* 0xE66 */ bool _EEE;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset guess
    /* 0xE67 */ u8 _951;  // SMG2 offset vote-gap  // SMG2 offset vote-gap  // SMG2 offset vote-gap
        };
    };
    /* 0xE68 */ u8 _974;  // _954 count
    /* 0xE69 */ u8 _F0D;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xE6A */ u8 _padE6A[0x2];
    /* 0xE6C */ TVec3f _978;  // SMG2 offset inh  // SMG2 offset guess  // SMG2 offset inh
    /* 0xE78 */ f32 _984;
    /* 0xE7C */ u8 _988;  // SMG2 offset inh
    /* 0xE7D */ u8 _989;
    /* 0xE7E */ bool _1B5;  // SMG2 offset vote-gap  // SMG2 offset vote-gap
    /* 0xE7F */ u8 _1D0;  // SMG2 offset guess  // SMG2 offset guess-gap  // SMG2 offset guess
    /* 0xE80 */ union { MarioMessenger* _1BC; TornadoMario* mTornadoMario; };  // SMG2 offset inh  // SMG2 offset guess-gap  // SMG2 offset guess
    /* 0xE84 */ u8 _990;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset guess
    /* 0xE85 */ u8 _padE85[0x3];
    /* 0xE88 */ LiveActor* _994;  // probably some subclass of LiveActor  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset guess
    /* 0xE8C */ MarioSearchLight* mSearchLight;  // SMG2 offset guess-gap
    /* 0xE90 */ u8 _padE90[0x4];
    /* 0xE94 */ union { ModelHolder* _9C8; JetTurtleShadow* mJetTurtleShadowE94; };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0xE98 */ union { MarioTeresaModel* mTeresaModel; XanimePlayer* _9B8; };  // SMG2 offset guess-gap
    /* 0xE9C */ union { XanimeResourceTable* _9BC; ModelHolder* mModelE9C; };  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
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
    /* 0xED0 */ MarioParts* _9A4;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0xED4 */ f32 _9A8;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0xED8 */ f32 _9AC;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0xEDC */ union { ModelHolder* mModelEDC; f32 _9B0; };  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    union { ModelHolder* mModelEE0; struct {
    /* 0xEE0 */ u16 _9B4;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0xEE2 */ u8 _A09;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0xEE3 */ u8 _A08;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    }; };
    /* 0xEE4 */ union { YoshiHolder* mYoshiHolder; Color8 _1B0; };
    /* 0xEE8 */ ModelHolder* mModelEE8;
    /* 0xEEC */ ModelHolder* mModelEEC;
    /* 0xEF0 */ ModelHolder* mModelEF0;
    /* 0xEF4 */ ModelHolder* mModelEF4;
    union { struct {
    /* 0xEF8 */ bool mScreenBoxRequested;
    /* 0xEF9 */ bool mScreenBoxForced;
    /* 0xEFA */ bool mScreenBoxSuppressed;
    /* 0xEFB */ u8 _padEFB[5];
    }; struct { bool mWaterReflectionEF8; bool mSphereReflectionEF9; u8 _padReflectionEFA[2]; bool mGroundReflectionEFC; u8 mGroundReflectionAlphaEFD; u8 _padReflectionEFE[2]; }; };
    /* 0xF00 */ MarioAllModel* mAllModel;
    /* 0xF04 */ u8 _padF04[4];
    /* 0xF08 */ union { ModelHolder* _A00; J3DModelX* mSimpleModelF08; };  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xF0C */ TVec3f _A18;
    /* 0xF18 */ union { ModelHolder* _A04; struct { u8 mFlagF18; u8 mFlagF19; }; };  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    union { ModelHolder* mModelF1C; struct {
    /* 0xF1C */ u8 mCurrModel;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xF1D */ u8 _A0B;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xF1E */ u8 _padF1E[0x1];
    /* 0xF1F */ u8 _A60;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    }; };
    /* 0xF20 */ ModelHolder* _A10;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    union {
        struct {
    /* 0xF24 */ u8 _A5B;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xF25 */ u8 _A5A;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xF26 */ u8 _A59;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xF27 */ bool _8E4[64];  // SMG2 offset guess  // SMG2 offset guess  // SMG2 offset inh
    /* 0xF67 */ bool _A58;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xF68 */ J3DModelX* _A14;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xF6C */ ModelHolder* _A40;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xF70 */ ModelHolder* _A44;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
        };
        struct { ModelHolder* mModelF24; ModelHolder* mModelF28; };
        struct { u8 _padModelF24[0x14]; ModelHolder* mModelF38; };
        struct { u8 _padReflectionF24[0x20]; TVec3f mReflectionPositionF44; };
        struct { u8 _padHandF24[8]; ModelHolder* mHandModelL; ModelHolder* mHandModelR; };
        struct { u8 _padEffectF24[0x1C]; MarioActorEffectState* mActorEffectState; };
        struct { u8 _padF24[0x2C]; TVec2f mScreenF50; TVec2f mScreenF58; TVec2f mScreenF60; TVec2f mScreenF68; u8* mScreenBuffer; };
    };
    /* 0xF74 */ FootPrint* _B48;
    /* 0xF78 */ IceStep** _B4C;  // SMG2 offset guess-gap
    /* 0xF7C */ u16 _B50;  // _B4C count  // SMG2 offset guess-gap
    /* 0xF7E */ u16 _B6A;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xF80 */ union { J3DModelX* mModels[6]; struct { FireMarioBall* mFireBalls[5]; s32 mFireBallDelay; }; };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0xF98 */ union { struct { u8 _padEyeF98[2]; u16 mEyeTextureFrameF9A; }; ModelHolder* _A48; struct { u8 mBlinkF98; u8 _padF99[3]; }; };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0xF9C */ union { ModelHolder* _A4C; struct { u8 mBlinkF9C; u8 _padF9D; u16 mBlinkF9E; }; };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0xFA0 */ union { ModelHolder* _A50; J3DAnmTexPattern* mEyeRes; };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    union { JUTTexture* _B7C; ModelHolder* _A54; };
    union {
        JUTTexture* mMaskTextures[2];
        struct { JUTTexture* _B80; JUTTexture* _B84; };
        struct { ModelHolder* _A5C; bool _A61; bool _A62; u16 _A6C; };
    };
    /* 0xFB0 */ union { u32 _A64; struct { u16 mRandomTextureIndex; u16 _padFB2; }; };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0xFB4 */ MarioNullBck* mNullAnimation;
    /* 0xFB8 */ bool _B90;
    /* 0xFB9 */ bool _B91;
    /* 0xFBA */ s8 _B92;
    /* 0xFBB */ u8 _B72;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
    /* 0xFBC */ u16 _B94;
    /* 0xFBE */ u8 _B96;
    /* 0xFBF */ bool _EED;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xFC0 */ union { f32 _A68; struct { u16 mCommonEffectCount; u16 mTimedCommonEffectCount; }; };
    /* 0xFC4 */ union { SmokeEffectEntry** mTimedCommonEffects; struct { u16 _B60; u8 _padFC6[0x2]; }; };
    /* 0xFC8 */ ResTIMG** _B64;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    union {  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
        ModelObj* mWaterColumn;
        ModelObj* _BA8;
        /* 0xFCC */ TVec2f mScreenBoxPos;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
        struct {  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
            /* 0xFCC */ f32 _B34;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
            /* 0xFD0 */ f32 _B38;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
        };  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    };  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    union {  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
        /* 0xFD4 */ TVec2f mScreenBoxMax;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
        struct {  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
            /* 0xFD4 */ f32 _B2C;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
            /* 0xFD8 */ f32 _B30;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
        };  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    };  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xFDC */ TVec3f _B18;  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset inh
    /* 0xFE8 */ bool _EEB;
    /* 0xFE9 */ bool _EEC;  // SMG2 offset guess  // SMG2 offset inh  // SMG2 offset inh
    /* 0xFEA */ u16 _B70;  // SMG2 offset guess-gap  // SMG2 offset inh  // SMG2 offset inh
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
    /* 0x100E */ union { u16 _legacyB12; struct { u8 mFlag100E; u8 mFlag100F; }; };  // SMG2 offset guess  // SMG2 offset inh-gap  // SMG2 offset inh
    union {
        struct {
    union {
        struct {
    union {  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset inh
        /* 0x1010 */ u8 mSwimSurfaceState1010;
        /* 0x1010 */ TVec2f mScreenBoxMin;  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset inh
        struct {  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset inh
            /* 0x1010 */ f32 _B24;  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset inh
            /* 0x1014 */ f32 _B28;  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset inh
        };  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset inh
    };  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset inh
    /* 0x1018 */ u16 _legacyB14;  // SMG2 offset guess  // SMG2 offset inh-gap  // SMG2 offset inh
    /* 0x101A */ u16 _B74;  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset inh
    /* 0x101C */ u16 _F10;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x101E */ u16 _F0E;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x1020 */ u32 _pad1020;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x1024 */ JetTurtleShadow* _9A0;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x1028 */ u32 mSearchLightTimer;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    /* 0x102C */ u32 _pad102C;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
        };
        struct { u8 _padCamera1010[4]; TVec3f mCameraDirY1014; TVec3f mCameraDirX1020; };
    };
    /* 0x1030 */ u8 _pad1030[0x10];
    /* 0x1040 */ u32 _pad1040;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
        };
        struct { u8 _padBasis1010[0x10]; TVec3f mCameraBasisX; TVec3f mCameraBasisY; TVec3f mCameraBasisZ; };
    };
    union { TVec3f mPostureReference1044; struct {
    /* 0x1044 */ DLholder* _B6C;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    union {  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
        /* 0x1048 */ TVec2f mScreenBoxSize;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
        struct {  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
            /* 0x1048 */ f32 _B3C;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
            /* 0x104C */ f32 _B40;  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
        };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset inh
    }; };
    /* 0x1050 */ bool _FCC;
    /* 0x1051 */ bool _FCD;
    /* 0x1052 */ union { u16 _B88; struct { union { u8 _pad1052; u8 mFlag1052; }; u8 mDashWallHit1053; }; };  // SMG2 offset guess-gap  // SMG2 offset guess  // SMG2 offset guess
    union {  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset guess
        struct {  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset guess
            /* 0x1054 */ Mtx* _A70[8];  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset guess
            /* 0x1074 */ Mtx* _A90[8];  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset guess
        };  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset guess
        struct { u8 _padPunch1054[0x18]; const Triangle* mPunchTriangle106C; };
        struct { u8 _padFront1054[0x24]; MarioFrontHistory* mFrontHistory; };
        struct { u8 _padInput1054[0x10]; u8 mFlag1064; u8 _padFlag1065; bool mInputDisabled1066; bool mBlinkRequested1067; };
        struct { u8 _padCondition1054[0x34]; MarioActorConditionGroup* mConditionGroup; };
        struct { u8 _pad1054[0x28]; MarioInputConfig* mInputConfig; MarioActionTable* mActionTable1080; MarioScreenStrip* mScreenStrip1084; };
        u32 mRushAnimationDelay1054;
        struct { u8 _padCarry1054[4]; TVec3f mCarryAnimationPosition1058; };
        /* 0x1054 */ Mtx* mLegacyBlurMatrices[2][8];  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset guess
    };  // SMG2 offset guess-gap  // SMG2 offset inh-gap  // SMG2 offset guess
    /* 0x1094 */ union { MarioActionHash* mActionHash; void* _B44; };  // SMG2 offset guess-gap  // SMG2 offset guess-gap  // SMG2 offset guess
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
