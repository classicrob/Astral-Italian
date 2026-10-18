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

    bool isPlayerOnCapsule() {
        return MR::isOnPlayer(CASTLE()->mCapsule);
    }

    bool isPlayerOnHead() {
        return MR::isOnPlayer(CASTLE()->mHead);
    }

    bool isPlayerOnCastle() {
        return isPlayerOnCapsule() || isPlayerOnHead();
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

    bool bodyIsNotMove() {
        return !CASTLE()->mBody->isMove();
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

    u8 bodyGetStateFlag() {
        return CASTLE()->mBody->mState->getFlag();
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

    MtxPtr getCannon3Mtx(LiveActor* pHead) {
        return MR::getJointMtx(pHead, "Cannon3");
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
        PSVECSubtract(&playerPos, &offset, &playerPos);
        TVec3f local(playerPos);
        PSMTXMultVec(mtx, playerPos, local);
        TVec3f dir(local);
        PSVECSubtract(&dir, &pos, &dir);
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

    // fn_800D5340: star piece burst from the capsule, opposite its gravity, with the burst sound on the body.
    void appearCapsuleStarPiece() {
        KoopaJrCastle* pCastle = CASTLE();
        const TVec3f& rCapsulePos = pCastle->mCapsule->mPosition;
        TVec3f dir;
        JGeometry::negateInternal(&pCastle->mGravity.x, &dir.x);
        MR::appearStarPieceToDirection(pCastle, rCapsulePos, dir, 15, 30.0f, 40.0f, false);
        MR::startSound(pCastle->mBody, "SE_OJ_STAR_PIECE_BURST", -1, -1);
    }

    bool isElectricActive() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mElectric) && pCastle->mElectric->fn_800D2250()) {
            return true;
        }
        return false;
    }
};  // namespace KoopaJrCastleFunction

KoopaJrCastleHead::KoopaJrCastleHead(LiveActor* pHost, const char* pName) : PartsModel(pHost, pName, "KoopaJrCastleHead", 0, 0x13, false) {
    _A0 = nullptr;
    _D4 = true;
    PSMTXIdentity(_A4);
}

KoopaJrCastleHead::~KoopaJrCastleHead() {}

void KoopaJrCastleHead::setNerveA() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvA::sInstance);
}

void KoopaJrCastleHead::setNerveB() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvB::sInstance);
}

void KoopaJrCastleHead::setNerveC() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance);
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
