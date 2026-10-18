#pragma auto_inline off
// PLACEHOLDER(objects): unnamed core helper (demo end wait check); resolve when core names fn_80020800
extern "C" bool fn_80020800();

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

namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvA KoopaJrCastleHeadNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvB KoopaJrCastleHeadNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvC KoopaJrCastleHeadNrvC::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvD KoopaJrCastleHeadNrvD::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvE KoopaJrCastleHeadNrvE::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvF KoopaJrCastleHeadNrvF::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvG KoopaJrCastleHeadNrvG::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvH KoopaJrCastleHeadNrvH::sInstance ATTRIBUTE_WEAK; }

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
    void fn_800D44E0();
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
    // NONMATCHING: regalloc (name/camera in r28/r29 in target), li r4,0 before setPlayerPosAndWait, and an uninitialized r4 before setLodCtrl
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
            if (fn_80020800()) {
                MR::setPlayerPosAndWait("\x83}\x83\x8a\x83I\x88\xca\x92u[\x83f\x83\x82\x8c\xe3]");
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
            pCastle->mKoopaJr->setLodCtrl(pCastle->mKoopaJr->mLodCtrl)->validate();
            pCastle->mBody->offFlagF5();
            pCastle->mHead->setNerveD();
            MR::setPlayerPosAndWait("\x83}\x83\x8a\x83I\x88\xca\x92u[\x83f\x83\x82\x8c\xe3]");
            MR::showPlayer();
            return true;
        }
        return false;
    }

    // fn_800D3770: intro setup for the castle parts: movement on, demo anims, player placement.
    // NONMATCHING: target passes an unset register as the LodCtrl argument of setLodCtrl (uninitialized read)
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
            MR::setPlayerPos("\x83}\x83\x8a\x83I\x88\xca\x92u[\x83f\x83\x82\x92\x86]");
        } else {
            MR::hidePlayer();
        }
        MR::tryStartAllAnim(pCastle->mCapsule, pName);
        MR::tryStartAllAnim(pCastle->mHead, pName);
        MR::tryStartAllAnim(pCastle->mBody, pName);
        MR::tryStartAllAnim(pCastle->mKoopaJr, pName);
        pCastle->mKoopaJr->setLodCtrl(pCastle->mKoopaJr->mLodCtrl)->invalidate();
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

    const TVec3f& getStartPos() {
        return CASTLE()->mStartPos;
    }

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
    // NONMATCHING: target keeps more separate temporaries (144-byte frame, _savegpr_29); stack/regalloc only
    void fn_800D3F10(TVec3f* pOut, LiveActor* pCastle) {
        TVec3f v;
        JMathInlineVEC::PSVECSubtract(MR::getPlayerPos(), &pCastle->mPosition, &v);
        JMAVECScaleAdd(&pCastle->mGravity, &v, &v, -pCastle->mGravity.dot(v));
        MR::normalizeOrZero(&v);
        if (MR::isNearZero(v, 0.001f)) {
            MR::getPlayerFrontVec(&v);
            JGeometry::negateInternal(&v.x, &v.x);
        }
        TVec3f a = pCastle->mGravity * -10.0f;
        TVec3f b = v * 3200.0f;
        TVec3f c = pCastle->mPosition + b;
        JMathInlineVEC::PSVECSubtract(&c, &a, &c);
        *pOut = c;
        if (fn_800D41C0(pOut)) {
            return;
        }
        TVec3f e = pCastle->mGravity * -10.0f;
        TVec3f f = v * 3200.0f;
        TVec3f g(pCastle->mPosition);
        JMathInlineVEC::PSVECSubtract(&g, &f, &g);
        JMathInlineVEC::PSVECSubtract(&g, &e, &g);
        *pOut = g;
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
    // NONMATCHING: stack/temporary layout differs (dir kept at a different slot)
    void fn_800D4150() {
        TVec3f dir(0.0f, 0.0f, 1.0f);
        TVec3f offset = dir * 1000.0f;
        TVec3f pos = CASTLE()->mStartPos + offset;
        MR::tryFindNamePos("\x83v\x83\x8c\x83C\x83\x84\x81[\x88\xca\x92u", &pos, nullptr);
        MR::setPlayerPos(pos);
    }

    // fn_800D41C0: true when pPos is within 5000 units of the castle's start position.
    // NONMATCHING: callee-saved register assignment (r30/r31) and the castle temporary order differ
    bool fn_800D41C0(TVec3f* pPos) {
        KoopaJrCastle* pCastle = CASTLE();
        TVec3f d(*pPos);
        d -= pCastle->mStartPos;
        return PSVECMag(&d) < 5000.0f;
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

    // fn_800D44E0: body state helper (fn_800CE340 in the body TU).
    void fn_800D44E0() {
        CASTLE()->mBody->getStateFlag();
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

        TVec3f offset = *MR::getPlayerGravity() * 100.0f;
        TVec3f playerPos(*MR::getPlayerPos());
        JMathInlineVEC::PSVECSubtract(&playerPos, &offset, &playerPos);
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

        f32 cosRate = MR::cosDegree(rate);
        f32 sideAngle = 57.295780181884766f * side.angle(dir);
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
            TVec3f negUp;
            JGeometry::negateInternal(&up.x, &negUp.x);
            MR::rotateVecDegree(&side, side, negUp, cosRate);
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
        return xDir.angle(dir) <= 0.017453292f * tolerance;
    }

    // fn_800D5040: turns the castle head's matrix toward the player at the given rate (side/up re-orthonormalized).
    // NONMATCHING: register/stack allocation of the turn rate (f30 vs f31, 256-byte frame)
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

        TVec3f offset = *MR::getPlayerGravity() * 100.0f;
        TVec3f playerPos(*MR::getPlayerPos());
        JMathInlineVEC::PSVECSubtract(&playerPos, &offset, &playerPos);
        TVec3f local(playerPos);
        PSMTXMultVec(mtx, playerPos, local);
        TVec3f dir(local);
        JMathInlineVEC::PSVECSubtract(&dir, &pos, &dir);
        if (MR::normalizeOrZero(&dir)) {
            dir = front;
        }
        TVec3f planeDir;
        MR::turnVecToPlane(&planeDir, dir, up);
        MR::clampVecAngleDeg(&dir, planeDir, 10.0f);
        if (MR::normalizeOrZero(&dir)) {
            dir = front;
        }

        f32 cosRate = MR::cosDegree(turnRate);
        f32 sideAngle = 57.295780181884766f * side.angle(dir);
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
            TVec3f negUp;
            JGeometry::negateInternal(&up.x, &negUp.x);
            MR::rotateVecDegree(&side, side, negUp, cosRate);
        } else {
            MR::turnVecToVecCos(&side, side, dir, cosRate, cross, 0.02f);
        }
        if (MR::normalizeOrZero(&side)) {
            side = front;
        }
        MR::makeMtxSideUpPos(reinterpret_cast<TPos3f*>(headMtx), side, up, pos);
        return true;
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

    MtxPtr getCannon3Mtx(LiveActor* pHead) {
        return MR::getJointMtx(pHead, "Cannon3");
    }
};  // namespace KoopaJrCastleFunction



KoopaJrCastleHead::KoopaJrCastleHead(LiveActor* pHost, const char* pName) : PartsModel(pHost, pName, "KoopaJrCastleHead", 0, 0x13, false) {
    _A0 = nullptr;
    _D4 = true;
    PSMTXIdentity(_A4);
}



// fn_800D5440: head init: joint controller on "Cannon1", inverse rest matrix, then the 'Wait' action.
// NONMATCHING: target builds a derived JointController (vtable lbl_80674F10 with ptmf callback fields); base-class version emitted
void fn_800D5440(KoopaJrCastleHead* pHead, const JMapInfoIter& rIter) {
    JointController* pJoint = new JointController();
    MR::setJointControllerParam(pJoint, pHead, "Cannon1");
    pHead->_A0 = pJoint;
    MR::initJointTransform(pHead);
    MtxPtr rootMtx = MR::getJointMtx(pHead, "JointRoot");
    Mtx inv;
    PSMTXInverse(rootMtx, inv);
    PSMTXConcat(inv, MR::getJointMtx(pHead, "Cannon1"), pHead->_A4);
    pHead->init(rIter);
    pHead->initNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvD::sInstance, 0);
    MR::startAction(pHead, "Wait");
    pHead->makeActorAppeared();
    MR::invalidateHitSensor(pHead, "Panic");
    pHead->_D4 = false;
}

// fn_800D55B0: forwards an attack-sensor pair to the host unless the actor is in its C nerve.
void fn_800D55B0(PartsModel* pActor, HitSensor* pSender, HitSensor* pReceiver) {
    if (!pActor->isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance)) {
        LiveActor* pHost = pActor->mHost;
        pHost->attackSensor(pSender, pReceiver);
    }
}

// fn_800D5620: forwards receiveMsgPlayerAttack to the host unless the actor is in its C nerve.
bool fn_800D5620(PartsModel* pActor, u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (pActor->isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance)) {
        return false;
    }
    LiveActor* pHost = pActor->mHost;
    return pHost->receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

// fn_800D56A0: forwards receiveOtherMsg to the host unless the actor is in its C nerve.
bool fn_800D56A0(PartsModel* pActor, u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (pActor->isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance)) {
        return false;
    }
    LiveActor* pHost = pActor->mHost;
    return pHost->receiveOtherMsg(msg, pSender, pReceiver);
}

void KoopaJrCastleHead::setNerveA() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvA::sInstance);
}

void KoopaJrCastleHead::setNerveB() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvB::sInstance);
}

void KoopaJrCastleHead::setNerveC() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance);
}

// fn_800D5760: break the capsule (Break + Wait animations), then invalidate its collision.
void fn_800D5760(LiveActor* pActor) {
    MR::invalidateCollisionParts(pActor);
    KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
    MR::startBva(pCastle->mCapsule, "Break");
    MR::startBrk(pCastle->mCapsule, "Wait");
    MR::tryHitReactionBreak(pCastle->mCapsule);
    MR::invalidateCollisionParts(pCastle->mCapsule);
}

void KoopaJrCastleHead::setNerveD() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvD::sInstance);
}

void KoopaJrCastleHead::fn_800D57D0() {
    if (!isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvB::sInstance) && !isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance)) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvA::sInstance);
    }
}

bool KoopaJrCastleHead::fn_800D5830() {
    if (fn_800D5E90()) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvE::sInstance);
        return true;
    }
    return false;
}

bool KoopaJrCastleHead::fn_800D5880() {
    if (fn_800D5E90()) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvG::sInstance);
        return true;
    }
    return false;
}

bool KoopaJrCastleHead::fn_800D58D0() const {
    bool ret = false;
    if (isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvE::sInstance) || isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvF::sInstance) ||
        isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvG::sInstance) || isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvH::sInstance)) {
        ret = true;
    }
    return ret;
}

// fn_800D5960: first-step setup of the head's 'Wait' state (castle capsule and body follow).
// NONMATCHING: string table addressing: target uses lbl_80674E30 + offset (addi r4,r30,52), literals here
void KoopaJrCastleHead::exe800D5960() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::deleteEffect(this, "Spin");
        MR::invalidateHitSensor(this, "Panic");
        KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
        MR::startBrk(pCastle->mCapsule, "Wait");
        MR::startAction(pCastle->mKoopaJr, "Wait");
    }
}

// fn_800D59F0: first-step setup of the head's 1st damage state.
// NONMATCHING: string table addressing: target uses lbl_80674E30 + offset, literals here
void KoopaJrCastleHead::exe800D59F0() {
    if (MR::isFirstStep(this)) {
        KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
        MR::startAction(this, "1stDamage");
        MR::startAction(pCastle->mKoopaJr, "Damage");
        MR::startBva(pCastle->mCapsule, "Crack1");
        MR::startBrk(pCastle->mCapsule, "Wait");
        MR::tryHitReaction(pCastle->mCapsule, "\x82\xd0\x82\xd1\x8a\x84\x82\xea");
    }
}

// fn_800D5A80: first-step setup of the head's 2nd damage state.
// NONMATCHING: string table addressing: target uses lbl_80674E30 + offset, literals here
void KoopaJrCastleHead::exe800D5A80() {
    if (MR::isFirstStep(this)) {
        KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
        MR::startAction(this, "2ndDamage");
        MR::startAction(pCastle->mKoopaJr, "Damage");
        MR::startBva(pCastle->mCapsule, "Crack2");
        MR::startBrk(pCastle->mCapsule, "Wait");
        MR::tryHitReaction(pCastle->mCapsule, "\x82\xd0\x82\xd1\x8a\x84\x82\xea");
    }
}

// fn_800D5B10: 'Wait' phase of the head: animates, tracks the player, and returns to nerve D when the player leaves.
// NONMATCHING: string table addressing: target uses lbl_80674E30 + offset, literals here
void KoopaJrCastleHead::exe800D5B10() {
    KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startAction(pCastle->mKoopaJr, "PanicStart");
    }
    if (MR::isBckOneTimeAndStopped(pCastle->mKoopaJr)) {
        MR::startAction(pCastle->mKoopaJr, "Panic");
    }
    if (!KoopaJrCastleFunction::isPlayerOnCastle() && MR::isOnGroundPlayer()) {
        MR::startAction(pCastle->mKoopaJr, "Wait");
        setNerveD();
        return;
    }
    KoopaJrCastleFunction::fn_800D5040(0.4f);
    if (MR::isGreaterEqualStep(this, 0x78)) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvF::sInstance);
    }
}

// fn_800D5BF0: spin-sign sequence and the F/G/H follow-up of the head's panic phase.
// NONMATCHING: string table addressing: target uses lbl_80674E30 + offset, literals here
void KoopaJrCastleHead::exe800D5BF0() {
    if (MR::isFirstStep(this)) {
        KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
        MR::startAction(this, "SpinSign");
        MR::startAction(pCastle->mKoopaJr, "SpinSign");
        MR::startBrk(pCastle->mCapsule, "SpinSign");
    }
    MR::startActionSound(this, "EmLvWaterbazPreStorm", -1, -1, -1);
    if (isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvF::sInstance)) {
        if (!KoopaJrCastleFunction::isPlayerOnCastle() && MR::isOnGroundPlayer()) {
            setNerveD();
            return;
        }
    }
    KoopaJrCastleFunction::fn_800D5040(0.4f);
    if (isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvG::sInstance)) {
        if (MR::isGreaterStep(this, 0x59)) {
            setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvH::sInstance);
        }
    } else if (MR::isGreaterStep(this, 0x78)) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvH::sInstance);
    }
}

// fn_800D5D20: 'E' phase: eases the head's rest matrix back over the nerve step, with the burst sound.
// NONMATCHING: rest-matrix and rotation axis not recovered (target passes an unset register to rotateVecDegree)
void KoopaJrCastleHead::exe800D5D20() {
    if (MR::isFirstStep(this)) {
        KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
        MR::startAction(this, "Spin");
        MR::startAction(pCastle->mKoopaJr, "Spin");
        MR::validateHitSensor(this, "Panic");
    }
    if (MR::isStep(this, 0x5A)) {
        MR::invalidateHitSensor(this, "Panic");
    }
    TVec3f pos(_A4[0][3], _A4[1][3], _A4[2][3]);
    TVec3f side(_A4[0][0], _A4[1][0], _A4[2][0]);
    TVec3f up(0.0f, 1.0f, 0.0f);
    f32 ease = MR::calcNerveEaseOutValue(this, 0x96, 25.0f, 0.0f);
    MR::rotateVecDegree(&side, side, up, ease);
    MR::makeMtxSideUpPos(reinterpret_cast<TPos3f*>(&_A4), side, up, pos);
    MR::startActionSound(this, "EmLvWaterbazStorm", static_cast<s32>(100.0f * ease / 25.0f), -1, -1);
    if (MR::isGreaterStep(this, 0x96)) {
        setNerveD();
    }
}

// fn_800D5E90: true when the head is idle enough to take a new action (not in A/B/C and not in the E..H group).
bool KoopaJrCastleHead::fn_800D5E90() {
    if (isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvA::sInstance) || isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvB::sInstance) ||
        isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance)) {
        return false;
    }
    return !fn_800D58D0();
}

// fn_800D5F10: refreshes the base matrix and registers the joint controller callback.
void KoopaJrCastleHead::calcAndSetBaseMtx() {
    PartsModel::calcAndSetBaseMtx();
    _A0->registerCallBack();
}

// fn_800D5F50: writes the head's pose matrix from the given matrix's translation (idle head only).
// NONMATCHING: joint concat order and setMtxTrans argument shape inferred from the m2c read only
bool KoopaJrCastleHead::fn_800D5F50(MtxPtr pMtx) {
    if (_D4 || isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance)) {
        return false;
    }
    TPos3f mtx;
    PSMTXIdentity(mtx);
    MtxPtr jointMtx = MR::getJointMtx(this, "JointRoot");
    PSMTXConcat(_A4, jointMtx, mtx);
    TVec3f pos(0.0f);
    MR::extractMtxTrans(pMtx, &pos);
    MR::setMtxTrans(mtx, pos.x, pos.y, pos.z);
    return true;
}

// fn_800D6030: operator delete wrapper used by the array/destructor helpers.
void* fn_800D6030(void* pMem, s32 count) {
    if (pMem != nullptr && count > 0) {
        operator delete(pMem);
    }
    return pMem;
}

KoopaJrCastleHead::~KoopaJrCastleHead() {}
