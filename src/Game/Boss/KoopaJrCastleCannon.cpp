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

// PLACEHOLDER(objects): unnamed KoopaJrCastleFunction helper (0x378, gauge step check, returns s32); resolve when core names fn_800D4C40
extern "C" s32 fn_800D4C40(f32, f32);
// PLACEHOLDER(objects): unnamed KoopaJrCastleFunction helper (getJointMtx(head, "Cannon3") wrapper); resolve when names fn_800D5720
extern "C" MtxPtr fn_800D5720(LiveActor*, KoopaJrCastle*);
// PLACEHOLDER(objects): unnamed KoopaJrCastleCannonBall shoot helper (ball, mtx, speed); resolve when CannonBall TU names fn_800D1770
extern "C" void fn_800D1770(LiveActor*, MtxPtr, f32*);

namespace NrvKoopaJrCastleCannon { KoopaJrCastleCannonNrvPre KoopaJrCastleCannonNrvPre::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannon { KoopaJrCastleCannonNrvA KoopaJrCastleCannonNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannon { KoopaJrCastleCannonNrvB KoopaJrCastleCannonNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannon { KoopaJrCastleCannonNrvC KoopaJrCastleCannonNrvC::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannon { KoopaJrCastleCannonNrvD KoopaJrCastleCannonNrvD::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannon { KoopaJrCastleCannonNrvE KoopaJrCastleCannonNrvE::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannonShock { KoopaJrCastleCannonShockNrvA KoopaJrCastleCannonShockNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannonShock { KoopaJrCastleCannonShockNrvB KoopaJrCastleCannonShockNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleCannonShock { KoopaJrCastleCannonShockNrvC KoopaJrCastleCannonShockNrvC::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleCannon::KoopaJrCastleCannon(KoopaJrCastle* pHost, const char* pName)
    : LiveActor(pName), mHost(pHost), mShock(nullptr), _C8(1), _CC(0), _D0(0), _D4(false), _D8(0.0f) {
    for (int i = 0; i < 12; i++) {
        mBall[i] = nullptr;
    }
    MR::setPosition(this, pHost->mPosition);
}

void KoopaJrCastleCannon::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 0x29, -1, -1, -1);
    mShock = new KoopaJrCastleCannonShock("\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x92""e");
    mShock->initWithoutIter();
    for (int i = 0; i < 12; i++) {
        mBall[i] = new KoopaJrCastleCannonBall("\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x92""e");
        MR::setScale(mBall[i], 1.5f);
        mBall[i]->initWithoutIter();
    }
    initNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvA::sInstance, 0);
    MR::invalidateClipping(this);
    makeActorDead();
}

void KoopaJrCastleCannon::appear() {
    LiveActor::appear();
    setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvA::sInstance);
    _CC = 0;
    _D4 = false;
    _D8 = 0.0f;
}

void KoopaJrCastleCannon::fn_800D0540() {
    KoopaJrCastleCannon::appear();
    setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvB::sInstance);
}

void KoopaJrCastleCannon::fn_800D0580() {
    KoopaJrCastleCannon::appear();
    setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvB::sInstance);
    _D4 = true;
}

void KoopaJrCastleCannon::fn_800D05C0() {
    KoopaJrCastleCannon::appear();
    setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvC::sInstance);
    _D4 = true;
}

void KoopaJrCastleCannon::makeActorDead() {
    LiveActor::makeActorDead();
    mHost->mHead->fn_800D57D0();
}

void KoopaJrCastleCannon::exe800D0640() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(mHost->mHead, "Aim");
    }
    MR::startActionSound(mHost->mHead, "EmLvWaterbazTurn", -1, -1, -1);
    MR::startActionSound(mHost->mHead, "PreShoot", -1, -1, -1);
    updateGauge();
    if (fn_800D4C40(_D8, 15.0f) != 0 || MR::isGreaterEqualStep(this, 40)) {
        if (_D4) {
            setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvE::sInstance);
            return;
        }
        setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvC::sInstance);
    }
}

void KoopaJrCastleCannon::exe800D0720() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(mHost->mHead, "Aim");
    }
    if (_D4 && !MR::isEffectValid(mHost->mHead, "Shot")) {
        MR::emitEffect(mHost->mHead, "Shot");
    }
    MR::startActionSound(mHost->mHead, "EmLvWaterbazTurn", -1, -1, -1);
    updateGauge();
    fn_800D4C40(_D8, 15.0f);
}

void KoopaJrCastleCannon::exe800D07E0() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost->mHead, "Shot");
    }
    if (MR::isStep(this, 20)) {
        MR::startActionSound(mHost->mHead, "EmWaterbazTurnEnd", -1, -1, -1);
    }
    if (MR::isGreaterEqualStep(this, 20) && MR::isLessStep(this, 45)) {
        _D8 = 0.0f;
    } else {
        updateGauge();
        fn_800D4C40(_D8, 15.0f);
    }
    if (MR::isLessStep(this, 30)) {
        MR::startActionSound(mHost->mHead, "PreShoot", -1, -1, -1);
    }
    if (MR::isStep(this, 30)) {
        MR::startActionSound(mHost->mHead, "EmWaterbazShot", -1, -1, -1);
        shoot();
    }
    if (_CC + 1 < _C8) {
        if (MR::isActionEnd(mHost->mHead) && MR::isGreaterStep(this, 60)) {
            _CC += 1;
            setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvC::sInstance);
        }
    } else if (MR::isActionEnd(mHost->mHead)) {
        _CC = 0;
        if (_D0 <= 0) {
            setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvD::sInstance);
            return;
        }
        setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvPre::sInstance);
    }
}

void KoopaJrCastleCannon::exe800D09B0() {
    updateGauge();
    fn_800D4C40(_D8, 15.0f);
    if (MR::isGreaterStep(this, 30)) {
        kill();
    }
}

// NONMATCHING: mShock call: target passes a stale r4 to the 1-arg fn_800D5720 wrapper in this function (2-arg use in shoot); size matches, 1 insn differs
void KoopaJrCastleCannon::exe800D0A10() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost->mHead, "Shot");
    }
    if (MR::isStep(this, 30)) {
        MR::startActionSound(mHost->mHead, "EmWaterbazShot", -1, -1, -1);
        if (MR::isDead(mShock)) {
            MtxPtr pMtx = fn_800D5720(mHost->mHead, mHost);
            TPos3f mtx;
            PSMTXIdentity(mtx);
            TVec3f a, b, c;
            MR::extractMtxXYZDir(pMtx, &a, &b, &c);
            TVec3f pos;
            MR::extractMtxTrans(pMtx, &pos);
            TVec3f offset = a * 900.0f;
            pos += offset;
            MR::setMtxAxisXYZ(mtx, c, b, a);
            MR::setMtxTrans(mtx, pos.x, pos.y, pos.z);
            mShock->shoot(mtx, 12.0f);
        }
    }
    if (MR::isActionEnd(mHost->mHead)) {
        kill();
    }
}

void KoopaJrCastleCannon::killAllBalls() {
    for (int i = 0; i < 12; i++) {
        if (!MR::isDead(mBall[i])) {
            mBall[i]->makeActorDead();
        }
    }
}

bool KoopaJrCastleCannon::shoot() {
    KoopaJrCastle* pCastle = mHost;
    MtxPtr pMtx = fn_800D5720(pCastle->mHead, pCastle);
    KoopaJrCastleCannonBall* pBall = findDeadBall();
    if (pBall == nullptr) {
        return false;
    }
    TPos3f mtx;
    TVec3f a, b, c;
    PSMTXIdentity(mtx);
    MR::extractMtxXYZDir(pMtx, &a, &b, &c);
    TVec3f pos;
    MR::extractMtxTrans(pMtx, &pos);
    TVec3f offset = a * 750.0f;
    pos += offset;
    MR::setMtxAxisXYZ(mtx, c, b, a);
    MR::setMtxTrans(mtx, pos.x, pos.y, pos.z);
    f32 speed = 12.0f;
    if (KoopaJrCastleFunction::getBattlePhase() == 1) {
        speed = 14.0f;
    }
    fn_800D1770(pBall, mtx, &speed);
    _D0 -= 1;
    return true;
}

KoopaJrCastleCannonBall* KoopaJrCastleCannon::findDeadBall() {
    for (int i = 0; i < 12; i++) {
        if (MR::isDead(mBall[i])) {
            return mBall[i];
        }
    }
    return nullptr;
}

// NONMATCHING: constant/operand register order of the 0.02 add differs (fadds operands swapped)
void KoopaJrCastleCannon::updateGauge() {
    f32 temp = 0.02f + _D8;
    _D8 = temp;
    _D8 = MR::clamp(temp, 0.0f, 0.55f);
}

KoopaJrCastleCannonShock::KoopaJrCastleCannonShock(const char* pName) : LiveActor(pName) {
    mDir.x = 0.0f;
    mDir.y = 0.0f;
    mDir.z = 0.0f;
    _9C = 0.0f;
    _A0 = 1.0f;
}

void KoopaJrCastleCannonShock::init(const JMapInfoIter& rIter) {
    MR::initActor(this, "ElectricBullet", "KoopaJrCastle", false);
    initNerve(&NrvKoopaJrCastleCannonShock::KoopaJrCastleCannonShockNrvB::sInstance, 0);
    makeActorDead();
}

void KoopaJrCastleCannonShock::kill() {
    MR::tryHitReactionBreak(this);
    MR::emitEffect(this, "Break");
    MR::startActionSound(this, "EmElecbublletBreak", -1, -1, -1);
    LiveActor::kill();
}

// NONMATCHING: set<f> argument load scheduling differs (target loads matrix entries right to left)
void KoopaJrCastleCannonShock::shoot(MtxPtr pMtx, const f32& rSpeed) {
    _9C = rSpeed;
    mDir.set(pMtx[0][2], pMtx[1][2], pMtx[2][2]);
    mPosition.set(pMtx[0][3], pMtx[1][3], pMtx[2][3]);
    mRotation.z = 0.0f;
    mRotation.y = 0.0f;
    mRotation.x = 0.0f;
    makeActorAppeared();
    MR::validateHitSensors(this);
    MR::invalidateClipping(this);
    setNerve(&NrvKoopaJrCastleCannonShock::KoopaJrCastleCannonShockNrvA::sInstance);
    TVec3f scale(0.1f, 0.1f, 0.1f);
    mScale = scale;
}

// NONMATCHING: f64 clamp result: target re-stores mScale.x from the unrounded double after frsp (CSE differs)
void KoopaJrCastleCannonShock::exe800D0FA0() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shot");
        MR::startBtk(this, "ElectricBullet");
    }
    mScale.x += 0.5f;
    f64 v = MR::clamp(mScale.x, 1.0f, 6.0f);
    mScale.x = v;
    f32 w = v;
    mScale.y = w;
    mScale.z = w;
    MR::setSensorRadius(this, "Body", 75.0f * w);
    MR::setShadowVolumeCylinderRadius(this, nullptr, 80.0f * mScale.y);
    MR::setBinderRadius(this, 75.0f * mScale.y);
    MR::setStarPointerTargetRadius3d(this, 100.0f * mScale.y);
    if (MR::isActionEnd(this)) {
        setNerve(&NrvKoopaJrCastleCannonShock::KoopaJrCastleCannonShockNrvB::sInstance);
    }
}

void KoopaJrCastleCannonShock::exe800D10A0() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Move");
        mVelocity.scale(_9C, mDir);
    }
    MR::startActionSound(this, "EmLvElecbublletFly", -1, -1, -1);
    if (MR::isGreaterStep(this, 10)) {
        if (MR::isBinded(this)) {
            if (MR::isBindedGround(this)) {
                MR::sendMsgEnemyAttackElectric(MR::getGroundSensor(this), getSensor(nullptr));
            } else if (MR::isBindedRoof(this)) {
                MR::sendMsgEnemyAttackElectric(MR::getRoofSensor(this), getSensor(nullptr));
            } else if (MR::isBindedWall(this)) {
                MR::sendMsgEnemyAttackElectric(MR::getWallSensor(this), getSensor(nullptr));
            }
            setNerve(&NrvKoopaJrCastleCannonShock::KoopaJrCastleCannonShockNrvC::sInstance);
            _A0 = mScale.x;
            return;
        }
    }
    if (MR::isStep(this, 300)) {
        setNerve(&NrvKoopaJrCastleCannonShock::KoopaJrCastleCannonShockNrvC::sInstance);
        _A0 = mScale.x;
    }
}

// NONMATCHING: f64 calcNerveEaseOutValue result: target stores mScale.x unrounded, ours CSEs the frsp copy
void KoopaJrCastleCannonShock::exe800D1220() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
    }
    f64 v = MR::calcNerveEaseOutValue(this, 8, _A0, 1.0f);
    mScale.x = v;
    f32 w = v;
    mScale.y = w;
    mScale.z = w;
    MR::setSensorRadius(this, "Body", 75.0f * w);
    MR::setShadowVolumeCylinderRadius(this, nullptr, 80.0f * mScale.y);
    MR::setBinderRadius(this, 75.0f * mScale.y);
    if (MR::isGreaterStep(this, 8)) {
        makeActorDead();
    }
}

void KoopaJrCastleCannonShock::control() {
    if (MR::isStarPointerPointing2PAttachTarget(this) && !isNerve(&NrvKoopaJrCastleCannonShock::KoopaJrCastleCannonShockNrvC::sInstance)) {
        setNerve(&NrvKoopaJrCastleCannonShock::KoopaJrCastleCannonShockNrvC::sInstance);
        _A0 = mScale.x;
    }
}

void KoopaJrCastleCannonShock::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver) && MR::sendMsgEnemyAttackElectric(pReceiver, pSender)) {
        setNerve(&NrvKoopaJrCastleCannonShock::KoopaJrCastleCannonShockNrvC::sInstance);
        _A0 = mScale.x;
    }
}

KoopaJrCastleCannon::~KoopaJrCastleCannon() {}

KoopaJrCastleCannonShock::~KoopaJrCastleCannonShock() {}
