#pragma auto_inline off
#include "Game/Boss/KoopaJrCastleNerves.hpp"
#include <cmath>
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/PartsModelLodCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Ride/FluffWind.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"

namespace KoopaJrCastleFunction {
#define CASTLE() (MR::getSceneObj< KoopaJrCastleAccessor* >(0x8B)->mCastle)

    KoopaJrCastle* getCastle() {
        return CASTLE();
    }

    bool tryStartDemo(const char* pName, const char* pCameraName);
    bool tryEndDemo(const char* pName, const char* pCameraName, bool bForce);
    void fn_800D3770(const char* pName);
    void fn_800D38A0();
    void fn_800D3970();
    void fn_800D3A40();
    bool fn_800D3C60();
    void fn_800D3DB0();
    bool fn_800D3E10(LiveActor* pCastle);
    void fn_800D3F10(TVec3f* pOut, LiveActor* pCastle);
    bool fn_800D41C0(TVec3f* pPos);
    bool fn_800D4080(LiveActor* pActor, s32 arg);
    void fn_800D4150();
    void fn_800D4240();
    void fn_800D4520(LiveActor* pSrc, s32 arg);
    void fn_800D46F0();
    void fn_800D4740();
    bool isBodyMoveFlagSet();
    void fn_800D4B40();
    void killElectric();
    void killFireBar();
    void killCannon();
    void bodyTryStop();

    // fn_800D3540: starts the demo (plain or time-keep) and binds the castle camera to it.
    bool tryStartDemo(const char* pName, const char* pCameraName) {
        KoopaJrCastle* pCastle = CASTLE();
        bool started;
        if (pCameraName == nullptr) {
            started = MR::tryStartDemoMarioPuppetable(pCastle, pName);
        } else {
            started = MR::tryStartTimeKeepDemoMarioPuppetable(pCastle, pName, nullptr);
        }
        if (started) {
            if (pCameraName == nullptr) {
                pCameraName = pName;
            }
            MR::offBind(pCastle);
            MR::startAnimCameraTargetSelf(pCastle->mBody, pCastle->mCameraInfo, pCameraName, 1, false, 0.0f);
            return true;
        }
        return false;
    }

    // fn_800D35F0: ends the demo started by tryStartDemo and restores the castle parts.
    // Shared-header blocker: PartsModelLodCtrl getter is still declared as setLodCtrl(LodCtrl*)
    bool tryEndDemo(const char* pName, const char* pCameraName, bool bForce) {
        KoopaJrCastle* pCastle = CASTLE();
        bool ended;
        if (pCameraName == nullptr) {
            if (bForce) {
                if (MR::isActionEnd(pCastle->mBody)) {
                    MR::endDemo(pCastle, pName);
                    ended = true;
                } else {
                    ended = false;
                }
            } else {
                ended = MR::tryEndDemo(pCastle, pName, MR::isActionEnd(pCastle->mBody));
            }
        } else {
            ended = MR::tryEndDemoAndKillEffects(pCastle, pName);
        }

        if (MR::isDemoEndWaitCameraInterpolating()) {
            if (MR::isDemoEndWaitCinemaFrameStopped()) {
                MR::setPlayerPosAndWait("\x83}\x83\x8a\x83I\x88\xca\x92u[\x83""f\x83\x82\x8c\xe3]", false);
            }
        }

        if (ended) {
            if (pCameraName == nullptr) {
                pCameraName = pName;
            }
            if (MR::isDemoEndWaitCameraInterpolating()) {
                MR::tryForceDeleteEffectAll(pCastle->mBody);
                MR::tryForceDeleteEffectAll(pCastle->mHead);
                MR::tryForceDeleteEffectAll(pCastle->mCapsule);
            }
            MR::endAnimCamera(pCastle->mBody, pCastle->mCameraInfo, pCameraName, 1, true);
            MR::onBind(pCastle);
            pCastle->mKoopaJr->mCalcOwnMatrix = true;
            pCastle->mKoopaJr->getLodCtrl()->validate();
            pCastle->mBody->offFlagF5();
            pCastle->mHead->setNerveD();
            MR::setPlayerPosAndWait("\x83}\x83\x8a\x83I\x88\xca\x92u[\x83""f\x83\x82\x8c\xe3]", false);
            MR::showPlayer();
            return true;
        }
        return false;
    }

    // fn_800D3770: intro setup for the castle parts: movement on, demo anims, player placement.
    // Shared-header blocker: PartsModelLodCtrl getter is still declared as setLodCtrl(LodCtrl*)
    void fn_800D3770(const char* pName) {
        KoopaJrCastle* pCastle = CASTLE();
        MR::requestMovementOn(pCastle->mHead);
        MR::requestMovementOn(pCastle->mCapsule);
        MR::requestMovementOn(pCastle->mBody);
        MR::requestMovementOn(pCastle->mKoopaJr);
        MR::requestMovementOn(pCastle->mFireBar);
        MR::requestMovementOn(pCastle->mBreakModel);
        if (MR::isEqualString(pName, "DemoOpeningKoopaJrCastle")) {
            MR::startBckPlayer(pName, nullptr);
            MR::setPlayerPos("\x83}\x83\x8a\x83I\x88\xca\x92u[\x83""f\x83\x82\x92\x86]");
        } else {
            MR::hidePlayer();
        }
        MR::tryStartAllAnim(pCastle->mCapsule, pName);
        MR::tryStartAllAnim(pCastle->mHead, pName);
        MR::tryStartAllAnim(pCastle->mBody, pName);
        MR::tryStartAllAnim(pCastle->mKoopaJr, pName);
        pCastle->mKoopaJr->getLodCtrl()->invalidate();
        if (MR::isEqualString(pName, "DemoOpeningKoopaJrCastle")) {
            pCastle->mKoopaJr->mCalcOwnMatrix = false;
            MR::copyTransRotate(pCastle, pCastle->mKoopaJr);
        }
        pCastle->mBody->onFlagF5();
        pCastle->mFireBar->requestMovementOnChildren();
        pCastle->mHead->setNerveC();
    }

    // fn_800D38A0: kills the battle helpers (cannon, balls, wind, ring beam, electric) and stops the body.
    void fn_800D38A0() {
        KoopaJrCastle* pCastle = CASTLE();
        if (pCastle->mHead->fn_800D58D0()) {
            pCastle->mHead->setNerveD();
        }
        if (!MR::isDead(pCastle->mCannon)) {
            pCastle->mCannon->kill();
        }
        pCastle->mCannon->killAllBalls();
        if (!MR::isDead(pCastle->mWind)) {
            pCastle->mWind->kill();
        }
        if (!MR::isDead(pCastle->mRingBeam)) {
            pCastle->mRingBeam->kill();
        }
        pCastle->mRingBeam->fn_800D7CE0();
        killElectric();
        bodyTryStop();
    }

    // fn_800D3970: end-of-battle kill pass on the parts (unless the castle is in a cutscene).
    // NONMATCHING: early-return branch shape: target emits beq+b around the cutscene return
    void fn_800D3970() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isOnSwitchB(pCastle)) {
            if (!MR::isStageStateScenarioOpeningCamera()) {
                pCastle->mBody->offFlagF4();
                if (!MR::isDead(pCastle->mHead)) {
                    pCastle->mHead->kill();
                }
                if (!MR::isDead(pCastle->mCapsule)) {
                    pCastle->mCapsule->kill();
                }
                if (!MR::isDead(pCastle->mKoopaJr)) {
                    pCastle->mKoopaJr->kill();
                }
                fn_800D38A0();
                killFireBar();
            }
        }
    }

    // fn_800D3A40: switch-B / cutscene branch of the reappear pass.
    void fn_800D3A40() {
        KoopaJrCastle* pCastle = CASTLE();
        if (MR::isOnSwitchB(pCastle) || MR::isStageStateScenarioOpeningCamera()) {
            pCastle->mBody->onFlagF4();
            if (MR::isDead(pCastle->mHead)) {
                pCastle->mHead->makeActorAppeared();
            }
            if (MR::isDead(pCastle->mCapsule)) {
                pCastle->mCapsule->makeActorAppeared();
            }
        }
    }

    s32 getBattlePhase() {
        return CASTLE()->mStateBattle->mPhase;
    }

    #pragma dont_inline on
    const TVec3f& getStartPos() {
        return static_cast<KoopaJrCastleAccessor*>(MR::getSceneObjHolder()->getObj(0x8B))->mCastle->mStartPos;
    }
    #pragma dont_inline off

    bool isCapsuleSensorHost(const KoopaJrCastle* pCastle, const HitSensor* pSensor) {
        LiveActor* pCapsule = pCastle->mCapsule;
        return pCapsule == MR::getSensorHost(pSensor);
    }

    bool isPlayerOnCastle() {
        return isPlayerOnCapsule() || isPlayerOnHead();
    }

    bool isPlayerOnCapsule() {
        return MR::isOnPlayer(CASTLE()->mCapsule);
    }

    bool isPlayerOnHead() {
        return MR::isOnPlayer(CASTLE()->mHead);
    }


    bool fn_800D3C60() {
        const TVec3f* player;
        KoopaJrCastle* castle = CASTLE();
        player = MR::getPlayerPos();
        TVec3f delta = (castle->mHead->mPosition - castle->mGravity * 1000.0f) - *player;
        TVec3f planar(0.0f);
        f32 height = MR::vecKillElement(delta, -castle->mGravity, &planar);
        if (-700.0f < height && height < 1500.0f && PSVECMag(&planar) < 3500.0f) {
            return true;
        }
        return false;
    }

    bool isElectricActive() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mElectric) && pCastle->mElectric->fn_800D2250()) {
            return true;
        }
        return false;
    }

    // fn_800D3DB0: after the battle reset the body rests (stop or wait) depending on the battle phase.
    void fn_800D3DB0() {
        fn_800D38A0();
        KoopaJrCastle* pCastle = CASTLE();
        pCastle->mHead->setNerveD();
        if (getBattlePhase() == 1) {
            pCastle->mBody->setNerveStop();
        } else {
            pCastle->mBody->setNerveWait();
        }
    }

    // fn_800D3E10: pushes the player off the castle surface along the castle gravity when standing on it.
    bool fn_800D3E10(LiveActor* pCastle) {
        bool onCastle = false;
        if (isPlayerOnCapsule() || isPlayerOnHead()) {
            onCastle = true;
        }
        if (!onCastle) {
            return false;
        }
        TVec3f out;
        fn_800D3F10(&out, pCastle);
        const Vec* pPlayerPos = MR::getPlayerPos();
        TVec3f rel(out);
        JMathInlineVEC::PSVECSubtract(&rel, pPlayerPos, &rel);
        TVec3f* pGravity = &pCastle->mGravity;
        TVec3f proj;
        JMAVECScaleAdd(pGravity, &rel, &proj, -pGravity->dot(rel));
        TVec3f dir;
        JGeometry::negateInternal(&proj.x, &dir.x);
        MR::offBind(MR::getPlayerDemoActor());
        MR::unlockPlayerAnimation();
        MR::forceFlyPlayer(out, dir, (s32)(MR::calcDistanceToPlayer(out) / 30.0f));
        return true;
    }

    // fn_800D3F10: player position projected onto the castle surface (plane of gravity) relative to the castle.
        void fn_800D3F10(TVec3f* pOut, LiveActor* pCastle) {
        TVec3f v;
        JMathInlineVEC::PSVECSubtract(MR::getPlayerPos(), &pCastle->mPosition, &v);
        TVec3f* gravity = &pCastle->mGravity;
        JMAVECScaleAdd(gravity, &v, &v, -gravity->dot(v));
        MR::normalizeOrZero(&v);
        if (MR::isNearZero(v, 0.001f)) {
            MR::getPlayerFrontVec(&v);
            JGeometry::negateInternal(&v.x, &v.x);
        }
        *pOut = (pCastle->mPosition + v * 3200.0f) - pCastle->mGravity * -10.0f;
        if (fn_800D41C0(pOut)) {
            return;
        }
        *pOut = (pCastle->mPosition - v * 3200.0f) - pCastle->mGravity * -10.0f;
    }

    // fn_800D4080: spawns the dominate/cannon sequence and plays the PlayerBlow action sound.
    bool fn_800D4080(LiveActor* pActor, s32 arg) {
        KoopaJrCastle* pCastle = CASTLE();
        TVec3f pos(0.0f);
        fn_800D3F10(&pos, pCastle);
        pCastle->mDominate->_9C = 0.0f;
        pCastle->mDominate->fn_800D7560("Blow", "WaterBulletWait");
        if (pCastle->mDominate->fn_800D71B0(pActor, arg, &pos, -1)) {
            killCannon();
            MR::startActionSound(pActor, "PlayerBlow", -1, -1, -1);
            return true;
        }
        return false;
    }

    // fn_800D4150: moves the player to the castle start point (offset 1000 along +Z), snapped to the named position.
        void fn_800D4150() {
        TVec3f pos = getStartPos() + TVec3f(0.0f, 0.0f, 1.0f) * 1000.0f;
        MR::tryFindNamePos("\x83v\x83\x8c\x83""C\x83\x84\x81[\x88\xca\x92u", &pos, nullptr);
        MR::setPlayerPos(pos);
    }

    // fn_800D41C0: true when pPos is within 5000 units of the castle's start position.
        bool fn_800D41C0(TVec3f* pPos) {
        const TVec3f& start = CASTLE()->mStartPos;
        TVec3f d(*pPos);
        d -= start;
        return __fabsf(PSVECMag(&d)) < 5000.0f;
    }

    // fn_800D4240: resets the body's F0 flag and binds the castle to its fire bar shadows.
    void fn_800D4240() {
        KoopaJrCastle* pCastle = CASTLE();
        pCastle->mBody->resetF0();
        pCastle->mFireBar->updateChildShadows();
        MR::onBind(pCastle);
    }

    void windFn_800D4290() {
        CASTLE()->mWind->fn_800D9B40();
    }

    bool windIsDead() {
        return MR::isDead(CASTLE()->mWind);
    }

    bool windFn_800D4310() {
        return CASTLE()->mWind->fn_800D9C40();
    }

    void bodyStartMove0() {
        CASTLE()->mBody->startMove0();
    }

    void bodyStartMove1() {
        CASTLE()->mBody->startMove1();
    }

    void bodyStartMove2() {
        CASTLE()->mBody->startMove2();
    }

    void bodyTryStop() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!bodyIsNotMove()) {
            pCastle->mBody->setNerveStop();
        }
    }

    bool bodyIsMoveState() {
        return CASTLE()->mBody->isMoveState();
    }

    bool bodyIsNotMove() {
        return !CASTLE()->mBody->isMove();
    }

    bool isBodyMoveFlagSet() {
        return CASTLE()->mBody->getStateFlag();
    }

    // fn_800D4520: hands a source actor and value to the cannon and makes it appear.
    void fn_800D4520(LiveActor* pSrc, s32 arg) {
        KoopaJrCastle* pCastle = CASTLE();
        pCastle->mCannon->_D0 = reinterpret_cast<s32>(pSrc);
        pCastle->mCannon->_C8 = arg;
        pCastle->mCannon->appear();
    }

    void cannonFn_800D4590() {
        CASTLE()->mCannon->fn_800D0540();
    }

    void cannonFn_800D45D0() {
        CASTLE()->mCannon->fn_800D05C0();
    }

    void cannonFn_800D4610() {
        CASTLE()->mCannon->fn_800D0580();
    }

    void killCannon() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mCannon)) {
            pCastle->mCannon->kill();
        }
    }

    bool isCannonDead() {
        return MR::isDead(CASTLE()->mCannon);
    }

    // fn_800D46F0: head action 'A' (only when the head is not already in an E/F/G/H nerve).
    void fn_800D46F0() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!pCastle->mHead->fn_800D58D0()) {
            fn_800D38A0();
            pCastle->mHead->fn_800D5830();
        }
    }

    // fn_800D4740: same as fn_800D46F0 but with the head's 'G' variant.
    void fn_800D4740() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!pCastle->mHead->fn_800D58D0()) {
            fn_800D38A0();
            pCastle->mHead->fn_800D5880();
        }
    }

    bool isHeadFn_800D4790() {
        return !CASTLE()->mHead->fn_800D58D0();
    }

    void appearFireBar() {
        KoopaJrCastle* pCastle = CASTLE();
        if (MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->appear();
        }
    }

    void killFireBar() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->kill();
        }
    }

    void fireBarOff() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->_CC = 0;
        }
    }

    void fireBarOn() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->_CC = 1;
        }
    }

    void appearFireBarAndStart() {
        KoopaJrCastle* pCastle = CASTLE();
        if (MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->appear();
        }
        pCastle->mFireBar->setCurrentToMax();
    }

    f32 getFireBarB0() {
        KoopaJrCastleFireBar* pFireBar = CASTLE()->mFireBar;
        if (pFireBar != nullptr) {
            return pFireBar->_B0;
        }
        return 0.0f;
    }

    void killRingBeam() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mRingBeam)) {
            pCastle->mRingBeam->kill();
        }
    }

    void appearElectric() {
        KoopaJrCastle* pCastle = CASTLE();
        if (MR::isDead(pCastle->mElectric)) {
            pCastle->mElectric->appear();
        }
    }

    void killElectric() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mElectric)) {
            pCastle->mElectric->kill();
        }
    }

    bool isElectricDead() {
        return MR::isDead(CASTLE()->mElectric);
    }

    // fn_800D4B40: validates the hit sensors of the body and head (type 0x35) and the head's BreakObject sensor.
    void fn_800D4B40() {
        LiveActor* pBody = CASTLE()->mBody;
        LiveActor* pHead = CASTLE()->mHead;
        if (pBody != nullptr && pHead != nullptr) {
            for (s32 i = 0; i < MR::getSensorNum(pBody); i++) {
                HitSensor* pSensor = MR::getSensor(pBody, i);
                if (MR::isSensorType(pSensor, 0x35)) {
                    MR::validateHitSensor(pSensor);
                }
            }
            for (s32 i = 0; i < MR::getSensorNum(pHead); i++) {
                HitSensor* pSensor = MR::getSensor(pHead, i);
                if (MR::isSensorType(pSensor, 0x35)) {
                    MR::validateHitSensor(pSensor);
                }
            }
            MR::validateHitSensor(pHead->getSensor("BreakObject"));
        }
    }

    // fn_800D4C40: turns the head's matrix toward the player (joint "JointRoot" local space) and reports
    // whether the side vector is within 1 degree of the player direction.
    bool aimHeadAtPlayer(f32 turnRate, f32 limitAngle) {
        f32 rate = turnRate;
        if (MR::isNearZero(turnRate, 0.001f)) {
            rate = 0.1f;
        }

        MtxPtr headMtx = CASTLE()->mHead->_A4;
        MtxPtr jointMtx = MR::getJointMtx(CASTLE()->mHead, "JointRoot");
        TPos3f mtx;
        PSMTXIdentity(mtx);
        PSMTXCopy(jointMtx, mtx);
        MR::makeMtxWithoutScale(&mtx, mtx);
        PSMTXInverse(mtx, mtx);

        TVec3f up(0.0f, 1.0f, 0.0f);
        TVec3f front(0.0f, 0.0f, 1.0f);
        TVec3f side(front);
        MR::extractMtxXDir(headMtx, &side);
        TVec3f pos(0.0f);
        MR::extractMtxTrans(headMtx, &pos);

        TVec3f playerPos = *MR::getPlayerPos() - *MR::getPlayerGravity() * 100.0f;
        TVec3f local(playerPos);
        PSMTXMultVec(mtx, playerPos, local);
        TVec3f dir(local);
        JMathInlineVEC::PSVECSubtract(&dir, &pos, &dir);
        if (MR::normalizeOrZero(&dir)) {
            dir = front;
        }

        TVec3f planeDir;
        MR::turnVecToPlane(&planeDir, dir, up);
        if (dir.dot(up) >= 0.0f) {
            MR::clampVecAngleDeg(&dir, planeDir, limitAngle);
        } else {
            MR::clampVecAngleDeg(&dir, planeDir, 10.0f);
        }
        if (MR::normalizeOrZero(&dir)) {
            dir = front;
        }

        f32 sideAngle;
        f32 cosRate = MR::cosDegree(rate);
        sideAngle = 57.295780181884766f * side.angle(dir);
        TVec3f cross;
        PSVECCrossProduct(&side, &dir, &cross);
        if (MR::normalizeOrZero(&cross)) {
            cross = up;
        }

        if (sideAngle > 90.0f) {
            TVec3f sideXZ(side);
            sideXZ.y = 0.0f;
            if (MR::normalizeOrZero(&sideXZ)) {
                sideXZ = front;
            }
            MR::vecBlend(&side, side, sideXZ, 0.02f);
            if (MR::normalizeOrZero(&side)) {
                side = front;
            }
            MR::rotateVecDegree(&side, side, -up, cosRate);
        } else {
            MR::turnVecToVecCos(&side, side, dir, cosRate, cross, 0.02f);
        }
        if (MR::normalizeOrZero(&side)) {
            side = front;
        }
        MR::makeMtxSideUpPos(reinterpret_cast<TPos3f*>(headMtx), side, up, pos);

        TVec3f xDir;
        MR::extractMtxXDir(headMtx, &xDir);
        f32 tolerance = 1.0f;
        sideAngle = 0.017453292f * tolerance;
        return xDir.angle(dir) <= sideAngle;
    }

    // fn_800D5040: turns the castle head's matrix toward the player at the given rate (side/up re-orthonormalized).
        bool fn_800D5040(f32 farg0) {
        f32 turnRate = MR::isNearZero(farg0, 0.001f) ? 0.1f : farg0;
        MtxPtr headMtx = CASTLE()->mHead->_A4;
        MtxPtr jointMtx = MR::getJointMtx(CASTLE()->mHead, "JointRoot");
        TPos3f mtx;
        PSMTXIdentity(mtx);
        PSMTXCopy(jointMtx, mtx);
        MR::makeMtxWithoutScale(&mtx, mtx);
        PSMTXInverse(mtx, mtx);

        TVec3f up(0.0f, 1.0f, 0.0f);
        TVec3f front(0.0f, 0.0f, 1.0f);
        TVec3f side(front);
        MR::extractMtxXDir(headMtx, &side);
        TVec3f pos(0.0f);
        MR::extractMtxTrans(headMtx, &pos);

        TVec3f playerPos = *MR::getPlayerPos() - *MR::getPlayerGravity() * 100.0f;
        TVec3f local(playerPos);
        PSMTXMultVec(mtx, playerPos, local);
        TVec3f dir(side);
        MR::vecKillElement(dir, TVec3f(0.0f, 1.0f, 0.0f), &dir);
        if (MR::normalizeOrZero(&dir)) {
            dir = front;
        }
        f32 sideAngle = 57.295780181884766f * side.angle(dir);
        TVec3f cross;
        PSVECCrossProduct(&side, &dir, &cross);
        if (MR::normalizeOrZero(&cross)) {
            cross = up;
        }
        f32 cosRate = MR::cosDegree(turnRate);
        if (sideAngle > 90.0f) {
            TVec3f sideXZ(side);
            sideXZ.y = 0.0f;
            if (MR::normalizeOrZero(&sideXZ)) {
                sideXZ = front;
            }
            MR::vecBlend(&side, side, sideXZ, 0.02f);
            if (MR::normalizeOrZero(&side)) {
                side = front;
            }
            MR::rotateVecDegree(&side, side, -up, cosRate);
        } else {
            MR::turnVecToVecCos(&side, side, dir, cosRate, cross, 0.02f);
        }
        MR::makeMtxSideUpPos(reinterpret_cast<TPos3f*>(headMtx), side, up, pos);
        TVec3f xDir;
        MR::extractMtxXDir(headMtx, &xDir);
        f32 tolerance = 1.0f;
        f32 maxAngle = 0.017453292f * tolerance;
        return xDir.angle(dir) <= maxAngle;
    }

    // fn_800D5340: star piece burst from the capsule, opposite its gravity, with the burst sound on the body.
    void appearCapsuleStarPiece() {
        KoopaJrCastle* pCastle = CASTLE();
        const TVec3f& rCapsulePos = pCastle->mCapsule->mPosition;
        TVec3f dir;
        JGeometry::negateInternal(&pCastle->mGravity.x, &dir.x);
        MR::appearStarPieceToDirection(pCastle, rCapsulePos, dir, 15, 30.0f, 40.0f, false);
        MR::startSound(pCastle->mBody, "SE_OJ_STAR_PIECE_BURST", -1, -1);
    }

};  // namespace KoopaJrCastleFunction
