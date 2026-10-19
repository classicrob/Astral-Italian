#pragma once

#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution.h>

class BckCtrlData;
class CameraTargetArg;
class HitSensor;
class JAISoundID;
class LiveActor;
class MarioActor;
class NameObj;
class Triangle;
class XanimeResourceTable;

class JAISoundHandle;
namespace MR {
    JAISoundHandle* startSoundPlayerById(s32, s32, s32);
    JAISoundHandle* startLevelSoundPlayerById(s32, s32);
    bool isPlayerDefaultModel();
    void enablePlayerResourceYoshi();
    bool fn_80050020();
    bool isCreatePlayerResourceTornado();
    bool isPlayerPowerupTransforming();
    bool isPlayerInRushOrDemoOrTalk();
    void enablePlayerResourceFire();
    void killPlayerFireBalls();
    void forceCurePlayerElementMode();
    void enablePlayerResourceFreezeIce();
    bool isOnGroundPlayer();
    Triangle* getPlayerGroundingPolygon();
    void forceKillPlayerByAbyss();
    void forceKillPlayerByWaterOrGroundRace();
    void forceKillPlayerByGroundRace();

    bool isPlayerInBind();
    bool isPlayerDead();
    bool isPlayerRefuseTalk();
    bool isPlayerTeresaDisappear();
    bool isPlayerInAreaObj(const char*);
    TVec3f* getPlayerPos() NO_INLINE;
    TVec3f* getPlayerCenterPos();
    void getPlayerTakePos(TVec3f*);
    void setPlayerPos(const TVec3f&);
    void setPlayerUpperRotateY(f32);
    TVec3f* getPlayerRotate();
    TVec3f* getPlayerShadowRotate();
    void copyPlayerTR(LiveActor*);
    TVec3f* getPlayerVelocity();
    TVec3f* getPlayerLastMove();
    void setPlayerJumpVec(const TVec3f&);
    f32 getPlayerHitRadius();

    const JGeometry::TVec3<float>* getPlayerGravity();
    void calcPlayerSpinPullVelocity(TVec3f*, const TVec3f&);
    bool checkPlayerSwingTrigger();
    f32 calcDistanceToPlayer(const TVec3f&);
    bool isPlayerInRange(f32, const TVec3f&);
    void getPlayerUpVec(TVec3f*);
    void getPlayerFrontVec(TVec3f*);
    void getPlayerSideVec(TVec3f*);
    void getPlayerThrowVec(TVec3f*);
    void getPlayerGroundPos(TVec3f*);
    const JGeometry::TVec3<float>* getPlayerGroundNormal();
    void setPlayerFrontTargetVec(const TVec3f&, s32);
    void setPlayerFrontVec(const TVec3f&, s32);
    void setPlayerSwingInhibitTimer(u16);
    void setPlayerSwingPermission(bool);
    void setPlayerStateWait();
    void startBckPlayer(const char*, const char* = nullptr);
    void startBckPlayer(const char*, const BckCtrlData&);

    bool isBckStoppedPlayer();
    bool isBckOneTimeAndStoppedPlayer();
    f32 getBckFrameMaxPlayer();
    s16 getBckFrameMaxPlayer(const char*);
    void startBckPlayerJ(const char*);
    void becomeContinuousBckPlayer();
    void progressPlayerBckOnPause();
    const char* getPlayerCurrentBckName();
    void setBckBlendWeight(f32, f32);
    void setBckBlendWeight(f32, f32, f32);
    void setBckBlendWeight(f32, f32, f32, f32);
    void setBckRatePlayer(f32);

    XanimeResourceTable* getPlayerXanimeResource();
    void jumpPlayer(const TVec3f&);
    void forceJumpPlayer(const TVec3f&);
    void forceFlyPlayer(const TVec3f&, const TVec3f&, s32);
    void tornadoJumpPlayer();
    void miniTornadoJumpPlayer();

    void becomePlayerNormalJumpStatus();
    bool isOnPlayer(const HitSensor*);
    bool isActorOnPlayer(const LiveActor*);
    bool isOnPlayerShadow(const LiveActor*);
    void setPlayerPos(const char*) NO_INLINE;
    void setPlayerPosAndWait(const TVec3f&, bool);
    void setPlayerPosAndWait(const char*, bool);
    void setPlayerLinkPosAndWait(const NameObj*, const char*, bool);
    void setPlayerPosOnGround(const char*);
    void setPlayerPosOnGroundAndWait(const char*, bool);
    bool isPlayerHipDropFalling();
    bool isPlayerHipDropLand();
    s32 getPlayerLife();
    bool isPlayerLifeOne();

    void incPlayerLife(u32);
    void incPlayerOxygen(u32);
    bool isPlayerConfrontDeath();
    void getStarPiecePlayer();
    void notifyDirectGetStarPiecePlayer();
    void scatterStarPiecePlayer(u32);

    bool isPlayerSwingAction();

    bool isPlayerPointedBy2POnTriggerButton();
    bool isPlayerSquat();
    bool isPlayerInRush();
    bool isPlayerRecovering();
    bool isPlayerNeedBrakingCamera();

    bool isPlayerFlying();

    bool isPlayerHanging();
    bool isPlayerBury();

    bool isPlayerElementModeTornado();
    bool isPlayerElementModeInvincible();
    bool isPlayerElementModeBee();
    bool isPlayerElementModeHopper();
    bool isPlayerElementModeTeresa();
    bool isPlayerElementModeIce();
    bool isPlayerElementModeFire();
    bool isPlayerElementModeCloud();
    bool isPlayerElementModeRock();
    bool isPlayerElementModeNormal();
    bool isPlayerElementModeYoshi();
    bool isPlayerElementModeYoshiDash();
    bool isPlayerElementModeYoshiLight();
    void changePlayerModeBee();
    void changePlayerModeHopper();
    void changePlayerModeTeresa();
    void changePlayerModeIce();
    void changePlayerModeFire();
    void changePlayerModeYoshi();
    void changePlayerModeCloud();
    void changePlayerModeRock();
    void changePlayerModeFoo();
    void changePlayerModeInvincible();
    void changePlayerItemLifeUp();
    void curePlayerElementMode();
    void startDashYoshiMode();
    void startBalloonYoshiMode();

    // SMG2-only (original names unknown; named by address)
    void startPlayerNormalBva();
    bool fn_804225C0();

    bool isPlayerParalyzing();
    bool isPlayerDamaging();

    bool isPlayerStaggering();
    bool isPlayerSwimming();
    bool isPlayerSleeping();
    bool isPlayerJumpRising();
    void endBindAndPlayerWait(LiveActor*);
    void endBindAndPlayerJump(LiveActor*, const TVec3f&, u32);
    void endBindAndPlayerForceJump(LiveActor*, const TVec3f&, u32);
    void endBindAndPlayerWeakGravityJump(LiveActor*, const TVec3f&);
    void endBindAndPlayerForceWeakGravityJump(LiveActor*, const TVec3f&);
    void endBindAndPlayerForceWeakGravityJumpInputOff(LiveActor*, const TVec3f&);
    void endBindAndPlayerWeakGravityLimitJump(LiveActor*, const TVec3f&);
    void endBindAndSpinDriverJump(LiveActor*, const TVec3f&);
    void endBindAndPlayerDamage(LiveActor*, const TVec3f&);
    void endBindAndPlayerFlip(LiveActor*, const TVec3f&);
    void endBindAndPlayerJumpWithRollLanding(LiveActor*, const TVec3f&, u32);
    void endBindAndPlayerDamageMsg(LiveActor*, u32);
    void endBindAndPlayerDamageMsg(LiveActor*, u32, const TVec3f&);
    void endBindAndPlayerAcidDamage(LiveActor*);
    void endBindAndPlayerFreezeDamage(LiveActor*);
    void endBindAndPlayerFireDamage(LiveActor*);
    void endBindAndPlayerElectricDamage(LiveActor*);
    LiveActor* getCurrentRushActor();
    HitSensor* getCurrentRushSensor();
    void tryPlayerCoinPull();
    void tryPlayerPullActor(HitSensor*);
    void tryPlayerDropTakingActor();
    void tryPlayerKillTakingActor();
    bool isPlayerCarrySensorType(u32);
    bool isPlayerCarryAny();
    bool isPlayerLifeDecreasedThisFrame();
    LiveActor* getPlayerCarrySensorHost();

    void startSoundPlayer(const char*, s32, s32);
    void startLevelSoundPlayer(const char*, s32, s32);
    void startActionSoundPlayer(const char*, s32, s32);
    void startLevelSoundPlayer(const char*, s32);  // SMG2 fn_80050E10 (unnamed in symbols.txt)
    void startLevelSoundPlayerOrYoshi(const char* pPlayerSoundName, const char* pYoshiSoundName, s32 unk);

    void stopSoundPlayer(const char*, u32);
    void startSoundPlayerJ(const char*);
    void emitEffectPlayer(const char*);
    void deleteEffectPlayer(const char*);
    void showPlayer();
    void hidePlayer();

    void hidePlayerTryDeleteEffect();
    void showPlayerJoint(const char*);
    void hidePlayerJoint(const char*);

    void setPlayerSpot(f32, u32);
    void startPlayerDownWipe();
    void setCameraTargetToPlayer(CameraTargetArg*);
    bool isPlayerDisableFpView();
    bool isFpViewChangingFailure();
    void stopPlayerFpView();
    void startPlayerTalk(const LiveActor*);
    void endPlayerTalk();

    bool isCurrentRushActor(const char*);
    bool isCurrentRushRockMario();
    bool isCurrentRushTamakoro();
    bool isCurrentRushSpinDriver();
    bool isCurrentRushItemDrill();
    bool isExistMario();
    void startPlayerEvent(const char*);
    void offPlayerControl();
    void onPlayerControl(bool);
    bool isOffPlayerControl();
    void unlockPlayerAnimation();
    void resetPlayerStatus();
    void resetPlayerEffect();
    void setPlayerBaseMtx(MtxPtr);

    MtxPtr getPlayerBaseMtx();

    LiveActor* getPlayerDemoActor();

    void calcPlayerJointMtx(MtxPtr, const char*);
    void concatWithPlayerJointMtx(TPos3f*, const char*);
    void concatWithPlayerJointMtx(MtxPtr, const char*);

    void pushPlayer(const TVec3f&);
    void pushPlayerFromArea(const TVec3f&);

    bool isPlayerInWaterMode();
    bool isPlayerOnWaterSurface();
    bool isPlayerHidden();
    void calcPlayerWorldPadDir(TVec3f*, f32, f32);

    // getFullScreenBlurTexture();

    u16 getPlayerMovementTimer();
    HitSensor* getPlayerBodySensor();

    bool changePlayerAnimAndStartBvaIfExist(const char*);
    // PlayerUtil (enemies lane, callees named by address; lead renames symbols)
    void fn_800515E0();
    // PlayerUtil (enemies lane, callees named by address; lead renames symbols)
    void calcYoshiTongueTipMtx(TPos3f*);
    void fn_80422190();
    void fn_80422210();
    bool fn_80421D10();
};  // namespace MR

// Merged from Petari (core lane): declarations missing here
#include <JSystem/JGeometry/TMatrix.hpp>
class CubeCameraArea;
class JUTTexture;
class Triangle;

namespace MR {
    void forceKillPlayerByWaterRace();
    void forceKillPlayerByGhostRace();
    void setPlayerWalkingResist(f32);
    bool checkPlayerActionTrigger();
    void forceGameOverPlayerBySink();
    bool isPlayerElementMode(s32);
    bool isPlayerSkating();
    void changePlayerAnimationAndBva(const char*);
    void changePlayerItemStatus(s32);
    void validatePlayerSensor();
    bool isPlayerTakingActor(const char*);
    void setRasterScroll(s32, s32, s32);
    void noticePlayerDashChance();
    void preventPlayerRush();
    void initPlayerAfterOpeningDemo();
    void readyPlayerDemo();
    void onFollowDemoEffect();
    void requestMovementOnPlayer();
    bool isPlayerOnPress();
    CubeCameraArea* getCameraCube();
};  // namespace MR

// Merged from Petari (core lane): overloads missing here


namespace MR {
    void calcPlayerJointMtx(TPos3f*, const char*);
    JUTTexture* getFullScreenBlurTexture();
    void createPlayerFollowPass();
};  // namespace MR

namespace MR {
    void calcPlayerFollowPassPosition(TVec3f*, f32);
}

namespace MR {
    XanimeResourceTable* createPlayerXanimeResource(bool);
}

namespace MR {
    void startPlayerNormalBva();
}

namespace MR {
    void forceKillPlayer();
}

namespace MR { bool isPlayerVisible(); }

namespace MR {
    LiveActor* getPlayerCarrySensorActor();
    void preparePlayerForPositionReset();
}

namespace MR { bool fn_804222D0(); f32 getYoshiTongueScale(); }
namespace MR { void requestPlayerMovementOnDuringDemo(); }

namespace MR {
    // PLACEHOLDER(m1): clears Mario movement flag 0x20000000; resolve when Player owner recovers its meaning.
    void fn_8004F310();
    void extendPlayerBeeAirWalk(s32);
}

namespace MR { void enablePlayerResourceCloud(); }

namespace MR {
    // PLACEHOLDER(m1): semantic name unknown; resolve after Mario flags 0x00300000 are identified.
    void fn_8004FA20();
}

namespace MR {
    void startDemoBckPlayer(const char*, const BckCtrlData&);
    // original name unknown; named by behaviour: Player access, animation, model, and movement helpers (see PlayerUtil.cpp).
    bool startPlayerBvaWithCtrl(const char*, const BckCtrlData&);
    f32 getPlayerCeilingDistance();
    void calcYoshiEatItemVelocity(TVec3f*);
    void decrementPlayerSwimLife();
    bool isPlayerInJumpAction();
    bool isPlayerSwingActionStartNow();
    void enablePlayerSilhouetteDuringRush();
    void setPlayerSilhouetteHiddenFrames(s32);
    void clearPlayerFootprints();
    void disablePlayerReflection();
    void enablePlayerReflection();
    bool isPlayerDefaultModel();
    bool isPlayerFireDamageActive();
    void hidePlayerDeleteEffect();
    void setPlayerQuatAndPos(TQuat4f*, TVec3f*);
    void getPlayerQuatAndPos(TQuat4f*, TVec3f*);
    void calcPlayerKillDiffVec(TVec3f*, const TVec3f&, const TVec3f&, const TVec3f&);
    void calcPlayerKillVecOnPlane(TVec3f*, const TVec3f&, const TVec3f&, const TVec3f&, const TVec3f&);
    void calcPlayerScaledVec(TVec3f*, const TVec3f&, const TVec3f&, const TVec3f&, f32);
    const JUTTexture* getPlayerShadowTexture();
    void startSoundPlayer(JAISoundID, s32, s32);
    void startLevelSoundPlayer(JAISoundID, s32);
}

namespace MR {
    void fn_803B6830(bool);
}

namespace MR {
    void startPlayerBlink();
    void stopPlayerBlink();
    void pauseRescuedStrayTicoFollow();
    void launchRescuedStrayTicoFromMtx(MtxPtr);
}
