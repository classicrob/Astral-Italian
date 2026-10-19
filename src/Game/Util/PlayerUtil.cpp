#include "Game/Util/PlayerUtil.hpp"
#include "Game/Animation/XanimeResource.hpp"
#include "Game/Util.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/MapObj/StarPieceDirector.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioAccess.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/PlayerEvent.hpp"
#include "Game/Player/RushEndInfo.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Animation/BckCtrl.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/MapObj/SwingActionWatcher.hpp"
#include "Game/Player/Yoshi.hpp"
#include "JSystem/JAudio2/JAISound.hpp"
#include "JSystem/JGeometry/TUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

extern XanimeGroupInfo playerGhostAnimeGroups[];
extern XanimeOfsInfo playerGhostAnimeOffsets[];
extern XanimeBckTable1 playerGhostSingleAnimes[];
extern XanimeBckTable2 playerGhostDoubleAnimes[];
extern XanimeBckTable3 playerGhostTripleAnimes[];
extern XanimeBckTable4 playerGhostQuadAnimes[];
extern XanimeSwapTable playerGhostLuigiSwap[];

namespace MR {
    bool isOnGroundPlayer() {
        return MarioAccess::isOnGround(0);
    }

    Triangle* getPlayerGroundingPolygon() {
        return MarioAccess::getGroundingPolygon(0);
    }

    void forceKillPlayerByAbyss() {
        return MarioAccess::forceKill(0, 0);
    }


    void forceKillPlayerByGroundRace() {
        return MarioAccess::forceKill(5, 0);
    }

    void forceGameOverPlayerBySink() {
        MarioAccess::forceGameOverSink();
    }


    bool isPlayerDead() {
        return !MarioAccess::getPlayerActor()->isEnableNerveChange();
    }

    bool isPlayerRefuseTalk() {
        return MarioAccess::getPlayerActor()->isRefuseTalk();
    }

    bool isPlayerTeresaDisappear() {
        return MarioAccess::isTeresaDisappear();
    }

    bool isPlayerInAreaObj(const char* pName) {
        return isInAreaObj(pName, *getPlayerPos());
    }

    TVec3f* getPlayerPos() {
        if (!MarioAccess::isVisible() && isCurrentRushActor("岩マリオ")) {
            return &getCurrentRushActor()->mPosition;
        }

        return &MarioAccess::getPlayerActor()->mPosition;
    }

    TVec3f* getPlayerCenterPos() {
        return &MarioAccess::getPlayerActor()->_240;
    }

    void getPlayerTakePos(TVec3f* pPos) {
        return MarioAccess::getTakePos(pPos);
    }

    void setPlayerPos(const TVec3f& rPos) {
        MarioAccess::setTrans(rPos, 0);
    }

    void setPlayerUpperRotateY(f32 angle) {
        MarioAccess::getPlayerActor()->setUpperRotateY(angle);
    }

    TVec3f* getPlayerRotate() {
        return &MarioAccess::getPlayerActor()->mRotation;
    }

    TVec3f* getPlayerShadowRotate() {
        return &MarioAccess::getPlayerActor()->_A18;
    }

    TVec3f* getPlayerVelocity() {
        return MarioAccess::getVelocity();
    }

    TVec3f* getPlayerLastMove() {
        return MarioAccess::getLastMove();
    }

    void setPlayerJumpVec(const TVec3f& rJumpVec) {
        MarioAccess::setJumpVec(rJumpVec);
    }

    f32 getPlayerHitRadius() {
        return MarioAccess::getPlayerActor()->mPlayerMode == (s16)6 ? 100.0f : 60.0f;
    }

    // Yoshi-eat item launch velocity (MarioAccess::calcYoshiEatItemVelocity -> fn_8041BE00: velocity * 2 - gravity * 40); caller ActorItemGenerator
    void calcYoshiEatItemVelocity(TVec3f* pOut) {
        MarioAccess::calcYoshiEatItemVelocity(pOut);
    }

    // original name unknown; named by behaviour: returns the cached ceiling distance, or a fixed value during confront-death or disabled nerve changes.
    f32 getPlayerCeilingDistance() {
        return MarioAccess::getCeilingDistance();
    }

    void clearPlayerMovementFlag20000000() {
        MarioAccess::fn_803B72A0();
    }


    const TVec3f* getPlayerGravity() {
        return &MarioAccess::getPlayerActor()->getGravityVec();
    }

    void calcPlayerSpinPullVelocity(TVec3f* pVelocity, const TVec3f& rPos) {
        MarioAccess::calcSpinPullVelocity(pVelocity, rPos);
    }


    bool checkPlayerSwingTrigger() {
        return MarioAccess::getPlayerActor()->_1E1;
    }

    f32 calcDistanceToPlayer(const TVec3f& rPos) {
        return getPlayerPos()->distance(rPos);
    }

    void getPlayerUpVec(TVec3f* pVec) {
        MarioAccess::getPlayerActor()->getUpVec(pVec);
    }

    void getPlayerFrontVec(TVec3f* pVec) {
        MarioAccess::getPlayerActor()->getFrontVec(pVec);
    }

    void getPlayerSideVec(TVec3f* pVec) {
        MarioAccess::getPlayerActor()->getSideVec(pVec);
    }

    void getPlayerThrowVec(TVec3f* pVec) {
        MarioAccess::getThrowVec(pVec);
    }

    void getPlayerGroundPos(TVec3f* pPos) {
        MarioAccess::getPlayerActor()->getGroundPos(pPos);
    }

    const TVec3f* getPlayerGroundNormal() {
        return getPlayerGroundingPolygon()->getFaceNormal();
    }

    void setPlayerFrontTargetVec(const TVec3f& rTarget, s32 param2) {
        MarioAccess::setFrontVecTarget(rTarget, param2);
    }

    void setPlayerFrontVec(const TVec3f& rVec, s32 param2) {
        MarioAccess::setFrontVecKeepUp(rVec, param2);
    }

    void setPlayerSwingInhibitTimer(u16 timer) {
        MarioAccess::getPlayerActor()->_EF6 = timer;
    }

    void setPlayerSwingPermission(bool permission) {
        MarioAccess::getPlayerActor()->_EEB = permission;
    }

    void setPlayerStateWait() {
        MarioAccess::setStateWait();
    }

    void startBckPlayer(const char* pName, const char* pParam2) {
        if (pParam2 != nullptr) {
            MarioAccess::changeAnimationE(pName, pParam2);
        } else {
            MarioAccess::changeAnimationE(pName, pName);
        }
    }

    void startBckPlayer(const char* pName, const BckCtrlData& rCtrlData) {
        MarioAccess::changeAnimationE(pName, rCtrlData);
    }

    bool startPlayerBvaWithCtrl(const char* pName, const BckCtrlData& rCtrlData) {
        if (!startBvaIfExist(MarioAccess::getPlayerActor(), pName)) {
            return false;
        }

        BckCtrlFunction::reflectBckCtrlDataToFrameCtrl(rCtrlData, getBvaCtrl(MarioAccess::getPlayerActor()));
        return true;
    }

    bool isBckStoppedPlayer() {
        return MR::isActionEnd(MarioAccess::getPlayerOrYoshiActor());
    }

    bool isBckOneTimeAndStoppedPlayer() {
        return MR::isBckOneTimeAndStopped(MarioAccess::getPlayerOrYoshiActor());
    }


    s16 getBckFrameMaxPlayer(const char* pName) {
        return MR::getBckFrameMax(MarioAccess::getPlayerActor(), pName);
    }

    void startBckPlayerJ(const char* pName) {
        MarioAccess::changeAnimationJ(pName);
    }

    void becomeContinuousBckPlayer() {
        MarioAccess::keepCurrentAnimation();
    }

    void progressPlayerBckOnPause() {
        MarioAccess::progressAnimation();
    }

    const char* getPlayerCurrentBckName() {
        return MarioAccess::getCurrentBckName();
    }

    void setBckBlendWeight(f32 param1, f32 param2) {
        const f32 blendWeight[4] = {param1, param2, 0.0f, 0.0f};

        MarioAccess::setAnimationBlendWeight(blendWeight);
    }

    void setBckBlendWeight(f32 param1, f32 param2, f32 param3) {
        const f32 blendWeight[4] = {param1, param2, param3, 0.0f};

        MarioAccess::setAnimationBlendWeight(blendWeight);
    }

    void setBckBlendWeight(f32 param1, f32 param2, f32 param3, f32 param4) {
        const f32 blendWeight[] = {param1, param2, param3, param4};

        MarioAccess::setAnimationBlendWeight(blendWeight);
    }

    void setBckRatePlayer(f32 rate) {
        MR::setBckRate(MarioAccess::getPlayerActor(), rate);
    }

    XanimeResourceTable* getPlayerXanimeResource() {
        return MarioAccess::getPlayerActor()->getResourceTable();
    }

    // NONMATCHING: external animation-table address materialization; target table identities verified
    XanimeResourceTable* createPlayerXanimeResource(bool isLuigi) {
        XanimeSwapTable* swap = nullptr;
        if (isLuigi) swap = playerGhostLuigiSwap;
        return new XanimeResourceTable(MR::getResourceHolder(MarioAccess::getPlayerActor()), playerGhostAnimeGroups,
                                      playerGhostAnimeOffsets, playerGhostSingleAnimes, playerGhostDoubleAnimes,
                                      playerGhostTripleAnimes, playerGhostQuadAnimes, swap);
    }

    void jumpPlayer(const TVec3f& rParam1) {
        MarioAccess::freeJump(rParam1, 0);
    }

    void forceJumpPlayer(const TVec3f& rParam1) {
        MarioAccess::forceJump(rParam1, 0);
    }

    void forceFlyPlayer(const TVec3f& rParam1, const TVec3f& rParam2, s32 param3) {
        MarioAccess::forceFly(rParam1, rParam2, param3);
    }

    void tornadoJumpPlayer() {
        MarioAccess::tornadoJump();
    }

    void miniTornadoJumpPlayer() {
        MarioAccess::tornadoJumpMini();
    }

    void clearPlayerStateFlags00300000() {
        MarioAccess::fn_803B51B0();
    }

    void becomePlayerNormalJumpStatus() {
        MarioAccess::becomeNormalJumpStatus();
    }

    bool isOnPlayer(const HitSensor* pSensor) {
        if (MarioAccess::isInRush()) {
            if (isBindedGround(getCurrentRushActor())) {
                return getGroundSensor(getCurrentRushActor()) == pSensor;
            }

            return false;
        }

        Triangle* polygon = MarioAccess::getGroundingPolygon(0);
        if (polygon != nullptr) {
            return polygon->mSensor == pSensor;
        }

        return false;
    }

    bool isActorOnPlayer(const LiveActor* pActor) {
        if (MarioAccess::isInRush()) {
            if (isBindedGround(getCurrentRushActor())) {
                return getGroundActor(getCurrentRushActor()) == pActor;
            }

            return false;
        }

        return MarioAccess::isOnActor(pActor);
    }

    bool isOnPlayerShadow(const LiveActor* pActor) {
        Triangle* polygon = MarioAccess::getShadowingPolygon();

        if (polygon != nullptr) {
            return polygon->mSensor->mHost == pActor;
        }

        return false;
    }


    void setPlayerPos(const char* pParam1) {
        TPos3f mtx;
        mtx.identity();

        MR::findNamePos(pParam1, mtx.toMtxPtr());
        MarioAccess::setBaseMtx(mtx.toMtxPtr());

        TVec3f trans;
        MR::extractMtxTrans(mtx.toMtxPtr(), &trans);

        setPlayerPos(trans);
    }

    void setPlayerPosAndWait(const TVec3f& rPos, bool resetBeforePosition) {
        if (resetBeforePosition) MarioAccess::prepareForPositionReset();
        MarioAccess::setTrans(rPos, 0);
        MarioAccess::getPlayerActor()->resetCondition();
        MarioAccess::setStateWait();
        startBckPlayer("Wait", static_cast<const char*>(nullptr));
    }

    void setPlayerPosAndWait(const char* pName, bool resetBeforePosition) {
        if (resetBeforePosition) MarioAccess::prepareForPositionReset();
        setPlayerPos(pName);
        MarioAccess::getPlayerActor()->resetCondition();
        MarioAccess::setStateWait();
        startBckPlayer("Wait", static_cast<const char*>(nullptr));
    }

    void setPlayerLinkPosAndWait(const NameObj* pParam1, const char* pParam2, bool resetBeforePosition) {
        if (resetBeforePosition) MarioAccess::prepareForPositionReset();
        TPos3f mtx;
        mtx.identity();

        MR::findLinkNamePos(pParam1, pParam2, mtx.toMtxPtr());
        MarioAccess::setBaseMtx(mtx.toMtxPtr());
        MarioAccess::getPlayerActor()->resetCondition();
        MarioAccess::setStateWait();
        startBckPlayer("Wait", static_cast<const char*>(nullptr));
    }

    void setPlayerPosOnGround(const char* pParam1) {
        TPos3f mtx;
        mtx.identity();

        MR::findNamePosOnGround(pParam1, mtx.toMtxPtr());
        MarioAccess::setBaseMtx(mtx.toMtxPtr());
    }

    void setPlayerPosOnGroundAndWait(const char* pParam1, bool resetBeforePosition) {
        if (resetBeforePosition) MarioAccess::prepareForPositionReset();
        TPos3f mtx;
        mtx.identity();

        MR::findNamePosOnGround(pParam1, mtx.toMtxPtr());
        MarioAccess::setBaseMtx(mtx.toMtxPtr());
        MarioAccess::getPlayerActor()->resetCondition();
        MarioAccess::setStateWait();
        MR::startBckPlayer("Wait",static_cast<const char*>(nullptr));
    }

    bool isPlayerHipDropFalling() {
        return MarioAccess::isHipDropFalling();
    }

    bool isPlayerHipDropLand() {
        return MarioAccess::isHipDropLand();
    }

    s32 getPlayerLife() {
        return MarioAccess::getLife();
    }

    bool isPlayerLifeOne() {
        return MarioAccess::getLife() == 1;
    }

    // original name unknown; named by behaviour: decrements swim life, otherwise requests a life decrement with timer 20.
    void decrementPlayerSwimLife() {
        MarioAccess::decrementLife();
    }

    void incPlayerLife(u32 add) {
        MarioAccess::incLife(add);
    }

    void incPlayerOxygen(u32 add) {
        MarioAccess::incOxygen(add);
    }

    bool isPlayerConfrontDeath() {
        return MarioAccess::isConfrontDeath();
    }

    void getStarPiecePlayer() {
        MarioAccess::addStarPiece();
    }

    void notifyDirectGetStarPiecePlayer() {
        MarioAccess::getStarPieceDirect();
    }

    void scatterStarPiecePlayer(u32 num) {
        MarioAccess::scatterStarPiece(num);
    }

    void extendPlayerBeeAirWalk(s32 param1) {
        MarioAccess::extendBeeAirWalk(param1);
    }

    bool isPlayerSwingAction() {
        return MarioAccess::isSwingAction();
    }

    // original name unknown; named by behaviour: tests jumping and Mario jump action 5.
    bool isPlayerInJumpAction() {
        return MarioAccess::isInJumpAction();
    }

    // Separate SMG2 entrypoint that forwards to the same swing-start predicate.
    bool isPlayerSwingActionStartNow() {
        return isPlayerSwingActionStart();
    }

    bool isPlayerPointedBy2POnTriggerButton() {
        return MR::isStarPointerPointing2POnTriggerButton(MarioAccess::getPlayerActor(), "弱", true, false);
    }

    bool isPlayerSquat() {
        return MarioAccess::isSquat();
    }

    bool isPlayerInRush() {
        return MarioAccess::isInRush();
    }

    bool isPlayerNeedBrakingCamera() {
        return MarioAccess::isNeedBrakingCamera();
    }

    // original name unknown; named by behaviour: sets the rush silhouette override and clears its hidden-frame timer.
    void enablePlayerSilhouetteDuringRush() {
        MarioAccess::enableSilhouetteDuringRush();
    }

    // original name unknown; named by behaviour: sets the countdown that suppresses silhouette visibility.
    void setPlayerSilhouetteHiddenFrames(s32 pParam1) {
        MarioAccess::setSilhouetteHiddenFrames(pParam1);
    }

    // original name unknown; named by behaviour: clears the actor footprint emitter if present.
    void clearPlayerFootprints() {
        MarioAccess::clearFootprints();
    }

    // original name unknown; named by behaviour: sets the flag that skips reflection drawing and view updates.
    void disablePlayerReflection() {
        MarioAccess::disableReflection();
    }

    // original name unknown; named by behaviour: clears the flag that skips reflection drawing and view updates.
    void enablePlayerReflection() {
        MarioAccess::enableReflection();
    }

    bool isPlayerFlying() {
        return MarioAccess::isFlying();
    }


    bool isPlayerElementModeTornado() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 9;
    }

    bool isPlayerElementModeInvincible() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 1;
    }

    bool isPlayerElementModeBee() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 4;
    }

    bool isPlayerElementModeHopper() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 5;
    }

    bool isPlayerElementModeTeresa() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 6;
    }

    bool isPlayerElementModeIce() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 3;
    }

    bool isPlayerElementModeNormal() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 0;
    }



    void curePlayerElementMode() {
        MarioAccess::changeItemStatus(8);
    }

    void forceCurePlayerElementMode() {
        MarioAccess::changeItemStatus(11);
    }

    bool isPlayerParalyzing() {
        return MarioAccess::isParalyzing();
    }

    bool isPlayerDamaging() {
        return MarioAccess::getPlayerActor()->isDamaging();
    }

    // original name unknown; named by behaviour: tests Mario flag 0x80000000 or fire states 8 through 10.
    bool isPlayerFireDamageActive() {
        return MarioAccess::getPlayerActor()->isFireDamageActive();
    }

    bool isPlayerStaggering() {
        return MarioAccess::getPlayerActor()->isStaggering();
    }

    bool isPlayerSwimming() {
        return MarioAccess::isSwimming();
    }

    bool isPlayerSleeping() {
        return MarioAccess::getPlayerActor()->isSleeping();
    }

    bool isPlayerJumpRising() {
        return MarioAccess::getPlayerActor()->isJumpRising();
    }



    void endBindAndPlayerWait(LiveActor* pActor) {
        TVec3f velocity(0.0f, 0.0f, 0.0f);
        RushEndInfo info(pActor, 0, velocity, false, 0);

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerJump(LiveActor* pActor, const TVec3f& rParam2, u32 param3) {
        RushEndInfo info = RushEndInfo(pActor, 2, rParam2, true, param3);

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerForceJump(LiveActor* pActor, const TVec3f& rParam2, u32 param3) {
        RushEndInfo info = RushEndInfo(pActor, 2, rParam2, true, param3);
        info._20 |= 0x40000000;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerWeakGravityJump(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerForceWeakGravityJump(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0x40000000;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerForceWeakGravityJumpInputOff(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0xC0000000;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerWeakGravityLimitJump(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0x800000;

        MarioAccess::endRush(&info);
    }

    void endBindAndSpinDriverJump(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0xC0000000;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerDamage(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 1;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerFlip(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 6;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerJumpWithRollLanding(LiveActor* pActor, const TVec3f& rParam2, u32 param3) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, param3);
        info._20 |= 0x400000;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerDamageMsg(LiveActor* pActor, u32 param2) {
        endBindAndPlayerDamageMsg(pActor, param2, TVec3f(0.0f, 0.0f, 0.0f));
    }

    void endBindAndPlayerDamageMsg(LiveActor* pActor, u32 param2, const TVec3f& rParam3) {
        switch (param2) {
        case 0x65:
        case 0x66:
        case 0x67:
            endBindAndPlayerFireDamage(pActor);
            break;
        case 0x6A:
            endBindAndPlayerAcidDamage(pActor);
            break;
        case 0x6B:
            endBindAndPlayerFreezeDamage(pActor);
            break;
        case 0x68:
        case 0x69:
            endBindAndPlayerElectricDamage(pActor);
            break;
        case 0x64:
            endBindAndPlayerDamage(pActor, rParam3);
            break;
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5F:
        case 0x60:
            endBindAndPlayerWeakGravityLimitJump(pActor, rParam3);
            break;
        case 0x5E:
            endBindAndPlayerFlip(pActor, rParam3);
            break;
        default:
            endBindAndPlayerDamage(pActor, rParam3);
            break;
        }
    }

    void endBindAndPlayerAcidDamage(LiveActor* pActor) {
        RushEndInfo info = RushEndInfo(pActor, 3, TVec3f(0.0f, 0.0f, 0.0f), true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 4;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerFreezeDamage(LiveActor* pActor) {
        RushEndInfo info = RushEndInfo(pActor, 3, TVec3f(0.0f, 0.0f, 0.0f), true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 3;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerFireDamage(LiveActor* pActor) {
        RushEndInfo info = RushEndInfo(pActor, 3, TVec3f(0.0f, 0.0f, 0.0f), true, 0);
        info._20 |= 0x40000000;
        info.mFlags.mDamageType = 2;

        MarioAccess::endRush(&info);
    }

    void endBindAndPlayerElectricDamage(LiveActor* pActor) {
        RushEndInfo info = RushEndInfo(pActor, 3, TVec3f(0.0f, 0.0f, 0.0f), true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 5;

        MarioAccess::endRush(&info);
    }

    LiveActor* getCurrentRushActor() {
        if (isPlayerInRush() == false) {
            return nullptr;
        }

        return MarioAccess::getPlayerActor()->_924->mHost;
    }

    HitSensor* getCurrentRushSensor() {
        if (isPlayerInRush() == false) {
            return nullptr;
        }

        return MarioAccess::getPlayerActor()->_924;
    }

    void tryPlayerCoinPull() {
        MarioAccess::tryCoinPull();
    }

    void tryPlayerPullActor(HitSensor* pSensor) {
        MarioAccess::getPlayerActor()->tryTornadoPull(pSensor);
    }

    void tryPlayerDropTakingActor() {
        MarioAccess::dropTakingActor();
    }

    void tryPlayerKillTakingActor() {
        MarioAccess::killTakingActor();
    }

    bool isPlayerCarrySensorType(u32 type) {
        if (MarioAccess::getTakingSensor() == nullptr) return false;
        if (type == 0) return true;
        return MarioAccess::getTakingSensor()->isType(type);
    }

    bool isPlayerLifeDecreasedThisFrame() {
        return MarioAccess::getPlayerActor()->mLifeDecreasedThisFrame;
    }

    bool isPlayerCarryAny() {
        return MarioAccess::getPlayerActor()->_468 != 0;
    }

    LiveActor* getPlayerCarrySensorActor() {
        HitSensor* pSensor = MarioAccess::getTakingSensor();
        if (pSensor == nullptr) {
            return nullptr;
        }

        return getSensorHost(pSensor);
    }

    void startSoundPlayer(const char* pName, s32 param1, s32 param2) {
        MR::startSound(MarioAccess::getPlayerActor(), pName, param1, param2);
    }

    void startSoundPlayer(JAISoundID id, s32 param2, s32 param3) {
        MR::startSound(MarioAccess::getPlayerActor(), id, param2, param3);
    }

    JAISoundHandle* startSoundPlayerById(s32 id, s32 pitch, s32 velocity) {
        return MR::startSound(MarioAccess::getPlayerActor(), JAISoundID(id), pitch, velocity);
    }

    void startLevelSoundPlayer(const char* pName, s32 param1) {
        MR::startLevelSound(MarioAccess::getPlayerActor(), pName, param1);
    }

    JAISoundHandle* startLevelSoundPlayerById(s32 id, s32 pitch) {
        return MR::startLevelSound(MarioAccess::getPlayerActor(), JAISoundID(id), pitch, -1, -1);
    }

    void stopSoundPlayer(const char* pName, u32 param1) {
        MR::stopSound(MarioAccess::getPlayerActor(), pName, param1);
    }

    void startSoundPlayerJ(const char* pName) {
        MarioAccess::getPlayerActor()->playSound(pName, -1);
    }

    void showPlayer() {
        MarioAccess::show();
    }

    void hidePlayer() {
        MarioAccess::hide(true);
    }

    void hidePlayerDeleteEffect() {
        MarioAccess::hide(true);
    }

    void showPlayerJoint(const char* pName) {
        MR::showJoint(MarioAccess::getPlayerActor(), pName);
    }

    void hidePlayerJoint(const char* pName) {
        MR::hideJoint(MarioAccess::getPlayerActor(), pName);
    }

    bool isPlayerVisible() {
        return MarioAccess::isVisible();
    }

    void setPlayerSpot(f32 param1, u32 param2) {
        MarioAccess::setSpot(param1, param2);
    }

    void startPlayerDownWipe() {
        MarioAccess::startDownWipe();
    }

    void setCameraTargetToPlayer(CameraTargetArg* pTarget) {
        pTarget->mMarioActor = MarioAccess::getPlayerActor();
        pTarget->mTargetObj = nullptr;
        pTarget->mTargetMtx = nullptr;
        pTarget->mLiveActor = nullptr;
    }

    bool isPlayerDisableFpView() {
        return MarioAccess::isDisableFpView();
    }

    bool isFpViewChangingFailure() {
        return MarioAccess::isFpViewChangingFailure();
    }

    void stopPlayerFpView() {
        return MarioAccess::stopFpView();
    }



    void startPlayerTalk(const LiveActor* pActor) {
        MarioAccess::startTalk(pActor);
    }

    void endPlayerTalk() {
        MarioAccess::endTalk();
    }

    void preparePlayerForPositionReset() {
        MarioAccess::prepareForPositionReset();
    }


    bool isExistMario() {
        return MR::isExistSceneObj(SceneObj_MarioHolder);
    }

    void startPlayerEvent(const char* pName) {
        EventSequencer* eventSequencer;

        eventSequencer = MR::getSceneObj< EventSequencer >(SceneObj_EventSequencer);
        eventSequencer->startEvent(pName);

        requestMovementOn(eventSequencer);
        requestMovementOnPlayer();
    }

    void offPlayerControl() {
        MarioAccess::offControl();
        offSupportTicoControl();
    }

    void onPlayerControl(bool param1) {
        MarioAccess::onControl(param1);
        onSupportTicoControl();
    }

    bool isOffPlayerControl() {
        return MarioAccess::isOffControl();
    }

    void unlockPlayerAnimation() {
        MarioAccess::getPlayerActor()->setB90(false);
    }

    void resetPlayerStatus() {
        MarioAccess::getPlayerActor()->resetCondition();
    }

    void resetPlayerEffect() {
        MR::forceDeleteEffectAll(MarioAccess::getPlayerActor());
        resetChasingStarPiece();
    }

    void setPlayerBaseMtx(MtxPtr pMtx) {
        MarioAccess::setBaseMtx(pMtx);
    }

    // NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
    void setPlayerQuatAndPos(TQuat4f* pQuat, TVec3f* pPos) {
        TPos3f mtx;
        f32 y2 = 2.0f * pQuat->y;
        f32 x2 = 2.0f * pQuat->x;
        f32 z2 = 2.0f * pQuat->z;
        f32 yy = y2 * pQuat->y;
        f32 xx = x2 * pQuat->x;
        f32 w2 = 2.0f * pQuat->w;
        mtx.setTrans(*pPos);
        f32 zz = z2 * pQuat->z;
        mtx.mMtx[1][1] = 1.0f - xx - zz;
        mtx.mMtx[0][0] = 1.0f - yy - zz;
        f32 xy = x2 * pQuat->y;
        f32 xz = x2 * pQuat->z;
        f32 wz = w2 * pQuat->z;
        mtx.mMtx[2][2] = 1.0f - xx - yy;
        f32 wy = w2 * pQuat->y;
        mtx.mMtx[0][1] = xy - wz;
        mtx.mMtx[0][2] = xz + wy;
        mtx.mMtx[1][0] = xy + wz;
        mtx.mMtx[2][0] = xz - wy;
        f32 yz = y2 * pQuat->z;
        f32 wx = w2 * pQuat->x;
        mtx.mMtx[1][2] = yz - wx;
        mtx.mMtx[2][1] = yz + wx;
        MarioAccess::setBaseMtx(mtx.toMtxPtr());
    }

    MtxPtr getPlayerBaseMtx() {
        return MarioAccess::getBaseMtx();
    }

    void getPlayerQuatAndPos(TQuat4f* pQuat, TVec3f* pPos) {
        TPos3f mtx;
        mtx.set(MarioAccess::getBaseMtx());
        mtx.getQuat(*pQuat);
        pPos->set(*getPlayerPos());
    }

    LiveActor* getPlayerDemoActor() {
        return MarioAccess::getPlayerActor();
    }

    void requestMovementOnPlayer() {
        MarioAccess::getPlayerActor()->requestMovementOn();
    }

    void requestPlayerMovementOnDuringDemo() {
        MarioAccess::getPlayerActor()->requestHairAndHatMovementOn();
    }

    void killPlayerFireBalls() {
        MarioAccess::killAllFireBall();
    }





    void fn_800515E0() {
        MarioAccess::tryPressAgainstCeiling(30);
    }

    void calcPlayerJointMtx(TPos3f* pMtx, const char* pName) {
        pMtx->set(MarioAccess::getJointMtx(pName));
    }

    void concatWithPlayerJointMtx(TPos3f* pMtx, const char* pName) {
        MarioAccess::concatWithJointMtx(pMtx, pName);
    }


    void pushPlayer(const TVec3f& rAccel) {
        MarioAccess::addVelocity(rAccel);
    }

    void pushPlayerFromArea(const TVec3f& rAccel) {
        MarioAccess::addVelocityFromArea(rAccel);
    }

    void noticePlayerDashChance() {
        MarioAccess::noticeDashChance();
    }

    bool isPlayerInWaterMode() {
        return MarioAccess::isInWaterMode();
    }

    bool isPlayerOnWaterSurface() {
        return MarioAccess::isOnWaterSurface();
    }

    bool isPlayerHidden() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 6;
    }

    void calcPlayerWorldPadDir(TVec3f* pDir, f32 param2, f32 param3) {
        MarioAccess::calcWorldPadDir(pDir, param2, param3);
    }

    void calcPlayerKillDiffVec(TVec3f* pDst, const TVec3f& rA, const TVec3f& rB, const TVec3f& rUp) {
        pDst->set(rA - rB);
        vecKillElement(*pDst, rUp, pDst);
        normalizeOrZero(pDst);
    }

    void calcPlayerKillVecOnPlane(TVec3f* pDst, const TVec3f& rVec, const TVec3f& rA, const TVec3f& rB, const TVec3f& rNormal) {
        TVec3f tmp;
        tmp.set(rA - rB);
        vecKillElement(tmp, rNormal, &tmp);
        normalizeOrZero(&tmp);
        vecKillElement(rVec, tmp, pDst);
    }

    // Keep the normal displacement; clamp the planar radius to scale - 200.
    // NONMATCHING: register/stack or instruction scheduling (FP order/grouping proven by default rdclass a).
    void calcPlayerScaledVec(TVec3f* pDst, const TVec3f& rA, const TVec3f& rB, const TVec3f& rC, f32 scale) {
        TVec3f tmp = rA - rB;
        f32 normalDistance = vecKillElement(tmp, rC, &tmp);
        tmp.setLength(scale - 200.0f);

        TVec3f add(rC);
        add.scale(normalDistance);
        tmp += add;

        TVec3f result(rB);
        result += tmp;
        pDst->set(result);
    }

    JUTTexture* getFullScreenBlurTexture() {
        return MarioAccess::getPlayerActor()->_B7C;
    }

    // Mario's shadow texture (CollisionShadow+0x308; DisplayListMaker draws it).
    const JUTTexture* getPlayerShadowTexture() {
        return MarioAccess::getShadowTexture();
    }

    u16 getPlayerMovementTimer() {
        return MarioAccess::getPlayerActor()->_378;
    }

    void forceKillPlayer() {
        if (MarioAccess::isInWaterMode()) {
            MarioAccess::forceKill(1, 0);
        } else {
            MarioAccess::forceKill(5, 0);
        }
    }

    f32 getBckFrameMaxPlayer() {
        return MarioAccess::getAnimationFrameMax();
    }

    bool isPlayerSkating() {
        return MarioAccess::isSkating();
    }

    bool isPlayerPowerupTransforming() {
        return MarioAccess::isPowerupTransforming();
    }

    // original name unknown; named by behaviour: compares the selected Mario model with MarioAllModel::getJ3DModel().
    bool isPlayerDefaultModel() {
        return MarioAccess::isDefaultModel();
    }

    bool isPlayerRecovering() {
        return MarioAccess::isRecovering();
    }

    bool isPlayerHanging() {
        return MarioAccess::isHanging();
    }

    bool isPlayerBury() {
        return MarioAccess::isBury();
    }

    bool fn_80050020() {
        return MarioAccess::isRushingWithRestrictedYoshiJointControl();
    }

    void changePlayerModeBee() {
        MarioAccess::changeItemStatus(2);
    }

    void changePlayerModeHopper() {
        MarioAccess::changeItemStatus(1);
    }

    void changePlayerModeTeresa() {
        MarioAccess::changeItemStatus(3);
    }

    void changePlayerModeIce() {
        MarioAccess::changeItemStatus(4);
    }

    void changePlayerModeFire() {
        MarioAccess::changeItemStatus(5);
    }

    void changePlayerModeYoshi() {
        MarioAccess::changePlayerModeYoshi();
    }

    void changePlayerModeCloud() {
        MarioAccess::changePlayerModeCloud();
    }

    void changePlayerModeRock() {
        MarioAccess::changePlayerModeRock();
    }

    void changePlayerModeFoo() {
        MarioAccess::changeItemStatus(6);
    }

    void changePlayerModeInvincible() {
        MarioAccess::changeItemStatus(7);
    }

    void changePlayerItemLifeUp() {
        MarioAccess::changeItemStatus(10);
    }

    void hidePlayerTryDeleteEffect() {
        MarioAccess::hide(false);
    }

    void emitEffectPlayer(const char* pName) {
        MarioAccess::getPlayerActor()->playEffect(pName);
    }

    void deleteEffectPlayer(const char* pName) {
        MarioAccess::getPlayerActor()->stopEffect(pName);
    }

    bool isCurrentRushActor(const char* pName) {
        LiveActor* pRushActor = getCurrentRushActor();
        if (pRushActor != nullptr) {
            return isEqualString(pRushActor->getName(), pName);
        } else {
            return false;
        }
    }

    bool isCurrentRushRockMario() {
        return isCurrentRushActor("岩マリオ");
    }

    bool isCurrentRushTamakoro() {
        return isCurrentRushActor("たまころ");
    }

    // NONMATCHING: literal pool sharing (target lis/addi per literal, ours shares one base)
    bool isCurrentRushSpinDriver() {
        bool result = false;
        if (isCurrentRushActor("スーパースピンドライバー") || isCurrentRushActor("スピンドライバ") || isCurrentRushActor("ピンクスーパースピンドライバー")) {
            result = true;
        }

        return result;
    }

    bool isCurrentRushItemDrill() {
        return isCurrentRushActor("アイテムドリル");
    }

    HitSensor* getPlayerBodySensor() {
        return MarioAccess::getPlayerActor()->getSensor("Body");
    }

    void copyPlayerTR(LiveActor* pActor) {
        setPosition(pActor, *getPlayerPos());
        setRotation(pActor, MarioAccess::getPlayerActor()->mRotation);
    }

    void changePlayerAnimationAndBva(const char* pName) {
        MarioAccess::changeAnimationE(pName, pName);
        startBvaIfExist(MarioAccess::getPlayerActor(), pName);
    }

    void startDemoBckPlayer(const char* pName, const BckCtrlData& rCtrlData) {
        MarioAccess::changeAnimationE(pName, rCtrlData);
        startPlayerBvaWithCtrl(pName, rCtrlData);
    }

    void startPlayerNormalBva() {
        startBva(MarioAccess::getPlayerActor(), "Normal");
    }

    void startLevelSoundPlayer(const char* pName, s32 pitch, s32 volume) {
        startActionSound(MarioAccess::getPlayerActor(), pName, pitch, volume, -1);
    }

    void startLevelSoundPlayer(JAISoundID id, s32 param2) {
        MR::startLevelSound(MarioAccess::getPlayerActor(), id, param2, -1, -1);
    }

    void startActionSoundPlayer(const char* pName, s32 param2, s32 param3) {
        startActionSound(MarioAccess::getPlayerActor()->getYoshi(), pName, param2, param3, -1);
    }

    // SMG2 mPlayerMode ids (Fire 2, Yoshi 0xA, Cloud 0xC, Rock 0xD); PlayerMode enum in MarioActor.hpp is SMG1-shaped
    bool isPlayerElementModeFire() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 2;
    }

    bool isPlayerElementModeYoshi() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 10;
    }

    bool isPlayerElementModeCloud() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 12;
    }

    bool isPlayerElementModeRock() {
        return MarioAccess::getPlayerActor()->mPlayerMode == 13;
    }

};  // namespace MR

namespace MR {
    void startPlayerBlink() {
        MarioAccess::getPlayerActor()->fn_803D9710();
    }
    void stopPlayerBlink() {
        MarioAccess::getPlayerActor()->fn_803D9730();
    }
}
