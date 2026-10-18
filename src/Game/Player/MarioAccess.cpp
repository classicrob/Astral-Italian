#include "Game/Player/MarioAccess.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/MapObj/CollectCounter.hpp"
#include "Game/MapObj/StarPiece.hpp"
#include "Game/MapObj/StarPieceDirector.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Player/MarioFpView.hpp"
#include "Game/Player/MarioHang.hpp"
#include "Game/Player/MarioHolder.hpp"
#include "Game/Player/MarioParts.hpp"
#include "Game/Player/MarioState.hpp"
#include "Game/Player/MarioSwim.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Player/YoshiTongueAction.hpp"
#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Player/RushEndInfo.hpp"
#include "Game/Util.hpp"


namespace MarioAccess {
    void getTakePos(TVec3f* pOut) {
        if (getPlayerActor()->_494 != nullptr) {
            getPlayerActor()->_494->calc();
            getPlayerActor()->_494->copyTrans(pOut);
        }
    }

    bool isOnActor(const LiveActor* pActor) {
        if (getPlayerActor()->getMario()->_1C._13) {
            Triangle* marioGroundPolygon = getPlayerActor()->getMario()->mGroundPolygon;
            if (marioGroundPolygon->isValid()) {
                return marioGroundPolygon->mSensor->mHost == pActor;
            }

            return false;
        }

        if (getPlayerActor()->getMario()->_1C._14 || (getPlayerActor()->IsMarioSwimming() && getPlayerActor()->getMovementStates()._2)) {
            Triangle* marioTri = getPlayerActor()->getMario()->_45C;
            if (!marioTri->isValid()) {
                return false;
            }

            return marioTri->mSensor->mHost == pActor;
        }

        return false;
    }

    bool isOnGround(u32 a1) {
        if (a1 != 0) {
            return false;
        }

        if (getPlayerActor()->_934) {
            return MR::isOnGround(getPlayerActor()->_924->mHost);
        }

        return getPlayerActor()->getMovementStates()._1;
    }

    bool isHipDropFalling() {
        if (getPlayerActor()->getMovementStates().jumping && getPlayerActor()->getMovementStates()._B && !getPlayerActor()->getMovementStates()._1 &&
            !getPlayerActor()->isJumpRising()) {
            return true;
        }

        return false;
    }

    bool isHipDropLand() {
        return getPlayerActor()->getDrawStates()._14;
    }

    bool isSwingAction() {
        if (getPlayerActor()->_934) {
            return false;
        }

        if (getPlayerActor()->getMovementStates()._F) {
            return true;
        }

        return getPlayerActor()->isPunching();
    }

    bool isInRush() {
        return getPlayerActor()->_934;
    }

    bool isSquat() {
        return getPlayerActor()->getMovementStates()._A && getPlayerActor()->getMovementStates()._1;
    }

    bool isParalyzing() {
        return getPlayerActor()->getMario()->isStatusActive(MarioStatus_Paralyze);
    }

    bool isTeresaDisappear() {
        if (getPlayerActor()->mPlayerMode != PlayerMode_Teresa) {
            return false;
        }

        return getPlayerActor()->getMario()->_418;
    }

    bool isFlying() {
        if (getPlayerActor()->getMario()->isStatusActive(MarioStatus_Foo)) {
            return true;
        }

        return getPlayerActor()->getMario()->_10._21;
    }

    bool isNeedBrakingCamera() {
        return getPlayerActor()->getMario()->_1C._9;
    }

    CubeCameraArea* getCameraCubeCode() {
        return getPlayerActor()->getMario()->getCameraCubeCode();
    }

    bool isSwimming() {
        return getPlayerActor()->getMario()->isSwimming();
    }

    bool isSkating() {
        return getPlayerActor()->_3E5;
    }

    Triangle* getGroundingPolygon(u32) {
        if (isSwimming()) {
            if (getPlayerActor()->getMovementStates()._2) {
                return getPlayerActor()->getMario()->_45C;
            }

            return nullptr;
        }

        if (!isOnGround(0)) {
            return nullptr;
        }

        if (getPlayerActor()->_934) {
            return &getPlayerActor()->_924->mHost->mBinder->mGroundInfo.mParentTriangle;
        }

        return getPlayerActor()->getMario()->mGroundPolygon;
    }

    Triangle* getShadowingPolygon() {
        if (getPlayerActor()->getMovementStates()._2) {
            return getPlayerActor()->getMario()->_45C;
        }

        return nullptr;
    }

    void forceGameOverSink() {
        getPlayerActor()->forceGameOverSink();
    }

    void forceKill(u32 type, u32) {
        getPlayerActor()->forceKill(type);
    }

    void fn_803B5020(s32 value) {
        getPlayerActor()->fn_803BEB10(value);
    }

    bool isDisableFpView() {
        return getPlayerActor()->getMario()->isDisableFpViewMode();
    }

    bool isFpViewChangingFailure() {
        return getPlayerActor()->getMario()->_898;
    }

    void stopFpView() {
        if (getPlayerActor()->getMario()->isStatusActive(MarioStatus_FpView)) {
            getPlayerActor()->getMario()->mFpView->forceClose();
            getPlayerActor()->getMario()->closeStatus(nullptr);
        }
    }

    void noticeDashChance() {
        getPlayerActor()->getMario()->fn_80395310();
    }

    void fn_803B51B0() {
        getPlayerActor()->getMario()->fn_803B1710();
    }

    void forceFly(const TVec3f& rVec1, const TVec3f& rVec2, s32 a3) {
        getPlayerActor()->getMario()->doPointWarp(rVec1, rVec2, a3);
    }

    void setJumpVec(const TVec3f& rJumpVec) {
        getPlayerActor()->getMario()->mJumpVec = rJumpVec;
    }

    void forceJump(const TVec3f& rVec, u32) {
        getPlayerActor()->getMario()->mMovementStates._21 = true;
        getPlayerActor()->getMario()->cancelSquatMode();
        getPlayerActor()->getMario()->tryForceJumpDelay(rVec);
    }

    void freeJump(const TVec3f& rVec, u32) {
        getPlayerActor()->getMario()->mMovementStates._21 = true;
        getPlayerActor()->getMario()->cancelSquatMode();
        getPlayerActor()->getMario()->tryFreeJumpDelay(rVec);
    }

    void tornadoJump() {
        getPlayerActor()->getMario()->_544 = getPlayerActor()->getConst().getTable()->mTornadoTimeAir;
        getPlayerActor()->getMario()->tryTornadoJump();
        getPlayerActor()->mMario->startTornadoCentering(getPlayerActor()->_928);
    }

    void tornadoJumpMini() {
        getPlayerActor()->getMario()->_544 = getPlayerActor()->getConst().getTable()->mTornadoTimeAir;
        getPlayerActor()->getMario()->tryTornadoJump();
        TVec3f up(-*getPlayerActor()->getMario()->getGravityVec() * getPlayerActor()->getConst().getTable()->mTornadoBoostPower);
        getPlayerActor()->getMario()->setRocketBooster(up, getPlayerActor()->getConst().getTable()->mTornadoBoostAttnMini,
                                                       getPlayerActor()->getConst().getTable()->mTornadoBoostTimerMini);
        getPlayerActor()->mMario->startTornadoCentering(getPlayerActor()->_928);
    }

    void becomeNormalJumpStatus() {
        getPlayerActor()->getMario()->_430 = 0;
    }

    void setFrontVecKeepUp(const TVec3f& rVec, u16) {
        if (getPlayerActor()->getMario()->isStatusActive(MarioStatus_Hang)) {
            return;
        }

        getPlayerActor()->getMario()->setFrontVecKeepUp(rVec);
        getPlayerActor()->_2DC = getPlayerActor()->getMario()->mFrontVec;
        getPlayerActor()->mUpVec = getPlayerActor()->getMario()->mHeadVec;
        getPlayerActor()->_2E8 = getPlayerActor()->getMario()->mSideVec;
    }

    void setFrontVecTarget(const TVec3f& rVec, u16 a2) {
        MarioActor* actor = getPlayerActor();
        actor->_3B4 = rVec;
        actor->_3D0 = a2;
    }

    // NONMATCHING: equivalent player flag extraction.
    void getThrowVec(TVec3f* pOut) {
        if (getPlayerActor()->getMario()->isPlayerModeYoshi()) {
            pOut->set(getPlayerActor()->fn_803B9810()->_90);
            return;
        }
        if (getPlayerActor()->getMario()->isStatusActive(MarioStatus_Foo)) {
            *pOut = getPlayerActor()->getMario()->mHeadVec;
            return;
        }
        if (getPlayerActor()->getMario()->mMovementStates_HIGH_WORD & 0x100) {
            if (getPlayerActor()->getMario()->isSwimming()) {
                getPlayerActor()->getMario()->mSwim->fn_80400A40(pOut);
                return;
            }
        }
        if (getPlayerActor()->getMario()->isSwimming()) {
            getPlayerActor()->getThrowVec(pOut);
            return;
        }
        getPlayerActor()->getFrontHistoryVec(pOut);
    }

    void setTrans(const TVec3f& rTrans, u16) {
        MR::setPosition(getPlayerActor(), rTrans);
        getPlayerActor()->getMario()->mPosHolder->fn_80405F90(rTrans);

        if (getPlayerActor()->getMovementStates()._37) {
            getPlayerActor()->getMario()->_688 = rTrans;
        }

        getPlayerActor()->_1C0 = true;

        getPlayerActor()->_294 = rTrans;
        MR::updateHitSensorsAll(getPlayerActor());
    }

    void endRush(const RushEndInfo* pRushEndInfo) {
        getPlayerActor()->endRush(pRushEndInfo);
    }

    void incLife(u32 amt) {
        if (getPlayerActor()->IsMarioSwimming()) {
            for (int i = 0; i < amt; i++) {
                getPlayerActor()->getMario()->mSwim->incLife();
            }
        } else {
            getPlayerActor()->incLife(amt);
        }

        if (getPlayerActor()->mPlayerMode == PlayerMode_Bee) {
            for (u32 i = 0; i < getPlayerActor()->getConst().getTable()->mAirWalkTime / 8; i++) {
                getPlayerActor()->getMario()->incAirWalkTimer();
            }
        }
    }

    bool isConfrontDeath() {
        if (!getPlayerActor()->mSuperKinokoCollected) {
            if (getPlayerActor()->mHealth == 0) {
                return true;
            }

            if (getPlayerActor()->mHealth == 1 && getPlayerActor()->getMario()->isDamaging() && !getPlayerActor()->IsMarioSwimming()) {
                return true;
            }
        }

        if (getPlayerActor()->getMario()->mSinkTimer != 0 && getPlayerActor()->getMario()->isCurrentFloorSink()) {
            return true;
        }

        return !getPlayerActor()->isEnableNerveChange();
    }

    void addStarPiece() {
        f32 fogLevel = getPlayerActor()->getConst().getTable()->mStarPieceFogLevel;
        u8 fogTime = getPlayerActor()->getConst().getTable()->mStarPieceFogTime;
        MarioActor* actor = getPlayerActor();
        actor->_1AA = fogTime;
        actor->_1AC = fogLevel;
        actor->_1B0.set(0xFF, 0xFF, 0xFF, 0);
        actor->_1B5 = false;
    }

    void getStarPieceDirect() {
    }

    MtxPtr getJointMtx(const char* pName) {
        return getPlayerActor()->getGlobalJointMtx(pName);
    }

    TVec3f* getVelocity() {
        if (getPlayerActor()->_934) {
            return getLastMove();
        }

        return &getPlayerActor()->getMario()->mVelocity;
    }

    TVec3f* getLastMove() {
        return const_cast< TVec3f* >(&getPlayerActor()->getLastMove());
    }

    void hide() {
        getPlayerActor()->_482 = true;
        MR::forceDeleteEffectAll(getPlayerActor());
        getPlayerActor()->updateHand();
        getPlayerActor()->updateFace();
    }

    void show() {
        getPlayerActor()->_482 = false;
    }

    HitSensor* getTakingSensor() {
        if (getPlayerActor()->_428[0] != nullptr) {
            return getPlayerActor()->_428[0];
        }

        if (getPlayerActor()->_424 != nullptr) {
            return getPlayerActor()->_424;
        }

        return nullptr;
    }

    void dropTakingActor() {
        getPlayerActor()->rushDropThrowMemoSensor();
    }

    void killTakingActor() {
        getPlayerActor()->damageDropThrowMemoSensor();
    }

    f32 getAnimationFrameMax() {
        return getPlayerActor()->mMarioAnim->fn_803DD530();
    }

    void changeAnimationJ(const char* pAnimName) {
        getPlayerActor()->mMarioAnim->setAnimationFrame(0.0f);
        getPlayerActor()->changeAnimationNonStop(pAnimName);
    }

    void changeAnimationE(const char* pAnimName, s32 a2) {
        // unused
        getPlayerActor();
        if (getPlayerActor()->_B91) {
            return;
        }

        if (getPlayerActor()->getMario()->isPlayerModeTeresa()) {
            getPlayerActor()->changeTeresaAnimation(pAnimName, a2);
            return;
        }

        if (getPlayerActor()->_468 == 0) {
            getPlayerActor()->getMario()->stopAnimationUpperForce();
        }

        MR::startBck(getPlayerActor(), pAnimName);

        // unused
        getPlayerActor();

        if (a2 >= 0) {
            getPlayerActor()->mMarioAnim->getXanimePlayer()->changeInterpoleFrame(a2);
        }

        getPlayerActor()->setBlink(pAnimName);
        getPlayerActor()->mMarioAnim->closeCallback();
        getPlayerActor()->mMarioAnim->entryCallback(pAnimName);
    }

    void changeAnimationE(const char* pAnimName, const char* pChar2) {
        getPlayerActor()->fn_803BEA60(pAnimName, pChar2);
        getPlayerActor()->fn_803BEA80(pAnimName);
    }

    void changeAnimationE(const char* pAnimName, const BckCtrlData& rBckCtrlData) {
        getPlayerActor()->fn_803BEA70(pAnimName, &rBckCtrlData);
        getPlayerActor()->fn_803BEAA0(pAnimName, rBckCtrlData);
    }

    void keepCurrentAnimation() {
        if (getPlayerActor()->mMarioAnim->isFrameCtrlAttribute(0)) {
            getPlayerActor()->mMarioAnim->setAnimationFrameCtrlAttribute(1);
        }
    }

    void progressAnimation() {
        getPlayerActor()->mMarioAnim->update();
        getPlayerActor()->calcAnimInMovement();

        XanimePlayer* animePlayer = getPlayerActor()->mMarioAnim->getXanimePlayer();
        animePlayer->updateBeforeMovement();
        animePlayer->updateAfterMovement();
    }

    const char* getCurrentBckName() {
        return getPlayerActor()->getMario()->getCurrentBckName();
    }

    void setAnimationBlendWeight(const f32* pWeights) {
        getPlayerActor()->mMarioAnim->forceSetBlendWeight(pWeights);
    }

    void setSpot(f32, u32) {
    }

    void offControl() {
        getPlayerActor()->_3C0 = true;
    }

    bool isOffControl() {
        return getPlayerActor()->_3C0;
    }

    void onControl(bool resetCondition) {
        getPlayerActor()->_3C0 = false;

        if (resetCondition) {
            getPlayerActor()->resetCondition();
        }
    }

    void setStateWait() {
        getPlayerActor()->getMario()->stopJump();
        getPlayerActor()->getMario()->stopWalk();
    }

    void startTalk(const LiveActor* pTalkActor) {
        getPlayerActor()->getMario()->startTalk(pTalkActor);
        getPlayerActor()->stopSpinTicoEffect(false);
    }

    void endTalk() {
        getPlayerActor()->getMario()->endTalk();
    }

    // NONMATCHING: zero-vector constructor inlines to three stores.
    void readyRemoteDemo() {
        if (getPlayerActor()->_EA4) {
            return;
        }

        if (getPlayerActor()->getMario()->isStatusActive(MarioStatus_13) || getPlayerActor()->mPlayerMode == PlayerMode_Hopper) {
            MR::offPlayerControl();
            getPlayerActor()->getMario()->_10._20 = true;
            return;
        }

        getPlayerActor()->getMario()->_10._20 = false;

        if (getPlayerActor()->_934) {
            getPlayerActor()->_924->receiveMessage(ACTMES_RUSH_CANCEL, getPlayerActor()->getSensor("Body"));
            if (getPlayerActor()->_934) {
                RushEndInfo endInfo(nullptr, 4, TVec3f(0.0f, 0.0f, 0.0f), false, 0);
                endRush(&endInfo);
            }
        }

        if (getPlayerActor()->getMario()->isStatusActive(MarioStatus_FpView)) {
            getPlayerActor()->getMario()->closeStatus(nullptr);
        }

        getPlayerActor()->flushCoinPull();
        getPlayerActor()->calcAndSetBaseMtx();

        MR::offPlayerControl();

        getPlayerActor()->_EA4 = true;
        MR::invalidateHitSensors(getPlayerActor());

        PSMTXCopy(getPlayerActor()->getBaseMtx(), getPlayerActor()->mRemoteDemoMtx4E4.toMtxPtr());

        getPlayerActor()->_B90 = true;
        getPlayerActor()->getMario()->stopWalk();

        MR::deleteEffectAll(getPlayerActor());

        getPlayerActor()->_1B8->kill();
        getPlayerActor()->mScreenStrip1084->mProgress = 0.0f;
        getPlayerActor()->_EA6 = false;
    }

    void onFollowDemo() {
        if (getPlayerActor()->mPlayerMode == PlayerMode_Teresa) {
            MR::requestMovementOn(getPlayerActor()->_9A4);
        }
    }

    void setBaseMtx(MtxPtr mtx) {
        getPlayerActor()->forceSetBaseMtx(mtx);
    }

    MtxPtr getBaseMtx() {
        if (getPlayerActor()->_EA5) {
            return getPlayerActor()->mRemoteDemoMtx4E4.toMtxPtr();
        }

        if (getPlayerActor()->getMario()->isPlayerModeYoshi())
            return getPlayerActor()->fn_803B9850();
        return getPlayerActor()->getBaseMtx();
    }

    bool calcSpinPullVelocity(TVec3f* pOut, const TVec3f& rVec) {
        getPlayerActor()->tryPullTrans(pOut, rVec);
        return true;
    }

    void tryCoinPull() {
        if (!getPlayerActor()->getSensor("eye")->isValid()) {
            validateSensor();
        }
        getPlayerActor()->_6D0 = true;
    }

    void addVelocity(const TVec3f& rVec) {
        if (getPlayerActor()->IsMarioSwimming() && getPlayerActor()->getMario()->mSwim->fn_80400910()) {
            return;
        }

        if (getPlayerActor()->_EA4) {
            return;
        }

        if (getPlayerActor()->_934) {
            return;
        }

        getPlayerActor()->getMario()->push(rVec);

        if (getPlayerActor()->getMario()->isStatusActive(MarioStatus_Hang)) {
            getPlayerActor()->getMario()->mHang->forceDrop();
        }
    }

    void addVelocityFromArea(const TVec3f& rVec) {
        if (getPlayerActor()->IsMarioSwimming() && getPlayerActor()->getMario()->mSwim->fn_80400910()) {
            return;
        }

        if (getPlayerActor()->_EA4) {
            return;
        }

        if (getPlayerActor()->_934) {
            return;
        }

        getPlayerActor()->getMario()->push(rVec);
    }

    bool isOnWaterSurface() {
        if (getPlayerActor()->getMario()->isStatusActive(MarioStatus_Swim)) {
            return getPlayerActor()->getMario()->mSwim->isOnWaterSurface();
        }

        return getPlayerActor()->isAnimationRun("水泳ジェット");
    }

    void calcWorldPadDir(TVec3f* pOut, f32 f1, f32 f2) {
        getPlayerActor()->getMario()->calcWorldPadDir(pOut, f1, f2, false);
    }

    bool fn_803B72E0() {
        return getPlayerActor()->fn_803BEBE0();
    }

    MarioActor* getPlayerActor() {
        return MR::getMarioHolder()->getMarioActor();
    }

    void validateSensor() {
        getPlayerActor()->getSensor("eye")->validate();
    }

    void incOxygen(u32 amt) {
        if (!getPlayerActor()->IsMarioSwimming()) {
            return;
        }

        for (int i = 0; i < amt; i++) {
            getPlayerActor()->getMario()->mSwim->incOxygen();
        }

        getPlayerActor()->playEffect("酸素回復");
    }

    void scatterStarPiece(u32 amt) {
        if (MR::getStarPieceNum() > 0) {
            MR::startSound(getPlayerActor(), "SE_OJ_STAR_PIECE_BURST", -1, -1);
        }

        for (int i = 0; i < amt; i++) {
            if (MR::getStarPieceNum() == 0) {
                return;
            }

            StarPiece* launchPiece = MR::getDeadStarPiece();

            launchPiece->launch(getPlayerActor()->_2A0, 24.0f, 24.0f, false, false);
            MR::addStarPiece(-1);
        }
    }

    void startDownWipe() {
        if (getPlayerActor()->fn_803BED10()) {
            return;
        }

        if (getPlayerActor()->isAnimationRun("氷結")) {
            return;
        }

        if (getPlayerActor()->_390 != 0) {
            return;
        }

        getPlayerActor()->fn_803BBAD0();
    }

    // Not in SB4E01 (no caller, no symbol). Uses ACTMES_RUSH_FORCE_CANCEL, which is not in our headers.
#if 0
    void readyDemo() {
        if (getPlayerActor()->_934) {
            if (!getPlayerActor()->_924->receiveMessage(ACTMES_RUSH_CANCEL, getPlayerActor()->getSensor("body"))) {
                getPlayerActor()->_924->receiveMessage(ACTMES_RUSH_FORCE_CANCEL, getPlayerActor()->getSensor("body"));
            }

            if (getPlayerActor()->_934) {
                RushEndInfo endInfo(nullptr, 4, TVec3f(0.0f, 0.0f, 0.0f), false, 0);
                endRush(&endInfo);
            }
        }

        if (getPlayerActor()->getMario()->isStatusActive(MarioStatus_FpView)) {
            getPlayerActor()->getMario()->closeStatus(nullptr);
        }
    }
#endif

    void endRemoteDemo(const RushEndInfo* pEndInfo) {
        if (getPlayerActor()->getMario()->isStatusActive(MarioStatus_13) || getPlayerActor()->getMario()->isStatusActive(MarioStatus_Talk) ||
            getPlayerActor()->getMario()->_10._20 || !getPlayerActor()->_EA4) {
            getPlayerActor()->_3C0 = false;
            return;
        }

        getPlayerActor()->_B90 = false;
        onControl(true);
        getPlayerActor()->_EA4 = false;
        getPlayerActor()->stopAnimation(nullptr);

        if (getPlayerActor()->_468 != 0) {
            MarioActor* actor = getPlayerActor();
            getPlayerActor()->mMarioAnim->updateTakingAnimation(actor->_468 == 0 ? nullptr : actor->_428[0]);
        }

        MR::validateHitSensors(getPlayerActor());
        getPlayerActor()->getSensor("dummy")->invalidate();
        getPlayerActor()->_EA6 = false;
    }

    bool isInWaterMode() {
        if (getPlayerActor()->getMario()->isStatusActive(MarioStatus_Swim)) {
            return true;
        }

        return getPlayerActor()->isAnimationRun("水泳ジェット");
    }

    void changeItemStatus(s32 a1) {
        switch (a1) {
        case 0:
        case 8:
            getPlayerActor()->setPlayerMode(PlayerMode_Normal, true, true);
            break;
        case 1:
            getPlayerActor()->setPlayerMode(PlayerMode_Hopper, true, true);
            break;
        case 2:
            getPlayerActor()->setPlayerMode(PlayerMode_Bee, true, true);
            break;
        case 3:
            getPlayerActor()->setPlayerMode(PlayerMode_Teresa, true, true);
            break;
        case 4:
            getPlayerActor()->setPlayerMode(PlayerMode_Ice, true, true);
            break;
        case 5:
            getPlayerActor()->setPlayerMode(PlayerMode_2, true, true);
            break;
        case 6:
            getPlayerActor()->setPlayerMode(PlayerMode_Foo, true, true);
            break;
        case 7:
            getPlayerActor()->setPlayerMode(PlayerMode_Invincible, true, true);
            break;
        case 9:
            getPlayerActor()->getMario()->mMovementStates._F = true;
            getPlayerActor()->getMario()->_544 = 3;
            break;
        case 10:
            if (!getPlayerActor()->isEnableNerveChange()) {
                return;
            }

            if (getPlayerActor()->mHealth == 6) {
                MR::addPlayerLeft(1);
                return;
            }
            if (getPlayerActor()->mMaxHealth > 3) {
                MR::startSound(getPlayerActor(), "SE_OJ_MORPH_ITEM_INVALID", -1, -1);
                getPlayerActor()->changeMaxLife(6);
                return;
            }

            getPlayerActor()->mSuperKinokoCollected = true;
            break;
        case 11:
            getPlayerActor()->setPlayerMode(PlayerMode_Normal, false, false);
            break;
        }
    }
};  // namespace MarioAccess

namespace MarioAccess {
    bool isRecovering() { return getPlayerActor()->getMario()->isStatusActive(0x13); }
    bool isHanging() { return getPlayerActor()->getMario()->isStateHanging(); }

    bool fn_803B4E40() {
        bool result = false;
        if (getPlayerActor()->isJumping() && getPlayerActor()->getMario()->_430 == 5) result = true;
        return result;
    }

    // NONMATCHING: jump vector address is hoisted differently.
    f32 fn_803B57A0() {
        if (getPlayerActor()->getMario()->isSwimming()) {
            return getPlayerActor()->getMario()->mSwim->mForwardSpeed;
        }
        if (getPlayerActor()->isJumping()) {
            const TVec3f& jump = getPlayerActor()->getMario()->mJumpVec;
            f32 dot = getPlayerActor()->getMario()->mFrontVec.dot(jump);
            return dot / getPlayerActor()->mConst->getTable()->mJumpFrontSpeed;
        }
        return getPlayerActor()->getMario()->mWalkSpeed;
    }

    u32 fn_803B5970() { return getPlayerActor()->mHealth; }

    void fn_803B59A0() {
        if (getPlayerActor()->getMario()->isSwimming()) {
            getPlayerActor()->getMario()->mSwim->decLife();
            return;
        }
        if (getPlayerActor()->mHealth && !getPlayerActor()->_92C) {
            getPlayerActor()->_92C = 20;
        }
    }

    void concatWithJointMtx(TPos3f* matrix, const char* joint) {
        getPlayerActor()->fn_803BDA20(matrix->toMtxPtr(), joint);
    }

    void hide(bool force) {
        getPlayerActor()->_482 = true;
        if (force) MR::forceDeleteEffectAll(getPlayerActor());
        else MR::deleteEffectAll(getPlayerActor());
        getPlayerActor()->updateHand();
        getPlayerActor()->updateFace();
    }

    bool fn_803B5FE0() { return !getPlayerActor()->_482; }
    void fn_803B6090() { getPlayerActor()->fn_803BE6D0(); }

    // NONMATCHING: equivalent bit test and sensor-selection branch order.
    void fn_803B6830() {
        if (getPlayerActor()->getMario()->isStatusActive(0x13)
            || getPlayerActor()->getMario()->isStatusActive(0x22)
            || (getPlayerActor()->getMario()->_10_HIGH_WORD & 0x80000000)
            || !getPlayerActor()->_EA4) {
            MR::onPlayerControl(false);
            return;
        }
        getPlayerActor()->_B90 = false;
        MR::onPlayerControl(true);
        getPlayerActor()->_EA4 = false;
        getPlayerActor()->stopAnimation(nullptr);
        if (getPlayerActor()->_468) {
            HitSensor* sensor;
            MarioActor* actor = getPlayerActor();
            if (!actor->_468) sensor = nullptr;
            else sensor = actor->_428[0];
            getPlayerActor()->mMarioAnim->updateTakingAnimation(sensor);
        }
        getPlayerActor()->fn_803BEAC0();
        MR::validateHitSensors(getPlayerActor());
        getPlayerActor()->getSensor("dummy")->invalidate();
        getPlayerActor()->_EA6 = false;
    }

    void changePlayerModeYoshi() { getPlayerActor()->setPlayerMode(0xA, false, true); }
    void changePlayerModeCloud() { getPlayerActor()->setPlayerMode(0xC, true, true); }
    void changePlayerModeRock() { getPlayerActor()->setPlayerMode(0xD, true, true); }

    void fn_803B7070(s32 frames) {
        if (getPlayerActor()->mPlayerMode == 4) {
            for (s32 i = 0; i < frames; i++) getPlayerActor()->getMario()->incAirWalkTimer();
        }
    }

    void fn_803B70E0() { getPlayerActor()->killAllFireBall(); }

    bool isPowerupTransforming() {
        MarioActor* actor = getPlayerActor();
        bool result = false;
        if (actor->mSuperKinokoCollected || actor->mPowerupCollected || actor->mTransforming) result = true;
        return result;
    }

    void prepareForPositionReset() { getPlayerActor()->fn_803BE6F0(); }
    bool fn_803B71B0() { return getPlayerActor()->fn_803BE800(); }

    f32 fn_803B7240() {
        if (isConfrontDeath()) return 200.0f;
        if (!getPlayerActor()->isEnableNerveChange()) return 200.0f;
        return getPlayerActor()->getMario()->_72C;
    }

    void fn_803B72A0() { getPlayerActor()->getMario()->mMovementStates_HIGH_WORD &= ~0x20000000; }
}

namespace MarioAccess {
    bool isBury() { return getPlayerActor()->getMario()->isStateBury(); }
    void fn_803B6010() {
        MarioActor* actor = getPlayerActor();
        actor->_7E2 = 1;
        actor->mAnimationDelayE28 = 0;
    }
    void fn_803B6050(s32 frames) { getPlayerActor()->mAnimationDelayE28 = frames; }
    void fn_803B7170(TVec3f* velocity) { YoshiParam::fn_8041BE00(velocity); }
    void fn_803B71E0() { getPlayerActor()->mInputDisabled1066 = true; }
    void fn_803B7210() { getPlayerActor()->mInputDisabled1066 = false; }
}
