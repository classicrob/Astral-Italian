#include "Game/Enemy/MogucchiShooter.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/HomingKiller.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/ActorSensorUtil.hpp"

// Nerves of the shooter: SMG2 target instances 0x807D2C00..0x807D2C20. Each execute runs the
// fn_ body below (binding decoded from the setNerve calls; see renames file for the names).
namespace NrvMogucchiShooter {
    class Nrv2C00 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->exeShooterWaitSwitch(); }
        static Nrv2C00 sInstance;
    };
    Nrv2C00 Nrv2C00::sInstance ATTRIBUTE_WEAK;

    class Nrv2C04 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->exeShooterHide(); }
        static Nrv2C04 sInstance;
    };
    Nrv2C04 Nrv2C04::sInstance ATTRIBUTE_WEAK;

    class Nrv2C08 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->exeShooterAppear(); }
        static Nrv2C08 sInstance;
    };
    Nrv2C08 Nrv2C08::sInstance ATTRIBUTE_WEAK;

    class Nrv2C0C : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->exeShooterBreak(); }
        static Nrv2C0C sInstance;
    };
    Nrv2C0C Nrv2C0C::sInstance ATTRIBUTE_WEAK;

    class Nrv2C10 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->exeShooterShotWait(); }
        static Nrv2C10 sInstance;
    };
    Nrv2C10 Nrv2C10::sInstance ATTRIBUTE_WEAK;

    class Nrv2C14 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->exeShooterShotSign(); }
        static Nrv2C14 sInstance;
    };
    Nrv2C14 Nrv2C14::sInstance ATTRIBUTE_WEAK;

    class Nrv2C18 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->exeShooterShotSignWait(); }
        static Nrv2C18 sInstance;
    };
    Nrv2C18 Nrv2C18::sInstance ATTRIBUTE_WEAK;

    class Nrv2C1C : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast< MogucchiShooter* >(pSpine->mExecutor)->exeShooterShotStart(); }
        static Nrv2C1C sInstance;
    };
    Nrv2C1C Nrv2C1C::sInstance ATTRIBUTE_WEAK;

    class Nrv2C20 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            MogucchiShooter* pActor = static_cast< MogucchiShooter* >(pSpine->mExecutor);
            if (MR::updateActorState(pActor) && !pActor->tryHideShooterNearPlayer()) {
                pActor->setNerve(&Nrv2C10::sInstance);
            }
        }
        static Nrv2C20 sInstance;
    };
    Nrv2C20 Nrv2C20::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvMogucchiShooter

#define MS_NRV(name) (&NrvMogucchiShooter::name::sInstance)

namespace {
    TVec3f sPropellerAxis(1.0f, 0.0f, 0.0f);
    TVec3f sShotOffset(-100.0f, 5.0f, 0.0f);
    s32 sSwingPeriod = 240;
    extern const f32 sViewAngle;
    extern const f32 sViewHalf;
};

MogucchiShooter::MogucchiShooter(const char* pName) : LiveActor(pName) {
    mHomePos.set(0.0f, 0.0f, 0.0f);
    mFront.set(0.0f, 0.0f, 0.0f);
    mFront.z = 1.0f;
    mAimDirection.set(0.0f, 0.0f, 1.0f);
    mSwayTimer = 0;
    mShotTimer = 0;
    mPropellerStep = 0;
    mShotInterval = 90;
    mProjectileKind = 0;
    mUnusedFlag = 1;
    mHideDelay = 0;
    mSpike = nullptr;
    mBreakModel = nullptr;
    mHomingKiller = nullptr;
    mBindStarPointer = nullptr;
}

MogucchiShooter::~MogucchiShooter() {}

// NONMATCHING: callback stack placement and string-pool address-load scheduling; calls, literals and field offsets verified.
void MogucchiShooter::init(const JMapInfoIter& rIter) {
    MR::initRotation(&mAimDirection, rIter);
    MR::initActor(this, rIter, "GliderBazooka", false);
    const char* objectName;
    MR::getObjectName(&objectName, rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mShotInterval);
    if (MR::isEqualObjectName(rIter, "KillerShooter")) {
        mProjectileKind = 1;
        mHomingKiller = new HomingKiller("ホーミングキラー");
        mHomingKiller->initWithoutIter();
        mHomingKiller->makeActorDead();
        if (MR::isValidSwitchA(this)) {
            MR::listenStageSwitchOnOffA(this, MR::Functor(this, &MogucchiShooter::invalidateShooterClipping), MR::Functor(this, &MogucchiShooter::validateShooterClipping));
        }
    } else {
        mProjectileKind = 0;
        MR::createSceneObj(0x8D);
    }
    bool (MogucchiShooter::*callback)(TPos3f*, const JointControllerInfo&) = &MogucchiShooter::calcShooterPropellerJoint;
    const char* jointName = "Propeller";
    JointControlDelegator< MogucchiShooter >* pJoint = new JointControlDelegator< MogucchiShooter >(this, callback, nullptr);
    MR::setJointControllerParam(pJoint, this, jointName);
    mJointCtrl = pJoint;
    MogucchiSpike* spike = new MogucchiSpike(this, MR::getJointMtx(this, "Cockpit"));
    mSpike = spike;
    mBreakModel = MR::createModelObjEnemy("壊れモデル", "GliderBazookaBreak", getBaseMtx());
    mBreakModel->makeActorDead();
    if (MR::isValidSwitchA(this)) {
        initNerve(MS_NRV(Nrv2C00), 1);
    } else {
        initNerve(MS_NRV(Nrv2C10), 1);
    }
    mBindStarPointer = new WalkerStateBindStarPointer(this, mSpike->mScaleCtrl);
    MR::initActorState(this, mBindStarPointer, MS_NRV(Nrv2C20), "SupportFreeze");
    MR::startAction(this, "ShotSign");
    mShotInterval -= MR::getBckFrameMax(this);
    MR::startAction(this, "ShotStart");
    mShotInterval -= MR::getBckFrameMax(this);
    MR::startAction(this, "ShotWait");
    MR::startAction(mSpike, "ShotWait");
    makeActorAppeared();
    MR::extractMtxYDir(getBaseMtx(), &mUpVec);
    MR::extractMtxZDir(getBaseMtx(), &mFront);
    mHomePos.set(mPosition);
}

#pragma auto_inline on
// NONMATCHING: integer and boolean register allocation differs; branch structure matches.
void MogucchiShooter::control() {
    mPropellerStep++;
    if (!isNerve(MS_NRV(Nrv2C14)) && !isNerve(MS_NRV(Nrv2C18)) && !isNerve(MS_NRV(Nrv2C1C)) && !isNerve(MS_NRV(Nrv2C0C))) {
        mSwayTimer++;
    }
    f32 phase = 360.0f * (mSwayTimer % sSwingPeriod) / sSwingPeriod;
    mPosition = mHomePos + mUpVec * (20.0f * MR::sinDegree(phase));
    bool v1;
    bool v3;
    bool v2;
    v1 = true;
    v2 = true;
    v3 = true;
    if (!isNerve(MS_NRV(Nrv2C00)) && !isNerve(MS_NRV(Nrv2C08))) { v2 = false; }
    if (!v2 && !isNerve(MS_NRV(Nrv2C04))) { v3 = false; }
    if (!v3 && !isNerve(MS_NRV(Nrv2C10))) { v1 = false; }
    if (v1 && mBindStarPointer->tryStartPointBind()) setNerve(MS_NRV(Nrv2C20));

}

#pragma auto_inline off

void MogucchiShooter::endClipped() {
    LiveActor::endClipped();
    if (mProjectileKind != 1 && !isNerve(MS_NRV(Nrv2C0C))) {
        getShooterTargetMotion(&mAimDirection, nullptr);
        mAimDirection.sub(mPosition);
        MR::normalize(&mAimDirection);
    }
}

void MogucchiShooter::calcAndSetBaseMtx() {
    mJointCtrl->registerCallBack();
    MR::calcAndSetBaseMtxFromFrontAndGravity(this, &mAimDirection);
}

void MogucchiShooter::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool MogucchiShooter::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiProjectileOrEat(msg)) {
        return tryBreakShooterAtPosition(pSender->mPosition);
    }
    if (MR::isMsgStarPieceReflect(msg) || MR::isMsgStarPieceAttack(msg)) {
        tryHideShooterOnDamage();
        return true;
    }
    return false;
}

bool MogucchiShooter::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg)) {
        return tryBreakShooterAtPosition(pSender->mPosition);
    }
    return false;
}

bool MogucchiShooter::calcShooterPropellerJoint(TPos3f* pMtx, const JointControllerInfo&) {
    s32 period = 20;
    TVec3f angle(sPropellerAxis * (360.0f * (mPropellerStep % period) / period));
    TPos3f rot;
    PSMTXIdentity(rot.mMtx);
    MR::makeMtxRotate(rot.mMtx, angle);
    PSMTXConcat(pMtx->mMtx, rot.mMtx, pMtx->mMtx);
    return true;

}

void MogucchiShooter::invalidateShooterClipping() {
    MR::invalidateClipping(this);
}

void MogucchiShooter::validateShooterClipping() {
    MR::validateClipping(this);
}

void MogucchiShooter::getShooterTargetMotion(TVec3f* pPos, TVec3f* pVel) {
    if (MR::getCurrentRushActor()) {
        if (pPos) pPos->set(MR::getCurrentRushActor()->mPosition);
        if (pVel) pVel->set(MR::getCurrentRushActor()->mVelocity);
    } else {
        if (pPos) pPos->set(*MR::getPlayerPos());
        if (pVel) pVel->set(*MR::getPlayerVelocity());
    }

}

// NONMATCHING: vector temporary stack slots and f29/f30 allocation differ; calls, constants and branches match.
void MogucchiShooter::aimShooterAtPredictedTarget(TVec3f* pFront, const TVec3f& rPos) {
    f32 distance;
    f32 bestDist;

    TVec3f origin(0.0f);
    TVec3f velocity(0.0f);
    getShooterTargetMotion(&origin, &velocity);
    TVec3f best(rPos);
    TVec3f muzzle(rPos + *pFront * 800.0f);
    bestDist = 3.4028234663852886e+38f;
    for (s32 i = 0; i < 150; i++) {
        TVec3f candidate(origin + velocity * i);
        TVec3f direction(candidate);
        direction.sub(muzzle);
        MR::normalize(&direction);
        TVec3f shot(muzzle + (direction * 22.0f) * i);
        distance = PSVECDistance(&shot, &candidate);
        if (distance < bestDist) {
            best.set(candidate);
            bestDist = distance;
        }
    }
    TVec3f dir(best);
    dir.sub(rPos);
    MR::normalize(&dir);
    TVec3f up;
    up.set(0.0f, 1.0f, 0.0f);
    MR::turnVecToVecDegree(pFront, *pFront, dir, 0.3f, up);

}

void MogucchiShooter::exeShooterWaitSwitch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startAction(mSpike, "Wait");
    }
    if (isShooterTargetInView()) {
        aimShooterAtPredictedTarget(&mAimDirection, mPosition);
    }
    if (MR::isOnSwitchA(this)) {
        if (mProjectileKind == 1) {
            MR::validateClipping(this);
        }
        setNerve(MS_NRV(Nrv2C10));
    }
}

void MogucchiShooter::exeShooterAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mSpike, "Appear");
    }
    if (MR::isActionEnd(mSpike)) {
        setNerve(MS_NRV(Nrv2C10));
    }
}

void MogucchiShooter::exeShooterHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::setBckFrameEndAndStop(this);
        const char* pName = MR::getPlayingBckName(mSpike);
        if (!MR::isEqualString(pName, "Hide") && !MR::isEqualString(pName, "HideAttack")) {
            MR::startAction(mSpike, "Hide");
        }
    }
    tryAppearShooterAfterDelay();
}

void MogucchiShooter::exeShooterBreak() {
    if (MR::isFirstStep(this)) {
        MR::emitItemHit(this);
        MR::tryHitReactionBreak(this);
        MR::hideModel(this);
        MR::invalidateHitSensors(this);
        mBreakModel->makeActorAppeared();
        MR::startAction(mBreakModel, "Break");
        MR::startAction(mSpike, "Damage");
    }
    if (!MR::isDead(mBreakModel) && MR::isActionEnd(mBreakModel)) {
        mBreakModel->makeActorDead();
    }
    if (!MR::isDead(mSpike) && MR::isActionEnd(mSpike)) {
        mSpike->makeActorDead();
    }
    if (MR::isDead(mBreakModel) && MR::isDead(mSpike)) {
        makeActorDead();
    }
}

void MogucchiShooter::exeShooterShotWait() {
    s32 fn = isShooterTargetInView();
    if (fn != 0) {
        aimShooterAtPredictedTarget(&mAimDirection, mPosition);
    }
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ShotWait");
        MR::startAction(mSpike, "ShotWait");
    }
    if (tryWaitShooterSwitchOff() == 0 && tryHideShooterNearPlayer() == 0) {
        if (tryStartShooterShotSign()) return;
    }
}

void MogucchiShooter::exeShooterShotSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ShotSign");
        MR::startAction(mSpike, "ShotSign");
        appearShooterHomingKiller();
    }
    if (MR::isActionEnd(this)) {
        setNerve(MS_NRV(Nrv2C18));
    }
}

// NONMATCHING: epilogue restores LR and r31 in reverse order.
void MogucchiShooter::exeShooterShotSignWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mSpike, "ShotSignWait");
    }
    tryStartShooterShot();
}

void MogucchiShooter::exeShooterShotStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ShotStart");
        MR::startAction(mSpike, "ShotStart");
        fireShooterBullet();
    }
    if (MR::isActionEnd(mSpike)) {
        setNerve(MS_NRV(Nrv2C10));
    }
}

void MogucchiShooter::appearShooterHomingKiller() {
    if (mProjectileKind == 1) {
        TPos3f mtx;
        mtx.set(MR::getJointMtx(this, "Bazooka"));
        TVec3f direction;
        TVec3f trans;
        TVec3f offset;
        MR::extractMtxXDir(mtx.mMtx, &direction);
        MR::extractMtxTrans(mtx.mMtx, &trans);
        MR::setMtxTrans(mtx.mMtx, 0.0f, 0.0f, 0.0f);
        mtx.mult(sShotOffset, offset);
        MR::normalizeOrZero(&direction);
        mHomingKiller->appear(trans + offset, direction);
    }

}

void MogucchiShooter::fireShooterBullet() {
    if (mProjectileKind == 0) {
        MtxPtr mtx = MR::getJointMtx(this, "Bazooka");
        TVec3f direction;
        TVec3f trans;
        MR::extractMtxXDir(mtx, &direction);
        MR::extractMtxTrans(mtx, &trans);
        MR::normalizeOrZero(&direction);
        LiveActorGroup* group = static_cast<LiveActorGroup*>(MR::getSceneObjHolder()->getObj(0x8D));
        LiveActor* actor = group->getDeadActor();
        if (actor) {
            MR::setPosition(actor, trans);
            MR::setVelocity(actor, direction * 22.0f);
            actor->appear();
        }
    }

}

bool MogucchiShooter::tryBreakShooterAtPosition(const TVec3f& rPos) {
    TPos3f mtx;
    TVec3f dir;
    TVec3f up;
    TVec3f rotation;
    MR::extractMtxYDir(getBaseMtx(), &up);
    const TVec3f& spikePosition = mSpike->mPosition;
    TVec3f difference(rPos);
    difference.sub(spikePosition);
    MR::normalizeOrZero(&dir, difference);
    MR::makeMtxUpFront(&mtx, up, dir);
    mtx.getEuler(rotation);
    mSpike->mCalcOwnMatrix = false;
    MR::setRotation(mSpike, 57.295780181884766f * rotation.x, 57.295780181884766f * rotation.y, 57.295780181884766f * rotation.z);
    MR::invalidateHitSensors(mSpike);
    setNerve(MS_NRV(Nrv2C0C));
    return 1;

}

s32 MogucchiShooter::isShooterTargetInView() {
    if (mProjectileKind == 0) {
        return 1;
    }
    TVec3f up;
    MR::extractMtxYDir(getBaseMtx(), &up);
    TVec3f origin;
    getShooterTargetMotion(&origin, nullptr);
    TVec3f sp8(origin);
    sp8.sub(mPosition);
    return MR::isNearAngleDegreeHV(sp8, mFront, up, sViewAngle * sViewHalf, 360.0f);
}

s32 MogucchiShooter::tryHideShooterOnDamage() {
    bool allowed = isNerve(MS_NRV(Nrv2C00)) || isNerve(MS_NRV(Nrv2C08)) || isNerve(MS_NRV(Nrv2C04)) || isNerve(MS_NRV(Nrv2C10));
    if (allowed) {
        mHideDelay = 70;
        setNerve(MS_NRV(Nrv2C04));
        return 1;
    }
    return 0;
}

s32 MogucchiShooter::tryHideShooterNearPlayer() {
    if (MR::isNearPlayer(this, 1000.0f)) {
        setNerve(MS_NRV(Nrv2C04));
        return 1;
    }
    return 0;
}

s32 MogucchiShooter::tryWaitShooterSwitchOff() {
    bool isValid = MR::isValidSwitchA(this);
    if (isValid && !MR::isOnSwitchA(this)) {
        setNerve(MS_NRV(Nrv2C00));
        return 1;
    }
    return 0;
}

s32 MogucchiShooter::tryAppearShooterAfterDelay() {
    if (mHideDelay > 0) {
        mHideDelay -= 1;
        if (mHideDelay > 0) {
            return 0;
        }
    }
    if (!MR::isNearPlayer(this, 1200.0f)) {
        setNerve(MS_NRV(Nrv2C08));
        return 1;
    }
    return 0;
}

s32 MogucchiShooter::tryStartShooterShotSign() {
    if (mProjectileKind != 1 || MR::isDead(mHomingKiller)) {
        mShotTimer += 1;
    }
    if (mShotTimer >= mShotInterval) {
        mShotTimer = 0;
        setNerve(MS_NRV(Nrv2C14));
        return 1;
    }
    return 0;
}

s32 MogucchiShooter::tryStartShooterShot() {
    bool done;
    if (mProjectileKind == 1) {
        done = !mHomingKiller->isNerveAppear();
    } else {
        done = MR::isBckLooped(mSpike);
    }
    if (done) {
        setNerve(MS_NRV(Nrv2C1C));
        return 1;
    }
    return 0;
}

// ---- MogucchiSpike ----


void MogucchiSpike::calcAndSetBaseMtx() {
    MR::updateBaseScale(this, mScaleCtrl);
    PartsModel::calcAndSetBaseMtx();
}

void MogucchiSpike::control() {
    mScaleCtrl->updateNerve();
}

#pragma pool_strings off
inline void MogucchiSpike::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver) && MR::isSensorEnemyAttack(pSender)) {
        MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
        const char* name = MR::getPlayingBckName(this);
        if (MR::isEqualString(name, "Hide") || MR::isEqualString(name, "HideAttack")) {
            MR::startAction(this, "HideAttack");
        }
        return;
    }
    MR::sendMsgPush(pReceiver, pSender);
}

#pragma pool_strings reset

bool MogucchiSpike::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return MR::sendMsgStarPieceAttack(pReceiver, pSender);
    }
    return false;
}

namespace {
    const f32 sViewAngle = 90.0f;
    const f32 sViewHalf = 0.5f;
}

#pragma pool_strings off
inline MogucchiSpike::MogucchiSpike(LiveActor* pActor, MtxPtr pMtx)
    : PartsModel(pActor, "狙撃手", "MogucchiSpike", pMtx, 0x14, false) {
    mScaleCtrl = new AnimScaleController(nullptr);
    initWithoutIter();
    mCheckHiddenHost = true;
}
#pragma pool_strings reset

